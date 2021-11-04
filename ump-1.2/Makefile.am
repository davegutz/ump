SUBDIRS = src data scripts doc config

TEXINFO_TEX=./doc/texinfo.tex

EXTRA_DIST = common.h TODO exmask.ps autogen.sh  \
             Blink.desktop  Blink.desktop.in

noinst_SCRIPTS=autogen.sh

desktopdir = $(datadir)/applications
desktop_in_files = Blink.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)




