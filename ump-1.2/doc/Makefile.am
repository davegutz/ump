info_TEXINFOS = ump.texi
EXTRA_DIST    = exmask.ps exmask.txt exmask.png \
	cuts.lyx vision.bib cuts.pdf \
	error.ps exmask.ps focal.ps im139.ps im139+.ps \
	interpOXX.ps interpXOX.ps interpXXO.ps perim.ps \
	realsum.ps realvid0.ps realvid1.ps scrnshot.ps \
	strike.ps tempRCx.ps transform.ps triangulate.ps \
	video0ResM.ps video0TResM.ps video1ResM.ps \
	video1TResM.ps videoOneAa.ps videoOneFa10.ps \
	videoOneFa10+.ps xerror.ps yerror.ps zerror.ps ump.omf \
        pitchBoxplots.png  scatterWire.png  stillBoxPlots.png
SUFFIXES      = .html .xml .dvi
HTML          =--html
SUBDIRS       = ump

ump.dvi: ump.texi
	texi2dvi ump.texi

#ump.xml: ump.texi
#	makeinfo --xml ump.texi

ump.html: ump.texi
	makeinfo --html ump  && touch ump.html

#	@cd $(srcdir) && rm -f $@ $@-[0-9] $@-[0-9][0-9]
#	cd $(srcdir) \
#	 && $(MAKEINFO) $(HTML) `echo $< | sed 's,.*/,,'`

#install-data-hook:
#	sudo mkdir -p /usr/share/gnome/help/ump
#	sudo mkdir -p /usr/share/gnome/help/ump/C
#	sudo cp ump.xml /usr/share/gnome/help/ump/C
#	scrollkeeper-update ump.omf
