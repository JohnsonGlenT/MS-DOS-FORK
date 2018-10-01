lib_LTLIBRARIES = libgnufdisk-devicemanager.la

libgnufdisk_devicemanager_la_SOURCES = ../include/gnufdisk-devicemanager.h devicemanager.c
libgnufdisk_devicemanager_la_CPPFLAGS = 	-I$(top_srcdir)/include \
				-I$(top_srcdir)/../common/include \
				-I$(top_srcdir)/../debug/include \
				-I$(top_srcdir)/../device/include \
				-I$(top_srcdir)/../exception/include \
				-I$(top_srcdir)/../userinterface/include

libgnufdisk_devicemanager_la_LIBADD = 	-L../../common/src \
				-L../../debug/src \
				-L../../exception/src \
				-L../../device/src \
				-L../../userinterface/src \
				-lgnufdisk-common -lgnufdisk-debug -lgnufdisk-exception -lgnufdisk-device

