## automake.am
##
## Automake.am for rivendell/utils/rdbackup
##
##   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.6 2007/09/14 14:07:01 fredg Exp $
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

bin_PROGRAMS = rdbackup

dist_rdbackup_SOURCES = rdbackup.cpp rdbackup.h
rdbackup_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

EXTRA_DIST = 

CLEANFILES = *~ moc_* *.qm *.obj *.idb *.pdb *ilk
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
