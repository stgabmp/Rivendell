// rdsound_panel.cpp
//
// The sound panel widget for RDAirPlay
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdsound_panel.cpp,v 1.53.2.3 2009/03/26 23:14:04 cvs Exp $
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

#include <qsignalmapper.h>
#include <rddb.h>
#include <rdlog_line.h>
#include <rdsound_panel.h>
#include <rdbutton_dialog.h>
#include <rdmacro.h>
#include <rdcut.h>
#include <rdedit_panel_name.h>
#include <rdescape_string.h>
#include <rdconf.h>

RDSoundPanel::RDSoundPanel(int cols,int rows,int station_panels,
			   int user_panels,bool flash,
			   const QString &label_template,bool extended,
			   RDEventPlayer *player,RDRipc *ripc,RDCae *cae,
			   RDStation *station,RDCartDialog *cart_dialog,
			   QWidget *parent,const char *name,int button_x_size)
  : QWidget(parent,name)
{
  panel_button_x_size=button_x_size;
  panel_playmode_box=NULL;
  panel_button_columns=cols;
  panel_button_rows=rows;
  panel_cue_port=-1;
  if(extended) {
    panel_tablename="EXTENDED_PANELS";
    panel_name_tablename="EXTENDED_PANEL_NAMES";
  }
  else {
    panel_tablename="PANELS";
    panel_name_tablename="PANEL_NAMES";
  }
  panel_label_template=label_template;

  panel_type=RDAirPlayConf::StationPanel;
  panel_number=0;
  panel_setup_mode=false;
  panel_reset_mode=false;
  panel_parent=parent;
  panel_cae=cae;
  panel_user=NULL;
  panel_ripc=ripc;
  panel_station=station;
  panel_station_panels=station_panels;
  panel_user_panels=user_panels;
  panel_event_player=player;
  panel_action_mode=RDAirPlayConf::Normal;
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    panel_active_buttons[i]=NULL;
  }
  panel_flash=flash;
  panel_flash_count=0;
  panel_flash_state=false;
  panel_config_panels=false;
  panel_pause_enabled=false;
  for(unsigned i=0;i<PANEL_MAX_OUTPUTS;i++) {
    panel_card[i]=-1;
    panel_port[i]=-1;
  }
  panel_cart_dialog=cart_dialog;
  for(int i=0;i<PANEL_MAX_OUTPUTS;i++) {
    panel_timescaling_supported[i]=false;
  }
  panel_onair_flag=false;
  panel_sizehint_width=panel_button_columns*(panel_button_x_size+15);
  panel_sizehint_height=panel_button_rows*(PANEL_BUTTON_SIZE_Y+15)+50;

  panel_area=new QFrame(this);

  //
  // Create Fonts
  //
  QFont button_font=QFont("Helvetica",14,QFont::Bold);
  button_font.setPixelSize(14);

  //
  // Load Buttons
  //
  panel_mapper=new QSignalMapper(this,"panel_mapper");
  connect(panel_mapper,SIGNAL(mapped(int)),this,SLOT(buttonMapperData(int)));

  LoadPanels();

  //
  // Panel Selector
  //
  panel_selector_box=new RDComboBox(this,"panel_selector_box");
  panel_selector_box->setFont(button_font);
  connect(panel_selector_box,SIGNAL(activated(int)),
	  this,SLOT(panelActivatedData(int)));
  connect(panel_selector_box,SIGNAL(setupClicked()),
	  this,SLOT(panelSetupData()));

  if(panel_station_panels>0) {
    panel_number=0;
    panel_type=RDAirPlayConf::StationPanel;
    panel_buttons[0].show();
  }
  else {
    if(panel_user_panels>0) {
      panel_number=0;
      panel_type=RDAirPlayConf::UserPanel;
      panel_buttons[0].show();
    }
    else {
      setDisabled(true);
    }
  }
  
  //
  // Play Mode Box
  //
  panel_playmode_box=new QComboBox(this,"panel_playmode_box");
  panel_playmode_box->setFont(button_font);
  connect(panel_playmode_box,SIGNAL(activated(int)),
	  this,SLOT(playmodeActivatedData(int)));
  panel_playmode_box->insertItem(tr("Play All"));
  panel_playmode_box->insertItem(tr("Play Hook"));

  //
  // Reset Button
  //
  panel_reset_button=new RDPushButton(this,"reset_button");
  panel_reset_button->setFont(button_font);
  panel_reset_button->setText(tr("Reset"));
  panel_reset_button->setFlashColor(QColor(RDPANEL_RESET_FLASH_COLOR));
  panel_reset_button->setFocusPolicy(QWidget::NoFocus);
  connect(panel_reset_button,SIGNAL(clicked()),this,SLOT(resetClickedData()));

  //
  // All Button
  //
  panel_all_button=new RDPushButton(this,"all_button");
  panel_all_button->setFont(button_font);
  panel_all_button->setText(tr("All"));
  panel_all_button->setFlashColor(QColor(RDPANEL_RESET_FLASH_COLOR));
  panel_all_button->setFocusPolicy(QWidget::NoFocus);
  panel_all_button->hide();
  connect(panel_all_button,SIGNAL(clicked()),this,SLOT(allClickedData()));

  //
  // Setup Button
  //
  panel_setup_button=new RDPushButton(this,"setup_button");
  panel_setup_button->setFont(button_font);
  panel_setup_button->setText(tr("Setup"));
  panel_setup_button->setFlashColor(QColor(RDPANEL_SETUP_FLASH_COLOR));
  panel_setup_button->setFocusPolicy(QWidget::NoFocus);
  connect(panel_setup_button,SIGNAL(clicked()),this,SLOT(setupClickedData()));

  //
  // Button Dialog Box
  //
  panel_button_dialog=
    new RDButtonDialog(panel_station->name(),panel_label_template,
		       panel_cart_dialog,panel_svcname,this,
		       "panel_button_dialog");

  //
  // CAE Setup
  //
  connect(panel_cae,SIGNAL(timescalingSupported(int,bool)),
	  this,SLOT(timescalingSupportedData(int,bool)));

  //
  // RIPC Setup
  //
  connect(panel_ripc,SIGNAL(onairFlagChanged(bool)),
	  this,SLOT(onairFlagChangedData(bool)));

  //
  // Load Panel Names
  //

  QString sql;
  sql=QString().sprintf("select PANEL_NO,NAME from %s \
                         where (TYPE=%d)&&(OWNER=\"%s\") order by PANEL_NO",
			(const char *)panel_name_tablename,
			RDAirPlayConf::StationPanel,
			(const char *)panel_station->name());
  RDSqlQuery *q=new RDSqlQuery(sql);
  q->first();
  for(int i=0;i<panel_station_panels;i++) {
    if(q->isValid()&&(q->value(0).toInt()==i)) {
      panel_selector_box->
	insertItem(QString().sprintf("[S:%d] %s",i+1,
				     (const char *)q->value(1).toString()));
      q->next();
    }
    else {
      panel_selector_box->insertItem(QString().sprintf("[S:%d] Panel S:%d",
						       i+1,i+1));
    }
  }
  delete q;
  for(int i=0;i<panel_user_panels;i++) {
    panel_selector_box->insertItem(QString().sprintf("[U:%d] Panel U:%d",
						     i+1,i+1));
  }
}

