# rdairplay.pro
#
# The QMake project file for RDPanel.
#
# (C) Copyright 2003-2007 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: rdpanel.pro,v 1.2 2008/04/01 18:27:35 fredg Exp $
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

x11 {
  SOURCES += rdpanel.cpp
}

x11 {
  HEADERS += rdpanel.h
}

TRANSLATIONS += rdpanel_es.ts
TRANSLATIONS += rdpanel_fr.ts
TRANSLATIONS += rdpanel_de.ts
