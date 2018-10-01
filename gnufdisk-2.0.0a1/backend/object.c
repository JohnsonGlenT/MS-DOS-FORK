#include "common.h"

struct object {
  enum object_type type;
  int nref;
  struct object_private_operations operations;
  void* private;
};

static void object_check(struct object* _p)
{
  if(gnufdisk_check_memory(_p, sizeof(struct object), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct object* %p", _p);
}

struct object* object_new(enum object_type _type, 
			  const struct object_private_operations* _operations,
			  void* _private)
{
  struct object* ret;

  if(gnufdisk_check_memory((void*) _operations, sizeof(struct object_private_operations), 1) != 0)
    GNUFDISK_THROW(0, 
		   NULL, 
		   GNUFDISK_DEVICE_EINTERNAL, 
		   NULL, 
		   "invalid struct object_private_operations* %p", 
		   _operations);

  if((ret = malloc(sizeof(struct object))) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, ret);

  ret->type = _type;
  ret->nref = 1;
  memcpy(&ret->operations, _operations, sizeof(struct object_private_operations));
  ret->private = _private;

  gnufdisk_exception_unregister_unwind_handler(&free, ret);

  GNUFDISK_LOG((OBJECT, "new struct object* %p (type %d)", ret, _type));

  return ret;
}

struct object* object_ref(struct object* _o)
{
  object_check(_o);

  _o->nref++;

  GNUFDISK_LOG((OBJECT, "struct object* %p has %d references", _o, _o->nref));

  return _o;
}

enum object_type object_type(struct object* _o)
{
  object_check(_o);

  return _o->type;
}

#ifdef GNUFDISK_DEBUG
int object_nref(struct object* _o)
{
  object_check(_o);

  return _o->nref;
}
#endif

void object_delete(struct object* _o)
{
  object_check(_o);

  if(_o->nref >= 1)
    {
      _o->nref--;

      GNUFDISK_LOG((OBJECT, "struct object* %p has %d references", _o, _o->nref));

      if(_o->nref == 0)
	{
	  GNUFDISK_LOG((OBJECT, "delete struct object* %p", _o));

	  if(gnufdisk_check_memory(_o->operations.delete, 1, 1) == 0)
	    (*_o->operations.delete)(_o->private);

	  free(_o);
	}
    }
  else
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid reference count on struct object* %p", _o);
}

struct object* object_cast(struct object* _o, enum object_type _type)
{
  struct object* ret;

  GNUFDISK_LOG((OBJECT, 
		"try cast struct object* %p to type %d",
		_o,
		_type));

  object_check(_o);

  ret = NULL;

  if(_o->type == _type)
    ret = _o;
  else if(gnufdisk_check_memory(_o->operations.cast, 1, 1) == 0)
    ret = (*_o->operations.cast)(_o->private, _type);

  if(ret == NULL)
    GNUFDISK_THROW(0, 
		   NULL, 
		   GNUFDISK_DEVICE_EINTERNAL, 
		   NULL, 
		   "can not cast struct object* %p from type %d to type %d", 
		   _o, 
		   _o->type, 
		   _type);

  return ret;
}

gnufdisk_integer object_start(struct object* _o)
{
  gnufdisk_integer ret;

  GNUFDISK_LOG((OBJECT, "perform start on struct object* %p", _o));

  object_check(_o);

  if(gnufdisk_check_memory(_o->operations.start, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "object does not support `start'");

  ret = (*_o->operations.start)(_o->private);

  GNUFDISK_LOG((OBJECT, "done perform start, result: %" PRId64, ret));

  return ret;
}

gnufdisk_integer object_end(struct object* _o)
{
  gnufdisk_integer ret;

  GNUFDISK_LOG((OBJECT, "perform end on struct object* %p", _o));

  object_check(_o);

  if(gnufdisk_check_memory(_o->operations.end, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "object does not support `end'");

  ret = (*_o->operations.end)(_o->private);

  GNUFDISK_LOG((OBJECT, "done perform end, result: %" PRId64, ret));

  return ret;
}

void* object_private(struct object* _o, enum object_type _type)
{
  object_check(_o);

  GNUFDISK_LOG((OBJECT, "get private from struct object* %p (types: %d-%d)", _o, _o->type, _type));

  if(_o->type != _type)
    _o = object_cast(_o, _type);

  return _o->private;
}


