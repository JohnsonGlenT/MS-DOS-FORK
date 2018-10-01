#include "common.h"

struct disklabel_private {
  struct disklabel_implementation implementation;
  struct object* parent;
};

static void delete_object(void* _p)
{
  GNUFDISK_LOG((DISKLABEL, "delete struct object* %p", _p));
  object_delete(_p);
}

static void disklabel_private_check(struct disklabel_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct disklabel_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct disklabel_private* %p", _private);
}

/* object operations */
static struct object* disklabel_private_cast(void* _private, enum object_type _type)
{
  struct disklabel_private* private;
  struct object* ret;

  GNUFDISK_LOG((DISKLABEL, "perform cast on struct disklabel_private* %p", _private));
  GNUFDISK_LOG((DISKLABEL, "type: %d", _type));

  disklabel_private_check(_private);

  private = _private;

  ret = object_cast(private->parent, _type);

  GNUFDISK_LOG((DISKLABEL, "done perform cast, result: %p", ret));

  return ret;
}

static gnufdisk_integer disklabel_private_start(void* _private)
{
  struct disklabel_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DISKLABEL, "perform start on struct disklabel_private* %p", _private));

  disklabel_private_check(_private);

  private = _private;

  ret = object_start(private->parent);

  GNUFDISK_LOG((DISKLABEL, "done perform start, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer disklabel_private_end(void* _private)
{
  struct disklabel_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DISKLABEL, "perform end on struct disklabel_private* %p", _private));

  disklabel_private_check(_private);

  private = _private;

  ret = object_end(private->parent);

  GNUFDISK_LOG((DISKLABEL, "done perform end, result: %" PRId64, ret));

  return ret;
}

static void disklabel_private_delete(void* _private)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform delete on struct disklabel_private* %p", _private));

  disklabel_private_check(_private);

  private = _private;

  object_delete(private->parent);

  memset(private, 0, sizeof(struct disklabel_private));

  free(private);

  GNUFDISK_LOG((DISKLABEL, "done perform delete"));
}

static struct object_private_operations disklabel_private_operations = { 
  cast: &disklabel_private_cast,
  start: &disklabel_private_start,
  end: &disklabel_private_end,
  delete: &disklabel_private_delete
};

/* public functions */ 
void disklabel_commit(struct object* _object)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform commit on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.commit, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `commit'");

  (*private->implementation.commit)(private->implementation.private);

  GNUFDISK_LOG((DISKLABEL, "done perform commit"));
}

void disklabel_enumerate_partitions(void* _object, 
                                    int (*_filter)(struct object*, void*),
                                    void* _filter_data,
                                    void (*_callback)(struct object*, void*),
                                    void* _callback_data)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform enumerate_partitions on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  if(gnufdisk_check_memory(private->implementation.enumerate_partitions, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `enumerate_partitions'");

  (*private->implementation.enumerate_partitions)(private->implementation.private,
						  _filter,
						  _filter_data,
						  _callback,
						  _callback_data);

  GNUFDISK_LOG((DISKLABEL, "done perform enumerate_partitions"));
}