QSize RDSoundPanel::sizeHint() const
{
  return QSize(panel_sizehint_width,panel_sizehint_height);
}

void RDSoundPanel::setSizeHint(int width,int height)
{
  panel_sizehint_width=width;
  panel_sizehint_height=height;
}



QSizePolicy RDSoundPanel::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


int RDSoundPanel::card(int outnum) const
{
  return panel_card[outnum];
}


void RDSoundPanel::setCard(int outnum,int card)
{
  panel_card[outnum]=card;
  panel_cae->requestTimescale(card);
}


int RDSoundPanel::port(int outnum) const
{
  return panel_port[outnum];
}


void RDSoundPanel::setPort(int outnum,int port)
{
  panel_port[outnum]=port;
}


QString RDSoundPanel::outputText(int outnum) const
{
  return panel_output_text[outnum];
}


void RDSoundPanel::setOutputText(int outnum,const QString &text)
{
  panel_output_text[outnum]=text;
}


void RDSoundPanel::setRmls(int outnum,const QString &start_rml,
			   const QString &stop_rml)
{
  panel_start_rml[outnum]=start_rml;
  panel_stop_rml[outnum]=stop_rml;
}


void RDSoundPanel::setLogName(const QString &logname)
{
  panel_logname=logname;
}


void RDSoundPanel::setSvcName(const QString &svcname)
{
  panel_svcname=svcname;
  panel_svcname.replace(" ","_");
}


void RDSoundPanel::setButton(RDAirPlayConf::PanelType type,int panel,
			   int row,int col,unsigned cartnum)
{
  QString str;

  RDPanelButton *button=
    panel_buttons[PanelOffset(type,panel)].panelButton(row,col);
  if(button->playDeck()!=NULL) {
    return;
  }
  button->clear();
  if(cartnum>0) {
    button->setCart(cartnum);
    RDCart *cart=new RDCart(cartnum);
    if(cart->exists()) {
      button->
	setText(RDLogLine::resolveWildcards(cartnum,panel_label_template));
      button->setLength(false,cart->forcedLength());
      if(cart->averageHookLength()>0) {
	button->setLength(true,cart->averageHookLength());
      }
      else {
	button->setLength(true,cart->forcedLength());
      }
      button->setHookMode(panel_playmode_box->currentItem()==1);
      button->setActiveLength(button->length(button->hookMode()));
    }
    else {
      str=QString(tr("Cart"));
      button->setText(QString().sprintf("%s %06u",(const char *)str,cartnum));
    }
    delete cart;
  }
  SaveButton(type,panel,row,col);
}


void RDSoundPanel::setLogfile(QString filename)
{
  panel_logfile=filename;
}


void RDSoundPanel::play(RDAirPlayConf::PanelType type,int panel,
			int row, int col,RDLogLine::StartSource src,int mport,
                        bool pause_when_finished)
{
  PlayButton(type,panel,row,col,src,panel_playmode_box->currentItem()==1,
	     mport,pause_when_finished);
}


bool RDSoundPanel::pause(RDAirPlayConf::PanelType type,int panel,
			 int row,int col,int mport)
{
  if(panel_pause_enabled) {
    PauseButton(type,panel,row,col,mport);
    return true;
  }
  return false;
}


void RDSoundPanel::stop(RDAirPlayConf::PanelType type,int panel,
			int row,int col,
                        int mport,bool pause_when_finished,int fade_out)
{
  StopButton(type,panel,row,col,mport,pause_when_finished,fade_out);
}


void RDSoundPanel::duckVolume(RDAirPlayConf::PanelType type,int panel,int row,int col,
		  int level,int fade,int mport)
{
  int edit_mport=mport;
  if (edit_mport==0) {
    edit_mport=-1;
    }
    for(int i=0;i<panel_button_columns;i++) {
	    for(int j=0;j<panel_button_rows;j++) {
      RDPlayDeck *deck=
        panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->playDeck();
      if((row==j || row==-1) && (col==i || col==-1)) {
	if(mport==-1) {
	  panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->setDuckVolume(level);
	}    
        if(deck!=NULL) {
          if(edit_mport==-1 || 
             edit_mport==panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->
                     output()+1) {
	    deck->duckVolume(level,fade);
          }
        }
      }
    }
  }
}


RDAirPlayConf::ActionMode RDSoundPanel::actionMode() const
{
  return panel_action_mode;
}


void RDSoundPanel::setActionMode(RDAirPlayConf::ActionMode mode)
{
  if(panel_setup_mode) {
    return;
  }
  switch(mode) {
      case RDAirPlayConf::CopyFrom:
	mode=RDAirPlayConf::CopyFrom;
	break;

      case RDAirPlayConf::CopyTo:
	mode=RDAirPlayConf::CopyTo;
	break;

      case RDAirPlayConf::AddTo:
	mode=RDAirPlayConf::AddTo;
	break;

      case RDAirPlayConf::DeleteFrom:
	mode=RDAirPlayConf::DeleteFrom;
	break;

      default:
	mode=RDAirPlayConf::Normal;
	break;
  }
  if(mode!=panel_action_mode) {
    panel_action_mode=mode;
    panel_setup_button->setEnabled(panel_action_mode==RDAirPlayConf::Normal);
    for(unsigned i=0;i<panel_buttons.size();i++) {
      if(i<(unsigned)panel_station_panels &&
          (!panel_config_panels) &&   
          (mode==RDAirPlayConf::AddTo || mode==RDAirPlayConf::CopyTo || mode==RDAirPlayConf::DeleteFrom)) {
        panel_buttons[i].setActionMode(RDAirPlayConf::Normal);
        }
      else {
        panel_buttons[i].setActionMode(panel_action_mode);
        }
      }
    }
}


