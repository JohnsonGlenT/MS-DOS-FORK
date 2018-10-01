#include "common.h"

struct logical_private {
  struct object* parent;
  gnufdisk_integer start;
  gnufdisk_integer end;
};

static void delete_object(void* _p)
{
  GNUFDISK_LOG((PARTITION, "delete struct object* %p", _p));
  object_delete(_p);
}

static void logical_private_check(struct logical_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct logical_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct logical_private* %p", _private);
}

static struct gnufdisk_string* logical_private_type(void* _private)
{
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((PARTITION, "perform type on struct logical_private* %p", _private));

  if((ret = gnufdisk_string_new("LOGICAL")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((PARTITION, "done perform type, result: %p", ret));

  return ret;
}

static gnufdisk_integer logical_private_start(void* _private)
{
  struct logical_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform start on struct logical_private* %p", _private));

  logical_private_check(_private);

  private = _private;

  ret = private->start;

  GNUFDISK_LOG((PARTITION, "done perform start, result: %"PRId64, ret));

  return ret;
}

static gnufdisk_integer logical_private_end(void* _private)
{
  struct logical_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform end on struct logical_privateé %p", _private));

  logical_private_check(_private);

  private = _private;

  ret = private->end;

  GNUFDISK_LOG((PARTITION, "done perform end, result: %"PRId64, ret));

  return ret;
}

static int logical_private_have_disklabel(void* _private)
{
  int ret;

  GNUFDISK_LOG((PARTITION, "perform have_disklabel on struct logical_private* %p", _private));
  
  ret = 0;

  GNUFDISK_LOG((PARTITION, "done perform have_disklabel, result: %d", ret));

  return ret;
}

static int logical_private_read(void* _private, gnufdisk_integer _sector, void* _buf, size_t _size)
{
  struct logical_private* private;
  struct object* device;
  gnufdisk_integer real_sector;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform read on struct logical_private* %p", _private));

  logical_private_check(_private);

  private = _private;
 
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  real_sector = _sector + private->start;

  GNUFDISK_LOG((PARTITION, "real_sector: %"PRId64, real_sector));
  
  if(device_seek(device, real_sector, 0, SEEK_SET) != real_sector)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  ret = device_read(device, _buf, _size);

  GNUFDISK_LOG((PARTITION, "done perform read, result: %d", ret));

  return ret; 
}

static int logical_private_write(void* _private, gnufdisk_integer _sector, const void* _buf, size_t _size)
{
  struct logical_private* private;
  struct object* device;
  gnufdisk_integer real_sector;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform write on struct logical_private* %p", _private));

  logical_private_check(_private);

  private = _private;
 
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  real_sector = _sector + private->start;

  GNUFDISK_LOG((PARTITION, "real_sector: %"PRId64, real_sector));
  
  if(device_seek(device, real_sector, 0, SEEK_SET) != real_sector)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  ret = device_write(device, _buf, _size);

  GNUFDISK_LOG((PARTITION, "done perform write, result: %d", ret));

  return ret; 
}

static void logical_private_commit(void* _private)
{
  GNUFDISK_LOG((PARTITION, "perform commit on struct logical_private* %p", _private));

  GNUFDISK_LOG((PARTITION, "done_perform commit"));
}

static void logical_private_delete(void* _private)
{
  struct logical_private* private;

  GNUFDISK_LOG((PARTITION, "perform delete on struct logical_private* %p", _private));

  logical_private_check(_private);

  private = _private;

  object_delete(private->parent);

  memset(private, 0, sizeof(struct logical_private));

  free(private);

  GNUFDISK_LOG((PARTITION, "done perform delete"));
}

static struct partition_implementation logical_implementation = {
  set_parameter: NULL,
  get_parameter: NULL,
  type: &logical_private_type,
  start: &logical_private_start,
  end: &logical_private_end,
  have_disklabel: &logical_private_have_disklabel,
  disklabel: NULL,
  move: NULL,
  resize: NULL,
  read: &logical_private_read,
  write: &logical_private_write,
  commit: &logical_private_commit,
  delete: &logical_private_delete
};

struct object* logical_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end)
{
  struct logical_private* private;
  struct partition_implementation implementation;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "create new LOGICAL partition with struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %" PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %" PRId64, _end));

  if((private = malloc(sizeof(struct logical_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);
  
  memcpy(&implementation, &logical_implementation, sizeof(struct partition_implementation));
  implementation.private = private;

  ret = partition_new(_parent, _start, _end, &implementation);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;
  private->start = _start;
  private->end = _end;

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done create LOGICAL partition, result: %p", ret));

  return ret;
}

