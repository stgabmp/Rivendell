// rdmatrix.cpp
//
// Abstract a Rivendell Switcher Matrix
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdmatrix.cpp,v 1.24.2.2.2.2 2010/05/08 23:01:57 cvs Exp $
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

#include <rddb.h>
#include <rdmatrix.h>
#include <rdescape_string.h>


RDMatrix::RDMatrix(const QString &station,int matrix)
{
  mx_station=station;
  mx_number=matrix;
}


QString RDMatrix::station() const
{
  return mx_station;
}


int RDMatrix::matrix() const
{
  return mx_number;
}


bool RDMatrix::exists() const
{
  QString sql=QString().sprintf("select TYPE from MATRICES where MATRIX=%d",
				mx_number);
  RDSqlQuery *q=new RDSqlQuery(sql);
  bool result=q->first();
  delete q;
  return result;
}


RDMatrix::Type RDMatrix::type() const
{
  return (RDMatrix::Type)GetRow("TYPE").toInt();
}


void RDMatrix::setType(RDMatrix::Type type) const
{
  SetRow("TYPE",type);
}


int RDMatrix::layer() const
{
  return GetRow("LAYER").toInt();
}


void RDMatrix::setLayer(int layer)
{
  SetRow("LAYER",layer);
}


QString RDMatrix::typeString() const
{
  switch(type()) {
      case RDMatrix::LocalGpio:
	return QString("Local GPIO");
	break;

      case RDMatrix::GenericGpo:
	return QString("Generic GPO");
	break;

      case RDMatrix::GenericSerial:
	return QString("Generic Serial");
	break;

      case RDMatrix::Sas32000:
	return QString("SAS 32000");
	break;

      case RDMatrix::Sas64000:
	return QString("SAS 64000");
	break;

      case RDMatrix::Unity4000:
	return QString("Wegener Unity 4000");
	break;

      case RDMatrix::BtSs82:
	return QString("BroadcastTools SS8.2");
	break;

      case RDMatrix::Bt10x1:
	return QString("BroadcastTools 10x1");
	break;

      case RDMatrix::Sas64000Gpi:
	return QString("SAS 64000-GPI");
	break;

      case RDMatrix::Bt16x1:
	return QString("BroadcastTools 16x1");
	break;

      case RDMatrix::Bt8x2:
	return QString("BroadcastTools 8x2");
	break;

      case RDMatrix::BtAcs82:
	return QString("BroadcastTools ACS 8.2");
	break;

      case RDMatrix::SasUsi:
	return QString("SAS User Serial Interface");
	break;

      case RDMatrix::Bt16x2:
	return QString("BroadcastTools 16x2");
	break;

      case RDMatrix::BtSs124:
	return QString("BroadcastTools SS12.4");
	break;

      case RDMatrix::LocalAudioAdapter:
	return QString("Local Audio Adapter");
	break;

      case RDMatrix::LogitekVguest:
	return QString("Logitek vGuest");
	break;

      case RDMatrix::BtSs164:
	return QString("BroadcastTools SS16.4");
	break;

      case RDMatrix::StarGuideIII:
	return QString("StarGuide III");
	break;

      case RDMatrix::BtSs42:
	return QString("BroadcastTools SS4.2");
	break;

      case RDMatrix::LiveWire:
	return QString("Axia LiveWire");
	break;

      case RDMatrix::Quartz1:
	return QString("Quartz Type 1");
	break;

      case RDMatrix::BtSs44:
	return QString("BroadcastTools SS4.4");
	break;

      case RDMatrix::BtSrc8III:
	return QString("BroadcastTools SRC-8 III");
	break;

      case RDMatrix::BtSrc16:
	return QString("BroadcastTools SRC-16");
	break;

      default:
	return QString("Unknown Type");
	break;
  }
  return QString("Unknown Type");
}


QString RDMatrix::name() const
{
  return GetRow("NAME").toString();
}


