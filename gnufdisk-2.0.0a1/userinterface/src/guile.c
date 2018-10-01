
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

#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include <gnufdisk-common.h>
#include <gnufdisk-debug.h>
#include <gnufdisk-exception.h>
#include <gnufdisk-devicemanager.h>

#include <gnufdisk-userinterface.h>

#include "internals.h"

#define GUILE 1

#define SYM_USERINTERFACE "*userinterface*"
#define SYM_COMMANDLINE "*command-line*"
#define SYM_GNUFDISK_HELP "gnufdisk-help"
#define SYM_GNUFDISK_DEVICEMANAGER_MAKE_GEOMETRY "gnufdisk-devicemanager-make-geometry"
#define SYM_GNUFDISK_GEOMETRY_P "gnufdisk-geometry?"
#define SYM_GNUFDISK_GEOMETRY_SET "gnufdisk-geometry-set"
#define SYM_GNUFDISK_GEOMETRY_START "gnufdisk-geometry-start"
#define SYM_GNUFDISK_GEOMETRY_END "gnufdisk-geometry-end"
#define SYM_GNUFDISK_GEOMETRY_LENGTH "gnufdisk-geometry-length"
#define SYM_GNUFDISK_MAKE_DEVICEMANAGER "gnufdisk-make-devicemanager"
#define SYM_GNUFDISK_DEVICEMANAGER_P "gnufdisk-devicemanager?"
#define SYM_GNUFDISK_DEVICEMANAGER_MAKE_DEVICE "gnufdisk-devicemanager-make-device"
#define SYM_GNUFDISK_DEVICE_P "gnufdisk-device?"
#define SYM_GNUFDISK_DEVICE_OPEN "gnufdisk-device-open"
#define SYM_GNUFDISK_DEVICE_DISKLABEL "gnufdisk-device-disklabel"
#define SYM_GNUFDISK_DEVICE_CREATE_DISKLABEL "gnufdisk-device-create-disklabel"
#define SYM_GNUFDISK_DEVICE_SET_PARAMETER "gnufdisk-device-set-parameter"
#define SYM_GNUFDISK_DEVICE_GET_PARAMETER "gnufdisk-device-get-parameter"
#define SYM_GNUFDISK_DEVICE_COMMIT "gnufdisk-device-commit"
#define SYM_GNUFDISK_DEVICE_CLOSE "gnufdisk-device-close"
#define SYM_GNUFDISK_DISKLABEL_P "gnufdisk-disklabel?"
#define SYM_GNUFDISK_DISKLABEL_RAW "gnufdisk-disklabel-raw"
#define SYM_GNUFDISK_DISKLABEL_SYSTEM "gnufdisk-disklabel-system"
#define SYM_GNUFDISK_DISKLABEL_PARTITION "gnufdisk-disklabel-partition"
#define SYM_GNUFDISK_DISKLABEL_COUNT_PARTITIONS "gnufdisk-disklabel-count-partitions"
#define SYM_GNUFDISK_DISKLABEL_CREATE_PARTITION "gnufdisk-disklabel-create-partition"
#define SYM_GNUFDISK_DISKLABEL_REMOVE_PARTITION "gnufdisk-disklabel-remove-partition"
#define SYM_GNUFDISK_DISKLABEL_SET_PARAMETER "gnufdisk-disklabel-set-parameter"
#define SYM_GNUFDISK_DISKLABEL_GET_PARAMETER "gnufdisk-disklabel-get-parameter"
#define SYM_GNUFDISK_PARTITION_P "gnufdisk-partition?"
#define SYM_GNUFDISK_PARTITION_SET_PARAMETER "gnufdisk-partition-set-parameter"
#define SYM_GNUFDISK_PARTITION_GET_PARAMETER "gnufdisk-partition-set-parameter"
#define SYM_GNUFDISK_PARTITION_TYPE "gnufdisk-partition-type"
#define SYM_GNUFDISK_PARTITION_GEOMETRY "gnufdisk-partition-geometry"
#define SYM_GNUFDISK_PARTITION_NUMBER "gnufdisk-partition-number"
#define SYM_GNUFDISK_PARTITION_HAVE_DISKLABEL "gnufdisk-partition-have-disklabel?"
#define SYM_GNUFDISK_PARTITION_DISKLABEL "gnufdisk-partition-disklabel"
#define SYM_GNUFDISK_PARTITION_MOVE "gnufdisk-partition-move"
#define SYM_GNUFDISK_PARTITION_RESIZE "gnufdisk-partition-resize"
#define SYM_GNUFDISK_PARTITION_READ "gnufdisk-partition-read"
#define SYM_GNUFDISK_PARTITION_WRITE "gnufdisk-partition-write"
#define SYM_GNUFDISK_MAKE_RAW "gnufdisk-make-raw"
#define SYM_GNUFDISK_RAW_P "gnufdisk-raw?"
#define SYM_GNUFDISK_RAW_REF "gnufdisk-raw-ref"
#define SYM_GNUFDISK_RAW_SET_X "gnufdisk-raw-set!"
#define SYM_GNUFDISK_RAW_LENGTH "gnufdisk-raw-length"
#define SYM_GNUFDISK_USERINTERFACE_SET_HOOK "gnufdisk-userinterface-set-hook"

/* non public functions */
#define SYM_GNUFDISK_USERINTERFACE_PRINT "gnufdisk-userinterface-print"
#define SYM_GNUFDISK_USERINTERFACE_ERROR "gnufdisk-userinterface-error"
#define SYM_GNUFDISK_USERINTERFACE_YES_NO "gnufdisk-userinterface-yes-no"
#define SYM_GNUFDISK_USERINTERFACE_GET_PATH "gnufdisk-userinterface-get-path"
#define SYM_GNUFDISK_USERINTERFACE_GET_DISKLABEL_SYSTEM "gnufdisk-userinterface-get-path"
#define SYM_GNUFDISK_USERINTERFACE_GET_GEOMETRY "gnufdisk-userinterface-get-geometry"
#define SYM_GNUFDISK_USERINTERFACE_GET_PARTITION_TYPE "gnufdisk-userinterface-get-geometry"

#define HOOK_PRINT "print"
#define HOOK_ERROR "error"
#define HOOK_YES_NO "yes-no"
#define HOOK_GET_PATH "get-path"
#define HOOK_GET_DISKLABEL_SYSTEM "get-disklabel-system"
#define HOOK_GET_GEOMETRY "get-geometry"
#define HOOK_GET_PARTITION_TYPE "get-partition-system"

/* As suggested in the guidelines of guile, 
 * we export only one smob and use a secondary layer of type dispatching
 * on top of it.  This second layer use the 16 smob bits to check its type, */
static scm_t_bits scheme_object_bits;

/* We use this structure to generalize the type of data contained in a smob. 
 * When guile call one of the functions related to this object, 
 * the call is diverted to the specific function 
 * (such as virtual functions of a C++ class). */
struct scheme_object {
  void *specific;
  SCM (*mark) (struct scheme_object*);
  size_t (*free)(struct scheme_object*);
  int (*print)(struct scheme_object*, SCM, scm_print_state*);
  SCM (*equalp)(struct scheme_object*, struct scheme_object*);
};

/* As mentioned above, we use the 16 additional smob bits to identify the type 
 * of data contained in a scheme_object. These labels indicate the types. */
enum scheme_object_type {
  SCHEME_OBJECT_TYPE_DEVICEMANAGER = 0x01,
  SCHEME_OBJECT_TYPE_DEVICE,
  SCHEME_OBJECT_TYPE_DISKLABEL,
  SCHEME_OBJECT_TYPE_PARTITION,
  SCHEME_OBJECT_TYPE_GEOMETRY,
  SCHEME_OBJECT_TYPE_CHS_GEOMETRY,
  SCHEME_OBJECT_TYPE_USERINTERFACE,
  SCHEME_OBJECT_TYPE_RAW
};

enum parameter_type {
  PARAMETER_TYPE_INTEGER,
  PARAMETER_TYPE_STRING,
  PARAMETER_TYPE_UNKNOWN
};

struct scheme_userinterface {
  struct gnufdisk_userinterface* userinterface;
};

struct scheme_devicemanager {
  struct gnufdisk_devicemanager* devicemanager;
};

struct scheme_device {
  SCM devicemanager;
  struct gnufdisk_device* device;
};

struct scheme_disklabel {
  SCM parent;
  struct gnufdisk_disklabel* disklabel;
};

struct scheme_partition {
  SCM disklabel;
  struct gnufdisk_partition* partition;
};

struct scheme_geometry {
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_geometry* geometry;
};

struct scheme_raw {
  void* buf;
  size_t size;
};

/* With these macros we will monitor the content of SCM objects */
#define SCHEME_OBJECT_P(_smob)                                                 \
  (SCM_SMOB_PREDICATE(scheme_object_bits, _smob))

#define SCHEME_OBJECT_TYPE_P(_smob, _type)                                     \
  (SCHEME_OBJECT_P(_smob) && (SCM_SMOB_FLAGS(_smob) == _type))

#define SCHEME_OBJECT_TYPE_DEVICEMANAGER_P(_smob)                              \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_DEVICEMANAGER)

#define SCHEME_OBJECT_TYPE_DEVICE_P(_smob)                                     \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_DEVICE)

#define SCHEME_OBJECT_TYPE_DISKLABEL_P(_smob)                                  \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_DISKLABEL)

#define SCHEME_OBJECT_TYPE_PARTITION_P(_smob)                                  \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_PARTITION)

#define SCHEME_OBJECT_TYPE_GEOMETRY_P(_smob)                                   \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_GEOMETRY)

#define SCHEME_OBJECT_TYPE_CHS_GEOMETRY_P(_smob)                               \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_CHS_GEOMETRY)

#define SCHEME_OBJECT_TYPE_USERINTERFACE_P(_smob)                              \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_USERINTERFACE)

#define SCHEME_OBJECT_TYPE_RAW_P(_smob)                                        \
  SCHEME_OBJECT_TYPE_P(_smob, SCHEME_OBJECT_TYPE_RAW)

static void scheme_export_env(struct gnufdisk_userinterface* _ui, int argc, char** _argv);
static struct gnufdisk_devicemanager* scheme_partition_to_gnufdisk_devicemanager(SCM _smob);

static void delete_string(void* _p)
{
  GNUFDISK_LOG((GUILE, "delete gnufdisk_string* %p", _p));
  gnufdisk_string_delete(_p);
}

static struct gnufdisk_string* scm_to_gnufdisk_string(SCM _s)
{
  struct gnufdisk_string* ret;
  char *s;
  
  s = scm_to_locale_string(_s);
  ret = gnufdisk_string_new(s);
  free(s);

  return ret;
}

static enum parameter_type scm_symbol_to_parameter_type(SCM _sym)
{
  enum parameter_type ret;
  char *s;


