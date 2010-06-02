// marker_bar.h
//
// A marker widget for the EditEvent dialog.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: marker_bar.h,v 1.3.6.1 2009/03/30 19:02:52 cvs Exp $
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


#ifndef MARKER_BAR_H
#define MARKER_BAR_H

#include <qlabel.h>
#include <qpixmap.h>

#include <colors.h>


class MarkerBar : public QLabel
{
  Q_OBJECT
 public:
  enum Marker {Play=0,Start=1,End=2,MaxSize=3};
  MarkerBar(QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 public slots:
  int length() const;
  void setLength(int msecs);
  int marker(Marker marker) const;
  void setMarker(Marker marker,int msecs);

 private:
  void DrawMap();
  int marker_pos[MarkerBar::MaxSize];
  int marker_length;
};


#endif 
