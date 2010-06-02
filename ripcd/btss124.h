// btss124.h
//
// A Rivendell switcher driver for the BroadcastTools SS 8.2
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: btss124.h,v 1.4 2007/09/14 14:06:59 fredg Exp $
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

#ifndef BTSS124_H
#define BTSS124_H

#include <qobject.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>

#define BTSS124_UNIT_ID 0


class BtSs124 : public QObject
{
 Q_OBJECT
 public:
  BtSs124(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~BtSs124();
  void processCommand(RDMacro *cmd);

 public slots:
  void processStatus();

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpiChanged(int matrix,int line,bool state);

 private:
  RDTTYDevice *bt_device;
  int bt_matrix;
  int bt_inputs;
  int bt_outputs;
  int bt_gpis;
  int bt_gpos;
};


#endif  // BTSS124_H