bool RDSoundPanel::pauseEnabled() const
{
  return panel_pause_enabled;
}


void RDSoundPanel::setPauseEnabled(bool state)
{
  if(state) {
    panel_reset_button->show();
  }
  else {
    panel_reset_button->hide();
  }
  panel_pause_enabled=state;
}


int RDSoundPanel::currentNumber() const
{
   return panel_number;
}


RDAirPlayConf::PanelType RDSoundPanel::currentType() const
{
   return panel_type;
}


void RDSoundPanel::changeUser()
{
  if(panel_user!=NULL) {
    delete panel_user;
  }
  panel_user=new RDUser(panel_ripc->user());
  panel_config_panels=panel_user->configPanels();
  LoadPanels();
  panel_buttons[PanelOffset(panel_type,panel_number)].show();

  //
  // Remove Old Panel Names
  //
  int current_item=panel_selector_box->currentItem();
  for(int i=0;i<panel_user_panels;i++) {
    panel_selector_box->removeItem(panel_station_panels);
  }

  //
  // Load New Panel Names
  //
  QString sql;
  sql=QString().sprintf("select PANEL_NO,NAME from %s \
                         where (TYPE=%d)&&(OWNER=\"%s\") order by PANEL_NO",
			(const char *)panel_name_tablename,
			RDAirPlayConf::UserPanel,
			(const char *)panel_user->name());
  RDSqlQuery *q=new RDSqlQuery(sql);
  q->first();
  for(int i=0;i<panel_user_panels;i++) {
    if(q->isValid()&&(q->value(0).toInt()==i)) {
      panel_selector_box->
	insertItem(QString().sprintf("[U:%d] %s",i+1,
				     (const char *)q->value(1).toString()));
      q->next();
    }
    else {
      panel_selector_box->insertItem(QString().sprintf("[U:%d] Panel U:%d",
						       i+1,i+1));
    }
  }
  delete q;
  panel_selector_box->setCurrentItem(current_item);
}


void RDSoundPanel::tickClock()
{
  emit tick();
  if(panel_flash) {
    if(panel_flash_count++>1) {
      emit buttonFlash(panel_flash_state);
      panel_flash_state=!panel_flash_state;
      panel_flash_count=0;
    }
  }
}


void RDSoundPanel::panelActivatedData(int n)
{
  panel_buttons[PanelOffset(panel_type,panel_number)].hide();
  if(n<panel_station_panels) {
    panel_type=RDAirPlayConf::StationPanel;
    panel_number=n;
  }
  else {
    panel_type=RDAirPlayConf::UserPanel;
    panel_number=n-panel_station_panels;
  }
  panel_buttons[PanelOffset(panel_type,panel_number)].show();
}


void RDSoundPanel::resetClickedData()
{
  if(panel_reset_mode) {
    panel_reset_mode=false;
    panel_reset_button->setFlashingEnabled(false);
    panel_all_button->hide();
    panel_setup_button->show();
  }
  else {
    panel_reset_mode=true;
    panel_reset_button->setFlashingEnabled(true);
    panel_setup_button->hide();
    panel_all_button->show();
  }
}


void RDSoundPanel::allClickedData()
{
    StopButton(panel_type,panel_number,-1,-1);
}


void RDSoundPanel::playmodeActivatedData(int n)
{
  LoadPanel(panel_type,panel_number);
}


void RDSoundPanel::setupClickedData()
{
  if(panel_setup_mode) {
    panel_setup_mode=false;
    panel_setup_button->setFlashingEnabled(false);
    panel_reset_button->setEnabled(true);
    panel_playmode_box->setEnabled(true);
    emit releaseKey();
  }
  else {
    panel_setup_mode=true;
    panel_setup_button->setFlashingEnabled(true);
    panel_reset_button->setDisabled(true);
    panel_playmode_box->setDisabled(true);
    emit getKey();
  }
  panel_selector_box->setSetupMode(panel_setup_mode);
}


void RDSoundPanel::buttonMapperData(int id)
{
  int row=id/panel_button_columns;
  int col=id-row*panel_button_columns;
  unsigned cartnum;

  switch(panel_action_mode) {
      case RDAirPlayConf::CopyFrom:
	if((cartnum=panel_buttons[PanelOffset(panel_type,panel_number)].
	    panelButton(row,col)->cart())>0) {
	  emit selectClicked(cartnum,0,0);
	}
	break;
	
      case RDAirPlayConf::CopyTo:
         if(panel_buttons[PanelOffset(panel_type,panel_number)].
	    panelButton(row,col)->playDeck()==NULL
             && ((panel_type==RDAirPlayConf::UserPanel) || 
		 panel_config_panels)) { 
	   emit selectClicked(0,row,col);
           }
	break;
	
      case RDAirPlayConf::AddTo:
         if(panel_buttons[PanelOffset(panel_type,panel_number)].
	    panelButton(row,col)->playDeck()==NULL
             && ((panel_type==RDAirPlayConf::UserPanel) || 
		 panel_config_panels)) { 
	   emit selectClicked(0,row,col);
           }
	break;
	
      case RDAirPlayConf::DeleteFrom:
         if(panel_buttons[PanelOffset(panel_type,panel_number)].
	    panelButton(row,col)->playDeck()==NULL
             && ((panel_type==RDAirPlayConf::UserPanel) || 
		 panel_config_panels)) { 
	   emit selectClicked(0,row,col);
           }
	break;
	
      default:
	if(panel_setup_mode) {
	  if((panel_type==RDAirPlayConf::StationPanel)&&
	     (!panel_config_panels)) {
	    ClearReset();
	    return;
	  }
	  if(panel_button_dialog->
	     exec(panel_buttons[PanelOffset(panel_type,panel_number)].
		  panelButton(row,col),panel_playmode_box->currentItem()==1)
	     ==0) {
	    SaveButton(panel_type,panel_number,row,col);
	  }
	}
	else {
	  RDPanelButton *button=
	    panel_buttons[PanelOffset(panel_type,panel_number)].
	    panelButton(row,col);
	  RDPlayDeck *deck=button->playDeck();
	  if(panel_reset_mode) {
	    StopButton(panel_type,panel_number,row,col);
	  }
	  else {
	    if(deck==NULL) {
	      PlayButton(panel_type,panel_number,row,col,
			 RDLogLine::StartManual,
			 panel_playmode_box->currentItem()==1);
	    }
	    else {
	      if(panel_pause_enabled) {
		if(deck->state()!=RDPlayDeck::Paused) {
		  PauseButton(panel_type,panel_number,row,col);
		}
		else {
		  PlayButton(panel_type,panel_number,row,col,
			     RDLogLine::StartManual,button->hookMode());
		}
	      }
	      else {
		StopButton(panel_type,panel_number,row,col);
	      }
	    }
	  }
	}
  }
  ClearReset();
}


