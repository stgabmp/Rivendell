// rdadmin.h
//
// The Administration Utility for Rivendell.
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdadmin.h,v 1.22.2.1 2009/03/24 12:11:19 cvs Exp $
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


#ifndef RDADMIN_H
#define RDADMIN_H

#include <qwidget.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qsqldatabase.h>
#include <qpixmap.h>

#include <rduser.h>
#include <rdconfig.h>

#define RDADMIN_USAGE "[--check-db]\n\nWhen invoked with the '--check-db' flag, RDAdmin will check and (if necessary)\nupdate the database schema to the current version, then exit.\n"

class MainWidget : public QWidget
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  ~MainWidget();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  
 private slots:
  void manageUsersData();
  void manageGroupsData();
  void manageServicesData();
  void manageStationsData();
  void systemSettingsData();
  void reportsData();
  void podcastsData();
  void quitMainWidget();
  void manageSchedCodes();
  void systemInfoData();
  void backupData();
  void restoreData();
  
 private:
  void ClearTables();
  QString admin_username;
  QString admin_password;
  QPixmap *admin_rivendell_map;
  QString admin_filter;
  QString admin_group;
};


#endif 
