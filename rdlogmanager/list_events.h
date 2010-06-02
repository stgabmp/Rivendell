// list_events.h
//
// List Rivendell Log Events
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_events.h,v 1.15 2007/10/05 13:50:34 fredg Exp $
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

#ifndef LIST_EVENTS_H
#define LIST_EVENTS_H

#include <vector>

#include <qdialog.h>
#include <qsqldatabase.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qcombobox.h>

#include <rduser.h>
#include <rdmatrix.h>
#include <rddb.h>


class ListEvents : public QDialog
{
 Q_OBJECT
 public:
  ListEvents(QString *eventname,QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void addData();
  void editData();
  void deleteData();
  void renameData();
  void doubleClickedData(QListViewItem *,const QPoint &,int);
  void filterActivatedData(int id);
  void closeData();
  void okData();
  void cancelData();

 private:
  void RefreshList();
  void RefreshItem(QListViewItem *item,std::vector<QString> *new_events=NULL);
  void UpdateItem(QListViewItem *item,QString name);
  void WriteItem(QListViewItem *item,RDSqlQuery *q);
  int ActiveEvents(QString event_name,QString *clock_list);
  void DeleteEvent(QString event_name);
  QString GetEventFilter(QString svc_name);
  QString GetNoneFilter();
  QListView *edit_events_list;
  QString *edit_eventname;
  QComboBox *edit_filter_box;
};


#endif

