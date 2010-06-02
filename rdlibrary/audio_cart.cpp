// audio_cart.cpp
//
// The audio cart editor for RDLibrary.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: audio_cart.cpp,v 1.50.2.1 2010/02/09 21:43:53 cvs Exp $
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

#include <qbitmap.h>
#include <unistd.h>
#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qapplication.h>

#include <rdcddbrecord.h>
#include <rddb.h>
#include <rdwavedata.h>
#include <rd.h>
#include <rdconf.h>
#include <rdimport_audio.h>
#include <rdaudio_exists.h>
#include <rdedit_audio.h>

#include <cdripper.h>
#include <audio_cart.h>
#include <record_cut.h>
#include <globals.h>

#include <validate_cut.h>

bool import_active=false;


AudioCart::AudioCart(AudioControls *controls,RDCart *cart,QString *path,
		     bool select_cut,QWidget *parent,const char *name)
  : QWidget(parent,name)
{
  rdcart_import_metadata=true;
  rdcart_controls=controls;
  rdcart_cart=cart;
  rdcart_import_path=path;
  rdcart_select_cut=select_cut;
  rdcart_modification_allowed=lib_user->editAudio()&&cart->owner().isEmpty();

  setCaption(QString().sprintf("%u",rdcart_cart->number())+" - "+
    rdcart_cart->title());

  //
  // Generate Fonts
  //
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont line_edit_font=QFont("Helvetica",12,QFont::Normal);
  line_edit_font.setPixelSize(12);

  //
  // Progress Dialog
  //
  rdcart_progress_dialog=new QProgressDialog(this);
  rdcart_progress_dialog->setLabelText(tr("Copying audio..."));
  rdcart_progress_dialog->setCancelButton(NULL);
  rdcart_progress_dialog->setTotalSteps(10);
  rdcart_progress_dialog->setMinimumDuration(1000);

  //
  // Add Cut Button
  //
  QPushButton *add_cut_button=new QPushButton(this,"add_cut_button");
  add_cut_button->setGeometry(10,0,80,50);
  add_cut_button->setFont(button_font);
  add_cut_button->setText(tr("Add"));
  connect(add_cut_button,SIGNAL(clicked()),this,SLOT(addCutData()));

  //
  // Delete Cut Button
  //
  QPushButton *delete_cut_button=new QPushButton(this,"delete_cut_button");
  delete_cut_button->setGeometry(10,60,80,50);
  delete_cut_button->setFont(button_font);
  delete_cut_button->setText(tr("Delete"));
  connect(delete_cut_button,SIGNAL(clicked()),this,SLOT(deleteCutData()));

  //
  // Copy Cut Button
  //
  QPushButton *copy_cut_button=new QPushButton(this,"copy_cut_button");
  copy_cut_button->setGeometry(10,120,80,50);
  copy_cut_button->setFont(button_font);
  copy_cut_button->setText(tr("Copy"));
  connect(copy_cut_button,SIGNAL(clicked()),this,SLOT(copyCutData()));

  //
  // Paste Cut Button
  //
  paste_cut_button=new QPushButton(this,"paste_cut_button");
  paste_cut_button->setGeometry(10,180,80,50);
  paste_cut_button->setFont(button_font);
  paste_cut_button->setText(tr("Paste"));
  connect(paste_cut_button,SIGNAL(clicked()),this,SLOT(pasteCutData()));

  //
  // Cart Cut List
  //
  rdcart_cut_list=new RDListView(this,"rdcart_cut_list");
  rdcart_cut_list->setGeometry(100,0,430,sizeHint().height());
  rdcart_cut_list->setAllColumnsShowFocus(true);
  rdcart_cut_list->setItemMargin(5);
  rdcart_cut_list->setSorting(11);
  connect(rdcart_cut_list,
	  SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	  this,
	  SLOT(doubleClickedData(QListViewItem *,const QPoint &,int)));

  rdcart_cut_list->addColumn(tr("WT"));
  rdcart_cut_list->setColumnAlignment(0,Qt::AlignHCenter);

  rdcart_cut_list->addColumn(tr("DESCRIPTION"));
  rdcart_cut_list->setColumnAlignment(1,Qt::AlignLeft);

  rdcart_cut_list->addColumn(tr("LENGTH"));
  rdcart_cut_list->setColumnAlignment(2,Qt::AlignRight);
  rdcart_cut_list->setColumnSortType(2,RDListView::TimeSort);

  rdcart_cut_list->addColumn(tr("LAST PLAYED"));
  rdcart_cut_list->setColumnAlignment(3,Qt::AlignHCenter);

  rdcart_cut_list->addColumn(tr("# OF PLAYS"));
  rdcart_cut_list->setColumnAlignment(4,Qt::AlignHCenter);

  rdcart_cut_list->addColumn(tr("ORIGIN"));
  rdcart_cut_list->setColumnAlignment(5,Qt::AlignHCenter);

  rdcart_cut_list->addColumn(tr("OUTCUE"));
  rdcart_cut_list->setColumnAlignment(6,Qt::AlignLeft);

  rdcart_cut_list->addColumn(tr("START DATE"));
  rdcart_cut_list->setColumnAlignment(7,Qt::AlignLeft);

  rdcart_cut_list->addColumn(tr("END DATE"));
  rdcart_cut_list->setColumnAlignment(8,Qt::AlignLeft);

  rdcart_cut_list->addColumn(tr("DAYPART START"));
  rdcart_cut_list->setColumnAlignment(9,Qt::AlignLeft);

  rdcart_cut_list->addColumn(tr("DAYPART END"));
  rdcart_cut_list->setColumnAlignment(10,Qt::AlignLeft);

  rdcart_cut_list->addColumn(tr("NAME"));
  rdcart_cut_list->setColumnAlignment(11,Qt::AlignLeft);

  rdcart_cut_list_label=new QLabel(rdcart_cut_list,tr("Cuts"),
				   this,"rdcartcut_list_label");
  rdcart_cut_list_label->setGeometry(105,345,430,22);
  rdcart_cut_list_label->setFont(QFont("Helvetica",16,QFont::Bold));

  RefreshList();

  //
  // Record Cut Button
  //
  QBitmap *pix=new QBitmap(QSize(80,50));
  QPainter *p=new QPainter(pix);
  QFontMetrics *m=new QFontMetrics(button_font);
  p->eraseRect(0,0,80,50);
  p->setPen(QColor(color1));
  p->setFont(button_font);
  p->drawText((80-m->width(tr("Cut Info")))/2,20,tr("Cut Info"));
  p->moveTo(10,24);
  p->lineTo(70,24);
  p->drawText((80-m->width(tr("Record")))/2,38,tr("Record"));
  p->end();
  QPushButton *record_cut_button=new QPushButton(this,"record_cut_button");
  record_cut_button->setGeometry(550,0,80,50);
  record_cut_button->setPixmap(*pix);
  connect(record_cut_button,SIGNAL(clicked()),this,SLOT(recordCutData()));
  
  //
  // Send to (external) Editor Button (ex: Audacity)
  //
  QPushButton *ext_editor_cut_button=
    new QPushButton(this,"ext_editor_cut_button");
  ext_editor_cut_button->setGeometry(550,60,80,50);
  ext_editor_cut_button->setFont(button_font);
  ext_editor_cut_button->setText(tr("Edit\nAudio"));
  connect(ext_editor_cut_button,SIGNAL(clicked()),
	  this,SLOT(extEditorCutData()));
  int yoffset=60;
  if((!rdlibrary_conf->enableEditor())||
     rdstation_conf->editorPath().isEmpty()) {
    ext_editor_cut_button->hide();
    yoffset=0;
  }

  //
  // Edit Cut Button
  //
  QPushButton *edit_cut_button=new QPushButton(this,"edit_cut_button");
  edit_cut_button->setGeometry(550,60+yoffset,80,50);
  edit_cut_button->setFont(button_font);
  edit_cut_button->setText(tr("Edit\nMarkers"));
  connect(edit_cut_button,SIGNAL(clicked()),this,SLOT(editCutData()));

  //
  // Import Cut Button
  //
  p=new QPainter(pix);
  m=new QFontMetrics(button_font);
  p->eraseRect(0,0,80,50);
  p->setPen(QColor(color1));
  p->setFont(button_font);
  p->drawText((80-m->width(tr("Import")))/2,20,tr("Import"));
  p->moveTo(10,24);
  p->lineTo(70,24);
  p->drawText((80-m->width(tr("Export")))/2,38,tr("Export"));
  p->end();
  QPushButton *import_cut_button=new QPushButton(this,"import_cut_button");
  import_cut_button->setPixmap(*pix);
  import_cut_button->setGeometry(550,120+yoffset,80,50);
  connect(import_cut_button,SIGNAL(clicked()),this,SLOT(importCutData()));

  //
  // Rip Cut Button
  //
  QPushButton *rip_cut_button=new QPushButton(this,"rip_cut_button");
  rip_cut_button->setGeometry(550,180+yoffset,80,50);
  rip_cut_button->setFont(button_font);
  rip_cut_button->setText(tr("Rip CD"));
  connect(rip_cut_button,SIGNAL(clicked()),this,SLOT(ripCutData()));

  //
  // Set Control Perms
  //
  add_cut_button->setEnabled(rdcart_modification_allowed);
  delete_cut_button->setEnabled(rdcart_modification_allowed);
  if((cut_clipboard==NULL)||(!rdcart_modification_allowed)) {
    paste_cut_button->setDisabled(true);
  }
  rip_cut_button->setEnabled(rdcart_modification_allowed);
  import_cut_button->setEnabled(rdcart_modification_allowed);
  ext_editor_cut_button->setEnabled(rdcart_modification_allowed);
}


