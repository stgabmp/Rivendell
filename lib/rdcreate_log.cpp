// rdcreate_log.cpp
//
// Create a new, empty Rivendell log table.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcreate_log.cpp,v 1.34.2.2 2010/02/08 23:50:13 cvs Exp $
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
#include <rdcreate_log.h>
#include <rdescape_string.h>
#include <rd.h>


void RDCreateLogTable(const QString &name)
{
  QString sql=RDCreateLogTableSql(name);
  RDSqlQuery *q=new RDSqlQuery(sql);
  delete q;
}


QString RDCreateLogTableSql(QString name)
{
  return QString().sprintf("CREATE TABLE IF NOT EXISTS `%s` \
(ID INT NOT NULL PRIMARY KEY,\
COUNT INT NOT NULL,\
TYPE INT DEFAULT 0,\
SOURCE INT NOT NULL,\
START_TIME int,\
GRACE_TIME int default 0,\
CART_NUMBER INT UNSIGNED NOT NULL,\
TIME_TYPE INT NOT NULL,\
POST_POINT enum('N','Y') default 'N',\
TRANS_TYPE INT NOT NULL,\
START_POINT INT NOT NULL DEFAULT -1,\
END_POINT INT NOT NULL DEFAULT -1,\
FADEUP_POINT int default -1,\
FADEUP_GAIN int default %d,\
FADEDOWN_POINT int default -1,\
FADEDOWN_GAIN int default %d,\
SEGUE_START_POINT INT NOT NULL DEFAULT -1,\
SEGUE_END_POINT INT NOT NULL DEFAULT -1,\
SEGUE_GAIN int default %d,\
DUCK_UP_GAIN int default 0,\
DUCK_DOWN_GAIN int default 0,\
COMMENT CHAR(255),\
LABEL CHAR(64),\
ORIGIN_USER char(255),\
ORIGIN_DATETIME datetime,\
LINK_EVENT_NAME char(64),\
LINK_START_TIME int,\
LINK_LENGTH int default 0,\
LINK_START_SLOP int default 0,\
LINK_END_SLOP int default 0,\
LINK_ID int default -1,\
LINK_EMBEDDED enum('N','Y') default 'N',\
EXT_START_TIME time,\
EXT_LENGTH int,\
EXT_CART_NAME char(32),\
EXT_DATA char(32),\
EXT_EVENT_ID char(32),\
EXT_ANNC_TYPE char(8),\
INDEX COUNT_IDX (COUNT),\
INDEX CART_NUMBER_IDX (CART_NUMBER),\
INDEX LABEL_IDX (LABEL))",
			   (const char *)
			   RDEscapeStringSQLColumn(name.replace(" ","_")),
			   RD_FADE_DEPTH,RD_FADE_DEPTH,RD_FADE_DEPTH);
}


QString RDCreateClockTableSql(QString name)
{
  return QString().sprintf("create table `%s_CLK` (\
                          ID int unsigned auto_increment not null primary key,\
                          EVENT_NAME char(64) not null,\
                          START_TIME int not null,\
                          LENGTH int not null,\
                          INDEX EVENT_NAME_IDX (EVENT_NAME))",
			   (const char *)
			   RDEscapeStringSQLColumn(name.replace(" ","_")));
}


QString RDCreateReconciliationTableSql(QString name)
{
  QString sql=QString().sprintf("create table `%s_SRT` (\
                            ID int unsigned auto_increment primary key,\
                            LENGTH int,\
                            LOG_NAME char(64),\
                            LOG_ID int,\
                            CART_NUMBER int unsigned,\
                            CUT_NUMBER int,\
                            TITLE char(255),\
                            ARTIST char(255),\
                            PUBLISHER char(64),\
                            COMPOSER char(64),\
                            ALBUM char(255),\
                            LABEL char(64),\
                            USAGE_CODE int,\
                            ISRC char(12),\
                            STATION_NAME char(64),\
                            EVENT_DATETIME datetime,\
                            SCHEDULED_TIME time,\
                            EVENT_TYPE int,\
                            EVENT_SOURCE int,\
                            PLAY_SOURCE int,\
                            START_SOURCE int default 0,\
                            ONAIR_FLAG enum('N','Y') default 'N',\
                            EXT_START_TIME time,\
                            EXT_LENGTH int,\
                            EXT_CART_NAME char(32),\
                            EXT_DATA char(32),\
                            EXT_EVENT_ID char(8),\
                            EXT_ANNC_TYPE char(8),\
                            index EVENT_DATETIME_IDX(EVENT_DATETIME))",
			   (const char *)
				RDEscapeStringSQLColumn(name.replace(" ","_")));
  // printf("SQL: %s\n",(const char *)sql);
  return sql;
}


QString RDCreateStackTableSql(QString name)
{
  QString sql;
  sql=QString().sprintf("create table if not exists `%s_STACK` (\
                         SCHED_STACK_ID int unsigned not null primary key,\
                         CART int unsigned not null,\
                         ARTIST varchar(255),\
                         SCHED_CODES varchar(255),\
                         SCHEDULED_AT datetime default '1000-01-01 00:00:00')",
			(const char *)
			RDEscapeStringSQLColumn(name.replace(" ","_")));
  return sql;
}
