// rdnownext.cpp
//
// Rivendell Now & Next Implementation
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdnownext.cpp,v 1.1.2.1 2008/11/25 22:15:49 fredg Exp $
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

#include <rdnownext.h>

void RDResolveNowNext(QString *str,RDLogLine **loglines)
{
  //
  // NOW PLAYING Event
  //
  if(loglines[0]!=NULL) {
    str->replace("%n",QString().sprintf("%06u",loglines[0]->cartNumber()));
    str->replace("%h",QString().sprintf("%d",loglines[0]->effectiveLength()));
    str->replace("%g",loglines[0]->groupName());
    str->replace("%t",loglines[0]->title());
    str->replace("%a",loglines[0]->artist());
    str->replace("%l",loglines[0]->album());
    str->replace("%y",loglines[0]->year().toString("yyyy"));
    str->replace("%b",loglines[0]->label());
    str->replace("%c",loglines[0]->client());
    str->replace("%e",loglines[0]->agency());
    str->replace("%m",loglines[0]->composer());
    str->replace("%p",loglines[0]->publisher());
    str->replace("%u",loglines[0]->userDefined());
  }
  else {   // No NOW PLAYING Event
    str->replace("%n","");
    str->replace("%h","");
    str->replace("%g","");
    str->replace("%t","");
    str->replace("%a","");
    str->replace("%l","");
    str->replace("%y","");
    str->replace("%b","");
    str->replace("%c","");
    str->replace("%e","");
    str->replace("%m","");
    str->replace("%p","");
    str->replace("%u","");
  }

  //
  // NEXT Event
  //
  if(loglines[1]!=NULL) {
    str->replace("%N",QString().sprintf("%06u",loglines[1]->cartNumber()));
    str->replace("%H",QString().sprintf("%d",loglines[1]->effectiveLength()));
    str->replace("%G",loglines[1]->groupName());
    str->replace("%T",loglines[1]->title());
    str->replace("%A",loglines[1]->artist());
    str->replace("%L",loglines[1]->album());
    str->replace("%Y",loglines[1]->year().toString("yyyy"));
    str->replace("%B",loglines[1]->label());
    str->replace("%C",loglines[1]->client());
    str->replace("%E",loglines[1]->agency());
    str->replace("%M",loglines[1]->composer());
    str->replace("%P",loglines[1]->publisher());
    str->replace("%U",loglines[1]->userDefined());
  }
  else {   // No NEXT Event
    str->replace("%N","");
    str->replace("%H","");
    str->replace("%G","");
    str->replace("%T","");
    str->replace("%A","");
    str->replace("%L","");
    str->replace("%Y","");
    str->replace("%B","");
    str->replace("%C","");
    str->replace("%E","");
    str->replace("%M","");
    str->replace("%P","");
    str->replace("%U","");
  }
  str->replace("%%","%");
  str->replace("%r","\n");
  str->replace("%R","\r\n");
}
