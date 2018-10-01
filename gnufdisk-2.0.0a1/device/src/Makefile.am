lib_LTLIBRARIES = libgnufdisk-device.la

libgnufdisk_device_la_SOURCES = $(top_srcdir)/include/gnufdisk-device.h $(top_srcdir)/include/gnufdisk-device-internals.h geometry.c device.c disklabel.c partition.c
libgnufdisk_device_la_CPPFLAGS = -I$(top_srcdir)/../common/include -I$(top_srcdir)/../debug/include -I$(top_srcdir)/../exception/include -I$(top_srcdir)/include
libgnufdisk_device_la_LIBADD = -L../../common/src -L../../exception/src -L../../debug/src -ldl -lgnufdisk-common -lgnufdisk-exception -lgnufdisk-debug 

