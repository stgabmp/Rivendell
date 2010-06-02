## automake.am
##
## icons/automake.am for Rivendell
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@salemradiolabs.com>
##
##      $Id: Makefile.am,v 1.16.2.2 2009/06/30 22:31:51 cvs Exp $
##      $Date: 2009/06/30 22:31:51 $
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

install-exec-am:	
	mkdir -p $(DESTDIR)@libexecdir@
	cp greencheckmark.png $(DESTDIR)@libexecdir@
	cp redx.png $(DESTDIR)@libexecdir@
	cp greenball.png $(DESTDIR)@libexecdir@
	cp redball.png $(DESTDIR)@libexecdir@
	cp whiteball.png $(DESTDIR)@libexecdir@
	cp progressbar.gif $(DESTDIR)@libexecdir@

uninstall:	
	rm -f $(DESTDIR)@libexecdir@/greencheckmark.png
	rm -f $(DESTDIR)@libexecdir@/redx.png
	rm -f $(DESTDIR)@libexecdir@/greenball.png
	rm -f $(DESTDIR)@libexecdir@/redball.png
	rm -f $(DESTDIR)@libexecdir@/whiteball.png
	rm -f $(DESTDIR)@libexecdir@/progressbar.gif

EXTRA_DIST = rivendell.res play.xpm macro.xpm\
             fillstart.xpm fillstop.xpm\
             note.xpm note2.xpm\
             post.xpm post2.xpm post3.xpm\
             record.xpm record2.xpm record3.xpm\
             rml.xpm rml2.xpm rml3.xpm rml5.xpm\
             switch.xpm switch2.xpm switch3.xpm\
             marker.xpm chain.xpm rivendell.ico\
             download.xpm upload.xpm track.xpm\
             checkmark.xpm ex.xpm track_cart.xpm\
             track_cart.xpm rivendell-16x16.xpm\
             greencheckmark.xpm redcheckmark.xpm\
             redx.xpm whiteball.xpm blueball.xpm\
             redball.xpm greenball.xpm mic16.xpm\
             music.xpm traffic.xpm notemarker.xpm\
             rivendell-22x22.xpm rivendell-32x32.xpm\
             rivendell-48x48.xpm rivendell-64x64.xpm\
             rivendell-128x128.xpm greencheckmark.png redx.png\
             greenball.png redball.png whiteball.png\
             user.xpm admin.xpm\
             progressbar.xcf progressbar.gif

CLEANFILES = *~
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4
