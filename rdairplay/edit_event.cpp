// edit_event.cpp
//
// Event Editor for RDAirPlay
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_event.cpp,v 1.43.2.4 2010/02/08 23:50:13 cvs Exp $
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

#include <qpushbutton.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qmessagebox.h>

#include <rdconf.h>

#include <colors.h>
#include <edit_event.h>
#include <globals.h>


EditEvent::EditEvent(LogPlay *log,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  edit_log=log;
  edit_height=325;
  edit_slider_pressed=false;
  edit_shift_pressed=false;
  edit_right_click_stop=false;
  setCaption(tr("Edit Event"));

  //
  // Create Fonts
  //
  QFont radio_font=QFont("Helvetica",10,QFont::Normal);
  radio_font.setPixelSize(10);
  QFont label_font=QFont("Helvetica",12,QFont::Bold);
  label_font.setPixelSize(12);
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont counter_font=QFont("Helvetica",20,QFont::Bold);
  counter_font.setPixelSize(20);

  //
  // Create Palettes
  //
  edit_play_color=
    QPalette(QColor(BUTTON_PLAY_BACKGROUND_COLOR),backgroundColor());
  edit_start_color=palette();
  edit_start_color.setColor(QColorGroup::Foreground,EVENT_EDITOR_START_MARKER);

  //
  // Time Type
  //
  edit_timetype_box=new QCheckBox(this,"edit_timetype_box");
  edit_timetype_box->setGeometry(10,22,15,15);
  edit_timetype_label=new QLabel(edit_timetype_box,tr("Start at:"),
			   this,"edit_timetype_label");
  edit_timetype_label->setGeometry(30,21,85,17);
  edit_timetype_label->setFont(label_font);
  edit_timetype_label->setAlignment(AlignLeft|AlignVCenter);

  //
  // Start Time
  //
  edit_time_edit=new RDTimeEdit(this,"edit_time_edit");
  edit_time_edit->setGeometry(85,19,85,20);
  edit_time_edit->setDisplay(RDTimeEdit::Hours|RDTimeEdit::Minutes|
			     RDTimeEdit::Seconds|RDTimeEdit::Tenths);
  connect(edit_time_edit,SIGNAL(valueChanged(const QTime &)),
	  this,SLOT(timeChangedData(const QTime &)));

  //
  // Grace Time
  //
  edit_grace_group
    =new QButtonGroup(1,Qt::Vertical,
		      tr("Action If Previous Event Still Playing"),
		      this,"edit_grace_group");
  edit_grace_group->setGeometry(175,11,435,50);
  edit_grace_group->setFont(label_font);
  edit_grace_group->setRadioButtonExclusive(true);
  QRadioButton *radio_button=
    new QRadioButton(tr("Start Immediately"),edit_grace_group);
  edit_grace_group->insert(radio_button);
  radio_button->setFont(radio_font);
  radio_button=new QRadioButton(tr("Make Next"),edit_grace_group);
  edit_grace_group->insert(radio_button);
  radio_button->setFont(radio_font);
  radio_button=new QRadioButton(tr("Wait up to"),edit_grace_group);
  edit_grace_group->insert(radio_button);
  radio_button->setFont(radio_font);
  edit_grace_edit=new RDTimeEdit(this,"edit_grace_edit");
  edit_grace_edit->setGeometry(538,31,65,20);
  edit_grace_edit->setDisplay(RDTimeEdit::Minutes|RDTimeEdit::Seconds|
			      RDTimeEdit::Tenths);
  connect(edit_timetype_box,SIGNAL(toggled(bool)),
	  this,SLOT(timeToggledData(bool)));
  connect(edit_grace_group,SIGNAL(clicked(int)),
	  this,SLOT(graceClickedData(int)));

  //
  // Transition Type
  //
  edit_transtype_box=new QComboBox(this,"edit_transtype_box");
  edit_transtype_box->setGeometry(485,68,110,26);
  edit_transtype_box->insertItem(tr("Play"));
  edit_transtype_box->insertItem(tr("Segue"));
  edit_transtype_box->insertItem(tr("Stop"));  
  edit_time_label=new QLabel(edit_transtype_box,tr("Start Transition Type:"),
			     this,"edit_transtype_label");
  edit_time_label->setGeometry(190,68,290,26);
  edit_time_label->setFont(label_font);
  edit_time_label->setAlignment(AlignRight|AlignVCenter);

  // Overlap Box
  edit_overlap_box=new QCheckBox(this,"edit_overlap_box");
  edit_overlap_box->setGeometry(30,72,15,15);
  edit_overlap_label=new QLabel(edit_overlap_box,tr("No Fade at Segue Out"),
				this,"edit_overlap_label");
  edit_overlap_label->setGeometry(50,68,130,26);
  edit_overlap_label->setFont(button_font);
  edit_overlap_label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
  

  //
  // Horizontal Rule
  //
  QLabel *label=new QLabel(this,"horizontal_label");
  label->setGeometry(0,100,sizeHint().width(),3);
  QPixmap *pix=new QPixmap(sizeHint().width(),3);
  QPainter *p=new QPainter(pix);
  p->setPen(QColor(black));
  p->setBrush(QColor(black));
  p->fillRect(0,0,sizeHint().width(),3,backgroundColor());
  p->moveTo(10,1);
  p->lineTo(sizeHint().width()-10,1);
  p->end();
  label->setPixmap(*pix);
  delete p;
  delete pix;

  //
  // Position Widget
  //
  edit_position_label=new QLabel(this,"edit_position_label");
  edit_position_label->setGeometry(15,110,sizeHint().width()-30,30);
  edit_position_label->setBackgroundColor(QColor(white));
  edit_position_label->setLineWidth(1);
  edit_position_label->setMidLineWidth(0);
  edit_position_label->setFrameStyle(QFrame::Box|QFrame::Plain);

  edit_position_bar=new MarkerBar(this,"edit_position_bar");
  edit_position_bar->setGeometry(100,118,sizeHint().width()-200,14);

  edit_up_label=new QLabel("00:00:00",this,"edit_up_label");
  edit_up_label->setGeometry(20,118,70,14);
  edit_up_label->setBackgroundColor(white);
  edit_up_label->setFont(label_font);
  edit_up_label->setAlignment(AlignRight|AlignVCenter);

  edit_down_label=new QLabel("00:00:00",this,"edit_down_label");
  edit_down_label->setGeometry(sizeHint().width()-95,118,70,14);
  edit_down_label->setBackgroundColor(white);
  edit_down_label->setFont(label_font);
  edit_down_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Position Slider
  //
  edit_slider=new RDSlider(RDSlider::Right,this,"edit_slider");
  edit_slider->setGeometry(75,140,sizeHint().width()-150,50);
  edit_slider->setKnobSize(50,50);
  edit_slider->setKnobColor(QColor(EVENT_EDITOR_KNOB_COLOR));
  connect(edit_slider,SIGNAL(sliderMoved(int)),
	  this,SLOT(sliderChangedData(int)));
  connect(edit_slider,SIGNAL(sliderPressed()),this,SLOT(sliderPressedData()));
  connect(edit_slider,SIGNAL(sliderReleased()),
	  this,SLOT(sliderReleasedData()));

  //
  // Button Area
  //
  label=new QLabel(this,"button_area");
  label->setGeometry(15,195,sizeHint().width()-30,60);
  label->setBackgroundColor(QColor(gray));
  label->setLineWidth(1);
  label->setMidLineWidth(0);
  label->setFrameStyle(QFrame::Box|QFrame::Plain);

  //
  //  Audition Button
  //
  edit_audition_button=new RDTransportButton(RDTransportButton::PlayBetween,
					    this,"edit_audition_button");
  edit_audition_button->setGeometry(sizeHint().width()/2-130,200,80,50);
  edit_audition_button->
    setPalette(QPalette(backgroundColor(),QColor(gray)));
  edit_audition_button->setFont(button_font);
  edit_audition_button->setText(tr("&Audition"));
  connect(edit_audition_button,SIGNAL(clicked()),
	  this,SLOT(auditionButtonData()));

  //
  //  Pause Button
  //
  edit_pause_button=new RDTransportButton(RDTransportButton::Pause,
					 this,"edit_pause_button");
  edit_pause_button->setGeometry(sizeHint().width()/2-40,200,80,50);
  edit_pause_button->
    setPalette(QPalette(backgroundColor(),QColor(gray)));
  edit_pause_button->setFont(button_font);
  edit_pause_button->setText(tr("&Pause"));
  connect(edit_pause_button,SIGNAL(clicked()),this,SLOT(pauseButtonData()));

  //
  //  Stop Button
  //
  edit_stop_button=new RDTransportButton(RDTransportButton::Stop,
					this,"edit_stop_button");
  edit_stop_button->setGeometry(sizeHint().width()/2+50,200,80,50);
  edit_stop_button->setOnColor(QColor(red));
  edit_stop_button->
    setPalette(QPalette(backgroundColor(),QColor(gray)));
  edit_stop_button->setFont(button_font);
  edit_stop_button->setText(tr("&Stop"));
  connect(edit_stop_button,SIGNAL(clicked()),this,SLOT(stopButtonData()));

  //
  // Start Marker Control
  //
  edit_start_button=new RDPushButton(this,"button");
  edit_start_button->setToggleButton(true);
  edit_start_button->setGeometry(15,265,66,45);
  edit_start_button->setFlashColor(backgroundColor());
  edit_start_button->setFlashPeriod(EVENT_EDITOR_BUTTON_FLASH_PERIOD);
  edit_start_button->setPalette(QPalette(QColor(EVENT_EDITOR_START_MARKER),
					   backgroundColor()));
  edit_start_button->setFont(button_font);
  edit_start_button->setText(tr("Start"));
  connect(edit_start_button,SIGNAL(clicked()),this,SLOT(startClickedData()));

  //
  // End Marker Control
  //
  edit_end_button=new RDPushButton(this,"button");
  edit_end_button->setToggleButton(true);
  edit_end_button->setGeometry(105,265,66,45);
  edit_end_button->setFlashColor(backgroundColor());
  edit_end_button->setFlashPeriod(EVENT_EDITOR_BUTTON_FLASH_PERIOD);
  edit_end_button->setPalette(QPalette(QColor(EVENT_EDITOR_START_MARKER),
				       backgroundColor()));
  edit_end_button->setFont(button_font);
  edit_end_button->setText(tr("End"));
  connect(edit_end_button,SIGNAL(clicked()),this,SLOT(endClickedData()));

  //
  // Audition Stop Timer
  //
  edit_audition_timer=new QTimer(this,"edit_audition_timer");
  connect(edit_audition_timer,SIGNAL(timeout()),this,SLOT(auditionTimerData()));

  //
  //  Ok Button
  //
  edit_ok_button=new QPushButton(this,"edit_ok_button");
  edit_ok_button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,
			      80,50);
  edit_ok_button->setDefault(true);
  edit_ok_button->setFont(button_font);
  edit_ok_button->setText(tr("&OK"));
  connect(edit_ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  edit_cancel_button=new QPushButton(this,"edit_cancel_button");
  edit_cancel_button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
				  80,50);
  edit_cancel_button->setFont(button_font);
  edit_cancel_button->setText(tr("&Cancel"));
  connect(edit_cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Play Deck
  //
  edit_play_deck=new RDPlayDeck(rdcae,RDPLAYDECK_AUDITION_ID,
				this,"edit_play_deck");
  connect(edit_play_deck,SIGNAL(stateChanged(int,RDPlayDeck::State)),this,
	  SLOT(stateChangedData(int,RDPlayDeck::State)));
  connect(edit_play_deck,SIGNAL(position(int,int)),
	  this,SLOT(positionData(int,int)));
}


