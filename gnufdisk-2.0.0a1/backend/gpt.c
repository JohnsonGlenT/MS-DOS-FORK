#include <uuid/uuid.h>
#include "common.h"

/* from parted-3.0/libparted/ */

/*
 * Dec 5, 2000 Matt Domsch <Matt_Domsch@dell.com>
 * - Copied crc32.c from the linux/drivers/net/cipe directory.
 * - Now pass seed as an arg
 * - changed unsigned long to uint32_t, added #include<stdint.h>
 * - changed len to be an unsigned long
 * - changed crc32val to be a register
 * - License remains unchanged!  It's still GPL-compatable!
 */

  /* ============================================================= */
  /*  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or       */
  /*  code or tables extracted from it, as desired without restriction.     */
  /*                                                                        */
  /*  First, the polynomial itself and its table of feedback terms.  The    */
  /*  polynomial is                                                         */
  /*  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0   */
  /*                                                                        */
  /*  Note that we take it "backwards" and put the highest-order term in    */
  /*  the lowest-order bit.  The X^32 term is "implied"; the LSB is the     */
  /*  X^31 term, etc.  The X^0 term (usually shown as "+1") results in      */
  /*  the MSB being 1.                                                      */
  /*                                                                        */
  /*  Note that the usual hardware shift register implementation, which     */
  /*  is what we're using (we're merely optimizing it by doing eight-bit    */
  /*  chunks at a time) shifts bits into the lowest-order term.  In our     */
  /*  implementation, that means shifting towards the right.  Why do we     */
  /*  do it this way?  Because the calculated CRC must be transmitted in    */
  /*  order from highest-order term to lowest-order term.  UARTs transmit   */
  /*  characters in order from LSB to MSB.  By storing the CRC this way,    */
  /*  we hand it to the UART in the order low-byte to high-byte; the UART   */
  /*  sends each low-bit to hight-bit; and the result is transmission bit   */
  /*  by bit from highest- to lowest-order term without requiring any bit   */
  /*  shuffling on our part.  Reception works similarly.                    */
  /*                                                                        */
  /*  The feedback terms table consists of 256, 32-bit entries.  Notes:     */
  /*                                                                        */
  /*      The table can be generated at runtime if desired; code to do so   */
  /*      is shown later.  It might not be obvious, but the feedback        */
  /*      terms simply represent the results of eight shift/xor opera-      */
  /*      tions for all combinations of data and CRC register values.       */
  /*                                                                        */
  /*      The values must be right-shifted by eight bits by the "updcrc"    */
  /*      logic; the shift must be unsigned (bring in zeroes).  On some     */
  /*      hardware you could probably optimize the shift in assembler by    */
  /*      using byte-swap instructions.                                     */
  /*      polynomial $edb88320                                              */
  /*                                                                        */
  /*  --------------------------------------------------------------------  */

static const uint32_t crc32_tab[] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
   };

/* Return a 32-bit CRC of the contents of the buffer. */

static int32_t
efi_crc32(const void *buf, unsigned long len, uint32_t seed)
{
  unsigned long i;
  register uint32_t crc32val;
  const unsigned char *s = buf;

  crc32val = seed;

  for (i = 0;  i < len;  i ++)
    crc32val = crc32_tab[(crc32val ^ s[i]) & 0xff] ^ (crc32val >> 8);

  return crc32val;
}

/* end of import */

enum {
  OVERLAP_START, /* start sector overlap with the partition */
  OVERLAP_END, /* end sector overlap with the partition */
  OVERLAP_START_END, /* start before partition start, end after partition end */ 
  OVERLAP_NONE /* no overlaps */
};

#define GUID_UNUSED "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

struct gpt_header {
  unsigned char signature[8];
  uint32_t revision;
  uint32_t size;
  uint32_t header_crc32;
  uint32_t reserved;
  uint64_t lba_current;
  uint64_t lba_copy;
  uint64_t lba_first;
  uint64_t lba_last;
  unsigned char guid[16];
  uint64_t lba_first_entry;
  uint32_t npartitions;
  uint32_t partition_entry_size;
  uint32_t partition_crc32;
};

#define PARTITION_NAME_LEN 72

struct gpt_partition {
  unsigned char guid[16];
  unsigned char id[16];
  uint64_t first_lba; /* little endian */
  uint64_t last_lba; /* inclusive (little endian */
  uint64_t flags;
  unsigned char name[PARTITION_NAME_LEN];
};

