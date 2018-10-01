
/* GNU Fidsk (gnufdisk-debug), a library for debugging.
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

#ifndef GNUFDISK_DEBUG_H_INCLUDED
#define GNUFDISK_DEBUG_H_INCLUDED

extern int gnufdisk_log_implementation(int _category, const char* _file, const int _line, const char* _format, ...);
extern int gnufdisk_warning_implementation (const char *_file, const int _line, const char *_format, ...);

#if defined(GNUFDISK_DEBUG)
# define GNUFDISK_LOG_IMPLEMENTATION(_mp_cat, _mp_args...) gnufdisk_log_implementation(_mp_cat, __FILE__, __LINE__, _mp_args)
# define GNUFDISK_LOG(_mp_args) GNUFDISK_LOG_IMPLEMENTATION _mp_args 
#else
# define GNUFDISK_LOG(_mp_args) (void) 0
#endif /* GNUFDISK_DEBUG */

#define GNUFDISK_WARNING(_mp_args...) gnufdisk_warning_implementation(__FILE__, __LINE__, _mp_args)

#endif /* GNUFDISK_DEBUG_H_INCLUDED */


