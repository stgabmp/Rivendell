## automake.am
##
## Automake.am for rivendell/rdlogedit
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.33.2.2 2010/01/21 17:11:48 cvs Exp $
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

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

# I18N Stuff
install-exec-local:	
	mkdir -p $(DESTDIR)$(prefix)/share/rivendell
	cp rdlogedit_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdlogedit_*.qm

all:	
	@QT_BIN@/lupdate rdlogedit.pro
	@QT_BIN@/lrelease rdlogedit.pro

bin_PROGRAMS = rdlogedit

dist_rdlogedit_SOURCES = rdlogedit.cpp rdlogedit.h globals.h\
                         edit_log.cpp edit_log.h\
                         edit_logline.cpp edit_logline.h\
                         edit_marker.cpp edit_marker.h\
                         edit_track.cpp edit_track.h\
                         edit_chain.cpp edit_chain.h\
                         add_meta.cpp add_meta.h\
                         list_logs.cpp list_logs.h\
                         voice_tracker.cpp voice_tracker.h\
                         import_track.cpp import_track.h\
                         log_listview.cpp log_listview.h\
                         list_listviewitem.cpp list_listviewitem.h\
                         list_reports.cpp list_reports.h

nodist_rdlogedit_SOURCES = moc_rdlogedit.cpp\
                           moc_edit_log.cpp moc_edit_logline.cpp\
                           moc_edit_marker.cpp moc_edit_chain.cpp\
                           moc_add_meta.cpp moc_list_logs.cpp\
                           moc_edit_track.cpp moc_voice_tracker.cpp\
                           moc_import_track.cpp moc_log_listview.cpp\
                           moc_list_reports.cpp

rdlogedit_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@
# rdlogedit_LDFLAGS = -static

EXTRA_DIST = rdlogedit.pro rdlogedit_es.ts rdlogedit_fr.ts rdlogedit_de.ts\
             rdlogedit_nb.ts rdlogedit_nn.ts

CLEANFILES = *~ moc_* *.qm *.obj *.idb *.pdb *ilk
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
