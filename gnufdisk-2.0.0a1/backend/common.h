#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <gnufdisk-common.h>
#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-device-internals.h>

/* log tags */
#define INFO 1
#define ENDIAN 1
#define MATH 1
#define LIST 1
#define OBJECT 1
#define DEVICE 1
#define DISKLABEL 1
#define PARTITION 1

#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t

UINT16 swap16(UINT16 _val);
UINT32 swap32(UINT32 _val);
UINT64 swap64(UINT64 _val);

#if HOST_BIG_ENDIAN
# define CPU_TO_LE16(_v) swap16(_v)
# define CPU_TO_LE32(_v) swap32(_v)
# define CPU_TO_LE64(_v) swap64(_v)
# define LE16_TO_CPU(_v) swap16(_v)
# define LE32_TO_CPU(_v) swap32(_v)
# define LE64_TO_CPU(_v) swap64(_v)
#else 
# define CPU_TO_LE16(_v) (_v)
# define CPU_TO_LE32(_v) (_v)
# define CPU_TO_LE64(_v) (_v)
# define LE16_TO_CPU(_v) (_v)
# define LE32_TO_CPU(_v) (_v)
# define LE64_TO_CPU(_v) (_v)
#endif

struct module_options {
  int readonly;
  gnufdisk_integer cylinders;
  gnufdisk_integer heads;
  gnufdisk_integer sectors;
  gnufdisk_integer sector_size;
};

#define DIV_T lldiv_t
#define DIV lldiv

DIV_T math_div(gnufdisk_integer _a, gnufdisk_integer _b);
gnufdisk_integer math_round_up(gnufdisk_integer _val, gnufdisk_integer _grain);
gnufdisk_integer math_round_down(gnufdisk_integer _val, gnufdisk_integer _grain);
gnufdisk_integer math_round(gnufdisk_integer _val, gnufdisk_integer _grain);

/* common errors */
#define THROW_ENOMEM GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOMEM, NULL, "can not allocate memory")

struct list;
struct list* list_prev(struct list* _n);
struct list* list_next(struct list* _n);
void* list_data(struct list* _n);
struct list* list_first(struct list* _n);
struct list* list_last(struct list* _n);
struct list* list_append(struct list* _n, void* _data);
struct list* list_append_ordered(struct list* _n, int (*_compare)(const void*, const void*), void* _data);
struct list* list_insert(struct list* _n, void* _data);
void* list_remove(struct list* _n);
void list_delete(struct list* _n, void (*_free)(void*));

struct object;

enum object_type {
  OBJECT_TYPE_DEVICE,
  OBJECT_TYPE_DISKLABEL,
  OBJECT_TYPE_PARTITION
};

struct object_private_operations {
  struct object* (*cast)(void* _private, enum object_type _type);
  gnufdisk_integer (*start)(void* _private);
  gnufdisk_integer (*end)(void* _private);
  void (*delete)(void* _private);
};

/* object interface */
struct object* object_new(enum object_type _type, const struct object_private_operations* _operations, void* _private);
struct object* object_ref(struct object* _o);
#ifdef GNUFDISK_DEBUG
int object_nref(struct object* _o);
#endif
enum object_type object_type(struct object* _o);
void object_delete(struct object* _o);
struct object* object_cast(struct object* _o, enum object_type _type);
gnufdisk_integer object_start(struct object* _o);
gnufdisk_integer object_end(struct object* _o);
void* object_private(struct object* _o, enum object_type _type);

/* device object functionalities */
gnufdisk_integer device_seek(void* _object, gnufdisk_integer _lba, gnufdisk_integer _offset, int _whence);
gnufdisk_integer device_read(void* _object, void* _buf, size_t _size);
gnufdisk_integer device_write(void* _object, const void* _buf, size_t _size);
gnufdisk_integer device_sector_size(void* _object);
gnufdisk_integer device_minimum_alignment(void* _object);
gnufdisk_integer device_optimal_alignment(void* _object);
void device_get_parameter(void* _object, struct gnufdisk_string* _param, void* _dest, size_t _size);

