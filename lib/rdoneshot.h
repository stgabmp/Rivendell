// rdoneshot.h
//
// A class for providing one-shot single use timers.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdoneshot.h,v 1.1 2008/09/30 18:19:40 fredg Exp $
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

#ifndef RDONESHOT_H
#define RDONESHOT_H

#include <map>

#include <qobject.h>
#include <qsignalmapper.h>
#include <qtimer.h>

class RDOneShot : public QObject
{
  Q_OBJECT
 public:
  RDOneShot(QObject *parent=0,const char *name=0);
  void start(void *data,int msecs);

 signals:
  void timeout(void *data);

 private slots:
  void timeoutData(int id);
  void zombieData();

 private:
  std::map<int,QTimer *> shot_timers;
  std::map<int,void *> shot_pointers;
  QSignalMapper *shot_mapper;
  QTimer *shot_zombie_timer;
  int shot_count;
};


#endif  // RDONESHOT_H
