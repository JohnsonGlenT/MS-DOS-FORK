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
AC_INIT([gnufdisk], [2.0.0a1], [bug-fdisk@gnu.org])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([autoconf])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/gnufdisk.c])
AC_CONFIG_SUBDIRS([common debug exception device backend devicemanager userinterface])

LT_INIT()
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile doc/Makefile src/Makefile])
AC_OUTPUT
