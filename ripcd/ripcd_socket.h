// ripcd_socket.h
//
// Rivendell Interprocess Communication Daemon
//
//   (C) Copyright 2002 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: ripcd_socket.h,v 1.3 2007/02/14 21:57:04 fredg Exp $
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

#ifndef RIPCD_SOCKET_H
#define RIPCD_SOCKET_H

#include <qobject.h>
#include <qstring.h>
#include <qserversocket.h>
#include <qhostaddress.h>


class RipcdSocket : public QServerSocket
{
  Q_OBJECT
  public:
   RipcdSocket(Q_UINT16 port,int backlog=0,QObject *parent=0,
	       const char *name=0);
   RipcdSocket(const QHostAddress &address,Q_UINT16 port,int backlog=0,
	       QObject *parent=0,const char *name=0);
   void newConnection(int socket);

  signals:
   void connection(int);

  private:
   QServerSocket *socket;
};


#endif 
