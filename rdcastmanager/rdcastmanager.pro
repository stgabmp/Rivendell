# rdcastmanager.pro
#
# The rdcastmanager/ QMake project file for Rivendell
#
# (C) Copyright 2003-2004 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: rdcastmanager.pro,v 1.2.2.2 2010/01/21 17:11:47 cvs Exp $
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

TARGET = rdlogmanager

win32 {
  DEFINES += WIN32
  DEFINES += VERSION=\"$$[VERSION]\"
  DEFINES += PACKAGE=\"rivendell\" 
  DEFINES += PACKAGE_VERSION=\"$$[VERSION]\" 
  DEFINES += PACKAGE_NAME=\"rivendell\"
  DEFINES += PACKAGE_BUGREPORT=\"fredg@paravelsystems.com\"
}

SOURCES += rdcastmanager.cpp
SOURCES += edit_cast.cpp
SOURCES += list_casts.cpp

HEADERS += rdcastmanager.h
HEADERS += edit_cast.h
HEADERS += list_casts.h

RES_FILE += ..\icons\rivendell.res

INCLUDEPATH += ..\lib

LIBS = -lqui -L..\lib -llib

CONFIG += qt

TRANSLATIONS += rdcastmanager_de.ts
TRANSLATIONS += rdcastmanager_es.ts
TRANSLATIONS += rdcastmanager_fr.ts
TRANSLATIONS += rdcastmanager_nb.ts
TRANSLATIONS += rdcastmanager_nn.ts
