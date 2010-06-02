// rdsystem.cpp
//
// System-wide Rivendell settings
//
//   (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdsystem.cpp,v 1.1.2.2 2009/05/21 17:32:17 cvs Exp $
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
#include <rddb.h>
#include <rdconf.h>
#include <rdsystem.h>

RDSystem::RDSystem()
{
}


bool RDSystem::allowDuplicateCartTitles() const
{
  bool ret=false;
  QString sql;
  RDSqlQuery *q;

  sql="select DUP_CART_TITLES from SYSTEM";
  q=new RDSqlQuery(sql);
  if(q->first()) {
    ret=RDBool(q->value(0).toString());
  }
  delete q;
  return ret;
}


void RDSystem::setAllowDuplicateCartTitles(bool state) const
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("update SYSTEM set DUP_CART_TITLES=\"%s\"",
			(const char *)RDYesNo(state));
  q=new RDSqlQuery(sql);
  delete q;
}


unsigned RDSystem::maxPostLength() const
{
  unsigned ret;

  QString sql="select MAX_POST_LENGTH from SYSTEM";
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    ret=q->value(0).toUInt();
  }
  else {
    ret=RD_DEFAULT_MAX_POST_LENGTH;
  }
  delete q;
  return ret;
}


void RDSystem::setMaxPostLength(unsigned bytes) const
{
  QString sql=QString().sprintf("update SYSTEM set MAX_POST_LENGTH=%u",bytes);
  RDSqlQuery *q=new RDSqlQuery(sql);
  delete q;
}