  scm_dynwind_begin(0);

  s = scm_to_locale_string(scm_symbol_to_string(_sym));

  scm_dynwind_unwind_handler(&free, s, 0);

  if(strcmp(s, "INT") == 0 || strcmp(s, "INTEGER") == 0)
    ret = PARAMETER_TYPE_INTEGER;
  else if(strcmp(s, "STR") == 0 || strcmp(s, "STRING") == 0)
    ret = PARAMETER_TYPE_STRING;
  else
    ret = PARAMETER_TYPE_UNKNOWN;

  scm_dynwind_end();

  free(s);

  return ret;
}

/* scheme object hooks */
static SCM scheme_object_mark(SCM _p)
{
  struct scheme_object* data;

  GNUFDISK_LOG((GUILE, "mark %p", _p));

  data = (struct scheme_object*) SCM_SMOB_DATA(_p);

  /* forward if possible */
  if(data && data->mark)
    return (*data->mark)(data);

  return SCM_BOOL_F;
}

static size_t scheme_object_free(SCM _p)
{
  struct scheme_object* data;
  size_t bytes;

  GNUFDISK_LOG((GUILE, "request to free SMOB %p", _p));

  data = (struct scheme_object*) SCM_SMOB_DATA(_p);
  bytes = 0;

  if(data)
    {
      SCM_SET_SMOB_DATA(_p, NULL);

      if(data->free)
        bytes += (*data->free)(data);

      memset(data, 0, sizeof(struct scheme_object));
      
      free(data);

      bytes += sizeof(struct scheme_object);
    }

  return bytes;
}

static int scheme_object_print(SCM _obj, SCM _port, scm_print_state* _state)
{
  struct scheme_object* data;

  GNUFDISK_LOG((GUILE, "request to print SMOB %p", _obj));

  data = (struct scheme_object*) SCM_SMOB_DATA(_obj);

  if(data && data->print)
    return (*data->print)(data, _port, _state);

  return 0;
}

static SCM scheme_object_equalp(SCM _obj1, SCM _obj2)
{
  struct scheme_object* data1;
  struct scheme_object* data2;
  scm_t_bits smob1_flags;
  scm_t_bits smob2_flags;

  GNUFDISK_LOG((GUILE, "request to compare SMOB's %p and %p", _obj1, _obj2));

  data1 = (struct scheme_object*) SCM_SMOB_DATA(_obj1);
  data2 = (struct scheme_object*) SCM_SMOB_DATA(_obj2);

  smob1_flags = SCM_SMOB_FLAGS(_obj1);
  smob2_flags = SCM_SMOB_FLAGS(_obj2);

  if(smob1_flags != smob2_flags)
    return SCM_BOOL_F;
  
  if((data1 && data2) && (data1->equalp))
    return (*data1->equalp)(data1, data2);

  return SCM_BOOL_F;
}

static SCM scheme_object_new (scm_t_bits _flags, 
                              void *_specific,
                              SCM (*_mark) (struct scheme_object *),
                              size_t (*_free) (struct scheme_object *),
                              int (*_print) (struct scheme_object *, SCM, scm_print_state *),
                              SCM (*_equalp) (struct scheme_object *, struct scheme_object *))
{
  struct scheme_object *object;
  SCM smob;

  scm_dynwind_begin (0);

  object = scm_calloc (sizeof (struct scheme_object));
  scm_dynwind_unwind_handler (free, object, 0);

  object->specific = _specific;
  object->mark = _mark;
  object->free = _free;
  object->print = _print;
  object->equalp = _equalp;

  GNUFDISK_LOG ((GUILE, "new struct scheme_object* %p", object));

  SCM_NEWSMOB (smob, scheme_object_bits, object);
  SCM_SET_SMOB_FLAGS (smob, _flags);

  scm_dynwind_end();

  return smob;
}

static void* scheme_object_specific(SCM _smob)
{
  struct scheme_object* object;

  if(!SCHEME_OBJECT_P(_smob))
   scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  object = (struct scheme_object*) SCM_SMOB_DATA(_smob);

  GNUFDISK_LOG((GUILE, "get specific from struct scheme_object: %p", object));

  return object ? object->specific : NULL;
}

/* userinterface SMOB functions */
static SCM scheme_userinterface_mark(struct scheme_object* _object)
{
  struct scheme_userinterface* p;
  struct gnufdisk_userinterface* ui;

  GNUFDISK_LOG((GUILE, "mark struct scheme_object* %p", _object));

  p = _object->specific;
  ui = p->userinterface;

  if(ui != NULL)
    {
      if(ui->print)
        scm_gc_mark(ui->print);

      if(ui->error)
      	scm_gc_mark(ui->error);

      if(ui->yes_no)
      	scm_gc_mark(ui->yes_no);

      if(ui->get_path)
        scm_gc_mark(ui->get_path);

      if(ui->get_disklabel_system)
        scm_gc_mark(ui->get_disklabel_system);

      if(ui->get_geometry)
        scm_gc_mark(ui->get_geometry);
	
      if(ui->get_partition_type)
      	scm_gc_mark(ui->get_partition_type);
    }

  return SCM_BOOL_F;
}

static size_t scheme_userinterface_free(struct scheme_object* _object)
{
  struct scheme_userinterface* ui;

  GNUFDISK_LOG((GUILE, "free struct scheme_object %p", _object));

  ui = _object->specific;

  if(ui)
    {
      if(ui->userinterface)
	gnufdisk_userinterface_delete(ui->userinterface);

      memset(ui, 0, sizeof(struct scheme_userinterface));

      free(ui);

      return sizeof(struct scheme_userinterface);
    }

  return 0;
}

static int scheme_userinterface_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_userinterface* ui;

  ui = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<struct gnufdisk_userinterface %p>\n", ui->userinterface);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_userinterface_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_userinterface* userinterface1;
  struct scheme_userinterface* userinterface2;

  GNUFDISK_LOG((GUILE, "compare %p and %p", _obj1, _obj2));

  userinterface1 = _obj1->specific;
  userinterface2 = _obj2->specific;

  return userinterface1 == userinterface2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_userinterface_new(struct gnufdisk_userinterface* _userinterface)
{
  struct scheme_userinterface* userinterface;
  SCM smob;

  userinterface = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  userinterface = scm_calloc(sizeof(struct scheme_userinterface));

  GNUFDISK_LOG((GUILE, "struct scheme_userinterface* %p", userinterface));

  scm_dynwind_unwind_handler (free, userinterface, 0);

  userinterface->userinterface = _userinterface;

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_USERINTERFACE,
                           userinterface, 
                           &scheme_userinterface_mark, 
                           &scheme_userinterface_free, 
                           &scheme_userinterface_print, 
                           &scheme_userinterface_equalp);

  scm_dynwind_end();

  return smob;
}

static struct gnufdisk_userinterface*
scheme_userinterface_to_gnufdisk_userinterface(SCM _p)
{
  struct scheme_userinterface* userinterface;

  userinterface = scheme_object_specific(_p);

  if(!SCHEME_OBJECT_TYPE_USERINTERFACE_P(_p))
    scm_wrong_type_arg(__FUNCTION__, 1, _p);

  return userinterface->userinterface;
}

/* devicemanager SMOB functions */
static SCM scheme_devicemanager_mark(struct scheme_object* _object)
{
  struct scheme_devicemanager* devicemanager;

  GNUFDISK_LOG((GUILE, "mark %p", _object));

  devicemanager = _object->specific;

  /* do nothing */

  return SCM_BOOL_F;
}

static size_t scheme_devicemanager_free(struct scheme_object* _object)
{
  struct scheme_devicemanager* devicemanager;

  devicemanager = _object->specific;

  GNUFDISK_LOG((GUILE, "free: %p", devicemanager));

  if(devicemanager)
    {
      if(devicemanager->devicemanager)
	gnufdisk_devicemanager_delete(devicemanager->devicemanager);

      memset(devicemanager, 0, sizeof(struct scheme_devicemanager));
      free(devicemanager);

      return sizeof(struct scheme_devicemanager);
    }

  return 0;
}

static int scheme_devicemanager_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_devicemanager* devicemanager;

  devicemanager = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<struct gnufdisk_devicemanager* %p>\n",
	   devicemanager->devicemanager);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_devicemanager_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_devicemanager* devicemanager1;
  struct scheme_devicemanager* devicemanager2;

  devicemanager1 = _obj1->specific;
  devicemanager2 = _obj2->specific;

  return devicemanager1 == devicemanager2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_devicemanager_new(struct gnufdisk_devicemanager* _p)
{
  struct scheme_devicemanager* devicemanager;
  SCM smob;

  devicemanager = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  devicemanager = scm_calloc(sizeof(struct scheme_devicemanager));
  scm_dynwind_unwind_handler (free, devicemanager, 0);

  devicemanager->devicemanager = _p;

  GNUFDISK_LOG((GUILE, "inew struct gnufdisk_devicemanager: %p", devicemanager));

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_DEVICEMANAGER, 
      devicemanager, &scheme_devicemanager_mark, &scheme_devicemanager_free, 
      &scheme_devicemanager_print, &scheme_devicemanager_equalp);

  scm_dynwind_end();

  return smob;
}

static struct gnufdisk_devicemanager*
scheme_devicemanager_to_gnufdisk_devicemanager(SCM _smob)
{
  struct scheme_devicemanager* devicemanager;

  devicemanager = scheme_object_specific(_smob);

  if(!SCHEME_OBJECT_TYPE_DEVICEMANAGER_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  return devicemanager->devicemanager;
}

/* device SMOB functions */
static SCM scheme_device_mark(struct scheme_object* _object)
{
  struct scheme_device* device;

  GNUFDISK_LOG((GUILE, "mark %p", _object));

  device = _object->specific;

  return device->devicemanager;
}

static size_t scheme_device_free(struct scheme_object* _object)
{
  struct scheme_device* device;

  GNUFDISK_LOG((GUILE, "free %p", _object));

  device = _object->specific;

  if(device)
    {
      struct gnufdisk_devicemanager* dm;

      dm = scheme_devicemanager_to_gnufdisk_devicemanager(device->devicemanager);

      if(device->device)
	gnufdisk_devicemanager_device_delete(dm, device->device);

      memset(device, 0, sizeof(struct scheme_device));

      free(device);
      
      return sizeof(struct scheme_device);
    }

  return 0;
}

static int scheme_device_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_device* device;

  device = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<struct gnufdisk_device* %p>\n", device->device);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_device_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_device* device1;
  struct scheme_device* device2;

  device1 = _obj1->specific;
  device2 = _obj2->specific;

  return device1 == device2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_device_new(struct gnufdisk_device* _device, SCM _dm)
{
  struct scheme_device* device;
  SCM smob;

  device = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  device = scm_calloc(sizeof(struct scheme_device));
  scm_dynwind_unwind_handler (free, device, 0);

  device->device = _device;
  device->devicemanager = _dm;

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_DEVICE,
    device, &scheme_device_mark, &scheme_device_free, 
    &scheme_device_print, &scheme_device_equalp);

  scm_dynwind_end();

  return smob;
}

