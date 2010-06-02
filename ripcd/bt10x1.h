// bt10x1.h
//
// A Rivendell switcher driver for the BroadcastTools 10x1
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: bt10x1.h,v 1.4 2007/09/14 14:06:59 fredg Exp $
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

#ifndef BT10X1_H
#define BT10X1_H

#include <qobject.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>

#define BT10X1_MIN_GAIN -99
#define BT10X1_MAX_GAIN 28

class Bt10x1 : public QObject
{
 Q_OBJECT
 public:
 Bt10x1(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~Bt10x1();
  void processCommand(RDMacro *cmd);
  void processStatus(char *buf,int size);

 signals:
  void rmlEcho(RDMacro *cmd);

 private:
  RDTTYDevice *bt_device;
  int bt_inputs;
  int bt_outputs;
};


#endif  // BT10X1_H
