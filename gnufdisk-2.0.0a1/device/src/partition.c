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

struct gnufdisk_disklabel *
gnufdisk_device_internals__allocate_disklabel (struct gnufdisk_device *_dev,
					       struct gnufdisk_disklabel_operations *_operations,
					       void *_implementation_data);

struct gnufdisk_device*
gnufdisk_device_internals__disklabel_get_device (struct gnufdisk_disklabel *_d);

#define PARTITION 1

struct gnufdisk_partition {
  struct gnufdisk_partition_operations operations;
  void* implementation_data;
  struct gnufdisk_disklabel* disklabel;
  int nref;
};

static void free_pointer(void* _p)
{
  GNUFDISK_LOG((PARTITION, "free pointer %p", _p));
  free(_p);
}

static void check_partition(struct gnufdisk_partition** _p)
{
  GNUFDISK_RETRY rp0;
  union gnufdisk_device_exception_data data;

  GNUFDISK_RETRY_SET(rp0);

  if(gnufdisk_check_memory(*_p, sizeof(struct gnufdisk_partition), 0) != 0)
    {
      data.epartitionpointer = _p;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0, 
		     GNUFDISK_DEVICE_EPARTITIONPOINTER, &data, "invalid struct gnufdisk_partition* %p", *_p);
    }
  else if(gnufdisk_check_memory((*_p)->disklabel, 1, 1) != 0)
    {
      data.edisklabelpointer = &(*_p)->disklabel;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_EDISKLABELPOINTER, &data, "invalid struct gnufdisk_disklabel* %p", (*_p)->disklabel);
    }
}
struct gnufdisk_partition*
gnufdisk_device_internals__allocate_partition(struct gnufdisk_disklabel* _d,
					      struct gnufdisk_partition_operations* _operations,
					      void *_implementation_data)
{
  struct gnufdisk_partition* ret;

  if(gnufdisk_check_memory(_d, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct gnufdisk_disklabel* %p", _d);
  else if(gnufdisk_check_memory(_operations, sizeof(struct gnufdisk_partition_operations), 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct gnufdisk_partition_operations* %p", _operations);
  
  if((ret = malloc(sizeof(struct gnufdisk_partition))) == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOMEM, NULL, "cannot allocate memory");

  if(gnufdisk_exception_register_unwind_handler(&free_pointer, ret) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_register_unwind_handler failed. Missing GNUFDISK_TRY?");

  memset(ret, 0, sizeof(struct gnufdisk_partition));

  memcpy(&ret->operations, _operations, sizeof(struct gnufdisk_partition_operations));
  ret->implementation_data = _implementation_data;
  ret->disklabel = _d;
  gnufdisk_disklabel_ref(_d);
  ret->nref = 1;

  if(gnufdisk_exception_unregister_unwind_handler(&free_pointer, ret) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_unregister_unwind_handler failed. Missing GNUFDISK_TRY?");

  return ret;
}

void gnufdisk_partition_ref(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  _p->nref++;
}

void gnufdisk_partition_delete(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  if(_p->nref < 2)
    {
      if(_p->implementation_data && _p->operations.delete != NULL)
	{
	  void* data;

	  data = _p->implementation_data;
	  _p->implementation_data = NULL;
	
	  (*_p->operations.delete)(data);
	}

      if(_p->disklabel)
	{
	  gnufdisk_disklabel_delete(_p->disklabel);
	  _p->disklabel = NULL;
	}

      free(_p);
    }
  else _p->nref--;
}

void gnufdisk_partition_set_parameter(struct gnufdisk_partition* _p,
				      struct gnufdisk_string* _param,
				      const void* _data,
				      size_t _size)
{
  check_partition(&_p);

  if(gnufdisk_check_memory(_param, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERPOINTER, NULL, "invalid struct gnufdisk_string* %p", _param);
  else if(gnufdisk_check_memory((void*)_data, _size, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERDATA, NULL, "invalid parameter data: %p", _data);
  else if(_p->operations.set_parameter == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `set_parameter'");

  (*_p->operations.set_parameter)(_p->implementation_data, _param, _data, _size);
}

void gnufdisk_partition_get_parameter(struct gnufdisk_partition* _p,
				      struct gnufdisk_string* _param,
				      void* _data,
				      size_t _size)
{
  check_partition(&_p);

  if(gnufdisk_check_memory(_param, 1, 1) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERPOINTER, NULL, "invalid struct gnufdisk_string* %p", _param);
  else if(gnufdisk_check_memory(_data, _size, 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERDATA, NULL, "invalid parameter data: %p", _data);
  else if(_p->operations.get_parameter == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `get_parameter'");

  (*_p->operations.get_parameter)(_p->implementation_data, _param, _data, _size);
}

struct gnufdisk_string* gnufdisk_partition_type(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  if(_p->operations.type == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `type'");

  return (*_p->operations.type)(_p->implementation_data);
}

gnufdisk_integer gnufdisk_partition_start(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  if(_p->operations.start == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `start'");

  return (*_p->operations.start)(_p->implementation_data);
}

gnufdisk_integer gnufdisk_partition_length(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  if(_p->operations.length == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `length'");

  return (*_p->operations.length)(_p->implementation_data);
}

int gnufdisk_partition_number(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  if(_p->operations.number == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `number'");

  return (*_p->operations.number)(_p->implementation_data);
}

int gnufdisk_partition_have_disklabel(struct gnufdisk_partition* _p)
{
  check_partition(&_p);

  if(_p->operations.have_disklabel == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `have_disklabel'");

  return (*_p->operations.have_disklabel)(_p->implementation_data);
}

struct gnufdisk_disklabel * gnufdisk_partition_disklabel (struct gnufdisk_partition *_p)
{
  struct gnufdisk_disklabel_operations operations;
  void *implementation_data;
  struct gnufdisk_device* device;

  check_partition (&_p);

  if (_p->operations.disklabel == NULL)
    GNUFDISK_THROW (0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL,
		    "operation not supported `disklabel'");

  (*_p->operations.disklabel) (_p->implementation_data, &operations, &implementation_data);
  
  device = gnufdisk_device_internals__disklabel_get_device(_p->disklabel);

  return gnufdisk_device_internals__allocate_disklabel (device, &operations, implementation_data);
}

void gnufdisk_partition_move(struct gnufdisk_partition* _p, 
			     struct gnufdisk_geometry* _g)
{
  GNUFDISK_RETRY rp0;

  check_partition(&_p);

  GNUFDISK_RETRY_SET(rp0);

  if(gnufdisk_check_memory(_g, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.egeometrypointer = &_g;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_EGEOMETRYPOINTER, 
		     &data, "invalid struct gnufdisk_geometry* %p", _g);
    }
  else if(_p->operations.move == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `move'");

  (*_p->operations.move)(_p->implementation_data, _g);
}

void gnufdisk_partition_resize(struct gnufdisk_partition* _p,
			       struct gnufdisk_geometry* _e)
{
  GNUFDISK_RETRY rp0;

  check_partition(&_p);

  GNUFDISK_RETRY_SET(rp0);

  if(gnufdisk_check_memory(_e, 1, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.egeometrypointer = &_e;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_EGEOMETRYPOINTER,
		     &data, "invalid struct gnufdisk_geometry* %p", _e);
    }
  else if(_p->operations.resize == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `resize'");

  (*_p->operations.resize)(_p->implementation_data, _e);
}

int gnufdisk_partition_read(struct gnufdisk_partition* _p,
			    gnufdisk_integer _sector,
			    void* _dest,
			    size_t _size)
{
  GNUFDISK_RETRY rp0;

  check_partition(&_p);

  GNUFDISK_RETRY_SET(rp0);

  if(gnufdisk_check_memory(_dest, _size, 0) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.ereadbuffer = &_dest;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_EREADBUFFER, &data, "invalid read buffer %p", _dest);
    }
  else if(_p->operations.read == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `read'");

  return (*_p->operations.read)(_p->implementation_data, _sector, _dest, _size);
}

int gnufdisk_partition_write(struct gnufdisk_partition* _p,
			     gnufdisk_integer _sector,
			     const void* _data,
			     size_t _size)
{
  GNUFDISK_RETRY rp0;

  check_partition(&_p);

  GNUFDISK_RETRY_SET(rp0);

  if(gnufdisk_check_memory((void*)_data, _size, 1) != 0)
    {
      union gnufdisk_device_exception_data data;

      data.ewritebuffer = (void**) &_data;

      GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL, &rp0,
		     GNUFDISK_DEVICE_EWRITEBUFFER, &data, "invalid write buffer %p", _data);
    }
  else if(_p->operations.write == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOTSUP, NULL, "operation not supported `write'");

  return (*_p->operations.write)(_p->implementation_data, _sector, _data, _size);
}

