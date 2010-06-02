// rdcatchd_socket.cpp
//
// An automated event executer.
//
//   (C) Copyright 2002 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcatchd_socket.cpp,v 1.4 2007/09/14 14:06:52 fredg Exp $
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


#include <qapplication.h>
#include <qobject.h>
#include <qserversocket.h>
#include <qhostaddress.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <rdcatchd_socket.h>

RDCatchdSocket::RDCatchdSocket(Q_UINT16 port,int backlog,QObject *parent,
		     const char *name) 
  : QServerSocket(port,0,parent,name)
{
}


RDCatchdSocket::RDCatchdSocket(const QHostAddress &address,Q_UINT16 port,int backlog,
		     QObject *parent,const char *name) 
  : QServerSocket(address,port,0,parent,name)
{
}


void RDCatchdSocket::newConnection(int fd)
{
  emit connection(fd);
}