static struct gnufdisk_device*
scheme_device_to_gnufdisk_device(SCM _smob)
{
  struct scheme_device* device;

  if(!SCHEME_OBJECT_TYPE_DEVICE_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  device = scheme_object_specific(_smob);

  return device->device;
}

static struct gnufdisk_devicemanager*
scheme_device_to_gnufdisk_devicemanager(SCM _smob)
{
  struct scheme_device* device;
  struct gnufdisk_devicemanager* ret;

  if(!SCHEME_OBJECT_TYPE_DEVICE_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  device = scheme_object_specific(_smob);
  
  ret = scheme_devicemanager_to_gnufdisk_devicemanager(device->devicemanager);

  return ret;
}

/* disklabel SMOB functions */
static SCM scheme_disklabel_mark(struct scheme_object* _object)
{
  struct scheme_disklabel* disklabel;

  GNUFDISK_LOG((GUILE, "mark %p", _object));

  disklabel = _object->specific;

  return disklabel->parent;
}

static size_t scheme_disklabel_free(struct scheme_object* _object)
{
  struct scheme_disklabel* disklabel;

  GNUFDISK_LOG((GUILE, "free %p", _object));

  disklabel = _object->specific;

  if(disklabel)
    {
      struct gnufdisk_devicemanager* dm;

      if(SCHEME_OBJECT_TYPE_DEVICE_P(disklabel->parent))
        dm = scheme_device_to_gnufdisk_devicemanager(disklabel->parent);
      else
        dm = scheme_partition_to_gnufdisk_devicemanager(disklabel->parent);

      if(disklabel->disklabel)
	gnufdisk_devicemanager_disklabel_delete(dm, disklabel->disklabel);

      memset(disklabel, 0, sizeof(struct scheme_disklabel));
      
      free(disklabel);

      return sizeof(struct scheme_disklabel);
    }

  return 0;
}

static int scheme_disklabel_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_disklabel* disklabel;

  disklabel = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<struct gnufdisk_disklabel* %p>\n", disklabel->disklabel);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_disklabel_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_disklabel* disklabel1;
  struct scheme_disklabel* disklabel2;

  disklabel1 = _obj1 ? _obj1->specific : NULL;
  disklabel2 = _obj2 ? _obj2->specific : NULL;

  return disklabel1 == disklabel2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_disklabel_new(struct gnufdisk_disklabel* _disklabel, SCM _parent)
{
  struct scheme_disklabel* disklabel;
  SCM smob;

  disklabel = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  disklabel = scm_calloc(sizeof(struct scheme_disklabel));
  scm_dynwind_unwind_handler (free, disklabel, 0);

  disklabel->disklabel = _disklabel;
  disklabel->parent = _parent;

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_DISKLABEL,
    disklabel, &scheme_disklabel_mark, &scheme_disklabel_free, 
    &scheme_disklabel_print, &scheme_disklabel_equalp);

  scm_dynwind_end();

  return smob;
}

static struct gnufdisk_disklabel*
scheme_disklabel_to_gnufdisk_disklabel(SCM _smob)
{
  struct scheme_disklabel* disklabel;

  if(!SCHEME_OBJECT_TYPE_DISKLABEL_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  disklabel = scheme_object_specific(_smob);

  return disklabel->disklabel;
}

static struct gnufdisk_devicemanager*
scheme_disklabel_to_gnufdisk_devicemanager(SCM _smob)
{
  struct scheme_disklabel* disklabel;
  struct gnufdisk_devicemanager* ret;

  if(!SCHEME_OBJECT_TYPE_DISKLABEL_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  disklabel = scheme_object_specific(_smob);
 
  if(SCHEME_OBJECT_TYPE_DEVICE_P(disklabel->parent))
    ret = scheme_device_to_gnufdisk_devicemanager(disklabel->parent);
  else
    ret = scheme_partition_to_gnufdisk_devicemanager(disklabel->parent);

  return ret;
}

#if 0
static SCM 
scheme_disklabel_to_scheme_device(SCM _smob)
{
  struct scheme_disklabel* disklabel;
  SCM ret;

  if(!SCHEME_OBJECT_TYPE_DISKLABEL_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  disklabel = scheme_object_specific(_smob);
 
  if(SCHEME_OBJECT_TYPE_DEVICE_P(disklabel->parent))
    ret = disklabel->parent;
  else
    ret = scheme_disklabel_to_scheme_device(scheme_partition_scheme_disklabel(disklabel->parent));

  return ret;
}
#endif

/* partition SMOB functions */
static SCM scheme_partition_mark(struct scheme_object* _object)
{
  struct scheme_partition* partition;

  partition = _object->specific;

  /* do nothing */

  return partition->disklabel;
}

static size_t scheme_partition_free(struct scheme_object* _object)
{
  struct scheme_partition* partition;

  partition = _object->specific;

  if(partition)
    {
      struct gnufdisk_devicemanager* dm;

      dm = scheme_disklabel_to_gnufdisk_devicemanager(partition->disklabel);

      if(partition->partition)
	gnufdisk_devicemanager_partition_delete(dm, partition->partition);

      memset(partition, 0, sizeof(struct scheme_partition));
      free(partition);
      
      return sizeof(struct scheme_partition);
    }

  return 0;
}

static int scheme_partition_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_partition* partition;

  partition = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<struct gnufdisk_partition* %p>\n", partition->partition);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_partition_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_partition* partition1;
  struct scheme_partition* partition2;

  partition1 = _obj1->specific;
  partition2 = _obj2->specific;

  return partition1 == partition2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_partition_new(struct gnufdisk_partition* _partition, 
                                SCM _disklabel)
{
  struct scheme_partition* partition;
  SCM smob;

  partition = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  partition = scm_calloc(sizeof(struct scheme_partition));
  scm_dynwind_unwind_handler (free, partition, 0);

  partition->partition = _partition;
  partition->disklabel = _disklabel;

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_PARTITION,
    partition, &scheme_partition_mark, &scheme_partition_free, 
    &scheme_partition_print, &scheme_partition_equalp);

  scm_dynwind_end();

  return smob;
}

static struct gnufdisk_partition*
scheme_partition_to_gnufdisk_partition(SCM _smob)
{
  struct scheme_partition* partition;

  if(!SCHEME_OBJECT_TYPE_PARTITION_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  partition = scheme_object_specific(_smob);

  return partition->partition;
}

static struct gnufdisk_devicemanager* 
scheme_partition_to_gnufdisk_devicemanager(SCM _smob)
{
  struct scheme_partition* partition;
  struct gnufdisk_devicemanager* ret;

  if(!SCHEME_OBJECT_TYPE_PARTITION_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  partition = scheme_object_specific(_smob);

  ret = scheme_disklabel_to_gnufdisk_devicemanager(partition->disklabel);

  return ret;
}
#if 0
static SCM 
scheme_partition_scheme_disklabel(SCM _smob)
{
  struct scheme_partition* partition;

  if(!SCHEME_OBJECT_TYPE_PARTITION_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  partition = scheme_object_specific(_smob);

  return partition->disklabel;
}
#endif

/* geometry SMOB functions */
static SCM scheme_geometry_mark(struct scheme_object* _object)
{
  struct scheme_geometry* geometry;

  geometry = _object->specific;

  /* do nothing */

  return SCM_BOOL_F;
}

static size_t scheme_geometry_free(struct scheme_object* _object)
{
  struct scheme_geometry* geometry;

  geometry = _object->specific;

  if(geometry)
    {
      if(geometry->dm)
        gnufdisk_devicemanager_delete(geometry->dm);

      if(geometry->geometry)
	gnufdisk_geometry_delete(geometry->geometry);

      memset(geometry, 0, sizeof(struct scheme_geometry));

      free(geometry);
      
      return sizeof(struct scheme_geometry);
    }

  return 0;
}

static int scheme_geometry_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_geometry* geometry;

  geometry = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<struct gnufdisk_geometry* %p> )>\n", geometry->geometry);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_geometry_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_geometry* geometry1;
  struct scheme_geometry* geometry2;

  geometry1 = _obj1->specific;
  geometry2 = _obj2->specific;

  return geometry1 == geometry2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_geometry_new(struct gnufdisk_geometry* _geom, 
                               struct gnufdisk_devicemanager* _dm)
{
  struct scheme_geometry* geometry;
  SCM smob;

  geometry = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  geometry = scm_calloc(sizeof(struct scheme_geometry));
  scm_dynwind_unwind_handler (free, geometry, 0);

  geometry->geometry = _geom;

  gnufdisk_devicemanager_ref(_dm);
  geometry->dm = _dm;

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_GEOMETRY,
                           geometry, 
                           &scheme_geometry_mark, 
                           &scheme_geometry_free,
                           &scheme_geometry_print, 
                           &scheme_geometry_equalp);

  scm_dynwind_end();

  return smob;
}

static struct gnufdisk_geometry* scheme_geometry_to_gnufdisk_geometry(SCM _smob)
{
  struct scheme_geometry* geometry;

  if(!SCHEME_OBJECT_TYPE_GEOMETRY_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  geometry = scheme_object_specific(_smob);

  return geometry->geometry;
}

static struct gnufdisk_devicemanager* scheme_geometry_to_gnufdisk_devicemanager(SCM _smob)
{
  struct scheme_geometry* geometry;

  if(!SCHEME_OBJECT_TYPE_GEOMETRY_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  geometry = scheme_object_specific(_smob);

  return geometry->dm;
}

/* raw SMOB functions */
static SCM scheme_raw_mark(struct scheme_object* _object)
{
  struct scheme_raw* raw;

  raw = _object->specific;

  /* do nothing */

  return SCM_BOOL_F;
}

static size_t scheme_raw_free(struct scheme_object* _object)
{
  struct scheme_raw* raw;

  raw = _object->specific;

  if(raw)
    {
      if(raw->buf)
	free(raw->buf);

      memset(raw, 0, sizeof(struct scheme_raw));
      free(raw);
      
      return sizeof(struct scheme_raw);
    }

  return 0;
}

static int scheme_raw_print(struct scheme_object* _obj, SCM _port, scm_print_state* _state)
{
  char buffer[1024];
  struct scheme_raw* raw;

  raw = _obj->specific;

  snprintf(buffer, sizeof(buffer), "#<raw* %p (size: %u)> )>\n", raw->buf, raw->size);

  scm_c_write(_port, buffer, strlen(buffer));

  return 1;
}

static SCM scheme_raw_equalp(struct scheme_object* _obj1, struct scheme_object* _obj2)
{
  struct scheme_raw* raw1;
  struct scheme_raw* raw2;

  raw1 = _obj1->specific;
  raw2 = _obj2->specific;

  return raw1 == raw2 ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_raw_new(void* _buf, size_t _size)
{
  struct scheme_raw* raw;
  SCM smob;

  raw = NULL;
  smob = NULL;

  scm_dynwind_begin(0);

  raw = scm_calloc(sizeof(struct scheme_raw));
  scm_dynwind_unwind_handler (free, raw, 0);

  raw->buf = _buf;
  raw->size = _size;

  smob = scheme_object_new(SCHEME_OBJECT_TYPE_RAW,
                           raw, 
                           &scheme_raw_mark, 
                           &scheme_raw_free,
                           &scheme_raw_print, 
                           &scheme_raw_equalp);

  scm_dynwind_end();

  return smob;
}

static void* scheme_raw_data(SCM _smob)
{
  struct scheme_raw* raw;

  if(!SCHEME_OBJECT_TYPE_RAW_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  raw = scheme_object_specific(_smob);

  return raw->buf;
}

static size_t scheme_raw_size(SCM _smob)
{
  struct scheme_raw* raw;

  if(!SCHEME_OBJECT_TYPE_RAW_P(_smob))
    scm_wrong_type_arg(__FUNCTION__, 1, _smob);

  raw = scheme_object_specific(_smob);

  return raw->size;
}

static SCM scheme_make_raw(SCM _size)
{
  size_t size;
  void* buf;

  size = scm_to_size_t(_size);

  if((buf = malloc(size)) == NULL)
    scm_memory_error(SYM_GNUFDISK_MAKE_RAW);

  return scheme_raw_new(buf, size);
}

static SCM scheme_raw_p(SCM _smob)
{
  return SCHEME_OBJECT_TYPE_RAW_P(_smob) ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_raw_ref(SCM _smob, SCM _index)
{
  struct scheme_raw* raw;
  int index;

  if(!SCHEME_OBJECT_TYPE_RAW_P(_smob))
    scm_wrong_type_arg(SYM_GNUFDISK_RAW_REF, 1, _smob);

  raw = scheme_object_specific(_smob);

  if(!scm_is_unsigned_integer(_index, 0, raw->size))
    scm_wrong_type_arg(SYM_GNUFDISK_RAW_REF, 2, _index);
  
  index = scm_to_int(_index);

  return scm_from_uchar(((unsigned char*) raw->buf)[index]);
}

static SCM scheme_raw_set_x(SCM _smob, SCM _index, SCM _val)
{
  struct scheme_raw* raw;
  int index;
  unsigned char val;

  if(!SCHEME_OBJECT_TYPE_RAW_P(_smob))
    scm_wrong_type_arg(SYM_GNUFDISK_RAW_SET_X, 1, _smob);

  raw = scheme_object_specific(_smob);

  if(!scm_is_unsigned_integer(_index, 0, raw->size))
    scm_wrong_type_arg(SYM_GNUFDISK_RAW_SET_X, 2, _index);
 
  index = scm_to_int(_index);

  if(!scm_is_unsigned_integer(_val, 0, 255))
    scm_wrong_type_arg(SYM_GNUFDISK_RAW_SET_X, 3, _val);
  
  val = scm_to_uchar(_val);

  ((unsigned char*) raw->buf)[index] = val;

  return SCM_UNSPECIFIED;
}

static SCM scheme_raw_length(SCM _smob)
{
  struct scheme_raw* raw;

  if(!SCHEME_OBJECT_TYPE_RAW_P(_smob))
    scm_wrong_type_arg(SYM_GNUFDISK_RAW_LENGTH, 1, _smob);

  raw = scheme_object_specific(_smob);

  return scm_from_size_t(raw->size);
}

static SCM scheme_preunwind_catch_handler (void* _data, SCM _key, SCM _args)
{
  if(_data)
    {
      SCM* stack;

      stack = _data;

      *stack = scm_make_stack(SCM_BOOL_T, SCM_EOL);
    }

  return SCM_BOOL_F;
}

/* This function is used as an exception handler to scm_catch 
 *
 * Exceptions thrown in scheme use symbols to indicate the type of error. 
 * In this function symbols are converted to error codes and packed
 * within it's data into a struct scheme_error. */
static SCM scheme_catch_handler (void *_data, SCM _key, SCM _args)
{
  GNUFDISK_LOG ((GUILE, "caught an exception"));

  return SCM_BOOL_F;
}

/* SCHEME API IMPLEMENTATION */
static SCM scheme_gnufdisk_help(void)
{
  scm_puts("GNU Fdisk SCHEME API: \n", scm_current_output_port());

  scm_puts("  Global symbols:\n"
	   "    " SYM_USERINTERFACE "\n"
	   "    " SYM_COMMANDLINE "\n"
	   "    " SYM_GNUFDISK_HELP "\n"
	   "    " SYM_GNUFDISK_DEVICEMANAGER_MAKE_GEOMETRY " devicemanager start length\n"
	   "    " SYM_GNUFDISK_GEOMETRY_P " geometry\n"
	   "    " SYM_GNUFDISK_GEOMETRY_SET " geometry start length\n"
	   "    " SYM_GNUFDISK_GEOMETRY_START " geometry\n"
	   "    " SYM_GNUFDISK_GEOMETRY_END " geometry\n"
	   "    " SYM_GNUFDISK_GEOMETRY_LENGTH " geometry\n"
	   "    " SYM_GNUFDISK_MAKE_DEVICEMANAGER " userinterface\n"
	   "    " SYM_GNUFDISK_DEVICEMANAGER_P " devicemanager\n"
	   "    " SYM_GNUFDISK_DEVICEMANAGER_MAKE_DEVICE " devicemanager module module-options\n"
	   "    " SYM_GNUFDISK_DEVICE_P " device\n"
	   "    " SYM_GNUFDISK_DEVICE_OPEN " device path\n"
	   "    " SYM_GNUFDISK_DEVICE_DISKLABEL " device\n"
	   "    " SYM_GNUFDISK_DEVICE_CREATE_DISKLABEL " device type\n" 
	   "    " SYM_GNUFDISK_DEVICE_SET_PARAMETER " device param-name value\n"
	   "    " SYM_GNUFDISK_DEVICE_GET_PARAMETER " device param-name type\n"
	   "    " SYM_GNUFDISK_DEVICE_COMMIT " device\n"
	   "    " SYM_GNUFDISK_DEVICE_CLOSE " device\n"
	   "    " SYM_GNUFDISK_DISKLABEL_P " disklabel\n"
	   "    " SYM_GNUFDISK_DISKLABEL_RAW " disklabel\n"
	   "    " SYM_GNUFDISK_DISKLABEL_SYSTEM " disklabel\n"
	   "    " SYM_GNUFDISK_DISKLABEL_PARTITION " disklabel number\n"
           "    " SYM_GNUFDISK_DISKLABEL_COUNT_PARTITIONS " disklabel\n"
	   "    " SYM_GNUFDISK_DISKLABEL_CREATE_PARTITION " disklabel start-geometry end-geometry type\n"
	   "    " SYM_GNUFDISK_DISKLABEL_REMOVE_PARTITION " disklabel number\n"
	   "    " SYM_GNUFDISK_DISKLABEL_SET_PARAMETER " disklabel param-name value\n"
	   "    " SYM_GNUFDISK_DISKLABEL_GET_PARAMETER " disklabel param-name type\n"
           "    " SYM_GNUFDISK_PARTITION_P " partition\n"
           "    " SYM_GNUFDISK_PARTITION_SET_PARAMETER " partition param-name value\n"
           "    " SYM_GNUFDISK_PARTITION_GET_PARAMETER " partition param-name type\n"
           "    " SYM_GNUFDISK_PARTITION_TYPE " partition\n"
           "    " SYM_GNUFDISK_PARTITION_GEOMETRY " partition\n"
           "    " SYM_GNUFDISK_PARTITION_NUMBER " partition\n"
           "    " SYM_GNUFDISK_PARTITION_HAVE_DISKLABEL " partition\n"
           "    " SYM_GNUFDISK_PARTITION_DISKLABEL " partition\n"
           "    " SYM_GNUFDISK_PARTITION_MOVE " partition start-range\n"
           "    " SYM_GNUFDISK_PARTITION_RESIZE " partition end-range\n"
           "    " SYM_GNUFDISK_PARTITION_READ " partition start-sector size\n"
           "    " SYM_GNUFDISK_PARTITION_WRITE " partition start-sector raw-data\n"
           "    " SYM_GNUFDISK_RAW_P " raw\n", 
    scm_current_output_port());
  
  return SCM_BOOL_T;
}

static SCM scheme_make_devicemanager(SCM _smob)
{
  struct gnufdisk_userinterface* ui;
  struct gnufdisk_devicemanager* dm;

  ui = scheme_userinterface_to_gnufdisk_userinterface(_smob);

  dm = gnufdisk_devicemanager_new(ui);

  return scheme_devicemanager_new(dm);
}

static SCM scheme_devicemanager_p(SCM _smob)
{
  return SCHEME_OBJECT_TYPE_DEVICEMANAGER_P(_smob) ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_devicemanager_make_geometry(SCM _dm, SCM _start, SCM _length)
{
  struct gnufdisk_geometry* geom;
  struct gnufdisk_devicemanager* dm;
  gnufdisk_integer start;
  gnufdisk_integer length;

  dm = scheme_devicemanager_to_gnufdisk_devicemanager(_dm);

  if(!scm_is_integer(_start))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICEMANAGER_MAKE_GEOMETRY, 1, _start);

  if(!scm_is_integer(_length))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICEMANAGER_MAKE_GEOMETRY, 2, _length);

  start = scm_to_int64(_start);
  length = scm_to_int64(_length);

  if((geom = gnufdisk_devicemanager_geometry_new(dm, start, length)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICEMANAGER_MAKE_GEOMETRY,
	      "cannot create geometry",
	      SCM_EOL, SCM_UNDEFINED);

  return scheme_geometry_new(geom, dm);
}

static SCM scheme_geometry_p(SCM _smob)
{
  return SCHEME_OBJECT_TYPE_GEOMETRY_P(_smob) ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_geometry_set(SCM _smob, SCM _start, SCM _length)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_geometry* geom;
  gnufdisk_integer start;
  gnufdisk_integer length;
 
  if(!scm_is_integer(_start))
    scm_wrong_type_arg(SYM_GNUFDISK_GEOMETRY_SET, 2, _start);

  if(!scm_is_integer(_length))
    scm_wrong_type_arg(SYM_GNUFDISK_GEOMETRY_SET, 3, _length);

  dm = scheme_geometry_to_gnufdisk_devicemanager(_smob);
  geom = scheme_geometry_to_gnufdisk_geometry(_smob);
  start = scm_to_long_long(_start);
  length = scm_to_long_long(_length);

  if(gnufdisk_devicemanager_geometry_set(dm, geom, start, length) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_GEOMETRY_SET,
	      "cannot set geometry",
	      SCM_EOL, SCM_UNDEFINED);

  return _smob;
}

static SCM scheme_geometry_start(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_geometry* geom;
  gnufdisk_integer start;

  dm = scheme_geometry_to_gnufdisk_devicemanager(_smob);
  geom = scheme_geometry_to_gnufdisk_geometry(_smob);
  
  if((start = gnufdisk_devicemanager_geometry_start(dm, geom)) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_GEOMETRY_START,
	      "cannot get geometry start",
	      SCM_EOL, SCM_UNDEFINED);

  return scm_from_long_long(start);
}

static SCM scheme_geometry_end(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_geometry* geom;
  gnufdisk_integer end;

  dm = scheme_geometry_to_gnufdisk_devicemanager(_smob);
  geom = scheme_geometry_to_gnufdisk_geometry(_smob);
  
  if((end = gnufdisk_devicemanager_geometry_end(dm, geom)) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_GEOMETRY_END,
	      "cannot get geometry end",
	      SCM_EOL, SCM_UNDEFINED);

  return scm_from_long_long(end);
}

static SCM scheme_geometry_length(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_geometry* geom;
  gnufdisk_integer length;

  dm = scheme_geometry_to_gnufdisk_devicemanager(_smob);
  geom = scheme_geometry_to_gnufdisk_geometry(_smob);
  
  if((length = gnufdisk_devicemanager_geometry_length(dm, geom)) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_GEOMETRY_LENGTH,
	      "cannot get geometry length",
	      SCM_EOL, SCM_UNDEFINED);

  return scm_from_long_long(length);
}

static SCM scheme_devicemanager_make_device(SCM _smob, SCM _mod, SCM _options)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_string* module;
  struct gnufdisk_string* options;
  struct gnufdisk_device* dev;

  scm_dynwind_begin(0);

  if(!scm_is_string(_mod))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICEMANAGER_MAKE_DEVICE, 2, _mod);

  if(!scm_is_string(_options))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICEMANAGER_MAKE_DEVICE, 3, _options);

  dm = scheme_devicemanager_to_gnufdisk_devicemanager(_smob);

  module = scm_to_gnufdisk_string(_mod);
  scm_dynwind_unwind_handler(&delete_string, module, 0);

  options = scm_to_gnufdisk_string(_options);
  scm_dynwind_unwind_handler(&delete_string, options, 0);

  if((dev = gnufdisk_devicemanager_device_new(dm, module, options)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICEMANAGER_MAKE_DEVICE,
	      "cannot create device",
	      SCM_EOL, SCM_UNDEFINED);
 
  scm_dynwind_end();

  gnufdisk_string_delete(module);
  gnufdisk_string_delete(options);
  return scheme_device_new(dev, _smob);
}

static SCM scheme_device_p(SCM _smob)
{
  return SCHEME_OBJECT_TYPE_DEVICE_P(_smob) ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_device_open(SCM _smob, SCM _path)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;
  struct gnufdisk_string* path;

  scm_dynwind_begin(0);

  if(!scm_is_string(_path))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_OPEN, 2, _path);

  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);
  
  path = scm_to_gnufdisk_string(_path);
  scm_dynwind_unwind_handler(&delete_string, path, 0);

  if(gnufdisk_devicemanager_device_open(dm, dev, path) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_OPEN,
	      "cannot open device",
	      SCM_EOL, SCM_UNDEFINED);
  
  scm_dynwind_end();

  gnufdisk_string_delete(path);

  return _smob;
}

static SCM scheme_device_disklabel(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;
  struct gnufdisk_disklabel* disk;

  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);

  if((disk = gnufdisk_devicemanager_device_disklabel(dm, dev)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_DISKLABEL,
	      "cannot get disklabel",
	      SCM_EOL, SCM_UNDEFINED);

  return scheme_disklabel_new(disk, _smob);
}

static SCM scheme_disklabel_p(SCM _smob)
{
  return SCHEME_OBJECT_TYPE_DISKLABEL_P(_smob) ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_device_create_disklabel(SCM _smob, SCM _system)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;
  struct gnufdisk_string* system;
  struct gnufdisk_disklabel* disk;

  if(!scm_is_string(_system))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_CREATE_DISKLABEL, 2, _system);

  scm_dynwind_begin(0);

  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);

  system = scm_to_gnufdisk_string(_system);
  scm_dynwind_unwind_handler(&delete_string, system, 0);

  if((disk = gnufdisk_devicemanager_device_create_disklabel(dm, dev, system)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_CREATE_DISKLABEL,
	      "cannot create disklabel",
	      SCM_EOL, SCM_UNDEFINED);

  scm_dynwind_end();

  gnufdisk_string_delete(system);

  return scheme_disklabel_new(disk, _smob);
}

static SCM scheme_device_set_parameter(SCM _smob, SCM _param, SCM _data)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;
  struct gnufdisk_string* param;
  void* data;
  size_t size;

  scm_dynwind_begin(0);

  if(!scm_is_string(_param))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_SET_PARAMETER, 2, _param);
  
  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);
  
  param = scm_to_gnufdisk_string(_param);
  scm_dynwind_unwind_handler(&delete_string, param, 0);

  if(scm_is_integer(_data))
    {
      data = scm_malloc(sizeof(gnufdisk_integer));
      *((int*)data) = scm_to_long_long(_data);
      size = sizeof(gnufdisk_integer);
      scm_dynwind_free(data);
    }
  else if(scm_is_string(_data))
    {
      data = scm_to_gnufdisk_string(_data);
      size = sizeof(struct gnufdisk_string*);
      scm_dynwind_unwind_handler(&delete_string, data, SCM_F_WIND_EXPLICITLY);
    }
  else
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_SET_PARAMETER, 3, _data);

  if(gnufdisk_devicemanager_device_set_parameter(dm, dev, param, data, size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_SET_PARAMETER,
	      "cannot set parameter",
	      SCM_EOL, SCM_UNDEFINED);
  
  scm_dynwind_end();

  gnufdisk_string_delete(param);

  return SCM_BOOL_T;
}

static SCM scheme_device_get_parameter(SCM _smob, SCM _param, SCM _type)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;
  struct gnufdisk_string* param;
  enum parameter_type type;
  void* data;
  size_t size;
  SCM ret;

  scm_dynwind_begin(0);

  if(!scm_is_string(_param))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 2, _param);

  if(!scm_is_symbol(_type))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 3, _type);

  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);
  
  param = scm_to_gnufdisk_string(_param);
  scm_dynwind_unwind_handler(&delete_string, param, 0);
  
  type = scm_symbol_to_parameter_type(_type);

  if(type == PARAMETER_TYPE_INTEGER)
    {
      data = scm_malloc(sizeof(gnufdisk_integer));
      size = sizeof(gnufdisk_integer);
      scm_dynwind_free(data);
    }
  else if(type == PARAMETER_TYPE_STRING)
    {
      data = gnufdisk_string_new("");
      size = sizeof(struct gnufdisk_string*);
      scm_dynwind_unwind_handler(&delete_string, data, SCM_F_WIND_EXPLICITLY);
    }
  else
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 3, _type);

  if(gnufdisk_devicemanager_device_get_parameter(dm, dev, param, data, size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_GET_PARAMETER,
	      "cannot get parameter",
	      SCM_EOL, SCM_UNDEFINED);

  if(type == PARAMETER_TYPE_INTEGER)
   ret = scm_from_long_long(*(gnufdisk_integer*) data);
  else 
   ret = scm_from_locale_string(gnufdisk_string_c_string(data));

  scm_dynwind_end();

  gnufdisk_string_delete(param);

  return ret;
}

