lib_LTLIBRARIES=libgnufdisk-userinterface.la


libgnufdisk_userinterface_la_SOURCES = $(top_srcdir)/include/gnufdisk-userinterface.h internals.h userinterface.c guile.c


libgnufdisk_userinterface_la_CPPFLAGS=	-I$(top_srcdir)/../common/include\
                                        -I$(top_srcdir)/../debug/include \
                                        -I$(top_srcdir)/../exception/include \
                                        -I$(top_srcdir)/../device/include \
                                        -I$(top_srcdir)/../devicemanager/include \
                                        -I$(top_srcdir)/include

libgnufdisk_userinterface_la_CFLAGS=@GUILE_CFLAGS@

libgnufdisk_userinterface_la_LIBADD=	-L../../common/src \
                                      -L../../debug/src \
                                      -L../../exception/src \
                                      -L../../device/src \
                                      -L../../devicemanager/src \
                                      -lgnufdisk-common \
                                      -lgnufdisk-debug \
                                      -lgnufdisk-exception \
                                      -lgnufdisk-device \
                                      -lgnufdisk-devicemanager 

libgnufdisk_userinterface_la_LDFLAGS=@GUILE_LDFLAGS@

