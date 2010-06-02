## automake.am
##
## helper/ Automake.am for Rivendell
##
## (C) Copyright 2003-2005 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.6.2.1 2009/06/30 22:31:51 cvs Exp $
##      $Date: 2009/06/30 22:31:51 $
##
## Use automake to process this into a Makefile.in
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


CFLAGS = -Wall

noinst_PROGRAMS = cwrap jsmin
noinst_SCRIPTS = rdtrans.sh rdpack.sh

dist_cwrap_SOURCES = cwrap.cpp cwrap.h
dist_jsmin_SOURCES = jsmin.c

EXTRA_DIST = rdtrans.sh rdpack.sh win32_frag1.txt win32_frag2.txt

CLEANFILES = *~ moc_* *.tar.gz
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 moc_* *.tar.gz
