// rdcae.h
//
// Connection to the Rivendell Core Audio Engine
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcae.h,v 1.24 2007/09/14 14:06:24 fredg Exp $
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

#include <vector>

#include <qsqldatabase.h>
#include <qstring.h>
#include <qobject.h>
#include <qsocketdevice.h>
#include <qlabel.h>

#include <rd.h>
#include <rdcmd_cache.h>


#ifndef RDCAE_H
#define RDCAE_H



class RDCae : public QObject
{
 Q_OBJECT
 public:
  enum ChannelMode {Normal=0,Swap=1,LeftOnly=2,RightOnly=3};
  enum SourceType {Analog=0,AesEbu=1};
  enum AudioCoding {Pcm16=0,MpegL1=1,MpegL2=2,MpegL3=3};
  RDCae(QObject *parent=0,const char *name=0);
  ~RDCae();
  void connectHost(QString hostname,Q_UINT16 hostport,QString password);
  bool loadPlay(int card,QString name,int *stream,int *handle);
  void unloadPlay(int handle);
  void positionPlay(int handle,int pos);
  void play(int handle,unsigned length,int speed,bool pitch);
  void stopPlay(int handle);
  void loadRecord(int card,int stream,QString name,AudioCoding coding,
		  int chan,int samp_rate,int bit_rate);
  void unloadRecord(int card,int stream);
  void record(int card,int stream,unsigned length,int threshold);
  void stopRecord(int card,int stream);
  void setInputVolume(int card,int stream,int level);
  void setOutputVolume(int card,int stream,int port,int level);
  void fadeOutputVolume(int card,int stream,int port,int level,int length);
  void setInputLevel(int card,int port,int level);
  void setOutputLevel(int card,int port,int level);
  void setInputMode(int card,int stream,RDCae::ChannelMode mode);
  void setOutputMode(int card,int stream,RDCae::ChannelMode mode);
  void setInputVOXLevel(int card,int stream,int level);
  void setInputType(int card,int port,RDCae::SourceType type);
  void setPassthroughVolume(int card,int in_port,int out_port,int level);
  bool inputStatus(int card,int port) const;
  void inputMeterUpdate(int card,int port,short levels[2]) const;
  void outputMeterUpdate(int card,int port,short levels[2]) const;
  unsigned playPosition(int handle);
  void requestTimescale(int card);
  bool playPortActive(int card,int port,int except_stream=-1);
  void setPlayPortActive(int card,int port,int stream);

 signals:
  void isConnected(bool state);
  void playLoaded(int handle);
  void playPositioned(int handle,unsigned pos);
  void playing(int handle);
  void playStopped(int handle);
  void playUnloaded(int handle);
  void recordLoaded(int card,int stream);
  void recording(int card,int stream);
  void recordStopped(int card,int stream);
  void recordUnloaded(int card,int stream);
  void gpiInputChanged(int line,bool state);
  void connected(bool state);
  void inputStatusChanged(int card,int stream,bool state);
  void playPositionChanged(int handle,unsigned sample);
  void timescalingSupported(int card,bool state);

 private slots:
  void readyData(int *stream=0,int *handle=0,QString name="");
  void clockData();
  
 private:
  void SendCommand(QString cmd);
  void DispatchCommand(RDCmdCache *cmd);
  int CardNumber(const char *arg);
  int StreamNumber(const char *arg);
  int GetHandle(const char *arg);
  bool restartCaed(void);
  bool restart_lock;
  QString save_hostname;
  Q_UINT16 save_hostport;
  QString save_password;
  RDMeterBlock *meter_block;
  QSocketDevice *cae_socket;
  QString cae_password;
  bool debug;
  char args[CAE_MAX_ARGS][CAE_MAX_LENGTH];
  int argnum;
  int argptr;
  bool cae_connected;
  bool input_status[RD_MAX_CARDS][RD_MAX_PORTS];
  int meter_block_id;
  int cae_handle[RD_MAX_CARDS][RD_MAX_STREAMS];
  unsigned cae_pos[RD_MAX_CARDS][RD_MAX_STREAMS];
  std::vector<RDCmdCache> delayed_cmds;
};


#endif 
