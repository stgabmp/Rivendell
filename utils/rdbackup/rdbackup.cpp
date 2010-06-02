// rdbackup.cpp
//
// A Database Backup Tool for Rivendell.
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdbackup.cpp,v 1.5 2007/09/14 14:07:01 fredg Exp $
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

#include <limits.h>
#include <glob.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <qapplication.h>
#include <qdir.h>

#include <rd.h>
#include <rdbackup.h>
#include <rdcart.h>
#include <rdlog.h>


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


MainObject::MainObject(QObject *parent,const char *name)
  :QObject(parent,name)
{
  QString logname;
  purge_after_days=7;

  //
  // Read Command Options
  //
  RDCmdSwitch *cmd=
    new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdbackup",RDBACKUP_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--destination-dir") {
      destination_dir=cmd->value(i);
    }
    if(cmd->key(i)=="--purge-after") {
      bool ok;
      purge_after_days=cmd->value(i).toUInt(&ok);
      if(!ok) {
	fprintf(stderr,"rdbackup: invalid --purge-after value\n");
	exit(256);
      }
    }
  }
  if(destination_dir.isEmpty()) {
    fprintf(stderr,"rdbackup: invalid --destination-dir value\n");
    exit(256);
  }
  QDir dir(destination_dir);
  if(!dir.exists()) {
    fprintf(stderr,"rdbackup: --destination-dir does not exist\n");
    exit(256);
  }

  //
  // Read Configuration
  //
  rdconfig=new RDConfig();
  rdconfig->load();

  //
  // Setup Signalling
  //
  signal(SIGCHLD,SigHandler);

  //
  // Dump the database
  //
  if(!DumpDatabase(&output_name)) {
    fprintf(stderr,"rdbackup: database dump failed");
    exit(256);
  }

  //
  // Purge old backups
  //
  PurgeFiles(destination_dir);

  //
  // Distribute the database
  //
  PushFile(output_name);

  exit(0);
}


bool MainObject::DumpDatabase(QString *filename)
{
  if(destination_dir.right(1)!="/") {
    destination_dir+="/";
  }
  *filename=GetOutputName(destination_dir);
  QString dumpcmd=QString().
    sprintf("mysqldump --opt %s -h %s -u %s -p%s | gzip - > %s",
	    (const char *)rdconfig->mysqlDbname(),
	    (const char *)rdconfig->mysqlHostname(),
	    (const char *)rdconfig->mysqlUsername(),
	    (const char *)rdconfig->mysqlPassword(),
	    (const char *)(*filename));
  return system(dumpcmd)==0;
}


void MainObject::PurgeFiles(const QString &path)
{
  QString dest;
  int c=0;
  int colon=0;

  if(purge_after_days==0) {
    return;
  }
  QString cmd=
    QString().sprintf("find %s -mtime %u -type f -exec rm \\{\\} \\;",
		      (const char *)path,purge_after_days);
  system(cmd);

  while(!(dest=rdconfig->destination(c++)).isEmpty()) {
    if((colon=dest.find(":"))>=0) {
      cmd=QString().
	sprintf("ssh %s \"find %s -mtime %u -type f -exec rm \\{\\} \\;",
		(const char *)dest.left(colon),
		(const char *)dest.right(dest.length()-colon-1),
		purge_after_days);
    }
  }
}


void MainObject::PushFile(const QString &filename)
{
  QString dest;
  int c=0;
  QString cmd;
  while(!(dest=rdconfig->destination(c++)).isEmpty()) {
    cmd=QString().sprintf("scp -q -B -o ConnectTimeout\\ 120 %s %s",
			  (const char *)filename,
			  (const char *)dest);
    if(system(cmd)!=0) {
      fprintf(stderr,"rdbackup: copy to \"%s\" failed\n",(const char *)dest);
    }
  }
}


QString MainObject::GetOutputName(const QString &path)
{
  unsigned ver=0;
  QString testname=
    QString().sprintf("%srivendell-%s.sql.gz",
		      (const char *)path,
		      (const char *)QDate::currentDate().toString("yyyyMMdd"));
  while(QFile::exists(testname)) {
  testname=
    QString().sprintf("%srivendell-%s[%d].sql.gz",
		      (const char *)path,
		      (const char *)QDate::currentDate().toString("yyyyMMdd"),
		      ver++);
  }
  return testname;
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
