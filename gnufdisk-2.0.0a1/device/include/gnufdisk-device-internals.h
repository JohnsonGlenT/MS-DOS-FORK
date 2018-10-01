
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

#ifndef GNUFDISK_DEVICE_INTERNALS_H_INCLUDED
#define GNUFDISK_DEVICE_INTERNALS_H_INCLUDED

#include <stdint.h>

#include <gnufdisk-device.h>

struct gnufdisk_device_operations;
struct gnufdisk_disklabel_operations;
struct gnufdisk_partition_operations;

struct gnufdisk_device_operations
{
  void (*open)(void* _data, struct gnufdisk_string* _path);
  void (*disklabel)(void* _data, struct gnufdisk_disklabel_operations*, void** _specific);
  void (*create_disklabel)(void* _data, struct gnufdisk_string* _type, struct gnufdisk_disklabel_operations*, void** _specific);
  void (*set_parameter)(void* _data, struct gnufdisk_string* _parameter, const void* _pdata, size_t _psize);
  void (*get_parameter)(void* _data, struct gnufdisk_string* _parameter, void* _pdata, size_t _psize);
  void (*commit)(void* _data);
  void (*close)(void* _data);
  void (*delete)(void* _data);
};

struct gnufdisk_disklabel_operations
{
  void (*raw)(void* _data, void** _dest, size_t* _size);
  struct gnufdisk_string* (*system)(void* _data);
  void (*partition)(void* _data, size_t _n, struct gnufdisk_partition_operations*, void** _impl_data);
  int (*count_partitions)(void* _data);
  void (*create_partition)(void* _data, struct gnufdisk_geometry* _s, struct gnufdisk_geometry* _e,
			   struct gnufdisk_string* _type, struct gnufdisk_partition_operations*, void** _specific);
  void (*remove_partition)(void* _data, size_t _n);
  void (*set_parameter)(void* _data, struct gnufdisk_string* _name, const void* _pdata, size_t _psize);
  void (*get_parameter)(void* _data, struct gnufdisk_string* _name, void* _pdata, size_t _psize);
  void (*delete)(void* _data);
};

struct gnufdisk_partition_operations 
{
  void (*set_parameter)(void*_data, struct gnufdisk_string* _name, const void* _pdata, size_t _psize);
  void (*get_parameter)(void*_data, struct gnufdisk_string* _name, void* _pdata, size_t _psize);
  struct gnufdisk_string* (*type)(void* _data);
  gnufdisk_integer (*start)(void* _data);
  gnufdisk_integer (*length)(void* _data);
  int (*number)(void*_data);
  int (*have_disklabel)(void* _data);
  void (*disklabel)(void* _data, struct gnufdisk_disklabel_operations*, void** _specific);
  void (*move)(void* _data, struct gnufdisk_geometry* _s);
  void (*resize)(void* _data, struct gnufdisk_geometry* _e);
  int (*read)(void* _data, gnufdisk_integer _sector, void* _dest, size_t _size);
  int (*write)(void* _data, gnufdisk_integer _sector, const void* _src, size_t _size);
  void (*delete)(void* _data);
};

#define _GNUFDISK_BYTE(x, n)  (((x) >> (8 * (n))) & 0xff) 

#define _GNUFDISK_SWAP16(x)  ( (_GNUFDISK_BYTE(x, 0) << 8) + (_GNUFDISK_BYTE(x, 1) << 0) )

#define _GNUFDISK_SWAP32(x)       \
  ((_GNUFDISK_BYTE(x, 0) << 24) + \
  (_GNUFDISK_BYTE(x, 1) << 16) +  \
  (_GNUFDISK_BYTE(x, 2) << 8)  +  \
  (_GNUFDISK_BYTE(x, 3) << 0))

#define _GNUFDISK_SWAP64(x)       \
  ((_GNUFDISK_BYTE(x, 0) << 56) + \
   (_GNUFDISK_BYTE(x, 1) << 48) + \
   (_GNUFDISK_BYTE(x, 2) << 40) + \
   (_GNUFDISK_BYTE(x, 3) << 32) + \
   (_GNUFDISK_BYTE(x, 4) << 24) + \
   (_GNUFDISK_BYTE(x, 5) << 16) + \
   (_GNUFDISK_BYTE(x, 6) << 8)  + \
   (_GNUFDISK_BYTE(x, 7) << 0))

#define GNUFDISK_SWAP16(x)		((uint16_t) _GNUFDISK_SWAP16( (uint16_t) (x) ))
#define GNUFDISK_SWAP32(x)		((uint32_t) _GNUFDISK_SWAP32( (uint32_t) (x) ))
#define GNUFDISK_SWAP64(x)		((uint64_t) _GNUFDISK_SWAP64( (uint64_t) (x) ))

#ifdef WORDS_BIGENDIAN

#define GNUFDISK_HTOLE16(x)	GNUFDISK_SWAP16(x)
#define GNUFDISK_HTOBE16(x)	(x)
#define GNUFDISK_HTOLE32(x)	GNUFDISK_SWAP32(x)
#define GNUFDISK_HTOBE32(x)	(x)
#define GNUFDISK_HTOLE64(x)	GNUFDISK_SWAP64(x)
#define GNUFDISK_HTOBE64(x)	(x)

#define GNUFDISK_LE16TOH(x)	GNUFDISK_SWAP16(x)
#define GNUFDISK_BE16TOH(x)	(x)
#define GNUFDISK_LE32TOH(x)	GNUFDISK_SWAP32(x)
#define GNUFDISK_BE32TOH(x)	(x)
#define GNUFDISK_LE64TOH(x)	GNUFDISK_SWAP64(x)
#define GNUFDISK_BE64TOH(x)	(x)

#else /* !WORDS_BIGENDIAN */

#define GNUFDISK_HTOLE16(x)	(x)
#define GNUFDISK_HTOBE16(x)	GNUFDISK_SWAP16(x)
#define GNUFDISK_HTOLE32(x)	(x)
#define GNUFDISK_HTOBE32(x)	GNUFDISK_SWAP32(x)
#define GNUFDISK_HTOLE64(x)	(x)
#define GNUFDISK_HTOBE64(x)	GNUFDISK_SWAP64(x)

#define GNUFDISK_LE16TOH(x)	(x)
#define GNUFDISK_BE16TOH(x)	GNUFDISK_SWAP16(x)
#define GNUFDISK_LE32TOH(x)	(x)
#define GNUFDISK_BE32TOH(x)	GNUFDISK_SWAP32(x)
#define GNUFDISK_LE64TOH(x)	(x)
#define GNUFDISK_BE64TOH(x)	GNUFDISK_SWAP64(x)

#endif /* !WORDS_BIGENDIAN */
#endif /* DEVICE_INTERNALS_H_INCLUDED */
