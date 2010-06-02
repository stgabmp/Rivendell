//   lib_listview.cpp
//
//   The Library ListView widget for RDLogManager.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: lib_listview.cpp,v 1.5 2007/02/14 21:57:04 fredg Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
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

#include <qdragobject.h>

#include <lib_listview.h>


LibListView::LibListView(QWidget *parent,const char *name)
  : QListView(parent,name)
{
}


void LibListView::focusOutEvent(QFocusEvent *e)
{
  QListViewItem *item=selectedItem();
  if(item==NULL) {
    return;
  }
  setSelected(item,false);
}


void LibListView::contentsMousePressEvent(QMouseEvent *e)
{
  QListView::contentsMousePressEvent(e);
  QListViewItem *item=selectedItem();
  if(item==NULL) {
    return;
  }
  QDragObject *d=new QTextDrag(item->text(1),this);
  d->dragCopy();
  emit clicked(item);
}

