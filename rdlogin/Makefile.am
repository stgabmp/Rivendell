## automake.am
##
## Automake.am for rivendell/rdlogin
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.18.2.2 2010/01/21 17:11:48 cvs Exp $
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/lib
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib
MOC = @QT_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

# I18N Stuff
install-exec-local:	
	mkdir -p $(DESTDIR)$(prefix)/share/rivendell
	cp rdlogin_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdlogin_*.qm

all:	
	@QT_BIN@/lupdate rdlogin.pro
	@QT_BIN@/lrelease rdlogin.pro

bin_PROGRAMS = rdlogin

dist_rdlogin_SOURCES = rdlogin.cpp rdlogin.h
nodist_rdlogin_SOURCES = moc_rdlogin.cpp
rdlogin_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

EXTRA_DIST = rdlogin.pro rdlogin_es.ts rdlogin_fr.ts rdlogin_de.ts\
             rdlogin_nb.ts rdlogin_nn.ts

CLEANFILES = *~ moc_* *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
