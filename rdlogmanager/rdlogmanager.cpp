// rdlogmanager.cpp
//
// The Log Editor Utility for Rivendell.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdlogmanager.cpp,v 1.37.2.5 2010/01/31 22:24:55 cvs Exp $
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

#ifndef WIN32
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif  // WIN32
#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qsqldatabase.h>
#include <qsqlpropertymap.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qsettings.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#include <rd.h>
#include <rduser.h>
#include <rdripc.h>
#include <rdstation.h>
#include <rdcheck_daemons.h>
#include <rdcreate_log.h>
#include <rdcmd_switch.h>
#include <rddbheartbeat.h>

#include <rdlogmanager.h>
#include <globals.h>
#include <list_events.h>
#include <list_clocks.h>
#include <list_grids.h>
#include <generate_log.h>
#include <list_svcs.h>


//
// Icons
//
#include "../icons/rivendell-22x22.xpm"

//
// Global Resources
//
RDStation *rdstation_conf;
RDUser *rduser;
RDRipc *rdripc;
RDCae *rdcae;
QString *event_filter;
QString *clock_filter;


#ifndef WIN32
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
	break;
  }
}
#endif  // WIN32


MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  //
  // HACK: Disable the Broken Custom SuSE Dialogs
  //
#ifndef WIN32
  setenv("QT_NO_KDE_INTEGRATION","1",1);
#endif  // WIN32

  //
  // Read Command Options
  //
  bool cmd_generate = false;
  bool cmd_merge_music = false;
  bool cmd_merge_traffic = false;
  QString cmd_service = NULL;
  QDate cmd_date = QDate::currentDate().addDays(1);

  RDCmdSwitch *cmd=
    new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdlogmanager","\n");
  for(unsigned i=0;i<cmd->keys();i++)
     {
     if (cmd->key(i)=="-g")
       cmd_generate = true;
     if (cmd->key(i)=="-m")
       cmd_merge_music = true;
     if (cmd->key(i)=="-t")
       cmd_merge_traffic = true;
     if (cmd->key(i)=="-s")
       if (i+1<cmd->keys())
         {
         i++;
         cmd_service = cmd->key(i);
         }
     if (cmd->key(i)=="-d")
       if (i+1<cmd->keys())
         {
         i++;
         cmd_date = QDate::currentDate().addDays(cmd->key(i).toInt());
         }
    }
  delete cmd;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Ensure that the system daemons are running
  //
#ifndef WIN32
  RDInitializeDaemons();
#endif  // WIN32

  //
  // Load Local Configs
  //
  log_config=new RDConfig();
  log_config->load();
  setCaption(tr("RDLogManager"));

  //
  // Open Database
  //
  QString err;
  log_db=RDInitDb(&err);
  if(!log_db) {
    QMessageBox::warning(this,tr("Can't Connect"),err);
    exit(0);
  }
  new RDDbHeartbeat(log_config->mysqlHeartbeatInterval(),this);

  //
  // Allocate Global Resources
  //
  rdstation_conf=new RDStation(log_config->stationName());
   
  //
  // CAE Connection
  //
#ifndef WIN32
  rdcae=new RDCae(parent,name);
  rdcae->connectHost("localhost",CAED_TCP_PORT,log_config->password());
#endif  // WIN32

  //
  // RIPC Connection
  //
  rdripc=new RDRipc(log_config->stationName());
  connect(rdripc,SIGNAL(userChanged()),this,SLOT(userData()));
  rdripc->connectHost("localhost",RIPCD_TCP_PORT,log_config->password());

  //
  // User
  //
  rduser=NULL;

  //
  // Generate Fonts
  //
  QFont default_font("Helvetica",12,QFont::Normal);
  default_font.setPixelSize(12);
  qApp->setFont(default_font);
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont label_font=QFont("Helvetica",12,QFont::Bold);
  label_font.setPixelSize(12);
  QFont day_font=QFont("Helvetica",12,QFont::Normal);
  day_font.setPixelSize(12);

  //
  // Create And Set Icon
  //
  log_rivendell_map=new QPixmap(rivendell_xpm);
  setIcon(*log_rivendell_map);

  //
  // Filters
  //
  event_filter=new QString();
  clock_filter=new QString();

  //
  // Title Label
  //
  QLabel *label=new QLabel(tr("RDLogManager"),this,"title_label");
  label->setGeometry(0,5,sizeHint().width(),32);
  label->setFont(label_font);
  label->setAlignment(AlignHCenter);
  label=new QLabel(tr("Select an operation:"),this,"instruction_label");
  label->setGeometry(0,25,sizeHint().width(),16);
  label->setFont(day_font);
  label->setAlignment(AlignCenter);

  //
  //  Edit Events Button
  //
  log_events_button=new QPushButton(this,"events_button");
  log_events_button->setGeometry(10,45,sizeHint().width()-20,50);
  log_events_button->setFont(button_font);
  log_events_button->setText(tr("Edit &Events"));
  connect(log_events_button,SIGNAL(clicked()),this,SLOT(eventsData()));

  //
  //  Edit Clocks Button
  //
  log_clocks_button=new QPushButton(this,"clocks_button");
  log_clocks_button->setGeometry(10,95,sizeHint().width()-20,50);
  log_clocks_button->setFont(button_font);
  log_clocks_button->setText(tr("Edit C&locks"));
  connect(log_clocks_button,SIGNAL(clicked()),this,SLOT(clocksData()));

  //
  //  Edit Grids Button
  //
  log_grids_button=new QPushButton(this,"grid_button");
  log_grids_button->setGeometry(10,145,sizeHint().width()-20,50);
  log_grids_button->setFont(button_font);
  log_grids_button->setText(tr("Edit G&rids"));
  connect(log_grids_button,SIGNAL(clicked()),this,SLOT(gridsData()));

  //
  //  Generate Logs Button
  //
  log_logs_button=new QPushButton(this,"logs_button");
  log_logs_button->setGeometry(10,195,sizeHint().width()-20,50);
  log_logs_button->setFont(button_font);
  log_logs_button->setText(tr("&Generate Logs"));
  connect(log_logs_button,SIGNAL(clicked()),this,SLOT(generateData()));

  //
  //  Generate Reports Button
  //
  log_reports_button=new QPushButton(this,"reports_button");
  log_reports_button->setGeometry(10,245,sizeHint().width()-20,50);
  log_reports_button->setFont(button_font);
  log_reports_button->setText(tr("Manage &Reports"));
  connect(log_reports_button,SIGNAL(clicked()),this,SLOT(reportsData()));

  //
  //  Close Button
  //
  log_close_button=new QPushButton(this,"close_button");
  log_close_button->setGeometry(10,sizeHint().height()-60,
				sizeHint().width()-20,50);
  log_close_button->setFont(button_font);
  log_close_button->setText(tr("&Close"));
  log_close_button->setDefault(true);
  connect(log_close_button,SIGNAL(clicked()),this,SLOT(quitMainWidget()));

  if (cmd_generate)
    {
    GenerateLog *generatelog=new GenerateLog(this,"list_grids",1,&cmd_service,&cmd_date);
    delete generatelog;
    }
  if (cmd_merge_music)
    {
    GenerateLog *generatelog=new GenerateLog(this,"list_grids",2,&cmd_service,&cmd_date);
    delete generatelog;
    }
  if (cmd_merge_traffic)
    {
    GenerateLog *generatelog=new GenerateLog(this,"list_grids",3,&cmd_service,&cmd_date);
    delete generatelog;
    }
   if (cmd_generate || cmd_merge_music ||cmd_merge_traffic )
    quitMainWidget();

