## automake.am
##
## Automake.am for utils/rdhpiinfo
##
## (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
##
##    $Id: Makefile.am,v 1.1 2007/09/14 16:29:22 fredg Exp $
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
##   Use automake to process this into a Makefile.in
##

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/rdhpi
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib -L$(top_srcdir)/rdhpi
MOC = @QT_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

bin_PROGRAMS = rdhpiinfo

dist_rdhpiinfo_SOURCES = rdhpiinfo.cpp rdhpiinfo.h\
                        change_mode.cpp change_mode.h
nodist_rdhpiinfo_SOURCES = moc_rdhpiinfo.cpp moc_change_mode.cpp
rdhpiinfo_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@ @LIBHPI@

EXTRA_DIST = rdhpiinfo.pro

CLEANFILES = *~ moc_* *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
