// rdpanel.cpp
//
// A Dedicated Cart Wall Utility for Rivendell.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdpanel.cpp,v 1.19.2.1 2009/03/26 23:14:04 cvs Exp $
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
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <qmessagebox.h>
#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qpainter.h>

#include <rdpanel.h>
#include <rd.h>
#include <rdcheck_daemons.h>
#include <rddbheartbeat.h>

#include <globals.h>

//
// Global Resources
//
RDStation *rdstation_conf;
RDAirPlayConf *rdairplay_conf;
RDAudioPort *rdaudioport_conf;
RDUser *rduser;
RDRipc *rdripc;
#include <rdcmd_switch.h>
RDCartDialog *panel_cart_dialog;

//
// Icons
//
#include "../icons/rivendell-22x22.xpm"


void SigHandler(int signo)
{
  pid_t pLocalPid;

  switch(signo) {
  case SIGCHLD:
    pLocalPid=waitpid(-1,NULL,WNOHANG);
    while(pLocalPid>0) {
      pLocalPid=waitpid(-1,NULL,WNOHANG);
    }
    signal(SIGCHLD,SigHandler);
    return;
  }
}


MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  QPixmap *pm;
  QPainter *pd;
  QPixmap *mainmap;

  //
  // Fix the Window Size
  //
#ifndef RESIZABLE
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
#endif  // RESIZABLE

  //
  // Load the command-line arguments
  //
  RDCmdSwitch *cmd=new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdpanel",
				   RDPANEL_USAGE);

  //
  // Generate Fonts
  //
  QFont button_font=QFont("Helvetica",16,QFont::Bold);
  button_font.setPixelSize(16);

  //
  // Create Icons
  //
  lib_rivendell_map=new QPixmap(rivendell_xpm);
  setIcon(*lib_rivendell_map);

  //
  // Ensure that system daemons are running
  //
  RDInitializeDaemons();

  //
  // Load Local Configs
  //
  panel_config=new RDConfig();
  panel_config->load();

  //
  // Open Database
  //
  panel_db=QSqlDatabase::addDatabase("QMYSQL3");
  if(!panel_db) {
    QMessageBox::warning(this,
	   "Can't Connect","Unable to connect to mySQL Server!",0,1,1);
    exit(0);
  }
  panel_db->setDatabaseName(panel_config->mysqlDbname());
  panel_db->setUserName(panel_config->mysqlUsername());
  panel_db->setPassword(panel_config->mysqlPassword());
  panel_db->setHostName(panel_config->mysqlHostname());
  if(!panel_db->open()) {
    QMessageBox::warning(this,
			 "Can't Connect","Unable to connect to mySQL Server!");
    panel_db->removeDatabase(panel_config->mysqlDbname());
    exit(0);
  }
  new RDDbHeartbeat(panel_config->mysqlHeartbeatInterval(),this);

  //
  // Master Clock Timer
  //
  panel_master_timer=new QTimer(this,"panel_master_timer");
  connect(panel_master_timer,SIGNAL(timeout()),this,SLOT(masterTimerData()));
  panel_master_timer->start(MASTER_TIMER_INTERVAL);

  //
  // CAE Connection
  //
  panel_cae=new RDCae(parent,name);
  panel_cae->connectHost("localhost",CAED_TCP_PORT,panel_config->password());

  //
  // Allocate Global Resources
  //
  rdstation_conf=new RDStation(panel_config->stationName());
  rdairplay_conf=new RDAirPlayConf(panel_config->stationName(),0,"RDPANEL");
  panel_skin_pixmap=new QPixmap(rdairplay_conf->skinPath());
  if(panel_skin_pixmap->isNull()||(panel_skin_pixmap->width()<1024)||
     (panel_skin_pixmap->height()<738)) {
    delete panel_skin_pixmap;
    panel_skin_pixmap=NULL;
  }
  else {
    setErasePixmap(*panel_skin_pixmap);
  }

  //
  // RIPC Connection
  //
  rdripc=new RDRipc(panel_config->stationName());
  connect(rdripc,SIGNAL(userChanged()),this,SLOT(userData()));
  connect(rdripc,SIGNAL(rmlReceived(RDMacro *)),
	  this,SLOT(rmlReceivedData(RDMacro *)));
