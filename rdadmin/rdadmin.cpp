// rdadmin.cpp
//
// The Administrator Utility for Rivendell.
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdadmin.cpp,v 1.64.2.3 2010/01/21 16:44:32 cvs Exp $
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#include <rdconf.h>
#include <rduser.h>
#include <rd.h>
#include <dbversion.h>
#include <rdcheck_daemons.h>
#include <rdcmd_switch.h>
#include <rddb.h>
#include <rddbheartbeat.h>

#include <globals.h>
#include <login.h>
#include <list_users.h>
#include <list_groups.h>
#include <list_svcs.h>
#include <list_stations.h>
#include <list_reports.h>
#include <list_feeds.h>
#include <list_schedcodes.h>
#include <edit_settings.h>
#include <rdadmin.h>
#include <opendb.h>
#include <info_dialog.h>
#include <createdb.h>

#include <rdescape_string.h>

//
// Icons
//
#include "../icons/rivendell-22x22.xpm"

//
// Global Classes
//
RDRipc *rdripc;
RDConfig *admin_config;
RDUser *admin_user;
RDStation *admin_station;
RDCartDialog *admin_cart_dialog;
bool exiting=false;


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


MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  QString str;

  //
  // HACK: Disable the Broken Custom SuSE Dialogs
  //
  setenv("QT_NO_KDE_INTEGRATION","1",1);

  //
  // Read Command Options
  //
  RDCmdSwitch *cmd=
    new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdadmin",RDADMIN_USAGE);
  delete cmd;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont default_font("Helvetica",12,QFont::Normal);
  default_font.setPixelSize(12);
  qApp->setFont(default_font);

  //
  // Create And Set Icon
  //
  admin_rivendell_map=new QPixmap(rivendell_xpm);
  setIcon(*admin_rivendell_map);

  //
  // Load Configs
  //
  admin_config=new RDConfig();
  admin_config->load();

  str=QString(tr("RDAdmin - Host:"));
  setCaption(QString().
	     sprintf("%s %s",(const char *)str,
		     (const char *)admin_config->stationName()));

  //
  // Open Database
  //
  if(!OpenDb(admin_config->mysqlDbname(),admin_config->mysqlUsername(),
	     admin_config->mysqlPassword(),admin_config->mysqlHostname(),
	     admin_config->stationName(),true)) {
    exit(1);
  }
  
  new RDDbHeartbeat(admin_config->mysqlHeartbeatInterval());

  //
  // Check (and possibly start) daemons
  //
  if(!RDStartDaemons()) {
    QMessageBox::warning(this,tr("Daemons Failed"),
			 tr("Unable to start Rivendell System Daemons!"));
    exit(1);
  }

  //
  // Initialize Global Classes
  //
  char temp[256];
  GetPrivateProfileString(RD_CONF_FILE,"Identity","Password",
			  temp,"",255);
  rdripc=new RDRipc(admin_config->stationName(),this,"rdripc");
  rdripc->connectHost("localhost",RIPCD_TCP_PORT,temp);
  admin_station=new RDStation(admin_config->stationName(),this);

  //
  // Log In
  //
  Login *login=new Login(&admin_username,&admin_password,this,"login");
  if(login->exec()!=0) {
    exit(0);
  }
  admin_user=new RDUser(admin_username);
  if(!admin_user->checkPassword(admin_password,false)) {
    QMessageBox::warning(this,"Login Failed","Login Failed!.\n");
    exiting=true;
  }
  else {
    if(!admin_user->adminConfig()) {
      QMessageBox::warning(this,tr("Insufficient Priviledges"),
         tr("This account has insufficient priviledges for this operation."));
      exiting=true;
    }
  }

  //
  // Cart Dialog
  //
  admin_cart_dialog=new RDCartDialog(&admin_filter,&admin_group,-1,-1,0,0,NULL,
				     rdripc,admin_station,"",
				     this,"admin_cart_dialog");

  //
  // User Labels
  //
  QLabel *name_label=new QLabel(this,"name_label");
  name_label->setGeometry(0,5,sizeHint().width(),20);
  name_label->setAlignment(Qt::AlignVCenter|Qt::AlignCenter);
  name_label->setFont(font);
  name_label->setText(QString().sprintf("USER: %s",(const char *)admin_user->name()));

  QLabel *description_label=new QLabel(this,"description_label");
  description_label->setGeometry(0,24,sizeHint().width(),14);
  description_label->setAlignment(Qt::AlignVCenter|Qt::AlignCenter);
  name_label->setFont(font);
  description_label->setText(admin_user->description());

  //
  // Manage Users Button
  //
  QPushButton *users_button=new QPushButton(this,"users_button");
  users_button->setGeometry(10,50,80,60);
  users_button->setFont(font);
  users_button->setText(tr("Manage\n&Users"));
  connect(users_button,SIGNAL(clicked()),this,SLOT(manageUsersData()));

  //
  // Manage Groups Button
  //
  QPushButton *groups_button=new QPushButton(this,"groups_button");
  groups_button->setGeometry(10,120,80,60);
  groups_button->setFont(font);
  groups_button->setText(tr("Manage\n&Groups"));
  connect(groups_button,SIGNAL(clicked()),this,SLOT(manageGroupsData()));
  
  //
  // Manage Services Button
  //
  QPushButton *services_button=new QPushButton(this,"services_button");
  services_button->setGeometry(100,50,80,60);
  services_button->setFont(font);
  services_button->setText(tr("Manage\n&Services"));
  connect(services_button,SIGNAL(clicked()),this,SLOT(manageServicesData()));
  
  //
  // Manage Stations (Hosts) Button
  //
  QPushButton *stations_button=new QPushButton(this,"stations_button");
  stations_button->setGeometry(100,120,80,60);
  stations_button->setFont(font);
  stations_button->setText(tr("Manage\nHo&sts"));
  connect(stations_button,SIGNAL(clicked()),this,SLOT(manageStationsData()));
  
  //
  // Manage Reports
  //
  QPushButton *reports_button=new QPushButton(this,"reports_button");
  reports_button->setGeometry(190,50,80,60);
  reports_button->setFont(font);
  reports_button->setText(tr("Manage\nR&eports"));
  connect(reports_button,SIGNAL(clicked()),this,SLOT(reportsData()));

  //
  // Manage Podcasts
  //
  QPushButton *podcasts_button=new QPushButton(this,"podcasts_button");
  podcasts_button->setGeometry(280,50,80,60);
  podcasts_button->setFont(font);
  podcasts_button->setText(tr("Manage\n&Feeds"));
  connect(podcasts_button,SIGNAL(clicked()),this,SLOT(podcastsData()));

  //
  // System Wide Settings Button
  //
  QPushButton *system_button=new QPushButton(this,"system_button");
  system_button->setGeometry(190,120,80,60);
  system_button->setFont(font);
  system_button->setText(tr("System\nSettings"));
  connect(system_button,SIGNAL(clicked()),this,SLOT(systemSettingsData()));

  //
  // Manage Scheduler Codes Button
  //
  QPushButton *schedcodes_button=new QPushButton(this,"schedcodes_button");
  schedcodes_button->setGeometry(280,120,80,60);
  schedcodes_button->setFont(font);
  schedcodes_button->setText(tr("Scheduler\nCodes"));
  connect(schedcodes_button,SIGNAL(clicked()),this,SLOT(manageSchedCodes()));

  //
  // System Info Button
  //
  QPushButton *info_button=new QPushButton(this,"info_button");
  info_button->setGeometry(55,190,80,60);
  info_button->setFont(font);
  info_button->setText(tr("System\nInfo"));
  connect(info_button,SIGNAL(clicked()),this,SLOT(systemInfoData()));

  //
  // Backup Database Button
  //
  QPushButton *backup_button=new QPushButton(this,"backup_button");
  backup_button->setGeometry(145,190,80,60);
  backup_button->setFont(font);
  backup_button->setText(tr("&Backup\nDatabase"));
  connect(backup_button,SIGNAL(clicked()),this,SLOT(backupData()));

  //
  // Restore Database Button
  //
  QPushButton *restore_button=new QPushButton(this,"restore_button");
  restore_button->setGeometry(235,190,80,60);
  restore_button->setFont(font);
  restore_button->setText(tr("&Restore\nDatabase"));
  connect(restore_button,SIGNAL(clicked()),this,SLOT(restoreData()));
  
  //
  // Quit Button
  //
  QPushButton *quit_button=new QPushButton(this,"quit_button");
  quit_button->setGeometry(10,sizeHint().height()-70,sizeHint().width()-20,60);
  quit_button->setFont(font);
  quit_button->setText(tr("&Quit"));
  connect(quit_button,SIGNAL(clicked()),this,SLOT(quitMainWidget()));

  signal(SIGCHLD,SigHandler);
}


