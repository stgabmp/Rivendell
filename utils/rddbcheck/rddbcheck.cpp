// rddbcheck.cpp
//
// A Database Check/Repair Tool for Rivendell.
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rddbcheck.cpp,v 1.11.2.1 2009/08/17 13:54:26 cvs Exp $
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
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <rddb.h>
#include <rd.h>
#include <rddbcheck.h>
#include <rdcart.h>
#include <rdlog.h>
#include <rdcreate_log.h>
#include <rdescape_string.h>

//
// MAINTAINER'S NOTE
// Be sure to use QSqlQuery here, *not* RDSqlQuery, otherwise the
// DB connection will be reset when we detect an error!
//

MainObject::MainObject(QObject *parent,const char *name)
  :QObject(parent,name)
{
  check_yes=false;
  check_no=false;

  //
  // Read Command Options
  //
  RDCmdSwitch *cmd=
    new RDCmdSwitch(qApp->argc(),qApp->argv(),"rddbcheck",RDDBCHECK_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--yes") {
      check_yes=true;
    }
    if(cmd->key(i)=="--no") {
      check_no=true;
    }
    if(cmd->key(i)=="--orphan-group") {
      orphan_group_name=cmd->value(i);
    }
    if(cmd->key(i)=="--dump-cuts-dir") {
      dump_cuts_dir=cmd->value(i);
    }
  }
  if(check_yes&&check_no) {
    fprintf(stderr,"rddbcheck: '--yes' and '--no' are mutually exclusive\n");
    exit(256);
  }

  //
  // Check for Root Perms
  //
  if(geteuid()!=0) {
    fprintf(stderr,"rddbcheck: must be user \"root\"\n");
    exit(256);
  }

  //
  // Check for dump cuts directory
  //
  if(!dump_cuts_dir.isEmpty()) {
    QFileInfo file(dump_cuts_dir);
    if(!file.exists()) {
      fprintf(stderr,"rddbcheck: directory \"%s\" does not exist.\n",
	      (const char *)dump_cuts_dir);
      exit(256);
    }
    if(!file.isDir()) {
      fprintf(stderr,"rddbcheck: \"%s\" is not a directory.\n",
	      (const char *)dump_cuts_dir);
      exit(256);
    }
    if(!file.isWritable()) {
      fprintf(stderr,"rddbcheck: \"%s\" is not writable.\n",
	      (const char *)dump_cuts_dir);
      exit(256);
    }
  }

  //
  // Read Configuration
  //
  rdconfig=new RDConfig();
  rdconfig->load();

  //
  // Open Database
  //
  QString err (tr("rddbcheck: "));
  QSqlDatabase *db=RDInitDb (&err);
  if(!db) {
    fprintf(stderr,err.ascii());
    delete cmd;
    exit(256);
  }

  if(!orphan_group_name.isEmpty()) {
    QString sql=QString().sprintf("select NAME from GROUPS where NAME=\"%s\"",
				  (const char *)orphan_group_name);
    printf("SQL: %s\n",(const char *)sql);
    QSqlQuery *q=new QSqlQuery(sql);
    if(!q->first()) {
      fprintf(stderr,"rddbcheck: invalid group \"%s\"\n",
	      (const char *)orphan_group_name);
      delete q;
      exit(256);
    }
    delete q;
  }

  //
  // Check for Orphaned Voice Tracks
  //
  printf("Checking voice tracks...\n");
  CheckOrphanedTracks();
  printf("done.\n\n");

  //
  // Check RDLogManager Consistency
  //
  printf("Checking RDLogManager events...\n");
  CheckEvents();
  printf("done.\n\n");
  printf("Checking RDLogManager clocks...\n");
  CheckClocks();
  printf("done.\n\n");

  //
  // Check for orphaned tables
  //
  printf("Checking for orphaned tables...\n");
  CheckOrphanedTables();
  printf("done.\n\n");

  //
  // Check for orphaned carts
  //
  printf("Checking for orphaned carts...\n");
  CheckOrphanedCarts();
  printf("done.\n\n");

  //
  // Check for orphaned cuts
  //
  printf("Checking for orphaned cuts...\n");
  CheckOrphanedCuts();
  printf("done.\n\n");

  //
  // Check Cart->Cut Counts
  //
  printf("Checking cart->cuts counters...\n");
  CheckCutCounts();
  printf("done.\n\n");

  //
  // Check Orphaned Audio
  //
  printf("Checking for orphaned audio...\n");
  CheckOrphanedAudio();
  printf("done.\n\n");

  exit(0);
}