EditEvent::~EditEvent()
{
}


QSize EditEvent::sizeHint() const
{
  return QSize(625,edit_height);
} 


QSizePolicy EditEvent::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


int EditEvent::exec(int line)
{
  edit_line=line;
  edit_time_changed=false;
  if((edit_logline=edit_log->logLine(line))==NULL) {
    return -1;
  }
  switch(edit_logline->timeType()) {
      case ::RDLogLine::Hard:
	edit_timetype_box->setChecked(true);
	timeToggledData(true);
	break;

      default:
	edit_timetype_box->setChecked(false);
	timeToggledData(false);
	break;
  }
  timeChangedData(edit_time_edit->time());
  switch(edit_logline->graceTime()) {
      case -1:
	edit_grace_group->setButton(1);
	graceClickedData(1);
	break;

      case 0:
	edit_grace_group->setButton(0);
	graceClickedData(0);
	break;

      default:
	edit_grace_group->setButton(2);
	graceClickedData(2);
	edit_grace_edit->setTime(QTime().addMSecs(edit_logline->graceTime()));
	break;
  }
  edit_transtype_box->setCurrentItem((int)edit_logline->transType());
  if(edit_logline->segueStartPoint(RDLogLine::LogPointer)<0
     && edit_logline->segueEndPoint(RDLogLine::LogPointer)<0
     && edit_logline->endPoint(RDLogLine::LogPointer)<0
     && edit_logline->fadedownPoint(RDLogLine::LogPointer)<0) {
    edit_overlap_box->setEnabled(true);
    edit_overlap_label->setEnabled(true);
    if(edit_logline->segueGain()==0) {
      edit_overlap_box->setChecked(true);
    }
    else {
      edit_overlap_box->setChecked(false);
    }
  }
  else {
    edit_overlap_box->setEnabled(false);
    edit_overlap_label->setEnabled(false);
    edit_overlap_box->setChecked(false);
  }  
  if(!edit_logline->startTime(RDLogLine::Logged).isNull()) {
    edit_time_edit->setTime(edit_logline->startTime(RDLogLine::Logged));
  }
  else {
    edit_time_edit->setTime(QTime());
  }
  setCaption(QString().sprintf("%d - %s",
			      edit_logline->cartNumber(),
			      (const char *)edit_logline->title()));
  switch(edit_logline->type()) {
      case RDLogLine::Cart:
	if((edit_logline->cutNumber()<1)||
	   (edit_logline->forcedLength()<=0)) {
	  ShowAudioControls(false);
	}
	else {
	  edit_position_bar->setLength(edit_logline->forcedLength());
	  edit_start_button->setOn(false);
	  ShowAudioControls(true);
          if(!(edit_logline->status()==RDLogLine::Scheduled) && 
              !(edit_logline->status()==RDLogLine::Paused)) {
              edit_start_button->hide();
              edit_end_button->hide();
            }
          else {
              edit_start_button->show();
              edit_end_button->show();
	  }
	  edit_slider->setRange(0,edit_logline->forcedLength());
	  edit_slider->setValue(edit_logline->playPosition());
	  sliderChangedData(edit_logline->playPosition());
	  startClickedData();
	  edit_stop_button->on();
	  edit_position_bar->
	    setMarker(MarkerBar::Play,edit_logline->playPosition());
	  edit_position_bar->
	    setMarker(MarkerBar::Start,edit_logline->playPosition());
	  edit_position_bar->
	    setMarker(MarkerBar::End,edit_logline->endPoint());
	  edit_slider->setValue(edit_logline->playPosition());
	  UpdateCounters();
	}
	break;

      case RDLogLine::Marker:
	setCaption(tr("Edit Marker"));
	ShowAudioControls(false);
	break;

      case RDLogLine::Track:
	setCaption(tr("Edit Track"));
	ShowAudioControls(false);
	break;

      case RDLogLine::Chain:
	setCaption(tr("Edit Log Track"));
	ShowAudioControls(false);
	break;

      default:
	ShowAudioControls(false);
	break;
  }
  return QDialog::exec();
}


