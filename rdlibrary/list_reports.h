// list_reports.h
//
// List and Generate RDLibrary Reports
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_reports.h,v 1.3 2007/02/14 21:55:07 fredg Exp $
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

#ifndef LIST_REPORTS_H
#define LIST_REPORTS_H

#include <qdialog.h>
#include <qcombobox.h>
#include <qsqldatabase.h>

#include <rdsvc.h>


class ListReports : public QDialog
{
 Q_OBJECT
 public:
 ListReports(const QString &filter,const QString &type_filter,
	     const QString &group,QWidget *parent=0,const char *name=0);
 ~ListReports();
 QSize sizeHint() const;
 QSizePolicy sizePolicy() const;
 
 private slots:
  void generateData();
  void closeData();

 private:
  void GenerateCartReport(QString *report);
  void GenerateCutReport(QString *report);
  void GenerateCartDump(QString *report);
  QComboBox *list_reports_box;
  QString list_filter;
  QString list_type_filter;
  QString list_group;
};


#endif  // LIST_REPORTS_H
