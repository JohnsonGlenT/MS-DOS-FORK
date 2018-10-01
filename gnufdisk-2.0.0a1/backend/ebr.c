#include "common.h"

#define EMPTY 0x00
#define DOS_EXTENDED 0x05
#define WINDOWS95_EXTENDED 0x0f
#define LINUX 0x83
#define LINUX_EXTENDED 0x85
#define EFI 0xEE

struct chs {
  unsigned char head;
  unsigned char sector;
  unsigned char cylinder;
} __attribute__((packed));

#define CHS_CYLINDER(_chs) ((_chs)->cylinder | (((_chs)->sector & 0xC0) << 2))
#define CHS_HEAD(_chs) ((_chs)->head)
#define CHS_SECTOR(_chs) ((_chs)->sector & 0x3F)

enum {
  OVERLAP_START, /* start sector overlap with the partition */
  OVERLAP_END, /* end sector overlap with the partition */
  OVERLAP_START_END, /* start before partition start, end after partition end */ 
  OVERLAP_NONE /* no overlaps */
};

static void delete_string(void* _p)
{
  GNUFDISK_LOG((DISKLABEL, "delete struct gnufdisk_string* %p", _p));
  gnufdisk_string_delete(_p);
}

static void delete_ebr_chain(void* _p)
{
  GNUFDISK_LOG((DISKLABEL, "delete struct ebr_chain* %p", _p));
  free(_p);
}

static void delete_list(void* _p)
{
  GNUFDISK_LOG((DISKLABEL, "delete struct list* %p", _p));
  list_delete(_p, &delete_ebr_chain);
}

static void delete_object(void* _p)
{
  GNUFDISK_LOG((DISKLABEL, "delete struct object* %p", _p));
  object_delete(_p);
}

static gnufdisk_integer chs_to_lba(struct chs* _chs, struct object* _device)
{
  struct gnufdisk_string* param;
  gnufdisk_integer sectors;
  gnufdisk_integer heads;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DISKLABEL, "perform chs_to_lba on struct chs* %p", _chs));
  GNUFDISK_LOG((DISKLABEL, 
		"values: C: %u, H: %u, S: %u", 
		CHS_CYLINDER(_chs),
		CHS_HEAD(_chs),
		CHS_SECTOR(_chs)));

  if((param = gnufdisk_string_new("")) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&delete_string, param);

  if(gnufdisk_string_set(param, "SECTORS") == -1)
    THROW_ENOMEM;

  device_get_parameter(_device, param, &sectors, sizeof(gnufdisk_integer));

  GNUFDISK_LOG((DISKLABEL, "device sectors per track: %" PRId64, sectors));

  if(gnufdisk_string_set(param, "HEADS") == -1)
    THROW_ENOMEM;

  device_get_parameter(_device, param, &heads, sizeof(gnufdisk_integer));

  GNUFDISK_LOG((DISKLABEL, "device heads: %" PRId64, heads));

  ret = (CHS_CYLINDER(_chs) * heads + CHS_HEAD(_chs)) * sectors + (CHS_SECTOR(_chs) - 1);

  gnufdisk_exception_unregister_unwind_handler(&delete_string, param);
  gnufdisk_string_delete(param);

  GNUFDISK_LOG((DISKLABEL, "done perform chs_to_lba, result: %" PRId64, ret));

  return ret;
}

static struct chs chs_from_lba(gnufdisk_integer _lba, struct object* _device)
{
  struct gnufdisk_string* param;
  gnufdisk_integer sectors;
  gnufdisk_integer heads;
  int c;
  int h;
  int s;
  struct chs ret;

  GNUFDISK_LOG((DISKLABEL, "perform chs_from_lba on LBA %"PRId64, _lba));

  if((param = gnufdisk_string_new("")) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&delete_string, param);

  if(gnufdisk_string_set(param, "SECTORS") == -1)
    THROW_ENOMEM;

  device_get_parameter(_device, param, &sectors, sizeof(gnufdisk_integer));

