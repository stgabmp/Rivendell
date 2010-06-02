// rdairplay_conf.h
//
// Abstract RDAirPlay Configuration
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdairplay_conf.h,v 1.31.2.3 2009/03/26 20:27:57 cvs Exp $
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

#ifndef RDAIRPLAY_CONF_H
#define RDAIRPLAY_CONF_H

#include <qsqldatabase.h>
#include <qhostaddress.h>

#include <rdlog_line.h>         


class RDAirPlayConf
{
 public:
  enum TimeMode {TwelveHour=0,TwentyFourHour=1};
  enum OpMode {Previous=0,LiveAssist=1,Auto=2,Manual=3};
  enum ActionMode {Normal=0,AddFrom=1,AddTo=2,DeleteFrom=3,MoveFrom=4,MoveTo=5,
		   CopyFrom=6,CopyTo=7,Audition=8};
  enum PieEndPoint {CartEnd=0,CartTransition=1};
  enum BarAction {NoAction=0,StartNext=1};
  enum TrafficAction {TrafficStart=1,TrafficStop=2,TrafficFinish=3,
		      TrafficPause=4,TrafficMacro=5};
  enum PanelType {StationPanel=0,UserPanel=1};
  enum ExitCode {ExitClean=0,ExitDirty=1};
  enum StartMode {StartEmpty=0,StartPrevious=1,StartSpecified=2};
  RDAirPlayConf(const QString &station,unsigned instance,
		const QString &tablename="RDAIRPLAY");
  QString station() const;
  unsigned instance() const;
  int card(int num) const;
  void setCard(int num,int card) const;
  int port(int num) const;
  void setPort(int num,int port) const;
  QString startRml(int num) const;
  void setStartRml(int num,QString str) const;
  QString stopRml(int num) const;
  void setStopRml(int num,QString str) const;
  int segueLength() const;
  void setSegueLength(int len) const;
  int transLength() const;
  void setTransLength(int len) const;
  RDAirPlayConf::OpMode opMode() const;
  void setOpMode(RDAirPlayConf::OpMode mode) const;
  RDAirPlayConf::OpMode startMode() const;
  void setStartMode(RDAirPlayConf::OpMode mode) const;
  int pieCountLength() const;
  void setPieCountLength(int len) const;
  RDAirPlayConf::PieEndPoint pieEndPoint() const;
  void setPieEndPoint(RDAirPlayConf::PieEndPoint point) const;
  bool checkTimesync() const;
  void setCheckTimesync(bool state) const;
  int panels(RDAirPlayConf::PanelType type) const;
  void setPanels(RDAirPlayConf::PanelType type,int quan) const;
  bool showAuxButton(int auxbutton) const;
  void setShowAuxButton(int auxbutton,bool state) const;
  bool clearFilter() const;
  void setClearFilter(bool state) const;
  RDLogLine::TransType defaultTransType() const;
  void setDefaultTransType(RDLogLine::TransType type) const;
  RDAirPlayConf::BarAction barAction() const;
  void setBarAction(RDAirPlayConf::BarAction action) const;
  bool flashPanel() const;
  void setFlashPanel(bool state) const;
  bool panelPauseEnabled() const;
  void setPanelPauseEnabled(bool state) const;
  QString buttonLabelTemplate() const;
  void setButtonLabelTemplate(const QString &str) const;
  bool pauseEnabled() const;
  void setPauseEnabled(bool state) const;
  QString defaultSvc() const;
  void setDefaultSvc(const QString &svcname) const;
  QHostAddress udpAddress(int logno) const;
  void setUdpAddress(int logno,QHostAddress addr) const;
  Q_UINT16 udpPort(int logno) const;
  void setUdpPort(int logno,Q_UINT16 port) const;
  QString udpString(int logno) const;
  void setUdpString(int logno,const QString &str) const;
  QString logRml(int logno) const;
  void setLogRml(int logno,const QString &str) const;
  RDAirPlayConf::ExitCode exitCode() const;
  void setExitCode(RDAirPlayConf::ExitCode code) const;
  bool exitPasswordValid(const QString &passwd) const;
  void setExitPassword(const QString &passwd) const;
  QString skinPath() const;
  void setSkinPath(const QString &path) const;
  bool showCounters() const;
  void setShowCounters(bool state) const;
  int auditionPreroll() const;
  void setAuditionPreroll(int msecs) const;
  RDAirPlayConf::StartMode startMode(int lognum) const;
  void setStartMode(int lognum,RDAirPlayConf::StartMode mode) const;
  bool autoRestart(int lognum) const;
  void setAutoRestart(int lognum,bool state) const;
  QString logName(int lognum) const;
  void setLogName(int lognum,const QString &name) const;
  QString currentLog(int lognum) const;
  void setCurrentLog(int lognum,const QString &name) const;
  bool logRunning(int lognum) const;
  void setLogRunning(int lognum,bool state) const;
  int logId(int lognum) const;
  void setLogId(int lognum,int id) const;
  int logCurrentLine(int lognum) const;
  void setLogCurrentLine(int lognum,int line) const;
  unsigned logNowCart(int lognum) const;
  void setLogNowCart(int lognum,unsigned cartnum) const;
  unsigned logNextCart(int lognum) const;
  void setLogNextCart(int lognum,unsigned cartnum) const;

 private:
  void SetRow(const QString &param,int value) const;
  void SetRow(const QString &param,unsigned value) const;
  void SetRow(const QString &param,const QString &value) const;
  QString air_station;
  unsigned air_instance;
  unsigned air_id;
  QString air_tablename;
};


#endif  // RDAIRPLAY_CONF_H