void RDSoundPanel::stateChangedData(int id,RDPlayDeck::State state)
{
  switch(state) {
      case RDPlayDeck::Playing:
	Playing(id);
	break;

      case RDPlayDeck::Stopped:
      case RDPlayDeck::Finished:
        Stopped(id);
	break;

      case RDPlayDeck::Paused:
	Paused(id);
	break;

      default:
	break;
  }
}


void RDSoundPanel::hookEndData(int id)
{
  RDPanelButton *button=panel_active_buttons[id];
  if(!button->hookMode()) {
    return;
  }
  RDPlayDeck *deck=button->playDeck();
  if(deck!=NULL) {
    switch(deck->state()) {
	case RDPlayDeck::Playing:
	case RDPlayDeck::Paused:
	  StopButton(id);
	  break;
	  
	default:
	  break;
    }
  }
}


void RDSoundPanel::timescalingSupportedData(int card,bool state)
{
  for(unsigned i=0;i<PANEL_MAX_OUTPUTS;i++) {
    if(card==panel_card[i]) {
      panel_timescaling_supported[i]=state;
    }
  }
}


void RDSoundPanel::panelSetupData()
{
  QString sql;
  RDSqlQuery *q;
  int cutpt=panel_selector_box->currentText().find(" ");
  if(panel_selector_box->currentText().left(5)==tr("Panel")) {
    cutpt=-1;
  }
  QString panel_name=panel_selector_box->currentText().right(panel_selector_box->currentText().length()-cutpt-1);
  RDEditPanelName *edn=new RDEditPanelName(&panel_name);
  if(edn->exec()==0) {
    panel_selector_box->
      setCurrentText(QString().sprintf("[%s] %s",
	     (const char *)PanelTag(panel_selector_box->currentItem()),
	     (const char *)panel_name));
    sql=QString().sprintf("delete from %s where \
                           (TYPE=%d)&&(OWNER=\"%s\")&&(PANEL_NO=%d)",
			  (const char *)panel_name_tablename,
			  panel_type,(const char *)PanelOwner(panel_type),
			  panel_number);
    q=new RDSqlQuery(sql);
    delete q;
    sql=QString().sprintf("insert into %s set TYPE=%d,OWNER=\"%s\",\
                           PANEL_NO=%d,NAME=\"%s\"",
			  (const char *)panel_name_tablename,panel_type,
			  (const char *)PanelOwner(panel_type),panel_number,
			  (const char *)RDEscapeString(panel_name));
    q=new RDSqlQuery(sql);
    delete q;
  }
  delete edn;
}


void RDSoundPanel::onairFlagChangedData(bool state)
{
  panel_onair_flag=state;
}


void RDSoundPanel::PlayButton(RDAirPlayConf::PanelType type,int panel,
		int row,int col,RDLogLine::StartSource src,bool hookmode,
		int mport,bool pause_when_finished)
{
  int edit_row=row;
  int edit_col=col;
  
  for(int i=0;i<panel_button_columns;i++) {
    for(int j=0;j<panel_button_rows;j++) {
      if(panel_buttons[PanelOffset(type,panel)].
	 panelButton(j,i)->cart()>0 && 
         panel_buttons[PanelOffset(type,panel)].
	 panelButton(j,i)->state()==false) {
        if(edit_col==-1 || col==i) {
	  edit_col=i;
	  if(edit_row==-1) {
	    edit_row=j;
	  } 
	}
      }
    }
  }
  if(edit_row==-1 || edit_col==-1) {
    return;
  }
  
  RDPanelButton *button=
    panel_buttons[PanelOffset(type,panel)].panelButton(edit_row,edit_col);
  RDPlayDeck *deck=button->playDeck();
  if(deck!=NULL) {
    if(deck->state()!=RDPlayDeck::Stopping) {
      deck->play(deck->currentPosition());
    } 
    if(button->hookMode()) {
      button->setStartTime(QTime::currentTime().
			   addMSecs(panel_station->timeOffset()).
			   addMSecs(-deck->currentPosition()+
				    deck->cut()->hookStartPoint()));
    }
    else {
      button->setStartTime(QTime::currentTime().
			   addMSecs(panel_station->timeOffset()).
			   addMSecs(-deck->currentPosition()));
    }
    return;
  }

  int cartnum=0;

  if((cartnum=button->cart())==0) {
    LogLine(QString().sprintf("Tried to start empty button.  Row=%d, Col=%d",
			      edit_row,edit_col));
    return;
  }
  RDCart *cart=new RDCart(cartnum);
  if(!cart->exists()) {
    delete cart;
    LogLine(QString().sprintf("Tried to start non-existent cart: %u",cartnum));
    return;
  }
  button->setStartSource(src);
  if(panel_pause_enabled) {
    button->setPauseWhenFinished(pause_when_finished);
    }
  else {
    button->setPauseWhenFinished(false);
    }
  switch(cart->type()) {
      case RDCart::Audio:
	PlayAudio(button,cart,hookmode,mport);
	break;

      case RDCart::Macro:
	PlayMacro(button,cart);
	break;

      default:
	break;
  }
  delete cart;
}