static SCM scheme_device_commit(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;

  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);

  if(gnufdisk_devicemanager_device_commit(dm, dev) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_COMMIT,
	      "cannot commit",
	      SCM_EOL, SCM_UNDEFINED);

  return SCM_BOOL_T;
}

static SCM scheme_device_close(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_device* dev;

  dm = scheme_device_to_gnufdisk_devicemanager(_smob);
  dev = scheme_device_to_gnufdisk_device(_smob);

  if(gnufdisk_devicemanager_device_close(dm, dev) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DEVICE_CLOSE,
	      "cannot close device",
	      SCM_EOL, SCM_UNDEFINED);

  return SCM_BOOL_T;
}

static SCM scheme_disklabel_raw(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  void* dest;
  size_t size;

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);

  if(gnufdisk_devicemanager_disklabel_raw(dm, disk, &dest, &size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_RAW,
	      "cannot get raw disklabel",
	      SCM_EOL, SCM_UNDEFINED);


  return scheme_raw_new(dest, size);
}

static SCM scheme_disklabel_system(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  struct gnufdisk_string* system;
  SCM ret;

  scm_dynwind_begin(0);

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);

  if((system = gnufdisk_devicemanager_disklabel_system(dm, disk)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_SYSTEM,
	      "cannot get disklabel system",
	      SCM_EOL, SCM_UNDEFINED);

  scm_dynwind_unwind_handler(&delete_string, system, 0);
  ret = scm_from_locale_string(gnufdisk_string_c_string(system));

  scm_dynwind_end();

  gnufdisk_string_delete(system);

  return ret;
}