  if(sectors == 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid number of sectors");

  GNUFDISK_LOG((DISKLABEL, "device sectors per track: %" PRId64, sectors));

  if(gnufdisk_string_set(param, "HEADS") == -1)
    THROW_ENOMEM;

  device_get_parameter(_device, param, &heads, sizeof(gnufdisk_integer));

  if(heads == 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid number of heads");

  GNUFDISK_LOG((DISKLABEL, "device heads: %" PRId64, heads));

  c = (_lba / sectors) / heads;
  h = (_lba / sectors) % heads;
  s = (_lba % sectors) + 1;

  ret.cylinder = c & 0xFF; /* eight bits cylinder */
  ret.head = h & 0xFF; /* eight bits head */
  ret.sector = (s & 0x3F) | ((c & 0x300) >> 2); /* five bits sector, 2 bits cylinder */ 

  gnufdisk_exception_unregister_unwind_handler(&delete_string, param);
  gnufdisk_string_delete(param);

  GNUFDISK_LOG((DISKLABEL, 
		"done perform chs_from_lba, result: (C: %u, H: %u S: %u)", 
		CHS_CYLINDER(&ret),
		CHS_HEAD(&ret),
		CHS_SECTOR(&ret)));

  return ret;
}

struct ebr {
  unsigned char code[446];
  struct ebr_partition {
    unsigned char status;
    struct chs first_sector; /* first absolute sector */
    unsigned char type;
    struct chs last_sector; /*last absolute sector */
    uint32_t first_lba; /* first absolute sector */
    uint32_t sectors; /* number of sectors */
  } partitions[4];
  unsigned char magic[2]; 
} __attribute__((packed));

struct ebr_chain {
  struct object* partition;
  struct ebr data;  
  gnufdisk_integer start;
};

struct ebr_private {
  struct object* parent;
  struct list* chain;
  int lba;
};

static void ebr_chain_check(struct ebr_chain* _chain)
{
  if(gnufdisk_check_memory(_chain, sizeof(struct ebr_chain), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct ebr_chain* %p", _chain);
}

static void ebr_private_check(struct ebr_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct ebr_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct ebr_private* %p", _private);
}

static int partition_overlap(struct ebr_partition* _part, struct object* _device, gnufdisk_integer _start, gnufdisk_integer _end)
{
  int ret;
  gnufdisk_integer pstart;
  gnufdisk_integer pend;
  
  GNUFDISK_LOG((DISKLABEL, "check wether %" PRId64 "-%" PRId64" overlaps with ebr_partition %p", _start, _end, _part));

  pstart = chs_to_lba(&_part->first_sector, _device);
  pend = chs_to_lba(&_part->last_sector, _device);

  if(_start >= pstart && _start <= pend)
    ret = OVERLAP_START;
  else if(_end >= pstart && _end <= pend)
    ret = OVERLAP_END;
  else if(_start < pstart && _end > pend)
    ret = OVERLAP_START_END;
  else
    ret = OVERLAP_NONE;

  GNUFDISK_LOG((DISKLABEL, "done check partition overlap, result: %d", ret));

  return ret;
}

static void ebr_private_raw(void* _private, void** _dest, size_t* _size)
{
  struct ebr_private* private;
  struct list* iter;
  size_t offset;
  void* buf;

  GNUFDISK_LOG((DISKLABEL, "perform raw on struct ebr_private* %p", _private));

  ebr_private_check(_private);

  private = _private;

  for(iter = list_first(private->chain), offset = 0, buf = NULL; iter != NULL; iter = list_next(iter), offset += sizeof(struct ebr))
    {
      void* tmp;
      struct ebr_chain* entry;

      if((tmp = buf == NULL ? malloc(sizeof(struct ebr)) : realloc(buf, offset + sizeof(struct ebr))) == NULL)
	{
	  if(buf)
	    free(buf);

	  THROW_ENOMEM;
	}

      entry = list_data(iter);

      ebr_chain_check(entry);

      memcpy(tmp + offset, &entry->data, sizeof(struct ebr));

      buf = tmp;
    }

  *_dest = buf;
  *_size = offset;

  GNUFDISK_LOG((DISKLABEL, "done perform raw"));
}

static struct gnufdisk_string* ebr_private_system(void* _private)
{
  struct ebr_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((DISKLABEL, "perform system on struct ebr_private* %p", _private));

  ebr_private_check(_private);

  private = _private;

  if((ret = gnufdisk_string_new("EBR")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((DISKLABEL, "done perform system, result: %p", ret));

  return ret;
}

static struct object* ebr_private_partition(void* _private, size_t _number)
{
  struct ebr_private* private;
  GNUFDISK_RETRY rp0;
  struct list* iter;
  int count;
  struct object* ret;

  GNUFDISK_LOG((DISKLABEL, "perform partition on struct ebr_private* %p", _private));
  GNUFDISK_LOG((DISKLABEL, "number: %u", _number));
  
  ebr_private_check(_private);

  private = _private;
  ret = NULL;

  GNUFDISK_RETRY_SET(rp0);

  for(count = 1, iter = list_first(private->chain); iter != NULL; iter = list_next(iter), count++)
    {
      if(count == _number)
	{
	  struct ebr_chain* data;

	  data = list_data(iter);

	  ebr_chain_check(data);

	  ret = data->partition;
	  break;
	}
    }
  
  if(ret == NULL)
    {
      union gnufdisk_device_exception_data data;

      data.epartitionnumber = &_number;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0,
		     GNUFDISK_DEVICE_EPARTITIONNUMBER,
		     &data,
		     "invalid partition number: %u", _number);
    }

  GNUFDISK_LOG((DISKLABEL, "done perform partition, result: %p", ret));

  return ret;
}

static int ebr_private_count_partitions(void* _private)
{
  struct ebr_private* private;
  struct list* iter;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform count_partitions on struct ebr_private* %p", _private));

  ebr_private_check(_private);

  private = _private;

  for(iter = list_first(private->chain), ret = 0; iter != NULL; iter = list_next(iter), ret++)
    ;

  GNUFDISK_LOG((DISKLABEL, "done perform count_partitions, result: %d", ret));

  return ret;
}

static struct object* ebr_private_create_partition(void* _private, 
						   struct gnufdisk_geometry* _start_range, 
						   struct gnufdisk_geometry* _end_range, 
						   struct gnufdisk_string* _system)
{
  struct ebr_private* private;
  struct object* device;
  struct gnufdisk_string* param;
  char* system;
  gnufdisk_integer sectors;
  gnufdisk_integer sector_size;
  gnufdisk_integer base;
  gnufdisk_integer start;
  gnufdisk_integer end;
  GNUFDISK_RETRY rp0;
  struct list* iter;
  GNUFDISK_RETRY rp1;
  struct object* ret;
  struct list* last_node;
  struct ebr_chain* last_entry;

  param = NULL;
  system = NULL;

  GNUFDISK_LOG((DISKLABEL, "perform create_partition on struct ebr_private* %p", _private));

  ebr_private_check(_private);

  private = _private;

  /* aligns the beginning and the end  of the partition on cylinder  boundary */
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  if((param = gnufdisk_string_new("SECTORS")) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&delete_string, param);

  device_get_parameter(device, param, &sectors, sizeof(gnufdisk_integer));

  if(gnufdisk_string_set(param, "SECTOR-SIZE") == -1)
    THROW_ENOMEM;
  
  device_get_parameter(device, param, &sector_size, sizeof(gnufdisk_integer));

  GNUFDISK_RETRY_SET(rp0);

  base = math_round(gnufdisk_geometry_start(_start_range) + gnufdisk_geometry_length(_start_range) / 2, sectors);
  start = math_round_up(base + 1, sectors);
  end = math_round(gnufdisk_geometry_end(_end_range) - gnufdisk_geometry_length(_end_range) / 2, sectors);

  GNUFDISK_LOG((DISKLABEL, "aligned base: %"PRId64, base));
  GNUFDISK_LOG((DISKLABEL, "aligned start: %" PRId64, start));
  GNUFDISK_LOG((DISKLABEL, "aligned end: %" PRId64, end));

  if(base >= start || end <= start)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "unable to calculate geometry");

  /* check that  the values remain within  the ranges indicated */
  if(base < gnufdisk_geometry_start(_start_range) || base >= gnufdisk_geometry_end(_start_range) || base < object_start(private->parent))
    {
      union gnufdisk_device_exception_data data;

      data.egeometry = _start_range;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, 
		     &rp0,
		     GNUFDISK_DEVICE_EGEOMETRY,
		     &data,
		     "invalid start range");
    }
  else if(end < gnufdisk_geometry_start(_end_range) || end > gnufdisk_geometry_end(_end_range) || end > object_end(private->parent))
    {
      union gnufdisk_device_exception_data data;

      data.egeometry =_end_range;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0, 
		     GNUFDISK_DEVICE_EGEOMETRY,
		     &data,
		     "invalid end range");
    }

  /* check whether the partition will overwrite other partitions */
  for(iter = private->chain; iter != NULL; iter = list_next(iter))
    {
      struct ebr_chain* entry;
      int mode;
      union gnufdisk_device_exception_data data;

      entry = list_data(iter);

      ebr_chain_check(entry);

      mode = partition_overlap(&entry->data.partitions[0], device, base, end);

      if(mode == OVERLAP_START)
	{
	  data.egeometry = _start_range;

	  GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
			 &rp0,
			 GNUFDISK_DEVICE_EGEOMETRY,
			 &data,
			 "start range overlap with another partition");
	}
      else if(mode == OVERLAP_END)
	{
	  data.egeometry = _end_range;

	  GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
			 &rp0, 
			 GNUFDISK_DEVICE_EGEOMETRY,
			 &data,
			 "end range overlap with another partition");
	}
      else if(mode == OVERLAP_START_END)
	{
	  /* start/end overwrites another partition. fix the problem in two steps. */
	  data.egeometry = _start_range;

	  GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
			 &rp0, 
			 GNUFDISK_DEVICE_EGEOMETRY,
			 &data,
			 "partition overwrite another partition");
	}
    }
 
  system = NULL;

  GNUFDISK_RETRY_SET(rp1);

  if(system != NULL)
    {
      /* we come from a throw_handler, system is already allocated */

      gnufdisk_exception_unregister_unwind_handler(&free, system);
      free(system);
      system = NULL;
    }

  if((system = gnufdisk_string_c_string_dup(_system)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, system);

  if(strcasecmp(system, "LOGICAL") == 0)
    ret = logical_new(private->parent, start, end);
  else if(strcasecmp(system, "EXTENDED") == 0)
    /* only linux supports nested extended partitions */
    ret = extended_new(private->parent, start, end, 1);
  else
    {
      union gnufdisk_device_exception_data data;

      data.epartitiontype = _system;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, 
		     &rp1,
		     GNUFDISK_DEVICE_EPARTITIONTYPE,
		     &data,
		     "unknown partition type: %s", system);
    }

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);

  last_node = list_last(private->chain);
  last_entry = list_data(last_node);

  ebr_chain_check(last_entry);
  
  if(gnufdisk_check_memory(last_entry->partition, 1, 1) != 0)
    {
      /* empty extended partition */
      GNUFDISK_LOG((DISKLABEL, "ebr entry empty, use it"));

      last_entry->data.magic[0] = 0x55;
      last_entry->data.magic[1] = 0xAA;
      last_entry->data.partitions[0].status = 0x00;
      last_entry->data.partitions[0].type = strcasecmp(system, "EXTENDED") == 0 ? LINUX_EXTENDED : LINUX;
      last_entry->data.partitions[0].first_sector = chs_from_lba(start, device);
      last_entry->data.partitions[0].last_sector = chs_from_lba(end, device);
      last_entry->data.partitions[0].first_lba = CPU_TO_LE32(start - base);
      last_entry->data.partitions[0].sectors = CPU_TO_LE32(end - start + 1);
      last_entry->start = base;
      last_entry->partition = ret;
    }
  else
    {
      struct ebr_chain* new_entry;

      GNUFDISK_LOG((DISKLABEL, "ebr entry not empty, link to a new entry"));

      /* link last entry */
      if(last_entry->data.partitions[1].first_sector.cylinder != 0 
	  || last_entry->data.partitions[1].first_sector.head != 0 
	  || last_entry->data.partitions[1].first_sector.sector != 0 
	  || last_entry->data.partitions[1].first_lba != 0)
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "last ebr entry damaged, abort operation");

      last_entry->data.partitions[1].status = 0x00;
      last_entry->data.partitions[1].type = strcasecmp(system, "EXTENDED") == 0 ? LINUX_EXTENDED : LINUX;
      last_entry->data.partitions[1].first_sector = chs_from_lba(base, device);
      last_entry->data.partitions[1].last_sector = chs_from_lba(end, device);
      last_entry->data.partitions[1].first_lba = CPU_TO_LE32(base - last_entry->start);
      last_entry->data.partitions[1].sectors = CPU_TO_LE32(end - base + 1);

      /* create new entry */
      if((new_entry = malloc(sizeof(struct ebr_chain))) == NULL)
	THROW_ENOMEM;

      gnufdisk_exception_register_unwind_handler(&free, new_entry);

      memset(new_entry, 0, sizeof(struct ebr_chain));
      
      new_entry->data.magic[0] = 0x55;
      new_entry->data.magic[1] = 0xAA;
      new_entry->data.partitions[0].status = 0x00;
      new_entry->data.partitions[0].type = strcasecmp(system, "EXTENDED") == 0 ? LINUX_EXTENDED : LINUX;
      new_entry->data.partitions[0].first_sector = chs_from_lba(start, device);
      new_entry->data.partitions[0].last_sector = chs_from_lba(end, device);
      new_entry->data.partitions[0].first_lba = CPU_TO_LE32(start - base);
      new_entry->data.partitions[0].sectors = CPU_TO_LE32(end - start + 1);
      new_entry->start = base;
      new_entry->partition = ret;
      
      private->chain = list_append(private->chain, new_entry);
  
      gnufdisk_exception_unregister_unwind_handler(&free, new_entry);
    }

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, system);
  gnufdisk_exception_unregister_unwind_handler(&delete_string, param);

  gnufdisk_string_delete(param);
  free(system);

  GNUFDISK_LOG((DISKLABEL, "done perform create_partition, result: %p", ret));

  return ret;
}

