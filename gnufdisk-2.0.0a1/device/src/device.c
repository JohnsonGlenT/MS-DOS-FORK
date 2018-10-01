
/* GNU fdisk, (gnufdisk-device) a library to manage a device
 * Copyright (C) 2011 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dlfcn.h>

#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-device.h>
#include <gnufdisk-device-internals.h>

#define DEVICE 1

extern struct gnufdisk_disklabel *
gnufdisk_device_internals__allocate_disklabel (struct gnufdisk_device *_dev,
					       struct gnufdisk_disklabel_operations *_operations,
					       void *_implementation_data);

extern void gnufdisk_device_internals__disklabel_set_device (struct gnufdisk_disklabel *_d,
							     struct gnufdisk_device *_dev);

struct gnufdisk_device
{
  void *handle;
  struct gnufdisk_device_operations operations;
  void *implementation_data;
  int is_open;
  int nref;
};

static void
check_device (struct gnufdisk_device **_dev)
{
  GNUFDISK_RETRY rp0;
  union gnufdisk_device_exception_data data;

  GNUFDISK_RETRY_SET (rp0);

  if (gnufdisk_check_memory (*_dev, sizeof (struct gnufdisk_device), 0) != 0)
    {
      data.edevicepointer = _dev;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EDEVICEPOINTER, &data,
		      "invalid struct gnufdisk_device* %p", *_dev);
    }
  else if (gnufdisk_check_memory((*_dev)->handle, 1, 1) != 0)
    {
      (*_dev)->handle = NULL;

      GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EDEVICE, NULL,
		      "device is not associated with a valid module");
    }
}

static void check_is_open(struct gnufdisk_device* _d)
{
  GNUFDISK_RETRY rp0;

  GNUFDISK_RETRY_SET(rp0);

  check_device(&_d);

  GNUFDISK_LOG((DEVICE, "device open status: %d", _d->is_open));

  if(!_d->is_open)
    {
      union gnufdisk_device_exception_data data;

      data.enotopen = _d;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
                     &rp0,
                     GNUFDISK_DEVICE_ENOTOPEN,
                     &data,
                     "device is not open");
    }
}

static void
free_pointer (void *_p)
{
  GNUFDISK_LOG ((DEVICE, "free pointer %p", _p));
  free (_p);
}

static void
close_dlhandle (void *_p)
{
  GNUFDISK_LOG ((DEVICE, "close dlhandle %p", _p));
  dlclose (_p);
}

struct gnufdisk_device *
gnufdisk_device_new (struct gnufdisk_string *_module, struct gnufdisk_string *_options)
{
  char library[PATH_MAX];

  GNUFDISK_RETRY rp0;

  struct gnufdisk_device *dev;
  void (*reg) (struct gnufdisk_string * _options,
	       struct gnufdisk_device_operations * _operations,
	       void **_private_data);

  if((dev = malloc (sizeof (struct gnufdisk_device))) == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOMEM, NULL, "cannot allocate memory");
  
  if(gnufdisk_exception_register_unwind_handler (&free_pointer, dev) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_register_unwind_handler failed. Missing GNUFDISK_TRY?");

  memset(dev, 0, sizeof(struct gnufdisk_device));

  GNUFDISK_RETRY_SET (rp0);

  if (gnufdisk_check_memory (_module, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.emodulepointer = &_module;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0, GNUFDISK_DEVICE_EMODULEPOINTER,
		      &data, "invalid struct gnufdisk_string* %p", _module);
    }

  snprintf (library, sizeof (library), "%s.so",
	    gnufdisk_string_c_string (_module));

  GNUFDISK_LOG ((DEVICE, "open module `%s'", library));

  if ((dev->handle = dlopen (library, RTLD_NOW)) == NULL)
    {
      union gnufdisk_device_exception_data data;

      data.emodule = _module;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0, GNUFDISK_DEVICE_EMODULE,
		      &data, "cannot open module: %s", dlerror ());
    }
  else if ((reg = dlsym (dev->handle, "module_register")) == NULL)
    {
      union gnufdisk_device_exception_data data;

      dlclose (dev->handle);	/* close module */
      dev->handle = NULL;

      data.emodule = _module;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0, GNUFDISK_DEVICE_EMODULE,
		      &data, "error register module `%s': %s", library,
		      dlerror ());
    }

  /* so if `reg' throw an exception handle is closed */
  if(gnufdisk_exception_register_unwind_handler (&close_dlhandle, dev->handle) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_register_unwind_handler failed. Missing GNUFDISK_TRY?");

  (*reg) (_options, &dev->operations, &dev->implementation_data);

  dev->nref = 1;

  GNUFDISK_LOG ((DEVICE, "new struct gnufdisk_device* %p", dev));

  if(gnufdisk_exception_unregister_unwind_handler(&close_dlhandle, dev->handle) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_unregister_unwind_handler failed. Missing GNUFDISK_TRY?");

  if(gnufdisk_exception_unregister_unwind_handler(&free_pointer, dev) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_unregister_unwind_handler failed. Missing GNUFDISK_TRY?");

  return dev;
}

void
gnufdisk_device_delete (struct gnufdisk_device *_d)
{
  check_device (&_d);

  if (_d->nref < 2)
    {
      GNUFDISK_LOG ((DEVICE, "delete struct gnufdisk_device* %p", _d));

      if (_d->implementation_data != NULL && _d->operations.delete != NULL)
	{
	  void *data;

	  data = _d->implementation_data;
	  _d->implementation_data = NULL;

	  (*_d->operations.delete) (data);
	}

      if (_d->handle)
	{
	  dlclose (_d->handle);
	  _d->handle = NULL;
	}

      free (_d);
    }
  else
    {
      _d->nref--;
      GNUFDISK_LOG ((DEVICE,
		     "struct gnufdisk_device* %p has now %d references", _d,
		     _d->nref));
    }
}

void
gnufdisk_device_ref (struct gnufdisk_device *_d)
{
  check_device (&_d);

  _d->nref++;

  GNUFDISK_LOG ((DEVICE, "struct gnufdisk_device* %p has now %d references",
		 _d, _d->nref));
}

void
gnufdisk_device_open (struct gnufdisk_device *_d,
		      struct gnufdisk_string *_path)
{
  GNUFDISK_RETRY rp0;

  check_device (&_d);

  GNUFDISK_RETRY_SET (rp0);

  if (gnufdisk_check_memory (_path, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.epathpointer = &_path;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EPATHPOINTER, &data,
		      "invalid istruct gnufdisk_string* %p", _path);
    }
  else if (_d->operations.open == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `open'");

  (*_d->operations.open) (_d->implementation_data, _path);
  _d->is_open = 1;
}

struct gnufdisk_disklabel *
gnufdisk_device_disklabel (struct gnufdisk_device *_d)
{
  struct gnufdisk_disklabel_operations operations;
  void *implementation_data;

  check_device (&_d);
  check_is_open(_d);

  if (_d->operations.disklabel == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `disklabel'");

  (*_d->operations.disklabel) (_d->implementation_data, &operations,
			       &implementation_data);

  return gnufdisk_device_internals__allocate_disklabel (_d, &operations, implementation_data);
}

struct gnufdisk_disklabel *
gnufdisk_device_create_disklabel (struct gnufdisk_device *_d,
				  struct gnufdisk_string* _type)
{
  struct gnufdisk_disklabel_operations operations;
  void *implementation_data;
  GNUFDISK_RETRY rp0;

  check_device (&_d);
  check_is_open(_d);

  GNUFDISK_RETRY_SET (rp0);

  if (gnufdisk_check_memory(_type, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.edisklabelsystempointer = &_type;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EDISKLABELSYSTEMPOINTER, &data,
		      "invalid struct gnufdisk_string* %p", _type);
    }

  if (_d->operations.create_disklabel == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `create disklabel'");

  (*_d->operations.create_disklabel) (_d->implementation_data, _type,
				      &operations, &implementation_data);

  return gnufdisk_device_internals__allocate_disklabel (_d, &operations, implementation_data);
}

void
gnufdisk_device_set_parameter (struct gnufdisk_device *_d,
			       struct gnufdisk_string* _parameter, 
			       const void *_data,
			       size_t _size)
{
  check_device (&_d);
  check_is_open(_d);

  if (gnufdisk_check_memory(_parameter, 1, 1) != 0)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EPARAMETERPOINTER, NULL,
		    "invalid struct gnufdisk_string* %p", _parameter);
  else if (gnufdisk_check_memory((void*)_data, _size, 1) != 0)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EPARAMETERDATA, NULL,
		    "invalid device parameter data: %p", _data);
  else if (_d->operations.set_parameter == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `set parameter'");

  (*_d->operations.set_parameter) (_d->implementation_data, _parameter, _data,
				   _size);
}

void
gnufdisk_device_get_parameter (struct gnufdisk_device *_d,
			       struct gnufdisk_string* _parameter,
			       void *_data,
			       size_t _size)
{
  check_device (&_d);
  check_is_open(_d);

  if (gnufdisk_check_memory(_parameter, 1, 1) != 0)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EPARAMETERPOINTER, NULL,
		    "invalid struct gnufdisk_string* %p", _parameter);
  else if (gnufdisk_check_memory(_data, _size, 0) != 0)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EPARAMETERDATA, NULL,
		    "invalid device parameter data: %p", _data);
  else if (_d->operations.get_parameter == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `get parameter'");

  (*_d->operations.get_parameter) (_d->implementation_data, _parameter, _data,
				   _size);
}

void
gnufdisk_device_commit (struct gnufdisk_device *_d)
{
  check_device (&_d);
  check_is_open(_d);

  if (_d->operations.commit == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `commit'");

  (*_d->operations.commit) (_d->implementation_data);
}

void
gnufdisk_device_close (struct gnufdisk_device *_d)
{
  check_device (&_d);
  check_is_open(_d);

  GNUFDISK_LOG((DEVICE, "close device %p", _d));

  if (_d->operations.close == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `close'");

  (*_d->operations.close) (_d->implementation_data);
  _d->is_open = 0;
}