//  rdripc->connectHost("localhost",RIPCD_TCP_PORT,panel_config->password());

  //
  // User
  //
  rduser=NULL;

  //
  // Meter Timer
  //
  QTimer *timer=new QTimer(this,"meter_timer");
  connect(timer,SIGNAL(timeout()),this,SLOT(meterData()));
  timer->start(METER_INTERVAL);

  //
  // Macro Player
  //
  panel_player=new RDEventPlayer(rdripc,this);

  //
  // Cart Picker
  //
  panel_cart_dialog=new RDCartDialog(&panel_filter,&panel_group,
				     rdairplay_conf->card(3),
				     rdairplay_conf->port(3),
				     0,0,panel_cae,rdripc,rdstation_conf,
				     rdstation_conf->editorPath(),
				     this,"panel_cart_dialog");

  //
  // Sound Panel Array
  //
  if (rdairplay_conf->panels(RDAirPlayConf::StationPanel) || 
      rdairplay_conf->panels(RDAirPlayConf::UserPanel)){
    int card=-1;
    panel_panel=
      new RDSoundPanel(RDPANEL_PANEL_BUTTON_COLUMNS,RDPANEL_PANEL_BUTTON_ROWS,
		       rdairplay_conf->panels(RDAirPlayConf::StationPanel),
		       rdairplay_conf->panels(RDAirPlayConf::UserPanel),
		       rdairplay_conf->flashPanel(),
		       rdairplay_conf->buttonLabelTemplate(),true,panel_player,
		       rdripc,panel_cae,rdstation_conf,panel_cart_dialog,
		       this,"panel_panel");
    panel_panel->setLogfile(panel_config->airplayLogname());
    panel_panel->setGeometry(10,10,panel_panel->sizeHint().width(),
			 panel_panel->sizeHint().height());
    if(panel_skin_pixmap!=NULL) {
      pm=new QPixmap(1024,738);
      pd=new QPainter(pm);
      pd->drawPixmap(-10,-10,*panel_skin_pixmap);
      pd->end();
      panel_panel->setErasePixmap(*pm);
      delete pd;
      delete pm;
    }
    panel_panel->setPauseEnabled(rdairplay_conf->panelPauseEnabled());
    panel_panel->setCard(0,rdairplay_conf->card(2));
    panel_panel->setPort(0,rdairplay_conf->port(2));
    panel_panel->setFocusPolicy(QWidget::NoFocus);
    if((card=rdairplay_conf->card(6))<0) {
      panel_panel->setCard(1,panel_panel->card(0));
      panel_panel->setPort(1,panel_panel->port(0));
    }
    else {
      panel_panel->setCard(1,card);
      panel_panel->setPort(1,rdairplay_conf->port(6));
    }
    if((card=rdairplay_conf->card(7))<0) {
      panel_panel->setCard(2,panel_panel->card(1));
      panel_panel->setPort(2,panel_panel->port(1));
    }
    else {
      panel_panel->setCard(2,card);
      panel_panel->setPort(2,rdairplay_conf->port(7));
    }
    if((card=rdairplay_conf->card(8))<0) {
      panel_panel->setCard(3,panel_panel->card(2));
      panel_panel->setPort(3,panel_panel->port(2));
    }
    else {
      panel_panel->setCard(3,card);
      panel_panel->setPort(3,rdairplay_conf->port(8));
    }
    if((card=rdairplay_conf->card(9))<0) {
      panel_panel->setCard(4,panel_panel->card(3));
      panel_panel->setPort(4,panel_panel->port(3));
    }
    else {
      panel_panel->setCard(4,card);
      panel_panel->setPort(4,rdairplay_conf->port(9));
    }

    //
    // Calculate Valid Ports for Reading Meter Data (No Duplicates)
    //
    for(int i=4;i>=0;i--) {
      meter_data_valid[i]=(panel_panel->card(i)>=0);
      for(int j=0;j<i;j++) {
	if((panel_panel->card(i)==panel_panel->card(j))&&
	   (panel_panel->port(i)==panel_panel->port(j))) {
	  meter_data_valid[i]=false;
	}
      }
    }

    panel_panel->
      setRmls(0,rdairplay_conf->startRml(2),rdairplay_conf->stopRml(2));
    panel_panel->
      setRmls(1,rdairplay_conf->startRml(6),rdairplay_conf->stopRml(6));
    panel_panel->
      setRmls(2,rdairplay_conf->startRml(7),rdairplay_conf->stopRml(7));
    panel_panel->
      setRmls(3,rdairplay_conf->startRml(8),rdairplay_conf->stopRml(8));
    panel_panel->
      setRmls(4,rdairplay_conf->startRml(9),rdairplay_conf->stopRml(9));
    panel_panel->setSvcName(rdairplay_conf->defaultSvc());
    connect(rdripc,SIGNAL(userChanged()),panel_panel,SLOT(changeUser()));
    connect(panel_master_timer,SIGNAL(timeout()),
	    panel_panel,SLOT(tickClock()));
  }

  //
  // Audio Meter
  //
  panel_stereo_meter=new RDStereoMeter(this,"panel_stereo_meter");
  panel_stereo_meter->
    setGeometry(40,
		sizeHint().height()-panel_stereo_meter->sizeHint().height()-7,
		panel_stereo_meter->sizeHint().width(),
		panel_stereo_meter->sizeHint().height());
  panel_stereo_meter->setMode(RDSegMeter::Peak);
  panel_stereo_meter->setFocusPolicy(QWidget::NoFocus);
  if(panel_config->useStreamMeters()) {
    panel_stereo_meter->hide();
  }

  rdripc->connectHost("localhost",RIPCD_TCP_PORT,panel_config->password());

  //
  // Signal Handlers
  //
  signal(SIGCHLD,SigHandler);
}


