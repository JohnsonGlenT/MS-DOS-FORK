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
AC_INIT([gnufdisk-userinterface], [2.0.0], [bug-fdisk@gnu.org])
AC_CONFIG_SRCDIR([include/gnufdisk-userinterface.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([autoconf])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([src/config.h])

LT_INIT()
# Checks for programs.

AC_PROG_CC
AC_PROG_CC_C_O

GUILE_PROGS
GUILE_FLAGS

AC_CHECK_HEADER(libguile.h, [], [AC_MSG_ERROR(["libguile.h not found, please install libguile and try again."])])
AC_CHECK_LIB(guile, scm_with_guile, [], [AC_MSG_ERROR("scm_with_guile is not available in your GUILE implementation.")], [])

# Checks for libraries.
for FUNC in scm_to_locale_string scm_dynwind_begin \
      scm_dynwind_unwind_handler scm_dynwind_end scm_calloc scm_wrong_type_arg \
      scm_gc_mark scm_c_write scm_make_stack scm_error scm_current_output_port \
      scm_is_integer scm_is_string scm_from_locale_symbol scm_malloc \
      scm_dynwind_free scm_with_guile scm_c_define_gsubr \
      scm_c_define scm_append scm_list_2  scm_list_1 \
      scm_to_long_long scm_from_long_long scm_to_size_t; do
  AC_CHECK_DECL($FUNC, [], [AC_MSG_ERROR("SFUNC is not declared in your libguile.h")], [#include <libguile.h>])
done

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