static SCM scheme_disklabel_partition(SCM _smob, SCM _number)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  struct gnufdisk_partition* part;
  size_t num;

  if(!scm_is_integer(_number))
    scm_wrong_type_arg(SYM_GNUFDISK_DISKLABEL_PARTITION, 2, _number);

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);
  num = scm_to_size_t(_number);

  if((part = gnufdisk_devicemanager_disklabel_partition(dm, disk, num)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_PARTITION,
	      "cannot get partition",
	      SCM_EOL, SCM_UNDEFINED);


  return scheme_partition_new(part, _smob);
}

static SCM scheme_disklabel_count_partitions(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  int res;

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);

  if((res = gnufdisk_devicemanager_disklabel_count_partitions(dm, disk)) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_COUNT_PARTITIONS,
	      "cannot count partitions",
	      SCM_EOL, SCM_UNDEFINED);

  return scm_from_int(res);
}

static SCM scheme_partition_p(SCM _smob)
{
  return SCHEME_OBJECT_TYPE_PARTITION_P(_smob) ? SCM_BOOL_T : SCM_BOOL_F;
}

static SCM scheme_disklabel_create_partition(SCM _smob, SCM _start, SCM _end, SCM _type)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  struct gnufdisk_geometry* start;
  struct gnufdisk_geometry* end;
  struct gnufdisk_string* type;
  struct gnufdisk_partition* part;

  scm_dynwind_begin(0);

  if(!scm_is_string(_type))
    scm_wrong_type_arg(SYM_GNUFDISK_DISKLABEL_CREATE_PARTITION, 4, _type);

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);
  start = scheme_geometry_to_gnufdisk_geometry(_start);
  end = scheme_geometry_to_gnufdisk_geometry(_end);

  type = scm_to_gnufdisk_string(_type);
  scm_dynwind_unwind_handler(&delete_string, type, 0);

  if((part = gnufdisk_devicemanager_disklabel_create_partition(dm, disk, start, end, type)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_CREATE_PARTITION,
	      "cannot create partition",
	      SCM_EOL, SCM_UNDEFINED);

  scm_dynwind_end();

  gnufdisk_string_delete(type);

  return scheme_partition_new(part, _smob);
}