#ifndef WIN32
  signal(SIGCHLD,SigHandler);
#endif  // WIN32
}


QSize MainWidget::sizeHint() const
{
  return QSize(200,360);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::userData()
{
  QString str1=tr("RDLogManager - User: ");
  setCaption(QString().sprintf("%s%s",(const char *)str1,
			       (const char *)rdripc->user()));

  if(rduser!=NULL) {
    delete rduser;
  }
  rduser=new RDUser(rdripc->user());

  //
  // Set Control Perms
  //
  bool templates_allowed=rduser->modifyTemplate();
  bool creation_allowed=rduser->createLog();
  bool rec_allowed=rduser->deleteRec();
  log_events_button->setEnabled(templates_allowed);
  log_clocks_button->setEnabled(templates_allowed);
  log_grids_button->setEnabled(templates_allowed);
  log_logs_button->setEnabled(creation_allowed);
  log_reports_button->setEnabled(creation_allowed|rec_allowed);
}


void MainWidget::eventsData()
{
  ListEvents *events=new ListEvents(NULL,this,"list_events");
  events->exec();
  delete events;
}


void MainWidget::clocksData()
{
  ListClocks *clocks=new ListClocks(NULL,this,"list_clocks");
  clocks->exec();
  delete clocks;
}


void MainWidget::gridsData()
{
  ListGrids *grids=new ListGrids(this,"list_grids");
  grids->exec();
  delete grids;
}


void MainWidget::generateData()
{
  GenerateLog *generatelog=new GenerateLog(this,"list_grids");
  generatelog->exec();
  delete generatelog;
}


void MainWidget::reportsData()
{
  ListSvcs *recs=new ListSvcs(this,"list_recs");
  recs->exec();
  delete recs;
}


void MainWidget::quitMainWidget()
{
  log_db->removeDatabase(log_config->mysqlDbname());
  exit(0);
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  
  //
  // Load Translations
  //
  QString tr_path;
  QString qt_path;
#ifdef WIN32
  QSettings settings;
  settings.insertSearchPath(QSettings::Windows,"/SalemRadioLabs");
  tr_path=QString().sprintf("%s\\",
			    (const char *)settings.
			    readEntry("/Rivendell/InstallDir"));
  qt_path=tr_path;
#else
  tr_path=QString(PREFIX)+QString("/share/rivendell/");
  qt_path=QString(QTDIR)+QString("/translation/");
#endif  // WIN32
  QTranslator qt(0);
  qt.load(qt_path+QString("qt_")+QTextCodec::locale(),".");
  a.installTranslator(&qt);

  QTranslator rd(0);
  rd.load(tr_path+QString("librd_")+QTextCodec::locale(),".");
  a.installTranslator(&rd);

  QTranslator rdhpi(0);
  rdhpi.load(tr_path+QString("librdhpi_")+QTextCodec::locale(),".");
  a.installTranslator(&rdhpi);

  QTranslator tr(0);
  tr.load(tr_path+QString("rdlogmanager_")+QTextCodec::locale(),".");
  a.installTranslator(&tr);

  //
  // Start Event Loop
  //
  MainWidget *w=new MainWidget(NULL,"main");
  a.setMainWidget(w);
  w->setGeometry(QRect(QPoint(w->geometry().x(),w->geometry().y()),
		       w->sizeHint()));
  w->show();
  return a.exec();
}