static void ebr_private_remove_partition(void* _private, size_t _n)
{

}

static void ebr_private_set_parameter(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size)
{

}

static void ebr_private_get_parameter(void* _private, struct gnufdisk_string* _param, void* _data, size_t _size)
{

}

static void ebr_private_commit(void* _private)
{
  struct ebr_private* private;
  struct object* device;

  GNUFDISK_LOG((DISKLABEL, "perform commit on struct ebr_private* %p", _private));

  ebr_private_check(_private);

  private = _private;
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  if(private->chain)
    {
      struct list* iter;

      for(iter = list_first(private->chain); iter != NULL; iter = list_next(iter))
	{
	  struct ebr_chain* entry;

	  entry = list_data(iter);

	  ebr_chain_check(entry);

	  GNUFDISK_LOG((DISKLABEL, "write ebr entry at LBA %"PRId64, entry->start));

	  if(device_seek(device, entry->start, 0, SEEK_SET) == -1)
	    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

	  if(device_write(device, &entry->data, sizeof(struct ebr)) != sizeof(struct ebr))
	    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not write on device");

	  GNUFDISK_LOG((DISKLABEL, "partition: %p", entry->partition));

	  if(gnufdisk_check_memory(entry->partition, 1, 1) == 0)
	    partition_commit(entry->partition);
	}
    }

  GNUFDISK_LOG((DISKLABEL, "done perform commit"));
}

