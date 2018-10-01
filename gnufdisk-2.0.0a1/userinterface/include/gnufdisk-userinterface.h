
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

#ifndef GNUFDISK_USERINTERFACE_H_INCLUDED
#define GNUFDISK_USERINTERFACE_H_INCLUDED

struct gnufdisk_userinterface;
struct gnufdisk_devicemanager;
struct gnufdisk_geometry;

struct gnufdisk_userinterface* gnufdisk_userinterface_new(void);

int gnufdisk_userinterface_ref(struct gnufdisk_userinterface* _ui);

int gnufdisk_userinterface_delete(struct gnufdisk_userinterface* _ui);

int gnufdisk_userinterface_run(struct gnufdisk_userinterface* _ui, 
			       struct gnufdisk_string* _implementation,
			       int _argc, 
			       char** _argv);

int gnufdisk_userinterface_print(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...);

int gnufdisk_userinterface_error(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...);

int gnufdisk_userinterface_yes_no(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...);

struct gnufdisk_string*
gnufdisk_userinterface_get_path(struct gnufdisk_userinterface* _ui,
                                 const char* _fmt, ...);

struct gnufdisk_string*
gnufdisk_userinterface_get_disklabel_system(struct gnufdisk_userinterface* _ui,
                                            const char* _fmt, ...);

int gnufdisk_userinterface_get_geometry(struct gnufdisk_userinterface* _ui,
                                        struct gnufdisk_devicemanager* _dm,
                                        struct gnufdisk_geometry* _geom,
                                        const char* _fmt, ...);

struct gnufdisk_string*
gnufdisk_userinterface_get_partition_type(struct gnufdisk_userinterface* _ui,
                                            const char* _fmt, ...);

#endif /* GNUFDISK_USERINTERFACE_H_INCLUDED */
