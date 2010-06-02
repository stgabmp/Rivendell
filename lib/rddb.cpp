// rddb.cpp
//
//   Database driver with automatic reconnect
//
//   (C) Copyright 2007 Dan Mills <dmills@exponent.myzen.co.uk>
//
//      $Id: rddb.cpp,v 1.7 2008/03/28 20:00:05 fredg Exp $
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

#include <qobject.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qserversocket.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <assert.h>

#include "rddb.h"
#include "rddbheartbeat.h"

static QSqlDatabase *db = NULL;
static RDSqlDatabaseStatus * dbStatus = NULL;

QSqlDatabase * RDInitDb (QString *error)
{
  static bool firsttime = true;

  RDConfig *cf = RDConfiguration();
  cf->load();
  assert (cf);
  if (!db){
    db=QSqlDatabase::addDatabase(cf->mysqlDriver());
    if(!db) {
      if (error){
	(*error) += QString(QObject::tr("Couldn't initialize QSql driver!"));
      }
      return NULL;
    }
    db->setDatabaseName(cf->mysqlDbname());
    db->setUserName(cf->mysqlUsername());
    db->setPassword(cf->mysqlPassword());
    db->setHostName(cf->mysqlHostname());
    if(!db->open()) {
      if (error){
	(*error) += QString(QObject::tr("Couldn't open mySQL connection!"));
      }
      db->removeDatabase(cf->mysqlDbname());
      db->close();
      return NULL;
    }
  }
  if (firsttime){
    new RDDbHeartbeat(cf->mysqlHeartbeatInterval());
    firsttime = false;
  }
  return db;
}

RDSqlQuery::RDSqlQuery (const QString &query, QSqlDatabase *dbase):
  QSqlQuery (query,dbase)
{
  // With any luck, by the time we get here, we have already done the biz...
  if (!isActive()){ //DB Offline?
    QSqlDatabase *ldb = QSqlDatabase::database();
    // Something went wrong with the DB, trying a reconnect
    ldb->removeDatabase(RDConfiguration()->mysqlDbname());
    ldb->close();
    db = NULL;
    RDInitDb ();
    QSqlQuery::prepare (query);
    QSqlQuery::exec ();
    if (RDDbStatus()){
      if (isActive()){
	RDDbStatus()->sendRecon();
      } else {
	RDDbStatus()->sendDiscon(query);
      }
    }
  } else {
    RDDbStatus()->sendRecon();
  }
}
void RDSqlDatabaseStatus::sendRecon()
{
  if (discon){
    discon = false;
    emit reconnected();
    fprintf (stderr,"Database connection restored.\n");
    emit logText(RDConfig::LogErr,QString(tr("Database connection restored.")));
  }
}

void RDSqlDatabaseStatus::sendDiscon(QString query)
{
  if (!discon){
    emit connectionFailed();
    fprintf (stderr,"Database connection failed: %s\n",(const char *)query);
    emit logText(RDConfig::LogErr,
		 QString(tr("Database connection failed : ")) + query);
    discon = true;
  }
}

RDSqlDatabaseStatus::RDSqlDatabaseStatus()
{
  discon = false;
}

RDSqlDatabaseStatus * RDDbStatus()
{
  if (!dbStatus){
    dbStatus = new RDSqlDatabaseStatus;
  }
  return dbStatus;
}