static SCM scheme_disklabel_remove_partition(SCM _smob, SCM _number)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  size_t num;

  if(!scm_is_integer(_number))
    scm_wrong_type_arg(SYM_GNUFDISK_DISKLABEL_REMOVE_PARTITION, 2, _number);

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);
  num = scm_to_size_t(_number);

  if(gnufdisk_devicemanager_disklabel_remove_partition(dm, disk, num) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_REMOVE_PARTITION,
	      "cannot remove partition",
	      SCM_EOL, SCM_UNDEFINED);
  
  return SCM_BOOL_T;
}

static SCM scheme_disklabel_set_parameter(SCM _smob, SCM _param, SCM _data)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  struct gnufdisk_string* param;
  void* data;
  size_t size;

  scm_dynwind_begin(0);

  if(!scm_is_string(_param))
    scm_wrong_type_arg(SYM_GNUFDISK_DISKLABEL_SET_PARAMETER, 2, _param);
  
  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);
  
  param = scm_to_gnufdisk_string(_param);
  scm_dynwind_unwind_handler(&delete_string, param, 0);

  if(scm_is_integer(_data))
    {
      data = scm_malloc(sizeof(gnufdisk_integer));
      *((int*)data) = scm_to_long_long(_data);
      size = sizeof(gnufdisk_integer);
      scm_dynwind_free(data);
    }
  else if(scm_is_string(_data))
    {
      data = scm_to_gnufdisk_string(_data);
      size = sizeof(struct gnufdisk_string*);
      scm_dynwind_unwind_handler(&delete_string, data, SCM_F_WIND_EXPLICITLY);
    }
  else
    scm_wrong_type_arg(SYM_GNUFDISK_DISKLABEL_SET_PARAMETER, 3, _data);

  if(gnufdisk_devicemanager_disklabel_set_parameter(dm, disk, param, data, size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_SET_PARAMETER,
	      "cannot set parameter",
	      SCM_EOL, SCM_UNDEFINED);
  
  scm_dynwind_end();

  gnufdisk_string_delete(param);

  return SCM_BOOL_T;
}

static SCM scheme_disklabel_get_parameter(SCM _smob, SCM _param, SCM _type)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_disklabel* disk;
  struct gnufdisk_string* param;
  enum parameter_type type;
  void* data;
  size_t size;
  SCM ret;

  scm_dynwind_begin(0);

  if(!scm_is_string(_param))
    scm_wrong_type_arg(SYM_GNUFDISK_DISKLABEL_GET_PARAMETER, 2, _param);
  
  if(!scm_is_symbol(_type))
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 3, _type);

  dm = scheme_disklabel_to_gnufdisk_devicemanager(_smob);
  disk = scheme_disklabel_to_gnufdisk_disklabel(_smob);
  
  param = scm_to_gnufdisk_string(_param);
  scm_dynwind_unwind_handler(&delete_string, param, 0);

  type = scm_symbol_to_parameter_type(_type);

  if(type == PARAMETER_TYPE_INTEGER)
    {
      data = scm_malloc(sizeof(gnufdisk_integer));
      size = sizeof(gnufdisk_integer);
      scm_dynwind_free(data);
    }
  else if(type == PARAMETER_TYPE_STRING)
    {
      data = gnufdisk_string_new("");
      size = sizeof(struct gnufdisk_string*);
      scm_dynwind_unwind_handler(&delete_string, data, SCM_F_WIND_EXPLICITLY);
    }
  else
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 3, _type);

  if(gnufdisk_devicemanager_disklabel_get_parameter(dm, disk, param, data, size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_DISKLABEL_GET_PARAMETER,
	      "cannot get parameter",
	      SCM_EOL, SCM_UNDEFINED);
 
  if(type == PARAMETER_TYPE_INTEGER)
   ret = scm_from_long_long(*(gnufdisk_integer*) data);
  else 
   ret = scm_from_locale_string(gnufdisk_string_c_string(data));

  scm_dynwind_end();

  gnufdisk_string_delete(param);

  return ret;
}

static SCM scheme_partition_set_parameter(SCM _smob, SCM _param, SCM _data)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  struct gnufdisk_string* param;
  void* data;
  size_t size;

  scm_dynwind_begin(0);

  if(!scm_is_string(_param))
    scm_wrong_type_arg(SYM_GNUFDISK_PARTITION_SET_PARAMETER, 2, _param);
  
  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
  
  param = scm_to_gnufdisk_string(_param);
  scm_dynwind_unwind_handler(&delete_string, param, 0);

  if(scm_is_integer(_data))
    {
      data = scm_malloc(sizeof(gnufdisk_integer));
      size = sizeof(gnufdisk_integer);
      scm_dynwind_free(data);
    }
  else if(scm_is_string(_data))
    {
      data = gnufdisk_string_new("");
      size = sizeof(struct gnufdisk_string*);
      scm_dynwind_unwind_handler(&delete_string, data, SCM_F_WIND_EXPLICITLY);
    }
  else
    scm_wrong_type_arg(SYM_GNUFDISK_PARTITION_SET_PARAMETER, 3, _data);

  if(gnufdisk_devicemanager_partition_set_parameter(dm, part, param, data, size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_SET_PARAMETER,
	      "cannot get parameter",
	      SCM_EOL, SCM_UNDEFINED);
 
  scm_dynwind_end();

  gnufdisk_string_delete(param);

  return SCM_BOOL_T;
}

static SCM scheme_partition_get_parameter(SCM _smob, SCM _param, SCM _type)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  struct gnufdisk_string* param;
  enum parameter_type type;
  void* data;
  size_t size;
  SCM ret;

  scm_dynwind_begin(0);

  if(!scm_is_string(_param))
    scm_wrong_type_arg(SYM_GNUFDISK_PARTITION_GET_PARAMETER, 2, _param);
  
  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
  
  param = scm_to_gnufdisk_string(_param);
  scm_dynwind_unwind_handler(&delete_string, param, 0);

  type = scm_symbol_to_parameter_type(_type);

  if(type == PARAMETER_TYPE_INTEGER)
    {
      data = scm_malloc(sizeof(gnufdisk_integer));
      size = sizeof(gnufdisk_integer);
      scm_dynwind_free(data);
    }
  else if(type == PARAMETER_TYPE_STRING)
    {
      data = gnufdisk_string_new("");
      size = sizeof(struct gnufdisk_string*);
      scm_dynwind_unwind_handler(&delete_string, data, SCM_F_WIND_EXPLICITLY);
    }
  else
    scm_wrong_type_arg(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 3, _type);

  if(gnufdisk_devicemanager_partition_get_parameter(dm, part, param, data, size) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_GET_PARAMETER,
	      "cannot get parameter",
	      SCM_EOL, SCM_UNDEFINED);
 
  if(type == PARAMETER_TYPE_INTEGER)
   ret = scm_from_long_long(*(gnufdisk_integer*) data);
  else 
   ret = scm_from_locale_string(gnufdisk_string_c_string(data));

  scm_dynwind_end();

  gnufdisk_string_delete(param);

  return ret;
}

static SCM scheme_partition_type(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  struct gnufdisk_string* type;
  SCM ret;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);

  if((type = gnufdisk_devicemanager_partition_type(dm, part)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_TYPE,
	      "cannot get type",
	      SCM_EOL, SCM_UNDEFINED);
  
  ret = scm_from_locale_string(gnufdisk_string_c_string(type));

  gnufdisk_string_delete(type);

  return ret;
}

static SCM scheme_partition_geometry(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  gnufdisk_integer start;
  gnufdisk_integer length;
  struct gnufdisk_geometry* geometry;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
 
  if((start = gnufdisk_devicemanager_partition_start(dm, part)) == -1
    || (length = gnufdisk_devicemanager_partition_length(dm, part)) == -1 
    || (geometry = gnufdisk_devicemanager_geometry_new(dm, start, length)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_GEOMETRY,
	      "cannot get geometry",
	      SCM_EOL, SCM_UNDEFINED);
 
  return scheme_geometry_new(geometry, scheme_partition_to_gnufdisk_devicemanager(_smob));
}

static SCM scheme_partition_number(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  int number;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);

  if((number = gnufdisk_devicemanager_partition_number(dm, part)) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_NUMBER,
	      "cannot get number",
	      SCM_EOL, SCM_UNDEFINED);
 
  return scm_from_int(number);
}

static SCM scheme_partition_have_disklabel(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  int have_disklabel;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);

  if((have_disklabel = gnufdisk_devicemanager_partition_have_disklabel(dm, part)) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_HAVE_DISKLABEL,
	      "cannot check wether partition have a disklabel",
	      SCM_EOL, SCM_UNDEFINED);
 
  return have_disklabel == 0 ? SCM_BOOL_F : SCM_BOOL_T;
}

static SCM scheme_partition_disklabel(SCM _smob)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  struct gnufdisk_disklabel* disklabel;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);

  if((disklabel = gnufdisk_devicemanager_partition_disklabel(dm, part)) == NULL)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_DISKLABEL,
	      "cannot get disklabel",
	      SCM_EOL, SCM_UNDEFINED);
 
  return scheme_disklabel_new(disklabel, _smob);
}

static SCM scheme_partition_move(SCM _smob, SCM _start)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  struct gnufdisk_geometry* geom;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
  geom = scheme_geometry_to_gnufdisk_geometry(_start);

  if(gnufdisk_devicemanager_partition_move(dm, part, geom) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_MOVE,
	      "cannot move partition",
	      SCM_EOL, SCM_UNDEFINED);
 
  return SCM_BOOL_T;
}

static SCM scheme_partition_resize(SCM _smob, SCM _start)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  struct gnufdisk_geometry* geom;

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
  geom = scheme_geometry_to_gnufdisk_geometry(_start);

  if(gnufdisk_devicemanager_partition_resize(dm, part, geom) != 0)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_RESIZE,
	      "cannot resize partition",
	      SCM_EOL, SCM_UNDEFINED);
 
  return SCM_BOOL_T;
}