void RDMatrix::setName(const QString &name) const
{
  SetRow("NAME",name);
}


RDMatrix::PortType RDMatrix::portType(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return (RDMatrix::PortType)GetRow("PORT_TYPE").toInt();

    case RDMatrix::Backup:
      return (RDMatrix::PortType)GetRow("PORT_TYPE_2").toInt();
  }
  return RDMatrix::TtyPort;
}


void RDMatrix::setPortType(RDMatrix::Role role,PortType type) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("PORT_TYPE",(int)type);
      break;

    case RDMatrix::Backup:
      SetRow("PORT_TYPE_2",(int)type);
      break;
  }
}


QHostAddress RDMatrix::ipAddress(RDMatrix::Role role) const
{
  QHostAddress addr;
  switch(role) {
    case RDMatrix::Primary:
      addr.setAddress(GetRow("IP_ADDRESS").toString());
      break;

    case RDMatrix::Backup:
      addr.setAddress(GetRow("IP_ADDRESS_2").toString());
      break;
  }

  return addr;
}


void RDMatrix::setIpAddress(RDMatrix::Role role,QHostAddress addr) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("IP_ADDRESS",addr.toString());
      break;

    case RDMatrix::Backup:
      SetRow("IP_ADDRESS_2",addr.toString());
      break;
  }
}


int RDMatrix::card() const
{
  return GetRow("CARD").toInt();
}


void RDMatrix::setCard(int card) const
{
  SetRow("CARD",card);
}


int RDMatrix::ipPort(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return GetRow("IP_PORT").toInt();

    case RDMatrix::Backup:
      return GetRow("IP_PORT_2").toInt();
  }
  return 0;
}


void RDMatrix::setIpPort(RDMatrix::Role role,int port) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("IP_PORT",port);

    case RDMatrix::Backup:
      SetRow("IP_PORT_2",port);
  }
}


QString RDMatrix::username(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return GetRow("USERNAME").toString();

    case RDMatrix::Backup:
      return GetRow("USERNAME_2").toString();
  }
  return QString();
}


void RDMatrix::setUsername(RDMatrix::Role role,const QString &name) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("USERNAME",name);

    case RDMatrix::Backup:
      SetRow("USERNAME_2",name);
  }
}


QString RDMatrix::password(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return GetRow("PASSWORD").toString();

    case RDMatrix::Backup:
      return GetRow("PASSWORD_2").toString();
  }
  return QString();
}


void RDMatrix::setPassword(RDMatrix::Role role,const QString &passwd) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("PASSWORD",passwd);

    case RDMatrix::Backup:
      SetRow("PASSWORD_2",passwd);

  }
}


unsigned RDMatrix::startCart(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return GetRow("START_CART").toUInt();

    case RDMatrix::Backup:
      return GetRow("START_CART_2").toUInt();
  }
  return 0;
}


void RDMatrix::setStartCart(RDMatrix::Role role,unsigned cartnum) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("START_CART",cartnum);

    case RDMatrix::Backup:
      SetRow("START_CART_2",cartnum);

  }
}


unsigned RDMatrix::stopCart(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return GetRow("STOP_CART").toUInt();

    case RDMatrix::Backup:
      return GetRow("STOP_CART_2").toUInt();
  }
  return 0;
}


void RDMatrix::setStopCart(RDMatrix::Role role,unsigned cartnum) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("STOP_CART",cartnum);

    case RDMatrix::Backup:
      SetRow("STOP_CART_2",cartnum);

  }
}


int RDMatrix::port(RDMatrix::Role role) const
{
  switch(role) {
    case RDMatrix::Primary:
      return GetRow("PORT").toInt();

    case RDMatrix::Backup:
      return GetRow("PORT_2").toInt();
  }
  return -1;
}


void RDMatrix::setPort(RDMatrix::Role role,int port) const
{
  switch(role) {
    case RDMatrix::Primary:
      SetRow("PORT",port);
      break;

    case RDMatrix::Backup:
      SetRow("PORT_2",port);
      break;
  }
}


