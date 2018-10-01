#include "common.h"

#define MAX_PARTITIONS 4

#define EMPTY 0x00
#define DOS_EXTENDED 0x05
#define WINDOWS95_EXTENDED 0x0f
#define LINUX_PRIMARY 0x83
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

  ret = (CHS_CYLINDER(_chs) * heads + CHS_HEAD(_chs)) * sectors + CHS_SECTOR(_chs) - 1;

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

struct mbr {
  const unsigned char unused[446];
  struct mbr_partition {
    unsigned char status;
    struct chs first_sector; /* first absolute sector */
    unsigned char type;
    struct chs last_sector; /*last absolute sector */
    uint32_t first_lba; /* first absolute sector */
    uint32_t sectors; /* number of sectors */
  } partitions[MAX_PARTITIONS];
  unsigned char magic[2]; 
} __attribute__((packed));

struct mbr_private {
  struct object* parent;
  struct mbr data;
  struct object* children[MAX_PARTITIONS];
};

static int partition_overlap(struct mbr_partition* _part, struct object* _device, gnufdisk_integer _start, gnufdisk_integer _end)
{
  int ret;
  gnufdisk_integer pstart;
  gnufdisk_integer pend;
  
  GNUFDISK_LOG((DISKLABEL, "check wether %" PRId64 "-%" PRId64" overlaps with mbr_partition %p", _start, _end, _part));

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

static void mbr_private_check(struct mbr_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct mbr_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct mbr_private* %p", _private);
}

static void mbr_private_raw(void* _private, void** _dest, size_t* _size)
{
  struct mbr_private* private;
  void* buf;

  GNUFDISK_LOG((DISKLABEL, "perform raw on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  if((buf = malloc(sizeof(struct mbr))) == NULL)
    THROW_ENOMEM;

  memcpy(buf, &private->data, sizeof(struct mbr));

  *_dest = buf;
  *_size = sizeof(struct mbr);

  GNUFDISK_LOG((DISKLABEL, "done perform raw"));
}

static struct gnufdisk_string* mbr_private_system(void* _private)
{
  struct mbr_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((DISKLABEL, "perform system on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  if((ret = gnufdisk_string_new("MBR")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((DISKLABEL, "done perform system, result: %p", ret));

  return ret;
}

struct object* mbr_private_partition(void* _private, size_t _number)
{
  struct mbr_private* private;
  GNUFDISK_RETRY rp0;

  GNUFDISK_LOG((DISKLABEL, "perform partition on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  GNUFDISK_RETRY_SET(rp0);

  if(_number == 0 
     || _number > MAX_PARTITIONS
     || gnufdisk_check_memory(private->children[_number - 1], 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.epartitionnumber = &_number;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0, 
		     GNUFDISK_DEVICE_EPARTITIONNUMBER,
		     &data,
		     "bad partition number: %u", _number);
    }

  GNUFDISK_LOG((DISKLABEL, "done perform partition, result: %p", private->children[_number - 1]));

  return private->children[_number - 1];
}

static int mbr_private_count_partitions(void* _private)
{
  struct mbr_private* private;
  int iter;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform count_partitions on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  for(iter = 0; iter < 4; iter++)
    if(private->data.partitions[iter].type != EMPTY)
      ret++; 

  GNUFDISK_LOG((PARTITION, "done perform count_partitions, result: %d", ret));

  return ret;
}

struct object* mbr_private_create_partition(void* _private, 
	                                    struct gnufdisk_geometry* _start_range,
				            struct gnufdisk_geometry* _end_range,
				            struct gnufdisk_string* _system)
{
  struct mbr_private* private;
  struct object* device;
  struct gnufdisk_string* param;
  char* system;
  gnufdisk_integer sectors;
  gnufdisk_integer sector_size;
  gnufdisk_integer start;
  gnufdisk_integer end;
  GNUFDISK_RETRY rp0;
  int iter;
  int slot;
  GNUFDISK_RETRY rp1;
  struct object* ret;

  param = NULL;
  system = NULL;

  GNUFDISK_LOG((DISKLABEL, "perform create_partition on struct mbr_private* %p", _private));

  mbr_private_check(_private);

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

  start = math_round(gnufdisk_geometry_start(_start_range) + gnufdisk_geometry_length(_start_range) / 2, sectors);
  end = math_round(gnufdisk_geometry_end(_end_range) - gnufdisk_geometry_length(_end_range) / 2, sectors);

  GNUFDISK_LOG((DISKLABEL, "aligned start: %" PRId64, start));
  GNUFDISK_LOG((DISKLABEL, "aligned end: %" PRId64, end));

  /* check that  the values remain within  the ranges indicated */
  if(start < gnufdisk_geometry_start(_start_range) || start >= gnufdisk_geometry_end(_start_range) || start < object_start(private->parent))
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

  /* look for an empty element in the table */
  for(slot = -1, iter = 0; iter < MAX_PARTITIONS; iter++)
    if(private->data.partitions[iter].type == EMPTY)
      {
	slot = iter;
	break;
      }

  if(slot == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EDISKLABELFULL, NULL, "partition table is full");

  /* check whether the partition will overwrite other partitions */
  for(iter = 0; iter < MAX_PARTITIONS; iter++)
    if(private->data.partitions[iter].type != EMPTY)
      {
	int mode;
	union gnufdisk_device_exception_data data;

	mode = partition_overlap(&private->data.partitions[iter], device, start, end);

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

  if(strcasecmp(system, "PRIMARY") == 0)
    {
      ret = primary_new(private->parent, start, end);
      private->data.partitions[slot].type = LINUX_PRIMARY;
    }
  else if(strcasecmp(system, "EXTENDED-LBA") == 0)
    {
      ret = extended_new(private->parent, start, end, 1); 
      private->data.partitions[slot].type = WINDOWS95_EXTENDED;
    }
  else if(strcasecmp(system, "EXTENDED") == 0)
    {
      ret = extended_new(private->parent, start, end, 0);
      private->data.partitions[slot].type = DOS_EXTENDED;
    }
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

  private->data.partitions[slot].first_sector = chs_from_lba(start, device);
  private->data.partitions[slot].last_sector = chs_from_lba(end, device);
  private->data.partitions[slot].first_lba = CPU_TO_LE32(start);
  private->data.partitions[slot].sectors = CPU_TO_LE32(end - start + 1);

  gnufdisk_exception_unregister_unwind_handler(&delete_string, param);
  gnufdisk_string_delete(param);

  gnufdisk_exception_unregister_unwind_handler(&free, system);
  free(system);

  GNUFDISK_LOG((DISKLABEL, "done perform create_partition, result: %p", ret));

  return ret;
}

static void mbr_private_remove_partition(void* _private, size_t _number)
{
  struct mbr_private* private;
  GNUFDISK_RETRY rp0;

  GNUFDISK_LOG((DISKLABEL, "perform remove_partition on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  GNUFDISK_RETRY_SET(rp0);

  if(_number == 0 
     || _number > MAX_PARTITIONS
     || gnufdisk_check_memory(private->children[_number - 1], 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.epartitionnumber = &_number;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0, 
		     GNUFDISK_DEVICE_EPARTITIONNUMBER,
		     &data,
		     "bad partition number: %u", _number);
    }

  private->children[_number - 1] = NULL;
  memset(&private->data.partitions[_number - 1], 0, sizeof(struct mbr_partition));

  GNUFDISK_LOG((DISKLABEL, "done perform remove_partition"));
}

static void mbr_private_set_parameter(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct mbr_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform set_parameter on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETER, NULL, "invalid parameter");
}

static void mbr_private_get_parameter(void* _private, struct gnufdisk_string* _param, void* _data, size_t _size)
{
  struct mbr_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform get_parameter on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETER, NULL, "invalid parameter");
}

static void mbr_private_commit(void* _private)
{
  struct mbr_private* private;
  gnufdisk_integer start;
  struct object* device;
  int iter;

  GNUFDISK_LOG((DISKLABEL, "perform commit on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  start = object_start(private->parent);

  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  if(device_seek(device, start, 0, SEEK_SET) != start)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "error seek device");

  if(device_write(device, &private->data, sizeof(struct mbr)) != sizeof(struct mbr))
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "error write %u bytes", sizeof(struct mbr));
  
  for(iter = 0; iter < MAX_PARTITIONS; iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0)
      partition_commit(private->children[iter]);
}


static void mbr_enumerate_partitions(void* _private,
				     int (* _filter)(struct object*, void*),
				     void* _filter_data,
				     void (*_callback)(struct object*, void*),
				     void* _callback_data)
{
  struct mbr_private* private;
  int iter;

  GNUFDISK_LOG((DISKLABEL, "perform enumerate_partitions on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  for(iter = 0; iter < MAX_PARTITIONS; iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0)
      {
	if(gnufdisk_check_memory(_filter, 1, 1) == 0
	   && (*_filter)(private->children[iter], _filter_data) == 0)
	  continue;

	if(gnufdisk_check_memory(_callback, 1, 1) == 0)
	  (*_callback)(private->children[iter], _callback_data);
      }

  GNUFDISK_LOG((DISKLABEL, "done perform enumerate_partitions"));
}

static int mbr_partition_number(void* _private, struct object* _object)
{
  struct mbr_private* private;
  int iter;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform partition_number on struct mbr_private* %p", _private));

  mbr_private_check(_private);
 
  private = _private;

  for(ret = -1, iter = 0; iter < MAX_PARTITIONS; iter++)
    if(private->children[iter] == _object)
      {
	ret = iter + 1;
	break;
      }

  GNUFDISK_LOG((DISKLABEL, "done perform partition_number, result: %d", ret));

  return ret;
}

static void mbr_private_delete(void* _private)
{
  struct mbr_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform delete on struct mbr_private* %p", _private));

  mbr_private_check(_private);

  private = _private;

  object_delete(private->parent);
  
  memset(private, 0, sizeof(struct mbr_private));
  free(private);

  GNUFDISK_LOG((DISKLABEL, "done perform delete"));
}

static struct disklabel_implementation mbr_implementation = {
  private: NULL,
  raw: &mbr_private_raw,
  system: &mbr_private_system,
  partition: &mbr_private_partition,
  count_partitions: &mbr_private_count_partitions,
  create_partition: &mbr_private_create_partition,
  remove_partition: &mbr_private_remove_partition,
  set_parameter: &mbr_private_set_parameter,
  get_parameter: &mbr_private_get_parameter,
  commit: &mbr_private_commit,
  enumerate_partitions: &mbr_enumerate_partitions,
  partition_number: &mbr_partition_number,
  delete: &mbr_private_delete
};

int mbr_probe(struct object* _parent, struct disklabel_implementation* _implementation)
{
  struct object* device;
  gnufdisk_integer start;
  struct mbr data;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "probe MBR disklabel with struct object* %p", _parent));

  device = object_cast(_parent, OBJECT_TYPE_DEVICE);

  start = object_start(_parent);

  GNUFDISK_LOG((DISKLABEL, "start sector: %" PRId64, start));

  if(device_seek(device, start, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device to sector %" PRId64, start);
  
  if(device_read(device, &data, sizeof(struct mbr)) != sizeof(struct mbr))
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not read disklabel (%u bytes)", sizeof(struct mbr));

  GNUFDISK_LOG((DISKLABEL, 
		"disklabel signature: 0x%02hhX%02hhX",
		data.magic[0], 
		data.magic[1]));

  if(data.magic[0] == 0x55 && data.magic[1] == 0xAA)
    {
      struct object* device;
      struct mbr_private* private;
      int iter;

      GNUFDISK_LOG((DISKLABEL, "MBR magic match"));

      device = object_cast(_parent, OBJECT_TYPE_DEVICE);

      if((private = malloc(sizeof(struct mbr_private))) == NULL)
	THROW_ENOMEM;

      gnufdisk_exception_register_unwind_handler(&free, private);
      
      memcpy(&private->data, &data, sizeof(struct mbr));
      
      object_ref(_parent);
      private->parent = _parent;
      memset(private->children, 0, sizeof(struct object*) * MAX_PARTITIONS);

      memcpy(_implementation, &mbr_implementation, sizeof(struct disklabel_implementation));
      _implementation->private = private;

      GNUFDISK_LOG((DISKLABEL, "probe partitions"));

      for(iter = 0; iter < MAX_PARTITIONS; iter++)
	if(private->data.partitions[iter].type != EMPTY)
	  {
	    struct object* part;

	    GNUFDISK_LOG((DISKLABEL, "found an entri on slot %d", iter));

	    switch(private->data.partitions[iter].type)
	      {
		case DOS_EXTENDED:
		  {
		    /* EXTENDED */
		    gnufdisk_integer start;
		    gnufdisk_integer end;

		    start = chs_to_lba(&private->data.partitions[iter].first_sector, device);
		    end = chs_to_lba(&private->data.partitions[iter].last_sector, device);

		    part = extended_probe(_parent, start, end, 0); /* chs */

		    break;
		  }
		case WINDOWS95_EXTENDED:
		case LINUX_EXTENDED:
		  {
		    /* EXTENDED LBA */
		    gnufdisk_integer start;
		    gnufdisk_integer end;

		    start = LE32_TO_CPU(private->data.partitions[iter].first_lba);
		    end = start + LE32_TO_CPU(private->data.partitions[iter].sectors);

		    part = extended_probe(_parent, start, end, 1); /* lba */

		    break;
		  }
		case EFI:
		  {
		    /* GUID Partition, use LBA values */
		    gnufdisk_integer start;
		    gnufdisk_integer end;

		    start = LE32_TO_CPU(private->data.partitions[iter].first_lba);
		    end = start + LE32_TO_CPU(private->data.partitions[iter].sectors);

		    part = guid_probe(_parent, start, end);
		    
		    break;
		  }
		default:
		  {
		    gnufdisk_integer start;
		    gnufdisk_integer end;

		    start = chs_to_lba(&private->data.partitions[iter].first_sector, device);
		    end = chs_to_lba(&private->data.partitions[iter].last_sector, device);

		    part = primary_new(_parent, start, end);

		    break;
		  }
	      }

	    GNUFDISK_LOG((DISKLABEL, "set partition %p on slot %d", part, iter));

	    private->children[iter] = part;
	  }

      GNUFDISK_LOG((DISKLABEL, "done probe partitions"));
#ifdef GNUFDISK_DEBUG

      GNUFDISK_LOG((DISKLABEL, "partition list: "));
      for(iter = 0; iter < MAX_PARTITIONS; iter++)
	GNUFDISK_LOG((DISKLABEL, 
		      "        > %u - type: 0x%hhx (child: %p)", 
		      iter, 
		      private->data.partitions[iter].type,
		      private->children[iter]));
#endif /* GNUFDISK_DEBUG */
      gnufdisk_exception_unregister_unwind_handler(&free, private);


      ret = 0;
    }
  else
    ret = -1;

  GNUFDISK_LOG((DISKLABEL, "done probe MBR disklabel, result: %d", ret));

  return ret;
}

void mbr_new(struct object* _parent, struct disklabel_implementation* _implementation)
{
  struct mbr_private* private;

  GNUFDISK_LOG((DISKLABEL, "create new MBR disklabel using struct object* %p as parent", _parent));

  if((private = malloc(sizeof(struct mbr_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  memset(private, 0, sizeof(struct mbr_private));

  object_ref(_parent);
  private->parent = _parent;

  private->data.magic[0] = 0x55;
  private->data.magic[1] = 0xAA;
  memset(private->children, 0, sizeof(struct object*) * MAX_PARTITIONS);

  memcpy(_implementation, &mbr_implementation, sizeof(struct disklabel_implementation));
  _implementation->private = private;

  gnufdisk_exception_unregister_unwind_handler(&free, private);

  GNUFDISK_LOG((DISKLABEL, "done create MBR disklabel"));
}
