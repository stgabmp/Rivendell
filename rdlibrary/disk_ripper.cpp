// disk_ripper.cpp
//
// CD Ripper Dialog for Rivendell.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: disk_ripper.cpp,v 1.23.2.4 2009/09/04 01:29:50 cvs Exp $
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
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <linux/cdrom.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qcheckbox.h>

#include <rddb.h>
#include <rd.h>
#include <rdconf.h>
#include <rdwavefile.h>
#include <rdcart.h>
#include <rdcut.h>
#include <rdcut_dialog.h>
#include <rdlist_groups.h>
#include <rdgroup.h>

#include <disk_ripper.h>
#include <globals.h>


DiskRipper::DiskRipper(QString *filter,QString *group,
		       QWidget *parent,const char *name) 
  : QDialog(parent,name)
{
  rip_filter_text=filter;
  rip_group_text=group;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());

  //
  // Generate Fonts
  //
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont label_font=QFont("Helvetica",12,QFont::Bold);
  label_font.setPixelSize(12);

  setCaption(tr("Rip Disk"));

  //
  // The CDROM Drive
  //
  rip_cdrom=new RDCdPlayer(this,"rip_cdrom");
  connect(rip_cdrom,SIGNAL(ejected()),this,SLOT(ejectedData()));
  connect(rip_cdrom,SIGNAL(mediaChanged()),this,SLOT(mediaChangedData()));
  connect(rip_cdrom,SIGNAL(played(int)),this,SLOT(playedData(int)));
  connect(rip_cdrom,SIGNAL(stopped()),this,SLOT(stoppedData()));
  rip_cdrom->setDevice(rdlibrary_conf->ripperDevice());
  rip_cdrom->open();

  //
  // CDDB Stuff
  //
  rip_cddb_lookup=new RDCddbLookup(this,"rip_cddb_lookup");
  connect(rip_cddb_lookup,SIGNAL(done(RDCddbLookup::Result)),
	  this,SLOT(cddbDoneData(RDCddbLookup::Result)));

  //
  // Artist Label
  //
  QLabel *label=new QLabel(tr("Artist:"),this,"artist_label");
  label->setGeometry(10,10,50,18);
  label->setFont(label_font);
  label->setAlignment(AlignRight|AlignVCenter);
  rip_artist_edit=new QLineEdit(this,"rip_artist_edit");
  rip_artist_edit->setReadOnly(true);

  //
  // Album Edit
  //
  label=new QLabel(tr("Album:"),this,"album_label");
  label->setGeometry(10,32,50,18);
  label->setFont(label_font);
  label->setAlignment(AlignRight|AlignVCenter);
  rip_album_edit=new QLineEdit(this,"rip_album_edit");
  rip_album_edit->setReadOnly(true);

  //
  // Other Edit
  //
  label=new QLabel(tr("Other:"),this,"other_label");
  label->setGeometry(10,54,50,16);
  label->setFont(label_font);
  label->setAlignment(AlignRight);
  rip_other_edit=new QTextEdit(this,"rip_other_edit");
  rip_other_edit->setReadOnly(true);

  //
  // Apply FreeDB Check Box
  //
  rip_apply_box=new QCheckBox(this,"rip_apply_box");
  rip_apply_box->setChecked(true);
  rip_apply_box->setDisabled(true);
  rip_apply_label=new QLabel(rip_apply_box,tr("Apply FreeDB Values to Carts"),
		   this,"rip_apply_label");
  rip_apply_label->setFont(label_font);
  rip_apply_label->setAlignment(AlignLeft);
  rip_apply_box->setChecked(false);
  rip_apply_label->setDisabled(true);

  //
  // Track List
  //
  rip_track_list=new QListView(this,"rip_track_list");
  rip_track_list->setAllColumnsShowFocus(true);
  rip_track_list->setItemMargin(5);
  rip_track_list->setSorting(-1);
  connect(rip_track_list,
	  SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	  this,
	  SLOT(doubleClickedData(QListViewItem *,const QPoint &,int)));
  label=new QLabel(rip_track_list,tr("Tracks"),this,"name_label");
  label->setGeometry(10,140,100,14);
  label->setFont(label_font);
  rip_track_list->addColumn(tr("TRACK"));
  rip_track_list->setColumnAlignment(0,Qt::AlignHCenter);
  rip_track_list->addColumn(tr("LENGTH"));
  rip_track_list->setColumnAlignment(1,Qt::AlignRight);
  rip_track_list->addColumn(tr("TITLE"));
  rip_track_list->setColumnAlignment(2,Qt::AlignLeft);
  rip_track_list->addColumn(tr("OTHER"));
  rip_track_list->setColumnAlignment(3,Qt::AlignLeft);
  rip_track_list->addColumn(tr("TYPE"));
  rip_track_list->setColumnAlignment(4,Qt::AlignLeft);
  rip_track_list->addColumn(tr("CUT"));
  rip_track_list->setColumnAlignment(5,Qt::AlignLeft);

  //
  // Progress Bars
  //
