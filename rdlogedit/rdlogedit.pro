# rdlogedit.pro
#
# The rdlogedit/ QMake project file for Rivendell
#
# (C) Copyright 2003-2004 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: rdlogedit.pro,v 1.14.2.2 2010/01/21 17:11:48 cvs Exp $
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

TARGET = rdlogedit

win32 {
  DEFINES += WIN32
  DEFINES += VERSION=\"$$[VERSION]\"
  DEFINES += PACKAGE=\"rivendell\" 
  DEFINES += PACKAGE_VERSION=\"$$[VERSION]\" 
  DEFINES += PACKAGE_NAME=\"rivendell\"
  DEFINES += PACKAGE_BUGREPORT=\"fredg@paravelsystems.com\"
}

SOURCES += rdlogedit.cpp
SOURCES += edit_log.cpp
SOURCES += edit_logline.cpp
SOURCES += edit_marker.cpp
SOURCES += edit_chain.cpp
SOURCES += add_meta.cpp
SOURCES += list_logs.cpp
SOURCES += edit_track.cpp
SOURCES += list_listviewitem.cpp
SOURCES += list_reports.cpp

HEADERS += rdlogedit.h
HEADERS += edit_log.h
HEADERS += edit_logline.h
HEADERS += edit_marker.h
HEADERS += edit_chain.h
HEADERS += add_meta.h
HEADERS += list_logs.h
HEADERS += edit_track.h
HEADERS += globals.h
HEADERS += list_listviewitem.h
HEADERS += list_reports.h

RES_FILE += ..\icons\rivendell.res

INCLUDEPATH += ..\lib

LIBS = -lqui -L..\lib -llib

CONFIG += qt

TRANSLATIONS += rdlogedit_de.ts
TRANSLATIONS += rdlogedit_es.ts
TRANSLATIONS += rdlogedit_fr.ts
TRANSLATIONS += rdlogedit_nb.ts
TRANSLATIONS += rdlogedit_nn.ts
