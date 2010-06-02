// log_traffic.cpp
//
// Add an entry to the reconciliation table.
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: log_traffic.cpp,v 1.17 2008/08/04 19:05:34 fredg Exp $
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

#include <qsqldatabase.h>

#include <rdconf.h>
#include <rddb.h>
#include <rdescape_string.h>

#include <log_traffic.h>
#include <globals.h>


void LogTraffic(const QString &svcname,const QString &logname,
		RDLogLine *logline,RDLogLine::PlaySource src,
		RDAirPlayConf::TrafficAction action,bool onair_flag)
{
  QString sql;
  RDSqlQuery *q;
  QDateTime datetime=QDateTime(QDate::currentDate(),QTime::currentTime());
  int length=logline->startTime(RDLogLine::Actual).msecsTo(datetime.time());
  if(length<0) {  // Event crossed midnight!
    length+=86400000;
    datetime.setDate(datetime.date().addDays(-1));
  }

  if((logline==NULL)||(svcname.isEmpty())) {
    return;
  }
  QString svctablename=RDEscapeStringSQLColumn(svcname);
  svctablename.replace(" ","_");
  sql=QString().sprintf("insert into `%s_SRT` set\
                         LENGTH=%d,LOG_NAME=\"%s\",LOG_ID=%d,CART_NUMBER=%u,\
                         STATION_NAME=\"%s\",EVENT_DATETIME=\"%s %s\",\
                         EVENT_TYPE=%d,EVENT_SOURCE=%d,EXT_START_TIME=\"%s\",\
                         EXT_LENGTH=%d,EXT_DATA=\"%s\",EXT_EVENT_ID=\"%s\",\
                         EXT_ANNC_TYPE=\"%s\",PLAY_SOURCE=%d,CUT_NUMBER=%d,\
                         EXT_CART_NAME=\"%s\",TITLE=\"%s\",ARTIST=\"%s\",\
                         SCHEDULED_TIME=\"%s\",ISRC=\"%s\",PUBLISHER=\"%s\",\
                         COMPOSER=\"%s\",USAGE_CODE=%d,START_SOURCE=%d,\
                         ONAIR_FLAG=\"%s\",ALBUM=\"%s\",LABEL=\"%s\"",
			(const char *)svctablename,
			length,
			(const char *)RDEscapeString(logname),
			logline->id(),
			logline->cartNumber(),
			(const char *)RDEscapeString(rdstation_conf->name()),
			(const char *)datetime.toString("yyyy-MM-dd"),
			(const char *)logline->startTime(RDLogLine::Actual).
			toString("hh:mm:ss"),
			action,
			logline->source(),
			(const char *)logline->extStartTime().
			toString("hh:mm:ss"),
			logline->extLength(),
			(const char *)RDEscapeString(logline->extData()),
			(const char *)RDEscapeString(logline->extEventId()),
			(const char *)RDEscapeString(logline->extAnncType()),
			src,
			logline->cutNumber(),
			(const char *)RDEscapeString(logline->extCartName()),
			(const char *)RDEscapeString(logline->title()),
			(const char *)RDEscapeString(logline->artist()),
			(const char *)logline->startTime(RDLogLine::Logged).
			toString("hh:mm:ss"),
			(const char *)logline->isrc(),
			(const char *)RDEscapeString(logline->publisher()),
			(const char *)RDEscapeString(logline->composer()),
			logline->usageCode(),
			logline->startSource(),
			(const char *)RDYesNo(onair_flag),
			(const char *)RDEscapeString(logline->album()),
			(const char *)RDEscapeString(logline->label()));
  q=new RDSqlQuery(sql);
  delete q;
}
