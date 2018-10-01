bin_PROGRAMS = gnufdisk

gnufdisk_SOURCES = gnufdisk.c

gnufdisk_CPPFLAGS = 	-I$(top_srcdir)/common/include \
			-I$(top_srcdir)/debug/include \
			-I$(top_srcdir)/exception/include \
			-I$(top_srcdir)/device/include \
			-I$(top_srcdir)/userinterface/include

gnufdisk_LDFLAGS = -shared

gnufdisk_LDADD = 	-L../common/src \
			-L../debug/src \
			-L../exception/src \
			-L../userinterface/src \
			-lgnufdisk-common -lgnufdisk-debug -lgnufdisk-exception -lgnufdisk-userinterface

