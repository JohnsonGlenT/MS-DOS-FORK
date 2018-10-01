#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <blkid/blkid.h>

#include "common.h"

enum device_type {
  DEVICE_TYPE_FILE,
  DEVICE_TYPE_IDE,
  DEVICE_TYPE_SCSI,
  DEVICE_TYPE_UNKNOWN
};

static int is_ide_major(int _major)
{
  int ret;

  switch(_major)
    {
    case 3:
    case 22:
    case 33:
    case 34:
    case 56:
    case 57:
      ret = 1;
      break;
    default:
      ret = 0;
    }

  return ret;
}

static int is_scsi_major(int _major)
{
  int ret;

  switch(_major)
    {
      case 8:
      case 11:
      case 65:
      case 66:
      case 67:
      case 68:
      case 69:
      case 70:
      case 71:
      case 128:
      case 129:
      case 130:
      case 131:
      case 132:
      case 133:
      case 134:
      case 135:
	ret = 1;
	break;
      default:
	ret = 0;
    }

  return ret;
}

static int get_device_type(struct stat* _info)
{
  int maj;
  int min;
  int ret;

  maj = major(_info->st_rdev);
  min = minor(_info->st_rdev);

  if(is_ide_major(maj) && (min % 0x40) == 0)
    ret = DEVICE_TYPE_IDE;
  else if(is_scsi_major(maj) && (min % 0x10) == 0)
    ret = DEVICE_TYPE_SCSI;
  else if(S_ISREG(_info->st_mode))
    ret = DEVICE_TYPE_FILE;
  else
    ret = DEVICE_TYPE_UNKNOWN;

  return ret;
}

struct linux_device_private {
  int fd;
  enum device_type type;
  gnufdisk_integer cylinders;
  gnufdisk_integer heads;
  gnufdisk_integer sectors;
  gnufdisk_integer sector_size;
  gnufdisk_integer minimal_io;
  gnufdisk_integer optimal_io;
  gnufdisk_integer size;
};

static void linux_device_private_check(struct linux_device_private* _private)
{
  if(gnufdisk_check_memory(_private, sizeof(struct linux_device_private), 0) != 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid struct linux_device_private* %p", _private);

  if(_private->fd < 0)
    GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EINTERNAL, NULL, "invalid file descriptor: %d", _private->fd);
}

