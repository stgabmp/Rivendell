//   rdlistview.h
//
//   A contiguous-selection only QListView widget for Rivendell
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdlistview.h,v 1.9 2007/02/14 21:48:41 fredg Exp $
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
//

#ifndef RDLISTVIEW_H
#define RDLISTVIEW_H

#include <vector>

#include <qlistview.h>
#include <qpixmap.h>


class RDListView : public QListView
{
  Q_OBJECT

 public:
  enum SortType {NormalSort=0,TimeSort=1,LineSort=2};
  RDListView(QWidget *parent,const char *name=0);
  int hardSortColumn() const;
  void setHardSortColumn(int col);
  RDListView::SortType columnSortType(int column) const;
  void setColumnSortType(int column,SortType type);
  int addColumn(const QString &label,int width=-1);
  int addColumn(const QIconSet &iconset,const QString &label,int width=-1);
  void selectLine(int line);

 protected:
  void keyPressEvent(QKeyEvent *e);
  void contentsMousePressEvent(QMouseEvent *e);
  void contentsMouseReleaseEvent(QMouseEvent *e);

 private:
  int list_hard_sort_column;
  std::vector<RDListView::SortType> sort_type;
};


#endif  // RDLISTVIEW_H