static const uint64_t partition_flag_system = 0x0000000000000001LL;
static const uint64_t partition_flag_bootable = 0x0000000000000004LL;
static const uint64_t partition_flag_readonly = 0x1000000000000000LL;
static const uint64_t partition_flag_hidden = 0x4000000000000000LL;
static const uint64_t partition_flag_no_automount = 0x8000000000000000LL;

struct gpt_private {
  struct object* parent;
  struct gpt_header* header;
  void* partitions;
  struct object** children;
  struct gpt_header* backup_header;
};

static int partition_overlap(struct gpt_partition* _part, gnufdisk_integer _start, gnufdisk_integer _end)
{
  int ret;
  gnufdisk_integer pstart;
  gnufdisk_integer pend;
  
  GNUFDISK_LOG((DISKLABEL, "check wether %" PRId64 "-%" PRId64" overlaps with gpt_partition %p", _start, _end, _part));

  pstart = LE64_TO_CPU(_part->first_lba);
  pend = LE64_TO_CPU(_part->last_lba);

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

static void gpt_private_check(struct gpt_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct gpt_private), 0) != 0
     || gnufdisk_check_memory(_private->header, sizeof(struct gpt_header), 0) != 0
     || gnufdisk_check_memory(_private->partitions, 
                              LE32_TO_CPU(_private->header->partition_entry_size)
                                * LE32_TO_CPU(_private->header->npartitions), 0) != 0
     || gnufdisk_check_memory(_private->children,
                              LE32_TO_CPU(_private->header->npartitions) * sizeof(struct object*), 0) != 0
     || gnufdisk_check_memory(_private->backup_header, sizeof(struct gpt_header), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct gpt_private* %p", _private);
}

static void delete_gpt_private(void* _p)
{
  struct gpt_private* private;

  GNUFDISK_LOG((DISKLABEL, "delete struct gpt_private* %p", _p));

  gpt_private_check(_p);

  private = _p;

  object_delete(private->parent);

  free(private->header);
  free(private->partitions);
  free(private->children);
  free(private->backup_header);

  memset(private, 0, sizeof(struct gpt_private));

  free(private);
}

