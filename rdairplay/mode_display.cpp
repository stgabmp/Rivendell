// mode_display.cpp
//
// The mode display widget for RDAirPlay in Rivendell
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: mode_display.cpp,v 1.11 2007/02/14 21:53:27 fredg Exp $
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

#include <mode_display.h>
#include <colors.h>

ModeDisplay::ModeDisplay(QWidget *parent,const char *name)
  : QPushButton(parent,name)
{
  mode_mode=RDAirPlayConf::LiveAssist;

  //
  // Generate Fonts
  //
  mode_large_font=QFont("Helvetica",26,QFont::Normal);
  mode_large_font.setPixelSize(26);
  mode_small_font=QFont("Helvetica",12,QFont::Normal);
  mode_small_font.setPixelSize(12);

  //
  // Create Palettes
  //
  auto_color=
    QPalette(QColor(BUTTON_MODE_AUTO_COLOR),backgroundColor());
  live_assist_color=
    QPalette(QColor(BUTTON_MODE_LIVE_ASSIST_COLOR),backgroundColor());
  manual_color=
    QPalette(QColor(BUTTON_MODE_MANUAL_COLOR),backgroundColor());

  setPalette(live_assist_color);
  DrawMaps();
  setPixmap(*mode_bitmap[0]);
}


QSize ModeDisplay::sizeHint() const
{
  return QSize(200,60);
}


QSizePolicy ModeDisplay::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void ModeDisplay::setMode(RDAirPlayConf::OpMode mode)
{
  if(mode==mode_mode) {
    return;
  }
  mode_mode=mode;
  switch(mode) {
      case RDAirPlayConf::LiveAssist:
	setPixmap(*mode_bitmap[0]);
	setPalette(live_assist_color);
	break;

      case RDAirPlayConf::Auto:
	setPixmap(*mode_bitmap[1]);
	setPalette(auto_color);
	break;

      case RDAirPlayConf::Manual:
	setPixmap(*mode_bitmap[2]);
	setPalette(manual_color);
	break;

      default:
	break;
  }
}


void ModeDisplay::DrawMaps()
{
  mode_bitmap[0]=new QBitmap(sizeHint().width(),sizeHint().height());
  QPainter *p=new QPainter(mode_bitmap[0]);
  p->eraseRect(0,0,sizeHint().width(),sizeHint().height());
  p->setPen(QColor(color1));
  p->setFont(mode_small_font);
  p->drawText((sizeHint().width()-p->fontMetrics().
	       width(tr("Operating Mode")))/2,
	      22,tr("Operating Mode"));
  p->setFont(mode_large_font);
  p->drawText((sizeHint().width()-p->fontMetrics().width(tr("LiveAssist")))/2,
	      48,tr("LiveAssist"));
  p->end();

  mode_bitmap[1]=new QBitmap(sizeHint().width(),sizeHint().height());
  p->begin(mode_bitmap[1]);
  p->eraseRect(0,0,sizeHint().width(),sizeHint().height());
  p->setPen(QColor(color1));
  p->setFont(mode_small_font);
  p->drawText((sizeHint().width()-p->fontMetrics().
	       width(tr("Operating Mode")))/2,
	      22,tr("Operating Mode"));
  p->setFont(mode_large_font);
  p->drawText((sizeHint().width()-p->fontMetrics().width(tr("Automatic")))/2,
	      49,tr("Automatic"));
  p->end();

  mode_bitmap[2]=new QBitmap(sizeHint().width(),sizeHint().height());
  p=new QPainter(mode_bitmap[2]);
  p->eraseRect(0,0,sizeHint().width(),sizeHint().height());
  p->setPen(QColor(color1));
  p->setFont(mode_small_font);
  p->drawText((sizeHint().width()-p->fontMetrics().
	       width(tr("Operating Mode")))/2,
	      22,tr("Operating Mode"));
  p->setFont(mode_large_font);
  p->drawText((sizeHint().width()-p->fontMetrics().width(tr("Manual")))/2,
	      48,tr("Manual"));
  p->end();
  delete p;
}
