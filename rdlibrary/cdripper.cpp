// cdripper.cpp
//
// CD Ripper Dialog for Rivendell.
//
//   (C) Copyright 2002-2003, 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cdripper.cpp,v 1.36.2.4 2009/09/04 01:29:50 cvs Exp $
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

#include <rd.h>
#include <rdconf.h>
#include <rdwavefile.h>
#include <rdcut.h>

#include <cdripper.h>
#include <globals.h>
#include <rdconfig.h>

//
// Global Variables
//
bool ripper_running;


CdRipper::CdRipper(QString cutname,RDCddbRecord *rec,RDLibraryConf *conf,
		   QWidget *parent,const char *name) 
  : QDialog(parent,name)
{
  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Generate Fonts
  //
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont label_font=QFont("Helvetica",12,QFont::Bold);
  label_font.setPixelSize(12);

  rip_cutname=cutname;
  rip_wavefile=RDCut::pathName(cutname); 
  rip_conf=conf;
  rip_cddb_record=rec;
  rip_track=-1;

  setCaption("Rip CD");

  //
  // The CDROM Drive
  //
  rip_cdrom=new RDCdPlayer(this,"rip_cdrom");
  connect(rip_cdrom,SIGNAL(ejected()),this,SLOT(ejectedData()));
  connect(rip_cdrom,SIGNAL(mediaChanged()),this,SLOT(mediaChangedData()));
  connect(rip_cdrom,SIGNAL(played(int)),this,SLOT(playedData(int)));
  connect(rip_cdrom,SIGNAL(stopped()),this,SLOT(stoppedData()));
  rip_cdrom->setDevice(rip_conf->ripperDevice());
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
  rip_artist_edit->setGeometry(65,9,sizeHint().width()-125,18);
  rip_artist_edit->setReadOnly(true);

  //
  // Album Edit
  //
  label=new QLabel(tr("Album:"),this,"album_label");
  label->setGeometry(10,32,50,18);
  label->setFont(label_font);
  label->setAlignment(AlignRight|AlignVCenter);
  rip_album_edit=new QLineEdit(this,"rip_album_edit");
  rip_album_edit->setGeometry(65,31,sizeHint().width()-125,18);
  rip_album_edit->setReadOnly(true);

  //
  // Other Edit
  //
  label=new QLabel(tr("Other:"),this,"other_label");
  label->setGeometry(10,54,50,16);
  label->setFont(label_font);
  label->setAlignment(AlignRight);
  rip_other_edit=new QTextEdit(this,"rip_other_edit");
  rip_other_edit->setGeometry(65,53,sizeHint().width()-125,60);
  rip_other_edit->setReadOnly(true);

  //
  // Apply FreeDB Check Box
  //
  rip_apply_box=new QCheckBox(this,"rip_apply_box");
  rip_apply_box->setGeometry(65,118,15,15);
  rip_apply_box->setChecked(true);
  rip_apply_box->setDisabled(true);
  rip_apply_label=new QLabel(rip_apply_box,tr("Apply FreeDB Values to Cart"),
		   this,"rip_apply_label");
  rip_apply_label->setGeometry(85,118,250,20);
  rip_apply_label->setFont(label_font);
  rip_apply_label->setAlignment(AlignLeft);
  rip_apply_box->setChecked(false);
  rip_apply_label->setDisabled(true);

  //
  // Track List
  //
  rip_track_list=new QListView(this,"rip_track_list");
  rip_track_list->setGeometry(10,156,sizeHint().width()-110,
			      sizeHint().height()-270);
  rip_track_list->setAllColumnsShowFocus(true);
  rip_track_list->setItemMargin(5);
  rip_track_list->setSorting(-1);
  connect(rip_track_list,SIGNAL(selectionChanged()),
	  this,SLOT(trackSelectionChangedData()));
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

  //
  // Progress Bar
  //
  rip_bar=new QProgressBar(100,this,"rip_bar");
  rip_bar->setGeometry(10,480,sizeHint().width()-110,20);

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
  rip_eject_button->setGeometry(sizeHint().width()-90,156,80,50);
  connect(rip_eject_button,SIGNAL(clicked()),this,SLOT(ejectButtonData()));
  
  //
  // Play Button
  //
  rip_play_button=new RDTransportButton(RDTransportButton::Play,
					this,"close_button");
  rip_play_button->setGeometry(sizeHint().width()-90,216,80,50);
  connect(rip_play_button,SIGNAL(clicked()),this,SLOT(playButtonData()));
  
  //
  // Stop Button
  //
  rip_stop_button=new RDTransportButton(RDTransportButton::Stop,
					this,"close_button");
  rip_stop_button->setGeometry(sizeHint().width()-90,276,80,50);
  rip_stop_button->setOnColor(red);
  rip_stop_button->on();
  connect(rip_stop_button,SIGNAL(clicked()),this,SLOT(stopButtonData()));
  
  //
  // Rip Track Button
  //
  rip_rip_button=new QPushButton(tr("&Rip\nTrack"),this,"rip_track_button");
  rip_rip_button->setGeometry(sizeHint().width()-90,380,80,50);
  rip_rip_button->setFont(button_font);
  rip_rip_button->setDisabled(true);
  connect(rip_rip_button,SIGNAL(clicked()),this,SLOT(ripTrackButtonData()));

  //
  // Normalize Check Box
  //
  rip_normalize_box=new QCheckBox(this,"rip_normalize_box");
  rip_normalize_box->setGeometry(10,508,20,20);
  rip_normalize_box->setChecked(true);
  label=new QLabel(rip_normalize_box,tr("Normalize"),
		   this,"normalize_check_label");
  label->setGeometry(30,508,85,20);
  label->setFont(label_font);
  label->setAlignment(AlignLeft|AlignVCenter);
  connect(rip_normalize_box,SIGNAL(toggled(bool)),
	  this,SLOT(normalizeCheckData(bool)));

  //
  // Normalize Level
  //
  rip_normalize_spin=new QSpinBox(this,"rip_normalize_spin");
  rip_normalize_spin->setGeometry(170,508,40,20);
  rip_normalize_spin->setRange(-30,0);
  rip_normalize_label=new QLabel(rip_normalize_spin,tr("Level:"),
				 this,"normalize_spin_label");
  rip_normalize_label->setGeometry(120,508,45,20);
  rip_normalize_label->setFont(label_font);
  rip_normalize_label->setAlignment(AlignRight|AlignVCenter);
  rip_normalize_unit=new QLabel(tr("dBFS"),this,"normalize_unit_label");
  rip_normalize_unit->setGeometry(215,508,40,20);
  rip_normalize_unit->setFont(label_font);
  rip_normalize_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Autotrim Check Box
  //
  rip_autotrim_box=new QCheckBox(this,"rip_autotrim_box");
  rip_autotrim_box->setGeometry(10,532,20,20);
  rip_autotrim_box->setChecked(true);
  label=new QLabel(rip_autotrim_box,tr("Autotrim"),
		   this,"autotrim_check_label");
  label->setGeometry(30,532,85,20);
  label->setFont(label_font);
  label->setAlignment(AlignLeft|AlignVCenter);
  connect(rip_autotrim_box,SIGNAL(toggled(bool)),
	  this,SLOT(autotrimCheckData(bool)));

  //
  // Autotrim Level
  //
  rip_autotrim_spin=new QSpinBox(this,"rip_autotrim_spin");
  rip_autotrim_spin->setGeometry(170,532,40,20);
  rip_autotrim_spin->setRange(-99,0);
  rip_autotrim_label=new QLabel(rip_autotrim_spin,tr("Level:"),
				 this,"autotrim_spin_label");
  rip_autotrim_label->setGeometry(120,532,45,20);
  rip_autotrim_label->setFont(label_font);
  rip_autotrim_label->setAlignment(AlignRight|AlignVCenter);
  rip_autotrim_unit=new QLabel(tr("dBFS"),this,"autotrim_unit_label");
  rip_autotrim_unit->setGeometry(215,532,40,20);
  rip_autotrim_unit->setFont(label_font);
  rip_autotrim_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Channels
  //
  rip_channels_box=new QComboBox(this,"rip_channels_box");
  rip_channels_box->setGeometry(90,556,50,20);
  label=new QLabel(rip_channels_box,tr("Channels:"),this,"rip_channels_label");
  label->setGeometry(10,556,75,20);
  label->setFont(label_font);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Close Button
  //
  rip_close_button=new QPushButton("&Close",this,"close_button");
  rip_close_button->
    setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
  rip_close_button->setFont(button_font);
  connect(rip_close_button,SIGNAL(clicked()),this,SLOT(closeData()));

  //
  // Populate Data
  //
  rip_normalize_spin->setValue(rip_conf->ripperLevel()/100);
  rip_autotrim_spin->setValue(rip_conf->trimThreshold()/100);
  rip_channels_box->insertItem("1");
  rip_channels_box->insertItem("2");
  rip_channels_box->setCurrentItem(rip_conf->defaultChannels()-1);
  rip_done=false;
}