QSize AudioCart::sizeHint() const
{
  return QSize(640,290);
} 


QSizePolicy AudioCart::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void AudioCart::addCutData()
{
  QString next_name=QString().
    sprintf("%06d_%03d",
	    rdcart_cart->number(),
	    rdcart_cart->addCut(rdlibrary_conf->defaultFormat(),
				rdlibrary_conf->
				defaultSampleRate(),
				rdlibrary_conf->defaultBitrate(),
				rdlibrary_conf->defaultChannels()));
  RDListViewItem *item=new RDListViewItem(rdcart_cut_list);
  item->setText(11,next_name);
  UpdateCutCount();
  RefreshLine(item);
  rdcart_cut_list->setSelected(item,true);
  rdcart_cut_list->ensureItemVisible(item);
  disk_gauge->update();
  emit cartDataChanged();
}


void AudioCart::deleteCutData()
{
  QString filename;
  QString str;
  QListViewItem *item=rdcart_cut_list->selectedItem();
  if(item==NULL) {
    return;
  }
  str=QString(tr("Are you sure you want to delete"));
  switch(QMessageBox::question(this,tr("Delete Cut"),
			      QString().
			      sprintf("%s \"%s\"?",(const char *)str,
				      (const char *)item->text(1)),
			      QMessageBox::Yes,QMessageBox::No)) {
      case QMessageBox::No:
      case QMessageBox::NoButton:
	return;

      default:
	break;
  }
  QString sql=QString().sprintf("select CUT_NAME from RECORDINGS where \
                                 CUT_NAME=\"%s\"",
				(const char *)item->text(11));
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    if(QMessageBox::warning(this,tr("RDCatch Event Exists"),
			    tr("This cut is used in one or more RDCatch events!\nDo you still want to delete it?"),QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
      delete q;
      return;
    }
  }
  delete q;
  if(cut_clipboard!=NULL) {
    if(item->text(11)==cut_clipboard->cutName()) {
      if(QMessageBox::question(this,tr("Empty Clipboard"),
			      tr("Deleting this cut will also empty the clipboard.\nDo you still want to proceed?"),QMessageBox::Yes,QMessageBox::No)==
	 QMessageBox::No) {
	return;
      }
      delete cut_clipboard;
      cut_clipboard=NULL;
      paste_cut_button->setDisabled(true);
    }
  }
  rdcart_cart->removeCut(item->text(11));
  UpdateCutCount();
  QListViewItem *next_item=item->nextSibling();
  if(next_item==NULL) {
    next_item=rdcart_cut_list->lastItem();
  }
  if(next_item!=NULL) {
    rdcart_cut_list->setSelected(next_item,true);
  }
  rdcart_cut_list->removeItem(item);
  rdcart_cart->updateLength(rdcart_controls->enforce_length_box->isChecked(),
			    QTime().msecsTo(rdcart_controls->
					    forced_length_edit->time()));
  rdcart_cart->resetRotation();
  disk_gauge->update();
  emit cartDataChanged();
}


