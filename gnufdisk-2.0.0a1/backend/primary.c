#include "common.h"

struct primary_private {
  struct object* parent;
  gnufdisk_integer start;
  gnufdisk_integer end;
};

static void delete_object(void* _p)
{
  GNUFDISK_LOG((PARTITION, "delete struct object* %p", _p));
  object_delete(_p);
}

static void primary_private_check(struct primary_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct primary_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct primary_private* %p", _private);
}

static void primary_private_set_parameter(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct primary_private* private;

  GNUFDISK_LOG((PARTITION, "perform set_parameter on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETER, NULL, "invalid parameter");

  GNUFDISK_LOG((PARTITION, "done perform set_parameter"));
}

static void primary_private_get_parameter(void* _private, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct primary_private* private;

  GNUFDISK_LOG((PARTITION, "perform get_parameter on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETER, NULL, "invalid parameter");

  GNUFDISK_LOG((PARTITION, "done perform get_parameter"));
}

static struct gnufdisk_string* primary_private_type(void* _private)
{
  struct primary_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((PARTITION, "perform type on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  if((ret = gnufdisk_string_new("PRIMARY")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((PARTITION, "done perform type, result: %p", ret));

  return ret;
}

static gnufdisk_integer primary_private_start(void* _private)
{
  struct primary_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform start on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  ret = private->start;

  GNUFDISK_LOG((PARTITION, "done perform start, result: %"PRId64, ret));

  return ret;
}

static gnufdisk_integer primary_private_end(void* _private)
{
  struct primary_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform end on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  ret = private->end;

  GNUFDISK_LOG((PARTITION, "done perform end, result: %"PRId64, ret));

  return ret;
}

static int primary_private_have_disklabel(void* _private)
{
  struct primary_private* private;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform have_disklabel on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  ret = 0;

  GNUFDISK_LOG((PARTITION, "done perform have_disklabel, result: %d", ret));

  return ret;
}

static int primary_private_read(void* _private, gnufdisk_integer _sector, void* _buf, size_t _size)
{
  struct primary_private* private;
  struct object* device;
  gnufdisk_integer real_sector;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform read on struct primary_private* %p", _private));
  GNUFDISK_LOG((PARTITION, "sector: %"PRId64, _sector));
  GNUFDISK_LOG((PARTITION, "size: %u", _size));

  primary_private_check(_private);

  private = _private;

  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);

  real_sector = private->start + _sector;

  GNUFDISK_LOG((PARTITION, "real sector: %"PRId64, real_sector));

  if((real_sector + _size / device_sector_size(device)) >= private->end)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "attempt to read out of partition space");

  if(device_seek(device, _sector + private->start, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");
  
  ret = device_read(device, _buf, _size);

  GNUFDISK_LOG((PARTITION, "done perform read, result: %d", ret));

  return ret;
}

static int primary_private_write(void* _private, gnufdisk_integer _sector, const void* _buf, size_t _size)
{
  struct primary_private* private;
  struct object* device;
  gnufdisk_integer real_sector;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform write on struct primary_private* %p", _private));
  GNUFDISK_LOG((PARTITION, "sector: %"PRId64, _sector));
  GNUFDISK_LOG((PARTITION, "size: %u", _size));

  primary_private_check(_private);

  private = _private;
  
  device = object_cast(private->parent, OBJECT_TYPE_DEVICE);
  
  real_sector = private->start + _sector;

  GNUFDISK_LOG((PARTITION, "real sector: %"PRId64, real_sector));

  if((real_sector + _size / device_sector_size(device)) >= private->end)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "attempt to write out of partition space");

  if(device_seek(device, _sector + private->start, 0, SEEK_SET) == -1)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "can not seek device");

  ret = device_write(device, _buf, _size);

  GNUFDISK_LOG((PARTITION, "done perform write, result: %d", ret));

  return ret;
}

static void primary_private_commit(void* _private)
{
  GNUFDISK_LOG((PARTITION, "perform commit on struct primary_private* %p", _private));

  GNUFDISK_LOG((PARTITION, "done perform commit"));
}

static void primary_private_delete(void* _private)
{
  struct primary_private* private;

  GNUFDISK_LOG((PARTITION, "perform delete on struct primary_private* %p", _private));

  primary_private_check(_private);

  private = _private;

  object_delete(private->parent);

  memset(private, 0, sizeof(struct primary_private));

  free(private);

  GNUFDISK_LOG((PARTITION, "done perform delete"));
}

static struct partition_implementation primary_implementation = {
  set_parameter: &primary_private_set_parameter,
  get_parameter: &primary_private_get_parameter,
  type: &primary_private_type,
  start: &primary_private_start,
  end: &primary_private_end,
  have_disklabel: &primary_private_have_disklabel,
  disklabel: NULL,
  move: NULL,
  resize: NULL,
  read: &primary_private_read,
  write: &primary_private_write,
  commit: &primary_private_commit,
  delete: &primary_private_delete
};

struct object* primary_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end)
{
  struct primary_private* private;
  struct partition_implementation implementation;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "create new PRIMARY partition with struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %" PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %" PRId64, _end));

  if((private = malloc(sizeof(struct primary_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);
  
  memcpy(&implementation, &primary_implementation, sizeof(struct partition_implementation));
  implementation.private = private;

  ret = partition_new(_parent, _start, _end, &implementation);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;
  private->start = _start;
  private->end = _end;

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done create PRIMARY partition, result: %p", ret));

  return ret;
}


