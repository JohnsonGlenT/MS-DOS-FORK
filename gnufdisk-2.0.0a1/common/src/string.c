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

struct gnufdisk_string {
  char* data;
  size_t size;
};

static int check_string(struct gnufdisk_string* _s)
{
  if(gnufdisk_check_memory(_s, sizeof(struct gnufdisk_string), 0) != 0
     || (_s->data != NULL && gnufdisk_check_memory(_s->data, _s->size, 0) != 0))
    return EFAULT;
  return 0;
}

struct gnufdisk_string* gnufdisk_string_new(const char* _fmt, ...)
{
  va_list args;
  struct gnufdisk_string* ret;
  int err;

  if((ret = malloc(sizeof(struct gnufdisk_string))) == NULL)
    {
      err = errno;
      goto lb_out;
    }

  memset(ret, 0, sizeof(struct gnufdisk_string));
    
  va_start(args, _fmt);
  ret->size = gnufdisk_vasprintf(&ret->data, _fmt, args);
  va_end(args);

  if(ret->size < 0)
    {
      err = errno;
      
      free(ret);
      ret = NULL;
    }

lb_out:
  
  errno = err;
  return ret;
}

int gnufdisk_string_set(struct gnufdisk_string* _p, const char* _fmt, ...)
{
  va_list args;
  char* s;
  int err;
  int ret;

  if((err = check_string(_p)) != 0)
    {
      ret = -1;
      goto lb_out;
    }
  
  va_start(args, _fmt);
  ret = gnufdisk_vasprintf(&s, _fmt, args);
  va_end(args);

  if(ret < 0)
    {
      ret = -1;
      err = errno;
      goto lb_out;
    }
  else 
    err = 0;

  if(_p->data)
    free(_p->data);

  _p->data = s;
  _p->size = ret;

lb_out:
  
  errno = err;
  return ret;
}

int gnufdisk_string_length(struct gnufdisk_string* _p)
{
  if((errno = check_string(_p)) != 0)
    return -1;

  return strlen(_p->data != NULL ? _p->data : "");
}

const char* gnufdisk_string_c_string(struct gnufdisk_string* _p)
{
  if((errno = check_string(_p)) != 0)
    return NULL;

  return _p->data;
}

char* gnufdisk_string_c_string_dup(struct gnufdisk_string* _p)
{
  char *r;

  if((errno = check_string(_p)) != 0)
    return NULL;

  r = strdup(_p->data ? _p->data : "");

  if(r == NULL)
    errno = ENOMEM;

  return r;
}

int gnufdisk_string_delete(struct gnufdisk_string* _s)
{
  if((errno = check_string(_s)) != 0)
    return -1;

  if(_s->data)
    free(_s->data);

  free(_s);

  return 0;
}

