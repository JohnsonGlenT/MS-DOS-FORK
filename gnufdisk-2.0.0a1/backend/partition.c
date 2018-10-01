#include "common.h"

struct partition_private {
  struct partition_implementation implementation;
  struct object *parent;
};

static void delete_object(void* _p)
{
  GNUFDISK_LOG((PARTITION, "delete struct object* %p", _p));
  object_delete(_p);
}

static void partition_private_check(struct partition_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct partition_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct partition_private* %p", _private);
}

/* object operations */
static struct object* partition_private_cast(void* _private, enum object_type _type)
{
  struct partition_private* private;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "perform cast on struct partition_private* %p", _private));

  partition_private_check(_private);

  private = _private;

  ret = object_cast(private->parent, _type);

  GNUFDISK_LOG((PARTITION, "done perform cast, result: %p", ret));

  return ret;
}

static gnufdisk_integer partition_private_start(void* _private)
{
  struct partition_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform start on struct partition_private* %p", _private));

  partition_private_check(_private);

  private = _private;

  if(gnufdisk_check_memory(private->implementation.start, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `start'");
  
  ret = (*private->implementation.start)(private->implementation.private);

  GNUFDISK_LOG((PARTITION, "done perform start, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer partition_private_end(void* _private)
{
  struct partition_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform end on struct partition_private* %p", _private));

  partition_private_check(_private);

  private = _private;
  
  if(gnufdisk_check_memory(private->implementation.end, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `end'");
  
  ret = (*private->implementation.end)(private->implementation.private);

  GNUFDISK_LOG((PARTITION, "done perform end, result: %" PRId64, ret));

  return ret;
}

static void partition_private_delete(void* _private)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform delete on struct partition_private* %p", _private));

  partition_private_check(_private);

  private = _private;

  if(gnufdisk_check_memory(private->implementation.delete, 1, 1) == 0)
    (*private->implementation.delete)(private->implementation.private);

  object_delete(private->parent);
  
  memset(private, 0, sizeof(struct partition_private));
  free(private);

  GNUFDISK_LOG((PARTITION, "done perform delete"));
}


static struct object_private_operations partition_private_operations = {
  cast: &partition_private_cast,
  start: &partition_private_start,
  end: &partition_private_end,
  delete: &partition_private_delete
};

/*implementation operations */
static void partition_set_parameter(void* _object, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform set_parameter on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);

  if(gnufdisk_check_memory(private->implementation.set_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `set_parameter'");

  (*private->implementation.set_parameter)(private->implementation.private, _param, _data, _size);

  GNUFDISK_LOG((PARTITION, "done perform set_parameter"));
}

static void partition_get_parameter(void* _object, struct gnufdisk_string* _param, void* _data, size_t _size)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform get_parameter on struct object* %p", _object));
  
  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  if(gnufdisk_check_memory(private->implementation.get_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `get_parameter'");

  (*private->implementation.get_parameter)(private->implementation.private, _param, _data, _size);
  
  GNUFDISK_LOG((PARTITION, "done perform get_parameter"));
}

static struct gnufdisk_string* partition_type(void* _object)
{
  struct partition_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((PARTITION, "perform type on struct object* %p", _object));
  
  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  if(gnufdisk_check_memory(private->implementation.type, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `type'");

  ret = (*private->implementation.type)(private->implementation.private);

  GNUFDISK_LOG((PARTITION, "done perform type, result: %p", ret));

  return ret;
}

static gnufdisk_integer partition_start(void* _object)
{
  struct partition_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform start on struct object* %p", _object));
  
  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  ret = object_start(_object);

  GNUFDISK_LOG((PARTITION, "done perform start, result: %"PRId64, ret));

  return ret;
}

static gnufdisk_integer partition_length(void* _object)
{
  struct partition_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((PARTITION, "perform length on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  ret = object_end(_object) - object_start(_object) + 1;

  GNUFDISK_LOG((PARTITION, "done perform length, result: "PRId64, ret));

  return ret;
}

static int partition_number(void* _object)
{
  struct partition_private* private;
  struct object* disklabel;
  int ret;
  
  GNUFDISK_LOG((PARTITION, "perform number on struct object* %p", _object));
  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);

  disklabel = object_cast(private->parent, OBJECT_TYPE_DISKLABEL);

  ret = disklabel_partition_number(disklabel, _object);

  GNUFDISK_LOG((PARTITION, "done perform number, result: %d", ret));

  return ret;
}

static int partition_have_disklabel(void* _object)
{
  struct partition_private* private;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform have_disklabel on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);

  if(gnufdisk_check_memory(private->implementation.have_disklabel, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `have_disklabel'");

  ret = (*private->implementation.have_disklabel)(private->implementation.private);

  GNUFDISK_LOG((PARTITION, "done perform have_disklabel, result: %d", ret));
  
  return ret;
}

static void partition_disklabel(void* _object, struct gnufdisk_disklabel_operations* _operations, void** _specific)
{
  struct partition_private* private;
  struct object* disklabel;

  GNUFDISK_LOG((PARTITION, "perform disklabel on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  if(gnufdisk_check_memory(private->implementation.disklabel, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `disklabel'");

  disklabel = (*private->implementation.disklabel)(private->implementation.private);

  object_ref(disklabel);

  memcpy(_operations, &disklabel_operations, sizeof(struct gnufdisk_disklabel_operations));
  *_specific = disklabel;

  GNUFDISK_LOG((PARTITION, "done perform disklabel"));
}

static void partition_move(void* _object, struct gnufdisk_geometry* _start_range)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform move on struct object* %p", _object));
  
  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  if(gnufdisk_check_memory(private->implementation.move, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `move'");

  (*private->implementation.move)(private->implementation.private, _start_range);

  GNUFDISK_LOG((PARTITION, "done perform move"));
}

static void partition_resize(void* _object, struct gnufdisk_geometry* _end_range)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform resize on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);

  if(gnufdisk_check_memory(private->implementation.resize, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `resize'");

  (*private->implementation.resize)(private->implementation.private, _end_range);

  GNUFDISK_LOG((PARTITION, "done perform resize"));
}

static int partition_read(void* _object, gnufdisk_integer _sector, void* _buf, size_t _size)
{
  struct partition_private* private;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform read on struct object* %p", _object));
  
  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);

  if(gnufdisk_check_memory(private->implementation.read, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `read'");

  ret = (*private->implementation.read)(private->implementation.private, _sector, _buf, _size);

  GNUFDISK_LOG((PARTITION, "done perform read, result: %d", ret));

  return ret;
}

static int partition_write(void* _object, gnufdisk_integer _sector, const void* _buf, size_t _size)
{
  struct partition_private* private;
  int ret;

  GNUFDISK_LOG((PARTITION, "perform write on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  if(gnufdisk_check_memory(private->implementation.write, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `write'");

  ret = (*private->implementation.write)(private->implementation.private, _sector, _buf, _size);

  GNUFDISK_LOG((PARTITION, "done perform write, result: %d", ret));

  return ret;
}

static void partition_delete(void* _object)
{
  GNUFDISK_LOG((PARTITION, "perform delete on struct object* %p", _object));

  object_delete(_object);

  GNUFDISK_LOG((PARTITION, "done perform delete"));
}

struct gnufdisk_partition_operations partition_operations = {
  set_parameter: &partition_set_parameter,
  get_parameter: &partition_get_parameter,
  type: &partition_type,
  start: &partition_start,
  length: &partition_length,
  number: &partition_number,
  have_disklabel: &partition_have_disklabel,
  disklabel: &partition_disklabel,
  move: &partition_move,
  resize: &partition_resize,
  read: &partition_read,
  write: &partition_write,
  delete: &partition_delete
};

struct object* partition_new(struct object* _parent, 
			     gnufdisk_integer _start,
			     gnufdisk_integer _end,
			     struct partition_implementation* _implementation)
{
  struct partition_private* private;
  struct object* ret;

  GNUFDISK_LOG((PARTITION, "create new partition using struct object* %p as parent", _parent));
  GNUFDISK_LOG((PARTITION, "start: %" PRId64, _start));
  GNUFDISK_LOG((PARTITION, "end: %" PRId64, _end));

  if((private = malloc(sizeof(struct partition_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  ret = object_new(OBJECT_TYPE_PARTITION, &partition_private_operations, private);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);

  gnufdisk_exception_unregister_unwind_handler(&free, private);

  object_ref(_parent);
  private->parent = _parent;

  memcpy(&private->implementation, _implementation, sizeof(struct partition_implementation));

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((PARTITION, "done create partitin, result: %p", ret));

  return ret;
}

void partition_commit(void* _object)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform commit on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);
  
  if(gnufdisk_check_memory(private->implementation.commit, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `commit'");

  (*private->implementation.commit)(private->implementation.private);

  GNUFDISK_LOG((PARTITION, "done perform commit"));
}

void partition_set_parent(void* _object, struct object* _parent)
{
  struct partition_private* private;

  GNUFDISK_LOG((PARTITION, "perform set_parent on struct object* %p", _object));
  GNUFDISK_LOG((PARTITION, "_parent: %p", _parent));

  private = object_private(_object, OBJECT_TYPE_PARTITION);

  partition_private_check(private);

  if(gnufdisk_check_memory(private->implementation.set_parent, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "partition implementation does not support `set_parent'");

  (*private->implementation.set_parent)(private->implementation.private, _parent);

  GNUFDISK_LOG((PARTITION, "done perform set_parent"));
}

