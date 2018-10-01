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
#ifndef GNUFDISK_COMMON_H_INCLUDED
#define GNUFDISK_COMMON_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

int gnufdisk_check_memory(void* _p, size_t _len, int _readonly);

/* same as vfprintf and vasprintf, but with SIGSEGV check */
int gnufdisk_vfprintf(FILE* _f, const char* _fmt, va_list _args);
int gnufdisk_vasprintf(char** _dest, const char* _fmt, va_list _args);

struct gnufdisk_string;
struct gnufdisk_string* gnufdisk_string_new(const char* _fmt, ...);
int gnufdisk_string_set(struct gnufdisk_string* _p, const char* _fmt, ...);
int gnufdisk_string_length(struct gnufdisk_string* _p);
const char* gnufdisk_string_c_string(struct gnufdisk_string* _p);
char* gnufdisk_string_c_string_dup(struct gnufdisk_string* _p);
int gnufdisk_string_delete(struct gnufdisk_string* _s);

struct gnufdisk_stack;
struct gnufdisk_stack* gnufdisk_stack_new(void);
int gnufdisk_stack_delete(struct gnufdisk_stack* _s);
int gnufdisk_stack_push(struct gnufdisk_stack* _s, void* _data, size_t _size);
int gnufdisk_stack_pop(struct gnufdisk_stack* _s, void* _dest, size_t _size);

#endif /* GNUFDISK_COMMON_H_INCLUDED */