void MainObject::CheckOrphanedTracks()
{
  QString logname;
  QString sql="select NUMBER,TITLE,OWNER from CART where OWNER!=\"\"";
  QSqlQuery *q=new QSqlQuery(sql);
  QSqlQuery *q1;

  while(q->next()) {
    logname=q->value(2).toString();
    logname.replace(" ","_");
    sql=QString().sprintf("select ID from %s where CART_NUMBER=%u",
			  (const char *)logname,q->value(0).toUInt());
    q1=new QSqlQuery(sql);
    if(!q1->first()) {
      printf("  Found orphaned track %u - \"%s\".  Delete? (y/N) ",
	     q->value(0).toUInt(),(const char *)q->value(1).toString());
      fflush(NULL);
      if(UserResponse()) {
	RDCart *cart=new RDCart(q->value(0).toUInt());
	cart->remove();
	delete cart;
	RDLog *log=new RDLog(q->value(2).toString());
	log->updateTracks();
	delete log;
      }
    }
    delete q1;
  }
  delete q;
}


void MainObject::CheckClocks()
{
  QString sql;
  QSqlQuery *q;
  QSqlQuery *q1;
  QSqlQuery *q2;
  QSqlQuery *q3;
  QString clockname;
  QString eventname;

  sql="select NAME from CLOCKS";
  q=new QSqlQuery(sql);
  while(q->next()) {
    clockname=q->value(0).toString();
    clockname.replace(" ","_");

    //
    // Check the CLK Table
    //
    sql=QString().sprintf ("select EVENT_NAME,ID from %s_CLK",
			   (const char *)clockname);
    q1=new QSqlQuery(sql);
    if(q1->isActive()) {
      //
      // Check the clock -> event linkage
      //
      while(q1->next()) {
	sql=QString().sprintf("select NAME from EVENTS where NAME=\"%s\"",
			      (const char *)q1->value(0).toString());
	q2=new QSqlQuery(sql);
	if(q2->first()) {
	  if(q1->value(0)!=q2->value(0)) {  // Make sure the cases match!
	    printf("  Clock %s's linkage to event %s is broken -- fix (y/N)? ",
		   (const char *)clockname,
		   (const char *)q2->value(0).toString());
	    fflush(NULL);
	    if(UserResponse()) {
	      sql=QString().sprintf("update %s_CLK set EVENT_NAME=\"%s\"\
                                     where ID=%d",
				    (const char *)clockname,
				    (const char *)q2->value(0).toString(),
				    q1->value(1).toInt());
	      q3=new QSqlQuery(sql);
	      delete q3;
	    }
	  }
	}
	delete q2;
      }
    }
    else {
      printf("  Clock %s is missing the CLK table -- fix (y/N)? ",
	     (const char *)q->value(0).toString());
      fflush(NULL);
      if(UserResponse()) {
	sql=RDCreateClockTableSql(clockname);
	q2=new QSqlQuery(sql);
	delete q2;
      }
    }
    delete q1;
  }
  delete q;

}


void MainObject::CheckEvents()
{
  QString sql;
  QSqlQuery *q;
  QSqlQuery *q1;
  QSqlQuery *q2;
  QString eventname;

  sql="select NAME from EVENTS";
  q=new QSqlQuery(sql);
  while(q->next()) {
    eventname=q->value(0).toString();
    eventname.replace(" ","_");

    //
    // Check the PRE Table
    //
    sql=QString().sprintf ("select ID from %s_PRE",(const char *)eventname);
    q1=new QSqlQuery(sql);
    if(!q1->isActive()) {
      printf("  Event %s is missing the PRE table -- fix (y/N)? ",
	     (const char *)q->value(0).toString());
      fflush(NULL);
      if(UserResponse()) {
	sql=RDCreateLogTableSql(eventname+"_PRE");
	q2=new QSqlQuery(sql);
	delete q2;
      }
    }
    delete q1;

    //
    // Check the POST Table
    //
    sql=QString().sprintf ("select ID from %s_POST",(const char *)eventname);
    q1=new QSqlQuery(sql);
    if(!q1->isActive()) {
      printf("  Event %s is missing the POST table -- fix (y/N)? ",
	     (const char *)q->value(0).toString());
      fflush(NULL);
      if(UserResponse()) {
	sql=RDCreateLogTableSql(eventname+"_POST");
	q2=new QSqlQuery(sql);
	delete q2;
      }
    }
    delete q1;
  }
  delete q;
}


