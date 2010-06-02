// rdreport.h
//
// Abstract a Rivendell Report Descriptor
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdreport.h,v 1.15.6.3 2010/01/13 18:29:11 cvs Exp $
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

#ifndef RDREPORT_H
#define RDREPORT_H

#include <qobject.h>
#include <qsqldatabase.h>

#include <rdstation.h>
#include <rdsvc.h>
#include <rdlog_line.h>

class RDReport
{
 public:
  enum ExportFilter {CbsiDeltaFlex=0,TextLog=1,BmiEmr=2,Technical=3,
		     SoundExchange=4,RadioTraffic=5,VisualTraffic=6,
		     CounterPoint=7,Music1=8,LastFilter=9};
  enum ExportOs {Linux=0,Windows=1};
  enum ExportType {Generic=0,Traffic=1,Music=2};
  enum StationType {TypeOther=0,TypeAm=1,TypeFm=2,TypeLast=3};
  enum ErrorCode {ErrorOk=0,ErrorCanceled=1,ErrorCantOpen=2};
  RDReport(const QString &rptname,QObject *parent=0,const char *name=0);
  QString name() const;
  bool exists();
  QString description();
  void setDescription(const QString &desc);
  ExportFilter filter();
  void setFilter(ExportFilter filter);
  QString exportPath(ExportOs ostype);
  void setExportPath(ExportOs ostype,const QString &path);
  bool exportTypeEnabled(ExportType type);
  void setExportTypeEnabled(ExportType type,bool state);
  bool exportTypeForced(ExportType type);
  void setExportTypeForced(ExportType type,bool state);
  QString stationId();
  void setStationId(const QString &id);
  unsigned cartDigits();
  void setCartDigits(unsigned num);
  bool useLeadingZeros();
  void setUseLeadingZeros(bool state);
  int linesPerPage() const;
  void setLinesPerPage(int lines);
  QString serviceName() const;
  void setServiceName(const QString &name);
  RDReport::StationType stationType() const;
  void setStationType(RDReport::StationType type);
  QString stationFormat() const;
  void setStationFormat(const QString &fmt);
  RDLogLine::StartSource startSource() const;
  void setStartSource(RDLogLine::StartSource src);
  bool filterOnairFlag() const;
  void setFilterOnairFlag(bool state);
  RDReport::ErrorCode errorCode() const;
  bool generateReport(const QDate &startdate,const QDate &enddate,
		      RDStation *station);
  static QString filterText(RDReport::ExportFilter filter);
  static QString stationTypeText(RDReport::StationType type);
  static bool multipleDaysAllowed(RDReport::ExportFilter filter);
  static bool multipleMonthsAllowed(RDReport::ExportFilter filter);
  static QString errorText(RDReport::ErrorCode code);

 private:
  bool ExportDeltaflex(const QDate &startdate,const QDate &enddate,
		       const QString &mixtable);
  bool ExportTextLog(const QDate &startdate,const QDate &enddate,
		     const QString &mixtable);
  bool ExportBmiEmr(const QDate &startdate,const QDate &enddate,
		    const QString &mixtable);
  bool ExportTechnical(const QDate &startdate,const QDate &enddate,
		       const QString &mixtable);
  bool ExportSoundEx(const QDate &startdate,const QDate &enddate,
		     const QString &mixtable);
  bool ExportRadioTraffic(const QDate &startdate,const QDate &enddate,
			  const QString &mixtable);
  void SetRow(const QString &param,QString value);
  void SetRow(const QString &param,int value);
  void SetRow(const QString &param,unsigned value);
  void SetRow(const QString &param,bool value);
  QString OsFieldName(ExportOs os);
  QString TypeFieldName(ExportType type,bool forced);
  QString report_name;
  RDReport::ErrorCode report_error_code;
};


#endif   // RDREPORT_H
