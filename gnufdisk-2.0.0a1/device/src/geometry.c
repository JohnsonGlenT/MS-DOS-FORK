
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

#include <stdlib.h>

#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-device.h>

#define GEOMETRY (getenv("GNUFDISK_GEOMETRY") != NULL)

struct gnufdisk_geometry {
  gnufdisk_integer start;
  gnufdisk_integer end;
};

static void
check_geometry (struct gnufdisk_geometry **_g)
{
  GNUFDISK_RETRY rp0;
  union gnufdisk_device_exception_data data;

  GNUFDISK_RETRY_SET (rp0);

  if (gnufdisk_check_memory (*_g, sizeof (struct gnufdisk_geometry), 0) != 0)
    {
      data.egeometrypointer = _g;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EGEOMETRYPOINTER, &data,
		      "invalid struct gnufdisk_geometry* %p", *_g);
    }
}

static void
free_pointer (void *_p)
{
  GNUFDISK_LOG ((GEOMETRY, "free ponter %p", _p));
  free(_p);
}

struct gnufdisk_geometry * gnufdisk_geometry_new (gnufdisk_integer _start,
                                                  gnufdisk_integer _length)
{
  struct gnufdisk_geometry *g;
  GNUFDISK_RETRY rp0;

  GNUFDISK_RETRY_SET (rp0);

  if((g = malloc(sizeof (struct gnufdisk_geometry))) == NULL)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_ENOMEM, NULL, "cannot allocate memory");

  if(gnufdisk_exception_register_unwind_handler (&free_pointer, g) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_register_unwind_handler failed. Missing GNUFDISK_TRY?");

  memset(g, 0, sizeof(struct gnufdisk_geometry));

  g->start = _start;
  g->end = _start + _length;

  if(gnufdisk_exception_unregister_unwind_handler(&free_pointer, g) != 0)
    GNUFDISK_WARNING("gnufdisk_exception_unregister_unwind_handler failed. Missing GNUFDISK_TRY?");

  return g;
}

struct gnufdisk_geometry* 
gnufdisk_geometry_duplicate(struct gnufdisk_geometry* _g)
{
  struct gnufdisk_geometry* ret;

  check_geometry(&_g);

  ret = gnufdisk_geometry_new(0, 0);

  ret->start = _g->start;
  ret->end = _g->end;

  return ret;
}

struct gnufdisk_geometry *
gnufdisk_geometry_set (struct gnufdisk_geometry *_g,
		       gnufdisk_integer _s, gnufdisk_integer _l)
{
  GNUFDISK_RETRY rp0;

  check_geometry (&_g);

  GNUFDISK_RETRY_SET (rp0);

  if (_l <= 0) /* FIXME: check device size overlap ? */
    {
      union gnufdisk_device_exception_data data;

      data.egeometrylength = &_l;

      GNUFDISK_THROW (GNUFDISK_EXCEPTION_ALL, &rp0,
		      GNUFDISK_DEVICE_EGEOMETRYLENGTH, &data,
		      "invalid length: %lld", _l);
    }

  _g->start = _s;
  _g->end = _s + _l;

  return _g;
}

gnufdisk_integer
gnufdisk_geometry_start (struct gnufdisk_geometry *_g)
{
  check_geometry (&_g);

  return _g->start;
}

gnufdisk_integer
gnufdisk_geometry_end (struct gnufdisk_geometry *_g)
{
  check_geometry (&_g);

  return _g->end;
}

gnufdisk_integer
gnufdisk_geometry_length (struct gnufdisk_geometry *_g)
{
  check_geometry (&_g);

  return (_g->end - _g->start) + 1;
}

void
gnufdisk_geometry_delete (struct gnufdisk_geometry *_g)
{
  check_geometry (&_g);

  memset(_g, 0, sizeof(struct gnufdisk_geometry));

  free (_g);
}