void AudioCart::copyCutData()
{
  if(rdcart_cut_list->currentItem()==0) {
    return;
  }
  if(cut_clipboard!=NULL) {
    delete cut_clipboard;
  }
  cut_clipboard=new RDCut(rdcart_cut_list->currentItem()->text(11));
  paste_cut_button->setEnabled(rdcart_modification_allowed);
}

void AudioCart::extEditorCutData()
{
  if(rdcart_cut_list->currentItem()==0) {
    return;
  }

  QString cmd=rdstation_conf->editorPath();
  cmd.replace("%f",RDCut::pathName(rdcart_cut_list->currentItem()->text(11)));
  // FIXME: other replace commands to match: lib/rdcart_dialog.cpp editorData()
  //        These substitions should be documented (maybe a text file),
  //            ex: %f = cart_cut filename
  //        and possibly also add some tooltips with help advice

  if(fork()==0) {
    system(cmd+" &");
    exit(0);
  }
}

void AudioCart::pasteCutData()
{
  QListViewItem *item=rdcart_cut_list->currentItem();
  if(item==0) {
    return;
  }
  if(!cut_clipboard->exists()) {
    QMessageBox::information(this,tr("Clipboard Empty"),
			     tr("Clipboard is currently empty."));
    delete cut_clipboard;
    cut_clipboard=NULL;
    paste_cut_button->setDisabled(true);
    return;
  }
  if(QFile::exists(RDCut::pathName(item->text(11)))) {
    if(QMessageBox::warning(this,tr("Audio Exists"),
			    tr("This will overwrite the existing recording.\nDo you want to proceed?"),
			    QMessageBox::Yes,
			    QMessageBox::No)==QMessageBox::No) {
      return;
    }
  }
  cut_clipboard->connect(this,SLOT(copyProgressData(const QVariant &)));
  cut_clipboard->copyTo(item->text(11));
  cut_clipboard->disconnect(this,SLOT(copyProgressData(const QVariant &)));
  rdcart_cart->updateLength(rdcart_controls->enforce_length_box->isChecked(),
			    QTime().msecsTo(rdcart_controls->
					    forced_length_edit->time()));
  rdcart_cart->resetRotation();
  disk_gauge->update();
  emit cartDataChanged();
  RefreshList();
}