void MainObject::CheckOrphanedTables()
{
  QSqlQuery *table_q;
  QSqlQuery *q;
  QString sql;

  //
  // Generate Table Query
  //
  sql="show tables";
  table_q=new QSqlQuery(sql);

  //
  // Look for orphaned clocks
  //
  sql="select NAME from CLOCKS";
  q=new QSqlQuery(sql);
  CleanTables("CLK",table_q,q);
  CleanTables("RULES",table_q,q);
  delete q;

  //
  // Look for orphaned events
  //
  sql="select NAME from EVENTS";
  q=new QSqlQuery(sql);
  CleanTables("PRE",table_q,q);
  CleanTables("POST",table_q,q);
  delete q;

  //
  // Look for orphaned logs
  //
  sql="select NAME from LOGS";
  q=new QSqlQuery(sql);
  CleanTables("LOG",table_q,q);
  CleanTables("STACK",table_q,q);
  delete q;

  //
  // Look for orphaned services
  //
  sql="select NAME from SERVICES";
  q=new QSqlQuery(sql);
  CleanTables("SRT",table_q,q);
  delete q;

  //
  // Look for orphaned feeds
  //
  sql="select KEY_NAME from FEEDS";
  q=new QSqlQuery(sql);
  CleanTables("FIELDS",table_q,q);
  CleanTables("FLG",table_q,q);
  delete q;

  //
  // Look for stale and obsolete tables
  //
  CleanTables("IMP",table_q);
  CleanTables("REC",table_q);


  delete table_q;
}


void MainObject::CheckCutCounts()
{
  QString sql;
  QSqlQuery *q;
  QSqlQuery *q1;

  sql="select NUMBER,CUT_QUANTITY,TITLE from CART";
  q=new QSqlQuery(sql);
  while(q->next()) {
    sql=QString().sprintf("select CUT_NAME from CUTS \
                           where (CART_NUMBER=%u)&&(LENGTH>0)",
			  q->value(0).toUInt());
    q1=new QSqlQuery(sql);
    if(q1->size()!=q->value(1).toInt()) {
      printf("  Cart %u [%s] has invalid cut count, fix (y/N)?",
	     q->value(0).toUInt(),(const char *)q->value(2).toString());
      if(UserResponse()) {
	RDCart *cart=new RDCart(q->value(0).toUInt());
	cart->updateLength();
	cart->resetRotation();
	delete cart;
      }
    }
    delete q1;
  }
  delete q;
}


void MainObject::CheckOrphanedCarts()
{
  QString sql;
  QSqlQuery *q;
  QSqlQuery *q1;

  sql="select CART.NUMBER,CART.TITLE from CART left join GROUPS \
       on CART.GROUP_NAME=GROUPS.NAME where GROUPS.NAME is null";
  q=new QSqlQuery(sql);
  while(q->next()) {
    printf("  Cart %06u [%s] has missing/invalid group.\n",
	   q->value(0).toUInt(),(const char *)q->value(1).toString());
    if(orphan_group_name.isEmpty()) {
      printf("  Rerun rddbcheck with the --orphan-group= switch to fix.\n\n");
    }
    else {
      printf("  Assign to group \"%s\" (y/N)?",(const char *)orphan_group_name);
      if(UserResponse()) {
	sql=QString().
	  sprintf("update CART set GROUP_NAME=\"%s\" where NUMBER=%u",
		  (const char *)RDEscapeString(orphan_group_name),
		  q->value(0).toUInt());
	q1=new QSqlQuery(sql);
	delete q1;
      }
      printf("\n");
    }
  }
  delete q;
}


void MainObject::CheckOrphanedCuts()
{
  QString sql;
  QSqlQuery *q;
  QSqlQuery *q1;
  QSqlQuery *q2;
  QFileInfo *file=NULL;

  sql="select CUTS.CUT_NAME,CUTS.DESCRIPTION from CUTS left join CART \
       on CUTS.CART_NUMBER=CART.NUMBER where CART.NUMBER is null";
  q=new QSqlQuery(sql);
  while(q->next()) {
    printf("  Cut %s [%s] is orphaned.\n",
	   (const char *)q->value(0).toString(),
	   (const char *)q->value(1).toString());
    //
    // Try to repair it
    //
    sql=QString().sprintf("select NUMBER from CART where NUMBER=%d",
			  q->value(0).toString().left(6).toUInt());
    q1=new QSqlQuery(sql);
    if(q1->first()) {
      printf("  Repair it (y/N)?");
      if(UserResponse()) {
	//
	// FIXME: Regen Cart Data
	//
	sql=QString().
	  sprintf("update CUTS set CART_NUMBER=%u where CUT_NAME=\"%s\"",
		  q1->value(0).toUInt(),
		  (const char *)q->value(0).toString());
	q2=new QSqlQuery(sql);
	delete q2;
	delete q1;
	printf("\n");
	continue;
      }
    }
    printf("\n");
    delete q1;

    //
    // Try to recover audio
    //
    file=new QFileInfo(RDCut::pathName(q->value(0).toString()));
    if(!file->exists()) {
      printf("  Clear it (y/N)?");
      if(UserResponse()) {
	sql=QString().sprintf("delete from CUTS where CUT_NAME=\"%s\"",
			      (const char *)q->value(0).toString());
	q1=new QSqlQuery(sql);
	delete q1;
      }
    }
    else {
      if(dump_cuts_dir.isEmpty()) {
	printf("  Rerun rddbcheck with the --dump-cuts-dir= switch to fix.\n");
      }
      else {
	printf("  Clear it (y/N)?");
	if(UserResponse()) {
	  system("mv "+file->filePath()+" "+dump_cuts_dir+"/");
	  sql=QString().sprintf("delete from CUTS where CUT_NAME=\"%s\"",
				(const char *)q->value(0).toString());
	  q1=new QSqlQuery(sql);
	  delete q1;
	  printf("  Saved audio in \"%s/%s\"\n",(const char *)dump_cuts_dir,
		 (const char *)file->fileName());
	}
      }
    }
    printf("\n");
    delete file;
    file=NULL;
  }
  delete q;
}


