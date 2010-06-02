// edit_svc.cpp
//
// Edit a Rivendell Service
//
//   (C) Copyright 2002-2004,2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_svc.cpp,v 1.40.2.5 2010/02/02 21:27:41 cvs Exp $
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
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include <rddb.h>
#include <rd.h>
#include <rduser.h>
#include <rdpasswd.h>
#include <rdtextvalidator.h>

#include <edit_svc.h>
#include <test_import.h>
#include <autofill_carts.h>
#include <edit_svc_perms.h>

EditSvc::EditSvc(QString svc,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  QString sql;
  RDSqlQuery *q;
  QString group;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  svc_svc=new RDSvc(svc);

  setCaption(tr("Edit Service"));

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont section_font=QFont("Helvetica",14,QFont::Bold);
  section_font.setPixelSize(14);

  //
  // Text Validators
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator");
  RDTextValidator *log_validator=new RDTextValidator(this,"validator");
  log_validator->addBannedChar('-');
  log_validator->addBannedChar('!');
  log_validator->addBannedChar('@');
  log_validator->addBannedChar('#');
  log_validator->addBannedChar('$');
  log_validator->addBannedChar('^');
  log_validator->addBannedChar('&');
  log_validator->addBannedChar('*');
  log_validator->addBannedChar('(');
  log_validator->addBannedChar(')');
  log_validator->addBannedChar('[');
  log_validator->addBannedChar(']');
  log_validator->addBannedChar('{');
  log_validator->addBannedChar('}');
  log_validator->addBannedChar('+');
  log_validator->addBannedChar('=');
  log_validator->addBannedChar('\\');
  log_validator->addBannedChar('|');
  log_validator->addBannedChar('?');
  log_validator->addBannedChar(';');
  log_validator->addBannedChar(':');
  log_validator->addBannedChar('.');
  log_validator->addBannedChar('<');
  log_validator->addBannedChar('>');
  log_validator->addBannedChar(',');
  log_validator->addBannedChar('/');

  //
  // General Section
  //
  QLabel *label=new QLabel("General",this,"traffic_import_label");
  label->setGeometry(10,10,120,24);
  label->setFont(section_font);
  label->setAlignment(AlignLeft);

  //
  // Service Name
  //
  svc_name_edit=new QLineEdit(this,"svc_name_edit");
  svc_name_edit->setGeometry(155,31,80,19);
  svc_name_edit->setMaxLength(10);
  svc_name_edit->setReadOnly(true);
  label=new QLabel(svc_name_edit,tr("&Service Name:"),this,
			   "svc_name_label");
  label->setGeometry(10,33,140,19);
  label->setAlignment(AlignRight|ShowPrefix);

  //
  // Service Description
  //
  svc_description_edit=new QLineEdit(this,"svc_description_edit");
  svc_description_edit->setGeometry(155,52,270,19);
  svc_description_edit->setMaxLength(255);
  svc_description_edit->setValidator(validator);
  label=new QLabel(svc_description_edit,
		   tr("Service &Description:"),this,
		   "svc_description_label");
  label->setGeometry(10,54,140,19);
  label->setAlignment(AlignRight|ShowPrefix);

  //
  // Service Template
  //
  svc_template_edit=new QLineEdit(this,"svc_template_edit");
  svc_template_edit->setGeometry(155,73,270,19);
  svc_template_edit->setMaxLength(255);
  svc_template_edit->setValidator(log_validator);
  label=new QLabel(svc_template_edit,
		   tr("Log &Template Name:"),this,
		   "svc_template_label");
  label->setGeometry(10,75,140,19);
  label->setAlignment(AlignRight|ShowPrefix);

  //
  // Voicetracking Group
  //
  svc_voice_group_box=new QComboBox(this,"svc_voice_group_group_box");
  svc_voice_group_box->setGeometry(155,94,270,19);
  svc_voice_group_box->insertItem(tr("[none]"));
  label=new QLabel(svc_voice_group_box,tr("Voicetrack Group:"),this,
		   "svc_voice_group_label");
  label->setGeometry(10,94,140,19);
  label->setAlignment(AlignRight|ShowPrefix);

  //
  // Chain Log
  //
  svc_chain_box=new QCheckBox(this,"svc_chain_box");
  svc_chain_box->setGeometry(30,117,15,15);
  label=new QLabel(svc_chain_box,tr("Insert CHAIN TO at log end"),
		   this,"svc_chain_label");
  label->setGeometry(50,117,170,19);
  label->setAlignment(AlignLeft|ShowPrefix);

  //
  // Default Auto Refresh
  //
  svc_autorefresh_box=new QCheckBox(this,"svc_autorefresh_box");
  svc_autorefresh_box->setGeometry(230,117,15,15);
  label=new QLabel(svc_autorefresh_box,tr("Enable AutoRefresh By Default"),
		   this,"svc_autorefresh_label");
  label->setGeometry(250,117,200,19);
  label->setAlignment(AlignLeft|ShowPrefix);

  //
  // Autofill Button
  //
  QPushButton *button=new QPushButton(this,"autofill_button");
  button->setGeometry(455,31,150,50);
  button->setFont(font);
  button->setText(tr("Configure \n&Autofill Carts"));
  connect(button,SIGNAL(clicked()),this,SLOT(autofillData()));

  //
  // Purge Expired Logs
  //
  svc_loglife_box=new QCheckBox(this,"svc_loglife_box");
  svc_loglife_box->setGeometry(460,95,15,15);
  label=new QLabel(svc_loglife_box,tr("Purge Logs after"),
		   this,"svc_loglife_label");
  label->setGeometry(480,95,200,19);
  label->setAlignment(AlignLeft|ShowPrefix);
  svc_loglife_spin=new QSpinBox(this,"svc_loglife_spin");
  svc_loglife_spin->setGeometry(585,93,50,19);
  svc_loglife_spin->setRange(0,365);
  connect(svc_loglife_box,SIGNAL(toggled(bool)),
	  svc_loglife_spin,SLOT(setEnabled(bool)));
  label=new QLabel(svc_loglife_box,tr("days"),
		   this,"svc_loglife_unit");
  label->setGeometry(645,95,40,19);
  label->setAlignment(AlignLeft|ShowPrefix);

  //
  // Purge Expired ELR Data
  //
  svc_shelflife_box=new QCheckBox(this,"svc_shelflife_box");
  svc_shelflife_box->setGeometry(460,117,15,15);
  label=new QLabel(svc_shelflife_box,tr("Purge ELR Data after"),
		   this,"svc_shelflife_label");
  label->setGeometry(480,117,200,19);
  label->setAlignment(AlignLeft|ShowPrefix);
  svc_shelflife_spin=new QSpinBox(this,"svc_shelflife_spin");
  svc_shelflife_spin->setGeometry(610,115,50,19);
  svc_shelflife_spin->setRange(0,365);
  connect(svc_shelflife_box,SIGNAL(toggled(bool)),
	  svc_shelflife_spin,SLOT(setEnabled(bool)));
  label=new QLabel(svc_shelflife_box,tr("days"),
		   this,"svc_shelflife_unit");
  label->setGeometry(670,117,40,19);
  label->setAlignment(AlignLeft|ShowPrefix);

  //
  // Enable Hosts Button
  //
  button=new QPushButton(this,"hosts_button");
  button->setGeometry(625,31,150,50);
  button->setFont(font);
  button->setText(tr("Enable &Hosts"));
  connect(button,SIGNAL(clicked()),this,SLOT(enableHostsData()));

  //
  // Traffic Import Section
  //
  label=new QLabel(tr("Traffic Data Import"),this,"traffic_import_label");
  label->setGeometry(10,150,160,24);
  label->setFont(section_font);
  label->setAlignment(AlignLeft);

  //
  // Linux Traffic Import Path
  //
  svc_tfc_path_edit=new QLineEdit(this,"svc_tfc_path_edit");
  svc_tfc_path_edit->setGeometry(185,171,240,19);
  svc_tfc_path_edit->setMaxLength(255);
  svc_tfc_path_edit->setValidator(validator);
  label=new QLabel(svc_tfc_path_edit,
		   tr("Linux Import Path:"),this,
		   "svc_tfc_path_label");
  label->setGeometry(10,171,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_path_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Linux Traffic Preimport Command
  //
  svc_tfc_preimport_cmd_edit=new QLineEdit(this,"svc_tfc_preimport_cmd_edit");
  svc_tfc_preimport_cmd_edit->setGeometry(185,192,240,19);
  svc_tfc_preimport_cmd_edit->setValidator(validator);
  label=new QLabel(svc_tfc_preimport_cmd_edit,
		   tr("Linux Preimport Command:"),this,
		   "svc_tfc_preimport_label");
  label->setGeometry(10,192,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_preimport_cmd_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Windows Traffic Import Path
  //
  svc_tfc_win_path_edit=new QLineEdit(this,"svc_tfc_win_path_edit");
  svc_tfc_win_path_edit->setGeometry(185,213,240,19);
  svc_tfc_win_path_edit->setMaxLength(255);
  label=new QLabel(svc_tfc_win_path_edit,
		   tr("Windows Import Path:"),this,
		   "svc_tfc_win_path_label");
  label->setGeometry(10,213,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_win_path_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Linux Traffic Preimport Command
  //
  svc_tfc_win_preimport_cmd_edit=
    new QLineEdit(this,"svc_tfc_win_preimport_cmd_edit");
  svc_tfc_win_preimport_cmd_edit->setGeometry(185,234,240,19);
  label=new QLabel(svc_tfc_win_preimport_cmd_edit,
		   tr("Windows Preimport Command:"),this,
		   "svc_tfc_win_preimport_label");
  label->setGeometry(10,234,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_win_preimport_cmd_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Note Cart
  //
  svc_tfc_label_cart_edit=new QLineEdit(this,"svc_tfc_label_cart_edit");
  svc_tfc_label_cart_edit->setGeometry(185,255,240,19);
  svc_tfc_label_cart_edit->setMaxLength(32);
  label=new QLabel(svc_tfc_label_cart_edit,
		   tr("Note Cart String:"),this,
		   "svc_tfc_label_cart_label");
  label->setGeometry(10,255,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_label_cart_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Track String
  //
  svc_tfc_track_edit=new QLineEdit(this,"svc_tfc_track_edit");
  svc_tfc_track_edit->setGeometry(185,276,240,19);
  svc_tfc_track_edit->setMaxLength(32);
  label=new QLabel(svc_tfc_track_edit,
		   tr("Insert Voice Track String:"),this,
		   "svc_tfc_track_cart_track");
  label->setGeometry(10,276,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_track_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Traffic Import Template
  //
  svc_tfc_import_template_box=new QComboBox(this,"svc_tfc_import_template_box");
  svc_tfc_import_template_box->setGeometry(185,297,240,19);
  label=new QLabel(svc_tfc_import_template_box,
		   tr("Import Template:"),this,
		   "svc_tfc_import_template_label");
  label->setGeometry(10,297,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_tfc_import_template_box,SIGNAL(activated(int)),
	  this,SLOT(tfcTemplateActivatedData(int)));

  //
  // Traffic Parser Settings
  //
  svc_tfc_fields=new ImportFields(this);
  svc_tfc_fields->setGeometry(10,318,svc_tfc_fields->sizeHint().width(),
			      svc_tfc_fields->sizeHint().height());

  //
  //  Traffic Test Button
  //
  button=new QPushButton(this,"tfc_button");
  button->setGeometry(360,318,60,40);
  button->setFont(font);
  button->setText(tr("Test \n&Traffic"));
  connect(button,SIGNAL(clicked()),this,SLOT(trafficData()));

  //
  // Music Import Section
  //
  label=new QLabel(tr("Music Data Import"),this,"music_import_label");
  label->setGeometry(445,150,160,24);
  label->setFont(section_font);
  label->setAlignment(AlignLeft);

  //
  // Linux Music Import Path
  //
  svc_mus_path_edit=new QLineEdit(this,"svc_mus_path_edit");
  svc_mus_path_edit->setGeometry(620,171,240,19);
  svc_mus_path_edit->setMaxLength(255);
  svc_mus_path_edit->setValidator(validator);
  label=new QLabel(svc_mus_path_edit,
		   tr("Linux Import Path:"),this,
		   "svc_mus_path_label");
  label->setGeometry(450,171,165,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_path_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Linux Music Preimport Command
  //
  svc_mus_preimport_cmd_edit=new QLineEdit(this,"svc_mus_preimport_cmd_edit");
  svc_mus_preimport_cmd_edit->setGeometry(620,192,240,19);
  svc_mus_preimport_cmd_edit->setValidator(validator);
  label=new QLabel(svc_mus_preimport_cmd_edit,
		   tr("Linux Preimport Command:"),this,
		   "svc_mus_preimport_label");
  label->setGeometry(450,192,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_preimport_cmd_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Windows Music Import Path
  //
  svc_mus_win_path_edit=new QLineEdit(this,"svc_mus_win_path_edit");
  svc_mus_win_path_edit->setGeometry(620,213,240,19);
  svc_mus_win_path_edit->setMaxLength(255);
  label=new QLabel(svc_mus_win_path_edit,
		   tr("Windows Import Path:"),this,
		   "svc_mus_win_path_label");
  label->setGeometry(450,213,165,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_win_path_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Windows Music Preimport Command
  //
  svc_mus_win_preimport_cmd_edit=
    new QLineEdit(this,"svc_mus_win_preimport_cmd_edit");
  svc_mus_win_preimport_cmd_edit->setGeometry(620,234,240,19);
  label=new QLabel(svc_mus_win_preimport_cmd_edit,
		   tr("Windows Preimport Command:"),this,
		   "svc_mus_win_preimport_label");
  label->setGeometry(450,234,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_win_preimport_cmd_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Label Cart
  //
  svc_mus_label_cart_edit=new QLineEdit(this,"svc_mus_label_cart_edit");
  svc_mus_label_cart_edit->setGeometry(620,255,240,19);
  svc_mus_label_cart_edit->setMaxLength(32);
  label=new QLabel(svc_mus_label_cart_edit,
		   tr("Note Cart String:"),this,
		   "svc_mus_label_cart_label");
  label->setGeometry(450,255,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_label_cart_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Insert Voice Track String
  //
  svc_mus_track_edit=new QLineEdit(this,"svc_mus_track_edit");
  svc_mus_track_edit->setGeometry(620,276,240,19);
  svc_mus_track_edit->setMaxLength(255);
  label=new QLabel(svc_mus_track_edit,
		   tr("Insert Voice Track String:"),this,
		   "svc_mus_track_label");
  label->setGeometry(450,276,165,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_track_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Insert Spot Break String
  //
  svc_mus_break_edit=new QLineEdit(this,"svc_mus_break_edit");
  svc_mus_break_edit->setGeometry(620,297,240,19);
  svc_mus_break_edit->setMaxLength(255);
  label=new QLabel(svc_mus_break_edit,
		   tr("Insert Traffic Break String:"),this,
		   "svc_mus_break_label");
  label->setGeometry(450,297,165,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_break_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Music Import Template
  //
  svc_mus_import_template_box=new QComboBox(this,"svc_mus_import_template_box");
  svc_mus_import_template_box->setGeometry(620,318,240,19);
  label=new QLabel(svc_mus_import_template_box,
		   tr("Import Template:"),this,
		   "svc_mus_import_template_label");
  label->setGeometry(450,318,170,19);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(svc_mus_import_template_box,SIGNAL(activated(int)),
	  this,SLOT(musTemplateActivatedData(int)));

  //
  // Music Parser Settings
  //
  svc_mus_fields=new ImportFields(this);
  svc_mus_fields->setGeometry(445,339,svc_mus_fields->sizeHint().width(),
			      svc_mus_fields->sizeHint().height());

  //
  // Music Test Button
  //
  button=new QPushButton(this,"mus_button");
  button->setGeometry(795,339,60,40);
  button->setFont(font);
  button->setText(tr("Test \n&Music"));
  connect(button,SIGNAL(clicked()),this,SLOT(musicData()));

  //
  //  Ok Button
  //
  button=new QPushButton(this,"ok_button");
  button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  button->setDefault(true);
  button->setFont(font);
  button->setText(tr("&OK"));
  connect(button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  button=new QPushButton(this,"cancel_button");
  button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			     80,50);
  button->setFont(font);
  button->setText(tr("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Load Group List
  //
  group=svc_svc->trackGroup();
  sql="select NAME from GROUPS order by NAME";
  q=new RDSqlQuery(sql);
  while(q->next()) {
    svc_voice_group_box->insertItem(q->value(0).toString());
    if(q->value(0).toString()==group) {
      svc_voice_group_box->setCurrentItem(svc_voice_group_box->count()-1);
    }
  }
  delete q;

  //
  // Populate Fields
  //
  svc_name_edit->setText(svc_svc->name());
  svc_description_edit->setText(svc_svc->description());
  svc_template_edit->setText(svc_svc->nameTemplate());
  svc_chain_box->setChecked(svc_svc->chainto());
  svc_autorefresh_box->setChecked(svc_svc->autoRefresh());
  if(svc_svc->defaultLogShelflife()>=0) {
    svc_loglife_box->setChecked(true);
    svc_loglife_spin->setValue(svc_svc->defaultLogShelflife());
  }
  else {
    svc_loglife_spin->setDisabled(true);
  }
  if(svc_svc->elrShelflife()>=0) {
    svc_shelflife_box->setChecked(true);
    svc_shelflife_spin->setValue(svc_svc->elrShelflife());
  }
  else {
    svc_shelflife_spin->setDisabled(true);
  }
  svc_tfc_path_edit->setText(svc_svc->importPath(RDSvc::Traffic,RDSvc::Linux));
  svc_tfc_preimport_cmd_edit->
    setText(svc_svc->preimportCommand(RDSvc::Traffic,RDSvc::Linux));
  svc_tfc_win_path_edit->
    setText(svc_svc->importPath(RDSvc::Traffic,RDSvc::Windows));  
  svc_tfc_win_preimport_cmd_edit->
    setText(svc_svc->preimportCommand(RDSvc::Traffic,RDSvc::Windows));  

  svc_tfc_import_template_box->insertItem(tr("[custom]"));
  svc_mus_import_template_box->insertItem(tr("[custom]"));
  sql="select NAME from IMPORT_TEMPLATES order by NAME";
  q=new RDSqlQuery(sql);
  while(q->next()) {
    svc_tfc_import_template_box->insertItem(q->value(0).toString());
    if(q->value(0).toString()==svc_svc->importTemplate(RDSvc::Traffic)) {
      svc_tfc_import_template_box->
	setCurrentItem(svc_tfc_import_template_box->count()-1);
    }
    svc_mus_import_template_box->insertItem(q->value(0).toString());
    if(q->value(0).toString()==svc_svc->importTemplate(RDSvc::Music)) {
      svc_mus_import_template_box->
	setCurrentItem(svc_mus_import_template_box->count()-1);
    }
  }
  delete q;
  svc_tfc_label_cart_edit->setText(svc_svc->labelCart(RDSvc::Traffic));
  svc_tfc_track_edit->setText(svc_svc->trackString(RDSvc::Traffic));
  svc_mus_path_edit->setText(svc_svc->importPath(RDSvc::Music,RDSvc::Linux));
  svc_mus_preimport_cmd_edit->
    setText(svc_svc->preimportCommand(RDSvc::Music,RDSvc::Linux));
  svc_mus_win_path_edit->
    setText(svc_svc->importPath(RDSvc::Music,RDSvc::Windows));  
  svc_mus_win_preimport_cmd_edit->
    setText(svc_svc->preimportCommand(RDSvc::Music,RDSvc::Windows));  
  svc_mus_label_cart_edit->setText(svc_svc->labelCart(RDSvc::Music));
  svc_mus_break_edit->
    setText(svc_svc->breakString());
  svc_mus_track_edit->
    setText(svc_svc->trackString(RDSvc::Music));
  svc_tfc_fields->setFields(svc_svc,RDSvc::Traffic);
  tfcTemplateActivatedData(svc_tfc_import_template_box->currentItem());
  musTemplateActivatedData(svc_mus_import_template_box->currentItem());
  svc_mus_fields->setFields(svc_svc,RDSvc::Music);
  import_changed=false;
}


EditSvc::~EditSvc()
{
  delete svc_name_edit;
  delete svc_description_edit;
}


QSize EditSvc::sizeHint() const
{
  return QSize(870,649);
} 


QSizePolicy EditSvc::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void EditSvc::autofillData()
{
  AutofillCarts *autofill=new AutofillCarts(svc_svc,this,"autofill");
  autofill->exec();
  delete autofill;
}


void EditSvc::enableHostsData()
{
  EditSvcPerms *edit_perms=new EditSvcPerms(svc_svc,this,"edit_perms");
  edit_perms->exec();
  delete edit_perms;
}


void EditSvc::trafficData()
{
  TestDataImport(RDSvc::Traffic);
}


void EditSvc::musicData()
{
  TestDataImport(RDSvc::Music);
}


void EditSvc::textChangedData(const QString &)
{
  import_changed=true;
}


void EditSvc::tfcTemplateActivatedData(int index)
{
  svc_tfc_fields->setEnabled(index==0);
  import_changed=true;
}


void EditSvc::musTemplateActivatedData(int index)
{
  svc_mus_fields->setEnabled(index==0);
  import_changed=true;
}


void EditSvc::okData()
{
  Save();
  done(0);
}


void EditSvc::cancelData()
{
  done(-1);
}


void EditSvc::TestDataImport(RDSvc::ImportSource src)
{
  if(import_changed||
     ((src==RDSvc::Traffic)&&(svc_tfc_fields->changed()))||
     ((src==RDSvc::Music)&&(svc_mus_fields->changed()))) {
    switch(QMessageBox::question(this,tr("Save Import Data"),
				 tr("Before testing, the import configuration\nmust be saved.  Save now?"),QMessageBox::Yes,QMessageBox::No)) {
	case QMessageBox::No:
	case QMessageBox::NoButton:
	  return;

	default:
	  break;
    }
    Save();
  }
  TestImport *testimport=new TestImport(svc_svc,src,this,"testimport");
  testimport->exec();
  delete testimport;
}


void EditSvc::Save()
{
  svc_svc->setDescription(svc_description_edit->text());
  svc_svc->setNameTemplate(svc_template_edit->text());
  svc_svc->setChainto(svc_chain_box->isChecked());
  svc_svc->setAutoRefresh(svc_autorefresh_box->isChecked());
  if(svc_loglife_box->isChecked()) {
    svc_svc->setDefaultLogShelflife(svc_loglife_spin->value());
  }
  else {
    svc_svc->setDefaultLogShelflife(-1);
  }
  if(svc_shelflife_box->isChecked()) {
    svc_svc->setElrShelflife(svc_shelflife_spin->value());
  }
  else {
    svc_svc->setElrShelflife(-1);
  }
  svc_svc->
    setImportPath(RDSvc::Traffic,RDSvc::Linux,svc_tfc_path_edit->text());
  svc_svc->
    setPreimportCommand(RDSvc::Traffic,RDSvc::Linux,
			svc_tfc_preimport_cmd_edit->text());
  svc_svc->
    setImportPath(RDSvc::Traffic,RDSvc::Windows,svc_tfc_win_path_edit->text());
  svc_svc->
    setPreimportCommand(RDSvc::Traffic,RDSvc::Windows,
			svc_tfc_win_preimport_cmd_edit->text());
  if(svc_tfc_import_template_box->currentItem()==0) {
    svc_svc->setImportTemplate(RDSvc::Traffic,"");
  }
  else {
    svc_svc->setImportTemplate(RDSvc::Traffic,
			       svc_tfc_import_template_box->currentText());
  }
  svc_svc->setLabelCart(RDSvc::Traffic,svc_tfc_label_cart_edit->text());
  svc_svc->setTrackString(RDSvc::Traffic,svc_tfc_track_edit->text());
  svc_tfc_fields->readFields(svc_svc,RDSvc::Traffic);
  svc_svc->setImportPath(RDSvc::Music,RDSvc::Linux,svc_mus_path_edit->text());
  svc_svc->setPreimportCommand(RDSvc::Music,RDSvc::Linux,
			       svc_mus_preimport_cmd_edit->text());
  svc_svc->
    setImportPath(RDSvc::Music,RDSvc::Windows,svc_mus_win_path_edit->text());
  svc_svc->
    setPreimportCommand(RDSvc::Music,RDSvc::Windows,
			svc_mus_win_preimport_cmd_edit->text());
  if(svc_mus_import_template_box->currentItem()==0) {
    svc_svc->setImportTemplate(RDSvc::Music,"");
  }
  else {
    svc_svc->setImportTemplate(RDSvc::Music,
			       svc_mus_import_template_box->currentText());
  }
  svc_svc->setBreakString(svc_mus_break_edit->text());
  svc_svc->setTrackString(RDSvc::Music,svc_mus_track_edit->text());
  svc_svc->setLabelCart(RDSvc::Music,svc_mus_label_cart_edit->text());
  svc_mus_fields->readFields(svc_svc,RDSvc::Music);
  import_changed=false;
  if(svc_voice_group_box->currentItem()==0) {
    svc_svc->setTrackGroup("");
  }
  else {
    svc_svc->setTrackGroup(svc_voice_group_box->currentText());
  }
}
