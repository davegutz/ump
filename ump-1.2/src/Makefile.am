bin_PROGRAMS= ump sliceraw triangulate extCal convertTsai lptout lptoutd tim2clk
bin_SCRIPTS = checkSetup checkRealAll ump.x Ump killUmp killSty
AM_CXXFLAGS = -DNO_BOUNDS_CHECK $(PTHREAD_CFLAGS)
TESTS       = checkKill checkSetup checkTriangulate checkCal checkReal \
              checkFile checkOfile checkUmp checkKillEnd
EXTRA_DIST  = checkKill checkSetup checkTriangulate checkCal checkReal \
	      checkFile checkOfile checkUmp checkKillEnd               \
              checkRealAll ump.x Ump killUmp killSty
lptout_SOURCES      = lptout.c
lptoutd_SOURCES     = lptoutd.c
ump_SOURCES         = main.cc  PixelLoc.h PixelLoc.cc Tune.h Tune.cc Image.h \
		      Image.cc Segment.h Segment.cc Ball.h Ball.cc \
		      Mask.h Mask.cc
extCal_SOURCES      = extCal.cc PixelLoc.h PixelLoc.cc Tune.h Tune.cc \
	              Segment.h Segment.cc Image.h Image.cc Mask.h Mask.cc
convertTsai_SOURCES = convertTsai.cc PixelLoc.h PixelLoc.cc Tune.h Tune.cc \
	              Segment.h Segment.cc Image.h Image.cc Mask.h Mask.cc
sliceraw_SOURCES    = sliceraw.cc
triangulate_SOURCES = triangulate.cc PixelLoc.h PixelLoc.cc Tune.h Tune.cc \
                      Segment.h Segment.cc Image.h Image.cc Mask.h Mask.cc
tim2clk_SOURCES     = tim2clk.cc

SUBDIRS = l image_utils realstream glade

LDADD            = ./l/libump.a
ump_LDADD        = ./l/libump.a ./realstream/librealstr.a $(PTHREAD_LIBS)

INCLUDES     = -I@top_srcdir@ -I@top_srcdir@/src/l -I@top_srcdir@/src/l/tnt -I@top_srcdir@/src/realstream

install-exec-hook:
	sudo chown root $(DESTDIR)$(bindir)/lptout; chmod +s $(DESTDIR)$(bindir)/lptout;