//  rip_disk_bar=new QProgressBar(this,"rip_diskbar_label");
//  rip_diskbar_label=new QLabel(tr("Disk Progress"),this,"rip_diskbar_label");
//  rip_diskbar_label->setFont(label_font);
//  rip_diskbar_label->setAlignment(AlignLeft|AlignVCenter);
//  rip_diskbar_label->setDisabled(true);
  rip_track_bar=new QProgressBar(100,this,"rip_track_bar");
  rip_trackbar_label=new QLabel(tr("Ripping Track"),
				this,"rip_trackbar_label");
//  rip_trackbar_label=new QLabel(tr("Track Progress"),
//				this,"rip_trackbar_label");
  rip_trackbar_label->setFont(label_font);
  rip_trackbar_label->setAlignment(AlignLeft|AlignVCenter);
  rip_trackbar_label->setDisabled(true);

  //
  // Progress Bar Timer
  //
  rip_bar_timer=new QTimer(this,"rip_bar_timer");
  connect(rip_bar_timer,SIGNAL(timeout()),this,SLOT(barTimerData()));

  //
  // Eject Button
  //
  rip_eject_button=new RDTransportButton(RDTransportButton::Eject,
					this,"close_button");
  connect(rip_eject_button,SIGNAL(clicked()),this,SLOT(ejectButtonData()));
  
  //
  // Play Button
  //
  rip_play_button=new RDTransportButton(RDTransportButton::Play,
					this,"close_button");
  connect(rip_play_button,SIGNAL(clicked()),this,SLOT(playButtonData()));
  
  //
  // Stop Button
  //
  rip_stop_button=new RDTransportButton(RDTransportButton::Stop,
					this,"close_button");
  rip_stop_button->setOnColor(red);
  rip_stop_button->on();
  connect(rip_stop_button,SIGNAL(clicked()),this,SLOT(stopButtonData()));
  
  //
  // Set Cut Button
  //
  rip_setcut_button=new QPushButton(tr("Set\n&Cut"),this,"setcut_button");
  rip_setcut_button->setFont(button_font);
  connect(rip_setcut_button,SIGNAL(clicked()),this,SLOT(setCutButtonData()));

  //
  // Set All Button
  //
  rip_setall_button=new QPushButton(tr("Set All\n&New Carts"),
				    this,"setall_button");
  rip_setall_button->setFont(button_font);
  connect(rip_setall_button,SIGNAL(clicked()),this,SLOT(setAllButtonData()));

  //
  // Normalize Check Box
  //
  rip_normalize_box=new QCheckBox(this,"rip_normalize_box");
  rip_normalize_box->setChecked(true);
  rip_normalizebox_label=new QLabel(rip_normalize_box,tr("Normalize"),
		   this,"normalize_check_label");
  rip_normalizebox_label->setFont(label_font);
  rip_normalizebox_label->setAlignment(AlignLeft|AlignVCenter);
  connect(rip_normalize_box,SIGNAL(toggled(bool)),
	  this,SLOT(normalizeCheckData(bool)));

  //
  // Normalize Level
  //
  rip_normalize_spin=new QSpinBox(this,"rip_normalize_spin");
  rip_normalize_spin->setRange(-30,0);
  rip_normalize_label=new QLabel(rip_normalize_spin,tr("Level:"),
				 this,"normalize_spin_label");
  rip_normalize_label->setFont(label_font);
  rip_normalize_label->setAlignment(AlignRight|AlignVCenter);
  rip_normalize_unit=new QLabel(tr("dBFS"),this,"normalize_unit_label");
  rip_normalize_unit->setFont(label_font);
  rip_normalize_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Autotrim Check Box
  //
  rip_autotrim_box=new QCheckBox(this,"rip_autotrim_box");
  rip_autotrim_box->setChecked(true);
  rip_autotrimbox_label=new QLabel(rip_autotrim_box,tr("Autotrim"),
		   this,"autotrim_check_label");
  rip_autotrimbox_label->setFont(label_font);
  rip_autotrimbox_label->setAlignment(AlignLeft|AlignVCenter);
  connect(rip_autotrim_box,SIGNAL(toggled(bool)),
	  this,SLOT(autotrimCheckData(bool)));

  //
  // Autotrim Level
  //
  rip_autotrim_spin=new QSpinBox(this,"rip_autotrim_spin");
  rip_autotrim_spin->setRange(-99,0);
  rip_autotrim_label=new QLabel(rip_autotrim_spin,tr("Level:"),
				 this,"autotrim_spin_label");
  rip_autotrim_label->setFont(label_font);
  rip_autotrim_label->setAlignment(AlignRight|AlignVCenter);
  rip_autotrim_unit=new QLabel(tr("dBFS"),this,"autotrim_unit_label");
  rip_autotrim_unit->setFont(label_font);
  rip_autotrim_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Channels
  //
  rip_channels_box=new QComboBox(this,"rip_channels_box");
  rip_channels_label=new QLabel(rip_channels_box,tr("Channels:"),
				this,"rip_channels_label");
  rip_channels_label->setFont(label_font);
  rip_channels_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Rip Track Button
  //
  rip_rip_button=new QPushButton(tr("&Rip\nDisc"),this,"rip_rip_button");
  rip_rip_button->setFont(button_font);
  connect(rip_rip_button,SIGNAL(clicked()),this,SLOT(ripDiskButtonData()));
  rip_rip_button->setDisabled(true);

  //
  // Close Button
  //
  rip_close_button=new QPushButton(tr("&Close"),this,"close_button");
  rip_close_button->setFont(button_font);
  connect(rip_close_button,SIGNAL(clicked()),this,SLOT(closeData()));

  //
  // Populate Data
  //
  rip_normalize_spin->setValue(rdlibrary_conf->ripperLevel()/100);
  rip_autotrim_spin->setValue(rdlibrary_conf->trimThreshold()/100);
  rip_channels_box->insertItem("1");
  rip_channels_box->insertItem("2");
  rip_channels_box->setCurrentItem(rdlibrary_conf->defaultChannels()-1);
  rip_done=false;
}