static void ebr_private_enumerate_partitions(void* _private, 
					     int (*_filter)(struct object*, void*),
					     void* _filter_data,
					     void (*_callback)(struct object*, void*),
					     void* _callback_data)
{

}

static void ebr_private_delete(void* _private)
{
  struct ebr_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform delete on struct ebr_private* %p", _private));

  ebr_private_check(_private);

  private = _private;

  object_delete(private->parent);
  
  if(private->chain)
    list_delete(private->chain, &delete_ebr_chain);

  memset(private, 0, sizeof(struct ebr_private));
  free(private);

  GNUFDISK_LOG((DISKLABEL, "done perform delete"));
}

static struct disklabel_implementation ebr_implementation = {
  private: NULL,
  raw: &ebr_private_raw,
  system: &ebr_private_system,
  partition: &ebr_private_partition,
  count_partitions: &ebr_private_count_partitions,
  create_partition: &ebr_private_create_partition,
  remove_partition: &ebr_private_remove_partition,
  set_parameter: &ebr_private_set_parameter,
  get_parameter: &ebr_private_get_parameter,
  commit: &ebr_private_commit,
  enumerate_partitions: &ebr_private_enumerate_partitions,
  delete: &ebr_private_delete
};

static struct ebr_chain* read_ebr(struct object* _parent, gnufdisk_integer _start, int _lba)
{
  struct object* device;
  struct ebr_chain tmp;
  struct ebr_chain* ret;