void EditEvent::timeChangedData(const QTime &time)
{
  QString str;

  if(edit_timetype_box->isChecked()) {
    str=QString(tr("Transition If Previous Cart Ends Before"));
    edit_time_label->
      setText(QString().sprintf("%s %s:",(const char *)str,
				(const char *)edit_time_edit->time().
				toString("hh:mm:ss.zzz").left(10)));
  }
}


void EditEvent::timeToggledData(bool state)
{
  QString str;

  edit_time_edit->setEnabled(state);
  edit_grace_group->setEnabled(state);
  if(state) {
    graceClickedData(edit_grace_group->selectedId());
    str=QString(tr("Transition If Previous Cart Ends Before"));
    edit_time_label->
      setText(QString().sprintf("%s %s:",(const char *)str,
	     (const char *)edit_time_edit->time().
				toString("hh:mm:ss.zzz").left(10)));
  }
  else {
    edit_grace_edit->setDisabled(true);
    edit_time_label->setText(tr("Start Transition Type:"));
  }
}


void EditEvent::graceClickedData(int id)
{
  switch(id) {
      case 0:
	edit_grace_edit->setDisabled(true);
	break;

      case 1:
	edit_grace_edit->setDisabled(true);
	break;

      case 2:
	edit_grace_edit->setEnabled(true);
	break;
  }
}


