// rdescape_string.h
//
// Escape non-valid characters in a string.
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdescape_string.h,v 1.4.6.1 2009/06/29 14:20:48 cvs Exp $
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

#include <qstring.h>

#ifndef RDESCAPE_STRING_H
#define RDESCAPE_STRING_H

QString RDEscapeString(const QString &str);

/**
 * Escape a string so it is safe to be used as a SQL (MySQL) column.
 * For details see http://dev.mysql.com/doc/refman/4.1/en/identifiers.html
 * 
 * @param str The string to escape.
 *
 * Returns escaped string
 **/
QString RDEscapeStringSQLColumn(const QString &str);



#endif 