  GNUFDISK_LOG((DISKLABEL, "read EBR with struct object* %p as parent, offset: %"PRId64, _parent, _start));

  device = object_cast(_parent, OBJECT_TYPE_DEVICE);

  memset(&tmp, 0, sizeof(struct ebr_chain));

  tmp.start = _start;

  if(device_seek(device, _start, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");
  else if(device_read(device, &tmp.data, sizeof(struct ebr)) != sizeof(struct ebr))
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not read %u bytes", sizeof(struct ebr));

  if(tmp.data.magic[0] != 0x55 || tmp.data.magic[1] != 0xAA)
    return NULL;

  if((ret = malloc(sizeof(struct ebr_chain))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, ret);

  if(tmp.data.partitions[0].type != EMPTY)
    {
      gnufdisk_integer start;
      gnufdisk_integer end;

      GNUFDISK_LOG((DISKLABEL, "found an entry:"));
      
      GNUFDISK_LOG((DISKLABEL, 
		    "first_sector: c: %d, h: %d, s: %d",
		    CHS_CYLINDER(&tmp.data.partitions[0].first_sector),
		    CHS_HEAD(&tmp.data.partitions[0].first_sector),
		    CHS_SECTOR(&tmp.data.partitions[0].first_sector)));
      
      GNUFDISK_LOG((DISKLABEL, 
		    "last_sector: c: %d, h: %d, s: %d",
		    CHS_CYLINDER(&tmp.data.partitions[0].last_sector),
		    CHS_HEAD(&tmp.data.partitions[0].last_sector),
		    CHS_SECTOR(&tmp.data.partitions[0].last_sector)));

      GNUFDISK_LOG((DISKLABEL,
		    "first_lba: %"PRIu32, LE32_TO_CPU(tmp.data.partitions[0].first_lba)));

      GNUFDISK_LOG((DISKLABEL, 
		    "sectors: %"PRIu32, LE32_TO_CPU(tmp.data.partitions[0].sectors)));

      if(_lba)
	{
	  start = tmp.data.partitions[0].first_lba + _start;
	  end = start + tmp.data.partitions[0].sectors;
	}
      else
	{
	  start = chs_to_lba(&tmp.data.partitions[0].first_sector, device);
	  end = chs_to_lba(&tmp.data.partitions[0].last_sector, device);
	}

      tmp.partition = logical_new(_parent, start, end);

      GNUFDISK_LOG((DISKLABEL, "ebr partition: %p", tmp.partition));
    }

  memcpy(ret, &tmp, sizeof(struct ebr_chain));
  
  gnufdisk_exception_unregister_unwind_handler(&free, ret);

  GNUFDISK_LOG((DISKLABEL, "done read EBR, result: %p", ret));

  return ret; 
}

static struct list* read_ebr_chain(struct object* _parent, int _lba)
{
  struct list* ret;
  struct ebr_chain* entry;
  gnufdisk_integer start;
  gnufdisk_integer offset;

  GNUFDISK_LOG((DISKLABEL, "read EBR chain using struct object* %p as parent", _parent));
  GNUFDISK_LOG((DISKLABEL, "lba: %d", _lba));

  ret = NULL;
  entry = NULL;

  start = object_start(_parent);

  while(3)
    {
      GNUFDISK_LOG((DISKLABEL, "step for new EBR"));

      if(entry == NULL)
	{
	  GNUFDISK_LOG((DISKLABEL, "first step, read first entry"));

	  if((entry = read_ebr(_parent, start, _lba)) == NULL)
	    break;
	}
      else
	{
	  struct ebr* data;

	  ebr_chain_check(entry);

	  data = &entry->data;

	  if(data->partitions[1].type != EMPTY)
	    {
	      struct object* device;

	      GNUFDISK_LOG((DISKLABEL, "found a link:"));
	      
	      GNUFDISK_LOG((DISKLABEL, 
			    "first_sector: c: %d, h: %d, s: %d",
			    CHS_CYLINDER(&data->partitions[1].first_sector),
			    CHS_HEAD(&data->partitions[1].first_sector),
			    CHS_SECTOR(&data->partitions[1].first_sector)));
	      
	      GNUFDISK_LOG((DISKLABEL, 
			    "last_sector: c: %d, h: %d, s: %d",
			    CHS_CYLINDER(&data->partitions[1].last_sector),
			    CHS_HEAD(&data->partitions[1].last_sector),
			    CHS_SECTOR(&data->partitions[1].last_sector)));

	      GNUFDISK_LOG((DISKLABEL,
			    "first_lba: %"PRIu32, LE32_TO_CPU(data->partitions[1].first_lba)));

	      GNUFDISK_LOG((DISKLABEL, 
			    "sectors: %"PRIu32, LE32_TO_CPU(data->partitions[1].sectors)));

	      device = object_cast(_parent, OBJECT_TYPE_DEVICE);

	      if(_lba)
		offset = start + data->partitions[1].first_lba;
	      else 
		offset = start + chs_to_lba(&data->partitions[1].first_sector, device);
	    }
	  else
	    break;

	  GNUFDISK_LOG((DISKLABEL, "try read EBR at offset %"PRId64, start));

	  entry = read_ebr(_parent, offset, _lba);
	}

      ret = list_append(ret, entry);
    }

  return ret;
}

int ebr_probe(struct object* _parent, int _lba, struct disklabel_implementation* _implementation)
{
  struct ebr_private* private;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "probe EBR disklabel with struct object* %p", _parent));

  if((private = malloc(sizeof(struct ebr_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  if((private->chain = read_ebr_chain(_parent, _lba)) == NULL)
    {
      gnufdisk_exception_unregister_unwind_handler(&free, private);
      free(private);
      ret = -1;
    }
  else
    {
      gnufdisk_exception_register_unwind_handler(&delete_list, private->chain);

      object_ref(_parent);
      private->parent = _parent;

      private->lba = _lba;

      memcpy(_implementation, &ebr_implementation, sizeof(struct disklabel_implementation));
      _implementation->private = private;

      gnufdisk_exception_unregister_unwind_handler(&free, private);
      gnufdisk_exception_unregister_unwind_handler(&delete_list, private->chain);

      ret = 0;
    }

  GNUFDISK_LOG((DISKLABEL, "done probe EBR disklabel, result: %d", ret));

#if GNUFDISK_DEBUG
  do {
    struct list* iter;
    int count;

    for(count = 0, iter = private->chain; iter != NULL; iter = list_next(iter), count++)
      {
	struct ebr_chain* entry;

	entry = list_data(iter);

	ebr_chain_check(entry);

	if(entry->partition)
	  GNUFDISK_LOG((DISKLABEL, 
			"        > %d - struct ebr* %p, partition: %p (end: %"PRId64")", 
			count, &entry->data, entry->partition, object_end(entry->partition)));
      }

  } while(0);  
#endif

  return ret;
}

void ebr_new(struct object* _parent, int _lba, struct disklabel_implementation* _implementation)
{
  struct ebr_private* private;
  struct ebr_chain* entry;

  GNUFDISK_LOG((DISKLABEL, "create new MBR disklabel using struct object* %p as parent", _parent));

  if((private = malloc(sizeof(struct ebr_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  memset(private, 0, sizeof(struct ebr_private));

  object_ref(_parent);
  private->parent = _parent;
  private->lba = _lba;

  if((entry = malloc(sizeof(struct ebr_chain))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, entry);

  memset(entry, 0, sizeof(struct ebr_chain));

  entry->data.magic[0] = 0x55;
  entry->data.magic[1] = 0xAA;

  entry->start = object_start(_parent);
  
  private->chain = list_append(private->chain, entry);

  memcpy(_implementation, &ebr_implementation, sizeof(struct disklabel_implementation));
  _implementation->private = private;

  gnufdisk_exception_unregister_unwind_handler(&free, private);
  gnufdisk_exception_unregister_unwind_handler(&free, entry);

  GNUFDISK_LOG((DISKLABEL, "done create MBR disklabel"));
}

