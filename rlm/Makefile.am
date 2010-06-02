## automake.am
##
## Automake.am for rivendell/rlm
##
##   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.1.2.11 2009/09/02 19:23:28 cvs Exp $
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

AM_CFLAGS = -fPIC -Wall

RLM_MODULES=rlm_test.rlm rlm_serial.rlm rlm_xmpad.rlm rlm_udp.rlm\
            rlm_twitter.rlm rlm_facebook.rlm rlm_ando.rlm rlm_inno713.rlm\
            rlm_liqcomp.rlm

all:	$(RLM_MODULES)

%.rlm:	%.c
	$(CC) $(AM_CFLAGS) -I$(top_srcdir) -shared $< -o $@

install:	all
	mkdir -p $(DESTDIR)$(prefix)/@RD_LIB_PATH@/rivendell
	cp $(RLM_MODULES) $(DESTDIR)$(prefix)/@RD_LIB_PATH@/rivendell/

headerdir = $(includedir)/rlm
header_HEADERS = rlm.h

EXTRA_DIST = rlm.h rlm_test.c rlm_serial.c rlm_xmpad.c rlm_udp.c\
             rlm_twitter.c rlm_facebook.c rlm_ando.c rlm_inno713.c\
             rlm_liqcomp.c Makefile-example

CLEANFILES = *~ moc_* *.qm *.obj *.idb *.pdb *ilk *.rlm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
