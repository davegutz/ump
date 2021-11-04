noinst_LIBRARIES = librealstr.a

librealstr_a_SOURCES = \
 graby.c grab-v4ly.c grab-v4l2y.c colorspace.c\
 frequencies.c writefile.c commands.c parseconfig.c capturey.c\
 grab.h colorspace.h byteorder.h videodev.h videodev2.h \
 frequencies.h writefile.h commands.h parseconfig.h capturey.h \
 channel.h  realStream.cc realStream.h grab-v4l-stat.h

channel-no-x11.o: channel.c
	$(CC) $(CFLAGS) -DNO_X11=1 -c -o $@ $<

librealstr_a_LIBADD =  channel-no-x11.o  ../l/libump.a

EXTRA_DIST = channel.c

INCLUDES     = -I@top_srcdir@ -I@top_srcdir@/src/l

bin_PROGRAMS      = streamery

streamery_SOURCES = streamery.c graby.c grab-v4ly.c grab-v4l2y.c colorspace.c\
frequencies.c writefile.c commands.c parseconfig.c capturey.c\
grab.h colorspace.h byteorder.h videodev.h videodev2.h \
frequencies.h writefile.h commands.h parseconfig.h capturey.h\
channel.h

streamery_LDADD   =   channel-no-x11.o -lpthread -lm