void EditEvent::sliderChangedData(int pos)
{
  if(edit_start_button->isOn()) {
    edit_position_bar->setMarker(MarkerBar::Start,pos);
  }
  else {
    if(edit_end_button->isOn()) {
      edit_position_bar->setMarker(MarkerBar::End,pos);
    }
    else {
      edit_position_bar->setMarker(MarkerBar::Play,pos);
    }
  }
  UpdateCounters();
}


void EditEvent::sliderPressedData()
{
  if(edit_play_deck->state()==RDPlayDeck::Playing) {
    edit_play_deck->stop();
    edit_slider_pressed=true;
  }
}


void EditEvent::sliderReleasedData()
{
  if(edit_slider_pressed) {
    auditionButtonData();
    edit_slider_pressed=false;
  }
}


void EditEvent::auditionButtonData()
{
  int start_pos=edit_slider->value();
  int play_len=-1;

  if(edit_play_deck->state()==RDPlayDeck::Playing) {
    return;
  }
  edit_play_deck->setCard(rdairplay_conf->card(3));
  edit_play_deck->setPort(rdairplay_conf->port(3));
  if(!edit_play_deck->setCart(edit_logline,false)) {
    return;
  }
  if(edit_start_button->isOn()) {
    if(edit_play_deck->state()==RDPlayDeck::Stopped) {
      start_pos=edit_position_bar->marker(MarkerBar::Start);
    }
    if(edit_play_deck->state()==RDPlayDeck::Paused) {
      start_pos=edit_play_deck->currentPosition();
    }
    play_len=edit_position_bar->marker(MarkerBar::End)-start_pos;
  }
  else {
    if(edit_end_button->isOn()) {
      if(edit_play_deck->state()==RDPlayDeck::Stopped) {
	play_len=rdairplay_conf->auditionPreroll();
	if(play_len>(edit_position_bar->marker(MarkerBar::End)-
		     edit_position_bar->marker(MarkerBar::Start))) {
	  play_len=edit_position_bar->marker(MarkerBar::End)-
	    edit_position_bar->marker(MarkerBar::Start);
	}
	start_pos=edit_position_bar->marker(MarkerBar::End)-play_len;	  
      }
    }
    else {
      if((edit_play_deck->state()==RDPlayDeck::Stopped)&&
	 (!edit_slider_pressed)) {
	edit_start_pos=edit_slider->value();
      }
    }
  }
  edit_play_deck->play(start_pos);
  if(play_len>=0) {
    edit_audition_timer->start(play_len,true);
  }
  QString rml=rdairplay_conf->startRml(3);
  if(!rml.isEmpty()) {
    rdevent_player->exec(edit_logline->resolveWildcards(rml));
  }
}


