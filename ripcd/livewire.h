// livewire.h
//
// A Rivendell switcher driver for LiveWire networks.
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: livewire.h,v 1.2 2008/09/30 18:19:43 fredg Exp $
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

#ifndef LIVEWIRE_H
#define LIVEWIRE_H

#include <vector>

#include <qobject.h>
#include <qsocket.h>
#include <qhostaddress.h>
#include <qtimer.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdlivewire.h>
#include <rdoneshot.h>

class LiveWire : public QObject
{
 Q_OBJECT
 public:
  LiveWire(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~LiveWire();
  void processCommand(RDMacro *cmd);
  void sendGpi();
  void sendGpo();

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpiChanged(int matrix,int line,bool state);
  void gpoChanged(int matrix,int line,bool state);
  void gpiState(int matrix,unsigned line,bool state);
  void gpoState(int matrix,unsigned line,bool state);

 private slots:
  void nodeConnectedData(unsigned id);
  void sourceChangedData(unsigned id,RDLiveWireSource *src);
  void destinationChangedData(unsigned id,RDLiveWireDestination *dst);
  void gpoConfigChangedData(unsigned id,unsigned slot,unsigned chan);
  void gpiChangedData(unsigned id,unsigned slot,unsigned line,bool state);
  void gpoChangedData(unsigned id,unsigned slot,unsigned line,bool state);
  void watchdogStateChangedData(unsigned id,const QString &msg);
  void gpiOneshotData(void *data);
  void gpoOneshotData(void *data);

 private:
  void CreateGpioEntry(const QString &tablename,int chan);
  QString livewire_stationname;
  RDOneShot *livewire_gpi_oneshot;
  RDOneShot *livewire_gpo_oneshot;
  int livewire_matrix;
  std::vector<RDLiveWire *> livewire_nodes;
};


#endif  // LIVEWIRE_H
