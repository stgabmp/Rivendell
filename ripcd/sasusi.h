// sasusi.h
//
// A Rivendell switcher driver for the SAS User Serial Interface Protocol
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: sasusi.h,v 1.8.2.1 2009/02/10 16:19:33 cvs Exp $
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

#ifndef SASUSI_H
#define SASUSI_H

#include <qobject.h>
#include <qsocket.h>
#include <qhostaddress.h>
#include <qtimer.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>

#define SASUSI_RECONNECT_INTERVAL 10000
#define SASUSI_MAX_LENGTH 256

class SasUsi : public QObject
{
 Q_OBJECT
 public:
  SasUsi(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  void processCommand(RDMacro *cmd);

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpoChanged(int matrix,int line,bool state);

 private slots:
  void ipConnect();
  void connectedData();
  void connectionClosedData();
  void readyReadData();
  void errorData(int err);

 private:
  void SendCommand(char *str);
  void DispatchCommand();
  void ExecuteMacroCart(unsigned cartnum);
  RDTTYDevice *sas_device;
  QSocket *sas_socket;
  char sas_buffer[SASUSI_MAX_LENGTH];
  unsigned sas_ptr;
  QHostAddress sas_ipaddress;
  int sas_matrix;
  int sas_ipport;
  int sas_inputs;
  int sas_outputs;
  int sas_gpis;
  int sas_gpos;
  QTimer *sas_reconnect_timer;
  unsigned sas_start_cart;
  unsigned sas_stop_cart;
  RDMatrix::PortType sas_porttype;
};


#endif  // SASUSI_H
