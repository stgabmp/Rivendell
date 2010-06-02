// disk_bar.h
//
// Progress Bar for the DiskGauge Widget.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: disk_bar.h,v 1.3 2007/02/14 21:55:07 fredg Exp $
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

#ifndef DISK_BAR_H
#define DISK_BAR_H

#include <qprogressbar.h>


class DiskBar : public QProgressBar
{
 public:
  DiskBar(QWidget *parent,const char *name=0);

 protected:
  bool setIndicator(QString &indicator,int progress,int totalSteps);

 private:
  int prev_progress;
  int prev_total;
};

#endif  // DISK_BAR_H
