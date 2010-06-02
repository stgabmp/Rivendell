// wall_clock.cpp
//
// A wall-clock widget with date.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: wall_clock.cpp,v 1.15 2007/10/01 10:40:45 fredg Exp $
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
#include <qbitmap.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qsize.h>
#include <qevent.h>
#include <stdio.h>
#include <string.h>

#include <rdconf.h>

#include <colors.h>
#include <wall_clock.h>
#include <globals.h>

WallClock::WallClock(QWidget *parent,const char *name)
  :RDPushButton(parent,name)
{
  time_offset=rdstation_conf->timeOffset();
  previous_time=QTime::currentTime().addMSecs(time_offset);
  time_mode=RDAirPlayConf::TwentyFourHour;
  previous_time_mode = RDAirPlayConf::TwentyFourHour;
  show_date=true;
  setFlashColor(QColor(BUTTON_TIME_SYNC_LOST_COLOR));
  check_sync=true;

  //
  // Generate Fonts
  //
  label_font=QFont("Helvetica",10,QFont::Normal);
  label_font.setPixelSize(10);
  label_metrics=new QFontMetrics(label_font);
  time_font=QFont("Helvetica",26,QFont::Normal);
  time_font.setPixelSize(26);

  connect(this,SIGNAL(clicked()),this,SLOT(clickedData()));
}


QSize WallClock::sizeHint() const
{
  return QSize(200,60);
}


QSizePolicy WallClock::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}


void WallClock::setDateDisplay(bool state)
{
  show_date=state;
}


void WallClock::setTimeMode(RDAirPlayConf::TimeMode mode)
{
  if(mode==time_mode) {
    return;
  }
  time_mode=mode;
  emit timeModeChanged(time_mode);
}


void WallClock::setCheckSyncEnabled(bool state)
{
  check_sync=state;
}


void WallClock::clickedData()
{
  if(time_mode==RDAirPlayConf::TwelveHour) {
    setTimeMode(RDAirPlayConf::TwentyFourHour);
  }
  else {
    setTimeMode(RDAirPlayConf::TwelveHour);
  }
}


void WallClock::tickClock()
{
  static QString date;
  QString accum;
  static QBitmap *pix=new QBitmap(sizeHint().width(),sizeHint().height());
  static bool synced=true;

  if(check_sync) {
    if(RDTimeSynced()!=synced) {
      synced=RDTimeSynced();
      setFlashingEnabled(!synced);
    }
  }
  current_time=QTime::currentTime().addMSecs(time_offset);
  current_date=QDate::currentDate();
  if((current_time.second()==previous_time.second()) && (previous_time_mode == time_mode)) {
    return;
  }
  previous_time_mode = time_mode;
  previous_time=current_time;

  //
  // Clock Display
  //
  if(time_mode==RDAirPlayConf::TwelveHour) {
    accum=current_time.toString("h:mm:ss ap");
  }
  else {
    accum=current_time.toString("hh:mm:ss");
  }
  if(previous_date!=current_date) {
    previous_date=current_date;
    date=current_date.toString("dddd, MMMM d, yyyy");
  }
  QPainter p(pix);
  p.eraseRect(0,0,width(),height());
  p.setPen(color1);
  p.setBrush(color1);
  p.setFont(label_font);
  p.drawText((sizeHint().width()-p.fontMetrics().width(date))/2,22,date);
  p.setFont(time_font);
  p.drawText((sizeHint().width()-p.fontMetrics().width(accum))/2,48,accum);
  p.end();
  setPixmap(*pix);
}


void WallClock::keyPressEvent(QKeyEvent *e)
{
  e->ignore();
}
