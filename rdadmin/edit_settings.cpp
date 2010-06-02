// edit_station.cpp
//
// Edit a Rivendell Workstation Configuration
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_settings.cpp,v 1.1.2.2 2009/05/21 17:32:17 cvs Exp $
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

#include <qdialog.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qprogressdialog.h>
#include <qapplication.h>

#include <rdescape_string.h>
#include <rddb.h>
#include <rdconf.h>

#include <edit_settings.h>
#include <globals.h>

EditSettings::EditSettings(QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);

  QFont normal_font=QFont("Helvetica",12,QFont::Normal);
  normal_font.setPixelSize(12);

  y_pos=0;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  edit_system=new RDSystem();

  setCaption(tr("System-Wide Settings"));

  edit_system=new RDSystem();

  //
  // Allow Duplicate Cart Titles Box
  //
  edit_duplicate_carts_box=new QCheckBox(this,"edit_duplicate_carts_box");
  edit_duplicate_carts_box->setGeometry(20,10,15,15);
  QLabel *label=new QLabel(edit_duplicate_carts_box,
			   tr("Allow Duplicate Cart Titles"),this,
			   "edit_duplicate_carts_box");
  label->setGeometry(40,8,sizeHint().width()-50,20);
  label->setFont(font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Maximum POST Size
  //
  edit_maxpost_spin=new QSpinBox(this,"edit_maxpost_spin");
  edit_maxpost_spin->setGeometry(200,32,60,20);
  edit_maxpost_spin->setRange(1,1000);
  label=new QLabel(edit_maxpost_spin,
		   tr("Maximum Remote Post Length:"),this,
		   "edit_maxpost_label");
  label->setGeometry(10,32,185,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  label=new QLabel(tr("Mbytes"),this,"edit_maxpost_unit");
  label->setGeometry(265,32,60,20);
  label->setFont(font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Duplicate List
  //
  edit_duplicate_label=new RDLabel(this,"edit_duplicate_label");
  edit_duplicate_label->setText(tr("The following duplicate titles must be corrected before \"Allow Duplicate Values\" can be turned off."));
  edit_duplicate_label->setWordWrapEnabled(true);
  edit_duplicate_label->setGeometry(15,62,sizeHint().width()-30,50);
  edit_duplicate_label->setFont(normal_font);
  edit_duplicate_label->hide();
  edit_duplicate_list=new QListView(this);
  edit_duplicate_list->setGeometry(10,122,sizeHint().width()-20,200);
  edit_duplicate_list->setItemMargin(5);
  edit_duplicate_list->setAllColumnsShowFocus(true);
  edit_duplicate_list->addColumn(tr("CART"));
  edit_duplicate_list->setColumnAlignment(0,AlignCenter);
  edit_duplicate_list->addColumn(tr("TITLE"));
  edit_duplicate_list->setColumnAlignment(1,AlignLeft);
  edit_duplicate_list->hide();
  edit_save_button=new QPushButton(this,"save_button");
  edit_save_button->
    setGeometry(sizeHint().width()-85,327,70,25);
  edit_save_button->setFont(normal_font);
  edit_save_button->setText(tr("&Save List"));
  connect(edit_save_button,SIGNAL(clicked()),this,SLOT(saveData()));
  edit_save_button->hide();

  //
  //  Ok Button
  //
  edit_ok_button=new QPushButton(this,"ok_button");
  edit_ok_button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,
				  80,50);
  edit_ok_button->setFont(font);
  edit_ok_button->setText(tr("&OK"));
  connect(edit_ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  edit_cancel_button=new QPushButton(this,"cancel_button");
  edit_cancel_button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
				  80,50);
  edit_cancel_button->setFont(font);
  edit_cancel_button->setText(tr("&Cancel"));
  connect(edit_cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));

  edit_duplicate_carts_box->setChecked(edit_system->allowDuplicateCartTitles());
  edit_maxpost_spin->setValue(edit_system->maxPostLength()/1000000);
}


EditSettings::~EditSettings()
{
  delete edit_system;
  delete edit_duplicate_carts_box;
  delete edit_duplicate_label;
  delete edit_duplicate_list;
}


QSize EditSettings::sizeHint() const
{
  return QSize(320,122+y_pos);
} 


