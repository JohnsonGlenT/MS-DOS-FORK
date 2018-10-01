
/* GNU fdisk, (gnufdisk-device) a library to manage a device
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

#ifndef GNUFDISK_DEVICE_H_INCLUDED
#define GNUFDISK_DEVICE_H_INCLUDED

#include <sys/types.h>
#include <gnufdisk-common.h>

typedef long long int gnufdisk_integer;

struct gnufdisk_chs {
  gnufdisk_integer cylinder;
  gnufdisk_integer head;
  gnufdisk_integer sector;
};

struct gnufdisk_partition;
struct gnufdisk_label;
struct gnufdisk_device;
struct gnufdisk_geometry;

struct gnufdisk_geometry * gnufdisk_geometry_new (gnufdisk_integer _start,
                                                  gnufdisk_integer _length);
struct gnufdisk_geometry* gnufdisk_geometry_duplicate(struct gnufdisk_geometry* _g);
struct gnufdisk_geometry* gnufdisk_geometry_set (struct gnufdisk_geometry *_geom, gnufdisk_integer _start, gnufdisk_integer _length);
struct gnufdisk_geometry * gnufdisk_geometry_set_chs (struct gnufdisk_geometry *_g, struct gnufdisk_chs _s, struct gnufdisk_chs _l);
gnufdisk_integer gnufdisk_geometry_start(struct gnufdisk_geometry*);
struct gnufdisk_chs gnufdisk_geometry_start_chs(struct gnufdisk_geometry*);

gnufdisk_integer gnufdisk_geometry_end(struct gnufdisk_geometry*);
struct gnufdisk_chs gnufdisk_geometry_end_chs(struct gnufdisk_geometry*);

gnufdisk_integer gnufdisk_geometry_length(struct gnufdisk_geometry*);
struct gnufdisk_chs gnufdisk_geometry_length_chs(struct gnufdisk_geometry*);
void gnufdisk_geometry_delete(struct gnufdisk_geometry* _g);

struct gnufdisk_device* gnufdisk_device_new(struct gnufdisk_string* _module, 
                                            struct gnufdisk_string* _options);
void gnufdisk_device_delete(struct gnufdisk_device* _d);
void gnufdisk_device_ref(struct gnufdisk_device* _d);
void gnufdisk_device_open(struct gnufdisk_device* _d, struct gnufdisk_string* _path);
struct gnufdisk_disklabel* gnufdisk_device_disklabel(struct gnufdisk_device* _d);
struct gnufdisk_disklabel* gnufdisk_device_create_disklabel(struct gnufdisk_device* _d, struct gnufdisk_string* _type);
void gnufdisk_device_set_parameter(struct gnufdisk_device* _d, struct gnufdisk_string* _parameter, const void* _data, size_t _size);
void gnufdisk_device_get_parameter(struct gnufdisk_device* _d, struct gnufdisk_string* _parameter, void* _data, size_t _size);
void gnufdisk_device_commit(struct gnufdisk_device* _d);
void gnufdisk_device_close(struct gnufdisk_device* _d);

void gnufdisk_disklabel_delete (struct gnufdisk_disklabel *_d);
void gnufdisk_disklabel_ref (struct gnufdisk_disklabel *_d);
void gnufdisk_disklabel_raw (struct gnufdisk_disklabel *_d, void **_dest, size_t * _size);
struct gnufdisk_string* gnufdisk_disklabel_system(struct gnufdisk_disklabel* _d);
int gnufdisk_disklabel_count_partitions(struct gnufdisk_disklabel* _d);
struct gnufdisk_partition* gnufdisk_disklabel_partition(struct gnufdisk_disklabel* _d, size_t _n);
struct gnufdisk_partition* gnufdisk_disklabel_create_partition(struct gnufdisk_disklabel* _d,
							       struct gnufdisk_geometry* _s,
							       struct gnufdisk_geometry* _e,
							       struct gnufdisk_string* _type);
void gnufdisk_disklabel_remove_partition(struct gnufdisk_disklabel* _d, size_t _n);
void gnufdisk_disklabel_set_parameter(struct gnufdisk_disklabel* _d,
				      struct gnufdisk_string* _parameter,
				      const void* _data,
				      size_t _size);
void gnufdisk_disklabel_get_parameter(struct gnufdisk_disklabel* _d,
				      struct gnufdisk_string* _parameter,
				      void* _data,
				      size_t _size);

void gnufdisk_partition_ref(struct gnufdisk_partition* _p);
void gnufdisk_partition_delete(struct gnufdisk_partition* _p);
void gnufdisk_partition_set_parameter(struct gnufdisk_partition* _p,
				      struct gnufdisk_string* _param,
				      const void* _data,
				      size_t _size);
void gnufdisk_partition_get_parameter(struct gnufdisk_partition* _p,
				      struct gnufdisk_string* _param,
				      void* _data,
				      size_t _size);
struct gnufdisk_string* gnufdisk_partition_type(struct gnufdisk_partition* _p);
gnufdisk_integer gnufdisk_partition_start(struct gnufdisk_partition* _p);
gnufdisk_integer gnufdisk_partition_length(struct gnufdisk_partition* _p);
int gnufdisk_partition_have_disklabel(struct gnufdisk_partition* _p);
struct gnufdisk_disklabel * gnufdisk_partition_disklabel (struct gnufdisk_partition *_p);
int gnufdisk_partition_number(struct gnufdisk_partition* _p);
void gnufdisk_partition_move(struct gnufdisk_partition* _p, 
			     struct gnufdisk_geometry* _g);
void gnufdisk_partition_resize(struct gnufdisk_partition* _p,
			       struct gnufdisk_geometry* _e);
int gnufdisk_partition_read(struct gnufdisk_partition* _p,
			    gnufdisk_integer _sector,
			    void* _dest,
			    size_t _size);
int gnufdisk_partition_write(struct gnufdisk_partition* _p,
			     gnufdisk_integer _sector,
			     const void* _data,
			     size_t _size);

enum gnufdisk_device_error {
  GNUFDISK_DEVICE_EMODULEPOINTER = 1000,
  GNUFDISK_DEVICE_EMODULE,
  GNUFDISK_DEVICE_EGEOMETRYLENGTH,
  GNUFDISK_DEVICE_EGEOMETRYPOINTER,
  GNUFDISK_DEVICE_EGEOMETRY,
  GNUFDISK_DEVICE_EDEVICEPOINTER,
  GNUFDISK_DEVICE_EDEVICE,
  GNUFDISK_DEVICE_EDISKLABELPOINTER,
  GNUFDISK_DEVICE_EDISKLABEL,
  GNUFDISK_DEVICE_EPARTITIONPOINTER,
  GNUFDISK_DEVICE_EPARTITION,
  GNUFDISK_DEVICE_EPARTITIONNUMBER,
  GNUFDISK_DEVICE_ENOTSUP,
  GNUFDISK_DEVICE_EPATHPOINTER,
  GNUFDISK_DEVICE_EPATH,
  GNUFDISK_DEVICE_EDESTINATIONPOINTER,
  GNUFDISK_DEVICE_ESIZEPOINTER,
  GNUFDISK_DEVICE_EDISKLABELSYSTEMPOINTER,
  GNUFDISK_DEVICE_EDISKLABELSYSTEM,
  GNUFDISK_DEVICE_EDISKLABELFULL,
  GNUFDISK_DEVICE_EPARTITIONTYPEPOINTER,
  GNUFDISK_DEVICE_EPARTITIONTYPE,
  GNUFDISK_DEVICE_EPARAMETERPOINTER,
  GNUFDISK_DEVICE_EPARAMETER,
  GNUFDISK_DEVICE_EPARAMETERDATA,
  GNUFDISK_DEVICE_EPARAMETERSIZE,
  GNUFDISK_DEVICE_EREADBUFFER,
  GNUFDISK_DEVICE_EWRITEBUFFER,
  GNUFDISK_DEVICE_EINTERNAL,
  GNUFDISK_DEVICE_ENOTOPEN,
  GNUFDISK_DEVICE_EIO,
  GNUFDISK_DEVICE_ECYLINDERS,
  GNUFDISK_DEVICE_EHEADS,
  GNUFDISK_DEVICE_ESECTORS,
  GNUFDISK_DEVICE_ESECTORSIZE,
  GNUFDISK_DEVICE_ENOMEM
};

union gnufdisk_device_exception_data {
  struct gnufdisk_string** emodulepointer;
  struct gnufdisk_string* emodule;
  struct gnufdisk_string** edisklabelsystempointer;
  struct gnufdisk_string* edisklabelsystem;
  struct gnufdisk_string** epartitiontypepointer;
  struct gnufdisk_string* epartitiontype;
  size_t* epartitionnumber;
  struct gnufdisk_string** epathpointer;
  struct gnufdisk_string* epath;
  void*** edestinationpointer;
  size_t ** esizepointer;
  gnufdisk_integer* egeometrylength;
  struct gnufdisk_geometry** egeometrypointer;
  struct gnufdisk_geometry* egeometry;
  struct gnufdisk_device** edevicepointer;
  struct gnufdisk_disklabel** edisklabelpointer;
  struct gnufdisk_partition** epartitionpointer;
  void **ereadbuffer;
  void **ewritebuffer;
  struct gnufdisk_device* enotopen;
  gnufdisk_integer* ecylinders;
  gnufdisk_integer* eheads;
  gnufdisk_integer* esectors;
  gnufdisk_integer* esectorsize;
};


#endif /* GNUFDISK_DEVICE_H_INCLUDED */