bool RDSoundPanel::PlayAudio(RDPanelButton *button,RDCart *cart,bool hookmode,int mport)
{
  RDLogLine logline;

  bool timescale=false;
  int button_deck=GetFreeButtonDeck();
  if(button_deck<0) {
    LogLine(QString().
	    sprintf("No button deck available, playout aborted.  Cart=%u",
		    cart->number()));
    return false;
  }
  if(mport<=0 || mport>PANEL_MAX_OUTPUTS) {
    button->setOutput(GetFreeOutput());
    }
  else {
    button->setOutput(mport-1);
    }
  button->setOutputText(panel_output_text[button->output()]);
  button->setHookMode(hookmode);
  button->setPlayDeck(new RDPlayDeck(panel_cae,button_deck,this));
  button->playDeck()->setCard(panel_card[button->output()]);
  button->playDeck()->setPort(panel_port[button->output()]);
  button->playDeck()->duckVolume(button->duckVolume(),0);
  if(panel_timescaling_supported[panel_card[button->output()]]&&
     cart->enforceLength()) {
    timescale=true;
  }
  logline.loadCart(cart->number(),RDLogLine::Play,0,timescale);
  if(!button->playDeck()->setCart(&logline,true)) {
    delete button->playDeck();
    button->setPlayDeck(NULL);
    LogLine(QString().
	    sprintf("No CAE stream available, playout aborted.  Cart=%u",
		    cart->number()));
    return false;
  }
  button->setCutName(logline.cutName());
  panel_active_buttons[button_deck]=button;

  //
  // Set Mappings
  //
  connect(button->playDeck(),SIGNAL(stateChanged(int,RDPlayDeck::State)),
	  this,SLOT(stateChangedData(int,RDPlayDeck::State)));
  connect(button->playDeck(),SIGNAL(hookEnd(int)),
	  this,SLOT(hookEndData(int)));
  connect(this,SIGNAL(tick()),button,SLOT(tickClock()));
  
  //
  // Calculate Start Parameters for Hook Playout
  //
  int start_pos=0;
  int segue_start=-1;
  int segue_end=-1;
  if(hookmode&&(logline.hookStartPoint()>=0)&&(logline.hookEndPoint()>=0)) {
    start_pos=logline.hookStartPoint()-logline.startPoint();
    segue_start=logline.hookEndPoint()-logline.startPoint();
    segue_end=logline.hookEndPoint()-logline.startPoint();
  }

  //
  // Start Playout
  //
  button->
    setStartTime(QTime::currentTime().addMSecs(panel_station->timeOffset()));
  if(hookmode&&(button->playDeck()->cut()->hookStartPoint()>=0)) {
    button->setActiveLength(button->playDeck()->cut()->hookEndPoint()-
      button->playDeck()->cut()->hookStartPoint());
  }
  else {
    if(timescale) {
      button->setActiveLength(cart->forcedLength());
    }
    else {
      button->setActiveLength(button->playDeck()->cut()->length());
    }
  }
  button->playDeck()->play(start_pos,segue_start,segue_end);
  panel_event_player->
    exec(logline.resolveWildcards(panel_start_rml[button->output()]));
  return true;
}


void RDSoundPanel::PlayMacro(RDPanelButton *button,RDCart *cart)
{
  RDMacro rml;
  rml.setRole(RDMacro::Cmd);
  rml.setAddress(panel_station->address());
  rml.setEchoRequested(false);
  rml.setCommand(RDMacro::EX);
  rml.setArgQuantity(1);
  rml.setArg(0,cart->number());
  panel_ripc->sendRml(&rml);
  if(!panel_svcname.isEmpty()) {
    LogTrafficMacro(button);
  }
  if(button->pauseWhenFinished() && panel_pause_enabled) {
    button->setState(true);
    button->WriteKeycap(-1);
    button->setColor(RDPANEL_PAUSED_BACKGROUND_COLOR);
  }
}


void RDSoundPanel::PauseButton(RDAirPlayConf::PanelType type,int panel,
			       int row,int col,int mport)
{
	for(int i=0;i<panel_button_columns;i++) {
		for(int j=0;j<panel_button_rows;j++) {
      RDPlayDeck *deck=
        panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->playDeck();
      if(deck!=NULL && (row==j || row==-1) && (col==i || col==-1)) {
        if(mport==-1 || 
           mport==panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->output()+1) {
          deck->pause();

          panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->
             setStartTime(QTime());
          }
       }
     }
   }
}


void RDSoundPanel::StopButton(RDAirPlayConf::PanelType type,int panel,
			    int row,int col,int mport,
                            bool pause_when_finished,int fade_out)
{
  int edit_fade_out=fade_out;
  if(edit_fade_out<=0) {
    edit_fade_out=1000;
  }
  int edit_mport=mport;
  if (edit_mport==0) {
    edit_mport=-1;
    }
    for(int i=0;i<panel_button_columns;i++) {
	    for(int j=0;j<panel_button_rows;j++) {
      RDPlayDeck *deck=
        panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->playDeck();
      if((row==j || row==-1) && (col==i || col==-1)) {
        if(deck!=NULL) {
          if(edit_mport==-1 || 
             edit_mport==panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->
                     output()+1) {
            if(panel_pause_enabled) {
              panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->
                    setPauseWhenFinished(pause_when_finished);
              }
            else {
              panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->
                    setPauseWhenFinished(false);
              }
            switch(deck->state()) {
	        case RDPlayDeck::Playing:
	          deck->stop(edit_fade_out,RD_FADE_DEPTH,true);
	          break;

  	        case RDPlayDeck::Paused:
	          deck->clear();
	          break;

	        default:
	          deck->clear();
	          break;
            }
          }
        }
      else {
        if(!pause_when_finished && panel_pause_enabled) {
          panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->setState(false); 
          panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->
              setPauseWhenFinished(false); 
          panel_buttons[PanelOffset(type,panel)].panelButton(j,i)->reset(); 
          }
        }
      }
    }
  }
  panel_reset_mode=false;
  panel_reset_button->setFlashingEnabled(false);
  panel_all_button->hide();
  panel_setup_button->show();
}


void RDSoundPanel::StopButton(int id)
{
  RDPlayDeck *deck=panel_active_buttons[id]->playDeck();
  StopButton(deck);
}


void RDSoundPanel::StopButton(RDPlayDeck *deck)
{
  if(deck!=NULL) {
    switch(deck->state()) {
	case RDPlayDeck::Playing:
	  deck->stop();
	  break;

	case RDPlayDeck::Paused:
	  deck->clear();
	  break;

	default:
	  break;
    }
  }    
}


