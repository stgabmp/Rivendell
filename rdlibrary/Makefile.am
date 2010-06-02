## automake.am
##
## Automake.am for rivendell/rdlibrary
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.47.2.4 2010/01/21 17:11:47 cvs Exp $
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
	cp rdlibrary_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdlibrary_*.qm

all:	
	@QT_BIN@/lupdate rdlibrary.pro
	@QT_BIN@/lrelease rdlibrary.pro

bin_PROGRAMS = rdlibrary

dist_rdlibrary_SOURCES = rdlibrary.cpp rdlibrary.h\
                         filter.cpp filter.h\
                         edit_cart.cpp edit_cart.h\
                         audio_cart.cpp audio_cart.h\
                         macro_cart.cpp macro_cart.h\
                         record_cut.cpp record_cut.h\
                         edit_macro.cpp edit_macro.h\
                         cdripper.cpp cdripper.h\
                         disk_gauge.cpp disk_gauge.h\
                         disk_bar.cpp disk_bar.h\
                         disk_ripper.cpp disk_ripper.h\
                         audio_controls.h\
                         list_reports.cpp list_reports.h\
                         validate_cut.cpp validate_cut.h\
                         edit_schedulercodes.cpp edit_schedulercodes.h\
                         edit_notes.cpp edit_notes.h\
                         cart_tip.cpp cart_tip.h\
                         globals.h

nodist_rdlibrary_SOURCES = moc_rdlibrary.cpp moc_filter.cpp\
                           moc_edit_cart.cpp\
                           moc_audio_cart.cpp moc_macro_cart.cpp\
                           moc_record_cut.cpp moc_cdripper.cpp\
                           moc_edit_macro.cpp moc_list_reports.cpp\
                           moc_edit_schedulercodes.cpp \
                           moc_disk_gauge.cpp moc_disk_ripper.cpp\
                           moc_edit_notes.cpp

rdlibrary_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

EXTRA_DIST = rdlibrary.pro rdlibrary_es.ts rdlibrary_fr.ts rdlibrary_de.ts\
             rdlibrary_nb.ts rdlibrary_nn.ts

CLEANFILES = *~ moc_* *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
