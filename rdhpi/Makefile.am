## automake.am
##
## Automake.am for rivendell/mlhpi
##
## by Fred Gleason <fredg@paravelsystems.com>
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.4.2.2 2010/01/21 17:11:47 cvs Exp $
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/lib
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib
MOC = @QT_MOC@
CWRAP = ../helpers/cwrap

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

# The cwrap dependency
html_%.cpp:	%.html
	$(CWRAP) -o $@ $<

instdir = @LOCAL_PREFIX@/lib

# I18N Stuff
install-exec-local:	
	mkdir -p $(DESTDIR)$(prefix)/share/rivendell
	cp rdhpi_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdhpi_*.qm

all:	
	@QT_BIN@/lupdate rdhpi.pro
	@QT_BIN@/lrelease rdhpi.pro

lib_LTLIBRARIES = librdhpi.la
dist_librdhpi_la_SOURCES = rdhpisoundcard.cpp rdhpisoundcard.h\
                           rdhpiplaystream.cpp rdhpiplaystream.h\
                           rdhpirecordstream.cpp rdhpirecordstream.h\
                           rdhpisoundselector.cpp rdhpisoundselector.h\
                           rdhpicardselector.cpp rdhpicardselector.h\
                           rdhpiinformation.cpp rdhpiinformation.h

nodist_librdhpi_la_SOURCES = moc_rdhpisoundcard.cpp moc_rdhpiplaystream.cpp\
                             moc_rdhpisoundselector.cpp\
                             moc_rdhpicardselector.cpp\
                             moc_rdhpirecordstream.cpp

librdhpi_la_LDFLAGS = -release $(VERSION)

EXTRA_DIST = rdhpi.pro rdhpi_es.ts rdhpi_fr.ts rdhpi_de.ts\
             rdhpi_nb.ts rdhpi_nn.ts

CLEANFILES = *~ moc_* *.obj *.lib *.idb *.pdb *.ilk *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