QSizePolicy EditSettings::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void EditSettings::saveData()
{
  QString filename=RDGetHomeDir();
  filename=QFileDialog::getSaveFileName(filename,"Text Files *.txt",this);
  if(filename.isNull()) {
    return;
  }
  if(filename.find(".")<0) {
    filename+=".txt";
  }
  if(QFile::exists(filename)) {
    if(QMessageBox::question(this,tr("File Exists"),
			     tr("The file \"")+filename+
			     tr("\" exists.\n\nOverwrite?"),
			     QMessageBox::Yes,QMessageBox::No)!=
       QMessageBox::Yes) {
      return;
    }
  }
  FILE *f=fopen(filename,"w");
  if(f==NULL) {
    QMessageBox::warning(this,tr("File Error"),
			 tr("Unable to write file \""+filename+"\"."));
    return;
  }
  QDateTime dt=QDateTime(QDate::currentDate(),QTime::currentTime());
  fprintf(f,"Rivendell Duplicate Cart Title List\n");
  fprintf(f,"Generated %s\n",
	  (const char *)dt.toString("hh:mm:ss - MM/dd/yyyy"));
  fprintf(f,"\n");
  fprintf(f,"Cart    Title\n");
  fprintf(f,"----    -----\n");
  QListViewItem *item=edit_duplicate_list->firstChild();
  while(item!=NULL) {
    fprintf(f,"%s  %s\n",(const char *)item->text(0),
	    (const char *)item->text(1));
    item=item->nextSibling();
  }
  fclose(f);
}


void EditSettings::okData()
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  std::map<unsigned,QString> dups;

  if(edit_duplicate_carts_box->isChecked()!=
     edit_system->allowDuplicateCartTitles()) {
    QLabel *msg=new QLabel(this);
    QProgressDialog *pd=new QProgressDialog(this);
    pd->setLabel(msg);
    pd->setCancelButton(NULL);
    pd->setMinimumDuration(2);
    if(!edit_duplicate_carts_box->isChecked()) {
      //
      // Check for dups
      //
      msg->setText(tr("Checking the Library for duplicates."));
      sql="select NUMBER,TITLE from CART order by NUMBER";
      q=new RDSqlQuery(sql);
      int count=0;
      int step=0;
      int step_size=q->size()/10;
      pd->setProgress(0,10);
      while(q->next()) {
	sql=QString().sprintf("select NUMBER from CART \
                           where (TITLE=\"%s\")&&(NUMBER!=%u)",
			      (const char *)RDEscapeString(q->value(1).toString()),
			      q->value(0).toUInt());
	q1=new RDSqlQuery(sql);
	while(q1->next()) {
	  dups[q1->value(0).toUInt()]=q->value(1).toString();
	}
	delete q1;
	count++;
	if(count>=step_size) {
	  pd->setProgress(++step);
	  count=0;
	  qApp->processEvents();
	}
      }
      delete q;
      pd->reset();
      if(dups.size()>0) {
	QListViewItem *item;
	y_pos=305;
	setMinimumWidth(sizeHint().width());
	setMaximumWidth(sizeHint().width());
	setMinimumHeight(sizeHint().height());
	setMaximumHeight(sizeHint().height());
	edit_duplicate_carts_box->setChecked(true);
	edit_duplicate_label->show();
	edit_duplicate_list->show();
	edit_save_button->show();
	edit_duplicate_list->clear();
	edit_ok_button->setGeometry(sizeHint().width()-180,
				    sizeHint().height()-60,
				    80,50);
	edit_cancel_button->
	  setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
	for(std::map<unsigned,QString>::const_iterator ci=dups.begin();
	    ci!=dups.end();ci++) {
	  item=new QListViewItem(edit_duplicate_list);
	  item->setText(0,QString().sprintf("%06u",ci->first));
	  item->setText(1,ci->second);
	}
	return;
      }

      //
      // All ok -- make the change
      //
      sql="alter table CART drop index TITLE_IDX";
      q=new RDSqlQuery(sql);
      delete q;
      sql="alter table CART modify column TITLE char(255) unique";
      q=new RDSqlQuery(sql);
      delete q;
      edit_system->setAllowDuplicateCartTitles(false);
    }
    else {
      sql="alter table CART drop index TITLE";
      q=new RDSqlQuery(sql);
      delete q;
      sql="alter table CART modify column TITLE char(255)";
      q=new RDSqlQuery(sql);
      delete q;
      sql="alter table CART add index TITLE_IDX(TITLE)";
      q=new RDSqlQuery(sql);
      delete q;
      edit_system->setAllowDuplicateCartTitles(true);
    }
    delete pd;
  }
  edit_system->setMaxPostLength(edit_maxpost_spin->value()*1000000);
    
  done(0);
}


void EditSettings::cancelData()
{
  done(-1);
}


void EditSettings::BuildDuplicatesList(std::map<unsigned,QString> *dups)
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;

  sql="select NUMBER,TITLE from CART order by NUMBER";
  q=new RDSqlQuery(sql);
  while(q->next()) {
    sql=QString().sprintf("select NUMBER from CART \
                           where (TITLE=\"%s\")&&(NUMBER!=%u)",
			  (const char *)RDEscapeString(q->value(1).toString()),
			  q->value(0).toUInt());
    q1=new RDSqlQuery(sql);
    while(q1->next()) {
      (*dups)[q1->value(0).toUInt()]=q->value(1).toString();
    }
    delete q1;
  }
  delete q;
}
