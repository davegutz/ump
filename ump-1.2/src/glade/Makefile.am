AM_CFLAGS =   `pkg-config --cflags libgnomeui-2.0` \
              `pkg-config --cflags libglade-2.0`   \
              -Wall \
	      -DPREFIX=\""$(prefix)\""        -DSYSCONFDIR=\""$(prefix)/etc"\" \
	      -DDATADIR=\""$(prefix)/share/ump"\" -DLIBDIR=\""$(prefix)/lib"\"

AM_CXXFLAGS = `pkg-config --cflags libgnomeui-2.0` \
              `pkg-config --cflags libglade-2.0`   \
              -Wall \
	      -DPREFIX=\""$(prefix)\""        -DSYSCONFDIR=\""$(prefix)/etc"\" \
	      -DDATADIR=\""$(prefix)/share/ump"\" -DLIBDIR=\""$(prefix)/lib"\"

LIBS         =`pkg-config --libs libgnomeui-2.0` \
              `pkg-config --libs libglade-2.0`

INCLUDES     =-DGNOMELOCALEDIR=\""$(datadir)/locale/"\" \
	      -DG_LOG_DOMAIN=\"ump\" \
	      -I@top_srcdir@ -I@top_srcdir@/src/l -I@top_srcdir@/src \
	      $(GNOME_CFLAGS)

bin_PROGRAMS = Blink

Blink_SOURCES= Blink.cc callbacks.cc callbacks.h blink.glade State.cc State.h \
               MyMask.cc MyMask.h
Blink_LDADD  = ../l/InFile.o ../l/stringset.o ../l/makeargv.o ../Tune.o ../Mask.o -lm ../l/libump.a

gladedir     = $(pkgdatadir)
glade_DATA   = blink.glade Blink.png

EXTRA_DIST   = Blink.png

install-data-hook:
	sudo cp $(DESTDIR)$(datadir)/ump/Blink.png /usr/share/icons/hicolor/48x48/apps/.

