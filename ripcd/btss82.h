// btss82.h
//
// A Rivendell switcher driver for the BroadcastTools SS 8.2
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: btss82.h,v 1.6 2008/09/30 18:19:42 fredg Exp $
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

#ifndef BTSS82_H
#define BTSS82_H

#include <qobject.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>
#include <rdoneshot.h>

#define BTSS82_UNIT_ID 0
#define BTSS82_POLL_INTERVAL 100
#define BTSS82_GPIO_PINS 16


class BtSs82 : public QObject
{
 Q_OBJECT
 public:
  BtSs82(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~BtSs82();
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
  int bt_inputs;
  int bt_outputs;
  int bt_gpis;
  int bt_gpos;
  int bt_istate;
  bool bt_gpi_state[BTSS82_GPIO_PINS];
  bool bt_gpi_mask[BTSS82_GPIO_PINS];
};


#endif  // BTSS82_H
