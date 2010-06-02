## automake.am
##
## Automake.am for rivendell/utils/rdgpimon
##
##   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.9.2.2 2010/01/21 17:11:48 cvs Exp $
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
	cp rdgpimon_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdgpimon_*.qm

all:	
	@QT_BIN@/lupdate rdgpimon.pro
	@QT_BIN@/lrelease rdgpimon.pro

bin_PROGRAMS = rdgpimon

dist_rdgpimon_SOURCES = rdgpimon.cpp rdgpimon.h\
                        gpi_label.cpp gpi_label.h

nodist_rdgpimon_SOURCES = moc_rdgpimon.cpp moc_gpi_label.cpp

rdgpimon_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

EXTRA_DIST = rdgpimon.pro rdgpimon_es.ts rdgpimon_fr.ts rdgpimon_de.ts\
             rdgpimon_nb.ts rdgpimon_nn.ts

CLEANFILES = *~ moc_* *.qm *.obj *.idb *.pdb *ilk
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
