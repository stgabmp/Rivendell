// rdgroup.cpp
//
// Abstract a Rivendell Group.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdgroup.cpp,v 1.18.2.1 2008/11/30 00:32:57 fredg Exp $
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

#include <rdconf.h>
#include <rdgroup.h>
#include <rddb.h>
#include <rdescape_string.h>

//
// Global Classes
//
RDGroup::RDGroup(QString name,bool create,QSqlDatabase *db)
{
  RDSqlQuery *q;
  QString sql;

  group_db=db;
  group_name=name;

  if(create) {
    sql=QString().sprintf("INSERT INTO GROUPS SET NAME=\"%s\"",
			  (const char *)RDEscapeString(group_name));
    q=new RDSqlQuery(sql,group_db);
    delete q;
  }
}


bool RDGroup::exists() const
{
  return RDDoesRowExist("GROUPS","NAME",group_name,group_db);
}


QString RDGroup::name() const
{
  return group_name;
}


QString RDGroup::description() const
{
  return RDGetSqlValue("GROUPS","NAME",group_name,"DESCRIPTION",group_db).
    toString();
}


void RDGroup::setDescription(const QString &desc) const
{
  SetRow("DESCRIPTION",desc);
}


RDCart::Type RDGroup::defaultCartType() const
{
  return (RDCart::Type)RDGetSqlValue("GROUPS","NAME",group_name,
				    "DEFAULT_CART_TYPE",group_db).toUInt();
}


void RDGroup::setDefaultCartType(RDCart::Type type) const
{
  SetRow("DEFAULT_CART_TYPE",(unsigned)type);
}


unsigned RDGroup::defaultLowCart() const
{
  return RDGetSqlValue("GROUPS","NAME",group_name,"DEFAULT_LOW_CART",group_db).
    toUInt();
}


void RDGroup::setDefaultLowCart(unsigned cartnum) const
{
  SetRow("DEFAULT_LOW_CART",cartnum);
}


unsigned RDGroup::defaultHighCart() const
{
  return RDGetSqlValue("GROUPS","NAME",group_name,"DEFAULT_HIGH_CART",group_db).
    toUInt();
}


void RDGroup::setDefaultHighCart(unsigned cartnum) const
{
  SetRow("DEFAULT_HIGH_CART",cartnum);
}


int RDGroup::cutShelflife() const
{
  return RDGetSqlValue("GROUPS","NAME",group_name,"CUT_SHELFLIFE",group_db).
    toInt();
}


void RDGroup::setCutShelflife(int days) const
{
  SetRow("CUT_SHELFLIFE",days);
}


QString RDGroup::defaultTitle() const
{
  return RDGetSqlValue("GROUPS","NAME",group_name,"DEFAULT_TITLE",group_db).
    toString();
}


void RDGroup::setDefaultTitle(const QString &str)
{
  SetRow("DEFAULT_TITLE",str);
}


bool RDGroup::enforceCartRange() const
{
  return RDBool(RDGetSqlValue("GROUPS","NAME",group_name,"ENFORCE_CART_RANGE",
			    group_db).toString());
}


void RDGroup::setEnforceCartRange(bool state) const
{
  SetRow("ENFORCE_CART_RANGE",RDYesNo(state));
}


bool RDGroup::exportReport(ExportType type) const
{
  return RDBool(RDGetSqlValue("GROUPS","NAME",group_name,ReportField(type),
			    group_db).toString());
}


void RDGroup::setExportReport(ExportType type,bool state) const
{
  SetRow(ReportField(type),RDYesNo(state));
}


bool RDGroup::enableNowNext() const
{
  return RDBool(RDGetSqlValue("GROUPS","NAME",group_name,"ENABLE_NOW_NEXT",
			    group_db).toString());
}


void RDGroup::setEnableNowNext(bool state) const
{
  SetRow("ENABLE_NOW_NEXT",RDYesNo(state));
}


