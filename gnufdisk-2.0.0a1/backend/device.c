#include "common.h"



extern int getsubopt( );
extern int linux_device_probe(const char* _path, struct module_options* _options, struct device_implementation* _implementation);

struct device_private {
  struct device_implementation implementation;
  struct module_options options;
  struct object* disklabel;
  int is_open;
};

enum {
  OPTION_READONLY,
  OPTION_CYLINDERS,
  OPTION_HEADS,
  OPTION_SECTORS,
  OPTION_SECTOR_SIZE,
  OPTION_NULL
};

static const char* options[] = {
  [OPTION_READONLY] = "readonly",
  [OPTION_CYLINDERS] = "cylinders",
  [OPTION_HEADS] = "heads",
  [OPTION_SECTORS] = "sectors",
  [OPTION_SECTOR_SIZE] = "sector-size",
  [OPTION_NULL] = NULL
};

static const struct {
  int (*probe)(const char* path, struct module_options*, struct device_implementation*);
} implementations[] = { 
      {&linux_device_probe}
};

#define IMPLEMENTATIONS_LENGTH sizeof(implementations) / sizeof(implementations[0])

static void device_open(void* _object, struct gnufdisk_string* _path);
static void device_close(void* _p);
static void device_set_parameter(void* _object, struct gnufdisk_string* _param, const void* _data, size_t _size);
static void device_delete(void* _object);

static void delete_object(void* _p)
{
  GNUFDISK_LOG((DEVICE, "delete struct object* %p", _p));
  object_delete(_p);
}

static void device_private_check(struct device_private* _p)
{
  if(gnufdisk_check_memory(_p, sizeof(struct device_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct device_private* %p", _p);
}

static void parse_module_options(const char* _options, struct module_options* _dest)
{
  GNUFDISK_LOG((DEVICE, "parse module options `%s'", _options));

  memset(_dest, 0, sizeof(struct module_options));

  while(*_options != 0)
    {
      char* argument;

      switch(getsubopt(&_options, options, &argument))
	{
	  case OPTION_READONLY:
	    _dest->readonly = 1;
	    break;
	  case OPTION_CYLINDERS:
	    if(argument == NULL)
	      {
		GNUFDISK_WARNING("missing parameter for option `%s'", options[OPTION_CYLINDERS]);
		break;
	      }
	    else if(sscanf(argument, "%" SCNd64, &_dest->cylinders) != 1)
	      GNUFDISK_WARNING("bad parameter for option `%s'", options[OPTION_CYLINDERS]);
	    break;
	  case OPTION_HEADS:
	    if(argument == NULL)
	      {
		GNUFDISK_WARNING("missing parameter for option `%s'", options[OPTION_HEADS]);
		break;
	      }
	    else if(sscanf(argument, "%" SCNd64, &_dest->heads) != 1)
	      GNUFDISK_WARNING("bad parameter for option `%s'", options[OPTION_HEADS]);
	    break;
	  case OPTION_SECTORS:
	    if(argument == NULL)
	      {
		GNUFDISK_WARNING("missing parameter for option `%s'", options[OPTION_SECTORS]);
		break;
	      }
	    else if(sscanf(argument, "%" SCNd64, &_dest->sectors) != 1)
	      GNUFDISK_WARNING("bad parameter for option `%s'", options[OPTION_SECTORS]);
	    break;
	  case OPTION_SECTOR_SIZE:
	    if(argument == NULL)
	      {
		GNUFDISK_WARNING("missing parameter for option `%s'", options[OPTION_SECTOR_SIZE]);
		break;
	      }
	    else if(sscanf(argument, "%" SCNd64, &_dest->sector_size) != 1)
	      GNUFDISK_WARNING("bad parameter for option `%s'", options[OPTION_SECTOR_SIZE]);
	    break;
	  default:
	    GNUFDISK_WARNING("unknown option: `%s'", argument);
	}
    } 

  GNUFDISK_LOG((DEVICE, "done parse module options:"));
  GNUFDISK_LOG((DEVICE, "  readonly    : %d", _dest->readonly));
  GNUFDISK_LOG((DEVICE, "  cylinders   : %" PRId64, _dest->cylinders));
  GNUFDISK_LOG((DEVICE, "  heads       : %" PRId64, _dest->heads));
  GNUFDISK_LOG((DEVICE, "  sectors     : %" PRId64, _dest->sectors));
  GNUFDISK_LOG((DEVICE, "  sector_size : %" PRId64, _dest->sector_size));  
}

/* OBJECT operations */
static struct object* device_private_cast(void* _p, enum object_type _type)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "cats device object to type %d", _type));

  device_private_check(_p);

  private = _p;

  if(_type == OBJECT_TYPE_DISKLABEL)
    return private->disklabel;

  return NULL;
}

