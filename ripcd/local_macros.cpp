// local_macros.cpp
//
// Local RML Macros for the Rivendell Interprocess Communication Daemon
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: local_macros.cpp,v 1.49.2.11.2.2 2010/05/08 23:01:57 cvs Exp $
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

#include <stdlib.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdtty.h>
#include <rduser.h>
#include <rddb.h>
#include <ripcd.h>
#include <rdescape_string.h>

void MainObject::gpiChangedData(int matrix,int line,bool state)
{
  if(state) {
    LogLine(RDConfig::LogInfo,QString().sprintf("GPI %d:%d ON",matrix,line+1));
  }
  else {
    LogLine(RDConfig::LogInfo,QString().sprintf("GPI %d:%d OFF",matrix,line+1));
  }
  ripcd_gpi_state[matrix][line]=state;
  BroadcastCommand(QString().sprintf("GI %d %d %d %d!",matrix,line,state,
				     ripcd_gpi_mask[matrix][line]));
  if(!ripcd_gpi_mask[matrix][line]) {
    return;
  }
  if(ripcd_gpi_macro[matrix][line][state]>0) {
    ExecCart(ripcd_gpi_macro[matrix][line][state]);
  }
}


void MainObject::gpoChangedData(int matrix,int line,bool state)
{
  if(state) {
    LogLine(RDConfig::LogInfo,QString().sprintf("GPO %d:%d ON",matrix,line+1));
  }
  else {
    LogLine(RDConfig::LogInfo,QString().sprintf("GPO %d:%d OFF",matrix,line+1));
  }
  ripcd_gpo_state[matrix][line]=state;
  BroadcastCommand(QString().sprintf("GO %d %d %d %d!",matrix,line,state,
				     ripcd_gpo_mask[matrix][line]));
  if(!ripcd_gpo_mask[matrix][line]) {
    return;
  }
  if(ripcd_gpo_macro[matrix][line][state]>0) {
    ExecCart(ripcd_gpo_macro[matrix][line][state]);
  }
}


void MainObject::gpiStateData(int matrix,unsigned line,bool state)
{
  // LogLine(RDConfig::LogWarning,QString().sprintf("gpiStateData(%d,%d,%d)",matrix,line,state));

  BroadcastCommand(QString().sprintf("GI %d %u %d!",matrix,line,state));
}


void MainObject::gpoStateData(int matrix,unsigned line,bool state)
{
  LogLine(RDConfig::LogWarning,QString().sprintf("gpoStateData(%d,%d,%d)",matrix,line,state));

  BroadcastCommand(QString().sprintf("GO %d %u %d!",matrix,line,state));
}


void MainObject::ttyTrapData(int cartnum)
{
  ExecCart(cartnum);
}


void MainObject::ttyScanData()
{
  char buf[256];
  int n;

  for(int i=0;i<MAX_TTYS;i++) {
    if(ripcd_tty_dev[i]!=NULL) {
      while((n=ripcd_tty_dev[i]->readBlock(buf,255))>0) {
	ripcd_tty_trap[i]->scan(buf,n);
      }
    }
  }
}


void MainObject::ExecCart(int cartnum)
{
  RDMacro rml;
  rml.setRole(RDMacro::Cmd);
  rml.setCommand(RDMacro::EX);
  rml.setAddress(rdstation->address());
  rml.setEchoRequested(false);
  rml.setArgQuantity(1);
  rml.setArg(0,cartnum);
  sendRml(&rml);
}