CdRipper::~CdRipper()
{
  rip_cdrom->close();
  delete rip_cdrom;
  delete rip_track_list;
  delete rip_rip_button;
  delete rip_close_button;
  delete rip_eject_button;
  delete rip_play_button;
  delete rip_stop_button;
  delete rip_bar;
  delete rip_bar_timer;
}


QSize CdRipper::sizeHint() const
{
  return QSize(470,584);
}


QSizePolicy CdRipper::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void CdRipper::trackSelectionChangedData()
{
  rip_rip_button->setDisabled(rip_track_list->selectedItem()==NULL);
}


void CdRipper::ejectButtonData()
{
  rip_cdrom->eject();
}


void CdRipper::playButtonData()
{
  if(rip_track_list->currentItem()!=NULL) {
    rip_cdrom->play(rip_track_list->currentItem()->text(0).toInt());
    rip_play_button->on();
    rip_stop_button->off();
  }
}


void CdRipper::stopButtonData()
{
  rip_cdrom->stop();
  rip_play_button->off();
  rip_stop_button->on();
}


void CdRipper::ripTrackButtonData()
{
  QString cmd;
  bool rc;
  mode_t prev_mask;

  if(!ripper_running) {
    rip_done=false;
    rip_rip_aborted=false;
    bar_file=new QFile(rip_wavefile);
    if(bar_file->exists()) {
      switch(QMessageBox::warning(this,tr("Audio Exists"),
				  tr("This will overwrite the existing recording.\nDo you want to proceed?"),
				  QMessageBox::Yes,QMessageBox::No)) {
	  case QMessageBox::No:
	  case QMessageBox::NoButton:
	    delete bar_file;
	    return;

	  default:
	    break;
      }
    }
    if(cut_clipboard!=NULL) {
      if(rip_cutname==cut_clipboard->cutName()) {
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
    rip_eject_button->setDisabled(true);
    rip_play_button->setDisabled(true);
    rip_stop_button->setDisabled(true);
    rip_rip_button->setDisabled(true);
    rip_close_button->setDisabled(true);
    rip_temp_length=(int)((double)rip_cdrom->
	    trackLength(rip_track_list->currentItem()->text(0).toInt())*176.4);
    switch(rip_conf->defaultFormat()) {
	case 0:  // PCM16
	  rip_finished_length=
	    (int)(((double)rip_temp_length/2.0)*
		  (double)(rip_channels_box->currentItem()+1.0)*
		  (double)rip_conf->defaultSampleRate()/44100.0);
	  break;
	case 1:  // MPEG-1 Layer 2
	case 2:  // MPEG-1 Layer 3
	  rip_finished_length=
	    (int)((double)rip_temp_length*
		  (double)(rip_channels_box->currentItem()+1)*
		  (double)rip_conf->defaultBitrate()/1411200.0);
	  break;
    }
    rip_track=rip_track_list->currentItem()->text(0).toInt()-1;
    int rip_format=rip_conf->defaultFormat();
    if(rip_format==5 || rip_format==3) {
      rip_format=0;
    }
    if(rip_format==2) {
      rip_format=1;
    }
    if(rip_normalize_box->isChecked()) {
     cmd=QString().
       sprintf("rd_rip_cd %d %s %d %6.4lf %d %d %d %d %s %s/%s %s/%s %d",
	       rip_track_list->currentItem()->text(0).toInt(),
	       (const char *)rip_conf->ripperDevice(),
	       rip_conf->paranoiaLevel(),
	       pow(10.0,(double)(rip_normalize_spin->value())/20.0),
	       rip_conf->defaultFormat(),
	       rip_channels_box->currentItem()+1,
	       rip_conf->defaultSampleRate(),
	       (rip_channels_box->currentItem()+1)*
	       rip_conf->defaultBitrate()/1000, 
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
	       rip_track_list->currentItem()->text(0).toInt(),
	       (const char *)rip_conf->ripperDevice(),
	       rip_conf->paranoiaLevel(),
	       rip_conf->defaultFormat(),
	       rip_channels_box->currentItem()+1,
	       rip_conf->defaultSampleRate(),
	       (rip_channels_box->currentItem()+1)*
	       rip_conf->defaultBitrate()/1000,
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
        if(rip_conf->defaultFormat()==5 || rip_conf->defaultFormat()==3) {
          system(QString().sprintf("rdfilewrite --add-mode --normalize=0 %s",(const char *)rip_wavefile).ascii());
          chmod(rip_wavefile+".energy",S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
          chown(rip_wavefile+".energy",RDConfiguration()->uid(),RDConfiguration()->gid());
          system(QString().sprintf("rd_encode %s %d %d %d %d %s %s",
                 rip_wavefile.ascii(),
                 rip_conf->defaultFormat(),
                 rip_conf->defaultSampleRate(),
                 rip_channels_box->currentItem()+1,
                 (rip_channels_box->currentItem()+1)*rip_conf->defaultBitrate()/1000,
                 RDConfiguration()->audioOwner().ascii(),
                 RDConfiguration()->audioGroup().ascii()));
        } 
      }
      exit(0);
    }
    rip_bar->setProgress(0);
    rip_bar->setPercentageVisible(false);
    rip_bar_timer->start(RIPPER_BAR_INTERVAL,true);
  }
  else {
    rip_rip_aborted=true;
    bar_file->remove();
    ::kill(rip_ripper_pid,SIGHUP);
  }
}


void CdRipper::ejectedData()
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


void CdRipper::mediaChangedData()
{
  QListViewItem *l;
  QString cdda_dir=tempnam(RIPPER_TEMP_DIR,"cdda");

  mkdir(cdda_dir,0700);
  rip_track_list->clear();
  rip_track=-1;
  for(int i=rip_cdrom->tracks();i>0;i--) {
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
  rip_cddb_record->clear();
  rip_cdrom->setCddbRecord(rip_cddb_record);
  rip_cddb_lookup->setCddbRecord(rip_cddb_record);
  rip_cddb_lookup->lookupRecord(cdda_dir,rip_conf->ripperDevice(),
				rip_conf->cddbServer(),8880,
				RIPPER_CDDB_USER,PACKAGE_NAME,VERSION);
  system(QString().sprintf("rm %s/*",(const char *)cdda_dir));
  system(QString().sprintf("rmdir %s",(const char *)cdda_dir));
}


void CdRipper::playedData(int track)
{
  rip_play_button->on();
  rip_stop_button->off();
}


void CdRipper::stoppedData()
{
  rip_play_button->off();
  rip_stop_button->on();
}


void CdRipper::cddbDoneData(RDCddbLookup::Result result)
{
  switch(result) {
      case RDCddbLookup::ExactMatch:
	if(rip_cdrom->status()!=RDCdPlayer::Ok) {
	  return;
	}
	rip_artist_edit->setText(rip_cddb_record->discArtist());
	rip_album_edit->setText(rip_cddb_record->discAlbum());
	rip_other_edit->setText(rip_cddb_record->discExtended());
	for(int i=0;i<rip_cddb_record->tracks();i++) {
	  rip_track_list->findItem(QString().sprintf("%d",i+1),0)->
	    setText(2,rip_cddb_record->trackTitle(i));
	  rip_track_list->findItem(QString().sprintf("%d",i+1),0)->
	    setText(3,rip_cddb_record->trackExtended(i));
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


void CdRipper::normalizeCheckData(bool state)
{
  rip_normalize_spin->setEnabled(state);
  rip_normalize_label->setEnabled(state);
  rip_normalize_unit->setEnabled(state);
}


void CdRipper::autotrimCheckData(bool state)
{
  rip_autotrim_spin->setEnabled(state);
  rip_autotrim_label->setEnabled(state);
  rip_autotrim_unit->setEnabled(state);
}


void CdRipper::barTimerData()
{
  if(ripper_running) {
    if(rip_bar->progress()==100) {
      rip_bar->setProgress(0);
    }
    else {
      rip_bar->setProgress(rip_bar->progress()+10);
    }
    rip_bar_timer->start(RIPPER_BAR_INTERVAL,true);
  }
  else {
    rip_bar->setPercentageVisible(false);
    rip_bar->reset();
    rip_eject_button->setEnabled(true);
    rip_play_button->setEnabled(true);
    rip_stop_button->setEnabled(true);
    rip_rip_button->setEnabled(true);
    rip_close_button->setEnabled(true);
    rip_cdrom->unlock();
    if(bar_file->exists()) {
      rip_done=true;
      QMessageBox::information(this,tr("Rip Complete"),
			       tr("Rip complete!"));
    }
    else {
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
    delete bar_file;
    delete bar_temp_file;
    bar_file=NULL;
    bar_temp_file=NULL;
    RDCut *cut=new RDCut(rip_cutname);
    cut->reset();
    cut->setOriginName(rdstation_conf->name());
    delete cut;
  }
}


void CdRipper::closeData()
{
  if(rip_done&&rip_autotrim_box->isChecked()) {
    AutoTrim();
  }
  if(rip_done&&rip_apply_box->isChecked()) {
    done(rip_track);
  }
  else {
    done(-1);
  }
}


void CdRipper::closeEvent(QCloseEvent *e)
{
  if(!ripper_running) {
    closeData();
  }
}


void CdRipper::AutoTrim()
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

