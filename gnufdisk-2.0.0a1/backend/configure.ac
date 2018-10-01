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

AC_PREREQ([2.65])
AC_INIT([gnufdisk-backend], [0.1.0], [bug-fdisk@gnu.org])
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([partition.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([blkid], [blkid_new_probe], [ ], AC_MSG_ERROR([[libblkid is missing, please install it and try again.]]))
AC_CHECK_LIB([uuid], [uuid_generate], [ ], AC_MSG_ERROR([[libuuid is missing, pleas install it and try again.]]))

# Checks for header files.
AC_CHECK_HEADERS([gnufdisk-common.h gnufdisk-debug.h gnufdisk-device.h gnufdisk-device-internals.h], 
    [], AC_MSG_ERROR([[GNU Fdisk headers are missing, install headers and try again.]]))

AC_CHECK_HEADERS([fcntl.h inttypes.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_HEADER_MAJOR
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strcasecmp strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
