// rdimport_audio.cpp
//
// Audio File Importation Dialog for Rivendell.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdimport_audio.cpp,v 1.22.2.2 2010/02/09 21:43:53 cvs Exp $
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

#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qpainter.h>

#include <rd.h>
#include <rdconf.h>
#include <rdwavefile.h>
#include <rdcart.h>
#include <rdcut.h>
#include <rdescape_string.h>
#include <rdstation.h>
#include <rdimport_audio.h>
#include <rdlibrary_conf.h>

RDImportAudio::RDImportAudio(QString cutname,QString *path,
			     RDSettings *settings,bool *import_metadata,
			     RDWaveData *wavedata,RDCut *clipboard,
			     RDStation *station,bool *running,RDConfig *config,
			     QWidget *parent,const char *name) 
  : QDialog(parent,name)
{
  import_config=config;
  import_default_settings=settings;
  import_path=path;
  import_settings=settings;
  import_cutname=cutname;
  import_import_metadata=import_metadata;
  import_wavedata=wavedata;
  import_clipboard=clipboard;
  import_station=station;
  import_running=running;
  import_file_filter=RD_AUDIO_FILE_FILTER;
  open_failed=false;

  setCaption(tr("Import/Export Audio File"));

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
  QFont mode_font=QFont("Helvetica",14,QFont::Bold);
  mode_font.setPixelSize(14);

  //
  // Mode Group
  //
  import_mode_group=new QButtonGroup(this,"import_mode_group");
  import_mode_group->hide();
  connect(import_mode_group,SIGNAL(clicked(int)),
	  this,SLOT(modeClickedData(int)));

  //
  // Input Mode Button
  //
  import_importmode_button=new QRadioButton(tr("Import File"), this,"import_importmode_button");
  import_mode_group->insert(import_importmode_button);
  import_importmode_button->setGeometry(10,10,sizeHint().width()-40,15);
  import_importmode_button->setFont(mode_font);
  import_importmode_button->setChecked(true);

  //
  // Input Filename
  //
  import_in_filename_edit=new QLineEdit(this,"import_in_filename_edit");
  import_in_filename_edit->setGeometry(85,30,sizeHint().width()-180,20);
  connect(import_in_filename_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(filenameChangedData(const QString &)));
  import_in_filename_label=
    new QLabel(import_in_filename_edit,tr("Filename:"),
	       this,"import_in_filename_label");
  import_in_filename_label->setGeometry(10,30,70,20);
  import_in_filename_label->setFont(label_font);
  import_in_filename_label->setAlignment(AlignVCenter|AlignRight);

  //
  // Input File Selector Button
  //
  import_in_selector_button=
    new QPushButton(tr("&Select"),this,"import_in_selector_button");
  import_in_selector_button->setGeometry(sizeHint().width()-85,27,70,26);
  connect(import_in_selector_button,SIGNAL(clicked()),
	  this,SLOT(selectInputFileData()));

  //
  // Input Metadata
  //
  import_in_metadata_box=new QCheckBox(tr("Import file metadata"),this,"import_in_metadata_box");
  import_in_metadata_box->setGeometry(95,56,160,15);
  import_in_metadata_box->setChecked(*import_import_metadata);
  import_in_metadata_box->setFont(label_font);

  //
  // Input Channels
  //
  import_channels_box=new QComboBox(this,"import_channels_box");
  import_channels_box->setGeometry(310,54,50,20);
  import_channels_label=
    new QLabel(import_channels_box,tr("Channels:"),
	       this,"import_channels_label");
  import_channels_label->setGeometry(230,54,75,20);
  import_channels_label->setFont(label_font);
  import_channels_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Autotrim Check Box
  //
  import_autotrim_box=new QCheckBox(tr("Autotrim"),this,"import_autotrim_box");
  import_autotrim_box->setGeometry(95,82,80,15);
  import_autotrim_box->setChecked(true);
  import_autotrim_box->setFont(label_font);
  connect(import_autotrim_box,SIGNAL(toggled(bool)),
	  this,SLOT(autotrimCheckData(bool)));

  //
  // Autotrim Level
  //
  import_autotrim_spin=new QSpinBox(this,"import_autotrim_spin");
  import_autotrim_spin->setGeometry(235,80,40,20);
  import_autotrim_spin->setRange(-99,0);
  import_autotrim_label=new QLabel(import_autotrim_spin,tr("Level:"),
				 this,"autotrim_spin_label");
  import_autotrim_label->setGeometry(185,80,45,20);
  import_autotrim_label->setFont(label_font);
  import_autotrim_label->setAlignment(AlignRight|AlignVCenter);
  import_autotrim_unit=new QLabel(tr("dBFS"),this,"autotrim_unit_label");
  import_autotrim_unit->setGeometry(280,80,40,20);
  import_autotrim_unit->setFont(label_font);
  import_autotrim_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Output Mode Button
  //
  import_exportmode_button=new QRadioButton(tr("Export File"),this,"import_exportmode_button");
  import_mode_group->insert(import_exportmode_button);
  import_exportmode_button->setGeometry(10,120,sizeHint().width()-40,15);
  import_exportmode_button->setFont(mode_font);

  //
  // Output Filename
  //
  import_out_filename_edit=new QLineEdit(this,"import_out_filename_edit");
  import_out_filename_edit->setGeometry(85,140,sizeHint().width()-180,20);
  connect(import_out_filename_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(filenameChangedData(const QString &)));
  import_out_filename_edit->setReadOnly(true);
  import_out_filename_label=
    new QLabel(import_out_filename_edit,tr("Filename:"),
	       this,"import_out_filename_label");
  import_out_filename_label->setGeometry(10,140,70,20);
  import_out_filename_label->setFont(label_font);
  import_out_filename_label->setAlignment(AlignVCenter|AlignRight);

  //
  // Output File Selector Button
  //
  import_out_selector_button=
    new QPushButton(tr("&Select"),this,"import_out_selector_button");
  import_out_selector_button->setGeometry(sizeHint().width()-85,137,70,26);
  connect(import_out_selector_button,SIGNAL(clicked()),
	  this,SLOT(selectOutputFileData()));

  //
  // Output Metadata
  //
  import_out_metadata_box=new QCheckBox(tr("Export file metadata"),this,"import_out_metadata_box");
  import_out_metadata_box->setGeometry(95,161,sizeHint().width()-210,15);
  import_out_metadata_box->setChecked(*import_import_metadata);
  import_out_metadata_box->setFont(label_font);
  import_out_metadata_box->hide();

  //
  // Output Format
  //
  import_format_edit=new QLineEdit(this,"import_format_edit");
  import_format_edit->setGeometry(85,181,sizeHint().width()-180,20);
  import_format_edit->setReadOnly(true);
  import_format_edit->setText(import_settings->description());
  import_format_label=new QLabel(import_out_filename_edit,tr("Format:"),
		   this,"import_out_filename_label");
  import_format_label->setGeometry(10,181,70,20);
  import_format_label->setFont(label_font);
  import_format_label->setAlignment(AlignVCenter|AlignRight);

  //
  // Output Format Selector Button
  //
  import_out_format_button=
    new QPushButton(tr("S&et"),this,"import_out_format_button");
  import_out_format_button->setGeometry(sizeHint().width()-85,178,70,26);
  connect(import_out_format_button,SIGNAL(clicked()),
	  this,SLOT(selectOutputFormatData()));

  //
  // Progress Bar
  //
  import_bar=new QProgressBar(100,this,"import_bar");
  import_bar->setGeometry(10,230,sizeHint().width()-20,20);

  //
  // Progress Bar Timer
  //
  import_bar_timer=new QTimer(this,"import_bar_timer");
  connect(import_bar_timer,SIGNAL(timeout()),this,SLOT(barTimerData()));

  //
  // Normalize Check Box
  //
  import_normalize_box=new QCheckBox(tr("Normalize"),this,"import_normalize_box");
  import_normalize_box->setGeometry(10,262,113,15);
  import_normalize_box->setChecked(true);
  import_normalize_box->setFont(label_font);
  connect(import_normalize_box,SIGNAL(toggled(bool)),
	  this,SLOT(normalizeCheckData(bool)));

  //
  // Normalize Level
  //
  import_normalize_spin=new QSpinBox(this,"import_normalize_spin");
  import_normalize_spin->setGeometry(160,260,40,20);
  import_normalize_spin->setRange(-30,0);
  import_normalize_label=new QLabel(import_normalize_spin,tr("Level:"),
				 this,"normalize_spin_label");
  import_normalize_label->setGeometry(110,260,45,20);
  import_normalize_label->setFont(label_font);
  import_normalize_label->setAlignment(AlignRight|AlignVCenter);
  import_normalize_unit=new QLabel(tr("dBFS"),this,"normalize_unit_label");
  import_normalize_unit->setGeometry(205,260,40,20);
  import_normalize_unit->setFont(label_font);
  import_normalize_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Import Button
  //
  import_import_button=new QPushButton(tr("&Import"),this,"import_button");
  import_import_button->
    setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  import_import_button->setFont(button_font);
  connect(import_import_button,SIGNAL(clicked()),this,SLOT(importData()));

  //
  // Cancel Button
  //
  import_cancel_button=new QPushButton(tr("&Cancel"),this,"cancel_button");
  import_cancel_button->
    setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
  import_cancel_button->setFont(button_font);
  import_cancel_button->setDefault(true);
  connect(import_cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Populate Data
  //
  import_normalize_spin->setValue(settings->normalizationLevel()/100);
  import_autotrim_spin->setValue(settings->autotrimLevel()/100);
  import_channels_box->insertItem("1");
  import_channels_box->insertItem("2");
  import_channels_box->setCurrentItem(settings->channels()-1);

  filenameChangedData("");
  modeClickedData(import_mode_group->selectedId());
}


RDImportAudio::~RDImportAudio()
{
}


QSize RDImportAudio::sizeHint() const
{
  return QSize(470,332);
}


QSizePolicy RDImportAudio::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void RDImportAudio::enableAutotrim(bool state)
{
  import_autotrim_box->setChecked(state);
  autotrimCheckData(state);
}


void RDImportAudio::setAutotrimLevel(int lvl)
{
  import_autotrim_spin->setValue(lvl/100);
}


void RDImportAudio::enableNormalization(bool state)
{
  import_normalize_box->setChecked(state);
  normalizeCheckData(state);
}


void RDImportAudio::setNormalizationLevel(int lvl)
{
  import_normalize_spin->setValue(lvl/100);
}


void RDImportAudio::setChannels(int chans)
{
  import_channels_box->setCurrentItem(chans-1);
}


int RDImportAudio::exec(bool enable_import,bool enable_export)
{
  import_importmode_button->setEnabled(enable_import);
  import_in_filename_label->setEnabled(enable_import);
  import_in_filename_edit->setEnabled(enable_import);
  import_in_metadata_box->setEnabled(enable_import&&(import_wavedata!=NULL));
  import_in_selector_button->setEnabled(enable_import);
  import_channels_label->setEnabled(enable_import);
  import_autotrim_box->setEnabled(enable_import);
  import_autotrim_spin->
    setEnabled(enable_import&&import_autotrim_box->isChecked());
  import_autotrim_label->
    setEnabled(enable_import&&import_autotrim_box->isChecked());
  import_autotrim_unit->
    setEnabled(enable_import&&import_autotrim_box->isChecked());
  import_channels_box->setEnabled(enable_import);

  import_exportmode_button->setEnabled(enable_export);

  if(enable_export&&(!enable_import)) {
    import_exportmode_button->setChecked(true);
    modeClickedData(import_mode_group->id(import_exportmode_button));
  }
  return QDialog::exec();
}


void RDImportAudio::modeClickedData(int id)
{
  import_in_filename_label->setDisabled(id);
  import_in_filename_edit->setDisabled(id);
  import_in_metadata_box->setDisabled(id||(import_wavedata==NULL));
  import_in_selector_button->setDisabled(id);
  import_autotrim_box->setDisabled(id);
  import_autotrim_spin->setDisabled(id);
  import_autotrim_label->setDisabled(id);
  import_autotrim_unit->setDisabled(id);
  import_channels_box->setDisabled(id);
  import_channels_label->setDisabled(id);
  import_out_filename_label->setEnabled(id);
  import_out_filename_edit->setEnabled(id);
  import_out_metadata_box->setEnabled(id);
  import_out_selector_button->setEnabled(id);
  import_format_edit->setEnabled(id);
  import_format_label->setEnabled(id);
  import_out_format_button->setEnabled(id);
  if(id) {
    import_import_button->setText(tr("Export"));
  }
  else {
    import_import_button->setText(tr("Import"));
  }
}


void RDImportAudio::filenameChangedData(const QString &str)
{
  import_import_button->setDisabled(str.isEmpty());
}


void RDImportAudio::normalizeCheckData(bool state)
{
  import_normalize_spin->setEnabled(state);
  import_normalize_label->setEnabled(state);
  import_normalize_unit->setEnabled(state);
}


void RDImportAudio::autotrimCheckData(bool state)
{
  import_autotrim_spin->setEnabled(state);
  import_autotrim_label->setEnabled(state);
  import_autotrim_unit->setEnabled(state);
}


void RDImportAudio::barTimerData()
{
  if(import_mode_group->selectedId()==0) {
    ImportProgress();
  }
  else {
    ExportProgress();
  }
}


void RDImportAudio::selectInputFileData()
{
  QString filename;

  if(import_in_filename_edit->text().isEmpty()) {
    filename=
      QFileDialog::getOpenFileName(*import_path,
				   import_file_filter,this);
  }
  else {
    filename=
      QFileDialog::getOpenFileName(import_in_filename_edit->text(),
				   import_file_filter,this);
  }
  if(!filename.isEmpty()) {
    import_in_filename_edit->setText(filename);
    *import_path=RDGetPathPart(import_in_filename_edit->text());
  }
}


void RDImportAudio::selectOutputFileData()
{
  QString filename;
  QString filter=import_settings->formatName()+" (*."+
    RDSettings::defaultExtension(import_station->name(),
				 import_settings->format())+")";

  if(import_out_filename_edit->text().isEmpty()) {
    filename=
      QFileDialog::getSaveFileName(*import_path,filter,this);
  }
  else {
    filename=QFileDialog::getSaveFileName(import_out_filename_edit->text(),
					  filter,this);
  }
  if(!filename.isEmpty()) {
    import_out_filename_edit->
      setText(RDSettings::pathName(import_station->name(),filename,
				   import_settings->format()));
    *import_path=RDGetPathPart(import_out_filename_edit->text());
  }
}


void RDImportAudio::selectOutputFormatData()
{
  RDExportSettingsDialog *dialog=
    new RDExportSettingsDialog(import_settings,import_station,this,"dialog");
  dialog->exec();
  delete dialog;
  import_format_edit->setText(import_settings->description());
  import_out_filename_edit->
    setText(RDSettings::pathName(import_station->name(),
				 import_out_filename_edit->text(),
				 import_settings->format()));
}


void RDImportAudio::importData()
{
  if(import_mode_group->selectedId()==0) {
    Import();
  }
  else {
    Export();
  }
}


void RDImportAudio::cancelData()
{
  done(-1);
}


void RDImportAudio::paintEvent(QPaintEvent *e)
{
  QPainter *p=new QPainter(this);
  p->setPen(QColor(black));
  p->moveTo(10,110);
  p->lineTo(sizeHint().width()-10,110);
  p->moveTo(0,215);
  p->lineTo(sizeHint().width(),215);
  p->moveTo(0,216);
  p->lineTo(sizeHint().width(),216);  
  p->end();
  delete p;
}


void RDImportAudio::closeEvent(QCloseEvent *e)
{
  if(!(*import_running)) {
//    cancelData();
  }
}


void RDImportAudio::Import()
{
  int format_in=0;

  if(*import_running) {
    return;
  }

  if(!QFile::exists(import_in_filename_edit->text())) {
    QMessageBox::warning(this,tr("Import Audio File"),
			 tr("File does not exist!"));
    return;
  }
  RDWaveFile *wave=new RDWaveFile(import_in_filename_edit->text());
  if(!wave->openWave(import_wavedata)) {
    if(import_default_settings->format()!=0 && import_default_settings->format()!=3 && import_default_settings->format()!=5) {
    QMessageBox::warning(this,tr("Import Audio File"),tr("Cannot open file!"));
    delete wave;
    return;
  }
    else {
      open_failed=true;
    }  
  }
  if(wave->type()==RDWaveFile::Unknown) {
    if(import_default_settings->format()!=5 && import_default_settings->format()!=3) {
    QMessageBox::warning(this,tr("Import Audio File"),
			 tr("Unsupported file type!"));
    wave->closeWave();
    delete wave;
    return;
  }
    else {
      open_failed=true;
    }  
  }
  int samplerate=wave->getSamplesPerSec();
  switch(wave->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	import_temp_length=wave->getSampleLength()*
	  wave->getChannels()*(wave->getBitsPerSample()/8);
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wave->getHeadLayer();
	import_temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;

      case WAVE_FORMAT_FLAC:
	format_in=0;
	import_temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;
      
      case WAVE_FORMAT_VORBIS:
	format_in=5;
	import_temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;
  }
  delete wave;

  import_import_aborted=false;
  import_dest_filename=RDCut::pathName(import_cutname);
  if(QFile::exists(import_dest_filename)) {
    switch(QMessageBox::
       warning(this,tr("Audio Exists"),
	       tr("This will overwrite the existing recording.\nDo you want to proceed?"),
	       QMessageBox::Yes,QMessageBox::No)) {
	case QMessageBox::No:
	case QMessageBox::NoButton:
	  return;
	  break;

	default:
	  break;
    }
    if(import_clipboard!=NULL) {
      if(import_cutname==import_clipboard->cutName()) {
	switch(QMessageBox::warning(this,tr("Empty Clipboard"),
				    tr("Importing this cut will also empty the clipboard.\nDo you still want to proceed?"),
				    QMessageBox::Yes,
				    QMessageBox::No)) {
	    case QMessageBox::No:
	    case QMessageBox::NoButton:
	      return;

	    default:
	      break;
	}
	delete import_clipboard;
	import_clipboard=NULL;
      }
    }
  }

  int lib_fmt=0;
  switch(import_default_settings->format()) {
      case RDSettings::Pcm16:  // PCM16
	import_finished_length=
	  (int)(((double)import_temp_length/2.0)*
		(double)(import_channels_box->currentItem()+1.0)*
		(double)import_default_settings->sampleRate()/44100.0);
	lib_fmt=0;
	break;

      case RDSettings::MpegL2:  // MPEG-1 Layer 2
	import_finished_length=
	  (int)((double)import_temp_length*
		(double)(import_channels_box->currentItem()+1)*
		(double)import_default_settings->bitRate()/1411200.0);
	lib_fmt=1;
	break;

      case RDSettings::MpegL1:
	break;
      case RDSettings::MpegL3:
	import_finished_length=
	  (int)((double)import_temp_length*
		(double)(import_channels_box->currentItem()+1)*
		(double)import_default_settings->bitRate()/1411200.0);
	lib_fmt=RDSettings::MpegL3;
	break;
      case RDSettings::Flac:
	import_finished_length=
	  (int)((double)import_temp_length*
		(double)(import_channels_box->currentItem()+1)*
		(double)import_default_settings->bitRate()/1411200.0);
	lib_fmt=RDSettings::Flac;
	break;
      case RDSettings::OggVorbis:
	import_finished_length=
	  (int)((double)import_temp_length*
		(double)(import_channels_box->currentItem()+1)*
		(double)import_default_settings->bitRate()/1411200.0);
	lib_fmt=RDSettings::OggVorbis;
	break;
      case RDSettings::Copy:
	break;
  }
  //
  // Generate Temporary Filenames
  //
  QString tempname=tempnam(NULL,IMPORT_TEMP_BASENAME);
  import_tempwav_name=tempname+".wav";
  import_tempdat_name=tempname+".dat";

  QString cmd;
  float normal=0.0;
  RDLibraryConf *rdlibrary=new RDLibraryConf(import_station->name(),0);
  if(import_normalize_box->isChecked()) {
    normal=pow(10.0,(double)(import_normalize_spin->value())/20.0);
    import_multipass=true;
    if (lib_fmt!=5 && lib_fmt!=3) {
    cmd=QString().
      sprintf("rd_import_file %6.4f %d %d %s %d %d %d %d %s %s %s %d",
	      normal,
	      format_in,
	      samplerate,
	      (const char *)RDEscapeString(import_in_filename_edit->text()).utf8(),  
	      lib_fmt,
	      import_channels_box->currentItem()+1,
	      import_default_settings->sampleRate(),
	      (import_channels_box->
	       currentItem()+1)*import_default_settings->bitRate()/1000,
	      RDCut::pathName(import_cutname).ascii(),  
	      (const char *)import_tempdat_name,
	      (const char *)import_tempwav_name,
	      rdlibrary->srcConverter());
  }
  else {
      if((format_in!=3 && format_in!=5) || open_failed || samplerate!=import_default_settings->sampleRate()) {
        cmd=QString().
          sprintf("rd_import_encode %s %s %d %d %d %d %f %s %s",
	        (const char *)RDEscapeString(import_in_filename_edit->text()).utf8(),  
	        RDCut::pathName(import_cutname).ascii(),  
	        lib_fmt,
	        import_default_settings->sampleRate(),
	        import_channels_box->currentItem()+1,
	        (import_channels_box->
	         currentItem()+1)*import_default_settings->bitRate()/1000,
	        normal,
	        import_config->audioOwner().ascii(),
	        import_config->audioGroup().ascii());
      }
      else {
        cmd=QString().
          sprintf("rd_import_copy %s %s %f %s %s",
	        (const char *)RDEscapeString(import_in_filename_edit->text()).utf8(),  
	        RDCut::pathName(import_cutname).ascii(),  
	        normal,
	        import_config->audioOwner().ascii(),
	        import_config->audioGroup().ascii());
      }  	      
    }	      
  }
  else {
    import_multipass=false;
    if (lib_fmt!=3 && lib_fmt!=5) {
    cmd=QString().
      sprintf("rd_import_file 0 %d %d %s %d %d %d %d %s %s %s %d",
	      format_in,
	      samplerate,
	      (const char *)RDEscapeString(import_in_filename_edit->text()).utf8(),
	      lib_fmt,
	      import_channels_box->currentItem()+1,
	      import_default_settings->sampleRate(),
	      (import_channels_box->
	       currentItem()+1)*import_default_settings->bitRate()/1000,
	      RDCut::pathName(import_cutname).ascii(),  
	      (const char *)import_tempdat_name,
	      (const char *)import_tempwav_name,
	      rdlibrary->srcConverter());
  }
    else {
      if((format_in!=3 && format_in!=5) || open_failed || samplerate!=import_default_settings->sampleRate()) {
        cmd=QString().
          sprintf("rd_import_encode %s %s %d %d %d %d 0 %s %s",
	        (const char *)RDEscapeString(import_in_filename_edit->text()).utf8(),  
	        RDCut::pathName(import_cutname).ascii(),  
	        lib_fmt,
	        import_default_settings->sampleRate(),
	        import_channels_box->currentItem()+1,
	        (import_channels_box->
	         currentItem()+1)*import_default_settings->bitRate()/1000,
	        import_config->audioOwner().ascii(),
	        import_config->audioGroup().ascii());
      }
    else {
        cmd=QString().
          sprintf("rd_import_copy %s %s 0 %s %s",
	        (const char *)RDEscapeString(import_in_filename_edit->text()).utf8(),  
	        RDCut::pathName(import_cutname).ascii(),
	        import_config->audioOwner().ascii(),
	        import_config->audioGroup().ascii());
      }  	      
    }	      
  }
  delete rdlibrary;
  *import_running=true;
  import_import_aborted=false;
  import_in_selector_button->setDisabled(true);
  import_import_button->setDisabled(true);
  import_cancel_button->setDisabled(true);
  if(import_normalize_box->isChecked()) {
    bar_temp_file.setName(import_tempwav_name);
  }
  if((import_script_pid=fork())==0) {
    if(system(cmd.utf8())==0) {
      chown(RDCut::pathName(import_cutname),import_config->uid(),
	    import_config->gid());
      chmod(RDCut::pathName(import_cutname),
	    S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    }
    else {
      unlink(import_dest_filename);
    }
    exit(0);
  }
  else {
    import_bar->setProgress(0);
    import_bar->setPercentageVisible(false);
    import_bar_timer->start(IMPORT_BAR_INTERVAL,true);
  }
}


void RDImportAudio::ImportProgress()
{
  QString cuts_sql;

  if(*import_running) {
/*    if(import_multipass) {
      import_bar->
	setProgress((int)(50.0*((double)bar_temp_file.
				size()/(double)import_temp_length+
				(double)GetFileSize(import_dest_filename)/
				(double)import_finished_length)));
    }
    else {
      import_bar->setProgress((int)(100.0*(double)GetFileSize(import_dest_filename)/
				    (double)import_finished_length));
    }*/
    if(import_bar->progress()==100) {
      import_bar->setProgress(0);
    }
    else {
      import_bar->setProgress(import_bar->progress()+10);
    }
    
    import_bar_timer->start(IMPORT_BAR_INTERVAL,true);
  }
  else {
    import_bar->setPercentageVisible(false);
    import_bar->reset();
    RDCart *cart=new RDCart(import_cutname.left(6).toUInt());
    RDCut *cut=new RDCut(import_cutname);
    cut->reset();
    cut->setOriginName(import_station->name());

    //
    // Update Metadata
    //
    if(import_wavedata!=NULL) {
      RDWaveFile *wave=new RDWaveFile(cut->pathName(cut->cutName()));
      if(wave->openWave()) {
         wave->hasEnergy();
         import_wavedata->setEndPos(wave->getExtTimeLength());
         wave->closeWave();
      }   
      delete wave;
      cart->setMetadata(import_wavedata);
      cut->setMetadata(import_wavedata);
    }
    if(import_autotrim_box->isChecked()) {
      AutoTrim();
    }
    delete cut;
    delete cart;
    if(QFile::exists(import_dest_filename)) {
      import_in_selector_button->setEnabled(import_wavedata!=NULL);
      import_import_button->setEnabled(true);
      import_cancel_button->setEnabled(true);
      QMessageBox::information(this,tr("Import Complete"),
			       tr("Import complete!"));
    }
    else {
      if(import_import_aborted) {
	QMessageBox::information(this,tr("Import Aborted"),
				 tr("The import has been aborted."));
      }
      else {
	QMessageBox::warning(this,tr("Import Failed"),
			     tr("The importer encountered an error.\n\
Please check your importer configuration\n\
or the input file and try again."));
      }
    }
    *import_import_metadata=import_in_metadata_box->isChecked();
    unlink(import_tempwav_name);
    unlink(import_tempdat_name);
    done(0);
  }
}


void RDImportAudio::Export()
{
  int format_in=0;
  QString custom_cmd;

  if(*import_running) {
    return;
  }

  import_dest_filename=import_out_filename_edit->text();
  if(QFile::exists(import_dest_filename)) {
    if(QMessageBox::warning(this,tr("File Exists"),
      tr("The selected file already exists!\nDo you want to overwrite it?"),
	       	    QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
      return;
    }
  }
  RDWaveFile *wave=new RDWaveFile(RDCut::pathName(import_cutname));
  if(!wave->openWave()) {
    QMessageBox::warning(this,tr("File Error"),
			 tr("Cannot open file in archive!"));
    delete wave;
    return;
  }
  if(wave->type()==RDWaveFile::Unknown) {
    QMessageBox::warning(this,tr("File Error"),
			 tr("Unsupported file type in archive!"));
    wave->closeWave();
    delete wave;
    return;
  }
  int samplerate=wave->getSamplesPerSec();
  switch(wave->getFormatTag()) {
      case WAVE_FORMAT_PCM:
	format_in=0;
	import_temp_length=wave->getSampleLength()*
	  wave->getChannels()*(wave->getBitsPerSample()/8);
	break;

      case WAVE_FORMAT_MPEG:
	format_in=wave->getHeadLayer();
	import_temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;

      case WAVE_FORMAT_VORBIS:
	format_in=5;
	import_temp_length=wave->getSampleLength()*wave->getChannels()*2;
	break;
  }
  import_import_aborted=false;

  int fd=open(import_dest_filename,
	      O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(fd<0) {
    QMessageBox::warning(this,tr("No Access"),
			 tr("Unable to open the file for writing!"),
			 tr("OK"));
    return;
  }
  ::close(fd);
  switch(import_settings->format()) {
    case RDSettings::Pcm16:
      import_finished_length=
	(int)(((double)import_temp_length/2.0)*
	      (double)import_settings->channels()*
	      (double)import_settings->sampleRate()/44100.0);
      break;
      
    case RDSettings::MpegL1:
    case RDSettings::MpegL2:
    case RDSettings::MpegL3:
      if(import_settings->bitRate()==0) {
	import_finished_length=
	  (int)((double)import_temp_length*
		(double)RDSettings::bytesPerSec(import_station->name(),
						import_settings->format(),
						import_settings->quality())/
		176400.0);
      }
      else {
	import_finished_length=
	  (int)((double)import_temp_length*
		(double)import_settings->channels()*
		(double)import_settings->bitRate()/1411200.0);
      }
      break;
      
    case RDSettings::Flac:
    case RDSettings::OggVorbis:
      import_finished_length=
	(int)((double)import_temp_length*
	      (double)RDSettings::bytesPerSec(import_station->name(),
					      import_settings->format(),
					      import_settings->quality())/
	      176400.0);
      break;
      
      case RDSettings::Copy:
	break;

    default:  // Custom format
      import_finished_length=0;
      custom_cmd=import_settings->
	resolvedCustomCommandLine(RDEscapeString(import_out_filename_edit->
						 text()));
      break;
  }

  //
  // Generate Temporary Filenames
  //
  QString tempname=tempnam(NULL,IMPORT_TEMP_BASENAME);
  import_tempwav_name=tempname+".wav";
  import_tempdat_name=tempname+".dat";

  QString cmd;
  float normal=0.0;
  RDLibraryConf *rdlibrary=new RDLibraryConf(import_station->name(),0);
  if(import_settings->format()<99) {
    if(import_normalize_box->isChecked()) {
      normal=pow(10.0,(double)(import_normalize_spin->value())/20.0);
      import_multipass=true;
      cmd=QString().
      sprintf("rd_export_file %6.4f %d %d %s %d %d %d %d %d %s %s %s %d",
	      normal,
	      format_in,
	      samplerate,
	      RDCut::pathName(import_cutname).ascii(),  
	      import_settings->format(),
	      import_settings->channels(),
	      import_settings->sampleRate(),
	      import_settings->bitRate()/1000,
	      import_settings->quality(),
	      (const char *)RDEscapeString(import_out_filename_edit->text()).utf8(),
	      (const char *)import_tempdat_name,
	      (const char *)import_tempwav_name,
	      rdlibrary->srcConverter());
    }
    else {
      import_multipass=false;
      cmd=QString().
      sprintf("rd_export_file 0 %d %d %s %d %d %d %d %d %s %s %s %d",
	      format_in,
	      samplerate,
	      RDCut::pathName(import_cutname).ascii(),  
	      import_settings->format(),
	      import_settings->channels(),
	      import_settings->sampleRate(),
	      import_settings->bitRate()/1000,
	      import_settings->quality(),
	      (const char *)RDEscapeString(import_out_filename_edit->text()).utf8(),  
	      (const char *)import_tempdat_name,
	      (const char *)import_tempwav_name,
	      rdlibrary->srcConverter());
    }
  }
  else { 
    cmd=QString().
      sprintf("cp %s %s",RDCut::pathName(import_cutname).ascii(),
	      (const char *)(const char *)RDEscapeString(import_out_filename_edit->text()).utf8());
  }
  delete rdlibrary;
  if(!custom_cmd.isEmpty()) {
    cmd+=" \""+custom_cmd+"\"";
    import_bar->setDisabled(true);
  }
  //printf("CMD: %s\n",(const char *)cmd);
  *import_running=true;
  import_import_aborted=false;
  import_out_selector_button->setDisabled(true);
  import_import_button->setDisabled(true);
  import_cancel_button->setDisabled(true);
  if(import_normalize_box->isChecked()) {
    bar_temp_file.setName(import_tempwav_name);
  }
  if((import_script_pid=fork())==0) {
    if(system((const char *)cmd.utf8())!=0) {
      unlink(import_dest_filename);
    }
    exit(0);
  }
  else {
    import_bar->setProgress(0);
    import_bar->setPercentageVisible(true);
    import_bar_timer->start(IMPORT_BAR_INTERVAL,true);
  }
}


void RDImportAudio::ExportProgress()
{
  if(*import_running) {
    if(import_multipass) {
      import_bar->
	setProgress((int)(50.0*((double)bar_temp_file.
				size()/(double)import_temp_length+
				(double)GetFileSize(import_dest_filename)/
				(double)import_finished_length)));
    }
    else {
      import_bar->setProgress((int)(100.0*(double)GetFileSize(import_dest_filename)/
				    (double)import_finished_length));
    }
    import_bar_timer->start(IMPORT_BAR_INTERVAL,true);
  }
  else {
    import_bar->setPercentageVisible(false);
    import_bar->reset();
    if(QFile::exists(import_dest_filename)) {
      import_out_selector_button->setEnabled(true);
      import_import_button->setEnabled(true);
      import_cancel_button->setEnabled(true);
      QMessageBox::information(this,tr("Export Complete"),
			       tr("Export complete!"));
    }
    else {
      if(import_import_aborted) {
	QMessageBox::information(this,tr("Export Aborted"),
				 tr("The export has been aborted."));
      }
      else {
	QMessageBox::warning(this,tr("Export Failed"),
			     tr("The Exporter encountered an error.\n\
Please check your configuration and try again."));
      }
    }
    unlink(import_tempwav_name);
    unlink(import_tempdat_name);
    done(0);
  }
}


void RDImportAudio::AutoTrim()
{
  RDCut *cut=new RDCut(import_cutname);
  cut->autoTrim(RDCut::AudioBoth,100*import_autotrim_spin->value());
  delete cut;
}


unsigned RDImportAudio::GetFileSize(const QString &filename)
{
  struct stat stat_data;

  memset(&stat_data,0,sizeof(stat_data));
  if(stat(filename,&stat_data)<0) {
    return 0;
  }
  return stat_data.st_size;
}