void AudioCart::editCutData()
{
  RDListViewItem *item=(RDListViewItem *)rdcart_cut_list->selectedItem();

  if(item==NULL) {
    return;
  }
  QString cutname=item->text(11);
  if(!RDAudioExists(cutname)) {
    return;
  }
  RDEditAudio *edit=
    new RDEditAudio(rdcart_cart,cutname,rdcae,lib_user,
		    rdlibrary_conf->outputCard(),rdlibrary_conf->outputPort(),
		    rdlibrary_conf->tailPreroll(),
		    rdlibrary_conf->trimThreshold(),this);
  if(edit->exec()!=-1) {
    emit cartDataChanged();
    rdcart_cart->updateLength(rdcart_controls->enforce_length_box->isChecked(),
			      QTime().msecsTo(rdcart_controls->
					      forced_length_edit->time()));
    RefreshLine(item);
  }
  delete edit;
}


void AudioCart::recordCutData()
{
  QString cutname;
  RDListViewItem *item=(RDListViewItem *)rdcart_cut_list->selectedItem();

  if(item==NULL) {
    return;
  }
  cutname=item->text(11);
  RecordCut *cut=new RecordCut(rdcart_cart,cutname,this,"cut");
  cut->exec();
  delete cut;
  if(cut_clipboard==NULL) {
    paste_cut_button->setDisabled(true);
  }
  disk_gauge->update();
  emit cartDataChanged();
  rdcart_cart->updateLength(rdcart_controls->enforce_length_box->isChecked(),
			    QTime().msecsTo(rdcart_controls->
					    forced_length_edit->time()));
  RefreshLine(item);
}


