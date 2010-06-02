// rdcheck_version.cpp
//
// Get the current Rivendell Database Version,.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcheck_version.cpp,v 1.3 2007/02/14 21:48:41 fredg Exp $
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

#include <rd.h>
#include <rdversion.h>

#include <rdcheck_version.h>


int RDCheckVersion()
{
  int ver;

  RDVersion *version=new RDVersion();
  ver=version->database();
  delete version;
  return ver;
}
