// ripcd.h
//
// Rivendell Interprocess Communication Daemon
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: ripcd.h,v 1.47.2.3.2.2 2010/05/08 23:01:57 cvs Exp $
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

#ifndef RIPCD_H
#define RIPCD_H

#include <sys/types.h>

#include <qobject.h>
#include <qstring.h>
#include <qserversocket.h>
#include <qsqldatabase.h>
#include <qsocketdevice.h>
#include <qtimer.h>

#include <rdsocket.h>
#include <rdttydevice.h>
#include <rdcodetrap.h>
#include <rdstation.h>
#include <rdmatrix.h>
#include <rdmacro.h>

#include <globals.h>
#include <local_gpio.h>
#include <sas32000.h>
#include <sas64000.h>
#include <unity4000.h>
#include <bt10x1.h>
#include <sas64000gpi.h>
#include <btss82.h>
#include <bt16x1.h>
#include <bt8x2.h>
#include <btacs82.h>
#include <sasusi.h>
#include <bt16x2.h>
#include <btss124.h>
#include <local_audio.h>
#include <vguest.h>
#include <btss164.h>
#include <starguide3.h>
#include <btss42.h>
#include <livewire.h>
#include <quartz1.h>
#include <btss44.h>
#include <btsrc8iii.h>
#include <btsrc16.h>

//
// Global RIPCD Definitions
//
#define RIPCD_MAX_CONNECTIONS 32
#define RIPCD_MAX_LENGTH 256
#define RIPCD_RML_READ_INTERVAL 100
#define RIPCD_TTY_READ_INTERVAL 100
#define RIPCD_USAGE "[-d]\n\nSupplying the '-d' flag will set 'debug' mode, causing ripcd(8) to stay\nin the foreground and print debugging info on standard output.\n" 

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0,const char *name=0);
  ~MainObject();

 public slots:
  void newConnection(int fd);

 private slots:
  void log(RDConfig::LogPriority prio,const QString &msg);
  void sendRml(RDMacro *rml);
  void readRml();
  void gpiChangedData(int matrix,int line,bool state);
  void gpoChangedData(int matrix,int line,bool state);
  void gpiStateData(int matrix,unsigned line,bool state);
  void gpoStateData(int matrix,unsigned line,bool state);
  void ttyTrapData(int cartnum);
  void ttyScanData();
  void databaseBackup();
  void macroTimerData(int num);
  void socketData(int);
  void socketKill(int);
  void checkMaintData();
  
 private:
  void SetUser(QString username);
  void ExecCart(int cartnum);
  void ParseCommand(int);
  void DispatchCommand(int);
  void KillSocket(int);
  void EchoCommand(int,const char *);
  void BroadcastCommand(const char *);
  void EchoArgs(int,const char);
  void ReadRmlSocket(QSocketDevice *dev,RDMacro::Role role,bool echo);
  QString StripPoint(QString);
  void LoadLocalMacros();
  void RunLocalMacros(RDMacro *rml);
  void LoadGpiTable();
  void SendGpi(int ch,int matrix);
  void SendGpo(int ch,int matrix);
  void SendGpiMask(int ch,int matrix);
  void SendGpoMask(int ch,int matrix);
  void SendGpiCart(int ch,int matrix);
  void SendGpoCart(int ch,int matrix);
  void RunSystemMaintRoutine();
  void RunLocalMaintRoutine();
  int GetMaintInterval() const;
  void ForwardConvert(RDMacro *rml) const;
  QSqlDatabase *ripcd_db;
  QString ripcd_host;
  bool debug;
  QServerSocket *server;
  RDSocket *socket[RIPCD_MAX_CONNECTIONS];
  char args[RIPCD_MAX_CONNECTIONS][RD_RML_MAX_ARGS][RD_RML_MAX_LENGTH];
  int istate[RIPCD_MAX_CONNECTIONS];
  int argnum[RIPCD_MAX_CONNECTIONS];
  int argptr[RIPCD_MAX_CONNECTIONS];
  bool auth[RIPCD_MAX_CONNECTIONS];
  QSocketDevice *ripcd_rml_send;
  QSocketDevice *ripcd_rml_echo;
  QSocketDevice *ripcd_rml_noecho;
  QSocketDevice *ripcd_rml_reply;
  QHostAddress ripcd_host_addr;
  RDMatrix::Type ripcd_matrix_type[MAX_MATRICES];
  LocalGpio *ripcd_gpio[MAX_MATRICES];
  Sas32000 *ripcd_sas32000[MAX_MATRICES];
  Sas64000 *ripcd_sas64000[MAX_MATRICES];
  Unity4000 *ripcd_unity4000[MAX_MATRICES];
  Bt10x1 *ripcd_bt10x1[MAX_MATRICES];
  Sas64000Gpi *ripcd_sas64000gpi[MAX_MATRICES];
  BtSs82 *ripcd_btss82[MAX_MATRICES];
  Bt16x1 *ripcd_bt16x1[MAX_MATRICES];
  Bt8x2 *ripcd_bt8x2[MAX_MATRICES];
  BtAcs82 *ripcd_btacs82[MAX_MATRICES];
  SasUsi *ripcd_sasusi[MAX_MATRICES];
  Bt16x2 *ripcd_bt16x2[MAX_MATRICES];
  BtSs124 *ripcd_btss124[MAX_MATRICES];
  LocalAudio *ripcd_local_audio[MAX_MATRICES];
  VGuest *ripcd_vguest[MAX_MATRICES];
  BtSs164 *ripcd_btss164[MAX_MATRICES];
  StarGuide3 *ripcd_starguide3[MAX_MATRICES];
  BtSs42 *ripcd_btss42[MAX_MATRICES];
  LiveWire *ripcd_livewire[MAX_MATRICES];
  Quartz1 *ripcd_quartz1[MAX_MATRICES];
  BtSs44 *ripcd_btss44[MAX_MATRICES];
  BtSrc8Iii *ripcd_btsrc8iii[MAX_MATRICES];
  BtSrc16 *ripcd_btsrc16[MAX_MATRICES];
  bool ripcd_gpi_state[MAX_MATRICES][MAX_GPIO_PINS];
  bool ripcd_gpo_state[MAX_MATRICES][MAX_GPIO_PINS];
  int ripcd_gpi_macro[MAX_MATRICES][MAX_GPIO_PINS][2];
  int ripcd_gpo_macro[MAX_MATRICES][MAX_GPIO_PINS][2];
  bool ripcd_gpi_mask[MAX_MATRICES][MAX_GPIO_PINS];
  bool ripcd_gpo_mask[MAX_MATRICES][MAX_GPIO_PINS];
  int ripcd_gpis[MAX_MATRICES];
  int ripcd_gpos[MAX_MATRICES];
  bool ripcd_tty_inuse[MAX_TTYS];
  int ripcd_switcher_tty[MAX_MATRICES][2];
  RDTTYDevice *ripcd_tty_dev[MAX_TTYS];
  RDTty::Termination ripcd_tty_term[MAX_TTYS];
  RDCodeTrap *ripcd_tty_trap[MAX_TTYS];
  QTimer *ripcd_backup_timer;
  bool ripc_onair_flag;
  QTimer *ripc_macro_timer[RD_MAX_MACRO_TIMERS];
  unsigned ripc_macro_cart[RD_MAX_MACRO_TIMERS];
  QTimer *ripcd_maint_timer;
};


#endif  // RIPCD_H
