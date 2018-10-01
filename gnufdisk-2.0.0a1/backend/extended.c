#include "common.h"

struct extended_private {
  struct object* parent;
  struct object* disklabel;
  gnufdisk_integer start;
  gnufdisk_integer end;
  int lba;
};

static void delete_object(void* _p)
{
  GNUFDISK_LOG((PARTITION, "delete struct object* %p", _p));
  object_delete(_p);
}

static void extended_private_check(struct extended_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct extended_private), 0) != 0
     || gnufdisk_check_memory(_private->disklabel, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct extended_private* %p", _private);
}

static struct gnufdisk_string* extended_private_type(void* _private)
{
  struct extended_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((PARTITION, "perform type on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;

  if((ret = gnufdisk_string_new(private->lba ? "EXTENDED LBA" : "EXTENDED")) == NULL)
    THROW_ENOMEM;

  GNUFDISK_LOG((PARTITION, "done perform type, result: %p", ret));

  return ret;
}

static gnufdisk_integer extended_private_start(void* _private)
{
  struct extended_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform start on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;

  ret = private->start;

  GNUFDISK_LOG((PARTITION, "done perform start, result: %"PRId64, ret)); 

  return ret;
}

static gnufdisk_integer extended_private_end(void* _private)
{
  struct extended_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform end on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;

  ret = private->end;

  GNUFDISK_LOG((PARTITION, "done perform end, result: %"PRId64, ret));

  return ret;
}

static int extended_private_have_disklabel(void* _private)
{
  struct extended_private* private;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform have_disklabel on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;

  ret = 1;

  GNUFDISK_LOG((PARTITION, "done perform have_disklabel, result: %d", ret));

  return ret;
}

static struct object* extended_private_disklabel(void* _private)
{
  struct extended_private* private;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "perform disklabel on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;
  ret = private->disklabel;

  GNUFDISK_LOG((PARTITION, "done perform disklabel, result: %p", ret));

  return ret;
}

static void extended_private_commit(void* _private)
{
  struct extended_private* private;

  GNUFDISK_LOG((PARTITION, "perform commit on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;

  if(private->disklabel != NULL)
    disklabel_commit(private->disklabel);

  GNUFDISK_LOG((PARTITION, "done perform commit"));
}

static void extended_private_delete(void* _private)
{
  struct extended_private* private;

  GNUFDISK_LOG((PARTITION, "perform delete on struct extended_private* %p", _private));

  extended_private_check(_private);

  private = _private;

  object_delete(private->parent);
  object_delete(private->disklabel);

  memset(private, 0, sizeof(struct extended_private));

  free(private);

  GNUFDISK_LOG((PARTITION, "done perform delete"));
}

static struct partition_implementation extended_implementation = {
  set_parameter: NULL,
  get_parameter: NULL,
  type: &extended_private_type,
  start: &extended_private_start,
  end: &extended_private_end,
  have_disklabel: &extended_private_have_disklabel,
  disklabel: &extended_private_disklabel,
  move: NULL,
  resize: NULL,
  read: NULL,
  write: NULL,
  commit: &extended_private_commit,
  delete: &extended_private_delete
};

struct object* extended_probe(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end, int _lba)
{
  struct extended_private* private;
  struct partition_implementation implementation;
  struct object* ret;
  struct disklabel_implementation ebr_implementation; 
  struct object* disklabel;

  GNUFDISK_LOG((PARTITION, "probe EXTENDED partition with struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %" PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %" PRId64, _end));
  GNUFDISK_LOG((PARTITION, "lba: %d", _lba));

  if((private = malloc(sizeof(struct extended_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);
  
  memcpy(&implementation, &extended_implementation, sizeof(struct partition_implementation));
  implementation.private = private;

  ret = partition_new(_parent, _start, _end, &implementation);

  GNUFDISK_LOG((PARTITION, "DELETE OBJECT: *** %p ***", &delete_object));

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;
  private->start = _start;
  private->end = _end;
  private->lba = _lba;

  GNUFDISK_LOG((PARTITION, "probe disklabel"));

  if(ebr_probe(ret, _lba, &ebr_implementation) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EIO, NULL, "error probing Extended boot record on sector %" PRId64, object_start(ret));

  disklabel = disklabel_new_with_implementation(ret, &ebr_implementation);
  
  /* avoid cross references */
  private->disklabel = disklabel;

  GNUFDISK_LOG((PARTITION, "done probe disklabel"));
  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done probe EXTENDED partition, result: %p", ret));

  return ret;
}

struct object* extended_new(struct object* _parent, gnufdisk_integer _start, gnufdisk_integer _end, int _lba)
{
  struct extended_private* private;
  struct partition_implementation implementation;
  struct object* ret;
  struct disklabel_implementation ebr_implementation; 
  struct object* disklabel;

  GNUFDISK_LOG((PARTITION, "create new EXTENDED partition with struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %"PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %"PRId64, _end));

  if((private = malloc(sizeof(struct extended_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  memcpy(&implementation, &extended_implementation, sizeof(struct partition_implementation));
  implementation.private = private;

  ret = partition_new(_parent, _start, _end, &implementation);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;
  private->start = _start;
  private->end = _end;

  ebr_new(ret, _lba, &ebr_implementation);
  disklabel = disklabel_new_with_implementation(ret, &ebr_implementation);

  /* avoid cross references */
  private->disklabel = disklabel;

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done create EXTENDED partition, result: %p", ret));

  return ret;
}
