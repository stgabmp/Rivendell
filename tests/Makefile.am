## automake.am
##
## Automake.am for rivendell/tests
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.11.2.1 2010/01/12 22:18:57 cvs Exp $
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

AM_CPPFLAGS = -Wall @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/lib
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib
MOC = @QT_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

noinst_PROGRAMS = sas_torture sas_switch_torture

dist_sas_torture_SOURCES = sas_torture.cpp sas_torture.h
nodist_sas_torture_SOURCES = moc_sas_torture.cpp
sas_torture_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

dist_sas_switch_torture_SOURCES = sas_switch_torture.cpp sas_switch_torture.h
nodist_sas_switch_torture_SOURCES = moc_sas_switch_torture.cpp
sas_switch_torture_LDADD = -lqui -lrd -lid3 -lFLAC @LIBVORBIS@

EXTRA_DIST = rivendell_standard.txt visualtraffic.txt

CLEANFILES = *~ moc_*
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
