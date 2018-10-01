/* GNU Fidsk (gnufdisk-common), a library of common functions.
 *
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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <gnufdisk-common.h>

struct gnufdisk_stack {
  void* data;
  int size;
  int index;
};

struct gnufdisk_stack* gnufdisk_stack_new(void)
{
  struct gnufdisk_stack* ret;
  int err;

  if((ret = malloc(sizeof(struct gnufdisk_stack))) == NULL)
    {
      err = ENOMEM;
      goto lb_out;
    }
  
  memset(ret, 0, sizeof(struct gnufdisk_stack));

lb_out:

  errno = err;
  return ret;
}

int gnufdisk_stack_delete(struct gnufdisk_stack* _s)
{
  int ret;
  int err;

  if(gnufdisk_check_memory(_s, sizeof(struct gnufdisk_stack), 0) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if(_s->data)
    {
      if(gnufdisk_check_memory(_s->data, _s->size, 0) != 0)
	{
	  err = errno;
	  ret = -1;
	  goto lb_out;
	}

      free(_s->data);
      _s->data = NULL;
    }

  free(_s);

  err = 0;
  ret = 0;

lb_out:

  errno = err;
  return ret;
}

int gnufdisk_stack_push(struct gnufdisk_stack* _s, void* _data, size_t _size)
{
  int ret;
  int err;

  if(gnufdisk_check_memory(_s, sizeof(struct gnufdisk_stack), 0) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }
  else if(gnufdisk_check_memory(_data, _size, 1) != 0)
    {
      err = EFAULT;
      ret = -1;
      goto lb_out;
    }

  if(_size > _s->size - _s->index)
    {
      void* data;
      int size;

      size = _s->size + 32;
      if((data = malloc(size)) == NULL)
	{
	  err = ENOMEM;
	  ret = -1;
	  goto lb_out;
	}

      if(_s->data)
	{
	  memcpy(data, _s->data, _s->index);
	  free(_s->data);
	}

      _s->data = data;
      _s->size = size;

      return gnufdisk_stack_push(_s, _data, _size);
    }

  memcpy(_s->data + _s->index, _data, _size);
 _s->index += _size;

 err = 0;
 ret = 0;

lb_out:
  
  errno = err;
  return ret; 
}

int gnufdisk_stack_pop(struct gnufdisk_stack* _s, void* _dest, size_t _size)
{
  int ret;
  int err;

  if(gnufdisk_check_memory(_s, sizeof(struct gnufdisk_stack), 0) != 0
     || (_s->data && gnufdisk_check_memory(_s->data, _s->size, 0) != 0)
     || gnufdisk_check_memory(_dest, _size, 0) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }
  else if(_size > _s->index)
    {
      err = ENOBUFS;
      ret = -1;
      goto lb_out;
    }

  memcpy(_dest, _s->data + (_s->index - _size), _size);
  _s->index -= _size;

  ret = 0;
  err = 0;

lb_out:

  errno = err;
  return ret;
}