/* device's can be files, hard disk, usb drives... */
struct device_implementation {
  void* private;
  gnufdisk_integer (*start)(void* _private);
  gnufdisk_integer (*end)(void* _private);
  gnufdisk_integer (*seek)(void* _private, gnufdisk_integer _lba, gnufdisk_integer _offset, int whence);
  gnufdisk_integer (*read)(void* _private, void* _buf, size_t _size); 
  gnufdisk_integer (*write)(void* _private, const void* _buf, size_t _size);
  gnufdisk_integer (*sector_size)(void* _private);
  gnufdisk_integer (*minimum_alignment)(void* _private);
  gnufdisk_integer (*optimal_alignment)(void* _private);
  void (*set_parameter)(void *_private, struct gnufdisk_string* _param, const void* _data, size_t _size);
  void (*get_parameter)(void *_private, struct gnufdisk_string* _param, void* _dest, size_t _size);
  void (*commit)(void* _p);
  void (*delete)(void* _p); /* delete private data */
};

extern struct gnufdisk_disklabel_operations disklabel_operations;

/* disklabel functionalities */
struct object* disklabel_probe(struct object* _parent);
void disklabel_commit(struct object* _object);
void disklabel_enumerate_partitions(void* _object, 
                                    int (*_filter)(struct object*, void*),
                                    void* _filter_data,
                                    void (*_callback)(struct object*, void*),
                                    void* _callback_data);
int disklabel_partition_number(void* _object, struct object* _partition);



/* diklabel maybe MBR, EBR, GPT, BSD an so on */
struct disklabel_implementation {
  void* private;
  void (*raw)(void* _private, void** _dest, size_t* _size);
  struct gnufdisk_string* (*system)(void* _private);
  struct object* (*partition)(void* _private, size_t _number);
  int (*count_partitions)(void* _private);
  struct object* (*create_partition)(void* _private, 
                                     struct gnufdisk_geometry* _s, 
                                     struct gnufdisk_geometry* _e, 
                                     struct gnufdisk_string* _type);
  void (*remove_partition)(void* _private, size_t _n);
  void (*set_parameter)(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size);
  void (*get_parameter)(void* _private, struct gnufdisk_string* _param, void* _data, size_t _size);
  void (*commit)(void* _private);
  void (*enumerate_partitions)(void* _private, 
                               int (*_filter)(struct object*, void*),
                               void* _filter_data,
                               void (*_callback)(struct object*, void*),
                               void* _callback_data);
  int (*partition_number)(void* _private, struct object* _partition);
  void (*delete)(void* _private);
};

struct object* disklabel_new(struct object* _parent, struct gnufdisk_string* _system);
struct object* disklabel_new_with_implementation(struct object* _parent, struct disklabel_implementation* _implementation);
/* master boot record */
int mbr_probe(struct object* _parent, struct disklabel_implementation* _implementation);
void mbr_new(struct object* _parent, struct disklabel_implementation* _implementation);

/* extended boot record */
int ebr_probe(struct object* _parent, int _lba, struct disklabel_implementation* _implementation);
void ebr_new(struct object* _parent, int _lba, struct disklabel_implementation* _implementation);

/* GUID partition table */
struct object* guid_probe(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end);
int gpt_probe(struct object* _parent, struct disklabel_implementation* _implementation);
void gpt_new(struct object* _parent, struct disklabel_implementation* _implementation);

/* partition functionalities */
extern struct gnufdisk_partition_operations partition_operations;

/* partition maybe PRIMARY, EXTENDED, LOGICAL and so on */
struct partition_implementation {
  void* private;
  void (*set_parameter)(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size);
  void (*get_parameter)(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size);
  struct gnufdisk_string* (*type)(void* _private);
  gnufdisk_integer (*start)(void* _private);
  gnufdisk_integer (*end)(void* _private);
  int (*have_disklabel)(void* _private);
  struct object* (*disklabel)(void* _private);
  void (*move)(void* _private, struct gnufdisk_geometry* _start_range);
  void (*resize)(void* _private, struct gnufdisk_geometry* _end_range);
  int (*read)(void* _private, gnufdisk_integer _sector, void* _buf, size_t _size);
  int (*write)(void* _private, gnufdisk_integer _sector, const void* _buf, size_t _size);
  void (*commit)(void* _private);
  void (*set_parent)(void* _private, struct object* _parent);
  void (*delete)(void* _private);
};

struct object* partition_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end, struct partition_implementation* _implementation);
struct object* primary_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end);
struct object* extended_probe(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end, int _lba);
struct object* extended_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end, int _lba);
struct object* logical_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end);
struct object* guid_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end);
void partition_commit(void* _object);
void partition_set_parent(void* _object, struct object* _parent);

#endif /* COMMON_H_INCLUDED */

