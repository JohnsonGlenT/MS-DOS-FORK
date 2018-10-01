/* GNU Fidsk (gnufdisk-devicemanager), a library to manage devices.
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
#ifndef GNUFDISK_DEVICEMANAGER_H_INCLUDED
#define GNUFDISK_DEVICEMANAGER_H_INCLUDED

#include <gnufdisk-common.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-device.h>

struct gnufdisk_devicemanager;
struct gnufdisk_userinterface;

struct gnufdisk_devicemanager *gnufdisk_devicemanager_new (struct
							   gnufdisk_userinterface
							   *_ui);

int gnufdisk_devicemanager_ref (struct gnufdisk_devicemanager *_dm);
int gnufdisk_devicemanager_delete (struct gnufdisk_devicemanager *_dm);

struct gnufdisk_geometry* 
gnufdisk_devicemanager_geometry_new(struct gnufdisk_devicemanager* _dm,
				    gnufdisk_integer _start,
				    gnufdisk_integer _length);

struct gnufdisk_geometry* 
gnufdisk_devicemanager_geometry_duplicate(struct gnufdisk_devicemanager* _dm,
                                          struct gnufdisk_geometry* _geom);

int gnufdisk_devicemanager_geometry_delete(struct gnufdisk_devicemanager* _dm,
                                           struct gnufdisk_geometry* _geom);

struct gnufdisk_geometry*
gnufdisk_devicemanager_geometry_set(struct gnufdisk_devicemanager* _dm,
                                    struct gnufdisk_geometry* _geom,
                                    gnufdisk_integer _start,
                                    gnufdisk_integer _end);

gnufdisk_integer 
gnufdisk_devicemanager_geometry_start(struct gnufdisk_devicemanager* _dm,
                                      struct gnufdisk_geometry* _geom);

gnufdisk_integer 
gnufdisk_devicemanager_geometry_end(struct gnufdisk_devicemanager* _dm,
                                      struct gnufdisk_geometry* _geom);

gnufdisk_integer 
gnufdisk_devicemanager_geometry_length(struct gnufdisk_devicemanager* _dm,
                                      struct gnufdisk_geometry* _geom);

struct gnufdisk_device *gnufdisk_devicemanager_device_new (struct
							   gnufdisk_devicemanager
							   *_dm,
							   struct
							   gnufdisk_string
							   *_module,
							   struct
							   gnufdisk_string
							   *_module_options);

int gnufdisk_devicemanager_device_open (struct gnufdisk_devicemanager *_dm,
					struct gnufdisk_device *_dev,
					struct gnufdisk_string *_path);

struct gnufdisk_disklabel *gnufdisk_devicemanager_device_disklabel (struct
								    gnufdisk_devicemanager
								    *_dm,
								    struct
								    gnufdisk_device
								    *_dev);

struct gnufdisk_disklabel
  *gnufdisk_devicemanager_device_create_disklabel (struct
						   gnufdisk_devicemanager
						   *_dm,
						   struct gnufdisk_device
						   *_dev,
						   struct gnufdisk_string
						   *_system);

int gnufdisk_devicemanager_device_set_parameter (struct gnufdisk_devicemanager
						 *_dm,
						 struct gnufdisk_device *_dev,
						 struct gnufdisk_string
						 *_param, const void *_data,
						 size_t _size);

int gnufdisk_devicemanager_device_get_parameter (struct gnufdisk_devicemanager
						 *_dm,
						 struct gnufdisk_device *_dev,
						 struct gnufdisk_string
						 *_param, void *_dest,
						 size_t _size);

int gnufdisk_devicemanager_device_commit (struct gnufdisk_devicemanager *_dm,
					  struct gnufdisk_device *_dev);

int gnufdisk_devicemanager_device_close (struct gnufdisk_devicemanager *_dm,
					 struct gnufdisk_device *_dev);

int gnufdisk_devicemanager_device_delete(struct gnufdisk_devicemanager* _dm, 
                                         struct gnufdisk_device* _dev);

int gnufdisk_devicemanager_disklabel_raw (struct gnufdisk_devicemanager *_dm,
					  struct gnufdisk_disklabel *_disk,
					  void **_dest, size_t * _size);

struct gnufdisk_string *gnufdisk_devicemanager_disklabel_system (struct
								 gnufdisk_devicemanager
								 *_dm,
								 struct
								 gnufdisk_disklabel
								 *_disk);

struct gnufdisk_partition *gnufdisk_devicemanager_disklabel_partition (struct
								       gnufdisk_devicemanager
								       *_dm,
								       struct
								       gnufdisk_disklabel
								       *_disk,
								       size_t
								       _number);
int 
gnufdisk_devicemanager_disklabel_count_partitions(struct gnufdisk_devicemanager* _dm,
                                                  struct gnufdisk_disklabel* _disk);

struct gnufdisk_partition
  *gnufdisk_devicemanager_disklabel_create_partition (struct
						      gnufdisk_devicemanager
						      *_dm,
						      struct
						      gnufdisk_disklabel
						      *_disk,
						      struct gnufdisk_geometry
						      *_start,
						      struct gnufdisk_geometry
						      *_end,
						      struct gnufdisk_string
						      *_type);

int gnufdisk_devicemanager_disklabel_remove_partition (struct
						       gnufdisk_devicemanager
						       *_dm,
						       struct
						       gnufdisk_disklabel
						       *_disk,
						       size_t _number);

int gnufdisk_devicemanager_disklabel_set_parameter (struct
						    gnufdisk_devicemanager
						    *_dm,
						    struct gnufdisk_disklabel
						    *_disk,
						    struct gnufdisk_string
						    *_param,
						    const void *_data,
						    size_t _size);

int gnufdisk_devicemanager_disklabel_get_parameter (struct
						    gnufdisk_devicemanager
						    *_dm,
						    struct gnufdisk_disklabel
						    *_disk,
						    struct gnufdisk_string
						    *_param, void *_dest,
						    size_t _size);

int gnufdisk_devicemanager_partition_set_parameter (struct
						    gnufdisk_devicemanager
						    *_dm,
						    struct gnufdisk_partition
						    *_part,
						    struct gnufdisk_string
						    *_param,
						    const void *_data,
						    size_t _size);

int gnufdisk_devicemanager_disklabel_delete(struct gnufdisk_devicemanager* _dm,
					    struct gnufdisk_disklabel* _disk);

int gnufdisk_devicemanager_partition_get_parameter (struct
						    gnufdisk_devicemanager
						    *_dm,
						    struct gnufdisk_partition
						    *_part,
						    struct gnufdisk_string
						    *_param, void *_dest,
						    size_t _size);

struct gnufdisk_string *gnufdisk_devicemanager_partition_type (struct
							       gnufdisk_devicemanager
							       *_dm,
							       struct
							       gnufdisk_partition
							       *_part);

gnufdisk_integer
gnufdisk_devicemanager_partition_start(struct gnufdisk_devicemanager* _dm,
                                       struct gnufdisk_partition* _part);

gnufdisk_integer
gnufdisk_devicemanager_partition_length(struct gnufdisk_devicemanager* _dm,
                                        struct gnufdisk_partition* _part);

int gnufdisk_devicemanager_partition_number (struct gnufdisk_devicemanager
					     *_dm,
					     struct gnufdisk_partition
					     *_part);

int gnufdisk_devicemanager_partition_have_disklabel(struct gnufdisk_devicemanager* _dm,
                                                    struct gnufdisk_partition* _part);

struct gnufdisk_disklabel* 
gnufdisk_devicemanager_partition_disklabel(struct gnufdisk_devicemanager* _dm,
                                           struct gnufdisk_partition* _part);

int gnufdisk_devicemanager_partition_move (struct gnufdisk_devicemanager *_dm,
					   struct gnufdisk_partition *_part,
					   struct gnufdisk_geometry *_range);

int gnufdisk_devicemanager_partition_resize (struct gnufdisk_devicemanager
					     *_dm,
					     struct gnufdisk_partition *_part,
					     struct gnufdisk_geometry
					     *_range);

int gnufdisk_devicemanager_partition_read (struct gnufdisk_devicemanager *_dm,
					   struct gnufdisk_partition *_part,
					   gnufdisk_integer _start,
					   void *_buf, size_t _size);

int gnufdisk_devicemanager_partition_write (struct gnufdisk_devicemanager
					    *_dm,
					    struct gnufdisk_partition *_part,
					    gnufdisk_integer _start,
					    const void *_buf, size_t _size);

int gnufdisk_devicemanager_partition_delete(struct gnufdisk_devicemanager* _dm,
                                            struct gnufdisk_partition* _part);

#endif /* GNUFDISK_DEVICEMANAGER_H_INCLUDED */

