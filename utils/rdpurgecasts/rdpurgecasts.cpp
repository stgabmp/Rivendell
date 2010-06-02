// rdpurgecasts.cpp
//
// A Utility to Purge Expired Podcasts.
//
//   (C) Copyright 2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdpurgecasts.cpp,v 1.4.4.1 2010/05/11 14:43:12 cvs Exp $
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
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <qapplication.h>
#include <qdir.h>

#include <rd.h>
#include <rdconf.h>
#include <rdpurgecasts.h>
#include <rdlibrary_conf.h>
#include <rdescape_string.h>
#include <rddb.h>
#include <rdurl.h>
#include <rdpodcast.h>


MainObject::MainObject(QObject *parent,const char *name)
  :QObject(parent,name)
{
  QString sql;
  RDSqlQuery *q;

  //
  // Initialize Data Structures
  //
  purge_verbose=false;

  //
  // Read Command Options
  //
  purge_cmd=new RDCmdSwitch(qApp->argc(),qApp->argv(),
			     "rdpurgecasts",RDPURGECASTS_USAGE);
  if(purge_cmd->keys()>2) {
    fprintf(stderr,"\n");
    fprintf(stderr,"%s",RDPURGECASTS_USAGE);
    fprintf(stderr,"\n");
    delete purge_cmd;
    exit(256);
  }
  for(unsigned i=0;i<purge_cmd->keys();i++) {
    if(purge_cmd->key(i)=="--verbose") {
      purge_verbose=true;
    }
  }

  //
  // Read Configuration
  //
  purge_config=new RDConfig();
  purge_config->load();

  //
  // Open Database
  //
  QString err (tr("rdpurgecasts: "));
  QSqlDatabase *db=RDInitDb(&err);
  if(!db) {
    fprintf(stderr,err.ascii());
    delete purge_cmd;
    exit(256);
  }
  delete purge_cmd;

  //
  // Scan Podcasts
  //
  QDateTime current_datetime=
    QDateTime(QDate::currentDate(),QTime::currentTime());
  sql=QString().sprintf("select ID,ORIGIN_DATETIME,SHELF_LIFE from PODCASTS \
                         where (SHELF_LIFE>0)&&(STATUS=%u)",
			RDPodcast::StatusActive);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    if(q->value(1).toDateTime().addDays(q->value(2).toInt())<
       current_datetime) {
      PurgeCast(q->value(0).toUInt());
    }
  }
  delete q;

  exit(0);
}


void MainObject::PurgeCast(unsigned id)
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  QString cmd;
  QDateTime current_datetime=
    QDateTime(QDate::currentDate(),QTime::currentTime());

  sql=QString().sprintf("select PODCASTS.AUDIO_FILENAME,FEEDS.PURGE_URL,\
                         FEEDS.PURGE_USERNAME,FEEDS.PURGE_PASSWORD,FEEDS.ID,\
                         FEEDS.KEEP_METADATA,FEEDS.KEY_NAME \
                         from PODCASTS left join FEEDS \
                         on(PODCASTS.FEED_ID=FEEDS.ID) \
                         where PODCASTS.ID=%u",id);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    cmd=GetPurgeCommand(q->value(0).toString(),q->value(1).toString(),
			q->value(2).toString(),q->value(3).toString());
    if(purge_verbose) {
      printf("purging cast: ID=%d,cmd=\"%s\"\n",id,(const char *)cmd);
    }

    system(cmd);
    if(RDBool(q->value(5).toString())) {
      sql=QString().sprintf("update PODCASTS set STATUS=%u \
                             where ID=%u",
			    RDPodcast::StatusExpired,id);
      q1=new RDSqlQuery(sql);
      delete q1;
    }
    else {
      QString keyname=q->value(6).toString();
      keyname.replace(" ","_");
      sql=QString().sprintf("delete from %s_FLG where CAST_ID=%d",
			    (const char *)keyname,id);
      q1=new RDSqlQuery(sql);
      delete q1;

      sql=QString().sprintf("delete from PODCASTS where ID=%d",id);
      q1=new RDSqlQuery(sql);
      delete q1;
    }
    sql=QString().sprintf("update FEEDS set LAST_BUILD_DATETIME=\"%s\" \
                           where ID=%u",
			  (const char *)current_datetime.
			  toString("yyyy-MM-dd hh:mm:ss"),
			  q->value(4).toUInt());
    q1=new RDSqlQuery(sql);
    delete q1;

  }
  delete q;
}


QString MainObject::GetPurgeCommand(const QString &filename,const QString &url,
				    const QString username,
				    const QString &passwd)
{
  QString cmd;
  RDUrl rdurl(url);
  QString protocol=rdurl.protocol();

  if(protocol=="file") {
    cmd=QString().sprintf("rm \"%s/%s\"",(const char *)rdurl.path(),
			  (const char *)filename);
  }
  if(protocol=="ftp") {
    cmd=QString().sprintf("lftp -e \"set net:max-retries 1;rm %s;bye\"",(const char *)filename);
    if(!username.isEmpty()) {
      cmd+=QString().sprintf(" -u \"%s:",(const char *)username);
      if(passwd.isEmpty()) {
	cmd+="\"";
      }
      else {
	cmd+=QString().sprintf("%s\"",(const char *)passwd);
      }
    }
    cmd+=QString().sprintf(" ftp://%s%s",(const char *)rdurl.host(),
				 (const char *)rdurl.path());
    cmd+=" > /dev/null 2> /dev/null";
  }
  if(protocol=="smb") {
    if(!rdurl.validSmbShare()) {
      return QString();
    }
    QString path=RDGetPathPart(rdurl.path());
    cmd=QString().sprintf("smbclient \"%s\" ",(const char *)rdurl.smbShare());
    if(!passwd.isEmpty()) {
      cmd+=QString().sprintf("\"%s\" ",(const char *)passwd);
    }
    if(!username.isEmpty()) {
      cmd+=QString().sprintf("-U %s ",(const char *)username);
    }
    cmd+=QString().
      sprintf("-c \"cd %s;del %s\"",(const char *)rdurl.smbPath(),
	      (const char *)filename);
  }  

  return cmd;
}
  
  
int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