void AudioCart::ripCutData()
{
  int track;
  QString cutname;
  RDListViewItem *item=(RDListViewItem *)rdcart_cut_list->selectedItem();

  if(item==NULL) {
    return;
  }
  cutname=item->text(11);
  RDCddbRecord *rec=new RDCddbRecord();
  CdRipper *ripper=new CdRipper(cutname,rec,rdlibrary_conf);
  if((track=ripper->exec())>=0) {
    if(rdcart_controls->title_edit->text().isEmpty()||
       (rdcart_controls->title_edit->text()==tr("[new cart]"))) {
      rdcart_controls->title_edit->setText(rec->trackTitle(track));
    }
    if(rdcart_controls->artist_edit->text().isEmpty()) {
      rdcart_controls->artist_edit->setText(rec->discArtist());
    }
    if(rdcart_controls->album_edit->text().isEmpty()) {
      rdcart_controls->album_edit->setText(rec->discAlbum());
    }
    RDCut *cut=new RDCut(cutname);
    cut->setIsrc(rec->isrc(track));
    delete cut;
  }
  if(cut_clipboard==NULL) {
    paste_cut_button->setDisabled(true);
  }
  emit cartDataChanged();
  delete ripper;
  delete rec;
  disk_gauge->update();
  rdcart_cart->updateLength(rdcart_controls->enforce_length_box->isChecked(),
			    QTime().msecsTo(rdcart_controls->
					    forced_length_edit->time()));
  RefreshLine(item);
}


void AudioCart::importCutData()
{
  QString cutname;
  RDWaveData wavedata;
  RDListViewItem *item=(RDListViewItem *)rdcart_cut_list->selectedItem();

  if(item==NULL) {
    return;
  }
  cutname=item->text(11);
  RDSettings settings;
  rdlibrary_conf->getSettings(&settings);
  RDImportAudio *import=new RDImportAudio(cutname,rdcart_import_path,
					  &settings,&rdcart_import_metadata,
					  &wavedata,cut_clipboard,
					  rdstation_conf,&import_active,
					  lib_config);
  import->enableAutotrim(rdlibrary_conf->defaultTrimState());
  import->setAutotrimLevel(rdlibrary_conf->trimThreshold());
  import->enableNormalization(rdlibrary_conf->ripperLevel()!=0);
  import->setNormalizationLevel(rdlibrary_conf->ripperLevel());
  if(import->exec(true,true)==0) {
    if(rdcart_controls->title_edit->text().isEmpty()||
       (rdcart_controls->title_edit->text()==tr("[new cart]"))) {
      rdcart_controls->title_edit->setText(wavedata.title());
    }
    if(rdcart_controls->artist_edit->text().isEmpty()) {
      rdcart_controls->artist_edit->setText(wavedata.artist());
    }
    if(rdcart_controls->album_edit->text().isEmpty()) {
      rdcart_controls->album_edit->setText(wavedata.album());
    }

    if(rdcart_controls->year_edit->text().isEmpty()&&
       wavedata.releaseYear()>0) {
      rdcart_controls->year_edit->
	setText(QString().sprintf("%d",wavedata.releaseYear()));
    }
    if(rdcart_controls->label_edit->text().isEmpty()) {
      rdcart_controls->label_edit->setText(wavedata.label());
    }
    if(rdcart_controls->client_edit->text().isEmpty()) {
      rdcart_controls->client_edit->setText(wavedata.client());
    }
    if(rdcart_controls->agency_edit->text().isEmpty()) {
      rdcart_controls->agency_edit->setText(wavedata.agency());
    }
    if(rdcart_controls->publisher_edit->text().isEmpty()) {
      rdcart_controls->publisher_edit->setText(wavedata.publisher());
    }
    if(rdcart_controls->composer_edit->text().isEmpty()) {
      rdcart_controls->composer_edit->setText(wavedata.composer());
    }
    if(rdcart_controls->user_defined_edit->text().isEmpty()) {
      rdcart_controls->user_defined_edit->setText(wavedata.userDefined());
    }
  }
  if(cut_clipboard==NULL) {
    paste_cut_button->setDisabled(true);
  }
  emit cartDataChanged();
  delete import;
  disk_gauge->update();
  rdcart_cart->updateLength(rdcart_controls->enforce_length_box->isChecked(),
			    QTime().msecsTo(rdcart_controls->
					    forced_length_edit->time()));
  RefreshLine(item);
}


