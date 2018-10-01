#                                               -*- Autoconf -*-
#
# GNU fdisk - Partitioning software.
# 
# Copyright (C) 2010 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([gnufdisk-exception], [2.0.0], [bug-fdisk@gnu.org])
AC_CONFIG_SRCDIR([src/exception.c])
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE
LT_INIT()

AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([setjmp.h stdlib.h])

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([vsnprintf])

# Checks for typedefs, structures, and compiler characteristics.
have_jmpbuf=0
AC_MSG_CHECKING([wether jmp_buf works])
AC_RUN_IFELSE(AC_LANG_PROGRAM(
[[
	#if HAVE_STDLIB_H
	#include <stdlib.h>
	#endif 
	#if HAVE_SETJMP_H
	#include <setjmp.h>
	#endif
	static void jumpto(jmp_buf* _ptr) { longjmp(*_ptr, 1); }
]],[[
	int error; jmp_buf jumper1, jumper2;

	if((error = setjmp(jumper1)) == 0)
		{
			if((error = setjmp(jumper2)) == 0)
				jumpto(&jumper2);
			else
				jumpto(&jumper1);					
		}
	else
		return EXIT_SUCCESS;
	return EXIT_FAILURE;]]), 
	[AC_MSG_RESULT([yes]); have_jmpbuf=1], 
	[AC_MSG_RESULT([no]); have_jmbuf=0], 
	[AC_MSG_RESULT([Can't check becouse we are cross compiling]); have_jmpbuf=0])
if test $have_jmpbuf -eq 0; then
	AC_MSG_ERROR([jmp_buf seem to be unsupported, please check your C library and try again])
fi

# Output
AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