void EditEvent::pauseButtonData()
{
  if(edit_play_deck->state()==RDPlayDeck::Playing) {
    edit_play_deck->pause();
  }
}


void EditEvent::stopButtonData()
{
  switch(edit_play_deck->state()) {
      case RDPlayDeck::Playing:
      case RDPlayDeck::Paused:
	edit_play_deck->stop();
	break;

      default:
	break;
  }
}


void EditEvent::stateChangedData(int id,RDPlayDeck::State state)
{
  if(id!=RDPLAYDECK_AUDITION_ID) {
    return;
  }
  switch(state) {
      case RDPlayDeck::Playing:
	Playing(id);
	break;

      case RDPlayDeck::Stopping:
	break;

      case RDPlayDeck::Paused:
	Paused(id);
	break;

      case RDPlayDeck::Stopped:
	Stopped(id);
	break;

      case RDPlayDeck::Finished:
	Stopped(id);
	break;
  }
}


void EditEvent::positionData(int id,int msecs)
{
  if(id!=RDPLAYDECK_AUDITION_ID) {
    return;
  }
  edit_position_bar->setMarker(MarkerBar::Play,msecs);
  if((!edit_start_button->isOn())&&(!edit_end_button->isOn())) {
    edit_slider->setValue(msecs);
  }
  UpdateCounters();
}