void AudioCart::doubleClickedData(QListViewItem *,const QPoint &,int)
{
  recordCutData();
}


void AudioCart::copyProgressData(const QVariant &step)
{
  rdcart_progress_dialog->setProgress(step.toInt());
  qApp->processEvents();
}


void AudioCart::RefreshList()
{
  RDSqlQuery *q;
  QString sql;
  RDListViewItem *l;
  unsigned total_length=0;
  QDateTime current_datetime=
    QDateTime(QDate::currentDate(),QTime::currentTime());
  int pass=0;
  bool err=false;

  UpdateCutCount();
  rdcart_cut_list->clear();
  sql=QString().sprintf("select WEIGHT,DESCRIPTION,LENGTH,LAST_PLAY_DATETIME,\
                         PLAY_COUNTER,ORIGIN_DATETIME,ORIGIN_NAME,OUTCUE,\
                         CUT_NAME,LENGTH,EVERGREEN,START_DATETIME,\
                         END_DATETIME,START_DAYPART,END_DAYPART,MON,TUE,WED,\
                         THU,FRI,SAT,SUN from CUTS where CART_NUMBER=%d",
			rdcart_cart->number());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    l=new RDListViewItem(rdcart_cut_list);
    l->setText(0,q->value(0).toString());
    l->setText(1,q->value(1).toString());
    l->setText(2,RDGetTimeLength(q->value(2).toUInt()));
    if (q->value(0) == 0){// zero weight
      l->setBackgroundColor(RD_CART_ERROR_COLOR);
      if(pass==0) {
	err=true;
      }
    } else {
      switch(ValidateCut(q,9,RDCart::NeverValid,current_datetime)) {
      case RDCart::NeverValid:
	l->setBackgroundColor(RD_CART_ERROR_COLOR);
	if(pass==0) {
	  err=true;
	}
	break;
	
      case RDCart::ConditionallyValid:
	if((!q->value(10).isNull())&&
	   (q->value(10).toDateTime()<current_datetime)) {
	  l->setBackgroundColor(RD_CART_ERROR_COLOR);
	}
	else {
	  l->setBackgroundColor(RD_CART_CONDITIONAL_COLOR);
	}
	if(pass==0) {
	  err=true;
	}
	break;
	
      case RDCart::EvergreenValid:
	l->setBackgroundColor(RD_CART_EVERGREEN_COLOR);
	if(pass==0) {
	  err=true;
	}
	break;
	
      case RDCart::AlwaysValid:
	break;
      }
    }
    if(q->value(4).toUInt()>0) {
      l->setText(3,q->value(3).toDateTime().toString("M/d/yy"));
    }
    else {
      l->setText(3,tr("Never"));
    }
    l->setText(4,q->value(4).toString());
    if(!q->value(5).toDateTime().isNull()) {
      l->setText(5,q->value(6).toString()+" - "+
		 q->value(5).toDateTime().toString("M/d/yy hh:mm:ss"));
    }
    l->setText(6,q->value(7).toString());
    if(!q->value(11).toDateTime().isNull()) {
      l->setText(7,q->value(11).toDateTime().toString("M/d/yyyy hh:mm:ss"));
    }
    else {
      l->setText(7,tr("None"));
    }
    if(!q->value(12).toDateTime().isNull()) {
      l->setText(8,q->value(12).toDateTime().toString("M/d/yyyy hh:mm:ss"));
    }
    else {
      l->setText(8,tr("None"));
    }
    if(!q->value(14).isNull()) {
      l->setText(9,q->value(13).toTime().toString("hh:mm:ss"));
      l->setText(10,q->value(14).toTime().toString("hh:mm:ss"));
    }
    else {
      l->setText(9,tr("None"));
      l->setText(10,tr("None"));
    }
    l->setText(11,q->value(8).toString());
    total_length+=q->value(2).toUInt();
    pass++;
  }
  if(q->size()>0) {
    rdcart_average_length=total_length/q->size();
  }
  else {
    rdcart_average_length=0;
  }
  delete q;
  if(((l=(RDListViewItem *)rdcart_cut_list->firstChild())!=NULL)&&
     ((!err)||rdcart_select_cut)) {
    rdcart_cut_list->setSelected(l,true);
    rdcart_select_cut=false;
  }
}