int RDMatrix::inputs() const
{
  return GetRow("INPUTS").toInt();
}


void RDMatrix::setInputs(int inputs) const
{
  SetRow("INPUTS",inputs);
}


QString RDMatrix::inputName(int input) const
{
  return GetEndpointName(input,"INPUTS");
}


RDMatrix::Mode RDMatrix::inputMode(int input) const
{
  QString sql=QString().sprintf("select CHANNEL_MODE from INPUTS where \
                                 STATION_NAME=\"%s\" && \
                                 MATRIX=%d && NUMBER=%d",
				(const char *)mx_station,
				mx_number,
				input);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return RDMatrix::Stereo;
  }
  RDMatrix::Mode mode=(RDMatrix::Mode)q->value(0).toInt();
  delete q;
  return mode;
}


int RDMatrix::outputs() const
{
  return GetRow("OUTPUTS").toInt();
}


void RDMatrix::setOutputs(int outputs) const
{
  SetRow("OUTPUTS",outputs);
}


QString RDMatrix::outputName(int output) const
{
  return GetEndpointName(output,"OUTPUTS");
}


int RDMatrix::gpis() const
{
  return GetRow("GPIS").toInt();
}


void RDMatrix::setGpis(int gpis) const
{
  SetRow("GPIS",gpis);
}


int RDMatrix::gpos() const
{
  return GetRow("GPOS").toInt();
}


void RDMatrix::setGpos(int gpos) const
{
  SetRow("GPOS",gpos);
}


QString RDMatrix::gpioDevice() const
{
  return GetRow("GPIO_DEVICE").toString();
}


void RDMatrix::setGpioDevice(const QString &dev) const
{
  SetRow("GPIO_DEVICE",dev);
}


int RDMatrix::faders() const
{
  return GetRow("FADERS").toInt();
}


void RDMatrix::setFaders(int quan) const
{
  SetRow("FADERS",quan);
}


int RDMatrix::displays() const
{
  return GetRow("DISPLAYS").toInt();
}


void RDMatrix::setDisplays(int quan) const
{
  SetRow("DISPLAYS",quan);
}


QString RDMatrix::GetEndpointName(int pointnum,const QString &table) const
{
  QString sql=QString().sprintf("select NAME from %s where \
                                 STATION_NAME=\"%s\" && \
                                 MATRIX=%d && NUMBER=%d",
				(const char *)table,
				(const char *)mx_station,
				mx_number,
				pointnum);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return QString("NULL");
  }
  QString str=q->value(0).toString();
  delete q;
  return str;
}


QVariant RDMatrix::GetRow(const QString &param) const
{
  QVariant var;
  QString sql=QString().sprintf("select %s from MATRICES where \
                                STATION_NAME=\"%s\" && MATRIX=%d",
				(const char *)RDEscapeString(param),
				(const char *)mx_station,
                                mx_number);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    var=q->value(0);
  }
  delete q;
  return var;
}


void RDMatrix::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE MATRICES SET %s=\"%s\" \
                         WHERE STATION_NAME=\"%s\" && MATRIX=%d",
			(const char *)RDEscapeString(param),
			(const char *)RDEscapeString(value),
			(const char *)mx_station,
			mx_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDMatrix::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE MATRICES SET %s=%d \
                         WHERE STATION_NAME=\"%s\" && MATRIX=%d",
			(const char *)RDEscapeString(param),
			value,
			(const char *)mx_station,
			mx_number);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDMatrix::SetRow(const QString &param,unsigned value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE MATRICES SET %s=%u \
                         WHERE STATION_NAME=\"%s\" && MATRIX=%d",
			(const char *)RDEscapeString(param),
			value,
			(const char *)mx_station,
			mx_number);
  q=new RDSqlQuery(sql);
  delete q;
}
