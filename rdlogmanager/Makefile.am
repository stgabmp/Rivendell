## automake.am
##
## Automake.am for rivendell/rdlogmanager
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.35.2.2 2010/01/21 17:11:48 cvs Exp $
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
	cp rdlogmanager_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdlogmanager_*.qm

all:	
	@QT_BIN@/lupdate rdlogmanager.pro
	@QT_BIN@/lrelease rdlogmanager.pro

bin_PROGRAMS = rdlogmanager

dist_rdlogmanager_SOURCES = rdlogmanager.cpp rdlogmanager.h\
                            list_clocks.cpp list_clocks.h\
                            list_events.cpp list_events.h\
                            list_grids.cpp list_grids.h\
                            list_svcs.cpp list_svcs.h\
                            add_event.cpp add_event.h\
                            add_clock.cpp add_clock.h\
                            edit_event.cpp edit_event.h\
                            edit_clock.cpp edit_clock.h\
                            edit_eventline.cpp edit_eventline.h\
                            edit_grid.cpp edit_grid.h\
                            lib_listview.cpp lib_listview.h\
                            import_listview.cpp import_listview.h\
                            edit_note.cpp edit_note.h\
                            edit_track.cpp edit_track.h\
                            edit_perms.cpp edit_perms.h\
                            svc_rec.cpp svc_rec.h\
                            svc_rec_dialog.cpp svc_rec_dialog.h\
                            pick_report_dates.cpp pick_report_dates.h\
                            rename_item.cpp rename_item.h\
                            generate_log.cpp generate_log.h\
                            clock_listview.cpp clock_listview.h\
                            edit_schedrules.cpp edit_schedrules.h\
                            edit_schedcoderules.cpp edit_schedcoderules.h\
                            globals.h

nodist_rdlogmanager_SOURCES = moc_rdlogmanager.cpp moc_list_events.cpp\
                              moc_add_event.cpp moc_edit_event.cpp\
                              moc_lib_listview.cpp moc_import_listview.cpp\
                              moc_edit_note.cpp moc_list_clocks.cpp\
                              moc_add_clock.cpp moc_edit_clock.cpp\
                              moc_edit_eventline.cpp moc_list_grids.cpp\
                              moc_generate_log.cpp moc_edit_grid.cpp\
                              moc_edit_perms.cpp\
                              moc_rename_item.cpp moc_clock_listview.cpp\
                              moc_edit_track.cpp moc_list_svcs.cpp\
                              moc_svc_rec.cpp\
                              moc_svc_rec_dialog.cpp moc_pick_report_dates.cpp\
                              moc_edit_schedrules.cpp\
                              moc_edit_schedcoderules.cpp

rdlogmanager_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

# rdlogmanager_LDFLAGS = -static

EXTRA_DIST = rdlogmanager.pro\
             rdlogmanager_es.ts rdlogmanager_fr.ts rdlogmanager_de.ts\
             rdlogmanager_nb.ts rdlogmanager_nn.ts

CLEANFILES = *~ moc_* *.qm *.obj *.idb *.pdb *ilk
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