void AudioCart::RefreshLine(RDListViewItem *item)
{
  QString sql;
  unsigned total_length=0;
  QDateTime current_datetime=
    QDateTime(QDate::currentDate(),QTime::currentTime());
  QString cut_name=item->text(11);
  sql=QString().sprintf("select WEIGHT,DESCRIPTION,LENGTH,LAST_PLAY_DATETIME,\
                         PLAY_COUNTER,ORIGIN_DATETIME,ORIGIN_NAME,OUTCUE,\
                         CUT_NAME,LENGTH,EVERGREEN,START_DATETIME,\
                         END_DATETIME,START_DAYPART,END_DAYPART,MON,TUE,WED,\
                         THU,FRI,SAT,SUN from CUTS \
                         where CART_NUMBER=%d && CUT_NAME=\"%s\"",
			rdcart_cart->number(),
			(const char *)cut_name);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    item->setText(0,q->value(0).toString());
    item->setText(1,q->value(1).toString());
    item->setText(2,RDGetTimeLength(q->value(2).toUInt()));
    if (q->value(0) == 0){ //zero weight
      	  item->setBackgroundColor(RD_CART_ERROR_COLOR);
    } else {
      switch(ValidateCut(q,9,RDCart::NeverValid,current_datetime)) {
      case RDCart::NeverValid:
	item->setBackgroundColor(RD_CART_ERROR_COLOR);
	break;
	
      case RDCart::ConditionallyValid:
	if((!q->value(11).isNull())&&
	   (q->value(11).toDateTime()<current_datetime)) {
	  item->setBackgroundColor(RD_CART_ERROR_COLOR);
	}
	else {
	  item->setBackgroundColor(RD_CART_CONDITIONAL_COLOR);
	}
	break;
	
      case RDCart::EvergreenValid:
	item->setBackgroundColor(RD_CART_EVERGREEN_COLOR);
	break;
	
      case RDCart::AlwaysValid:
	item->setBackgroundColor(backgroundColor());
	break;
      }
    }
    if(q->value(4).toUInt()>0) {
      item->setText(3,q->value(3).toDateTime().toString("M/d/yy"));
    }
    else {
      item->setText(3,tr("Never"));
    }
    item->setText(4,q->value(4).toString());
    if(!q->value(5).toDateTime().isNull()) {
      item->setText(5,q->value(6).toString()+" - "+
		 q->value(5).toDateTime().toString("M/d/yy hh:mm:ss"));
    }
    item->setText(6,q->value(7).toString());
    if(!q->value(11).toDateTime().isNull()) {
      item->setText(7,q->value(11).toDateTime().toString("M/d/yyyy hh:mm:ss"));
    }
    else {
      item->setText(7,tr("None"));
    }
    if(!q->value(12).toDateTime().isNull()) {
      item->setText(8,q->value(12).toDateTime().toString("M/d/yyyy hh:mm:ss"));
    }
    else {
      item->setText(8,tr("None"));
    }
    if(!q->value(14).isNull()) {
      item->setText(9,q->value(13).toTime().toString("hh:mm:ss"));
      item->setText(10,q->value(14).toTime().toString("hh:mm:ss"));
    }
    else {
      item->setText(9,tr("None"));
      item->setText(10,tr("None"));
    }
    item->setText(11,q->value(8).toString());
    total_length+=q->value(2).toUInt();
  }
  if(q->size()>0) {
    rdcart_average_length=total_length/q->size();
  }
  else {
    rdcart_average_length=0;
  }
  delete q;
}


void AudioCart::UpdateCutCount()
{
  QString str;

  if(rdcart_cart->cutQuantity()==1) {
    rdcart_cut_list_label->setText(tr("1 Cut"));
  }
  else {
    str=QString(tr("Cuts"));
    rdcart_cut_list_label->setText(QString().sprintf("%d %s",
						  rdcart_cart->cutQuantity(),
						     (const char *)str));
  }
}
