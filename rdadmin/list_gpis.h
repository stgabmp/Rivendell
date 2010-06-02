// list_gpis.h
//
// List Rivendell GPIs
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_gpis.h,v 1.4.2.1 2008/12/08 16:44:19 fredg Exp $
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

#ifndef LIST_GPIS_H
#define LIST_GPIS_H

#include <qdialog.h>
#include <qsqldatabase.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include <rduser.h>
#include <rdmatrix.h>


class ListGpis : public QDialog
{
 Q_OBJECT
 public:
  ListGpis(RDMatrix *matrix,RDMatrix::GpioType type,
	   QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void editData();
  void doubleClickedData(QListViewItem *,const QPoint &,int);
  void okData();
  void cancelData();

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  RDMatrix *list_matrix;
  RDMatrix::GpioType list_type;
  QString list_tablename;
  QListView *list_list_view;
  QLabel *list_list_label;
  int list_size;
  QPushButton *list_edit_button;
  QPushButton *list_ok_button;
  QPushButton *list_cancel_button;
};


#endif

