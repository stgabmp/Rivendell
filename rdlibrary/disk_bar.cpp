// disk_bar.cpp
//
// Progress Bar for the DiskGauge Widget.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: disk_bar.cpp,v 1.4 2007/12/27 15:00:19 fredg Exp $
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

#include <disk_bar.h>

DiskBar::DiskBar(QWidget *parent,const char *name)
  : QProgressBar(parent,name)
{
  QFont font("helvetica",12,QFont::Normal);
  font.setPixelSize(12);
  setFont(font);
  prev_progress=-1;
  prev_total=-1;
}


bool DiskBar::setIndicator(QString &indicator,int progress,int totalSteps)
{
  if((prev_progress==progress)&&(prev_total==totalSteps)) {
    return false;
  }
  prev_progress=progress;
  prev_total=totalSteps;
  indicator=
    QString().sprintf("%dh %02dm",progress/60,progress-60*(progress/60));
  return true;
}