MainWidget::~MainWidget()
{
  delete admin_user;
}


QSize MainWidget::sizeHint() const
{
  return QSize(370,330);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::manageUsersData()
{
  ListUsers *list_users=new ListUsers(admin_user->name(),this,"list_users");
  list_users->exec();
  delete list_users;
}


void MainWidget::manageGroupsData()
{
  ListGroups *list_groups=new ListGroups(this,"list_groups");
  list_groups->exec();
  delete list_groups;
}

void MainWidget::manageSchedCodes()
{
  ListSchedCodes *list_schedCodes=new ListSchedCodes(this,"list_schedCodes");
  list_schedCodes->exec();
  delete list_schedCodes;
}

void MainWidget::manageServicesData()
{
  ListSvcs *list_svcs=new ListSvcs(this,"list_svcs");
  list_svcs->exec();
  delete list_svcs;
}


void MainWidget::manageStationsData()
{
  ListStations *list_stations=new ListStations(this,"list_stations");
  list_stations->exec();
  delete list_stations;
}


void MainWidget::systemSettingsData()
{
  EditSettings *edit_settings=new EditSettings(this,"edit_settings");
  edit_settings->exec();
  delete edit_settings;
}


void MainWidget::backupData()
{
  QString filename;
  QString cmd;
  int status;

  filename=QFileDialog::getSaveFileName(RDGetHomeDir(),
				      tr("Rivendell Database Backup (*.sql)"),
					this);
  if(filename.isEmpty()) {
    return;
  }
  if(filename.right(4)!=QString(".sql")) {
    filename+=".sql";
  }
  cmd=QString().sprintf("mysqldump -c %s -h %s -u %s -p%s > %s",
			(const char *)admin_config->mysqlDbname(),
			(const char *)admin_config->mysqlHostname(),
			(const char *)admin_config->mysqlUsername(),
			(const char *)admin_config->mysqlPassword(),
			(const char *)filename);
  status=system((const char *)cmd);
  if(WEXITSTATUS(status)!=0) {
    unlink((const char *)filename);
    QMessageBox::warning(this,tr("Backup Error"),
			 tr("Unable to create backup!"));
    return;
  }
  QMessageBox::information(this,tr("Backup Complete"),
			   tr("Backup completed successfully."));
}


void MainWidget::restoreData()
{
  QString filename;
  QString cmd;
  int status;
  RDSqlQuery *q;
  int ver=RD_VERSION_DATABASE;

  if(QMessageBox::warning(NULL,tr("Restore Database"),
			  tr("WARNING: This operation will COMPLETELY\nOVERWRITE the existing Rivendell Database!\nDo you want to continue?"),
			  QMessageBox::Yes,QMessageBox::No)!=
     QMessageBox::Yes) {
    return;
  }      
  filename=QFileDialog::getOpenFileName(RDGetHomeDir(),
				    tr("Rivendell Database Backup (*.sql)"),
					this);
  if(filename.isEmpty()) {
    return;
  }
  RDKillDaemons();
  ClearTables();
  cmd=QString().sprintf("cat %s | mysql %s -h %s -u %s -p%s",
			(const char *)filename,
			(const char *)admin_config->mysqlDbname(),
			(const char *)admin_config->mysqlHostname(),
			(const char *)admin_config->mysqlUsername(),
			(const char *)admin_config->mysqlPassword());
  status=system((const char *)cmd);
  if(WEXITSTATUS(status)!=0) {
    QMessageBox::warning(this,tr("Restore Error"),
			 tr("Unable to restore backup!"));
    return;
  }
  q=new RDSqlQuery("select DB from VERSION");
  if(q->first()) {
    ver=q->value(0).toInt();
  }
  delete q;
  UpdateDb(ver);
  QMessageBox::information(this,tr("Restore Complete"),
			   tr("Restore completed successfully."));
  RDStartDaemons();
}


void MainWidget::systemInfoData()
{
  InfoDialog *info=new InfoDialog(this,"info_dialog");
  info->exec();
  delete info;
}


void MainWidget::reportsData()
{
  ListReports *list_reports=new ListReports(this,"list_reports");
  list_reports->exec();
  delete list_reports;
}


void MainWidget::podcastsData()
{
  ListFeeds *list_feeds=new ListFeeds(this,"list_feeds");
  list_feeds->exec();
  delete list_feeds;
}


void MainWidget::quitMainWidget()
{
  exit(0);
}


void MainWidget::ClearTables()
{
  RDSqlQuery *q1;

  QString sql="show tables";
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    sql=QString().sprintf("drop table %s",
			  (const char *)q->value(0).toString());
    q1=new RDSqlQuery(sql);
    delete q1;
  }
  delete q;
}