static gnufdisk_integer device_private_start(void* _private)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform start on struct device_private* %p", _private));

  device_private_check(_private);

  private = _private;

  if(gnufdisk_check_memory(private->implementation.start, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `start'");
  
  ret = (*private->implementation.start)(private->implementation.private);

  GNUFDISK_LOG((DEVICE, "done perform start, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer device_private_end(void* _private)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform end on struct device_private* %p", _private));

  device_private_check(_private);

  private = _private;

  if(gnufdisk_check_memory(private->implementation.end, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `end'");
  
  ret = (*private->implementation.end)(private->implementation.private);

  GNUFDISK_LOG((DEVICE, "done perform end, result: %" PRId64, ret));

  return ret;
}

static void device_private_delete(void* _p)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "perform delete on struct device_private* %p", _p));

  device_private_check(_p);

  private = _p;

  if(private->disklabel)
    object_delete(private->disklabel);

  memset(private, 0, sizeof(struct device_private));
  free(private);

  GNUFDISK_LOG((DEVICE, "done perform delete"));
}

static const struct object_private_operations device_private_operations = { 
  cast: &device_private_cast,
  start: &device_private_start,
  end: &device_private_end,
  delete: &device_private_delete 
};

static void device_open(void* _object, struct gnufdisk_string* _path)
{
  struct device_private* private;
  GNUFDISK_RETRY rp0;
  int iter;
  char* path;
  struct device_implementation device_implementation;

  GNUFDISK_LOG((DEVICE, "perform open on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  path = NULL;

  GNUFDISK_RETRY_SET(rp0);

  /* If there was an exception path is being allocated */

  if(path)
    {
      gnufdisk_exception_unregister_unwind_handler(&free, path);
      free(path);
    }

  if((path = gnufdisk_string_c_string_dup(_path)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, path);

  GNUFDISK_LOG((DEVICE, "path: %s", path, _object));

  for(iter = 0; iter < IMPLEMENTATIONS_LENGTH; iter++)
    if((*implementations[iter].probe)(path, &private->options, &device_implementation) == 0)
      break;

  if(iter == IMPLEMENTATIONS_LENGTH)
    {
      /* retry with a new path */
      union gnufdisk_device_exception_data data;

      GNUFDISK_LOG((DEVICE, "unable to open `%s', retry", path));

      data.epath = _path;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0, GNUFDISK_DEVICE_EPATH, &data, "can not open `%s'", path);
    }

  GNUFDISK_LOG((DEVICE, "device open successfully"));

  if(private->is_open)
    {
      GNUFDISK_LOG((DEVICE, "close previous device"));
      device_close(_object);
    }
  
  private->is_open = 1;

  memcpy(&private->implementation, &device_implementation, sizeof(struct device_implementation));

  gnufdisk_exception_unregister_unwind_handler(&free, path);
  free(path);

  GNUFDISK_LOG((DEVICE, "done perform open"));
}

static void device_disklabel(void* _object, struct gnufdisk_disklabel_operations* _operations, void** _specific)
{
  struct device_private* private;
  struct object* disklabel;

  GNUFDISK_LOG((DEVICE, "perform disklabel on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(private->disklabel)
    {
      object_ref(private->disklabel);
      memcpy(_operations, &disklabel_operations, sizeof(struct gnufdisk_disklabel_operations));
      *_specific = private->disklabel;
    }
  else if((disklabel = disklabel_probe(_object)) == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "unsupported disklabel type");

  memcpy(_operations, &disklabel_operations, sizeof(struct gnufdisk_disklabel_operations));
  *_specific = disklabel;

  object_ref(disklabel);
  private->disklabel = disklabel;

  GNUFDISK_LOG((DEVICE, "done perform disklabel"));
}

static void device_create_disklabel(void* _object, struct gnufdisk_string* _system, struct gnufdisk_disklabel_operations* _operations, void** _specific)
{
  struct device_private* private;
  struct object* disklabel;

  GNUFDISK_LOG((DEVICE, "perform create_disklabel on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  device_private_check(private);

  disklabel = disklabel_new(_object, _system);

  gnufdisk_exception_register_unwind_handler(&delete_object, disklabel);

  if(private->disklabel)
    {
      object_delete(private->disklabel);
      private->disklabel = NULL;
    }

  object_ref(disklabel);
  private->disklabel = disklabel;

  memcpy(_operations, &disklabel_operations, sizeof(struct gnufdisk_disklabel_operations));
  *_specific = disklabel;

  gnufdisk_exception_unregister_unwind_handler(&delete_object, disklabel);

  GNUFDISK_LOG((DEVICE, "done perform create_disklabel"));
}

static void device_set_parameter(void* _object, struct gnufdisk_string* _param, const void* _data, size_t _size)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "perform set_parameter on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  device_private_check(private);

  if(gnufdisk_check_memory(private->implementation.set_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device implementation does not support `set_parameter'");

  (*private->implementation.set_parameter)(private->implementation.private, _param, _data, _size);

  GNUFDISK_LOG((DEVICE, "done perform set_parameter"));
}

void device_get_parameter(void* _object, struct gnufdisk_string* _param, void* _dest, size_t _size)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "perform get_parameter on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  device_private_check(private);

  if(gnufdisk_check_memory(private->implementation.get_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device implementation does not support `get_parameter'");

  (*private->implementation.get_parameter)(private->implementation.private, _param, _dest, _size);

  GNUFDISK_LOG((DEVICE, "done perform get_parameter"));
}

static void device_commit(void* _object)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "perform commit on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.commit, 1, 1) == 0)
    (*private->implementation.commit)(private->implementation.private);

  if(gnufdisk_check_memory(private->disklabel, 1, 1) == 0)
    disklabel_commit(private->disklabel);

  GNUFDISK_LOG((DEVICE, "done perform commit"));
}

static void device_close(void* _object)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "perform close on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  device_private_check(private);

  GNUFDISK_LOG((DEVICE, "delete implementation instance"));

  if(gnufdisk_check_memory(private->implementation.delete, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device implementation does not support `delete'");

  (*private->implementation.delete)(private->implementation.private);

  memset(&private->implementation, 0, sizeof(struct device_implementation));
  private->is_open = 0;

  GNUFDISK_LOG((DEVICE, "done perform close"));
}

static void device_delete(void* _object)
{
  struct device_private* private;

  GNUFDISK_LOG((DEVICE, "perform delete on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(private->is_open)
    {
      GNUFDISK_LOG((DEVICE, "device is open, close before delete"));
      device_close(_object);
    }

  object_delete(_object);

  GNUFDISK_LOG((DEVICE, "done perform delete"));
}


static const struct gnufdisk_device_operations device_operations = { 
  open: &device_open,
  disklabel: &device_disklabel,
  create_disklabel: &device_create_disklabel,
  set_parameter: &device_set_parameter,
  get_parameter: &device_get_parameter,
  commit: &device_commit,
  close: &device_close,
  delete: &device_delete
};

gnufdisk_integer device_seek(void* _object, gnufdisk_integer _lba, gnufdisk_integer _offset, int _whence)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform seek on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.seek, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `seek'");
 
  ret = (*private->implementation.seek)(private->implementation.private, _lba, _offset, _whence);

  GNUFDISK_LOG((DEVICE, "done perform seek, result: %" PRId64, ret));

  return ret;
}

gnufdisk_integer device_read(void* _object, void* _buf, size_t _size)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform read on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.read, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `read'");
  
  ret = (*private->implementation.read)(private->implementation.private, _buf, _size);

  GNUFDISK_LOG((DEVICE, "done perform read, result: %" PRId64, ret));

  return ret;
}

gnufdisk_integer device_write(void* _object, const void* _buf, size_t _size)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform write on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.write, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `write'");
  
  ret = (*private->implementation.write)(private->implementation.private, _buf, _size);

  GNUFDISK_LOG((DEVICE, "done perform write, result: %" PRId64, ret));

  return ret;
}

gnufdisk_integer device_sector_size(void* _object)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform sector_size on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.sector_size, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `sector_size'");
  
  ret = (*private->implementation.sector_size)(private->implementation.private);

  GNUFDISK_LOG((DEVICE, "done perform sector_size, result: %" PRId64, ret));

  return ret;
}

gnufdisk_integer device_minimum_alignment(void* _object)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform minimum_alignment on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.minimum_alignment, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `minimum_alignment'");
  
  ret = (*private->implementation.minimum_alignment)(private->implementation.private);

  GNUFDISK_LOG((DEVICE, "done perform minimum_alignment, result: %" PRId64, ret));

  return ret;
}

gnufdisk_integer device_optimal_alignment(void* _object)
{
  struct device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform optimal_alignment on struct object* %p", _object));

  private = object_private(_object, OBJECT_TYPE_DEVICE);

  if(gnufdisk_check_memory(private->implementation.optimal_alignment, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "device_implementation does not support `optimal_alignment'");
  
  ret = (*private->implementation.optimal_alignment)(private->implementation.private);

  GNUFDISK_LOG((DEVICE, "done perform optimal_alignment, result: %" PRId64, ret));

  return ret;
}


/* main module entry point */
void module_register(struct gnufdisk_string* _options,
		     struct gnufdisk_device_operations* _ops, 
		     void** _spec)
{
  struct device_private* dev;
  struct object* object;

  GNUFDISK_LOG((DEVICE, "register new device"));  

  if((dev = malloc(sizeof(struct device_private))) == NULL)
    THROW_ENOMEM;
  
  gnufdisk_exception_register_unwind_handler(&free, dev);

  memset(dev, 0, sizeof(struct device_private));

  parse_module_options(gnufdisk_string_c_string(_options), &dev->options);
  
  object = object_new(OBJECT_TYPE_DEVICE, &device_private_operations, dev);

  memcpy(_ops, &device_operations, sizeof(struct gnufdisk_device_operations));
  *_spec = object;

  gnufdisk_exception_unregister_unwind_handler(&free, dev);

  GNUFDISK_LOG((DEVICE, "new device object allocated at %p", object));
}