int disklabel_partition_number(void* _object, struct object* _partition)
{
  struct disklabel_private* private;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform partition_number on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  if(gnufdisk_check_memory(private->implementation.partition_number, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `partition_number'");

  ret = (*private->implementation.partition_number)(private->implementation.private, _partition);

  GNUFDISK_LOG((DISKLABEL, "done perform partition_number, result: %d", ret));

  return ret;
}

struct object* disklabel_probe(struct object* _parent)
{
  struct disklabel_private* private;
  struct object* ret;
  int err;

  GNUFDISK_LOG((DISKLABEL, "probe disklabel using struct object* %p", _parent));

  if((private = malloc(sizeof(struct disklabel_private))) == NULL)
    THROW_ENOMEM;
  
  gnufdisk_exception_register_unwind_handler(&free, private);

  ret = object_new(OBJECT_TYPE_DISKLABEL, &disklabel_private_operations, private);
  gnufdisk_exception_register_unwind_handler(&delete_object, ret);

  gnufdisk_exception_unregister_unwind_handler(&free, private); /* avoid double free */

  object_ref(_parent);
  private->parent = _parent;

  err = 0;

  if(mbr_probe(ret, &private->implementation) != 0
     && gpt_probe(ret, &private->implementation) != 0)
    {
      err = 1;
    }
  
  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  if(err)
    {
      object_delete(ret);
      ret = NULL;
    }

  GNUFDISK_LOG((DISKLABEL, "done probe disklabel, result: %p", ret));

  return ret;
}

struct object* disklabel_new_with_implementation(struct object* _parent, struct disklabel_implementation* _implementation)
{
  struct disklabel_private* private;
  struct object* ret;

  GNUFDISK_LOG((DISKLABEL, "create new disklabel using struct object* %p as parent", _parent));

  if((private = malloc(sizeof(struct disklabel_private))) == NULL)
    THROW_ENOMEM;
  
  gnufdisk_exception_register_unwind_handler(&free, private);

  ret = object_new(OBJECT_TYPE_DISKLABEL, &disklabel_private_operations, private);
  gnufdisk_exception_register_unwind_handler(&delete_object, ret);

  gnufdisk_exception_unregister_unwind_handler(&free, private); /* avoid double free */

  object_ref(_parent);
  private->parent = _parent;

  memcpy(&private->implementation, _implementation, sizeof(struct disklabel_implementation));
  
  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);

  GNUFDISK_LOG((DISKLABEL, "done create disklabel, result: %p", ret));

  return ret;
}

struct object* disklabel_new(struct object* _parent, struct gnufdisk_string* _system)
{
  GNUFDISK_RETRY rp0;
  struct disklabel_private* private;
  struct object* ret;
  char* system;

  GNUFDISK_LOG((DISKLABEL, "create new disklabel using struct object* %p as parent", _parent));

  if((private = malloc(sizeof(struct disklabel_private))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, private);

  ret = object_new(OBJECT_TYPE_DISKLABEL, &disklabel_private_operations, private);

  gnufdisk_exception_register_unwind_handler(&delete_object, ret);
  gnufdisk_exception_unregister_unwind_handler(&free, private);

  system = NULL;

  GNUFDISK_RETRY_SET(rp0);

  if(system != NULL)
    {
      /* we come back from an exception handler, system is allocated */
      gnufdisk_exception_unregister_unwind_handler(&free, system);
      free(system);
      system = NULL;
    }

  if((system = gnufdisk_string_c_string_dup(_system)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, system);

  if(strcasecmp(system, "MBR") == 0)
    mbr_new(ret, &private->implementation);
  else if(strcasecmp(system, "GPT") == 0)
    gpt_new(ret, &private->implementation);
  else
    {
      union gnufdisk_device_exception_data data;

      data.edisklabelsystem = _system;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
		     &rp0, 
		     GNUFDISK_DEVICE_EDISKLABELSYSTEM, 
		     &data,
		     "unknown disklabel system: %s",
		     system);
    }

  gnufdisk_exception_unregister_unwind_handler(&free, system);
  free(system);

  gnufdisk_exception_unregister_unwind_handler(&delete_object, ret);
  
  GNUFDISK_LOG((DISKLABEL, "done create disklabel, result: %p", ret));

  return ret;
}

/* struct gnufdisk_disklabel_operations implementation: */

static void disklabel_raw(void* _object, void** _dest, size_t* _size)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform raw on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.raw, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `raw'");

  (*private->implementation.raw)(private->implementation.private, _dest, _size);

  GNUFDISK_LOG((DISKLABEL, "done perform raw"));
}

static struct gnufdisk_string* disklabel_system(void* _object)
{
  struct disklabel_private* private;
  struct gnufdisk_string* ret;

  GNUFDISK_LOG((DISKLABEL, "perform system on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.system, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `system'");

  ret = (*private->implementation.system)(private->implementation.private);

  GNUFDISK_LOG((DISKLABEL, "done perform system, result: %p", ret));

  return ret;
}

static void disklabel_partition(void* _object, size_t _number, struct gnufdisk_partition_operations* _operations, void** _specific)
{
  struct disklabel_private* private;
  struct object* object;

  GNUFDISK_LOG((DISKLABEL, "perform partition on struct object* %p", _object));
  GNUFDISK_LOG((DISKLABEL, "number: %u", _number));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);
  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.partition, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `partition'");

  object = (*private->implementation.partition)(private->implementation.private, _number);

  object_ref(object);

  memcpy(_operations, &partition_operations, sizeof(struct gnufdisk_partition_operations));
  *_specific = object;

  GNUFDISK_LOG((DISKLABEL, "done perform partition"));
}

static int disklabel_count_partitions(void* _object)
{
  struct disklabel_private* private;
  int ret;

  GNUFDISK_LOG((DISKLABEL, "perform count_partitions on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);
  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.count_partitions, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `count_partitions'");

  ret = (*private->implementation.count_partitions)(private->implementation.private);

  GNUFDISK_LOG((DISKLABEL, "done perform count_partitions, result: %d", ret));

  return ret;
}

static void disklabel_create_partition(void* _object, 
				       struct gnufdisk_geometry* _start_range,
				       struct gnufdisk_geometry* _end_range,
				       struct gnufdisk_string* _type,
				       struct gnufdisk_partition_operations* _operations,
				       void** _specific)
{
  struct disklabel_private* private;
  struct object* partition;

  GNUFDISK_LOG((DISKLABEL, "perform create_partition on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.create_partition, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `create_partition'");

  partition = (*private->implementation.create_partition)(private->implementation.private, 
							  _start_range,
							  _end_range,
							  _type);

  memcpy(_operations, &partition_operations, sizeof(struct gnufdisk_partition_operations));
  *_specific = partition;

  GNUFDISK_LOG((DISKLABEL, "done perform create_partition"));
}

static void disklabel_remove_partition(void* _object, size_t _number)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform remove_partition on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.remove_partition, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `remove_partition'");

  (*private->implementation.remove_partition)(private->implementation.private, _number);

  GNUFDISK_LOG((DISKLABEL, "done perform remove_partition"));
}

static void disklabel_set_parameter(void *_object, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform set_parameter on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.set_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `set_parameter'");

  (*private->implementation.set_parameter)(private->implementation.private, _param, _data, _size);

  GNUFDISK_LOG((DISKLABEL, "done perform set_parameter"));
}

static void disklabel_get_parameter(void* _object, struct gnufdisk_string* _param, void* _dest, size_t _size)
{
  struct disklabel_private* private;

  GNUFDISK_LOG((DISKLABEL, "perform get_parameter on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DISKLABEL);

  disklabel_private_check(private);

  if(gnufdisk_check_memory(private->implementation.get_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "disklabel implementation does not support `get_parameter'");

  (*private->implementation.get_parameter)(private->implementation.private, _param, _dest, _size);

  GNUFDISK_LOG((DISKLABEL, "done perform get_parameter"));
}

static void disklabel_delete(void* _object)
{
  GNUFDISK_LOG((DISKLABEL, "perform delete on struct object* %p", _object));

  object_delete(_object);

  GNUFDISK_LOG((DISKLABEL, "done perform delete"));
}

struct gnufdisk_disklabel_operations disklabel_operations = {
  raw: &disklabel_raw,
  system: &disklabel_system,
  partition: &disklabel_partition,
  count_partitions: &disklabel_count_partitions,
  create_partition: &disklabel_create_partition,
  remove_partition: &disklabel_remove_partition,
  set_parameter: &disklabel_set_parameter,
  get_parameter: &disklabel_get_parameter,
  delete: &disklabel_delete
};

