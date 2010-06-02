// local_audio.h
//
// A Rivendell switcher driver for the BroadcastTools 10x1
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: local_audio.h,v 1.3 2007/02/14 21:57:04 fredg Exp $
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

#ifndef LOCAL_AUDIO_H
#define LOCAL_AUDIO_H

#include <qobject.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>


class LocalAudio : public QObject
{
 Q_OBJECT
 public:
  LocalAudio(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~LocalAudio();
  void processCommand(RDMacro *cmd);
  void processStatus(char *buf,int size);

 signals:
  void rmlEcho(RDMacro *cmd);

 private:
  int bt_inputs;
  int bt_outputs;
  int bt_card;
};


#endif  // LOCAL_AUDIO_H