DiskRipper::~DiskRipper()
{
  rip_cdrom->close();
  delete rip_cdrom;
  delete rip_track_list;
  delete rip_rip_button;
  delete rip_close_button;
  delete rip_eject_button;
  delete rip_play_button;
  delete rip_stop_button;
  delete rip_track_bar;
  delete rip_bar_timer;
}


QSize DiskRipper::sizeHint() const
{
  return QSize(640,656);
}


QSizePolicy DiskRipper::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void DiskRipper::ejectButtonData()
{
  rip_cdrom->eject();
}


void DiskRipper::playButtonData()
{
  if(rip_track_list->currentItem()!=NULL) {
    rip_cdrom->play(rip_track_list->currentItem()->text(0).toInt());
    rip_play_button->on();
    rip_stop_button->off();
  }
}


void DiskRipper::stopButtonData()
{
  rip_cdrom->stop();
  rip_play_button->off();
  rip_stop_button->on();
}


void DiskRipper::ripDiskButtonData()
{
  QListViewItem *item=rip_track_list->selectedItem();
  if(item!=NULL) {
    rip_track_list->setSelected(item,false);
  }
  item=rip_track_list->firstChild();
  while(item!=NULL) {
    if(!item->text(5).isEmpty()) {
      rip_eject_button->setDisabled(true);
      rip_play_button->setDisabled(true);
      rip_stop_button->setDisabled(true);
      rip_rip_button->setDisabled(true);
      rip_setcut_button->setDisabled(true);
      rip_setall_button->setDisabled(true);
      rip_close_button->setDisabled(true);
      rip_normalize_box->setDisabled(true);
      rip_normalize_spin->setDisabled(true);
      rip_channels_box->setDisabled(true);
      rip_autotrim_box->setDisabled(true);
      rip_autotrim_spin->setDisabled(true);
      rip_base_length=0;
//      rip_disk_bar->setProgress(0,GetTotalLength());
//      rip_disk_bar->setPercentageVisible(true);
      RipTrack(item->text(0).toInt(),rip_cutnames[item->text(0).toInt()-1],
	       item->text(2));
      return;
    }
    item=item->nextSibling();
  }
}


void DiskRipper::ejectedData()
{
  rip_track_list->clear();
  rip_track=-1;
  rip_artist_edit->clear();
  rip_album_edit->clear();
  rip_other_edit->clear();
  rip_apply_box->setChecked(false);
  rip_apply_box->setDisabled(true);
  rip_apply_label->setDisabled(true);
}


void DiskRipper::setCutButtonData()
{
  QListViewItem *item=rip_track_list->selectedItem();
  if(item==NULL) {
    return;
  }
  QString cutname=rip_cutnames[item->text(0).toUInt()-1];
  RDCutDialog *dialog=new RDCutDialog(&cutname,rdstation_conf,rip_filter_text,
				      rip_group_text,lib_user->name(),true,
				      true,true,this,"cut_dialog");
  if(dialog->exec()==0) {
    if(cutname.isEmpty()) {
      rip_cutnames[item->text(0).toUInt()-1]="";
      item->setText(5,"");
    }
    else {
      for(unsigned i=0;i<rip_cutnames.size();i++) {
	if(cutname==rip_cutnames[i]) {
	  QMessageBox::warning(this,tr("Cut Conflict"),
			 tr("That cut has already been assigned a track!"));
	  return;
	}
      }
      if(QFile::exists(RDCut::pathName(cutname))){
	switch(QMessageBox::warning(this,tr("Cut Exists"),
				    tr("A recording already exists.\nDo you want to overwrite it?"),
				    QMessageBox::Yes,
				    QMessageBox::No)) {
	case QMessageBox::No:
	case QMessageBox::NoButton:
	  delete dialog;
	  return;
	  
	default:
	  break;
	}
      }
      if(cut_clipboard!=NULL) {
	if(cutname==cut_clipboard->cutName()) {
	  switch(QMessageBox::warning(this,tr("Empty Clipboard"),
				      tr("Ripping this cut will also empty the clipboard.\nDo you still want to proceed?"),
				      QMessageBox::Yes,
				      QMessageBox::No)) {
	  case QMessageBox::No:
	  case QMessageBox::NoButton:
	    return;
	    
	  default:
	    break;
	  }
	  delete cut_clipboard;
	  cut_clipboard=NULL;
	}
      }
      rip_cutnames[item->text(0).toUInt()-1]=cutname;
      RDCart *cart=new RDCart(cutname.left(6).toUInt());
      RDCut *cut=new RDCut(cutname);
      item->setText(5,QString().sprintf("%s -> %s",(const char *)cart->title(),
					(const char *)cut->description()));
      delete cut;
      delete cart;
    }
  }
  delete dialog;

  bool track_selected=false;
  item=rip_track_list->firstChild();
  while(item!=NULL) {
    if(!item->text(5).isEmpty()) {
      track_selected=true;
    }
    item=item->nextSibling();
  }
  rip_rip_button->setEnabled(track_selected);
}


void DiskRipper::setAllButtonData()
{
  RDListGroups *list_groups=new RDListGroups(rip_group_text,lib_user->name(),
					     this);
  if(list_groups->exec()<0) {
    delete list_groups;
    return;
  }
  delete list_groups;
  RDGroup *group=new RDGroup(*rip_group_text);
  if(group->freeCartQuantity()<rip_cdrom->tracks()) {
    QMessageBox::warning(this,tr("Library Error"),
			 tr("Insufficient free carts available in group!"));
    delete group;
    return;
  }
  unsigned cart_num=group->defaultLowCart();
  QListViewItem *item=rip_track_list->firstChild();
  while(item!=NULL) {
    cart_num=group->nextFreeCart(cart_num);
    rip_cutnames[item->text(0).toUInt()-1]=
      QString().sprintf("%06u_001",cart_num);
    item->setText(5,QString().sprintf("[New Cart %06u] -> Cut 001",cart_num));
    cart_num++;
    item=item->nextSibling();
  }
  rip_rip_button->setEnabled(true);
}


void DiskRipper::mediaChangedData()
{
  QListViewItem *l;
  QString cdda_dir=tempnam(RIPPER_TEMP_DIR,"cdda");

  mkdir(cdda_dir,0700);

  rip_cutnames.clear();
  rip_track_list->clear();
  rip_track=-1;
  for(int i=rip_cdrom->tracks();i>0;i--) {
    rip_cutnames.push_back(QString());
    l=new QListViewItem(rip_track_list);
    l->setText(0,QString().sprintf("%d",i));
    if(rip_cdrom->isAudio(i)) {
      l->setText(4,tr("Audio Track"));
    }
    else {
      l->setText(4,tr("Data Track"));
    }
    l->setText(1,RDGetTimeLength(rip_cdrom->trackLength(i)));
  }
  rip_cddb_record.clear();
  rip_cdrom->setCddbRecord(&rip_cddb_record);
  rip_cddb_lookup->setCddbRecord(&rip_cddb_record);
  rip_cddb_lookup->lookupRecord(cdda_dir,rdlibrary_conf->ripperDevice(),
				rdlibrary_conf->cddbServer(),8880,
				RIPPER_CDDB_USER,PACKAGE_NAME,VERSION);
  system(QString().sprintf("rm %s/*",(const char *)cdda_dir));
  system(QString().sprintf("rmdir %s",(const char *)cdda_dir));
}


void DiskRipper::playedData(int track)
{
  rip_play_button->on();
  rip_stop_button->off();
}


void DiskRipper::stoppedData()
{
  rip_play_button->off();
  rip_stop_button->on();
}


void DiskRipper::cddbDoneData(RDCddbLookup::Result result)
{
  switch(result) {
      case RDCddbLookup::ExactMatch:
	if(rip_cdrom->status()!=RDCdPlayer::Ok) {
	  return;
	}
	rip_artist_edit->setText(rip_cddb_record.discArtist());
	rip_album_edit->setText(rip_cddb_record.discAlbum());
	rip_other_edit->setText(rip_cddb_record.discExtended());
	for(int i=0;i<rip_cddb_record.tracks();i++) {
	  rip_track_list->findItem(QString().sprintf("%d",i+1),0)->
	    setText(2,rip_cddb_record.trackTitle(i));
	  rip_track_list->findItem(QString().sprintf("%d",i+1),0)->
	    setText(3,rip_cddb_record.trackExtended(i));
	}
	rip_apply_box->setChecked(true);
	rip_apply_box->setEnabled(true);
	rip_apply_label->setEnabled(true);
	break;
      case RDCddbLookup::PartialMatch:
	rip_track=-1;
	printf("Partial Match!\n");
	break;
      default:
	rip_track=-1;
	printf("No Match!\n");
	break;
  }
}


void DiskRipper::normalizeCheckData(bool state)
{
  rip_normalize_spin->setEnabled(state);
  rip_normalize_label->setEnabled(state);
  rip_normalize_unit->setEnabled(state);
}


void DiskRipper::autotrimCheckData(bool state)
{
  rip_autotrim_spin->setEnabled(state);
  rip_autotrim_label->setEnabled(state);
  rip_autotrim_unit->setEnabled(state);
}


void DiskRipper::barTimerData()
{
  if(ripper_running) {
    if(rip_track_bar->progress()==100) {
      rip_track_bar->setProgress(0);
    }
    else {
      rip_track_bar->setProgress(rip_track_bar->progress()+10);
    }
    rip_bar_timer->start(RIPPER_BAR_INTERVAL,true);
  }
  else {
    rip_track_bar->setPercentageVisible(false);
    rip_track_bar->reset();
    if(bar_file->exists()) {
      rip_done=true;
      QListViewItem *item=rip_track_list->firstChild();
      rip_base_length+=(rip_temp_length+rip_finished_length);
//      rip_disk_bar->setProgress(rip_base_length);
      RDCut *cut=new RDCut(rip_cutname);
      cut->reset();
      cut->setOriginName(rdstation_conf->name());
      RDCart *cart=new RDCart(rip_cutname.left(6).toUInt());
      cart->updateLength();
      cart->resetRotation();
      if(rip_apply_box->isChecked()&&(!rip_title.isEmpty())) {
	cart->setTitle(rip_title);
	cart->setArtist(rip_artist_edit->text());
	cut->setDescription(rip_title);
	cut->setIsrc(rip_cddb_record.isrc(rip_track_number-1));
      }
      if(rip_done&&rip_autotrim_box->isChecked()) {
	AutoTrim();
      }
      delete cart;
      delete cut;
      while(item!=NULL) {
	if((item->text(0).toInt()>rip_track_number)&&
	   (!item->text(5).isEmpty())) {
	  RipTrack(item->text(0).toInt(),
		   rip_cutnames[item->text(0).toInt()-1],item->text(2));
	  return;
	}
	item=item->nextSibling();
      }
      rip_cdrom->eject();
      QMessageBox::information(this,tr("Rip Complete"),
			       tr("Rip complete!"));
    }
    else {
      rip_cdrom->eject();
      if(rip_rip_aborted) {
	QMessageBox::information(this,tr("Rip Aborted"),
				 tr("The rip has been aborted."));

      }
      else {
	QMessageBox::warning(this,tr("Rip Failed"),
			     tr("The ripper encountered an error.\n\
Please check your ripper configuration and try again."));
      }
    }
    rip_eject_button->setEnabled(true);
    rip_play_button->setEnabled(true);
    rip_stop_button->setEnabled(true);
    rip_setcut_button->setEnabled(true);
    rip_setall_button->setEnabled(true);
    rip_close_button->setEnabled(true);
    rip_normalize_box->setEnabled(true);
    rip_normalize_spin->setEnabled(true);
    rip_channels_box->setEnabled(true);
    rip_autotrim_box->setEnabled(true);
    rip_autotrim_spin->setEnabled(true);
    //rip_disk_bar->setPercentageVisible(false);
//    rip_disk_bar->reset();
//    rip_diskbar_label->setDisabled(true);
    rip_trackbar_label->setDisabled(true);
//    rip_diskbar_label->setText(tr("Total Progress"));
    rip_trackbar_label->setText(tr("Ripping Track"));
//    rip_trackbar_label->setText(tr("Track Progress"));
    QListViewItem *item=rip_track_list->firstChild();
    while(item!=NULL) {
      item->setText(5,"");
      item=item->nextSibling();
    }
    delete bar_file;
    delete bar_temp_file;
    bar_file=NULL;
    bar_temp_file=NULL;
  }
}


void DiskRipper::resizeEvent(QResizeEvent *e)
{
  rip_artist_edit->setGeometry(65,9,size().width()-125,18);
  rip_album_edit->setGeometry(65,31,size().width()-125,18);
  rip_other_edit->setGeometry(65,53,size().width()-125,60);
  rip_apply_box->setGeometry(65,118,15,15);
  rip_apply_label->setGeometry(85,118,250,20);
  rip_track_list->setGeometry(10,156,size().width()-112,size().height()-342);
//  rip_diskbar_label->setGeometry(10,size().height()-174,size().width()-110,20);
//  rip_disk_bar->setGeometry(10,size().height()-154,size().width()-110,20); 
  rip_trackbar_label->setGeometry(10,size().height()-126,size().width()-110,
				  20);
  rip_track_bar->setGeometry(10,size().height()-106,size().width()-110,20);
  rip_eject_button->setGeometry(size().width()-90,156,80,50);
  rip_play_button->setGeometry(size().width()-90,216,80,50);
  rip_stop_button->setGeometry(size().width()-90,276,80,50);
  rip_setcut_button->setGeometry(size().width()-90,360,80,50);
  rip_setall_button->setGeometry(size().width()-90,420,80,50);
  rip_normalizebox_label->setGeometry(30,size().height()-78,85,20);
  rip_normalize_box->setGeometry(10,size().height()-78,20,20);
  rip_normalize_spin->setGeometry(170,size().height()-79,40,20);
  rip_normalize_label->setGeometry(120,size().height()-78,45,20);
  rip_normalize_unit->setGeometry(215,size().height()-78,40,20);
  rip_autotrimbox_label->setGeometry(30,size().height()-54,85,20);
  rip_autotrim_box->setGeometry(10,size().height()-54,20,20);
  rip_autotrim_spin->setGeometry(170,size().height()-54,40,20);
  rip_autotrim_label->setGeometry(120,size().height()-54,45,20);
  rip_autotrim_unit->setGeometry(215,size().height()-54,40,20);
  rip_channels_label->setGeometry(10,size().height()-30,75,20);
  rip_channels_box->setGeometry(90,size().height()-30,50,20);
  rip_rip_button->setGeometry(size().width()-200,size().height()-60,80,50);
  rip_close_button->setGeometry(size().width()-90,size().height()-60,80,50);
}


void DiskRipper::doubleClickedData(QListViewItem *item,const QPoint &pt,
				   int col)
{
  setCutButtonData();
}


void DiskRipper::closeData()
{
  if(rip_done&&rip_apply_box->isChecked()) {
    done(0);
  }
  else {
    done(-1);
  }
}


