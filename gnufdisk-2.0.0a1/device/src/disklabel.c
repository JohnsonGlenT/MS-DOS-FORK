

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

#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-device.h>
#include <gnufdisk-device-internals.h>

#define DISKLABEL 1

extern struct gnufdisk_partition*
gnufdisk_device_internals__allocate_partition(struct gnufdisk_disklabel* _d,
					      struct gnufdisk_partition_operations* _operations,
					      void *_implementation_data);

struct gnufdisk_disklabel {
	struct gnufdisk_disklabel_operations operations;
	void* implementation_data;
	struct gnufdisk_device* device;
	int nref;
};

static void
check_disklabel (struct gnufdisk_disklabel **_d)
{
  GNUFDISK_RETRY rp0;
  union gnufdisk_device_exception_data data;

  GNUFDISK_RETRY_SET (rp0);

  if (gnufdisk_check_memory (*_d, sizeof (struct gnufdisk_disklabel), 0) != 0)
    {
      data.edisklabelpointer = _d;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EDISKLABELPOINTER, &data,
		      "invalid struct gnufdisk_disklabel* NULL");
    }
  else if (gnufdisk_check_memory ((*_d)->device, 1, 1) != 0)
    {
      data.edevicepointer = &(*_d)->device;
      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EDEVICEPOINTER, &data,
		      "label is not associated with  a device");
    }
}

static void
free_pointer (void *_p)
{
  GNUFDISK_LOG ((DISKLABEL, "free pointer %p", _p));
  free (_p);
}

struct gnufdisk_disklabel *
gnufdisk_device_internals__allocate_disklabel (struct gnufdisk_device *_dev,
					       struct gnufdisk_disklabel_operations *_operations,
					       void *_implementation_data)
{
  struct gnufdisk_disklabel *disk;

  if (gnufdisk_check_memory (_dev, 1, 1) != 0)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL,
		    "invalid struct gnufdisk_device* %p", _dev);
  else if (gnufdisk_check_memory (_operations, sizeof(struct gnufdisk_disklabel_operations), 1) != 0)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL,
		    "invalid struct gnufdisk_disklabel_operations* %p",
		    _operations);

  if((disk = malloc (sizeof (struct gnufdisk_disklabel))) == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOMEM, NULL, "cannot allocate memory");

  if(gnufdisk_exception_register_unwind_handler (&free_pointer, disk) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_register_unwind_handler failed. Missing GNUFDISK_TRY?");

  memset(disk, 0, sizeof(struct gnufdisk_disklabel));

  memcpy (&disk->operations, _operations,
	  sizeof (struct gnufdisk_disklabel_operations));
  disk->implementation_data = _implementation_data;
  disk->device = _dev;
  gnufdisk_device_ref (_dev);
  disk->nref = 1;

  if(gnufdisk_exception_unregister_unwind_handler(&free_pointer, disk) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_unregister_unwind_handler failed. Missing GNUFDISK_TRY?");

  return disk;
}

void
gnufdisk_device_internals__disklabel_set_device (struct gnufdisk_disklabel *_d,
						 struct gnufdisk_device *_dev)
{
  check_disklabel (&_d);

  if (_d->device)
    {
      gnufdisk_device_delete (_d->device);
      _d->device = NULL;
    }

  gnufdisk_device_ref (_dev);
  _d->device = _dev;
}

struct gnufdisk_device*
gnufdisk_device_internals__disklabel_get_device (struct gnufdisk_disklabel *_d)
{
  check_disklabel (&_d);

  return _d->device;
}

void
gnufdisk_disklabel_delete (struct gnufdisk_disklabel *_d)
{
  check_disklabel (&_d);

  if (_d->nref < 2)
    {
      if (_d->implementation_data && _d->operations.delete != NULL)
	{
	  void *data;

	  data = _d->implementation_data;
	  _d->implementation_data = NULL;

	  (*_d->operations.delete) (data);
	}

      if (_d->device)
	{
	  gnufdisk_device_delete (_d->device);
	  _d->device = NULL;
	}

      free (_d);
    }
  else
    _d->nref--;
}

void
gnufdisk_disklabel_ref (struct gnufdisk_disklabel *_d)
{
  check_disklabel (&_d);

  _d->nref++;
}

void
gnufdisk_disklabel_raw (struct gnufdisk_disklabel *_d, void **_dest,
			size_t * _size)
{
  GNUFDISK_RETRY rp0;

  check_disklabel (&_d);

  GNUFDISK_RETRY_SET(rp0);

  if (gnufdisk_check_memory(_dest, 1, 0) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.edestinationpointer = &_dest;
      
      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EDESTINATIONPOINTER, &data,
		      "invalid destination pointer: %p", _dest);
    }
  else if(gnufdisk_check_memory(_size, 1, 0) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.esizepointer = &_size;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_ESIZEPOINTER, &data,
		     "invalid size pointer: %p", _size);
    
    
      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_ESIZEPOINTER, &data,
		     "invalid size pointer: %p", _size);
    }
  else if(_d->operations.raw == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported: `raw'");

  (*_d->operations.raw)(_d->implementation_data, _dest, _size);
}

struct gnufdisk_string* gnufdisk_disklabel_system(struct gnufdisk_disklabel* _d)
{
  check_disklabel(&_d);

  if(_d->operations.system == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `system'");

  return (*_d->operations.system)(_d->implementation_data);
}