void EditEvent::auditionTimerData()
{
  edit_play_deck->stop();
}


void EditEvent::okData()
{
  if(edit_timetype_box->isChecked()&&
     edit_log->exists(edit_time_edit->time(),edit_line)) {
    QMessageBox::warning(this,tr("Duplicate Start Time"),
	       	 tr("An event is already scheduled with this start time!"));
    return;
  }
  if(edit_play_deck->state()==RDPlayDeck::Playing) {
    edit_play_deck->stop();
  }
  if((edit_logline->status()==RDLogLine::Scheduled)||
     (edit_logline->status()==RDLogLine::Paused)) {
    if(edit_timetype_box->isChecked()) {
      edit_logline->setTimeType(RDLogLine::Hard);
      switch(edit_grace_group->selectedId()) {
	  case 0:
	    edit_logline->setGraceTime(0);
	    break;

	  case 1:
	    edit_logline->setGraceTime(-1);
	    break;

	  case 2:
	    edit_logline->
	      setGraceTime(QTime().msecsTo(edit_grace_edit->time()));
	    break;
      }
    }
    else {
      edit_logline->setTimeType(RDLogLine::Relative);
      edit_logline->setStartTime(RDLogLine::Logged,edit_logline->
				 startTime(RDLogLine::Imported));
    }
    edit_logline->
      setTransType((RDLogLine::TransType)edit_transtype_box->currentItem());
    if(edit_logline->segueStartPoint(RDLogLine::LogPointer)<0
       && edit_logline->segueEndPoint(RDLogLine::LogPointer)<0
       && edit_logline->endPoint(RDLogLine::LogPointer)<0
       && edit_logline->fadedownPoint(RDLogLine::LogPointer)<0) {
      if(edit_overlap_box->isChecked()) {
        edit_logline->setSegueGain(0);
      }
      else {
        edit_logline->setSegueGain(RD_FADE_DEPTH);
      }
    }
    if(edit_time_changed||
       (edit_logline->timeType()!=RDLogLine::Relative)) {
      edit_logline->
	setStartTime(RDLogLine::Logged,edit_time_edit->time());
    }
    if((unsigned)edit_position_bar->marker(MarkerBar::Start)!=
       edit_logline->playPosition()) {
      edit_logline->
	setPlayPosition(edit_position_bar->marker(MarkerBar::Start));
      edit_logline->setPlayPositionChanged(true);
    }
    if((unsigned)edit_position_bar->marker(MarkerBar::End)!=
       edit_logline->endPoint()) {
      edit_logline->setEndPoint(edit_position_bar->marker(MarkerBar::End),
				RDLogLine::LogPointer);
      edit_logline->setPlayPositionChanged(true);
    }
    edit_log->lineModified(edit_line);
  }
  done(0);
}


void EditEvent::cancelData()
{
  if(edit_play_deck->state()==RDPlayDeck::Playing) {
    edit_play_deck->stop();
  }
  done(1);
}


void EditEvent::startClickedData()
{
  if(edit_end_button->isOn()) {
    edit_end_button->toggle();
    SetEndMode(false);
  }
  SetStartMode(edit_start_button->isOn());
}


void EditEvent::endClickedData()
{
  if(edit_start_button->isOn()) {
    edit_start_button->toggle();
    SetStartMode(false);
  }
  SetEndMode(edit_end_button->isOn());
}


void EditEvent::closeEvent(QCloseEvent *e)
{
  cancelData();
}


