
/* GNU Fidsk (gnufdisk-userinterface), a library to manage guile userinterface.
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

#include <errno.h>
#include <stdlib.h>

#include <gnufdisk-common.h>
#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-device.h>
#include <gnufdisk-devicemanager.h>
#include <gnufdisk-userinterface.h>

#include "internals.h"

#if HAVE_CONFIG_H
# include "config.h"
#endif

#define UI (getenv("GNUFDISK_USERINTERFACE") != NULL)

struct gnufdisk_userinterface*
gnufdisk_userinterface_new(void)
{
  struct gnufdisk_userinterface* ret;

  ret = NULL;

  GNUFDISK_TRY(NULL, NULL)
    {
      if((ret = malloc(sizeof(struct gnufdisk_userinterface))) == NULL)
        GNUFDISK_THROW(0, NULL, ENOMEM, NULL, "cannot allocate memory");

      if(gnufdisk_exception_register_unwind_handler(&free, ret) != 0)
        GNUFDISK_WARNING("gnufdisk_register_unwind_handler failed.");

      memset(ret, 0, sizeof(struct gnufdisk_userinterface));

      ret->nref = 1;

      if(gnufdisk_exception_unregister_unwind_handler(&free, ret) != 0)
        GNUFDISK_WARNING("gnufdisk_unregister_unwind_handler failed.");
    }
  GNUFDISK_CATCH_DEFAULT  
    {
      ret = NULL;
    }
  GNUFDISK_EXCEPTION_END;

  return ret;
}

int gnufdisk_userinterface_ref(struct gnufdisk_userinterface* _ui)
{
  int ret;

  ret = 0;

  GNUFDISK_TRY(NULL, NULL)
    {
      if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
        GNUFDISK_THROW(0, NULL, EFAULT, NULL, "invalid struct gnufdisk_userinterface* %p");

      _ui->nref++;
      ret = 0;
    }
  GNUFDISK_CATCH_DEFAULT
    {
      ret = -1;
    }
  GNUFDISK_EXCEPTION_END;

  return ret;
}

int gnufdisk_userinterface_delete(struct gnufdisk_userinterface* _ui)
{
  int ret;
  int err;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  free(_ui);
  ret = 0;
  err = 0;

lb_out:

  errno = err;
  return ret;
}



int gnufdisk_userinterface_run(struct gnufdisk_userinterface* _ui,
                               struct gnufdisk_string* _implementation,
                               int _argc,
                               char** _argv)
{
  int ret;
  int err;  

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0
     || gnufdisk_check_memory(_implementation, 1, 1) != 0
     || gnufdisk_check_memory(_argv, sizeof(char*) * _argc, 1) != 0)
    {
      err = EFAULT;
      ret = -1;
      goto lb_out;
    }

  ret = gnufdisk_userinterface_internals__run(_ui, _implementation, _argc, _argv);
  err = errno;

lb_out:

  errno = err;
  return ret;
}

int gnufdisk_userinterface_print(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...)
{
  int ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = EFAULT;
      ret = -1;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__print(_ui, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

int gnufdisk_userinterface_error(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...)
{
  int ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = EFAULT;
      ret = -1;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__error(_ui, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

int gnufdisk_userinterface_yes_no(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...)
{
  int ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = EFAULT;
      ret = -1;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__yes_no(_ui, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

struct gnufdisk_string*
gnufdisk_userinterface_get_path(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...)
{
  struct gnufdisk_string* ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = EFAULT;
      ret = NULL;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__get_path(_ui, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

struct gnufdisk_string*
gnufdisk_userinterface_get_disklabel_system(struct gnufdisk_userinterface* _ui,
                                            const char* _fmt, ...)
{
  struct gnufdisk_string* ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = EFAULT;
      ret = NULL;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__get_disklabel_system(_ui, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

int gnufdisk_userinterface_get_geometry(struct gnufdisk_userinterface* _ui,
                                        struct gnufdisk_devicemanager* _dm,
                                        struct gnufdisk_geometry* _geom,
                                        const char* _fmt, ...)
{
  int ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0
     || gnufdisk_check_memory(_dm, 1, 1) != 0
     || gnufdisk_check_memory(_geom, 1, 1) != 0)
    {
      err = EFAULT;
      ret = -1;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__get_geometry(_ui, _dm, _geom, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

struct gnufdisk_string*
gnufdisk_userinterface_get_partition_type(struct gnufdisk_userinterface* _ui,
                                            const char* _fmt, ...)
{
  struct gnufdisk_string* ret;
  int err;
  va_list args;

  if(gnufdisk_check_memory(_ui, sizeof(struct gnufdisk_userinterface), 0) != 0)
    {
      err = EFAULT;
      ret = NULL;
      goto lb_out;
    }

  va_start(args, _fmt);
  ret = gnufdisk_userinterface_internals__get_partition_type(_ui, _fmt, args);
  err = errno;
  va_end(args);

lb_out:

  errno = err;
  return ret;
}

