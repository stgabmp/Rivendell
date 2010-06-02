// rdttyout.h
//
// Output a string on a Rivendell TTY
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdttyout.h,v 1.5 2008/08/04 19:05:34 fredg Exp $
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
#include <qsqldatabase.h>

#include <rdttydevice.h>

#ifndef RDTTYOUT_H
#define RDTTYOUT_H

bool RDTtyOut(const QString &station,unsigned port_id,const QString &string);


#endif 
