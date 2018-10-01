#include "common.h"

struct guid_private {
  struct object* parent;
  struct object* disklabel;
  gnufdisk_integer start;
  gnufdisk_integer end;
};

static void delete_object(void* _p)
{
  GNUFDISK_LOG((PARTITION, "delete struct object* %p", _p));
  object_delete(_p);
}

static void guid_private_check(struct guid_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct guid_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct guid_private* %p", _private);
}

static struct gnufdisk_string* guid_private_type(void* _private)
{
  struct guid_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((PARTITION, "perform type on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;

  if((ret = gnufdisk_string_new("GUID")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((PARTITION, "done perform type, result: %p", ret));

  return ret;
}

static gnufdisk_integer guid_private_start(void* _private)
{
  struct guid_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform start on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;
  ret = private->start;

  GNUFDISK_LOG((PARTITION, "done perform start, result: %"PRId64, ret));

  return ret;
}

static gnufdisk_integer guid_private_end(void* _private)
{
  struct guid_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform end on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;
  ret = private->end;

  GNUFDISK_LOG((PARTITION, "done perform end, result: %"PRId64, ret));

  return ret;
}

static int guid_private_have_disklabel(void* _private)
{
  struct guid_private* private;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform have_disklabel on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;

  ret = 1;

  GNUFDISK_LOG((PARTITION, "done perform have_disklabel, result: %d", ret));

  return ret;
}

static struct object* guid_private_disklabel(void* _private)
{
  struct guid_private* private;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "perform disklabel on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;
  ret = private->disklabel;

  GNUFDISK_LOG((PARTITION, "done perform disklabel, result: %p", ret));

  return ret;
}

static void guid_private_commit(void* _private)
{
  struct guid_private* private;

  GNUFDISK_LOG((PARTITION, "perform commit on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;

  disklabel_commit(private->disklabel);

  GNUFDISK_LOG((PARTITION, "done perform commit"));
}

static void guid_private_delete(void* _private)
{
  struct guid_private* private;

  GNUFDISK_LOG((PARTITION, "perform delete on struct guid_private* %p", _private));

  guid_private_check(_private);

  private = _private;

  object_delete(private->parent);
  memset(private, 0, sizeof(struct guid_private));

  free(private);

  GNUFDISK_LOG((PARTITION, "done perform delete"));
}

static struct partition_implementation guid_implementation = {
  private: NULL,
  set_parameter: NULL,
  get_parameter: NULL,
  type: guid_private_type,
  start: &guid_private_start,
  end: &guid_private_end,
  have_disklabel: &guid_private_have_disklabel,
  disklabel: &guid_private_disklabel,
  move: NULL,
  resize: NULL,
  read: NULL,
  write: NULL,
  commit: &guid_private_commit,
  delete: &guid_private_delete
};

struct object* guid_probe(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end)
{
  struct guid_private* private;
  struct partition_implementation partition_implementation;
  struct disklabel_implementation disklabel_implementation;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "probe GUID partition with struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %"PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %"PRId64, _end));

  if((private = malloc(sizeof(struct guid_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  memcpy(&partition_implementation, &guid_implementation, sizeof(struct partition_implementation));
  partition_implementation.private = private;

  ret = partition_new(_parent, _start, _end, &partition_implementation);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;
  private->start = _start;
  private->end = _end;

  if(gpt_probe(ret, &disklabel_implementation) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "error probe gpt disklabel on sector %"PRId64, _start);

  private->disklabel = disklabel_new_with_implementation(ret, &disklabel_implementation);

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done probe GUID partition, result"));

  return ret;
}    

struct object* guid_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end)
{
  struct guid_private* private;
  struct partition_implementation implementation;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "create new GUID partition with struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %" PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %" PRId64, _end));

  if((private = malloc(sizeof(struct guid_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);
  
  memcpy(&implementation, &guid_implementation, sizeof(struct partition_implementation));
  implementation.private = private;

  ret = partition_new(_parent, _start, _end, &implementation);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done create GUID partition, result: %p", ret));

  return ret;
}