void EditEvent::SetStartMode(bool state)
{
  if(state) {
    edit_slider->setRange(0,edit_position_bar->marker(MarkerBar::End));
    edit_slider->setGeometry(75,140,
			     (int)(50.0+((double)(sizeHint().width()-200)*
					 (double)edit_position_bar->
					 marker(MarkerBar::End)/
					 (double)edit_logline->
					 forcedLength())),50);
    edit_slider->setValue(edit_position_bar->marker(MarkerBar::Start));
    edit_slider->setKnobColor(EVENT_EDITOR_START_MARKER);
    edit_audition_button->setAccentColor(EVENT_EDITOR_START_MARKER);
    edit_start_button->setFlashingEnabled(true);
    edit_up_label->setPalette(edit_start_color);
    edit_down_label->setPalette(edit_start_color);
    UpdateCounters();
  }
  else {
    edit_slider->setRange(0,edit_logline->forcedLength());
    edit_slider->setGeometry(75,140,sizeHint().width()-150,50);
    edit_slider->setValue(edit_position_bar->marker(MarkerBar::Play));
    edit_slider->setKnobColor(EVENT_EDITOR_PLAY_MARKER); 
    edit_audition_button->setAccentColor(EVENT_EDITOR_PLAY_MARKER);
    edit_start_button->setFlashingEnabled(false);
    edit_up_label->setPalette(palette());
    edit_down_label->setPalette(palette());
    UpdateCounters();
  }
}


void EditEvent::SetEndMode(bool state)
{
  if(state) {
    edit_slider->setRange(edit_position_bar->marker(MarkerBar::Start),
			  edit_logline->forcedLength());
    edit_slider->setGeometry((int)(75.0+(double)(sizeHint().width()-200)*
				   (double)edit_position_bar->
				   marker(MarkerBar::Start)/
				   (double)edit_logline->forcedLength()),
			     140,(int)(50.0+((double)(sizeHint().width()-200)*
					     ((double)edit_logline->
					      forcedLength()-
					      (double)edit_position_bar->
					      marker(MarkerBar::Start))/
					     (double)edit_logline->
					     forcedLength())),50);
    edit_slider->setValue(edit_position_bar->marker(MarkerBar::End));
    edit_slider->setKnobColor(EVENT_EDITOR_START_MARKER);
    edit_audition_button->setAccentColor(EVENT_EDITOR_START_MARKER);
    edit_end_button->setFlashingEnabled(true);
    edit_up_label->setPalette(edit_start_color);
    edit_down_label->setPalette(edit_start_color);
    UpdateCounters();
  }
  else {
    edit_slider->setRange(0,edit_logline->forcedLength());
    edit_slider->setGeometry(75,140,sizeHint().width()-150,50);
    edit_slider->setValue(edit_position_bar->marker(MarkerBar::Play));
    edit_slider->setKnobColor(EVENT_EDITOR_PLAY_MARKER); 
    edit_audition_button->setAccentColor(EVENT_EDITOR_PLAY_MARKER);
    edit_end_button->setFlashingEnabled(false);
    edit_up_label->setPalette(palette());
    edit_down_label->setPalette(palette());
    UpdateCounters();
  }
}


void EditEvent::ShowAudioControls(bool state)
{
  if(state) {
    edit_height=325;
    edit_slider->show();
    edit_up_label->show();
    edit_down_label->show();
    edit_audition_button->show();
    edit_pause_button->show();
    edit_position_bar->show();
    edit_position_label->show();
  }
  else {
    edit_height=170;
    edit_slider->hide();
    edit_up_label->hide();
    edit_down_label->hide();
    edit_audition_button->hide();
    edit_pause_button->hide();
    edit_position_bar->hide();
    edit_position_label->hide();
  }
  edit_ok_button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,
			      80,50);
  edit_cancel_button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
				  80,50);
  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
  resize(sizeHint());
}


void EditEvent::Playing(int id)
{
  edit_audition_button->on();
  edit_pause_button->off();
  edit_stop_button->off();
  edit_right_click_stop=true;
}


