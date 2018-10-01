
/* GNU Fidsk (gnufdisk-userinterface), a library to manage guile userinterface.
 *
 * Copyright (C) 2010 Free Software Foundation
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

#ifndef GNUFDISK_USERINTERFACE_INTERNALS_H_INCLUDED
#define GNUFDISK_USERINTERFACE_INTERNALS_H_INCLUDED

#include <gnufdisk-userinterface.h>

#include <libguile.h>

struct gnufdisk_userinterface 
{
  SCM print;
  SCM error;
  SCM yes_no;
  SCM get_path;
  SCM get_disklabel_system;
  SCM get_geometry;
  SCM get_partition_type;
  int shell_mode;
  int nref;
};

int gnufdisk_userinterface_internals__run(struct gnufdisk_userinterface* _ui,
					  struct gnufdisk_string* _implementation,
					  int _argc,
					  char** _argv);

int gnufdisk_userinterface_internals__print(struct gnufdisk_userinterface* _ui,
					    const char* _fmt, va_list _args);

int gnufdisk_userinterface_internals__error(struct gnufdisk_userinterface* _ui,
					    const char* _fmt, va_list _args);

int gnufdisk_userinterface_internals__yes_no(struct gnufdisk_userinterface* _ui,
					    const char* _fmt, va_list _args);

struct gnufdisk_string *
gnufdisk_userinterface_internals__get_path(struct gnufdisk_userinterface* _ui,
			                   const char* _fmt, va_list _args);

struct gnufdisk_string *
gnufdisk_userinterface_internals__get_disklabel_system(struct gnufdisk_userinterface* _ui,
			                               const char* _fmt, va_list _args);

int
gnufdisk_userinterface_internals__get_geometry(struct gnufdisk_userinterface* _ui,
					       struct gnufdisk_devicemanager* _dm,
					       struct gnufdisk_geometry* _geom,
			                       const char* _fmt, va_list _args);

struct gnufdisk_string *
gnufdisk_userinterface_internals__get_partition_type(struct gnufdisk_userinterface* _ui,
			                             const char* _fmt, va_list _args);

#endif /* GNUFDISK_USERINTERFACE_INTERNALS_H_INCLUDED */

