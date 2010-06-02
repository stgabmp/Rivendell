# rdsoftkeys.pro
#
# The utils/rdsoftkeys QMake project file for Rivendell
#
# (C) Copyright 2003-2006 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: rdsoftkeys.pro,v 1.3 2007/02/14 21:59:12 fredg Exp $
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

TEMPLATE = app

TARGET = rdsoftkeys

win32 {
  DEFINES += WIN32
}

SOURCES += rdsoftkeys.cpp
HEADERS += rdsoftkeys.h

RES_FILE += ..\..\icons\rivendell.res

INCLUDEPATH += ..\..\..\libradio\radio ..\..\lib

LIBS = -lqui -L..\..\..\libradio\radio -lradio -L..\..\lib -llib

CONFIG += qt
