## automake.am
##
## Automake.am for rivendell/toolame
##
##   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.4 2007/02/14 21:59:12 fredg Exp $
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

AM_CFLAGS = -Wall -DNDEBUG -DINLINE=inline -DNEWENCODE -DNEWATAN

bin_PROGRAMS = toolame

##                       pds_subband.c\
dist_toolame_SOURCES = absthr.h\
                       ath.c ath.h\
                       audio_read.c audio_read.h\
                       availbits.c availbits.h\
                       bitstream.c bitstream.h\
                       bwf_wavefile.c bwf_wavefile.h\
                       common.c common.h\
                       crc.c crc.h\
                       critband.h\
                       encode.c encode.h\
                       encode_new.c encode_new.h\
                       encoder.h\
                       enwindow.h\
                       fft.c fft.h\
                       freqtable.h\
                       ieeefloat.c ieeefloat.h\
                       mem.c mem.h\
                       musicin.h\
                       options.h\
                       portableio.c portableio.h\
                       psycho_0.c psycho_0.h\
                       psycho_1.c psycho_1.h psycho_1_priv.h\
                       psycho_2.c psycho_2.h\
                       psycho_3.c psycho_3.h psycho_3priv.h\
                       psycho_4.c psycho_4.h\
                       psycho_n1.c psycho_n1.h\
                       subband.c subband.h\
                       tables.c tables.h\
                       toolame.c toolame.h

toolame_LDADD = -lm

EXTRA_DIST = FUTURE HISTORY LGPL.txt README Makefile.old

CLEANFILES = *~
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz
