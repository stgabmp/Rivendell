// edit_rdairplay.cpp
//
// Edit an RDAirPlay Configuration
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_rdairplay.cpp,v 1.45.2.4 2009/03/30 19:02:52 cvs Exp $
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
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpainter.h>
#include <qfiledialog.h>

#include <rd.h>
#include <rddb.h>
#include <rdtextvalidator.h>
#include <rdlist_logs.h>
#include <globals.h>

#include <edit_rdairplay.h>
#include <edit_now_next.h>


EditRDAirPlay::EditRDAirPlay(RDStation *station,unsigned instance,
			     QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  QString sql;
  RDSqlQuery *q;

  air_exitpasswd_changed=false;
  air_logmachine=0;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  air_conf=new RDAirPlayConf(station->name(),instance);

  //
  // Create Fonts
  //
  QFont unit_font=QFont("Helvetica",12,QFont::Normal);
  unit_font.setPixelSize(12);
  QFont small_font=QFont("Helvetica",12,QFont::Bold);
  small_font.setPixelSize(12);
  QFont big_font=QFont("Helvetica",14,QFont::Bold);
  big_font.setPixelSize(14);

  //
  // Text Validator
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator");

  //
  // Dialog Name
  //
  setCaption(tr("RDAirPlay config for ")+station->name());

  //
  // Channel Assignments Section
  //
  QLabel *label=new QLabel(tr("Channel Assignments"),this,"globals_label");
  label->setFont(big_font);
  label->setGeometry(10,10,200,16);

  //
  // Main Log Output 1
  //
  label=new QLabel(tr("Main Log Output 1"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(25,32,200,16);
  air_card_sel[0]=new RDCardSelector(this,"air_card0_sel");
  air_card_sel[0]->setGeometry(20,50,120,117);
  air_start_rml_edit[0]=new QLineEdit(this);
  air_start_rml_edit[0]->setGeometry(210,50,160,19);
  air_start_rml_edit[0]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[0],tr("Start RML:"),this);
  label->setGeometry(140,50,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[0]=new QLineEdit(this);
  air_stop_rml_edit[0]->setGeometry(210,71,160,19);
  air_stop_rml_edit[0]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[0],tr("Stop RML:"),this);
  label->setGeometry(140,71,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Main Log Output 1
  //
  label=new QLabel(tr("Main Log Output 2"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(25,100,200,16);
  air_card_sel[1]=new RDCardSelector(this,"air_card1_sel");
  air_card_sel[1]->setGeometry(20,118,120,117);
  air_start_rml_edit[1]=new QLineEdit(this);
  air_start_rml_edit[1]->setGeometry(210,118,160,19);
  air_start_rml_edit[1]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[1],tr("Start RML:"),this);
  label->setGeometry(140,118,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[1]=new QLineEdit(this);
  air_stop_rml_edit[1]->setGeometry(210,139,160,19);
  air_stop_rml_edit[1]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[1],tr("Stop RML:"),this);
  label->setGeometry(140,139,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Aux Log 1 Output
  //
  label=new QLabel(tr("Aux Log 1 Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(25,168,200,16);
  air_card_sel[4]=new RDCardSelector(this,"air_card2_sel");
  air_card_sel[4]->setGeometry(20,186,120,117);
  air_start_rml_edit[4]=new QLineEdit(this);
  air_start_rml_edit[4]->setGeometry(210,186,160,19);
  air_start_rml_edit[4]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[4],tr("Start RML:"),this);
  label->setGeometry(140,186,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[4]=new QLineEdit(this);
  air_stop_rml_edit[4]->setGeometry(210,207,160,19);
  air_stop_rml_edit[4]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[4],tr("Stop RML:"),this);
  label->setGeometry(140,207,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Aux Log 2 Output
  //
  label=new QLabel(tr("Aux Log 2 Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(25,236,200,16);
  air_card_sel[5]=new RDCardSelector(this,"air_card3_sel");
  air_card_sel[5]->setGeometry(20,254,120,117);
  air_start_rml_edit[5]=new QLineEdit(this);
  air_start_rml_edit[5]->setGeometry(210,254,160,19);
  air_start_rml_edit[5]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[5],tr("Start RML:"),this);
  label->setGeometry(140,254,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[5]=new QLineEdit(this);
  air_stop_rml_edit[5]->setGeometry(210,275,160,19);
  air_stop_rml_edit[5]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[5],tr("Stop RML:"),this);
  label->setGeometry(140,275,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Audition/Cue Output
  //
  label=new QLabel(tr("Audition/Cue Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(25,304,200,16);
  air_card_sel[3]=new RDCardSelector(this,"air_card4_sel");
  air_card_sel[3]->setGeometry(20,322,120,117);
  air_start_rml_edit[3]=new QLineEdit(this);
  air_start_rml_edit[3]->setGeometry(210,322,160,19);
  air_start_rml_edit[3]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[3],tr("Start RML:"),this);
  label->setGeometry(140,322,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[3]=new QLineEdit(this);
  air_stop_rml_edit[3]->setGeometry(210,343,160,19);
  air_stop_rml_edit[3]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[3],tr("Stop RML:"),this);
  label->setGeometry(140,343,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Sound Panel First Play Output
  //
  label=new QLabel(tr("SoundPanel First Play Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(395,32,300,16);
  air_card_sel[2]=new RDCardSelector(this,"air_card5_sel");
  air_card_sel[2]->setGeometry(390,50,120,117);
  air_start_rml_edit[2]=new QLineEdit(this);
  air_start_rml_edit[2]->setGeometry(580,50,160,19);
  air_start_rml_edit[2]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[2],tr("Start RML:"),this);
  label->setGeometry(510,50,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[2]=new QLineEdit(this);
  air_stop_rml_edit[2]->setGeometry(580,71,160,19);
  air_stop_rml_edit[2]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[2],tr("Stop RML:"),this);
  label->setGeometry(510,71,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Sound Panel Second Play Output
  //
  label=new QLabel(tr("SoundPanel Second Play Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(395,100,300,16);
  air_card_sel[6]=new RDCardSelector(this,"air_card5_sel");
  air_card_sel[6]->setGeometry(390,118,120,117);
  air_start_rml_edit[6]=new QLineEdit(this);
  air_start_rml_edit[6]->setGeometry(580,118,160,19);
  air_start_rml_edit[6]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[6],tr("Start RML:"),this);
  label->setGeometry(510,118,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[6]=new QLineEdit(this);
  air_stop_rml_edit[6]->setGeometry(580,139,160,19);
  air_stop_rml_edit[6]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[6],tr("Stop RML:"),this);
  label->setGeometry(510,139,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Sound Panel Third Play Output
  //
  label=new QLabel(tr("SoundPanel Third Play Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(395,168,300,16);
  air_card_sel[7]=new RDCardSelector(this,"air_card5_sel");
  air_card_sel[7]->setGeometry(390,186,120,117);
  air_start_rml_edit[7]=new QLineEdit(this);
  air_start_rml_edit[7]->setGeometry(580,186,160,19);
  air_start_rml_edit[7]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[7],tr("Start RML:"),this);
  label->setGeometry(510,186,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[7]=new QLineEdit(this);
  air_stop_rml_edit[7]->setGeometry(580,207,160,19);
  air_stop_rml_edit[7]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[7],tr("Stop RML:"),this);
  label->setGeometry(510,207,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Sound Panel Fourth Play Output
  //
  label=new QLabel(tr("SoundPanel Fourth Play Output"),this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(395,236,300,16);
  air_card_sel[8]=new RDCardSelector(this,"air_card5_sel");
  air_card_sel[8]->setGeometry(390,254,120,117);
  air_start_rml_edit[8]=new QLineEdit(this);
  air_start_rml_edit[8]->setGeometry(580,254,160,19);
  air_start_rml_edit[8]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[8],tr("Start RML:"),this);
  label->setGeometry(510,254,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[8]=new QLineEdit(this);
  air_stop_rml_edit[8]->setGeometry(580,275,160,19);
  air_stop_rml_edit[8]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[8],tr("Stop RML:"),this);
  label->setGeometry(510,275,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Sound Panel Fifth Play Output
  //
  label=new QLabel(tr("SoundPanel Fifth and Later Play Output"),
		   this,"globals_label");
  label->setFont(small_font);
  label->setGeometry(395,304,300,16);
  air_card_sel[9]=new RDCardSelector(this,"air_card5_sel");
  air_card_sel[9]->setGeometry(390,322,120,117);
  air_start_rml_edit[9]=new QLineEdit(this);
  air_start_rml_edit[9]->setGeometry(580,322,160,19);
  air_start_rml_edit[9]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[9],tr("Start RML:"),this);
  label->setGeometry(510,322,65,19);
  label->setAlignment(AlignVCenter|AlignRight);
  air_stop_rml_edit[9]=new QLineEdit(this);
  air_stop_rml_edit[9]->setGeometry(580,343,160,19);
  air_stop_rml_edit[9]->setValidator(validator);
  label=new QLabel(air_start_rml_edit[9],tr("Stop RML:"),this);
  label->setGeometry(510,343,65,19);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Main Log Play Section
  //
  label=new QLabel("Log Settings",this,"globals_label");
  label->setFont(big_font);
  label->setGeometry(805,10,200,16);

  //
  // Segue Length
  //
  air_segue_edit=new QLineEdit(this,"air_segue_edit");
  air_segue_edit->setGeometry(895,32,50,20);
  air_segue_label=new QLabel(air_segue_edit,tr("Manual Segue:"),
			     this,"air_segue_label");
  air_segue_label->setGeometry(790,32,100,20);
  air_segue_label->setAlignment(AlignRight|AlignVCenter);
  air_segue_unit=new QLabel(air_segue_edit,tr("msecs"),
			     this,"air_segue_unit");
  air_segue_unit->setGeometry(950,32,40,20);
  air_segue_unit->setAlignment(AlignLeft|AlignVCenter);
  
  //
  // Forced Transition Length
  //
  air_trans_edit=new QLineEdit(this,"air_trans_edit");
  air_trans_edit->setGeometry(895,54,50,20);
  air_trans_label=new QLabel(air_trans_edit,tr("Forced Segue:"),
			     this,"air_trans_label");
  air_trans_label->setGeometry(790,54,100,20);
  air_trans_label->setAlignment(AlignRight|AlignVCenter);
  air_trans_unit=new QLabel(air_trans_edit,tr("msecs"),
			     this,"air_trans_unit");
  air_trans_unit->setGeometry(950,54,40,20);
  air_trans_unit->setAlignment(AlignLeft|AlignVCenter);
  
  //
  // Pie Countdown Length
  //
  air_piecount_box=new QSpinBox(this,"air_piecount_box");
  air_piecount_box->setGeometry(895,76,50,20);
  air_piecount_box->setRange(0,60);
  air_piecount_label=new QLabel(air_piecount_box,tr("Pie Counts Last:"),
			     this,"air_piecount_label");
  air_piecount_label->setGeometry(785,76,105,20);
  air_piecount_label->setAlignment(AlignRight|AlignVCenter);
  air_piecount_unit=new QLabel(tr("secs"),this,"air_piecount_unit");
  air_piecount_unit->setGeometry(950,76,40,20);
  air_piecount_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Pie Countdown To
  //
  air_countto_box=new QComboBox(this,"air_countto_box");
  air_countto_box->setGeometry(895,98,100,20);
  air_countto_label=new QLabel(air_countto_box,tr("Pie Counts To:"),
			     this,"air_countto_label");
  air_countto_label->setGeometry(785,98,105,20);
  air_countto_label->setAlignment(AlignRight|AlignVCenter);
  air_countto_box->insertItem(tr("Cart End"));
  air_countto_box->insertItem(tr("Transition"));

  //
  // Default Transition Type
  //
  air_default_transtype_box=new QComboBox(this,"air_default_transtype_box");
  air_default_transtype_box->setGeometry(895,120,100,20);
  label=new QLabel(air_default_transtype_box,tr("Default Trans. Type:"),
		   this,"air_default_transtype_label");
  label->setGeometry(760,120,130,20);
  label->setAlignment(AlignRight|AlignVCenter);
  air_default_transtype_box->insertItem(tr("Play"));
  air_default_transtype_box->insertItem(tr("Segue"));
  air_default_transtype_box->insertItem(tr("Stop"));

  //
  // Default Service
  //
  air_defaultsvc_box=new QComboBox(this,"air_defaultsvc_box");
  air_defaultsvc_box->setGeometry(895,142,100,20);
  label=new QLabel(air_defaultsvc_box,tr("Default Service:"),
		   this,"air_defaultsvc_label");
  label->setGeometry(760,142,130,20);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Sound Panel Section
  //
  label=new QLabel(tr("Sound Panel Settings"),this,"globals_label");
  label->setFont(big_font);
  label->setGeometry(805,179,200,16);

  //
  // # of Station Panels
  //
  air_station_box=new QSpinBox(this,"air_station_box");
  air_station_box->setGeometry(895,204,50,20);
  air_station_box->setRange(0,MAX_PANELS);
  air_station_box->setSpecialValueText(tr("None"));
  air_station_label=new QLabel(air_station_box,tr("Host Panels:"),
			     this,"air_station_label");
  air_station_label->setGeometry(790,204,100,20);
  air_station_label->setAlignment(AlignRight|AlignVCenter);

  //
  // # of User Panels
  //
  air_user_box=new QSpinBox(this,"air_user_box");
  air_user_box->setGeometry(895,226,50,20);
  air_user_box->setRange(0,MAX_PANELS);
  air_user_box->setSpecialValueText(tr("None"));
  air_user_label=new QLabel(air_user_box,tr("User Panels:"),
			     this,"air_user_label");
  air_user_label->setGeometry(790,226,100,20);
  air_user_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Flash Active Button
  //
  air_flash_box=new QCheckBox(this,"air_flash_box");
  air_flash_box->setGeometry(810,254,15,15);
  label=new QLabel(air_flash_box,tr("Flash Active Buttons"),
			     this,"air_flash_label");
  label->setGeometry(830,254,150,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Pause Panel Button
  //
  air_panel_pause_box=new QCheckBox(this,"air_panel_pause_box");
  air_panel_pause_box->setGeometry(810,276,15,15);
  label=new QLabel(air_panel_pause_box,tr("Enable Button Pausing"),
			     this,"air_panel_pause_label");
  label->setGeometry(830,276,150,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Label Template
  //
  air_label_template_edit=new QLineEdit(this,"air_label_template_edit");
  air_label_template_edit->setGeometry(895,298,sizeHint().width()-910,20);
  label=new QLabel(air_label_template_edit,tr("Label Template:"),
		   this,"air_label_template_label");
  label->setGeometry(790,298,100,20);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Miscellaneous Section
  //
  label=new QLabel(tr("Miscellaneous Settings"),this,"globals_label");
  label->setFont(big_font);
  label->setGeometry(805,330,200,16);

  //
  // Startup Mode
  //
  air_startup_box=new QComboBox(this,"air_startup_box");
  air_startup_box->setGeometry(885,351,110,20);
  air_startup_label=new QLabel(air_startup_box,tr("Startup Mode:"),
			     this,"air_startup_label");
  air_startup_label->setGeometry(785,351,95,20);
  air_startup_label->setAlignment(AlignRight|AlignVCenter);
  air_startup_box->insertItem(tr("Previous"));
  air_startup_box->insertItem(tr("LiveAssist"));
  air_startup_box->insertItem(tr("Automatic"));
  air_startup_box->insertItem(tr("Manual"));

  //
  // Check Timesync
  //
  air_timesync_box=new QCheckBox(this,"air_timesync_box");
  air_timesync_box->setGeometry(810,378,15,15);
  air_timesync_label=new QLabel(air_timesync_box,tr("Check TimeSync"),
			     this,"air_timesync_label");
  air_timesync_label->setGeometry(830,378,100,15);
  air_timesync_label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Show Log Buttons
  //
  air_auxlog_box[0]=new QCheckBox(this,"air_auxlog1_box");
  air_auxlog_box[0]->setGeometry(810,400,15,15);
  label=new QLabel(air_auxlog_box[0],tr("Show Auxlog 1 Button"),
		   this,"air_auxlog_label");
  label->setGeometry(830,400,150,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  air_auxlog_box[1]=new QCheckBox(this,"air_auxlog2_box");
  air_auxlog_box[1]->setGeometry(810,423,15,15);
  label=new QLabel(air_auxlog_box[1],tr("Show Auxlog 2 Button"),
		   this,"air_auxlog_label");
  label->setGeometry(830,423,150,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Clear Cart Filter
  //
  air_clearfilter_box=new QCheckBox(this,"air_clearfilter_box");
  air_clearfilter_box->setGeometry(810,444,15,15);
  label=new QLabel(air_clearfilter_box,tr("Clear Cart Search Filter"),
		   this,"air_clearfilter_label");
  label->setGeometry(830,444,150,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Pause Enable Checkbox
  //
  air_pause_box=new QCheckBox(this,"air_pause_box");
  air_pause_box->setGeometry(810,466,15,15);
  label=new QLabel(air_pause_box,tr("Enable Paused Events"),
		   this,"air_pause_label");
  label->setGeometry(830,466,150,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Show Extra Counters/Buttons
  //
  air_show_counters_box=new QCheckBox(this,"air_show_counters_box");
  air_show_counters_box->setGeometry(810,488,15,15);
  label=new QLabel(air_show_counters_box,tr("Show Extra Buttons/Counters"),
		   this,"air_show_counters_label");
  label->setGeometry(830,488,170,15);
  label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Audition Preroll
  //
  air_audition_preroll_spin=new QSpinBox(this,"air_audition_preroll_spin");
  air_audition_preroll_spin->setGeometry(895,508,45,20);
  air_audition_preroll_spin->setRange(1,60);
  air_audition_preroll_label=new QLabel(air_audition_preroll_spin,
					tr("Audition Preroll:"),
					this,"air_audition_preroll_label");
  air_audition_preroll_label->setGeometry(800,510,90,15);
  air_audition_preroll_label->setAlignment(AlignRight|AlignVCenter);
  air_audition_preroll_unit=
    new QLabel(tr("secs"),this,"air_audition_preroll_unit");
  air_audition_preroll_unit->setGeometry(945,510,100,15);
  air_audition_preroll_unit->setAlignment(AlignLeft|AlignVCenter);

  //
  // Space Bar Action
  //
  air_bar_group=new QButtonGroup(1,Qt::Vertical,tr("Space Bar Action"),
				 this,"air_bar_group");
  air_bar_group->setGeometry(805,532,sizeHint().width()-815,55);
  QRadioButton *rbutton=
    new QRadioButton(tr("None"),air_bar_group,"none_button");
  rbutton=new QRadioButton(tr("Start Next"),air_bar_group,"start_next_button");

  //
  // Now & Next Button
  //
  QPushButton *button=new QPushButton(this,"nownext_button");
  button->setGeometry(815,603,180,50);
  button->setFont(small_font);
  button->setText(tr("Configure Now && Next\nParameters"));
  connect(button,SIGNAL(clicked()),this,SLOT(nownextData()));
  
  //
  // Start/Stop Section
  //
  label=new QLabel(tr("Start/Stop Settings"),this,"globals_label");
  label->setFont(big_font);
  label->setGeometry(10,411,200,16);

  //
  // Exit Password
  //
  air_exitpasswd_edit=new QLineEdit(this,"air_exitpasswd_edit");
  air_exitpasswd_edit->setGeometry(100,434,sizeHint().width()-905,20);
  air_exitpasswd_edit->setEchoMode(QLineEdit::Password);
  air_exitpasswd_edit->setText("******");
  label=new QLabel(air_exitpasswd_edit,tr("Exit Password:"),
		   this,"air_exitpasswd_label");
  label->setGeometry(0,434,95,20);
  label->setAlignment(AlignRight|AlignVCenter);
  connect(air_exitpasswd_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(exitPasswordChangedData(const QString &)));

  //
  // Log Machine Selector
  //
  air_logmachine_box=new QComboBox(this,"air_logmachine_box");
  air_logmachine_box->setGeometry(45,459,100,20);
  air_logmachine_box->insertItem(tr("Main Log"));
  for(unsigned i=1;i<RDAIRPLAY_LOG_QUANTITY;i++) {
    air_logmachine_box->insertItem(QString().sprintf("Aux %d Log",i));
  }
  connect(air_logmachine_box,SIGNAL(activated(int)),
	  this,SLOT(logActivatedData(int)));

  //
  // Startup Mode
  //
  air_startmode_box=new QComboBox(this,"air_startmode_box");
  air_startmode_box->setGeometry(100,484,240,20);
  air_startmode_box->insertItem(tr("start with empty log"));
  air_startmode_box->insertItem(tr("load previous log"));
  air_startmode_box->insertItem(tr("load specified log"));
  label=new QLabel(air_exitpasswd_edit,tr("At Startup:"),
		   this,"air_exitpasswd_label");
  label->setGeometry(30,484,65,20);
  label->setAlignment(AlignRight|AlignVCenter);
  connect(air_startmode_box,SIGNAL(activated(int)),
	  this,SLOT(startModeChangedData(int)));

  //
  // Auto Restart Checkbox
  //
  air_autorestart_box=new QCheckBox(this,"air_autorestart_box");
  air_autorestart_box->setGeometry(105,509,15,15);
  air_autorestart_label=
    new QLabel(air_autorestart_box,tr("Restart Log After Unclean Shutdown"),
	       this,"air_autorestart_label");
  air_autorestart_label->setGeometry(125,509,250,15);
  air_autorestart_label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Startup Log
  //
  air_startlog_edit=new QLineEdit(this,"air_startlog_edit");
  air_startlog_edit->setGeometry(100,529,240,20);
  air_startlog_label=new QLabel(air_startlog_edit,tr("Log:"),
		   this,"air_startlog_label");
  air_startlog_label->setGeometry(30,529,65,20);
  air_startlog_label->setAlignment(AlignRight|AlignVCenter);

  //
  //  Log Select Button
  //
  air_startlog_button=new QPushButton(this,"air_startlog_button");
  air_startlog_button->setGeometry(350,527,50,24);
  air_startlog_button->setFont(small_font);
  air_startlog_button->setText(tr("&Select"));
  connect(air_startlog_button,SIGNAL(clicked()),this,SLOT(selectData()));

  //
  // Display Settings Section
  //
  label=new QLabel(tr("Display Settings"),this,"globals_label");
  label->setFont(big_font);
  label->setGeometry(435,411,200,16);

  //
  // Skin Path
  //
  air_skin_edit=new QLineEdit(this,"air_skin_edit");
  air_skin_edit->setGeometry(555,433,180,20);
  label=new QLabel(air_skin_edit,tr("Background Image:"),
		   this,"air_skin_label");
  label->setGeometry(435,433,115,20);
  label->setAlignment(AlignRight|AlignVCenter);
  button=new QPushButton(tr("Select"),this,"skin_select_button");
  button->setGeometry(745,430,50,25);
  connect(button,SIGNAL(clicked()),this,SLOT(selectSkinData()));

  //
  //  Ok Button
  //
  button=new QPushButton(this,"ok_button");
  button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  button->setDefault(true);
  button->setFont(small_font);
  button->setText(tr("&OK"));
  connect(button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  button=new QPushButton(this,"cancel_button");
  button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
  button->setFont(small_font);
  button->setText(tr("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Populate Fields
  //
  if(station->scanned()) {
    for(int i=0;i<10;i++) {
      air_card_sel[i]->setMaxCards(station->cards());
      for(int j=0;j<air_card_sel[i]->maxCards();j++) {
	air_card_sel[i]->setMaxPorts(j,station->cardOutputs(j));
      }
    }
  }
  else {
    QMessageBox::information(this,tr("No Audio Configuration Data"),
			    tr("Channel assignments will not be available for this host, as audio resource data\nhas not yet been generated.  Please start the Rivendell daemons on this host\n(by executing, as user 'root',  the command \"/etc/init.d/rivendell start\")\nin order to populate the audio resources database."));
    for(int i=0;i<6;i++) {
      air_card_sel[i]->setDisabled(true);
    }
  }
  for(int i=0;i<10;i++) {
    air_card_sel[i]->setCard(air_conf->card(i));
    air_card_sel[i]->setPort(air_conf->port(i));
  }
  air_startup_box->setCurrentItem(air_conf->startMode());
  air_segue_edit->setText(QString().sprintf("%d",air_conf->segueLength()));
  air_trans_edit->setText(QString().sprintf("%d",air_conf->transLength()));
  air_piecount_box->setValue(air_conf->pieCountLength()/1000);
  air_countto_box->setCurrentItem(air_conf->pieEndPoint());
  air_default_transtype_box->setCurrentItem(air_conf->defaultTransType());
  air_defaultsvc_box->insertItem(tr("[none]"));
  QString defaultsvc=air_conf->defaultSvc();
  sql=QString().sprintf("select SERVICE_NAME from SERVICE_PERMS \
                         where STATION_NAME=\"%s\"",
			(const char *)air_conf->station());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    air_defaultsvc_box->insertItem(q->value(0).toString());
    if(defaultsvc==q->value(0).toString()) {
      air_defaultsvc_box->setCurrentItem(air_defaultsvc_box->count()-1);
    }
  }
  delete q;
  air_station_box->setValue(air_conf->panels(RDAirPlayConf::StationPanel));
  air_user_box->setValue(air_conf->panels(RDAirPlayConf::UserPanel));
  air_timesync_box->setChecked(air_conf->checkTimesync());
  for(int i=0;i<2;i++) {
    air_auxlog_box[i]->setChecked(air_conf->showAuxButton(i));
  }
  air_clearfilter_box->setChecked(air_conf->clearFilter());
  air_bar_group->setButton((int)air_conf->barAction());
  air_flash_box->setChecked(air_conf->flashPanel());
  air_panel_pause_box->setChecked(air_conf->panelPauseEnabled());
  air_label_template_edit->setText(air_conf->buttonLabelTemplate());
  air_pause_box->setChecked(air_conf->pauseEnabled());
  air_show_counters_box->setChecked(air_conf->showCounters());
  air_audition_preroll_spin->setValue(air_conf->auditionPreroll()/1000);
  for(int i=0;i<10;i++) {
    air_start_rml_edit[i]->setText(air_conf->startRml(i));
    air_stop_rml_edit[i]->setText(air_conf->stopRml(i));
  }
  for(int i=0;i<RDAIRPLAY_LOG_QUANTITY;i++) {
    air_startmode[i]=air_conf->startMode(i);
    air_startlog[i]=air_conf->logName(i);
    air_autorestart[i]=air_conf->autoRestart(i);
  }
  air_startmode_box->setCurrentItem((int)air_startmode[air_logmachine]);
  air_startlog_edit->setText(air_startlog[air_logmachine]);
  air_autorestart_box->setChecked(air_autorestart[air_logmachine]);
  air_skin_edit->setText(air_conf->skinPath());
  startModeChangedData(air_startmode[air_logmachine]);
}


EditRDAirPlay::~EditRDAirPlay()
{
}


QSize EditRDAirPlay::sizeHint() const
{
  return QSize(1010,743);
} 


QSizePolicy EditRDAirPlay::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void EditRDAirPlay::exitPasswordChangedData(const QString &str)
{
  air_exitpasswd_changed=true;
}


void EditRDAirPlay::logActivatedData(int lognum)
{
  air_startmode[air_logmachine]=
    (RDAirPlayConf::StartMode)air_startmode_box->currentItem();
  air_startlog[air_logmachine]=air_startlog_edit->text();
  air_autorestart[air_logmachine]=air_autorestart_box->isChecked();

  air_logmachine=lognum;
  air_startmode_box->setCurrentItem((int)air_startmode[lognum]);
  air_startlog_edit->setText(air_startlog[lognum]);
  air_autorestart_box->setChecked(air_autorestart[lognum]);
  startModeChangedData((int)air_startmode[lognum]);
}


void EditRDAirPlay::startModeChangedData(int mode)
{
  air_startlog_edit->setEnabled((RDAirPlayConf::StartMode)mode==
				RDAirPlayConf::StartSpecified);
  air_startlog_label->setEnabled((RDAirPlayConf::StartMode)mode==
				 RDAirPlayConf::StartSpecified);
  air_startlog_button->setEnabled((RDAirPlayConf::StartMode)mode==
				 RDAirPlayConf::StartSpecified);
  air_autorestart_box->setDisabled((RDAirPlayConf::StartMode)mode==
				   RDAirPlayConf::StartEmpty);
  air_autorestart_label->setDisabled((RDAirPlayConf::StartMode)mode==
				     RDAirPlayConf::StartEmpty);
}


void EditRDAirPlay::selectData()
{
  QString logname=air_startlog_edit->text();

  RDListLogs *ll=new RDListLogs(&logname,air_conf->station(),this,
                                "log",admin_user);
  if(ll->exec()==0) {
    air_startlog_edit->setText(logname);
  }
  delete ll;
}


void EditRDAirPlay::nownextData()
{
  EditNowNext *edit=new EditNowNext(air_conf,this,"edit");
  edit->exec();
  delete edit;
}


void EditRDAirPlay::selectSkinData()
{
  QString filename=air_skin_edit->text();
  filename=QFileDialog::getOpenFileName(filename,RD_IMAGE_FILE_FILTER,this,"",
					tr("Select Image File"));
  if(!filename.isNull()) {
    air_skin_edit->setText(filename);
  }
}


void EditRDAirPlay::okData()
{
  bool ok=false;
  int segue=air_segue_edit->text().toInt(&ok);
  if(!ok) {
    QMessageBox::warning(this,tr("Data Error"),tr("Invalid Segue Length!"));
    return;
  }
  int trans=air_trans_edit->text().toInt(&ok);
  if(!ok) {
    QMessageBox::warning(this,tr("Data Error"),
			 tr("Invalid Forced Segue Length!"));
    return;
  }
  for(int i=0;i<10;i++) {
    air_conf->setStartRml(i,air_start_rml_edit[i]->text());
    air_conf->setStopRml(i,air_stop_rml_edit[i]->text());
    air_conf->setCard(i,air_card_sel[i]->card());
    air_conf->setPort(i,air_card_sel[i]->port());
  }
  air_conf->
    setStartMode((RDAirPlayConf::OpMode)air_startup_box->currentItem());
  air_conf->setSegueLength(segue);
  air_conf->setTransLength(trans);
  air_conf->setPieCountLength(air_piecount_box->value()*1000);
  air_conf->
    setPieEndPoint((RDAirPlayConf::PieEndPoint)air_countto_box->currentItem());
  air_conf->setDefaultTransType((RDLogLine::TransType)
				air_default_transtype_box->currentItem());
  if(air_defaultsvc_box->currentItem()==0) {
    air_conf->setDefaultSvc("");
  }
  else {
    air_conf->setDefaultSvc(air_defaultsvc_box->currentText());
  }
  air_conf->setPanels(RDAirPlayConf::StationPanel,air_station_box->value());
  air_conf->setPanels(RDAirPlayConf::UserPanel,air_user_box->value());
  air_conf->setCheckTimesync(air_timesync_box->isChecked());
  for(int i=0;i<2;i++) {
    air_conf->setShowAuxButton(i,air_auxlog_box[i]->isChecked());
  }
  air_conf->setClearFilter(air_clearfilter_box->isChecked());
  air_conf->
    setBarAction((RDAirPlayConf::BarAction)air_bar_group->selectedId());
  air_conf->setFlashPanel(air_flash_box->isChecked());
  air_conf->setPanelPauseEnabled(air_panel_pause_box->isChecked());
  air_conf->setButtonLabelTemplate(air_label_template_edit->text());
  air_conf->setPauseEnabled(air_pause_box->isChecked());
  air_conf->setShowCounters(air_show_counters_box->isChecked());
  air_conf->setAuditionPreroll(air_audition_preroll_spin->value()*1000);
  if(air_exitpasswd_changed) {
    air_conf->setExitPassword(air_exitpasswd_edit->text());
  }
  air_startmode[air_logmachine]=
    (RDAirPlayConf::StartMode)air_startmode_box->currentItem();
  air_startlog[air_logmachine]=air_startlog_edit->text();
  air_autorestart[air_logmachine]=air_autorestart_box->isChecked();
  for(int i=0;i<RDAIRPLAY_LOG_QUANTITY;i++) {
    air_conf->setStartMode(i,air_startmode[i]);
    air_conf->setLogName(i,air_startlog[i]);
    air_conf->setAutoRestart(i,air_autorestart[i]);
  }
  air_conf->setSkinPath(air_skin_edit->text());
  done(0);
}


void EditRDAirPlay::cancelData()
{
  done(1);
}


void EditRDAirPlay::paintEvent(QPaintEvent *e)
{
  QPainter *p=new QPainter(this);
  p->setPen(black);
  p->drawRect(25,445,395,95);
  p->end();
  delete p;
}