void EditEvent::Paused(int id)
{
  if(!edit_slider_pressed) {
    edit_audition_button->off();
    edit_pause_button->on();
    edit_stop_button->off();
    ClearChannel();
    edit_right_click_stop=false;
  }
}


void EditEvent::Stopped(int id)
{
  if(!edit_slider_pressed) {
    edit_audition_button->off();
    edit_pause_button->off();
    edit_stop_button->on();
    ClearChannel();
    edit_right_click_stop=false;
  }
  if(edit_start_button->isOn()) {
    edit_position_bar->
      setMarker(MarkerBar::Play,edit_position_bar->marker(MarkerBar::Start));
    edit_slider->setValue(edit_position_bar->marker(MarkerBar::Start));
  }
  else {
    if(edit_end_button->isOn()) {
      edit_slider->setValue(edit_position_bar->marker(MarkerBar::End));
    }
    else {
      edit_position_bar->setMarker(MarkerBar::Play,edit_start_pos);
      edit_slider->setValue(edit_start_pos);
    }
  }
}


void EditEvent::UpdateCounters()
{
  if(edit_start_button->isOn()) {
   edit_up_label->
     setText(RDGetTimeLength(edit_position_bar->marker(MarkerBar::Start),true));
   edit_down_label->
     setText(RDGetTimeLength(edit_logline->
			    forcedLength()-edit_position_bar->
			    marker(MarkerBar::Start),true));
  }
  else {
    if(edit_end_button->isOn()) {
      edit_up_label->
	setText(RDGetTimeLength(edit_position_bar->marker(MarkerBar::End),
				true));
      edit_down_label->
	setText(RDGetTimeLength(edit_logline->
				forcedLength()-edit_position_bar->
				marker(MarkerBar::End),true));
    }
    else {
      edit_up_label->
	setText(RDGetTimeLength(edit_position_bar->marker(MarkerBar::Play),
				true));
      edit_down_label->
	setText(RDGetTimeLength(edit_logline->
				forcedLength()-edit_position_bar->
				marker(MarkerBar::Play),true));
    }
  }
}


void EditEvent::ClearChannel()
{
  if(rdcae->playPortActive(edit_play_deck->card(),edit_play_deck->port(),
			   edit_play_deck->stream())) {
    return;
  }
  rdevent_player->exec(rdairplay_conf->stopRml(3));
}


void EditEvent::wheelEvent(QWheelEvent *e)
{
  if(edit_audition_button->isShown()) {
    if(edit_play_deck->state()==RDPlayDeck::Playing) {
      edit_play_deck->pause();
    }
    if(edit_shift_pressed) {
      edit_slider->setValue(edit_slider->value()+(e->delta()*10)/12);
      }
    else {
      edit_slider->setValue(edit_slider->value()+(e->delta()*100)/12);
      }
    sliderChangedData(edit_slider->value());
  }
}


void EditEvent::mousePressEvent(QMouseEvent *e)
{
  switch(e->button()) {
      case QMouseEvent::RightButton:
        if(edit_audition_button->isShown()) {
          if(edit_right_click_stop) {
            stopButtonData();
            }
          else {
 	    auditionButtonData();
            }
          }
        break;

      case QMouseEvent::MidButton:
        if(edit_audition_button->isShown()) {
          if(edit_logline->forcedLength()>10000) {
            if(edit_play_deck->state()==RDPlayDeck::Playing) {
              edit_play_deck->pause();
              }
            edit_slider->setValue((edit_logline->forcedLength())-10000);
            sliderChangedData(edit_slider->value());
            }
          auditionButtonData();
          }
        break;

      default:
	QWidget::mousePressEvent(e);
	break;
  }
}


void EditEvent::keyPressEvent(QKeyEvent *e)
{
  switch(e->key()) {
      case Qt::Key_Shift:
        edit_shift_pressed=true;
  	break;

      default:
	e->ignore();
	break;
  }
}


void EditEvent::keyReleaseEvent(QKeyEvent *e)
{
  switch(e->key()) {
      case Qt::Key_Shift:
        edit_shift_pressed=false;
	QWidget::keyPressEvent(e);
  	break;

      default:
	QWidget::keyPressEvent(e);
	break;
  }
}
