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

# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([gnufdisk-device], [2.0.0a], [bug-fdisk@gnu.org])
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([include/gnufdisk-device.h.in])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
LT_INIT()

AC_PROG_CC
AM_PROG_CC_C_O

# Checks for header files.
AC_CHECK_HEADERS([stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_C_CONST
AC_TYPE_LONG_LONG_INT

if test "x$ac_cv_type_long_long_int" = "xyes" ; then
  WIDE_INTEGER='long long int'
else
  WIDE_INTEGER='long int'
fi

AC_SUBST(WIDE_INTEGER)

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile include/gnufdisk-device.h])
AC_OUTPUT
