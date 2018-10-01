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

#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <gnufdisk-common.h>
#include <gnufdisk-debug.h>

static int hexdump(FILE* _stream, const unsigned char* _data, const int _size)
{
  int ret;
  int iter;

  for(ret = 0, iter = 0; iter < _size; iter++)
    ret += fprintf(_stream, "%02x", _data[iter]);

  return ret;
}

static int print_info(FILE* _stream, const char* _file, const int _line)
{
  int ret;
  int pid;
  pthread_t thread;

  ret = 0;
  pid = getpid();
  thread = pthread_self();

  ret += fprintf(_stream, "%d-", getpid());
  ret += hexdump(_stream, (unsigned char *) &thread, sizeof(pthread_t));
  ret += fprintf(_stream, ":%s:%d: ", _file, _line);

  return ret;
}

int
gnufdisk_log_implementation (int _category, const char *_file,
			     const int _line, const char *_format, ...)
{
  if (_category)
    {
      int ret;
      va_list args;

      ret = 0;

      ret += print_info (stdout, _file, _line);

      va_start (args, _format);
      ret += gnufdisk_vfprintf (stdout, _format, args);
      va_end (args);

      fputc ('\n', stdout);

      return ret + 1;
    }

  return 0;
}

int
gnufdisk_warning_implementation (const char *_file, const int _line, const char *_format, ...)
{
  int ret;
  va_list args;

  ret = 0;

  ret += fprintf(stderr, "*** WARNING *** ");
  ret += print_info (stderr, _file, _line);

  va_start (args, _format);
  ret += gnufdisk_vfprintf (stderr, _format, args);
  va_end (args);

  fputc ('\n', stderr);

  return ret + 1;

  return 0;
}