void RDSoundPanel::LoadPanels()
{
  panel_buttons.clear();

  //
  // Load Buttons
  //
  for(int i=0;i<panel_station_panels;i++) {
    panel_buttons.push_back(RDButtonPanel(panel_button_columns,
					  panel_button_rows,
					  panel_station,panel_flash,this,panel_button_x_size,panel_area));
    for(int j=0;j<panel_button_rows;j++) {
      for(int k=0;k<panel_button_columns;k++) {
	connect(panel_buttons.back().panelButton(j,k),SIGNAL(clicked()),
		panel_mapper,SLOT(map()));
	panel_mapper->setMapping(panel_buttons.back().panelButton(j,k),
			   j*panel_button_columns+k);
      }
    }
    LoadPanel(RDAirPlayConf::StationPanel,i);
  }
  for(int i=0;i<panel_user_panels;i++) {
    panel_buttons.push_back(RDButtonPanel(panel_button_columns,
					  panel_button_rows,
					  panel_station,panel_flash,this,panel_button_x_size,panel_area));
    for(int j=0;j<panel_button_rows;j++) {
      for(int k=0;k<panel_button_columns;k++) {
	connect(panel_buttons.back().panelButton(j,k),SIGNAL(clicked()),
		panel_mapper,SLOT(map()));
	panel_mapper->setMapping(panel_buttons.back().panelButton(j,k),
			   j*panel_button_columns+k);
      }
    }
    LoadPanel(RDAirPlayConf::UserPanel,i);
  }
}


void RDSoundPanel::LoadPanel(RDAirPlayConf::PanelType type,int panel)
{
  QString owner;
  int offset=0;

  switch(type) {
      case RDAirPlayConf::UserPanel:
	if(panel_user==NULL) {
	  return;
	}
	owner=panel_user->name();
	offset=panel_station_panels+panel;
	break;

      case RDAirPlayConf::StationPanel:
	owner=panel_station->name();
	offset=panel;
	break;
  }

  QString sql=QString().sprintf("select %s.ROW_NO,%s.COLUMN_NO,\
    %s.LABEL,%s.CART,%s.DEFAULT_COLOR,CART.FORCED_LENGTH,\
    CART.AVERAGE_HOOK_LENGTH \
    from %s left join CART on %s.CART=CART.NUMBER\
    where %s.TYPE=%d && %s.OWNER=\"%s\" && %s.PANEL_NO=%d\
    order by %s.COLUMN_NO,%s.ROW_NO",
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				(const char *)panel_tablename,
				type,
				(const char *)panel_tablename,
				(const char *)owner,
				(const char *)panel_tablename,
				panel,
				(const char *)panel_tablename,
				(const char *)panel_tablename);
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    if(q->value(0).toInt()<panel_button_rows && q->value(1).toInt()<panel_button_columns) {
      if(panel_buttons[offset].panelButton(q->value(0).toInt(),
	      q->value(1).toInt())->playDeck()==NULL) {
        panel_buttons[offset].
	  panelButton(q->value(0).toInt(),q->value(1).toInt())->
	  setText(q->value(2).toString());
        panel_buttons[offset].
	  panelButton(q->value(0).toInt(),q->value(1).toInt())->
	  setCart(q->value(3).toInt());
        panel_buttons[offset].
	  panelButton(q->value(0).toInt(),q->value(1).toInt())->
	  setLength(false,q->value(5).toInt());
        panel_buttons[offset].
	  panelButton(q->value(0).toInt(),q->value(1).toInt())->
	  setLength(true,q->value(6).toInt());
        if((panel_playmode_box!=NULL)&&(panel_playmode_box->currentItem()==1)&&
	   (q->value(6).toUInt()>0)) {
	  panel_buttons[offset].
	    panelButton(q->value(0).toInt(),q->value(1).toInt())->
	    setActiveLength(q->value(6).toInt());
        }
        else {
	  panel_buttons[offset].
	    panelButton(q->value(0).toInt(),q->value(1).toInt())->
	    setActiveLength(q->value(5).toInt());
        }
        if(q->value(4).toString().isEmpty()) {
	  panel_buttons[offset].
	    panelButton(q->value(0).toInt(),q->value(1).toInt())->
	    setColor(palette().active().background());
	  panel_buttons[offset].
	    panelButton(q->value(0).toInt(),q->value(1).toInt())->
	    setDefaultColor(palette().active().background());
        }
        else {
	  panel_buttons[offset].
	    panelButton(q->value(0).toInt(),q->value(1).toInt())->
	    setColor(QColor(q->value(4).toString()));
	  panel_buttons[offset].
	    panelButton(q->value(0).toInt(),q->value(1).toInt())->
	    setDefaultColor(QColor(q->value(4).toString()));
        }
      }
    } 
  }
  delete q;
}


void RDSoundPanel::SaveButton(RDAirPlayConf::PanelType type,
			    int panel,int row,int col)
{
  QString sql;
  QString sql1;
  RDSqlQuery *q;
  QString owner;
  int offset=0;

  switch(type) {
      case RDAirPlayConf::UserPanel:
	owner=panel_user->name();
	offset=panel_station_panels+panel;
	break;

      case RDAirPlayConf::StationPanel:
	owner=panel_station->name();
	offset=panel;
	break;
  }

  //
  // Determine if the button exists
  //
  sql=QString().sprintf("select LABEL from %s where \
          TYPE=%d && OWNER=\"%s\" && PANEL_NO=%d && ROW_NO=%d && COLUMN_NO=%d",
			(const char *)panel_tablename,
			type,
			(const char *)owner,
			panel,
			row,
			col);
  q=new RDSqlQuery(sql);
  if(q->size()>0) {
    //
    // If so, update the record
    //
    delete q;
    sql1=QString().sprintf("update %s set LABEL=\"%s\",\
    CART=%d,DEFAULT_COLOR=\"%s\" where (TYPE=%d)&&(OWNER=\"%s\")&&\
    (PANEL_NO=%d)&&(ROW_NO=%d)&&(COLUMN_NO=%d)",
			   (const char *)panel_tablename,
			   (const char *)RDEscapeString(panel_buttons[offset].
							panelButton(row,col)->
							text().utf8()),
			   panel_buttons[PanelOffset(panel_type,panel_number)].
			   panelButton(row,col)->cart(),
			   (const char *)panel_buttons[offset].
			   panelButton(row,col)->defaultColor().
			   name(),
			   type,
			   (const char *)RDEscapeString(owner),
			   panel,
			   row,
			   col);
    q=new RDSqlQuery(sql1);
    if(q->isActive()) {
      delete q;
      return;
    }
    delete q;
  }
  else {
    delete q;
    
    //
    // Otherwise, insert a new one
    //
    sql1=QString().sprintf("insert into %s (TYPE,OWNER,\
    PANEL_NO,ROW_NO,COLUMN_NO,LABEL,CART,DEFAULT_COLOR)\
    values (%d,\"%s\",%d,%d,%d,\"%s\",%d,\"%s\")",
			   (const char *)panel_tablename,
			   type,
			   (const char *)RDEscapeString(owner),
			   panel,
			   row,
			   col,
			   (const char *)RDEscapeString(panel_buttons[offset].
							panelButton(row,col)->
							text().utf8()),
			   panel_buttons[PanelOffset(panel_type,panel_number)].
			   panelButton(row,col)->cart(),
			   (const char *)panel_buttons[offset].
			   panelButton(row,col)->
			   defaultColor().name());
    q=new RDSqlQuery(sql1);
    delete q;
  }
}


