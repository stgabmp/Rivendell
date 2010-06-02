// opendb.h
//
// Open a Rivendell Database
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: opendb.h,v 1.6 2007/02/14 21:51:02 fredg Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef OPENDB_H
#define OPENDB_H

#include <qstring.h>


bool OpenDb(QString dbname,QString login,QString password,QString hostname,
	    QString stationname,bool interactive,QString username=QString::QString(""));


#endif