static void gpt_private_raw(void* _private, void** _dest, size_t* _size)
{
  struct gpt_private* private;
  struct object* device;
  gnufdisk_integer sector_size;
  size_t size;
  void* buf;

  GNUFDISK_LOG((DISKLABEL, "perform raw on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;
  
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  sector_size = device_sector_size(device);
  size = sector_size + LE32_TO_CPU(private->header->npartitions) * LE32_TO_CPU(private->header->partition_entry_size);

  GNUFDISK_LOG((DISKLABEL, "raw data size: %u", size));

  if((buf = malloc(size)) == NULL)
    THROW_ENOMEM;

  memcpy(buf, private->header, sizeof(struct gpt_header));
  memcpy(buf + sector_size, private->partitions, size - sector_size);

  *_dest = buf;
  *_size = sizeof(struct gpt_header);

  GNUFDISK_LOG((DISKLABEL, "done perform raw"));
}

static struct gnufdisk_string* gpt_private_system(void* _private)
{
  struct gpt_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((DISKLABEL, "perform system on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  if((ret = gnufdisk_string_new("GPT")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((DISKLABEL, "done perform system, result: %p", ret));

  return ret;
}

static struct object* gpt_private_partition(void* _private, size_t _number)
{
  struct gpt_private* private;
  GNUFDISK_RETRY rp0;
  int iter;
  int count;
  struct object* ret;

  GNUFDISK_LOG((DISKLABEL, "perform partition on struct gpt_private%p", _private));

  gpt_private_check(_private);

  private = _private;
 
  GNUFDISK_RETRY_SET(rp0);

  for(ret = NULL, iter = 0, count = 0; iter < LE32_TO_CPU(private->header->npartitions); iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0 && ++count == _number)
      {
        ret = private->children[iter];
        break;
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

static int gpt_private_count_partitions(void* _private)
{
  struct gpt_private* private;
  int iter;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform count_partitions on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  for(iter = 0; iter < LE32_TO_CPU(private->header->npartitions); iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0)
      ret++;

  GNUFDISK_LOG((DISKLABEL, "done perform count_partitions, result: %d", ret));

  return ret;
}

static struct object* gpt_private_create_partition(void* _private,
						   struct gnufdisk_geometry* _s, 
	      					   struct gnufdisk_geometry* _e, 
	      					   struct gnufdisk_string* _type)
{
  struct gpt_private* private;
  struct object* device;
  gnufdisk_integer optimal_alignment;
  GNUFDISK_RETRY rp0;
  gnufdisk_integer start;
  gnufdisk_integer end;
  int iter;
  int slot;
  GNUFDISK_RETRY rp1;
  char* type;
  struct object* ret;

  GNUFDISK_LOG((DISKLABEL, "perform create_partition on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);
  
  optimal_alignment = device_optimal_alignment(device) / device_sector_size(device);

  GNUFDISK_LOG((DISKLABEL, "optimal alignment: %"PRId64" sectors", optimal_alignment));

  GNUFDISK_RETRY_SET(rp0);

  /* aligns the start and check that it is valid */
  start = math_round(gnufdisk_geometry_start(_s) + gnufdisk_geometry_length(_s) / 2, optimal_alignment);

  GNUFDISK_LOG((DISKLABEL, "start sector: %"PRId64, start));

  if(start < LE64_TO_CPU(private->header->lba_first) 
     || start < gnufdisk_geometry_start(_s) 
     || start > gnufdisk_geometry_end(_s))
    {
      union gnufdisk_device_exception_data data;

      data.egeometry = _s;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0, 
		     GNUFDISK_DEVICE_EGEOMETRY,
		     &data,
		     "invalid start range");
    }

  /* aligns the end and check that it is valid */
  end = math_round(gnufdisk_geometry_start(_e) + gnufdisk_geometry_length(_e) / 2, optimal_alignment);

  GNUFDISK_LOG((DISKLABEL, "end sector: %"PRId64, end));

  if(end > LE64_TO_CPU(private->header->lba_last)
     || end < gnufdisk_geometry_start(_e) 
     || end > gnufdisk_geometry_end(_e))
    {
      union gnufdisk_device_exception_data data;

      data.egeometry = _e;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0, 
		     GNUFDISK_DEVICE_EGEOMETRY,
		     &data,
		     "invalid end range");
    }

  /* check that there is no overlap */

  for(iter = 0; iter < LE32_TO_CPU(private->header->npartitions); iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0)
      {
	int mode;
	union gnufdisk_device_exception_data data;

	mode = partition_overlap(private->partitions + LE32_TO_CPU(private->header->partition_entry_size) * iter, start, end);

	switch(mode)
	  {
	  case OVERLAP_START:
	  case OVERLAP_START_END:
	    {
	      data.egeometry = _s;

	      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
			     &rp0,
			     GNUFDISK_DEVICE_EGEOMETRY,
			     &data,
			     "start range overlap with another partition");
	      break;
	    }
	  case OVERLAP_END:
	    {
	      data.egeometry = _e;

	      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
			     &rp0,
			     GNUFDISK_DEVICE_EGEOMETRY,
			     &data,
			     "end range overlap with another partition");
	      break;
	    }
	}
    }

  GNUFDISK_LOG((DISKLABEL, "partition geometry ok"));

  /* find a free slot */

  for(slot = 0; slot < LE32_TO_CPU(private->header->npartitions); slot++)
    if(gnufdisk_check_memory(private->children[slot], 1, 1) != 0)
      break;

  if(slot >= LE32_TO_CPU(private->header->npartitions))
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EDISKLABELFULL, NULL, "partition table full");

  type = NULL;

  GNUFDISK_RETRY_SET(rp1);

  if(type != NULL)
    {
      /* we come from a throw handler */
      gnufdisk_exception_unregister_unwind_handler(&free, type);
      free(type);
      type = NULL;
    }

  if((type = gnufdisk_string_c_string_dup(_type)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, type);

  if(strcasecmp(type, "PRIMARY") == 0)
    {
      struct gpt_partition* part;

      ret = private->children[slot] = primary_new(private->parent, start, end);

      part = private->partitions + LE32_TO_CPU(private->header->partition_entry_size) * slot;

      memcpy(part->guid, "\xEB\xD0\xA0\xA2\xB9\xE5\x44\x33\x87\xC0\x68\xB6\xB7\x26\x99\xC", 16);
      uuid_generate(part->id);
      part->first_lba = CPU_TO_LE64(start);
      part->last_lba = CPU_TO_LE64(end);

      private->header->partition_crc32 = 
	private->backup_header->partition_crc32 = 
	  CPU_TO_LE32(efi_crc32(private->partitions, 
				LE32_TO_CPU(private->header->partition_entry_size)
				  * LE32_TO_CPU(private->header->npartitions), ~0L) ^ ~0L);

      private->header->header_crc32 = 0;
      private->header->header_crc32 = CPU_TO_LE32(efi_crc32(private->header, 
							    LE32_TO_CPU(private->header->size),
							    ~0L) ^ ~0L);

      private->backup_header->header_crc32 = 0;
      private->backup_header->header_crc32 = CPU_TO_LE32(efi_crc32(private->backup_header, 
							    LE32_TO_CPU(private->backup_header->size),
							    ~0L) ^ ~0L);
    }
  else
    {
      union gnufdisk_device_exception_data data;

      data.epartitiontype = _type;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp1,
		     GNUFDISK_DEVICE_EPARTITIONTYPE,
		     &data,
		     "invalid partition type: %s", type);
    }
  
  gnufdisk_exception_unregister_unwind_handler(&free, type);

  free(type);

  GNUFDISK_LOG((DISKLABEL, "done perform create_partition, result: %p", ret));

  return ret;
}

static void gpt_private_remove_partition(void* _private, size_t _n)
{
  struct gpt_private* private;
  struct gpt_partition* part;
  size_t partition_array_size;
  int iter;
  int count;

  GNUFDISK_LOG((DISKLABEL, "perform remove_partition on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  /* check if partition exist */
  
  for(iter = 0, count = 0; iter < LE32_TO_CPU(private->header->npartitions); iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0 && ++count == _n)
      break;

  if(iter == LE32_TO_CPU(private->header->npartitions))
    {
      union gnufdisk_device_exception_data data;

      data.epartitionnumber = &_n;

      GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARTITIONNUMBER, NULL, "invalid partition number: %u", _n);
    }

  /* check flags */
  part = private->partitions + LE32_TO_CPU(private->header->partition_entry_size) * iter;

  if(LE64_TO_CPU(part->flags) & (partition_flag_system|partition_flag_readonly))
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARTITION, NULL, "partition is protected"); 

  /* reset entry */
  memset(part, 0, LE32_TO_CPU(private->header->partition_entry_size));

  /* unlink child */
  partition_set_parent(private->children[iter], NULL);
  private->children[iter] = NULL;

  /* update crc32 */
  partition_array_size = LE32_TO_CPU(private->header->partition_entry_size) * LE32_TO_CPU(private->header->npartitions);

  private->header->partition_crc32 = 
    private->backup_header->partition_crc32 = 
      CPU_TO_LE32(efi_crc32(private->partitions, partition_array_size, ~0L) ^ ~0L);

  private->header->header_crc32 = 0;
  private->header->header_crc32 = CPU_TO_LE32(efi_crc32(private->header, LE32_TO_CPU(private->header->size), ~0L) ^ ~0L);

  private->backup_header->header_crc32 = 0;
  private->backup_header->header_crc32 = CPU_TO_LE32(efi_crc32(private->backup_header, LE32_TO_CPU(private->backup_header->size), ~0L) ^ ~0L);
  
  GNUFDISK_LOG((DISKLABEL, "done perform remove_partition"));
}

#if 0
static void gpt_private_set_parameter(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct gpt_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform set_parameter on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  GNUFDISK_LOG((DISKLABEL, "done erform set_parameter"));
}

static void gpt_private_get_parameter(void* _private, struct gnufdisk_string* _param, void* _data, size_t _size)
{
  struct gpt_private* private;
  
  GNUFDISK_LOG((DISKLABEL, "perform get_parameter on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  GNUFDISK_LOG((DISKLABEL, "done perform get_parameter"));
}
#endif

static void gpt_private_commit(void* _private)
{
  struct gpt_private* private;
  struct object* device;
  gnufdisk_integer start;
  gnufdisk_integer end;
  gnufdisk_integer sector_size;
  gnufdisk_integer partition_array_size;

  GNUFDISK_LOG((DISKLABEL, "perform commit on struct gpt_private* %p", _private));

  gpt_private_check(_private);
  
  private = _private;
  
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  start = object_start(private->parent);
  end = object_end(private->parent);

  GNUFDISK_LOG((DISKLABEL, "parent start: %"PRId64, start));
  GNUFDISK_LOG((DISKLABEL, "parent end: %"PRId64, end));

  sector_size = device_sector_size(device);
  partition_array_size = 
    LE32_TO_CPU(private->header->partition_entry_size) *
    LE32_TO_CPU(private->header->npartitions);
  
  /* header */
  if(device_seek(device, start, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  if(device_write(device, private->header, sector_size) != sector_size)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not write GPT header");
  
  /*entries */
  if(device_seek(device, start + 1, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  if(device_write(device, private->partitions, partition_array_size) != partition_array_size)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not write partitions array");

  /* entries backup */
  if(device_seek(device, LE64_TO_CPU(private->header->lba_last) + 1, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  if(device_write(device, private->partitions, partition_array_size) != partition_array_size)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not write partitions array");

  /* header backup */
  if(device_seek(device, LE64_TO_CPU(private->header->lba_copy), 0, SEEK_SET) == -1)
   GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  if(device_write(device, private->backup_header, sector_size) != sector_size)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not write GPT header backup");

  GNUFDISK_LOG((DISKLABEL, "done perform commit"));  
}

static void gpt_private_enumerate_partitions(void* _private, 
					     int (*_filter)(struct object*, void*),
	      				     void* _filter_data,
	      				     void (*_callback)(struct object*, void*),
	      				     void* _callback_data)
{
  struct gpt_private* private;
  int iter;

  GNUFDISK_LOG((DISKLABEL, "perform enumerate_partitions on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  for(iter = 0;iter < LE32_TO_CPU(private->header->npartitions); iter++)
    if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0)
      {
        if(gnufdisk_check_memory(_filter, 1, 1) == 0 && (*_filter)(private->children[iter], _filter_data) == 0)
          continue;

        (*_callback)(private->children[iter], _callback_data);
      }

  GNUFDISK_LOG((DISKLABEL, "done perform enumerate_partitions"));
}

static int gpt_private_partition_number(void* _private, struct object* _partition)
{
  struct gpt_private* private;
  int count;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform partition_number on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  for(ret = -1, count = 0; count < LE32_TO_CPU(private->header->npartitions); count++)
    if(private->children[count] == _partition)
      {
	ret = count + 1;
	break;
      }

  GNUFDISK_LOG((DISKLABEL, "done perform partition_number, result: %d", ret));

  return ret;
}

static void gpt_private_delete(void* _private)
{
  struct gpt_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform delete on struct gpt_private* %p", _private));

  gpt_private_check(_private);

  private = _private;

  delete_gpt_private(_private);

  GNUFDISK_LOG((DISKLABEL, "done perform delete"));
}

static struct disklabel_implementation gpt_implementation = {
  private: NULL,
  raw: &gpt_private_raw,
  system: &gpt_private_system,
  partition: &gpt_private_partition,
  count_partitions: &gpt_private_count_partitions,
  create_partition: &gpt_private_create_partition,
  remove_partition: &gpt_private_remove_partition,
  set_parameter: NULL, 
  /* set_parameter: &gpt_private_set_parameter, */
  get_parameter: NULL,
  /*get_parameter: &gpt_private_get_parameter,*/
  commit: &gpt_private_commit,
  enumerate_partitions: &gpt_private_enumerate_partitions,
  partition_number: &gpt_private_partition_number,
  delete: &gpt_private_delete
};

int gpt_probe(struct object* _parent, struct disklabel_implementation* _implementation)
{
  struct object* device;
  gnufdisk_integer sector_size;
  gnufdisk_integer start;
  struct gpt_header* gpt;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "probe for GPT disklabel using struct object* %p", _parent));
  
  device = object_cast(_parent, OBJECT_TYPE_DEVICE);

  sector_size = device_sector_size(device);
  start = object_start(_parent);

  GNUFDISK_LOG((DISKLABEL, "start: %"PRId64, start));

  if((gpt = malloc(sector_size)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, gpt);

  if(device_seek(device, start, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "error seek to lba %" PRId64, start);
  else if(device_read(device, gpt, sector_size) != sector_size)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "error read first sector");

  GNUFDISK_LOG((DISKLABEL, 
		"GPT signature: %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		gpt->signature[0],
		gpt->signature[1],
		gpt->signature[2],
		gpt->signature[3],
		gpt->signature[4],
		gpt->signature[5],
		gpt->signature[6],
		gpt->signature[7]));

  if(memcmp(gpt->signature, "EFI PART", 8) == 0)
    {
      struct gpt_private* private;
      int iter;
      gnufdisk_integer partition_array_size;

      GNUFDISK_LOG((DISKLABEL, "GPT signature match"));

      if((private = malloc(sizeof(struct gpt_private))) == NULL)
	THROW_ENOMEM;

      memset(private, 0, sizeof(struct gpt_private));

      gnufdisk_exception_register_unwind_handler(&delete_gpt_private, private);

      private->header = gpt;

      gnufdisk_exception_unregister_unwind_handler(&free, gpt); /* freed by delete_gpt_private */
      
      object_ref(_parent);
      private->parent = _parent;
     
      memcpy(_implementation, &gpt_implementation, sizeof(struct disklabel_implementation));
      _implementation->private = private;

      GNUFDISK_LOG((DISKLABEL, "read entries"));

      partition_array_size = LE32_TO_CPU(gpt->partition_entry_size) * LE32_TO_CPU(gpt->npartitions);

      GNUFDISK_LOG((DISKLABEL, "array size: %"PRId64, partition_array_size));

      if((private->partitions = malloc(partition_array_size)) == NULL)
	THROW_ENOMEM;

      if(device_seek(device, LE64_TO_CPU(gpt->lba_first_entry), 0, SEEK_SET) == -1)
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

      if(device_read(device, private->partitions, partition_array_size) != partition_array_size)
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not read partition array");

      /* create children */
      if((private->children = malloc(sizeof(struct object*) * LE32_TO_CPU(gpt->npartitions))) == NULL)
	THROW_ENOMEM;

      memset(private->children, 0, sizeof(struct object*) * LE32_TO_CPU(gpt->npartitions));

      GNUFDISK_LOG((DISKLABEL, "read partition entries"));

      for(iter = 0; iter < LE64_TO_CPU(gpt->npartitions); iter++)
	{
	  struct gpt_partition* part;

	  part = private->partitions + LE32_TO_CPU(gpt->partition_entry_size) * iter;

	  GNUFDISK_LOG((DISKLABEL, "check partition entry %d, %p", iter, part));

	  if(memcmp(part->guid, GUID_UNUSED, sizeof(part->guid)) != 0)
	    private->children[iter] = primary_new(_parent, LE64_TO_CPU(part->first_lba), LE64_TO_CPU(part->last_lba));
	}	

      GNUFDISK_LOG((DISKLABEL, "done read entries"));

      /* backup header */

      if((private->backup_header = malloc(sector_size)) == NULL)
	THROW_ENOMEM;

      if(device_seek(device, LE64_TO_CPU(gpt->lba_copy), 0, SEEK_SET) == -1)
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

      if(device_read(device, private->backup_header, sector_size) != sector_size)
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not read header backup");

      GNUFDISK_LOG((DISKLABEL, "done read backup header"));

#if GNUFDISK_DEBUG
      for(iter = 0; iter < LE32_TO_CPU(gpt->npartitions); iter++)
	if(gnufdisk_check_memory(private->children[iter], 1, 1) == 0)
	  {
	    GNUFDISK_LOG((DISKLABEL, 
			  "  > %d start: %"PRId64", end: %"PRId64,
			  iter, 
			  object_start(private->children[iter]),	
      			  object_end(private->children[iter])));
	  }
#endif

      gnufdisk_exception_unregister_unwind_handler(&delete_gpt_private, private);

      ret = 0;
    }
  else
    {
      GNUFDISK_LOG((DISKLABEL, "GPT signature does not match"));
      gnufdisk_exception_unregister_unwind_handler(&free, gpt);
      free(gpt);
      ret = -1;
    }

  GNUFDISK_LOG((DISKLABEL, "done probe GPT disklabel"));

  return ret;
}

void gpt_new(struct object* _parent, struct disklabel_implementation* _implementation)
{
  struct object* device;
  struct gpt_header* header;
  struct gpt_private* private;
  gnufdisk_integer sector_size;

  GNUFDISK_LOG((DISKLABEL, "create new GPT disklabel using struct object* %p as parent", _parent));

  device = object_cast(_parent, OBJECT_TYPE_DEVICE);

  sector_size = device_sector_size(device);

  GNUFDISK_LOG((DISKLABEL, "sector size: %" PRId64, sector_size));

  if((header = malloc(sector_size)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, header);

  memset(header, 0, sector_size);
  memcpy(header->signature, "EFI PART", 8);

  if((private = malloc(sizeof(struct gpt_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  private->header = header;

  object_ref(_parent);
  private->parent = _parent;

  memcpy(_implementation, &gpt_implementation, sizeof(struct disklabel_implementation));
  _implementation->private = private;

  gnufdisk_exception_unregister_unwind_handler(&free, header);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  GNUFDISK_LOG((DISKLABEL, "done create new GPT disklabel"));
}

