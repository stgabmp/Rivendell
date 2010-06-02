## automake.am
##
## Automake.am for rivendell/rdairplay
##
## (C) 2002-2008 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.41.2.3 2010/01/21 17:11:47 cvs Exp $
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
	cp rdairplay_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:	
	rm -f $(DESTDIR)$(prefix)/share/srlabs/rdairplay_*.qm

all:	
	@QT_BIN@/lupdate rdairplay.pro
	@QT_BIN@/lrelease rdairplay.pro

bin_PROGRAMS = rdairplay

dist_rdairplay_SOURCES = rdairplay.cpp rdairplay.h\
                         loglinebox.cpp loglinebox.h\
                         wall_clock.cpp wall_clock.h\
                         post_counter.cpp post_counter.h\
                         pie_counter.cpp pie_counter.h\
                         stop_counter.cpp stop_counter.h\
                         start_button.cpp start_button.h\
                         globals.h\
                         mode_display.cpp mode_display.h\
                         list_log.cpp list_log.h\
                         log_play.cpp log_play.h\
                         edit_event.cpp edit_event.h\
                         button_log.cpp button_log.h\
                         list_logs.cpp list_logs.h\
                         local_macros.cpp colors.h\
                         log_traffic.cpp log_traffic.h\
                         marker_bar.cpp marker_bar.h\
                         nownext.cpp\
                         rlmhost.cpp rlmhost.h

nodist_rdairplay_SOURCES = moc_rdairplay.cpp moc_loglinebox.cpp\
                           moc_wall_clock.cpp moc_post_counter.cpp\
                           moc_pie_counter.cpp moc_stop_counter.cpp\
                           moc_start_button.cpp moc_mode_display.cpp\
                           moc_list_log.cpp moc_button_log.cpp\
                           moc_log_play.cpp moc_edit_event.cpp\
                           moc_list_logs.cpp moc_marker_bar.cpp\
                           moc_rlmhost.cpp

rdairplay_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@
rdairplay_LDFLAGS = -rdynamic

EXTRA_DIST = rdairplay.pro rdairplay_es.ts rdairplay_fr.ts rdairplay_de.ts\
             rdairplay_nb.ts rdairplay_nn.ts

CLEANFILES = *~ moc_* *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