static SCM scheme_partition_read(SCM _smob, SCM _start, SCM _size)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  gnufdisk_integer start;
  size_t size;
  void* buf;

  scm_dynwind_begin(0);

  if(!scm_is_integer(_start))
    scm_wrong_type_arg(SYM_GNUFDISK_PARTITION_READ, 2, _size);

  if(!scm_is_integer(_size))
    scm_wrong_type_arg(SYM_GNUFDISK_PARTITION_READ, 3, _size);

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
  start = scm_to_long_long(_start);
  size = scm_to_size_t(_size);
  
  if((buf = malloc(size)) == NULL)
    scm_memory_error(SYM_GNUFDISK_PARTITION_READ);

  scm_dynwind_unwind_handler(&free, buf, 0);

  if(gnufdisk_devicemanager_partition_read(dm, part, start, buf, size) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_READ,
	      "cannot read from partition",
	      SCM_EOL, SCM_UNDEFINED);
  
  scm_dynwind_end();

  return scheme_raw_new(buf, size);
}

static SCM scheme_partition_write(SCM _smob, SCM _start, SCM _raw)
{
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_partition* part;
  gnufdisk_integer start;
  void* data;
  size_t size;

  if(!scm_is_integer(_start))
    scm_wrong_type_arg(SYM_GNUFDISK_PARTITION_WRITE, 2, _start);

  dm = scheme_partition_to_gnufdisk_devicemanager(_smob);
  part = scheme_partition_to_gnufdisk_partition(_smob);
  start = scm_to_long_long(_start);
  data = scheme_raw_data(_raw);
  size = scheme_raw_size(_raw);

  if(gnufdisk_devicemanager_partition_write(dm, part, start, data, size) == -1)
    scm_error(scm_from_locale_symbol("operation-failed"), 
	      SYM_GNUFDISK_PARTITION_WRITE,
	      "cannot write on partition",
	      SCM_EOL, SCM_UNDEFINED);

  return SCM_BOOL_T;
}

static SCM scheme_userinterface_set_hook(SCM _ui, SCM _hook, SCM _proc)
{
  struct {
    char *symbol;
    size_t offset;
  } symbol_map[] = {
#define OFFSET(_mbr) offsetof(struct gnufdisk_userinterface, _mbr)
    {HOOK_PRINT, OFFSET(print)},
    {HOOK_ERROR, OFFSET(error)},
    {HOOK_YES_NO, OFFSET(yes_no)},
    {HOOK_GET_PATH, OFFSET(get_path)},
    {HOOK_GET_DISKLABEL_SYSTEM, OFFSET(get_disklabel_system)},
    {HOOK_GET_GEOMETRY, OFFSET(get_geometry)},
    {HOOK_GET_PARTITION_TYPE, OFFSET(get_partition_type)}
#undef OFFSET
   };

  struct gnufdisk_userinterface* ui;
  char* symbol;
  int i;

  ui = scheme_userinterface_to_gnufdisk_userinterface(_ui);

  if(!scm_is_symbol(_hook))
    scm_wrong_type_arg(SYM_GNUFDISK_USERINTERFACE_SET_HOOK, 2, _hook);
  else if(scm_procedure_p(_proc) != SCM_BOOL_T)
    scm_wrong_type_arg(SYM_GNUFDISK_USERINTERFACE_SET_HOOK, 3, _proc);

  scm_dynwind_begin(0);

  symbol = scm_to_locale_string(scm_symbol_to_string(_hook));
  scm_dynwind_unwind_handler(&free, symbol, 0);

  for(i = 0; i < sizeof(symbol_map) / sizeof(symbol_map[0]); i++)
    if(strcmp(symbol_map[i].symbol, symbol) == 0)
      {
        char* addr;

        addr = ((char*) ui) + symbol_map[i].offset;

        *((SCM*) addr) = _proc;

        break;
      }

  if(i >= sizeof(symbol_map) / sizeof(symbol_map[0]))
    scm_out_of_range(SYM_GNUFDISK_USERINTERFACE_SET_HOOK, _hook);
 
  scm_dynwind_end();

  free(symbol);

  return SCM_BOOL_T;
}

/* expect ui, _message */
static SCM scheme_userinterface_print_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_print_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->print == NULL || !scm_is_true(scm_procedure_p(ui->print)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_PRINT);
    }

  return scm_call_1(ui->print, scm_from_locale_string(message));
}

int gnufdisk_userinterface_internals__print(struct gnufdisk_userinterface* _ui,
					    const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  int err;
  int ret;
  SCM stack;
  SCM res;

  message = NULL;
  args = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = -1;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_print_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_bool(res) || !scm_is_true(res)) 
    {
      err = ECANCELED;
      ret = -1;
      goto lb_out;
    }

  err = 0;
  ret = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  errno = err;
  return ret;
}

static SCM scheme_userinterface_error_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_error_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->error == NULL || !scm_is_true(scm_procedure_p(ui->error)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_ERROR);
    }

  return scm_call_1(ui->error, scm_from_locale_string(message));
}

int gnufdisk_userinterface_internals__error(struct gnufdisk_userinterface* _ui,
					    const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  int err;
  int ret;
  SCM stack;
  SCM res;

  message = NULL;
  args = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = -1;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_error_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_bool(res) || !scm_is_true(res)) 
    {
      err = ECANCELED;
      ret = -1;
      goto lb_out;
    }

  err = 0;
  ret = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  errno = err;
  return ret;
}

static SCM scheme_userinterface_yes_no_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_yes_no_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->yes_no == NULL || !scm_is_true(scm_procedure_p(ui->yes_no)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_YES_NO);
    }

  return scm_call_1(ui->yes_no, scm_from_locale_string(message));
}

int gnufdisk_userinterface_internals__yes_no(struct gnufdisk_userinterface* _ui,
					    const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  int err;
  int ret;
  SCM stack;
  SCM res;

  message = NULL;
  args = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = -1;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_yes_no_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_bool(res)) 
    {
      err = ECANCELED;
      ret = -1;
      goto lb_out;
    }
  
  ret = scm_is_true(res) ? 1 : 0;
  err = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  errno = err;
  return ret;
}

static SCM scheme_userinterface_get_path_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_get_path_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->get_path == NULL || !scm_is_true(scm_procedure_p(ui->get_path)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_GET_PATH);
    }

  return scm_call_1(ui->get_path, scm_from_locale_string(message));
}

struct gnufdisk_string *
gnufdisk_userinterface_internals__get_path(struct gnufdisk_userinterface* _ui,
			                   const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  SCM stack;
  SCM res;
  char* string;
  struct gnufdisk_string* ret;
  int err;

  message = NULL;
  args = NULL;
  string = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = NULL;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = NULL;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0)
    {
      err = errno;
      ret = NULL;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_get_path_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_string(res)) 
    {
      err = ECANCELED;
      ret = NULL;
      goto lb_out;
    }

  string = scm_to_locale_string(res);
  ret = gnufdisk_string_new(string);

  err = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  if(string)
    free(string);

  errno = err;
  return ret;
}

static SCM scheme_userinterface_get_disklabel_system_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_get_disklabel_system_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->get_disklabel_system == NULL || !scm_is_true(scm_procedure_p(ui->get_disklabel_system)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_GET_DISKLABEL_SYSTEM);
    }

  return scm_call_1(ui->get_disklabel_system, scm_from_locale_string(message));
}

struct gnufdisk_string *
gnufdisk_userinterface_internals__get_disklabel_system(struct gnufdisk_userinterface* _ui,
			                               const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  SCM stack;
  SCM res;
  char* string;
  struct gnufdisk_string* ret;
  int err;

  message = NULL;
  args = NULL;
  string = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = NULL;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = NULL;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0)
    {
      err = errno;
      ret = NULL;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_get_disklabel_system_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_string(res)) 
    {
      err = ECANCELED;
      ret = NULL;
      goto lb_out;
    }

  string = scm_to_locale_string(res);
  ret = gnufdisk_string_new(string);

  err = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  if(string)
    free(string);

  errno = err;
  return ret;
}

static SCM scheme_userinterface_get_geometry_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  struct gnufdisk_devicemanager* dm;
  struct gnufdisk_geometry* geom;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_get_geometry_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &dm, sizeof(struct gnufdisk_devicemanager*));
  gnufdisk_stack_pop(args, &geom, sizeof(struct gnufdisk_geometry*));
  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->get_geometry == NULL || !scm_is_true(scm_procedure_p(ui->get_geometry)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_GET_GEOMETRY);
    }

  return scm_call_2(ui->get_geometry, 
		    scm_from_locale_string(message), 
		    scheme_geometry_new(geom,   dm));
}

int
gnufdisk_userinterface_internals__get_geometry(struct gnufdisk_userinterface* _ui,
					       struct gnufdisk_devicemanager* _dm,
					       struct gnufdisk_geometry* _geom,
			                       const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  struct gnufdisk_geometry* tmpgeom;
  SCM stack;
  SCM res;
  int err;
  int ret;

  args = NULL;
  message = NULL;
  tmpgeom = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = -1;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if((tmpgeom = gnufdisk_devicemanager_geometry_duplicate(_dm, _geom)) == NULL)
    {
      err = errno;
      ret = -1;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0
     || gnufdisk_stack_push(args, &tmpgeom, sizeof(struct gnufdisk_geometry*)) != 0
     || gnufdisk_stack_push(args, &_dm, sizeof(struct gnufdisk_devicemanager*)) != 0)
    {
      gnufdisk_devicemanager_geometry_delete(_dm, tmpgeom);
      err = errno;
      ret = -1;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_get_geometry_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_bool(res) || !scm_is_true(res))
    {
      gnufdisk_devicemanager_geometry_delete(_dm, tmpgeom);
      err = ECANCELED;
      ret = -1;
      goto lb_out;
    }


  gnufdisk_devicemanager_geometry_set(_dm,
				      _geom, 
				      gnufdisk_devicemanager_geometry_start(_dm, tmpgeom),
				      gnufdisk_devicemanager_geometry_length(_dm, tmpgeom));
  ret = 0;
  err = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  errno = err;
  return ret;
}

static SCM scheme_userinterface_get_partition_type_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  const char* message;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_partition_type_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &message, sizeof(char*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(ui->get_partition_type == NULL || !scm_is_true(scm_procedure_p(ui->get_partition_type)))
    {
      errno = ENOTSUP;
      scm_syserror(SYM_GNUFDISK_USERINTERFACE_GET_PARTITION_TYPE);
    }

  return scm_call_1(ui->get_partition_type, scm_from_locale_string(message));
}