struct gnufdisk_partition* 
gnufdisk_disklabel_partition(struct gnufdisk_disklabel* _d, size_t _n)
{
  struct gnufdisk_partition_operations operations;
  void* implementation_data;

  check_disklabel(&_d);

  if(_d->operations.partition == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `partition'");

  (*_d->operations.partition)(_d->implementation_data, _n, &operations, &implementation_data);

  return gnufdisk_device_internals__allocate_partition(_d, &operations, implementation_data);
}

int gnufdisk_disklabel_count_partitions(struct gnufdisk_disklabel* _d)
{
  check_disklabel(&_d);

  if(_d->operations.count_partitions == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `count_partitions'");

  return (*_d->operations.count_partitions)(_d->implementation_data);
}

struct gnufdisk_partition* gnufdisk_disklabel_create_partition(struct gnufdisk_disklabel* _d,
							       struct gnufdisk_geometry* _s,
							       struct gnufdisk_geometry* _e,
							       struct gnufdisk_string* _type)
{
  GNUFDISK_RETRY rp0;
  struct gnufdisk_partition_operations operations;
  void *implementation_data;

  check_disklabel(&_d);

  GNUFDISK_RETRY_SET(rp0);

  if(gnufdisk_check_memory(_s, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.egeometrypointer = &_s;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0, 
		     GNUFDISK_DEVICE_EGEOMETRYPOINTER, &data, "invalid struct gnufdisk_geometry* %p", _s);
    }
  else if(gnufdisk_check_memory(_e, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.egeometrypointer = &_e;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0, 
		     GNUFDISK_DEVICE_EGEOMETRYPOINTER, &data, "invalid struct gnufdisk_geometry* %p", _e);
    }
  else if(gnufdisk_check_memory(_type, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.epartitiontypepointer = &_type;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_EPARTITIONTYPEPOINTER, &data, "invalid struct gnufdisk_string* %p", _type);
    }
  else if(_d->operations.create_partition == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `create_partition'");

  (*_d->operations.create_partition)(_d->implementation_data, _s, _e, _type, &operations, &implementation_data);

  return gnufdisk_device_internals__allocate_partition(_d, &operations, implementation_data);
}


void gnufdisk_disklabel_remove_partition(struct gnufdisk_disklabel* _d, size_t _n)
{
  check_disklabel(&_d);

  if(_d->operations.remove_partition == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `remove_partition'");

  (*_d->operations.remove_partition)(_d->implementation_data, _n);
}

void gnufdisk_disklabel_set_parameter(struct gnufdisk_disklabel* _d,
				      struct gnufdisk_string* _parameter,
				      const void* _data,
				      size_t _size)
{
  check_disklabel(&_d);

  if(gnufdisk_check_memory(_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERPOINTER, NULL, "invalid struct gnufdisk_string* %p", _parameter);
  else if(gnufdisk_check_memory((void*)_data, _size, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERDATA, NULL, "invalid parameter data: %p", _data);
  else if(_d->operations.set_parameter == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `set_parameter'");

  (*_d->operations.set_parameter)(_d->implementation_data, _parameter, _data, _size);
}

void gnufdisk_disklabel_get_parameter(struct gnufdisk_disklabel* _d,
				      struct gnufdisk_string* _parameter,
				      void* _data,
				      size_t _size)
{
  check_disklabel(&_d);

  if(gnufdisk_check_memory(_parameter, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERPOINTER, NULL, "invalid struct gnufdisk_string* %p", _parameter);
  else if(gnufdisk_check_memory(_data, _size, 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERDATA, NULL, "invalid parameter data: %p", _data);
  else if(_d->operations.get_parameter == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `get_parameter'");

  (*_d->operations.get_parameter)(_d->implementation_data, _parameter, _data, _size);
}