int RDSoundPanel::PanelOffset(RDAirPlayConf::PanelType type,int panel)
{
  switch(type) {
      case RDAirPlayConf::StationPanel:
	return panel;
	break;

      case RDAirPlayConf::UserPanel:
	return panel_station_panels+panel;
	break;
  }
  return 0;
}


int RDSoundPanel::GetFreeButtonDeck()
{
  for(int i=0;i<RD_MAX_STREAMS;i++) {
    if(panel_active_buttons[i]==NULL) {
      return i;
    }
  }
  return -1;
}


int RDSoundPanel::GetFreeOutput()
{
  bool active=false;

  for(int i=0;i<PANEL_MAX_OUTPUTS;i++) {
    active=false;
    for(int j=0;j<RD_MAX_STREAMS;j++) {
      if((panel_active_buttons[j]!=NULL)&&
	 (panel_active_buttons[j]->output()==i)) {
	active=true;
      }
    }
    if(!active) {
      return i;
    }
  }
  return PANEL_MAX_OUTPUTS-1;
}


void RDSoundPanel::LogPlayEvent(unsigned cartnum,int cutnum)
{
  RDCut *cut=new RDCut(QString().sprintf("%06u_%03d",cartnum,cutnum));
  cut->logPlayout();
  delete cut;
}


void RDSoundPanel::LogTraffic(RDPanelButton *button)
{
  if(panel_svcname.isEmpty()) {
    return;
  }

  QString sql;
  RDSqlQuery *q;
  QDateTime datetime(QDate::currentDate(),QTime::currentTime());

  sql=QString().sprintf("select CART.TITLE,CART.ARTIST,CART.PUBLISHER,\
                         CART.COMPOSER,CART.USAGE_CODE,CUTS.ISRC,\
                         CART.ALBUM,CART.LABEL \
                         from CART left join CUTS \
                         on CART.NUMBER=CUTS.CART_NUMBER \
                         where CUTS.CUT_NAME=\"%s\"",
			(const char *)button->cutName());
  q=new RDSqlQuery(sql);
  if(q->first()) {
    sql=QString().sprintf("insert into `%s_SRT` set\
                         LENGTH=%d,CART_NUMBER=%u,\
                         STATION_NAME=\"%s\",EVENT_DATETIME=\"%s %s\",\
                         EVENT_TYPE=%d,EVENT_SOURCE=%d,PLAY_SOURCE=%d,\
                         CUT_NUMBER=%d,TITLE=\"%s\",ARTIST=\"%s\",\
                         PUBLISHER=\"%s\",COMPOSER=\"%s\",USAGE_CODE=%d,\
                         ISRC=\"%s\",START_SOURCE=%d,ALBUM=\"%s\",\
                         LABEL=\"%s\",ONAIR_FLAG=\"%s\"",
			  (const char *)panel_svcname,
			  button->startTime().msecsTo(datetime.time()),
			  button->cart(),
			  (const char *)RDEscapeString(panel_station->name()),
			  (const char *)datetime.toString("yyyy-MM-dd"),
			  (const char *)button->startTime().
			  toString("hh:mm:ss"),
			  RDAirPlayConf::TrafficStop,
			  RDLogLine::SoundPanel,
			  RDLogLine::SoundPanel,
			  button->cutName().right(3).toInt(),
			  (const char *)RDEscapeString(q->value(0).toString()),
			  (const char *)RDEscapeString(q->value(1).toString()),
			  (const char *)RDEscapeString(q->value(2).toString()),
			  (const char *)RDEscapeString(q->value(3).toString()),
			  q->value(4).toInt(),
			  (const char *)q->value(5).toString(),
			  button->startSource(),
			  (const char *)RDEscapeString(q->value(6).toString()),
			  (const char *)RDEscapeString(q->value(7).toString()),
			  (const char *)RDYesNo(panel_onair_flag));
    delete q;
    q=new RDSqlQuery(sql);
  }
  delete q;
}


void RDSoundPanel::LogTrafficMacro(RDPanelButton *button)
{
  QString sql;
  RDSqlQuery *q;
  QDateTime datetime(QDate::currentDate(),QTime::currentTime());

  sql=QString().sprintf("select TITLE,ARTIST,PUBLISHER,\
                         COMPOSER,USAGE_CODE,FORCED_LENGTH,\
                         ALBUM,LABEL from CART where NUMBER=%u",
			button->cart());
  q=new RDSqlQuery(sql);
  if(q->first()) {
    sql=QString().sprintf("insert into `%s_SRT` set\
                         LENGTH=%d,CART_NUMBER=%u,\
                         STATION_NAME=\"%s\",EVENT_DATETIME=\"%s\",\
                         EVENT_TYPE=%d,EVENT_SOURCE=%d,PLAY_SOURCE=%d,\
                         TITLE=\"%s\",ARTIST=\"%s\",\
                         PUBLISHER=\"%s\",COMPOSER=\"%s\",USAGE_CODE=%d,\
                         START_SOURCE=%d,ALBUM=\"%s\",\
                         LABEL=\"%s\",ONAIR_FLAG=\"%s\"",
			  (const char *)panel_svcname,
			  q->value(5).toUInt(),
			  button->cart(),
			  (const char *)RDEscapeString(panel_station->name()),
			  (const char *)datetime.
			  toString("yyyy-MM-dd hh:mm:ss"),
			  RDAirPlayConf::TrafficMacro,
			  RDLogLine::SoundPanel,
			  RDLogLine::SoundPanel,
			  (const char *)RDEscapeString(q->value(0).toString()),
			  (const char *)RDEscapeString(q->value(1).toString()),
			  (const char *)RDEscapeString(q->value(2).toString()),
			  (const char *)RDEscapeString(q->value(3).toString()),
			  q->value(4).toInt(),
			  button->startSource(),
			  (const char *)RDEscapeString(q->value(6).toString()),
			  (const char *)RDEscapeString(q->value(7).toString()),
			  (const char *)RDYesNo(panel_onair_flag));
    delete q;
    q=new RDSqlQuery(sql);
    delete q;
  }
}