void DiskRipper::closeEvent(QCloseEvent *e)
{
  if(!ripper_running) {
    closeData();
  }
}


void DiskRipper::RipTrack(int track,QString cutname,QString title)
{
  bool rc;
  mode_t prev_mask;
  QString cmd;
  unsigned cartnum=cutname.left(6).toUInt();
  QString sql=QString().sprintf("select NUMBER from CART where NUMBER=%u",
				cartnum);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(!q->first()) {  // Create Cart/cut entries
    delete q;
    sql=QString().sprintf("insert into CART set \
                           NUMBER=%u,TYPE=%d,GROUP_NAME=\"%s\",\
                           TITLE=\"[New Cart %06u]\",CUT_QUANTITY=1",
			  cartnum,RDCart::Audio,
			  (const char *)(*rip_group_text),cartnum);
    q=new RDSqlQuery(sql);
    delete q;
    sql=QString().sprintf("insert into CUTS set\
                           CUT_NAME=\"%s\",CART_NUMBER=%u,\
                           DESCRIPTION=\"Cut 001\"",
			  (const char *)cutname,cartnum);
    q=new RDSqlQuery(sql);
  }
  delete q;
  
  rip_done=false;
  rip_rip_aborted=false;
  rip_wavefile=RDCut::pathName(cutname); 
  rip_track_number=track;
  rip_title=title;
  rip_cutname=cutname;
  bar_file=new QFile(rip_wavefile);
  prev_mask = umask(0113);      // Set umask so files are user and group writable.
  rc=bar_file->open(IO_WriteOnly);
  umask(prev_mask);
  if(!rc) {
    QMessageBox::warning(this,tr("No Access"),
			 tr("Unable to open the file for writing!"),
			 tr("OK"));
    delete bar_file;
    return;
  }
  bar_file->close();

  if(title.isEmpty()) {
//    rip_trackbar_label->
//      setText(QString().sprintf("%s - Track %d",
//				(const char *)tr("Track Progress"),track));
    rip_trackbar_label->
      setText(QString().sprintf("%s - Track %d",
				(const char *)tr("Ripping Track"),track));
  }
  else {
//    rip_trackbar_label->
//      setText(QString().sprintf("%s - %s",
//				(const char *)tr("Track Progress"),
//				(const char *)title));
    rip_trackbar_label->
      setText(QString().sprintf("%s - %s",
				(const char *)tr("Ripping Track"),
				(const char *)title));
  }
//  rip_diskbar_label->setEnabled(true);
  rip_trackbar_label->setEnabled(true);

  rip_temp_length=(int)((double)rip_cdrom->trackLength(track)*176.4);
  switch(rdlibrary_conf->defaultFormat()) {
      case 0:  // PCM16
	rip_finished_length=
	  (int)(((double)rip_temp_length/2.0)*
		(double)(rip_channels_box->currentItem()+1.0)*
		(double)rdlibrary_conf->defaultSampleRate()/44100.0);
	break;
      case 1:  // MPEG-1 Layer 2
      case 2:  // MPEG-1 Layer 3
	rip_finished_length=
	  (int)((double)rip_temp_length*
		(double)(rip_channels_box->currentItem()+1)*
		(double)rdlibrary_conf->defaultBitrate()/1411200.0);
	break;
  }
  int rip_format=rdlibrary_conf->defaultFormat();
  if(rip_format==5 || rip_format==3) {
    rip_format=0;
  }
  if(rip_format==2) {
    rip_format=1;
  }
  if(rip_normalize_box->isChecked()) {
    cmd=QString().
      sprintf("rd_rip_cd %d %s %d %6.4lf %d %d %d %d %s %s/%s %s/%s %d",
	      rip_track_number,
	      (const char *)rdlibrary_conf->ripperDevice(),
	      rdlibrary_conf->paranoiaLevel(),
	      pow(10.0,(double)(rip_normalize_spin->value())/20.0),
	      rdlibrary_conf->defaultFormat(),
	      rip_channels_box->currentItem()+1,
	      rdlibrary_conf->defaultSampleRate(),
	      (rip_channels_box->currentItem()+1)*
	      rdlibrary_conf->defaultBitrate()/1000, 
	      (const char *)rip_wavefile,
	      RIPPER_TEMP_DIR,
	      RIPPER_TEMP_PEAK,
	      RIPPER_TEMP_DIR,
	      RIPPER_TEMP_WAV,
	      rdlibrary_conf->srcConverter());
  }
  else {
    cmd=QString().
      sprintf("rd_rip_cd %d %s %d 0 %d %d %d %d %s %s/%s %s/%s %d",
	      rip_track_number,
	      (const char *)rdlibrary_conf->ripperDevice(),
	      rdlibrary_conf->paranoiaLevel(),
	      rdlibrary_conf->defaultFormat(),
	      rip_channels_box->currentItem()+1,
	      rdlibrary_conf->defaultSampleRate(),
	      (rip_channels_box->currentItem()+1)*
	      rdlibrary_conf->defaultBitrate()/1000,
	      (const char *)rip_wavefile,
	      RIPPER_TEMP_DIR,
	      RIPPER_TEMP_PEAK,
	      RIPPER_TEMP_DIR,
	      RIPPER_TEMP_WAV,
	      rdlibrary_conf->srcConverter());
  }
  ripper_running=true;
  bar_temp_file=new QFile(QString(RIPPER_TEMP_DIR)+QString("/")+
			  QString(RIPPER_TEMP_WAV));
  if((rip_ripper_pid=fork())==0) {
    if(system(cmd)!=0) {
      bar_file->remove();
    }
    else {
      if(rdlibrary_conf->defaultFormat()==5 || rdlibrary_conf->defaultFormat()==3) {
        system(QString().sprintf("rdfilewrite --add-mode --normalize=0 %s",(const char *)rip_wavefile).ascii());
        chmod(rip_wavefile+".energy",S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        chown(rip_wavefile+".energy",RDConfiguration()->uid(),RDConfiguration()->gid());
        system(QString().sprintf("rd_encode %s %d %d %d %d %s %s",
               rip_wavefile.ascii(),
               rdlibrary_conf->defaultFormat(),
               rdlibrary_conf->defaultSampleRate(),
               rip_channels_box->currentItem()+1,
               (rip_channels_box->currentItem()+1)*rdlibrary_conf->defaultBitrate()/1000,
               RDConfiguration()->audioOwner().ascii(),
               RDConfiguration()->audioGroup().ascii()));
      }
    }
    exit(0);
  }
  rip_track_bar->setProgress(0);
  rip_track_bar->setPercentageVisible(false);
  rip_bar_timer->start(RIPPER_BAR_INTERVAL,true);
}


int DiskRipper::GetTotalLength()
{
  unsigned raw_length=0;
  QListViewItem *item=rip_track_list->firstChild();
  while(item!=NULL) {
    if(!item->text(5).isEmpty()) {
      raw_length+=(unsigned)((double)rip_cdrom->
			 trackLength(item->text(0).toInt())*176.4);
    }
    item=item->nextSibling();
  }
  switch(rdlibrary_conf->defaultFormat()) {
      case 0:  // PCM16
	return (int)(((double)raw_length/2.0)*
		     (double)(rip_channels_box->currentItem()+1.0)*
		     (double)rdlibrary_conf->defaultSampleRate()/44100.0)+
	  raw_length;
	break;
      case 1:  // MPEG-1 Layer 2
      case 2:  // MPEG-1 Layer 3
	return (int)((double)raw_length*
		     (double)(rip_channels_box->currentItem()+1)*
		     (double)rdlibrary_conf->defaultBitrate()/1411200.0)+
	  raw_length;
	break;
  }
  return 0;
}


void DiskRipper::AutoTrim()
{
  int point;

  RDCut *cut=new RDCut(rip_cutname);
  if(!cut->exists()) {
    delete cut;
    return;
  }
  RDWaveFile *wave=new RDWaveFile(rip_wavefile);
  if(!wave->openWave()) {
    delete wave;
    delete cut;
    return;
  }
  if((point=wave->
      startTrim(-rip_autotrim_spin->value()*100+REFERENCE_LEVEL))>-1) {
    cut->setStartPoint((int)(1000.0*(double)point/
			     (double)wave->getSamplesPerSec()));
  }
  if((point=wave->
      endTrim(-rip_autotrim_spin->value()*100+REFERENCE_LEVEL))>-1) {
    cut->setEndPoint((int)(1000.0*(double)point/
			     (double)wave->getSamplesPerSec()));
  }
  cut->setLength(cut->endPoint()-cut->startPoint());
  delete wave;
  delete cut;
}

