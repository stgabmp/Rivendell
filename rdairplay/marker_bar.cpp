// marker_bar.cpp
//
// A marker widget for the EditEvent dialog.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: marker_bar.cpp,v 1.4.6.1 2009/03/30 19:02:52 cvs Exp $
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

#include <qpainter.h>
#include <qpointarray.h>

#include <marker_bar.h>
#include <colors.h>


MarkerBar::MarkerBar(QWidget *parent,const char *name)
  : QLabel(parent,name)
{
  for(int i=0;i<MarkerBar::MaxSize;i++) {
    marker_pos[i]=0;
  }
  setLineWidth(1);
  setMidLineWidth(0);
  setFrameStyle(QFrame::Box|QFrame::Plain);
}


QSize MarkerBar::sizeHint() const
{
  return QSize(425,14);
}


QSizePolicy MarkerBar::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


int MarkerBar::length() const
{
  return marker_length;
}


void MarkerBar::setLength(int msecs)
{
  marker_length=msecs;
  DrawMap();
}


int MarkerBar::marker(Marker marker) const
{
  if(marker>=MarkerBar::MaxSize) {
    return 0;
  }
  return marker_pos[marker];
}


void MarkerBar::setMarker(Marker marker,int msecs)
{
  if(marker>=MarkerBar::MaxSize) {
    return;
  }
  marker_pos[marker]=msecs;
  DrawMap();
}


void MarkerBar::DrawMap()
{
  QPixmap *pix=new QPixmap(size());
  QPainter *p=new QPainter(pix);
  QPointArray *pt;
  p->fillRect(0,0,size().width(),size().height(),backgroundColor());
  if(marker_length>0) {
    p->setPen(EVENT_EDITOR_START_MARKER);
    p->setBrush(EVENT_EDITOR_START_MARKER);
    p->fillRect(size().width()*marker_pos[MarkerBar::Start]/marker_length-2,0,
		4,size().height(),EVENT_EDITOR_START_MARKER);
    pt=new QPointArray(3);
    pt->setPoint(0,size().width()*marker_pos[MarkerBar::Start]/marker_length-2,
		 size().height()/2-1);
    pt->setPoint(1,size().width()*marker_pos[MarkerBar::Start]/marker_length-12,
		 size().height()-2);
    pt->setPoint(2,size().width()*marker_pos[MarkerBar::Start]/marker_length-12,
		 1);
    p->drawPolygon(*pt);

    p->fillRect(size().width()*marker_pos[MarkerBar::End]/marker_length-2,0,
		4,size().height(),EVENT_EDITOR_START_MARKER);
    pt->setPoint(0,size().width()*marker_pos[MarkerBar::End]/marker_length+2,
		 size().height()/2-1);
    pt->setPoint(1,size().width()*marker_pos[MarkerBar::End]/marker_length+12,
		 size().height()-2);
    pt->setPoint(2,size().width()*marker_pos[MarkerBar::End]/marker_length+12,
		 1);
    p->drawPolygon(*pt);
    delete pt;

    p->setPen(EVENT_EDITOR_PLAY_MARKER);
    p->setBrush(EVENT_EDITOR_PLAY_MARKER);
    p->fillRect(size().width()*marker_pos[MarkerBar::Play]/marker_length-1,0,
		2,size().height(),EVENT_EDITOR_PLAY_MARKER);
  }
  p->end();
  setPixmap(*pix);
  delete p;
  delete pix;
}