void MainObject::LoadLocalMacros()
{
  QString sql;
  RDSqlQuery *q;
  RDMatrix *matrix;
  unsigned tty_port;

  for(int i=0;i<MAX_MATRICES;i++) {
    ripcd_matrix_type[i]=RDMatrix::None;
    ripcd_sas32000[i]=NULL;
    ripcd_gpis[i]=0;
    ripcd_gpos[i]=0;
    ripcd_switcher_tty[i][0]=-1;
    ripcd_switcher_tty[i][1]=-1;
  }
  for(int i=0;i<MAX_TTYS;i++) {
    ripcd_tty_inuse[i]=false;
    ripcd_tty_dev[i]=NULL;
  }

  //
  // Initialize Matrices
  //
  sql=QString().sprintf("select MATRIX,TYPE,PORT,INPUTS,OUTPUTS from MATRICES \
                         where STATION_NAME=\"%s\"",
			(const char *)rdstation->name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    ripcd_matrix_type[q->value(0).toInt()]=(RDMatrix::Type)q->value(1).toInt();
    matrix=new RDMatrix(rdstation->name(),q->value(0).toInt());
    ripcd_gpis[q->value(0).toInt()]=matrix->gpis();
    ripcd_gpos[q->value(0).toInt()]=matrix->gpos();
    switch(ripcd_matrix_type[q->value(0).toInt(0)]) {
	case RDMatrix::LocalGpio:
	  ripcd_gpio[q->value(0).toInt()]=new LocalGpio(matrix,this);
	  connect(ripcd_gpio[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_gpio[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_gpio[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::Sas32000:
	  ripcd_sas32000[q->value(0).toInt()]=new Sas32000(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][2]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_sas32000[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::Sas64000:
	  ripcd_sas64000[q->value(0).toInt()]=new Sas64000(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_sas64000[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::Unity4000:
	  ripcd_unity4000[q->value(0).toInt()]=new Unity4000(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_unity4000[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::Bt10x1:
	  ripcd_bt10x1[q->value(0).toInt()]=new Bt10x1(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt10x1[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::Sas64000Gpi:
	  ripcd_sas64000gpi[q->value(0).toInt()]=
	    new Sas64000Gpi(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_sas64000gpi[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_sas64000gpi[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::BtSs82:
	  ripcd_btss82[q->value(0).toInt()]=new BtSs82(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss82[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss82[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss82[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::Bt16x1:
	  ripcd_bt16x1[q->value(0).toInt()]=new Bt16x1(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt16x1[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::Bt8x2:
	  ripcd_bt8x2[q->value(0).toInt()]=new Bt8x2(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt8x2[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::BtAcs82:
	  ripcd_btacs82[q->value(0).toInt()]=new BtAcs82(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btacs82[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btacs82[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btacs82[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::SasUsi:
	  ripcd_sasusi[q->value(0).toInt()]=new SasUsi(matrix,this);
	  if(matrix->portType(RDMatrix::Primary)==RDMatrix::TtyPort) {
	    ripcd_switcher_tty[matrix->matrix()][0]=
	      matrix->port(RDMatrix::Primary);
	    ripcd_switcher_tty[matrix->matrix()][1]=-1;
	    ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  }
	  connect(ripcd_sasusi[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_sasusi[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::Bt16x2:
	  ripcd_bt16x2[q->value(0).toInt()]=new Bt16x2(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt16x2[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_bt16x2[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_bt16x2[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::BtSs124:
	  ripcd_btss124[q->value(0).toInt()]=new BtSs124(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss124[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss124[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  break;

	case RDMatrix::LocalAudioAdapter:
	  ripcd_local_audio[q->value(0).toInt()]=new LocalAudio(matrix,this);
	  connect(ripcd_local_audio[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_local_audio[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  break;

	case RDMatrix::LogitekVguest:
	  ripcd_vguest[q->value(0).toInt()]=new VGuest(matrix,this);
	  for(int i=0;i<2;i++) {
	    if(matrix->portType((RDMatrix::Role)i)==RDMatrix::TtyPort) {
	      ripcd_switcher_tty[matrix->matrix()][i]=
		matrix->port((RDMatrix::Role)i);
	      ripcd_tty_inuse[matrix->port((RDMatrix::Role)i)]=true;
	    }
	  }
	  connect(ripcd_vguest[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_vguest[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_vguest[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::BtSs164:
	  ripcd_btss164[q->value(0).toInt()]=new BtSs164(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss164[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss164[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss164[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::StarGuideIII:
	  ripcd_starguide3[q->value(0).toInt()]=new StarGuide3(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_starguide3[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::BtSs42:
	  ripcd_btss42[q->value(0).toInt()]=new BtSs42(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss42[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss42[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss42[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::LiveWire:
	  ripcd_livewire[q->value(0).toInt()]=new LiveWire(matrix,this);
	  connect(ripcd_livewire[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_livewire[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_livewire[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  connect(ripcd_livewire[q->value(0).toInt()],
		  SIGNAL(gpiState(int,unsigned,bool)),
		  this,SLOT(gpiStateData(int,unsigned,bool)));
	  connect(ripcd_livewire[q->value(0).toInt()],
		  SIGNAL(gpoState(int,unsigned,bool)),
		  this,SLOT(gpoStateData(int,unsigned,bool)));
	  break;

	case RDMatrix::Quartz1:
	  ripcd_quartz1[q->value(0).toInt()]=new Quartz1(matrix,this);
	  if(matrix->portType(RDMatrix::Primary)==RDMatrix::TtyPort) {
	    ripcd_switcher_tty[matrix->matrix()][0]=
	      matrix->port(RDMatrix::Primary);
	    ripcd_switcher_tty[matrix->matrix()][1]=-1;
	    ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  }
	  connect(ripcd_quartz1[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;

	case RDMatrix::BtSs44:
	  ripcd_btss44[q->value(0).toInt()]=new BtSs44(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss44[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss44[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss44[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::BtSrc8III:
	  ripcd_btsrc8iii[q->value(0).toInt()]=new BtSrc8Iii(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btsrc8iii[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btsrc8iii[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btsrc8iii[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	case RDMatrix::BtSrc16:
	  ripcd_btsrc16[q->value(0).toInt()]=new BtSrc16(matrix,this);
	  ripcd_switcher_tty[matrix->matrix()][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix->matrix()][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btsrc16[q->value(0).toInt()],
		  SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btsrc16[q->value(0).toInt()],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btsrc16[q->value(0).toInt()],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;

	default:
	  break;
    }
    delete matrix;
  }
  delete q;

  //
  // Initialize TTYs
  //
  sql=QString().sprintf("select PORT_ID,PORT,BAUD_RATE,DATA_BITS,PARITY,\
                         TERMINATION from TTYS where (STATION_NAME=\"%s\")&&\
                         (ACTIVE=\"Y\")",
			(const char *)rdstation->name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    tty_port=q->value(0).toUInt();
    if(!ripcd_tty_inuse[tty_port]) {
      ripcd_tty_dev[tty_port]=new RDTTYDevice();
      ripcd_tty_dev[tty_port]->setName(q->value(1).toString());
      ripcd_tty_dev[tty_port]->setSpeed(q->value(2).toInt());
      ripcd_tty_dev[tty_port]->setWordLength(q->value(3).toInt());
      ripcd_tty_dev[tty_port]->
	setParity((RDTTYDevice::Parity)q->value(4).toInt());
      if(ripcd_tty_dev[tty_port]->open(IO_ReadWrite)) {
	ripcd_tty_term[tty_port]=(RDTty::Termination)q->value(5).toInt();
	ripcd_tty_inuse[tty_port]=true;
	ripcd_tty_trap[tty_port]=new RDCodeTrap(this);
	connect(ripcd_tty_trap[tty_port],SIGNAL(trapped(int)),
		this,SLOT(ttyTrapData(int)));
      }
      else {
	delete ripcd_tty_dev[tty_port];
	ripcd_tty_dev[tty_port]=NULL;
      }
    }
  }
  delete q;
  QTimer *timer=new QTimer(this,"tty_scan_timer");
  connect(timer,SIGNAL(timeout()),this,SLOT(ttyScanData()));
  timer->start(RIPCD_TTY_READ_INTERVAL);
}


void MainObject::RunLocalMacros(RDMacro *rml)
{
  int matrix_num;
  int gpi;
  int tty_port;
  int tty_port2;
  int severity=0;
  QString str;
  QString sql;
  QString cmd;
  RDSqlQuery *q;
  RDMatrix *matrix;
  QHostAddress addr;
  RDUser *rduser;
  char logstr[RD_RML_MAX_LENGTH];
  char bin_buf[RD_RML_MAX_LENGTH];
  int d;
  RDMatrix::GpioType gpio_type;

  rml->generateString(logstr,RD_RML_MAX_LENGTH-1);
  LogLine(RDConfig::LogInfo,QString().sprintf("received rml: \'%s\' from %s",
	  (const char *)logstr,(const char *)rml->address().toString()));

  ForwardConvert(rml);

  switch(rml->command()) {
    case RDMacro::BO:
      tty_port=rml->arg(0).toInt();
      if((tty_port<0)||(tty_port>MAX_TTYS)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	  return;
	}
      }
      if(ripcd_tty_dev[tty_port]==NULL) {
	rml->acknowledge(false);
	sendRml(rml);
	return;
      }
      for(int i=1;i<(rml->argQuantity());i++) {
	sscanf((const char *)rml->arg(i).toString(),"%x",&d);
	bin_buf[i-1]=0xFF&d;
      }
      ripcd_tty_dev[tty_port]->writeBlock(bin_buf,rml->argQuantity()-1);
      rml->acknowledge(true);
      sendRml(rml);
      return;
      break;
      
    case RDMacro::DB:
      if(rml->argQuantity()!=1) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if(fork()==0) {
	cmd=QString().sprintf("mysqldump -c Rivendell -h %s -u %s -p%s > %s",
			      (const char *)ripcd_config->mysqlHostname(),
			      (const char *)ripcd_config->mysqlUsername(),
			      (const char *)ripcd_config->mysqlPassword(),
			      (const char *)rml->arg(0).toString());
	system((const char *)cmd);
	exit(0);
      }
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;
      
    case RDMacro::GI:
      if(rml->argQuantity()!=5) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      matrix_num=rml->arg(0).toInt();
      if(rml->arg(1).toString().lower()=="i") {
	gpio_type=RDMatrix::GpioInput;
      }
      else {
	if(rml->arg(1).toString().lower()=="o") {
	  gpio_type=RDMatrix::GpioOutput;
	}
	else {
	  if(rml->echoRequested()) {
	    rml->acknowledge(false);
	    sendRml(rml);
	  }
	  return;
	}
      }
      gpi=rml->arg(2).toInt()-1;
      if((ripcd_matrix_type[matrix_num]==RDMatrix::None)||
	 (gpi>(MAX_GPIO_PINS-1))||
	 (gpi<0)||
	 (rml->arg(3).toInt()<0)||(rml->arg(3).toInt()>1)||
	 (rml->arg(4).toInt()<-1)||(rml->arg(4).toInt()>999999)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      switch(gpio_type) {
	case RDMatrix::GpioInput:
	  ripcd_gpi_macro[matrix_num][gpi][rml->arg(3).toInt()]=
	    rml->arg(4).toInt();
	  BroadcastCommand(QString().sprintf("GC %d %d %d %d!",matrix_num,gpi,
				    ripcd_gpi_macro[matrix_num][gpi][0],
				    ripcd_gpi_macro[matrix_num][gpi][1]));

	  LogLine(RDConfig::LogWarning,QString().sprintf("cart: %u",				ripcd_gpi_macro[matrix_num][gpi][rml->arg(3).toInt()]));


	  break;

	case RDMatrix::GpioOutput:
	  ripcd_gpo_macro[matrix_num][gpi][rml->arg(3).toInt()]=
	    rml->arg(4).toInt();
	  BroadcastCommand(QString().sprintf("GD %d %d %d %d!",matrix_num,gpi,
				    ripcd_gpo_macro[matrix_num][gpi][0],
				    ripcd_gpo_macro[matrix_num][gpi][1]));
	  break;
      }
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;
      
    case RDMacro::GE:
      if(rml->argQuantity()!=4) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      matrix_num=rml->arg(0).toInt();
      if(rml->arg(1).toString().lower()=="i") {
	gpio_type=RDMatrix::GpioInput;
      }
      else {
	if(rml->arg(1).toString().lower()=="o") {
	  gpio_type=RDMatrix::GpioOutput;
	}
	else {
	  if(rml->echoRequested()) {
	    rml->acknowledge(false);
	    sendRml(rml);
	  }
	  return;
	}
      }
      gpi=rml->arg(2).toInt()-1;
      if((ripcd_matrix_type[matrix_num]==RDMatrix::None)||
	 (gpi>(MAX_GPIO_PINS-1))||
	 (gpi<0)||
	 (rml->arg(3).toInt()<0)||(rml->arg(3).toInt()>1)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      switch(gpio_type) {
	case RDMatrix::GpioInput:
	  if(rml->arg(3).toInt()==1) {
	    ripcd_gpi_mask[matrix_num][gpi]=true;
	    BroadcastCommand(QString().sprintf("GM %d %d 1!",matrix_num,gpi));
	  }
	  else {
	    ripcd_gpi_mask[matrix_num][gpi]=false;
	    BroadcastCommand(QString().sprintf("GM %d %d 0!",matrix_num,gpi));
	  }
	  break;

	case RDMatrix::GpioOutput:
	  if(rml->arg(3).toInt()==1) {
	    ripcd_gpo_mask[matrix_num][gpi]=true;
	    BroadcastCommand(QString().sprintf("GN %d %d 1!",matrix_num,gpi));
	  }
	  else {
	    ripcd_gpo_mask[matrix_num][gpi]=false;
	    BroadcastCommand(QString().sprintf("GN %d %d 0!",matrix_num,gpi));
	  }
	  break;
      }
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;
      
    case RDMacro::LO:
      if(rml->argQuantity()>2) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if(rml->argQuantity()==0) {
	rduser=new RDUser(rdstation->defaultName());
      }
      else {
	rduser=new RDUser(rml->arg(0).toString());
	if(!rduser->exists()) {
	  if(rml->echoRequested()) {
	    rml->acknowledge(false);
	    sendRml(rml);
	  }
	  delete rduser;
	  return;
	}
	if(!rduser->checkPassword(rml->arg(1).toString(),false)) {
	  if(rml->echoRequested()) {
	    rml->acknowledge(false);
	    sendRml(rml);
	  }
	  delete rduser;
	  return;
	}
      }
      SetUser(rduser->name());
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      delete rduser;
      break;
      
    case RDMacro::MB:
      if(rml->argQuantity()<3) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      severity=rml->arg(1).toInt();
      if((severity<1)||(severity>3)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if(fork()==0) {
	if(getuid()==0) {
	  if(setegid(ripcd_config->gid())<0) {
	    LogLine(RDConfig::LogWarning,QString().
		    sprintf("unable to set group id %d for RDPopup",
				      ripcd_config->gid()));
	    if(rml->echoRequested()) {
	      rml->acknowledge(false);
	      sendRml(rml);
	    }
	  }
	  if(seteuid(ripcd_config->uid())<0) {
	    LogLine(RDConfig::LogWarning,QString().
		    sprintf("unable to set user id %d for RDPopup",
				      ripcd_config->uid()));
	    if(rml->echoRequested()) {
	      rml->acknowledge(false);
	      sendRml(rml);
	    }
	  }
	}
	if(system(QString().
		  sprintf("rdpopup -display %s %s %s",
			(const char *)rml->arg(0).toString(),
			(const char *)rml->arg(1).toString(),
			(const char *)RDEscapeString(rml->rollupArgs(2))))<0) {
	  LogLine(RDConfig::LogWarning,"RDPopup returned an error");
	}
	exit(0);
      }
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;

    case RDMacro::MT:
      if(rml->argQuantity()!=3) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if((rml->arg(0).toUInt()==0)||
	 (rml->arg(0).toUInt()>RD_MAX_MACRO_TIMERS)||
	 (rml->arg(2).toUInt()>999999)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if((rml->arg(1).toUInt()==0)||
	 (rml->arg(2).toUInt()==0)) {
	ripc_macro_cart[rml->arg(0).toUInt()-1]=0;
	ripc_macro_timer[rml->arg(0).toUInt()-1]->stop();
	if(rml->echoRequested()) {
	  rml->acknowledge(true);
	  sendRml(rml);
	}
	return;
      }
      ripc_macro_cart[rml->arg(0).toUInt()-1]=rml->arg(2).toUInt();
      ripc_macro_timer[rml->arg(0).toUInt()-1]->stop();
      ripc_macro_timer[rml->arg(0).toUInt()-1]->
	start(rml->arg(1).toInt(),true);
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      return;
      
    case RDMacro::RN:
      if(rml->argQuantity()<1) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if(fork()==0) {
	QString cmd=rml->arg(0).toString();
	for(int i=1;i<=rml->argQuantity();i++) {
	  cmd+=" "+rml->arg(i).toString();
	}
	if(getuid()==0) {
	  if(setgid(ripcd_config->gid())<0) {
	    LogLine(RDConfig::LogWarning,QString().
		    sprintf("unable to set group id %d for RN",
				      ripcd_config->gid()));
	    if(rml->echoRequested()) {
	      rml->acknowledge(false);
	      sendRml(rml);
	    }
	  }
	  if(setuid(ripcd_config->uid())<0) {
	    LogLine(RDConfig::LogWarning,QString().
		    sprintf("unable to set user id %d for RN",
				      ripcd_config->uid()));
	    if(rml->echoRequested()) {
	      rml->acknowledge(false);
	      sendRml(rml);
	    }
	  }
	}
	system((const char *)cmd);
	exit(0);
      }
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;
      
    case RDMacro::SI:
      tty_port=rml->arg(0).toInt();
      if((tty_port<0)||(tty_port>MAX_TTYS)||(rml->argQuantity()!=3)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if(ripcd_tty_dev[tty_port]==NULL) {
	rml->acknowledge(false);
	sendRml(rml);
	return;
      }
      for(int i=2;i<(rml->argQuantity()-1);i++) {
	str+=(rml->arg(i).toString()+" ");
      }
      str+=rml->arg(rml->argQuantity()-1).toString();
      ripcd_tty_trap[tty_port]->addTrap(rml->arg(1).toInt(),
					str,str.length());
      rml->acknowledge(true);
      sendRml(rml);
      return;
      break;
      
    case RDMacro::SC:
      tty_port=rml->arg(0).toInt();
      if((tty_port<0)||(tty_port>MAX_TTYS)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	  return;
	}
      }
      if(ripcd_tty_dev[tty_port]==NULL) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      switch(rml->argQuantity()) {
	case 1:
	  ripcd_tty_trap[tty_port]->clear();
	  if(rml->echoRequested()) {
	    rml->acknowledge(true);
	    sendRml(rml);
	  }
	  break;
	  
	case 2:
	  ripcd_tty_trap[tty_port]->removeTrap(rml->arg(1).toInt());
	  if(rml->echoRequested()) {
	    rml->acknowledge(true);
	    sendRml(rml);
	  }
	  break;
	  
	case 3:
	  ripcd_tty_trap[tty_port]->removeTrap(rml->arg(1).toInt(),
					       (const char *)rml->arg(2).toString(),
					       rml->arg(2).toString().length());
	  if(rml->echoRequested()) {
	    rml->acknowledge(true);
	    sendRml(rml);
	  }
	  break;
	  
	default:
	  if(rml->echoRequested()) {
	    rml->acknowledge(false);
	    sendRml(rml);
	  }
	  return;
	  break;
      }
      break;
      
    case RDMacro::SO:
      tty_port=rml->arg(0).toInt();
      if((tty_port<0)||(tty_port>MAX_TTYS)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	  return;
	}
      }
      if(ripcd_tty_dev[tty_port]==NULL) {
	  rml->acknowledge(false);
	  sendRml(rml);
	  return;
      }
      for(int i=1;i<(rml->argQuantity()-1);i++) {
	str+=(rml->arg(i).toString()+" ");
      }
      str+=rml->arg(rml->argQuantity()-1).toString();
      switch(ripcd_tty_term[tty_port]) {
	case RDTty::CrTerm:
	  str+=QString().sprintf("\x0d");
	  break;
	  
	case RDTty::LfTerm:
	  str+=QString().sprintf("\x0a");
	  break;
	  
	case RDTty::CrLfTerm:
	  str+=QString().sprintf("\x0d\x0a");
	  break;
	  
	default:
	  break;
      }
      ripcd_tty_dev[tty_port]->writeBlock((const char *)str,str.length());
      rml->acknowledge(true);
      sendRml(rml);
      return;
      break;
      
    case RDMacro::CL:
    case RDMacro::FS:
    case RDMacro::GO:
    case RDMacro::ST:
    case RDMacro::SA:
    case RDMacro::SD:
    case RDMacro::SG:
    case RDMacro::SR:
    case RDMacro::SL:
    case RDMacro::SX:
      if((rml->arg(0).toInt()<0)||(rml->arg(0).toInt()>=MAX_MATRICES)) {
	if(!rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      switch(ripcd_matrix_type[rml->arg(0).toInt()]) {
	case RDMatrix::LocalGpio:
	  ripcd_gpio[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Sas32000:
	  ripcd_sas32000[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Sas64000:
	  ripcd_sas64000[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Unity4000:
	  ripcd_unity4000[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Bt10x1:
	  ripcd_bt10x1[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Sas64000Gpi:
	  ripcd_sas64000gpi[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSs82:
	  ripcd_btss82[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Bt16x1:
	  ripcd_bt16x1[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Bt8x2:
	  ripcd_bt8x2[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtAcs82:
	  ripcd_btacs82[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::SasUsi:
	  ripcd_sasusi[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Bt16x2:
	  ripcd_bt16x2[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSs124:
	  ripcd_btss124[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::LocalAudioAdapter:
	  ripcd_local_audio[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::LogitekVguest:
	  ripcd_vguest[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSs164:
	  ripcd_btss164[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::StarGuideIII:
	  ripcd_starguide3[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSs42:
	  ripcd_btss42[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::LiveWire:
	  ripcd_livewire[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::Quartz1:
	  ripcd_quartz1[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSs44:
	  ripcd_btss44[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSrc8III:
	  ripcd_btsrc8iii[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	case RDMatrix::BtSrc16:
	  ripcd_btsrc16[rml->arg(0).toInt()]->processCommand(rml);
	  break;
	  
	default:
	  if(rml->echoRequested()) {
	    rml->acknowledge(false);
	    sendRml(rml);
	  }
	  break;
      }
      break;
      
    case RDMacro::SY:
      if(rml->argQuantity()!=1) {
	return;
      }
      tty_port=rml->arg(0).toInt();
      if((tty_port<0)||(tty_port>=MAX_TTYS)) {
	return;
      }
      
      //
      // Shutdown TTY Port
      //
      if(ripcd_tty_dev[tty_port]!=NULL) {
	ripcd_tty_dev[tty_port]->close();
	delete ripcd_tty_dev[tty_port];
	ripcd_tty_dev[tty_port]=NULL;
	ripcd_tty_inuse[tty_port]=false;
	delete ripcd_tty_trap[tty_port];
	ripcd_tty_trap[tty_port]=NULL;
      }
      
      //
      // Try to Restart
      //
      sql=QString().sprintf("select PORT_ID,PORT,BAUD_RATE,DATA_BITS,PARITY,\
                         TERMINATION from TTYS where (STATION_NAME=\"%s\")&&\
                         (ACTIVE=\"Y\")&&(PORT_ID=%d)",
			    (const char *)rdstation->name(),tty_port);
      q=new RDSqlQuery(sql);
      if(q->first()) {
	if(!ripcd_tty_inuse[tty_port]) {
	  ripcd_tty_dev[tty_port]=new RDTTYDevice();
	  ripcd_tty_dev[tty_port]->setName(q->value(1).toString());
	  ripcd_tty_dev[tty_port]->setSpeed(q->value(2).toInt());
	  ripcd_tty_dev[tty_port]->setWordLength(q->value(3).toInt());
	  ripcd_tty_dev[tty_port]->
	    setParity((RDTTYDevice::Parity)q->value(4).toInt());
	  if(ripcd_tty_dev[tty_port]->open(IO_ReadWrite)) {
	    ripcd_tty_term[tty_port]=(RDTty::Termination)q->value(5).toInt();
	    ripcd_tty_inuse[tty_port]=true;
	    ripcd_tty_trap[tty_port]=new RDCodeTrap(this);
	    connect(ripcd_tty_trap[tty_port],SIGNAL(trapped(int)),
		    this,SLOT(ttyTrapData(int)));
	  }
	  else {
	    delete ripcd_tty_dev[tty_port];
	    ripcd_tty_dev[tty_port]=NULL;
	  }
	}
      }
      delete q;
      break;

    case RDMacro::SZ:
      if(rml->argQuantity()!=1) {
	return;
      }
      matrix_num=rml->arg(0).toInt();
      if((matrix_num<0)||(matrix_num>=MAX_MATRICES)) {
	return;
      }
      
      //
      // Shutdown the old switcher
      //
      ripcd_gpis[matrix_num]=0;
      for(int i=0;i<2;i++) {
	if(ripcd_switcher_tty[matrix_num][i]>-1) {
	  ripcd_tty_inuse[ripcd_switcher_tty[matrix_num][i]]=false;
	  ripcd_switcher_tty[matrix_num][i]=-1;
	}
      }
      switch(ripcd_matrix_type[matrix_num]) {
	case RDMatrix::LocalGpio:
	  delete ripcd_gpio[matrix_num];
	  ripcd_gpio[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Sas32000:
	  delete ripcd_sas32000[matrix_num];
	  ripcd_sas32000[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Sas64000:
	  delete ripcd_sas64000[matrix_num];
	  ripcd_sas64000[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Unity4000:
	  delete ripcd_unity4000[matrix_num];
	  ripcd_unity4000[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Bt10x1:
	  delete ripcd_bt10x1[matrix_num];
	  ripcd_bt10x1[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Sas64000Gpi:
	  delete ripcd_sas64000gpi[matrix_num];
	  ripcd_sas64000gpi[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSs82:
	  delete ripcd_btss82[matrix_num];
	  ripcd_btss82[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Bt16x1:
	  delete ripcd_bt16x1[matrix_num];
	  ripcd_bt16x1[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Bt8x2:
	  delete ripcd_bt8x2[matrix_num];
	  ripcd_bt8x2[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtAcs82:
	  delete ripcd_btacs82[matrix_num];
	  ripcd_btacs82[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::SasUsi:
	  delete ripcd_sasusi[matrix_num];
	  ripcd_sasusi[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Bt16x2:
	  delete ripcd_bt16x2[matrix_num];
	  ripcd_bt16x2[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSs124:
	  delete ripcd_btss124[matrix_num];
	  ripcd_btss124[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::LocalAudioAdapter:
	  delete ripcd_local_audio[matrix_num];
	  ripcd_local_audio[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::LogitekVguest:
	  delete ripcd_vguest[matrix_num];
	  ripcd_vguest[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSs164:
	  delete ripcd_btss164[matrix_num];
	  ripcd_btss164[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::StarGuideIII:
	  delete ripcd_starguide3[matrix_num];
	  ripcd_starguide3[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSs42:
	  delete ripcd_btss42[matrix_num];
	  ripcd_btss42[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::LiveWire:
	  delete ripcd_livewire[matrix_num];
	  ripcd_livewire[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::Quartz1:
	  delete ripcd_quartz1[matrix_num];
	  ripcd_quartz1[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSs44:
	  delete ripcd_btss44[matrix_num];
	  ripcd_btss44[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSrc8III:
	  delete ripcd_btsrc8iii[matrix_num];
	  ripcd_btsrc8iii[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	case RDMatrix::BtSrc16:
	  delete ripcd_btsrc16[matrix_num];
	  ripcd_btsrc16[matrix_num]=NULL;
	  ripcd_matrix_type[matrix_num]=RDMatrix::None;
	  break;
	  
	default:
	  break;
      }
      
      //
      // Startup the new
      //
      matrix=new RDMatrix(rdstation->name(),matrix_num);
      ripcd_gpis[matrix_num]=matrix->gpis();
      ripcd_gpos[matrix_num]=matrix->gpos();
      tty_port=matrix->port(RDMatrix::Primary);
      if((tty_port=matrix->port(RDMatrix::Primary))>-1) {
	ripcd_tty_inuse[tty_port]=true;
	ripcd_switcher_tty[matrix->matrix()][0]=tty_port;
      }
      tty_port2=matrix->port(RDMatrix::Backup);
      if((tty_port2=matrix->port(RDMatrix::Backup))>-1) {
	ripcd_tty_inuse[tty_port2]=true;
	ripcd_switcher_tty[matrix->matrix()][1]=tty_port2;
      }
      
      switch(matrix->type()) {
	case RDMatrix::LocalGpio:
	  ripcd_gpio[matrix_num]=new LocalGpio(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::LocalGpio;
	  connect(ripcd_gpio[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_gpio[matrix_num],SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_gpio[matrix_num],SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	case RDMatrix::Sas32000:
	  ripcd_sas32000[matrix_num]=new Sas32000(matrix,this);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Sas32000;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_sas32000[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::Sas64000:
	  ripcd_sas64000[matrix_num]=new Sas64000(matrix,this);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Sas64000;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_sas64000[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::Unity4000:
	  ripcd_unity4000[matrix_num]=new Unity4000(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Unity4000;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_unity4000[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::Bt10x1:
	  ripcd_bt10x1[matrix_num]=new Bt10x1(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Bt10x1;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt10x1[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::Sas64000Gpi:
	  ripcd_sas64000gpi[matrix_num]=new Sas64000Gpi(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Sas64000Gpi;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_sas64000gpi[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_sas64000gpi[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::BtSs82:
	  ripcd_btss82[matrix_num]=new BtSs82(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSs82;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss82[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss82[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss82[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::Bt16x1:
	  ripcd_bt16x1[matrix_num]=new Bt16x1(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Bt16x1;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt16x1[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::Bt8x2:
	  ripcd_bt8x2[matrix_num]=new Bt8x2(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Bt8x2;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt8x2[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::BtAcs82:
	  ripcd_btacs82[matrix_num]=new BtAcs82(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtAcs82;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btacs82[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btacs82[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btacs82[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::SasUsi:
	  ripcd_sasusi[matrix_num]=new SasUsi(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::SasUsi;
	  if(matrix->portType(RDMatrix::Primary)==RDMatrix::TtyPort) {
	    ripcd_switcher_tty[matrix_num][0]=
	      matrix->port(RDMatrix::Primary);
	    ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  }
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  connect(ripcd_sasusi[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_sasusi[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::Bt16x2:
	  ripcd_bt16x2[matrix_num]=new Bt16x2(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Bt16x2;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_bt16x2[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_bt16x2[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_bt16x2[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::BtSs124:
	  ripcd_btss124[matrix_num]=new BtSs124(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSs124;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss124[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss124[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::LocalAudioAdapter:
	  ripcd_local_audio[matrix_num]=new LocalAudio(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::LocalAudioAdapter;
	  connect(ripcd_local_audio[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_local_audio[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::LogitekVguest:
	  ripcd_vguest[matrix_num]=new VGuest(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::LogitekVguest;
	  for(int i=0;i<2;i++) {
	    if(matrix->portType((RDMatrix::Role)i)==RDMatrix::TtyPort) {
	      ripcd_switcher_tty[matrix_num][i]=
		matrix->port((RDMatrix::Role)i);
	      ripcd_tty_inuse[matrix->port((RDMatrix::Role)i)]=true;
	    }
	  }
	  connect(ripcd_vguest[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_vguest[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_vguest[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::BtSs164:
	  ripcd_btss164[matrix_num]=new BtSs164(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSs164;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss164[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss164[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss164[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::StarGuideIII:
	  ripcd_starguide3[matrix_num]=new StarGuide3(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::StarGuideIII;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_starguide3[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::BtSs42:
	  ripcd_btss42[matrix_num]=new BtSs42(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSs42;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss42[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss42[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss42[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::LiveWire:
	  ripcd_livewire[matrix_num]=new LiveWire(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::LiveWire;
	  connect(ripcd_livewire[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_livewire[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_livewire[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  connect(ripcd_livewire[matrix_num],
		  SIGNAL(gpiState(int,unsigned,bool)),
		  this,SLOT(gpiStateData(int,unsigned,bool)));
	  connect(ripcd_livewire[matrix_num],
		  SIGNAL(gpoState(int,unsigned,bool)),
		  this,SLOT(gpoStateData(int,unsigned,bool)));
	  break;
	  
	case RDMatrix::Quartz1:
	  ripcd_quartz1[matrix_num]=new Quartz1(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::Quartz1;
	  if(matrix->portType(RDMatrix::Primary)==RDMatrix::TtyPort) {
	    ripcd_switcher_tty[matrix_num][0]=
	      matrix->port(RDMatrix::Primary);
	    ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  }
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  connect(ripcd_quartz1[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  break;
	  
	case RDMatrix::BtSs44:
	  ripcd_btss44[matrix_num]=new BtSs44(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSs44;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btss44[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btss44[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btss44[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::BtSrc8III:
	  ripcd_btsrc8iii[matrix_num]=new BtSrc8Iii(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSrc8III;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btsrc8iii[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btsrc8iii[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btsrc8iii[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	case RDMatrix::BtSrc16:
	  ripcd_btsrc16[matrix_num]=new BtSrc16(matrix);
	  ripcd_matrix_type[matrix_num]=RDMatrix::BtSrc16;
	  ripcd_switcher_tty[matrix_num][0]=
	    matrix->port(RDMatrix::Primary);
	  ripcd_switcher_tty[matrix_num][1]=-1;
	  ripcd_tty_inuse[matrix->port(RDMatrix::Primary)]=true;
	  connect(ripcd_btsrc16[matrix_num],SIGNAL(rmlEcho(RDMacro *)),
		  this,SLOT(sendRml(RDMacro *)));
	  connect(ripcd_btsrc16[matrix_num],
		  SIGNAL(gpiChanged(int,int,bool)),
		  this,SLOT(gpiChangedData(int,int,bool)));
	  connect(ripcd_btsrc16[matrix_num],
		  SIGNAL(gpoChanged(int,int,bool)),
		  this,SLOT(gpoChangedData(int,int,bool)));
	  break;
	  
	default:
	  break;
      }
      delete matrix;
      break;

    case RDMacro::TA:
      if((rml->argQuantity()!=1)||
	 (rml->arg(0).toInt()<0)||(rml->arg(0).toInt()>1)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if((rml->arg(0).toInt()==0)&&ripc_onair_flag) {
	BroadcastCommand("TA 0!");
	LogLine(RDConfig::LogInfo,"onair flag OFF");
      }
      if((rml->arg(0).toInt()==1)&&(!ripc_onair_flag)) {
	BroadcastCommand("TA 1!");
	LogLine(RDConfig::LogInfo,"onair flag ON");
      }
      ripc_onair_flag=rml->arg(0).toInt();
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;

    case RDMacro::UO:
      if(rml->argQuantity()<3) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if(!addr.setAddress(rml->arg(0).toString())) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      if((rml->arg(1).toInt()<0)||(rml->arg(1).toInt()>0xFFFF)) {
	if(rml->echoRequested()) {
	  rml->acknowledge(false);
	  sendRml(rml);
	}
	return;
      }
      for(int i=2;i<(rml->argQuantity()-1);i++) {
	str+=(rml->arg(i).toString()+" ");
      }
      str+=rml->arg(rml->argQuantity()-1).toString();
      LogLine(RDConfig::LogDebug,QString().
	      sprintf("Sending \"%s\" to %s:%d",(const char *)str,
		      (const char *)addr.toString(),rml->arg(1).toInt()));
      ripcd_rml_send->writeBlock((const char *)str,str.length(),addr,
				 (Q_UINT16)(rml->arg(1).toInt()));
      if(rml->echoRequested()) {
	rml->acknowledge(true);
	sendRml(rml);
      }
      break;

      default:
//	LogLine(RDConfig::LogDebug,QString().sprintf("unhandled rml: \'%s\' from %s",
//	       (const char *)logstr,(const char *)rml->address().toString()));
	break;
  }
}


void MainObject::ForwardConvert(RDMacro *rml) const
{
  //
  // Convert old RML syntax to current forms
  //
  switch(rml->command()) {
    case RDMacro::GE:
      if(rml->argQuantity()==3) {
	rml->setArgQuantity(4);
	for(int i=2;i>=1;i--) {
	  rml->setArg(i+1,rml->arg(i));
	}
	rml->setArg(1,"I");
      }
      break;

    case RDMacro::GI:
      if(rml->argQuantity()==3) {
	rml->setArgQuantity(4);
	rml->setArg(3,0);
      }
      if(rml->argQuantity()==4) {
	rml->setArgQuantity(5);
	for(int i=3;i>=1;i--) {
	  rml->setArg(i+1,rml->arg(i));
	}
	rml->setArg(1,"I");
      }
      break;

    case RDMacro::GO:
      if(rml->argQuantity()==4) {
	rml->setArgQuantity(5);
	for(int i=3;i>=1;i--) {
	  rml->setArg(i+1,rml->arg(i));
	}
	rml->setArg(1,"O");
      }
      break;

    default:
      break;
  }
}
