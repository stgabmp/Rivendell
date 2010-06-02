// list_stations.h
//
// List Rivendell Stations
//
//   (C) Copyright 2002 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_stations.h,v 1.7 2007/02/14 21:51:02 fredg Exp $
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

#ifndef LIST_STATIONS_H
#define LIST_STATIONS_H

#include <qdialog.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qradiobutton.h>
#include <qsqldatabase.h>


class ListStations : public QDialog
{
  Q_OBJECT
 public:
  ListStations(QWidget *parent=0,const char *name=0);
  ~ListStations();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  
 private slots:
  void addData();
  void editData();
  void deleteData();
  void doubleClickedData(QListBoxItem *item);
  void closeData();
  
 private:
  void RefreshList(QString stationname="");
  void DeleteStation(QString name);
  QListBox *list_box;
};


#endif


