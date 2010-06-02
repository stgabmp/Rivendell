//   rdlistview.cpp
//
//   A contiguous-selection only QListView widget for Rivendell
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdlistview.cpp,v 1.9 2007/03/12 11:57:01 fredg Exp $
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

#include <rdlistview.h>
#include <rdlistviewitem.h>


RDListView::RDListView(QWidget *parent,const char *name)
  : QListView(parent,name)
{
  list_hard_sort_column=-1;
}


int RDListView::hardSortColumn() const
{
  return list_hard_sort_column;
}


void RDListView::setHardSortColumn(int col)
{
  list_hard_sort_column=col;
}


RDListView::SortType RDListView::columnSortType(int column) const
{
  return sort_type[column];
}


void RDListView::setColumnSortType(int column,SortType type)
{
  sort_type[column]=type;
}


int RDListView::addColumn(const QString &label,int width)
{
  sort_type.push_back(RDListView::NormalSort);
  return QListView::addColumn(label,width);
}


int RDListView::addColumn(const QIconSet &iconset,const QString &label,
			  int width)
{
  sort_type.push_back(RDListView::NormalSort);
  return QListView::addColumn(iconset,label,width);
}


void RDListView::selectLine(int line)
{
  RDListViewItem *item=(RDListViewItem *)firstChild();
  while(item!=NULL) {
    if(item->line()==line) {
      setSelected(item,true);
      return;
    }
    item=(RDListViewItem *)item->nextSibling();
  }
}


void RDListView::keyPressEvent(QKeyEvent *e)
{
 // e->ignore();
}


void RDListView::contentsMousePressEvent(QMouseEvent *e)
{
 // if((e->state()&ControlButton)==0) {
    QListView::contentsMousePressEvent(e);
 // }
}


void RDListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
 // if((e->state()&ControlButton)==0) {
    QListView::contentsMouseReleaseEvent(e);
 // }
}