void RDSoundPanel::LogLine(QString str)
{
  FILE *file;

  if(panel_logfile.isEmpty()) {
    return;
  }

  QDateTime current=QDateTime::currentDateTime();
  if((file=fopen(panel_logfile,"a"))==NULL) {
    return;
  }
  fprintf(file,"%02d/%02d/%4d - %02d:%02d:%02d.%03d : RDSoundPanel: %s\n",
	  current.date().month(),
	  current.date().day(),
	  current.date().year(),
	  current.time().hour(),
	  current.time().minute(),
	  current.time().second(),
	  current.time().msec(),
	  (const char *)str);
  fclose(file);
}


void RDSoundPanel::Playing(int id)
{
  if(panel_active_buttons[id]==NULL) {
    LogLine(QString().sprintf("Invalid ID=%d in RDSoundPanel::Playing()",
			      id));
    return;
  }
  panel_active_buttons[id]->setState(true);
  panel_active_buttons[id]->setColor(RDPANEL_PLAY_BACKGROUND_COLOR);
  LogPlayEvent(panel_active_buttons[id]->playDeck()->cart()->number(),
	       panel_active_buttons[id]->playDeck()->cut()->cutNumber());
  LogLine(QString().
	  sprintf("Playout started: id=%d  cart=%u  cut=%d",
		  id,panel_active_buttons[id]->playDeck()->cart()->number(),
		  panel_active_buttons[id]->playDeck()->cut()->cutNumber()));
}


void RDSoundPanel::Paused(int id)
{
  if(panel_active_buttons[id]==NULL) {
    LogLine(QString().sprintf("Invalid ID=%d in RDSoundPanel::Paused()",
			      id));
    return;
  }
  panel_active_buttons[id]->setState(true);
  panel_active_buttons[id]->setColor(RDPANEL_PAUSED_BACKGROUND_COLOR);
  LogLine(QString().
	  sprintf("Playout paused: id=%d  cart=%u  cut=%d",
		  id,panel_active_buttons[id]->playDeck()->cart()->number(),
		  panel_active_buttons[id]->playDeck()->cut()->cutNumber()));
}


void RDSoundPanel::Stopped(int id)
{
  if(panel_active_buttons[id]==NULL) {
    LogLine(QString().sprintf("Invalid ID=%d in RDSoundPanel::Stopped()",
			      id));
    return;
  }
  LogTraffic(panel_active_buttons[id]);
  ClearChannel(id);
  if(panel_active_buttons[id]->pauseWhenFinished()) {
    panel_active_buttons[id]->setState(true);
    panel_active_buttons[id]->setColor(RDPANEL_PAUSED_BACKGROUND_COLOR);
    panel_active_buttons[id]->WriteKeycap(-1);
    }
  else {
    panel_active_buttons[id]->setState(false);
    panel_active_buttons[id]->setHookMode(panel_playmode_box->currentItem()==1);
  }
  disconnect(this,SIGNAL(tick()),panel_active_buttons[id],SLOT(tickClock()));
  panel_active_buttons[id]->playDeck()->disconnect();
  delete panel_active_buttons[id]->playDeck();
  panel_active_buttons[id]->setPlayDeck(NULL);
  if(!panel_active_buttons[id]->pauseWhenFinished()) {
    panel_active_buttons[id]->reset();
    }
  panel_active_buttons[id]->setDuckVolume(0);
  panel_active_buttons[id]=NULL;
  LogLine(QString().sprintf("Playout stopped: id=%d",id));
}


void RDSoundPanel::ClearChannel(int id)
{
  RDPlayDeck *playdeck=panel_active_buttons[id]->playDeck();
  if(panel_cae->
     playPortActive(playdeck->card(),playdeck->port(),playdeck->stream())) {
    return;
  }
  panel_event_player->exec(panel_stop_rml[panel_active_buttons[id]->output()]);
}


void RDSoundPanel::ClearReset()
{
  panel_reset_mode=false;
  panel_reset_button->setFlashingEnabled(false);
  panel_setup_button->setEnabled(true);
}


QString RDSoundPanel::PanelTag(int index)
{
  if(index<panel_station_panels) {
    return QString().sprintf("S:%d",index+1);
  }
  return QString().sprintf("U:%d",index-panel_station_panels+1);
}


QString RDSoundPanel::PanelOwner(RDAirPlayConf::PanelType type)
{
  switch(type) {
      case RDAirPlayConf::StationPanel:
	return panel_station->name();

      case RDAirPlayConf::UserPanel:
	if(panel_user!=NULL) {
	  return panel_user->name();
	}
  }
  return QString();
}


void RDSoundPanel::addClickedData(unsigned cartnum,int row,int col)
{
emit selectMenuClicked(cartnum,row,col,RDAirPlayConf::AddTo);
}


void RDSoundPanel::copyClickedData(unsigned cartnum,int row,int col)
{
emit selectMenuClicked(cartnum,row,col,RDAirPlayConf::CopyFrom);
}


void RDSoundPanel::resizeEvent(QResizeEvent *e)
{
  panel_area->setGeometry(3,3,size().width(),size().height()-63);
  panel_selector_box->setGeometry(3,size().height()-56,191,50);
  panel_playmode_box->setGeometry(204,size().height()-56,98,50);
  panel_reset_button->setGeometry(310,size().height()-56,88,50);
  panel_all_button->setGeometry(413,size().height()-56,88,50);
  panel_setup_button->setGeometry(413,size().height()-56,88,50);
}
