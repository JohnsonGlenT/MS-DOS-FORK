lib_LTLIBRARIES = gnufdisk-backend.la
gnufdisk_backend_la_CPPFLAGS = -I$(top_srcdir)/../device/include
gnufdisk_backend_la_SOURCES = common.h endianness.c math.c list.c object.c device.c linux.c disklabel.c mbr.c ebr.c gpt.c partition.c primary.c extended.c logical.c guid.c
gnufdisk_backend_la_LDFLAGS = -module
gnufdisk_backend_la_LIBADD = -luuid -lblkid

ACLOCAL_AMFLAGS = -I m4