static gnufdisk_integer linux_device_start(void* _private)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform start on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = 0;

  GNUFDISK_LOG((DEVICE, "done perform start, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_end(void* _private)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform end on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = private->size / private->sector_size;

  GNUFDISK_LOG((DEVICE, "done perform end, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_seek(void* _private, gnufdisk_integer _lba, gnufdisk_integer _offset, int _whence)
{
  struct linux_device_private* private;
  gnufdisk_integer offset;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform seek on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  offset = _lba * private->sector_size + _offset;

  GNUFDISK_LOG((DEVICE, "real offset: %" PRId64, offset));

  ret = lseek(private->fd, offset, _whence);

  GNUFDISK_LOG((DEVICE, "done perform seek, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_read(void* _private, void* _buf, size_t _size)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform read on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = read(private->fd, _buf, _size);

  GNUFDISK_LOG((DEVICE, "done perform read, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_write(void* _private, const void* _buf, size_t _size)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform write on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = write(private->fd, _buf, _size);

  GNUFDISK_LOG((DEVICE, "done perform write, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_sector_size(void* _private)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform sector_size on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = private->sector_size;

  GNUFDISK_LOG((DEVICE, "done perform sector_size, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_minimum_alignment(void* _private)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform minimum_alignment on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = private->sector_size;

  GNUFDISK_LOG((DEVICE, "done perform minimum_alignment, result: %" PRId64, ret));

  return ret;
}

static gnufdisk_integer linux_device_optimal_alignment(void* _private)
{
  struct linux_device_private* private;
  gnufdisk_integer ret;

  GNUFDISK_LOG((DEVICE, "perform optimal_alignment on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  ret = private->sector_size;

  GNUFDISK_LOG((DEVICE, "done perform optimal_alignment, result: %" PRId64, ret));

  return ret;
}

static void linux_device_set_parameter(void *_private, struct gnufdisk_string* _param, void* _data, size_t _size)
{
  struct linux_device_private* private;
  char* param;

  GNUFDISK_LOG((DEVICE, "perform set_parameter on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);
 
  private = _private;

  if((param = gnufdisk_string_c_string_dup(_param)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, param);

  GNUFDISK_LOG((DEVICE, "parameter: %s, size: %u", param, _size));

  if(strcasecmp(param, "CYLINDERS") == 0)
    {
      if(_size != sizeof(gnufdisk_integer))
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      private->cylinders = *(gnufdisk_integer*) _data;
    }
  else if(strcasecmp(param, "HEADS") == 0)
    {
      if(_size != sizeof(gnufdisk_integer))
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      private->heads = *(gnufdisk_integer*) _data;
    }
  else if(strcasecmp(param, "SECTORS") == 0)
    {
      if(_size != sizeof(gnufdisk_integer))
       GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      private->sectors = *(gnufdisk_integer*) _data;
    }
  else if(strcasecmp(param, "SECTOR-SIZE") == 0)
    {
      if(_size != sizeof(gnufdisk_integer))
       GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      private->sector_size = *(gnufdisk_integer*) _data;
    }
  else
   GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETER, NULL, "invalid parameter: %s", param);

  gnufdisk_exception_unregister_unwind_handler(&free, param);

  free(param);

  GNUFDISK_LOG((DEVICE, "done perform set_parameter"));
}

static void linux_device_get_parameter(void *_private, struct gnufdisk_string* _param, void* _dest, size_t _size)
{
  struct linux_device_private* private;
  char* param;

  GNUFDISK_LOG((DEVICE, "perform get_parameter on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);
 
  private = _private;

  if((param = gnufdisk_string_c_string_dup(_param)) == NULL)
    THROW_ENOMEM;

  gnufdisk_exception_register_unwind_handler(&free, param);

  GNUFDISK_LOG((DEVICE, "parameter: %s, size: %u", param, _size));

  if(strcasecmp(param, "CYLINDERS") == 0)
    {
      GNUFDISK_RETRY rp0;

      if(_size != sizeof(gnufdisk_integer))
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      GNUFDISK_RETRY_SET(rp0);

      if(private->cylinders == 0)
        {
          union gnufdisk_device_exception_data data;

          data.ecylinders = &private->cylinders;

          GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
                         &rp0,
                         GNUFDISK_DEVICE_ECYLINDERS,
                         &data,
                         "can not determine device cylinders");
        }

      *((gnufdisk_integer*) _dest) = private->cylinders;
    }
  else if(strcasecmp(param, "HEADS") == 0)
    {
      GNUFDISK_RETRY rp0;

      if(_size != sizeof(gnufdisk_integer))
	GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      GNUFDISK_RETRY_SET(rp0);

      if(private->heads == 0)
        {
          union gnufdisk_device_exception_data data;

          data.eheads = &private->heads;

          GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
                         &rp0,
                         GNUFDISK_DEVICE_EHEADS,
                         &data,
                         "can not determine device heads");
        }

      *((gnufdisk_integer*) _dest) = private->heads;
    }
  else if(strcasecmp(param, "SECTORS") == 0)
    {
      GNUFDISK_RETRY rp0;

      if(_size != sizeof(gnufdisk_integer))
       GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      GNUFDISK_RETRY_SET(rp0);

      if(private->sectors == 0)
        {
          union gnufdisk_device_exception_data data;

          data.esectors = &private->sectors;

          GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
                         &rp0,
                         GNUFDISK_DEVICE_ESECTORS,
                         &data,
                         "can not determine device sectors");
        }

      *((gnufdisk_integer*) _dest) = private->sectors;
    }
  else if(strcasecmp(param, "SECTOR-SIZE") == 0)
    {
      GNUFDISK_RETRY rp0;

      if(_size != sizeof(gnufdisk_integer))
       GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETERSIZE, NULL, "invalid parameter size");

      if(private->sector_size == 0)
        {
          union gnufdisk_device_exception_data data;

          data.esectorsize = &private->sector_size;

          GNUFDISK_THROW(GNUFDISK_EXCEPTION_ALL,
                         &rp0,
                         GNUFDISK_DEVICE_ESECTORSIZE,
                         &data,
                         "can not determine device sector size");
        }

      *((gnufdisk_integer*) _dest) = private->sector_size;
    }
  else
   GNUFDISK_THROW(0, NULL, GNUFDISK_DEVICE_EPARAMETER, NULL, "invalid parameter: %s", param);

  gnufdisk_exception_unregister_unwind_handler(&free, param);

  free(param);

  GNUFDISK_LOG((DEVICE, "done perform get_parameter"));
}

static void linux_device_commit(void* _private)
{
  struct linux_device_private* private;

  GNUFDISK_LOG((DEVICE, "perform commit on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  GNUFDISK_LOG((DEVICE, "done perform linux_device_commit"));
}


static void linux_device_delete(void* _private)
{
  struct linux_device_private* private;

  GNUFDISK_LOG((DEVICE, "perform delete on struct linux_device_private* %p", _private));

  linux_device_private_check(_private);

  private = _private;

  GNUFDISK_LOG((DEVICE, "close file descriptor %d", private->fd));

  close(private->fd);

  memset(private, 0, sizeof(struct linux_device_private));
  free(private);

  GNUFDISK_LOG((DEVICE, "done perform delete"));
}

static struct device_implementation linux_device_implementation = {
    NULL, /* private */
    &linux_device_start,
    &linux_device_end,
    &linux_device_seek,
    &linux_device_read,
    &linux_device_write,
    &linux_device_sector_size,
    &linux_device_minimum_alignment,
    &linux_device_optimal_alignment,
    &linux_device_set_parameter,
    &linux_device_get_parameter,
    &linux_device_commit,
    &linux_device_delete
};

int linux_device_probe(const char* _path, struct module_options* _options, struct device_implementation* _implementation)
{
  struct stat info;
  struct linux_device_private* private;
  struct hd_geometry geometry;
  blkid_probe probe;
  blkid_topology topology;

  GNUFDISK_LOG((DEVICE, "perform linux_device_probe on %s", _path));

  private = NULL;
  probe = NULL;
  topology = NULL;

  if(stat(_path, &info) != 0)
    {
      GNUFDISK_LOG((DEVICE, "can not stat %s: %s", _path, strerror(errno)));
      goto lb_failure;
    }

  if((private = malloc(sizeof(struct linux_device_private))) == NULL)
    THROW_ENOMEM;

  memset(private, 0, sizeof(struct linux_device_private));

  if((private->type = get_device_type(&info)) == DEVICE_TYPE_UNKNOWN)
    {
      GNUFDISK_LOG((DEVICE, "unknown device type"));
      goto lb_failure;
    }

  if((private->fd = open(_path, _options->readonly ? O_RDONLY : O_RDWR)) == -1)
    {
      GNUFDISK_LOG((DEVICE, "error open %s: %s", _path, strerror(errno)));
      goto lb_failure;
    }
 
  /* probe geometry */
  if(ioctl(private->fd, HDIO_GETGEO, &geometry) != 0)
    {
      GNUFDISK_LOG((DEVICE, "error HDIO_GETGEO"));
      memset(&geometry, 0, sizeof(geometry));
    }

  private->heads = _options->heads ? _options->heads : geometry.heads;
  private->sectors = _options->sectors ? _options->sectors : geometry.sectors;
  private->cylinders = _options->cylinders ? _options->cylinders : geometry.cylinders;
  
  if((probe = blkid_new_probe_from_filename(_path)) == NULL
     || (topology = blkid_probe_get_topology(probe)) == NULL)
    {
      GNUFDISK_LOG((DEVICE, "error blkid"));
      private->sector_size = _options->sector_size ? _options->sector_size : 512;
      private->minimal_io = private->sector_size;
      private->optimal_io = private->sector_size;
    }
  else
    {
      private->sector_size = _options->sector_size ? _options->sector_size : blkid_topology_get_logical_sector_size(topology);
      private->minimal_io = blkid_topology_get_minimum_io_size(topology);
      private->optimal_io = blkid_topology_get_optimal_io_size(topology);
    }

  GNUFDISK_LOG((DEVICE, "device geometry:"));
  GNUFDISK_LOG((DEVICE, "\tcylinders    : %" PRId64, private->cylinders));
  GNUFDISK_LOG((DEVICE, "\theads        : %" PRId64, private->heads));
  GNUFDISK_LOG((DEVICE, "\tsectors      : %" PRId64, private->sectors));
  GNUFDISK_LOG((DEVICE, "\tsector_size  : %" PRId64, private->sector_size));
  GNUFDISK_LOG((DEVICE, "\tfile size    : %" PRId64, private->size));

  memcpy(_implementation, &linux_device_implementation, sizeof(struct device_implementation));
  _implementation->private = private;

  GNUFDISK_LOG((DEVICE, "done linux_device_probe"));
      
  if(probe)
    blkid_free_probe(probe);

  return 0;

lb_failure:

  if(probe)
    blkid_free_probe(probe);

  if(private)
    linux_device_delete(private);

  return -1;
}