int gui_main(int argc,char *argv[])
{
  QApplication a(argc,argv);

  //
  // Load Translations
  //
  QTranslator qt(0);
  qt.load(QString(QTDIR)+QString("/translations/qt_")+QTextCodec::locale(),
	  ".");
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
  tr.load(QString(PREFIX)+QString("/share/rivendell/rdadmin_")+
	  QTextCodec::locale(),".");
  a.installTranslator(&tr);

  //
  // Start Event Loop
  //
  MainWidget *w=new MainWidget(NULL,"main");
  if(exiting) {
      exit(0);
  }
  a.setMainWidget(w);
  w->setGeometry(QRect(QPoint(0,0),w->sizeHint()));
  w->show();
  return a.exec();
}


int cmdline_main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  bool update_db=false;
  QString username=QString::QString("");
  
  for(int i=0;i<argc;i++) {
    if(!strcmp(argv[i],"--update-db")) {
      update_db=true;
      if(argc>i+1) { 
        username=(QString)argv[i+1];
	i++;
      }
    }
  }
  
  //
  // Load Configs
  //
  admin_config=new RDConfig();
  admin_config->load();

  //
  // Open Database
  //
  if(!OpenDb(admin_config->mysqlDbname(),admin_config->mysqlUsername(),
	     admin_config->mysqlPassword(),admin_config->mysqlHostname(),
	     admin_config->stationName(),false,username)) {
    return 1;
  }

  return 0;
}


int main(int argc,char *argv[])
{
  int ret;
  bool found_check_db=false;

  for(int i=0;i<argc;i++) {
    if(!strcmp(argv[i],"--check-db")) {
      found_check_db=true;
    }
    if(!strcmp(argv[i],"--update-db")) {
      found_check_db=true;
    }
  }
  if(found_check_db) {
    ret=cmdline_main(argc,argv);
  }
  else {
    ret=gui_main(argc,argv);
  }
  return ret;
}