QSize MainWidget::sizeHint() const
{
  return QSize(935,738);
}


void MainWidget::rmlReceivedData(RDMacro *rml)
{
  RunLocalMacros(rml);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::userData()
{
  if(rduser!=NULL) {
    delete rduser;
  }
  rduser=new RDUser(rdripc->user());
  SetCaption();
  rdripc->sendOnairFlag();
}


void MainWidget::meterData()
{
#ifdef SHOW_METER_SLOTS
  printf("meterData()\n");
#endif
  double ratio[2]={0.0,0.0};
  short level[2];

  for(int i=0;i<PANEL_MAX_OUTPUTS;i++) {
    if(meter_data_valid[i]) {
      panel_cae->
	outputMeterUpdate(panel_panel->card(i),panel_panel->port(i),level);
      for(int j=0;j<2;j++) {
	ratio[j]+=pow(10.0,((double)level[j])/1000.0);
      }
    }
  }
  panel_stereo_meter->setLeftPeakBar((int)(log10(ratio[0])*1000.0));
  panel_stereo_meter->setRightPeakBar((int)(log10(ratio[1])*1000.0));
}


void MainWidget::masterTimerData()
{
}


void MainWidget::closeEvent(QCloseEvent *e)
{
  panel_db->removeDatabase(panel_config->mysqlDbname());
  exit(0);
}


void MainWidget::RunLocalMacros(RDMacro *rml)
{
}


void MainWidget::SetCaption()
{
  setCaption(QString().sprintf("RDPanel - Station: %s  User: %s",
			       (const char *)panel_config->stationName(),
			       (const char *)rdripc->user()));
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  
  //
  // Load Translations
  //
  QTranslator qt(0);
  qt.load(QString(QTDIR)+QString("/translations/qt_")+QTextCodec::locale(),".");
  a.installTranslator(&qt);

  QTranslator rd(0);
  rd.load(QString(PREFIX)+QString("/share/rivendell/librd_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&rd);

  QTranslator rdhpi(0);
  rdhpi.load(QString(PREFIX)+QString("/share/rivendell/librdhpi_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&rdhpi);

  QTranslator tr(0);
  tr.load(QString(PREFIX)+QString("/share/rivendell/rdpanel_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&tr);

  MainWidget *w=new MainWidget(NULL,"main");
  a.setMainWidget(w);
  w->setGeometry(QRect(QPoint(0,0),w->sizeHint()));
  w->show();
  return a.exec();
}