struct gnufdisk_string *
gnufdisk_userinterface_internals__get_partition_type(struct gnufdisk_userinterface* _ui,
			                             const char* _fmt, va_list _args)
{
  struct gnufdisk_stack* args;
  char* message;
  SCM stack;
  SCM res;
  char* string;
  struct gnufdisk_string* ret;
  int err;

  message = NULL;
  args = NULL;
  string = NULL;

  if(gnufdisk_vasprintf(&message, _fmt, _args) == -1) 
    {
      ret = NULL;
      goto lb_out;
    }

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = NULL;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &message, sizeof(char*)) != 0)
    {
      err = errno;
      ret = NULL;
      goto lb_out;
    }

  res = scm_c_catch (SCM_BOOL_T, &scheme_userinterface_get_partition_type_thunk, args,
		     &scheme_catch_handler, stack, 
		     &scheme_preunwind_catch_handler, &stack);

  if(!scm_is_string(res)) 
    {
      err = ECANCELED;
      ret = NULL;
      goto lb_out;
    }

  string = scm_to_locale_string(res);
  ret = gnufdisk_string_new(string);

  err = 0;

lb_out:

  if(message)
    free(message);

  if(args)
    gnufdisk_stack_delete(args);

  if(string)
    free(string);

  errno = err;
  return ret;
}

/* expect _ui, _implementation, _argc, _argv on struct gnufdisk_stack* _p */
static SCM scheme_userinterface_run_thunk(void* _p)
{
  struct gnufdisk_stack* args;
  struct gnufdisk_userinterface* ui;
  struct gnufdisk_string* implementation;
  int argc;
  char** argv;

  if(gnufdisk_check_memory(_p, 1, 1) != 0)
    scm_syserror("scheme_userinterface_run_thunk");

  args = _p;

  gnufdisk_stack_pop(args, &argv, sizeof(char**));
  gnufdisk_stack_pop(args, &argc, sizeof(int));
  gnufdisk_stack_pop(args, &implementation, sizeof(struct gnufdisk_string*));
  gnufdisk_stack_pop(args, &ui, sizeof(struct gnufdisk_userinterface*));

  if(strcmp(gnufdisk_string_c_string(implementation), "shell") == 0)
    ui->shell_mode = 1;

  scheme_export_env(ui, argc, argv);

  if(ui->shell_mode)
    {
      int dummy_argc = 1;
      char* dummy_argv[] = {"gnufdisk", NULL};

      GNUFDISK_LOG((GUILE, "run struct gnufdisk_userinterface* %p in shell mode", ui));
      scm_shell(dummy_argc, dummy_argv);
    }
  else
    {
      GNUFDISK_LOG((GUILE, "run struct gnufdisk_userinterface* %p with implementation `%s'", 
		    ui, gnufdisk_string_c_string(implementation)));
      
      scm_c_primitive_load(gnufdisk_string_c_string(implementation));
    }

  return SCM_BOOL_T;
}

static void* scheme_main(void* _arg)
{
  SCM res;
  SCM stack;
  struct thunk_argument_list* args;

  args = _arg;
  stack = NULL;

  res = scm_c_catch (SCM_BOOL_T, scheme_userinterface_run_thunk, _arg, 
                     scheme_catch_handler, stack, scheme_preunwind_catch_handler, &stack);

  if(!scm_is_bool(res))
    {
      GNUFDISK_WARNING("bad return from scheme main loop");
      return (void*) 0x01;
    }

  if(scm_is_false(res))
     return (void*) 0x01;

  return NULL;
}

int gnufdisk_userinterface_internals__run(struct gnufdisk_userinterface* _ui,
					  struct gnufdisk_string* _implementation,
					  int _argc,
					  char** _argv)
{
  struct gnufdisk_stack* args;
  int ret;
  int err;

  args = NULL;

  if((args = gnufdisk_stack_new()) == NULL)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if(gnufdisk_stack_push(args, &_ui, sizeof(struct gnufdisk_userinterface*)) != 0
     || gnufdisk_stack_push(args, &_implementation, sizeof(struct gnufdisk_string*)) != 0
     || gnufdisk_stack_push(args, &_argc, sizeof(int)) != 0
     || gnufdisk_stack_push(args, &_argv, sizeof(char**)) != 0)
    {
      err = errno;
      ret = -1;
      goto lb_out;
    }

  if((err = (int) scm_with_guile(scheme_main, args)) != 0)
    ret = -1;

lb_out:
  
  if(args)
    gnufdisk_stack_delete(args);

  errno = err;
  return ret;
}

static void scheme_export_env(struct gnufdisk_userinterface* _ui, int argc, char** _argv)
{
  SCM command_line;

  /* init C rappresentation of SCM objects */
  scheme_object_bits = scm_make_smob_type("gnufdisk-object", sizeof(struct scheme_object));
  scm_set_smob_mark(scheme_object_bits, &scheme_object_mark);
  scm_set_smob_free(scheme_object_bits, &scheme_object_free);
  scm_set_smob_print(scheme_object_bits, &scheme_object_print);
  scm_set_smob_equalp(scheme_object_bits, &scheme_object_equalp);

  if(_ui->shell_mode)
    scm_c_define_gsubr(SYM_GNUFDISK_HELP, 0, 0, 0, (SCM (*)()) &scheme_gnufdisk_help);
  
  scm_c_define_gsubr(SYM_GNUFDISK_USERINTERFACE_SET_HOOK, 3, 0, 0, (SCM (*)()) &scheme_userinterface_set_hook);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICEMANAGER_MAKE_GEOMETRY, 3, 0, 0, (SCM (*)()) &scheme_devicemanager_make_geometry);
  scm_c_define_gsubr(SYM_GNUFDISK_GEOMETRY_P, 1, 0, 0, (SCM (*)()) &scheme_geometry_p);
  scm_c_define_gsubr(SYM_GNUFDISK_GEOMETRY_SET, 3, 0, 0, (SCM (*)()) &scheme_geometry_set);
  scm_c_define_gsubr(SYM_GNUFDISK_GEOMETRY_START, 1, 0, 0, (SCM (*)()) &scheme_geometry_start);
  scm_c_define_gsubr(SYM_GNUFDISK_GEOMETRY_END, 1, 0, 0, (SCM (*)()) &scheme_geometry_end);
  scm_c_define_gsubr(SYM_GNUFDISK_GEOMETRY_LENGTH, 1, 0, 0, (SCM (*)()) &scheme_geometry_length);
  scm_c_define_gsubr(SYM_GNUFDISK_MAKE_DEVICEMANAGER, 1, 0, 0, (SCM (*)()) &scheme_make_devicemanager);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICEMANAGER_P, 1, 0, 0, (SCM (*)()) &scheme_devicemanager_p);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICEMANAGER_MAKE_DEVICE, 3, 0, 0, (SCM (*)()) &scheme_devicemanager_make_device);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_P, 1, 0, 0, (SCM (*)()) &scheme_device_p);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_OPEN, 2, 0, 0, (SCM (*)()) &scheme_device_open);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_DISKLABEL, 1, 0, 0, (SCM (*)()) &scheme_device_disklabel);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_CREATE_DISKLABEL, 2, 0, 0, (SCM (*)()) &scheme_device_create_disklabel);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_SET_PARAMETER, 3, 0, 0, (SCM (*)()) &scheme_device_set_parameter);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_GET_PARAMETER, 3, 0, 0, (SCM (*)()) &scheme_device_get_parameter);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_COMMIT, 1, 0, 0, (SCM (*)()) &scheme_device_commit);
  scm_c_define_gsubr(SYM_GNUFDISK_DEVICE_CLOSE, 1, 0, 0, (SCM (*)()) &scheme_device_close);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_P, 1, 0, 0, (SCM (*)()) &scheme_disklabel_p);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_RAW, 1, 0, 0, (SCM (*)()) &scheme_disklabel_raw);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_SYSTEM, 1, 0, 0, (SCM (*)()) &scheme_disklabel_system);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_PARTITION, 2, 0, 0, (SCM (*)()) &scheme_disklabel_partition);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_COUNT_PARTITIONS, 1, 0, 0, (SCM (*)()) &scheme_disklabel_count_partitions);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_P, 1, 0, 0, (SCM (*)()) &scheme_partition_p);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_CREATE_PARTITION, 4, 0, 0, (SCM (*)()) &scheme_disklabel_create_partition);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_REMOVE_PARTITION, 2, 0, 0, (SCM (*)()) &scheme_disklabel_remove_partition);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_SET_PARAMETER, 3, 0, 0, (SCM (*)()) &scheme_disklabel_set_parameter);
  scm_c_define_gsubr(SYM_GNUFDISK_DISKLABEL_GET_PARAMETER, 3, 0, 0, (SCM (*)()) &scheme_disklabel_get_parameter);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_SET_PARAMETER, 3, 0, 0, (SCM (*)()) &scheme_partition_set_parameter);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_GET_PARAMETER, 3, 0, 0, (SCM (*)()) &scheme_partition_get_parameter);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_TYPE, 1, 0, 0, (SCM (*)()) &scheme_partition_type);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_GEOMETRY, 1, 0, 0, (SCM (*)()) &scheme_partition_geometry);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_NUMBER, 1, 0, 0, (SCM (*)()) &scheme_partition_number);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_HAVE_DISKLABEL, 1, 0, 0, (SCM (*)()) &scheme_partition_have_disklabel);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_DISKLABEL, 1, 0, 0, (SCM (*)()) &scheme_partition_disklabel);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_MOVE, 2, 0, 0, (SCM (*)()) &scheme_partition_move);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_RESIZE, 2, 0, 0, (SCM (*)()) &scheme_partition_resize);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_READ, 3, 0, 0, (SCM (*)()) &scheme_partition_read);
  scm_c_define_gsubr(SYM_GNUFDISK_PARTITION_WRITE, 3, 0, 0, (SCM (*)()) &scheme_partition_write);
  scm_c_define_gsubr(SYM_GNUFDISK_MAKE_RAW, 1, 0, 0, (SCM (*)()) &scheme_make_raw);
  scm_c_define_gsubr(SYM_GNUFDISK_RAW_P, 1, 0, 0, (SCM (*)()) &scheme_raw_p);
  scm_c_define_gsubr(SYM_GNUFDISK_RAW_REF, 2, 0, 0, (SCM (*)()) &scheme_raw_ref);
  scm_c_define_gsubr(SYM_GNUFDISK_RAW_SET_X, 3, 0, 0, (SCM (*)()) &scheme_raw_set_x);
  scm_c_define_gsubr(SYM_GNUFDISK_RAW_LENGTH, 1, 0, 0, (SCM (*)()) &scheme_raw_length);

  scm_c_define(SYM_USERINTERFACE, scheme_userinterface_new(_ui));

  for(command_line = SCM_EOL; *_argv; _argv++)
    command_line = scm_append (scm_list_2 (command_line, scm_list_1(scm_from_locale_string(*_argv))));

  scm_c_define(SYM_COMMANDLINE, command_line);
}

