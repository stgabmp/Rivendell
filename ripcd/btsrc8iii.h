// btsrc8iii.h
//
// A Rivendell switcher driver for the BroadcastTools SRC-8 III
//
//   (C) Copyright 2002-2005,2010 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: btsrc8iii.h,v 1.1.2.2 2010/05/08 23:01:57 cvs Exp $
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

#ifndef BTSRC8III_H
#define BTSRC8III_H

#include <qobject.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>
#include <rdoneshot.h>

#define BTSRC8III_UNIT_ID 0
#define BTSRC8III_POLL_INTERVAL 100
#define BTSRC8III_GPIO_PINS 8


class BtSrc8Iii : public QObject
{
 Q_OBJECT
 public:
  BtSrc8Iii(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~BtSrc8Iii();
  void processCommand(RDMacro *cmd);

 public slots:
  void processStatus();

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpiChanged(int matrix,int line,bool state);
  void gpoChanged(int matrix,int line,bool state);

 private slots:
  void gpiOneshotData(void *data);
  void gpoOneshotData(void *data);

 private:
  RDTTYDevice *bt_device;
  RDOneShot *bt_gpi_oneshot;
  RDOneShot *bt_gpo_oneshot;
  int bt_matrix;
  int bt_gpis;
  int bt_gpos;
  int bt_istate;
  bool bt_gpi_state[BTSRC8III_GPIO_PINS];
  bool bt_gpi_mask[BTSRC8III_GPIO_PINS];
};


#endif  // BTSRC8III_H
