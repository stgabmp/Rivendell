## automake.am
##
## Rivendell Interprocess Communication Daemon Makefile.am
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.36.2.2.2.2 2010/05/08 23:01:57 cvs Exp $
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


bin_PROGRAMS = ripcd

dist_ripcd_SOURCES = ripcd.cpp ripcd.h globals.h\
                     ripcd_socket.cpp ripcd_socket.h\
                     local_gpio.cpp local_gpio.h\
                     sas32000.cpp sas32000.h\
                     sas64000.cpp sas64000.h\
                     unity4000.cpp unity4000.h\
                     unity_feed.cpp unity_feed.h\
                     bt10x1.cpp bt10x1.h\
                     sas64000gpi.cpp sas64000gpi.h\
                     btss82.cpp btss82.h\
                     bt16x1.cpp bt16x1.h\
                     bt8x2.cpp bt8x2.h\
                     btacs82.cpp btacs82.h\
                     sasusi.cpp sasusi.h\
                     bt16x2.cpp bt16x2.h\
                     btss124.cpp btss124.h\
                     local_audio.cpp local_audio.h\
                     vguest.cpp vguest.h\
                     btss164.cpp btss164.h\
                     starguide3.cpp starguide3.h\
                     starguide_feed.cpp starguide_feed.h\
                     btss42.cpp btss42.h\
                     btss44.cpp btss44.h\
                     livewire.cpp livewire.h\
                     quartz1.cpp quartz1.h\
                     btsrc8iii.cpp btsrc8iii.h\
                     btsrc16.cpp btsrc16.h\
                     local_macros.cpp maint_routines.cpp

nodist_ripcd_SOURCES = moc_ripcd.cpp moc_ripcd_socket.cpp\
                       moc_local_gpio.cpp moc_sas32000.cpp\
                       moc_sas64000.cpp moc_unity4000.cpp\
                       moc_bt10x1.cpp moc_sas64000gpi.cpp\
                       moc_btss82.cpp moc_bt16x1.cpp\
                       moc_bt8x2.cpp moc_btacs82.cpp\
                       moc_sasusi.cpp moc_bt16x2.cpp\
                       moc_btss124.cpp moc_local_audio.cpp\
                       moc_vguest.cpp moc_btss164.cpp\
                       moc_starguide3.cpp moc_btss42.cpp\
                       moc_livewire.cpp moc_quartz1.cpp\
                       moc_btss44.cpp moc_btsrc8iii.cpp\
                       moc_btsrc16.cpp

ripcd_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

EXTRA_DIST = ripcd.pro

CLEANFILES = *~ moc_* *.obj *.idb *.pdb *ilk
MAINTAINERCLEANFILES = *~ Makefile.in
