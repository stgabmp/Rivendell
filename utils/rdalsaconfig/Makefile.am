## automake.am
##
## Automake.am for utils/rdalsaconfig
##
## (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
##
##    $Id: Makefile.am,v 1.1.2.1 2009/08/03 10:57:54 cvs Exp $
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

bin_PROGRAMS = rdalsaconfig

dist_rdalsaconfig_SOURCES = alsaitem.cpp alsaitem.h\
                            rdalsaconfig.cpp rdalsaconfig.h\
                            rdalsa.cpp rdalsa.h

nodist_rdalsaconfig_SOURCES = moc_rdalsaconfig.cpp
rdalsaconfig_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@ @LIBALSA@

EXTRA_DIST = rdalsaconfig.pro

CLEANFILES = *~ moc_* *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