QColor RDGroup::color() const
{
  return QColor(RDGetSqlValue("GROUPS","NAME",group_name,"COLOR",group_db).
		toString());
}


void RDGroup::setColor(const QColor &color)
{
  SetRow("COLOR",color.name());
}


unsigned RDGroup::RDGroup::nextFreeCart(unsigned startcart) const
{
  QString sql;
  RDSqlQuery *q;
  unsigned cart_low_limit;
  unsigned cart_high_limit;

  sql=QString().sprintf("select DEFAULT_LOW_CART,DEFAULT_HIGH_CART\
                         from GROUPS where NAME=\"%s\"",
			(const char *)group_name);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    if(startcart>q->value(0).toUInt()) {
      cart_low_limit=startcart;
    }
    else {
      cart_low_limit=q->value(0).toUInt();
    }
    cart_high_limit=q->value(1).toUInt();
    delete q;
    if(cart_low_limit<1) {
      return 0;
    }
    sql=QString().sprintf("select NUMBER from CART where \
                         (NUMBER>=%u)&&(NUMBER<=%u) order by NUMBER",
			  cart_low_limit,cart_high_limit);
    q=new RDSqlQuery(sql);
    if(q->size()<1) {
      delete q;
      return cart_low_limit;
    }
    for(unsigned i=cart_low_limit;i<=cart_high_limit;i++) {
      if(!q->next()) {
	delete q;
	return i;
      }
      if(i!=q->value(0).toUInt()) {
	delete q;
	return i;
      }
    }
    delete q;
  }
  else {
    delete q;
  }
  return 0;
}


int RDGroup::freeCartQuantity() const
{
  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select DEFAULT_LOW_CART,DEFAULT_HIGH_CART\
                         from GROUPS where NAME=\"%s\"",
			(const char *)group_name);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return -1;
  }
  if((q->value(0).toInt()<0)||(q->value(1).toInt()<0)) {
    delete q;
    return -1;
  }
  int low=q->value(0).toInt();
  int high=q->value(1).toInt();
  sql=QString().sprintf("select NUMBER from CART\
                         where (NUMBER>=%d)&&(NUMBER<=%d)",
			q->value(0).toInt(),q->value(1).toInt());
  delete q;
  q=new RDSqlQuery(sql);
  int free=high-low-q->size();
  delete q;

  return free;
}


bool RDGroup::cartNumberValid(unsigned cartnum) const
{
  if((cartnum<1)||(cartnum>999999)) {
    return false;
  }
  bool ret=false;
  QString sql=QString().sprintf("select DEFAULT_LOW_CART,DEFAULT_HIGH_CART,\
                                 ENFORCE_CART_RANGE from GROUPS \
                                 where NAME=\"%s\"",(const char *)group_name);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    if(!RDBool(q->value(2).toString())) {
      ret=true;
    }
    else {
      if((cartnum>=q->value(0).toUInt())&&(cartnum<=q->value(1).toUInt())) {
	ret=true;
      }
    }
  }
  delete q;
  return ret;
}


void RDGroup::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE GROUPS SET %s=%d WHERE NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)group_name);
  q=new RDSqlQuery(sql,group_db);
  delete q;
}


void RDGroup::SetRow(const QString &param,unsigned value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE GROUPS SET %s=%u WHERE NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)group_name);
  q=new RDSqlQuery(sql,group_db);
  delete q;
}


void RDGroup::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE GROUPS SET %s=\"%s\" WHERE NAME=\"%s\"",
			(const char *)param,
			(const char *)RDEscapeString(value),
			(const char *)group_name);
  q=new RDSqlQuery(sql,group_db);
  delete q;
}


QString RDGroup::ReportField(ExportType type) const
{
  switch(type) {
      case RDGroup::Traffic:
	return QString("REPORT_TFC");
	break;

      case RDGroup::Music:
	return QString("REPORT_MUS");
	break;

      default:
	break;
  }
  return QString();
}