void MainObject::CheckOrphanedAudio()
{
  QDir dir(rdconfig->audioRoot());
  QStringList list=dir.entryList("??????_???.wav",QDir::Files);
  for(unsigned i=0;i<list.size();i++) {
    bool ok=false;
    list[i].left(6).toUInt(&ok);
    if(ok) {
      list[i].mid(7,3).toInt(&ok);
      if(ok) {
	QString sql=QString().sprintf("select CUT_NAME from CUTS \
                                       where CUT_NAME=\"%s\"",
				      (const char *)list[i].left(10));
	QSqlQuery *q=new QSqlQuery(sql);
	if(!q->first()) {
	  printf("  File \"%s/%s\" is orphaned.\n",
		 (const char *)rdconfig->audioRoot(),(const char *)list[i]);
	  if(dump_cuts_dir.isEmpty()) {
	    printf(
	     "  Rerun rddbcheck with the --dump-cuts-dir= switch to fix.\n\n");
	  }
	  else {
	    printf("  Move to \"%s\" (y/N)? ",(const char *)dump_cuts_dir);
	    if(UserResponse()) {
	      system(QString().sprintf("mv %s/%s %s/",
				       (const char *)rdconfig->audioRoot(),
				       (const char *)list[i],
				       (const char *)dump_cuts_dir));
	      printf("  Saved audio in \"%s/%s\"\n",(const char *)dump_cuts_dir,
		     (const char *)list[i]);
	    }
	  }
	}
	delete q;
      }
    }
  }
}


void MainObject::CleanTables(const QString &ext,QSqlQuery *table_q,
			     QSqlQuery *name_q)
{
  QString sql;
  QSqlQuery *q1;

  table_q->seek(-1);
  while(table_q->next()) {
    if(!IsTableLinked(name_q,ext,table_q->value(0).toString())) {
      printf("  Table %s is orphaned -- delete (y/N)? ",
	     (const char *)RDEscapeString(table_q->value(0).toString()));
      fflush(NULL);
      if(UserResponse()) {
	sql=QString().sprintf("drop table %s",
		  (const char *)RDEscapeString(table_q->value(0).toString()));
	q1=new QSqlQuery(sql);
	delete q1;
      }
    }
  }
}


void MainObject::CleanTables(const QString &ext,QSqlQuery *table_q)
{
  QString sql;
  QSqlQuery *q1;

  table_q->seek(-1);
  while(table_q->next()) {
    if(table_q->value(0).toString().right(ext.length())==ext) {
      printf("  Table %s is orphaned -- delete (y/N)? ",
	     (const char *)table_q->value(0).toString());
      fflush(NULL);
      if(UserResponse()) {
	sql=QString().sprintf("drop table %s",
		  (const char *)RDEscapeString(table_q->value(0).toString()));
	q1=new QSqlQuery(sql);
	delete q1;
      }
    }
  }
}


bool MainObject::IsTableLinked(QSqlQuery *name_q,const QString &ext,
			       const QString &table)
{
  QString tablename;

  if(table.right(ext.length())!=ext) {
    return true;
  }
  name_q->seek(-1);
  while(name_q->next()) {
    tablename=name_q->value(0).toString()+"_"+ext;
    tablename.replace(" ","_");
    if(tablename==table) {
      return true;
    }
  }
  return false;
}


bool MainObject::UserResponse()
{
  char c=0;

  if(check_yes) {
    printf("y\n");
    return true;
  }
  if(check_no) {
    printf("n\n");
    return false;
  }
  while((c!='y')&&(c!='Y')&&(c!='n')&&(c!='N')) {
    scanf("%c",&c);
    if((c=='y')||(c=='Y')) {
      scanf("%c",&c);
      return true;
    }
    if(c=='\n') {
      return false;
    }
  }
  scanf("%c",&c);
  return false;
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
