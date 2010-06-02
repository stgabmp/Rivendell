// rdaudio_port.h
//
// Abstract a Rivendell Audio Port
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdaudio_port.h,v 1.7 2007/02/14 21:48:41 fredg Exp $
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

#include <qsqldatabase.h>
#include <rdcae.h>

#ifndef RDAUDIO_PORT_H
#define RDAUDIO_PORT_H

class RDAudioPort
{
  public:
   enum PortType {Analog=0,AesEbu=1,SpDiff=2};
   enum ClockSource {InternalClock=0,AesEbuClock=1,SpDiffClock=2,WordClock=4};
   RDAudioPort(QString station,int card,bool create=false);
   QString station() const;
   int card() const;
   RDAudioPort::ClockSource clockSource();
   void setClockSource(RDAudioPort::ClockSource src);
   RDAudioPort::PortType inputPortType(int port);
   void setInputPortType(int port,RDAudioPort::PortType type);
   RDCae::ChannelMode inputPortMode(int port);
   void setInputPortMode(int port,RDCae::ChannelMode mode);
   int inputPortLevel(int port);
   void setInputPortLevel(int port,int level);
   int outputPortLevel(int port);
   void setOutputPortLevel(int port,int level);

  private:
   int GetIntValue(QString field);
   void SetRow(QString param,int value);
   QString port_station;
   int port_card;
};


#endif 
