// rdlibrary.cpp
//
// The Library Utility for Rivendell.
//
//   (C) Copyright 2002-2010 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdlibrary.cpp,v 1.106.2.7 2010/02/04 19:03:33 cvs Exp $
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

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qsqlpropertymap.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qprogressdialog.h>
#include <qtooltip.h>

#include <rd.h>
#include <rddb.h>
#include <rdconf.h>
#include <rduser.h>
#include <rdripc.h>
#include <rdmixer.h>
#include <rdadd_cart.h>
#include <rdprofile.h>
#include <rdaudio_port.h>
#include <rdcart_search_text.h>
#include <rdcheck_daemons.h>
#include <rdtextvalidator.h>
#include <rdcmd_switch.cpp>

#include <filter.h>
#include <edit_cart.h>
#include <rdlibrary.h>
#include <disk_ripper.h>
#include <cdripper.h>
#include <list_reports.h>
#include <globals.h>
#include <validate_cut.h>
#include <cart_tip.h>

//
// Global Resources
//
RDLibraryConf *rdlibrary_conf;
RDStation *rdstation_conf;
RDAudioPort *rdaudioport_conf;
RDRipc *rdripc;
RDCae *rdcae;
DiskGauge *disk_gauge;
RDCut *cut_clipboard=NULL;
RDConfig *lib_config;
RDUser *lib_user;
bool audio_changed;

//
// Prototypes
//
void SigHandler(int signo);

//
// Icons
//
#include "../icons/play.xpm"
#include "../icons/rml5.xpm"
#include "../icons/track_cart.xpm"
#include "../icons/rivendell-22x22.xpm"


MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  //
  // Ensure We're the Only Instance Running
  //
  lib_lock=
    new RDInstanceLock(QString().sprintf("%s/.rdlibrarylock",
					(const char *)RDHomeDir()));
  if(!lib_lock->lock()) {
    fprintf(stderr,"rdlibrary: multiple instances not allowd\n");
    exit(1);
  }

  //
  // HACK: Disable the Broken Custom SuSE Dialogs
  //
  setenv("QT_NO_KDE_INTEGRATION","1",1);

  //
  // Read Command Options
  //
  RDCmdSwitch *cmd=new RDCmdSwitch(qApp->argc(),qApp->argv(),"rdlibrary","\n");
  delete cmd;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());

  //
  // Generate Fonts
  //
  QFont default_font("Helvetica",12,QFont::Normal);
  default_font.setPixelSize(12);
  qApp->setFont(default_font);
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont filter_font=QFont("Helvetica",16,QFont::Bold);
  filter_font.setPixelSize(16);

  //
  // Create Icons
  //
  lib_playout_map=new QPixmap(play_xpm);
  lib_macro_map=new QPixmap(rml5_xpm);
  lib_track_cart_map=new QPixmap(track_cart_xpm);
  lib_rivendell_map=new QPixmap(rivendell_xpm);
  setIcon(*lib_rivendell_map);

  //
  // Text Validator
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator");

  //
  // Progress Dialog
  //
  lib_progress_dialog=
    new QProgressDialog(tr("Please Wait..."),"Cancel",10,this,
			"lib_progress_dialog",false,
			Qt::WStyle_Customize|Qt::WStyle_NormalBorder);
  lib_progress_dialog->setCaption(" ");
  QLabel *label=new QLabel(tr("Please Wait..."),lib_progress_dialog);
  label->setAlignment(AlignCenter);
  label->setFont(filter_font);
  lib_progress_dialog->setLabel(label);
  lib_progress_dialog->setCancelButton(NULL);
  lib_progress_dialog->setMinimumDuration(2000);

  //
  // Ensure that the system daemons are running
  //
  RDInitializeDaemons();

  //
  // Load Local Configs
  //
  lib_config=new RDConfig();
  lib_config->load();

  SetCaption("");
  lib_import_path=RDGetHomeDir();

  //
  // CAE Connection
  //
  rdcae=new RDCae(parent,name);
  rdcae->connectHost("localhost",CAED_TCP_PORT,lib_config->password());

  //
  // Open Database
  //
  QString err (tr("rdlibrary : "));
  QSqlDatabase *db=RDInitDb (&err);
  if(!db) {
    QMessageBox::warning(this,
			 tr("Can't Connect"),err);
    exit(0);
  }

  //
  // Allocate Global Resources
  //
  rdlibrary_conf=new RDLibraryConf(lib_config->stationName(),0);
  rdstation_conf=new RDStation(lib_config->stationName());
  lib_filter_mode=rdstation_conf->filterMode();
  rdaudioport_conf=new RDAudioPort(lib_config->stationName(),
				   rdlibrary_conf->inputCard());
  rdripc=new RDRipc(lib_config->stationName());
  connect(rdripc,SIGNAL(connected(bool)),this,SLOT(connectedData(bool)));
  connect(rdripc,SIGNAL(userChanged()),this,SLOT(userData()));
  rdripc->connectHost("localhost",RIPCD_TCP_PORT,lib_config->password());
  cut_clipboard=NULL;

  //
  // Load Audio Assignments
  //
  RDSetMixerPorts(lib_config->stationName(),rdcae);

  //
  // User
  //
  lib_user=NULL;

  //
  // Filter
  //
  lib_filter_edit=new QLineEdit(this,"filter_edit");
  lib_filter_edit->setFont(default_font);
  lib_filter_edit->setValidator(validator);
  lib_filter_label=new QLabel(lib_filter_edit,tr("Filter:"),
			      this,"filter_label");
  lib_filter_label->setFont(button_font);
  lib_filter_label->setAlignment(AlignVCenter|AlignRight);
  connect(lib_filter_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(filterChangedData(const QString &)));
  connect(lib_filter_edit,SIGNAL(returnPressed()),
	  this,SLOT(searchClickedData()));

  //
  // Filter Search Button
  //
  lib_search_button=new QPushButton(tr("&Search"),this,"search_button");
  lib_search_button->setFont(button_font);
  connect(lib_search_button,SIGNAL(clicked()),this,SLOT(searchClickedData()));
  switch(lib_filter_mode) {
    case RDStation::FilterSynchronous:
      lib_search_button->hide();
      break;

    case RDStation::FilterAsynchronous:
      break;
  }

  //
  // Filter Clear Button
  //
  lib_clear_button=new QPushButton(tr("&Clear"),this,"clear_button");
  lib_clear_button->setFont(button_font);
  lib_clear_button->setDisabled(true);
  connect(lib_clear_button,SIGNAL(clicked()),this,SLOT(clearClickedData()));

  //
  // Group Filter
  //
  lib_group_box=new QComboBox(this,"lib_group_box");
  lib_group_box->setFont(default_font);
  lib_group_label=new QLabel(lib_group_box,tr("Group:"),
			     this,"lib_group_label");
  lib_group_label->setFont(button_font);
  lib_group_label->setAlignment(AlignVCenter|AlignRight);
  connect(lib_group_box,SIGNAL(activated(const QString &)),
	  this,SLOT(groupActivatedData(const QString &)));

  //
  // Scheduler Codes Filter
  //
  lib_codes_box=new QComboBox(this,"lib_codes_box");
  lib_codes_box->setFont(default_font);
  lib_codes_label=new QLabel(lib_codes_box,tr("Scheduler Code:"),
			     this,"lib_codes_label");
  lib_codes_label->setFont(button_font);
  lib_codes_label->setAlignment(AlignVCenter|AlignRight);
  connect(lib_codes_box,SIGNAL(activated(const QString &)),
	  this,SLOT(groupActivatedData(const QString &)));

  //
  // Show Audio Carts Checkbox
  //
  lib_showaudio_box=new QCheckBox(this,"lib_showaudio_box");
  lib_showaudio_box->setChecked(true);
  lib_showaudio_label=new QLabel(lib_showaudio_box,tr("Show Audio Carts"),
			     this,"lib_showaudio_label");
  lib_showaudio_label->setFont(button_font);
  lib_showaudio_label->setAlignment(AlignVCenter|AlignLeft);
  connect(lib_showaudio_box,SIGNAL(stateChanged(int)),
	  this,SLOT(audioChangedData(int)));

  //
  // Show Macro Carts Checkbox
  //
  lib_showmacro_box=new QCheckBox(this,"lib_showmacro_box");
  lib_showmacro_box->setChecked(true);
  lib_showmacro_label=new QLabel(lib_showmacro_box,tr("Show Macro Carts"),
			     this,"lib_showmacro_label");
  lib_showmacro_label->setFont(button_font);
  lib_showmacro_label->setAlignment(AlignVCenter|AlignLeft);
  connect(lib_showmacro_box,SIGNAL(stateChanged(int)),
	  this,SLOT(macroChangedData(int)));


  lib_search_button->setEnabled(false);
  lib_group_box->setEnabled(false);
  lib_showaudio_box->setEnabled(false);
  lib_codes_box->setEnabled(false);
  lib_showmacro_box->setEnabled(false);

  //
  // Show Cart Notes Checkbox
  //
  lib_shownotes_box=new QCheckBox(this,"lib_shownotes_box");
  lib_shownotes_box->setChecked(true);
  lib_shownotes_label=new QLabel(lib_shownotes_box,tr("Show Note Bubbles"),
			     this,"lib_shownotes_label");
  lib_shownotes_label->setFont(button_font);
  lib_shownotes_label->setAlignment(AlignVCenter|AlignLeft);

  //
  // Cart List
  //
  lib_cart_list=new RDListView(this,"lib_cart_list");
  lib_cart_list->setFont(default_font);
  lib_cart_list->setAllColumnsShowFocus(true);
  lib_cart_list->setItemMargin(5);
  lib_cart_list->setSelectionMode(QListView::Extended);
  lib_cart_tip=new CartTip(lib_cart_list->viewport());
  connect(lib_cart_list,
	  SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	  this,
	  SLOT(cartDoubleclickedData(QListViewItem *,const QPoint &,int)));
  connect(lib_cart_list,SIGNAL(pressed(QListViewItem *)),
	  this,SLOT(cartClickedData(QListViewItem *)));
  connect(lib_cart_list,SIGNAL(onItem(QListViewItem *)),
	  this,SLOT(cartOnItemData(QListViewItem *)));
  lib_cart_list->addColumn("");
  lib_cart_list->setColumnAlignment(0,Qt::AlignHCenter);
  lib_cart_list->addColumn(tr("CART"));
  lib_cart_list->setColumnAlignment(1,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("GROUP"));
  lib_cart_list->setColumnAlignment(2,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("LENGTH"));
  lib_cart_list->setColumnAlignment(3,Qt::AlignRight);
  lib_cart_list->setColumnSortType(3,RDListView::TimeSort);

  lib_cart_list->addColumn(tr("TITLE"));

  lib_cart_list->addColumn(tr("ARTIST"));

  lib_cart_list->addColumn(tr("START"));
  lib_cart_list->setColumnAlignment(6,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("END"));
  lib_cart_list->setColumnAlignment(7,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("CLIENT"));

  lib_cart_list->addColumn(tr("AGENCY"));

  lib_cart_list->addColumn(tr("USER DEFINED"));

  lib_cart_list->addColumn(tr("CUTS"));
  lib_cart_list->setColumnAlignment(11,Qt::AlignRight);

  lib_cart_list->addColumn(tr("LAST CUT PLAYED"));
  lib_cart_list->setColumnAlignment(12,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("PLAY ORDER"));
  lib_cart_list->setColumnAlignment(13,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("ENFORCE LENGTH"));
  lib_cart_list->setColumnAlignment(14,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("PRESERVE PITCH"));
  lib_cart_list->setColumnAlignment(15,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("LENGTH DEVIATION"));
  lib_cart_list->setColumnAlignment(16,Qt::AlignHCenter);

  lib_cart_list->addColumn(tr("OWNED BY"));
  lib_cart_list->setColumnAlignment(17,Qt::AlignHCenter);

  //
  // Add Button
  //
  lib_add_button=new QPushButton(this,"lib_add_button");
  lib_add_button->setFont(button_font);
  lib_add_button->setText(tr("&Add"));
  connect(lib_add_button,SIGNAL(clicked()),this,SLOT(addData()));

  //
  // Edit Button
  //
  lib_edit_button=new QPushButton(this,"lib_edit_button");
  lib_edit_button->setFont(button_font);
  lib_edit_button->setText(tr("&Edit"));
  connect(lib_edit_button,SIGNAL(clicked()),this,SLOT(editData()));

  //
  // Delete Button
  //
  lib_delete_button=new QPushButton(this,"lib_delete_button");
  lib_delete_button->setFont(button_font);
  lib_delete_button->setText(tr("&Delete"));
  connect(lib_delete_button,SIGNAL(clicked()),this,SLOT(deleteData()));

  //
  // Selected
  //
  lib_selected_label=new QLabel(lib_group_box,tr(""),
			     this,"lib_selected_label");
  lib_selected_label->setFont(button_font);
  lib_selected_label->setAlignment(AlignVCenter|AlignCenter);


  //
  // Disk Gauge
  //
  disk_gauge=new DiskGauge(rdlibrary_conf->defaultSampleRate(),
			   rdlibrary_conf->defaultChannels(),
			   this,"disk_gauge");

  //
  // Rip Button
  //
  lib_rip_button=new QPushButton(this,"lib_rip_button");
  lib_rip_button->setFont(button_font);
  lib_rip_button->setText(tr("&Rip\nCD"));
  connect(lib_rip_button,SIGNAL(clicked()),this,SLOT(ripData()));

  //
  // Reports Button
  //
  lib_reports_button=new QPushButton(this,"lib_reports_button");
  lib_reports_button->setFont(button_font);
  lib_reports_button->setText(tr("Re&ports"));
  connect(lib_reports_button,SIGNAL(clicked()),this,SLOT(reportsData()));

  //
  // Cart Player
  //
  lib_player=
    new RDSimplePlayer(rdcae,rdripc,rdlibrary_conf->outputCard(),rdlibrary_conf->outputPort(),
		       0,0,this,"lib_player");
  lib_player->playButton()->
    setPalette(QPalette(backgroundColor(),QColor(lightGray)));
  lib_player->stopButton()->
    setPalette(QPalette(backgroundColor(),QColor(lightGray)));
  lib_player->stopButton()->setOnColor(red);

  //
  // Close Button
  //
  lib_close_button=new QPushButton(this,"lib_close_button");
  lib_close_button->setFont(button_font);
  lib_close_button->setText(tr("&Close"));
  connect(lib_close_button,SIGNAL(clicked()),this,SLOT(quitMainWidget()));

  // 
  // Setup Signal Handling 
  //
  ::signal(SIGCHLD,SigHandler);

  LoadGeometry();
}


QSize MainWidget::sizeHint() const
{
  return QSize(980,600);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::connectedData(bool state)
{
}


void MainWidget::userData()
{
  QString sql;
  RDSqlQuery *q;

  SetCaption(rdripc->user());
  if(lib_user!=NULL) {
    delete lib_user;
  }
  lib_user=new RDUser(rdripc->user());

  lib_group_box->clear();
  lib_group_box->insertItem(tr("ALL"));
  sql=QString().sprintf("select GROUP_NAME from USER_PERMS\
                              where USER_NAME=\"%s\" order by GROUP_NAME",
			     (const char *)lib_user->name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    lib_group_box->insertItem(q->value(0).toString());
  }
  delete q;
  groupActivatedData(lib_group_box->currentText());

  if(lib_group_box->count()==1) {
    lib_add_button->setDisabled(true);
    lib_edit_button->setDisabled(true);
    lib_delete_button->setDisabled(true);
    lib_rip_button->setDisabled(true);
  }
  else {
    lib_add_button->setEnabled(lib_user->createCarts());
    lib_edit_button->setEnabled(true);
    lib_delete_button->setEnabled(lib_user->deleteCarts());
    lib_rip_button->setEnabled(lib_user->editAudio());
  }

  lib_codes_box->clear();
  lib_codes_box->insertItem(tr(""));
  sql=QString().sprintf("select CODE from SCHED_CODES");
  q=new RDSqlQuery(sql);
  while(q->next()) {
    lib_codes_box->insertItem(q->value(0).toString());
  }
  delete q;
  RefreshList();
  lib_search_button->setDisabled(true);
}


void MainWidget::filterChangedData(const QString &str)
{
  lib_search_button->setEnabled(true);
  if(lib_filter_mode!=RDStation::FilterSynchronous) {
    return;
  }
  searchClickedData();
}


void MainWidget::searchClickedData()
{
  lib_search_button->setDisabled(true);
  if(lib_filter_edit->text().isEmpty()) {
    lib_clear_button->setDisabled(true);
  }
  else {
    lib_clear_button->setEnabled(true);
  }
  RefreshList();
}


void MainWidget::clearClickedData()
{
  lib_filter_edit->clear();
  filterChangedData("");
}


void MainWidget::groupActivatedData(const QString &str)
{
  if(str!=tr("ALL")) {
    lib_default_group=str;
  }
  filterChangedData("");
}


void MainWidget::addData()
{
  QString sql;
  RDSqlQuery *q;
  int cart_num;
  RDCart::Type cart_type=RDCart::All;
  QString cart_title;
  
  RDAddCart *add_cart=new RDAddCart(&lib_default_group,&cart_type,&cart_title,
				    lib_user->name(),this,"add_cart");
  if((cart_num=add_cart->exec())<0) {
    delete add_cart;
    return;
  }
  delete add_cart;
  sql=QString().sprintf("insert into CART set \
                         NUMBER=%u,TYPE=%d,GROUP_NAME=\"%s\",TITLE=\"%s\"",
			cart_num,cart_type,
			(const char *)lib_default_group,
			(const char *)cart_title);
  q=new RDSqlQuery(sql);
  delete q;
  
  EditCart *cart=new EditCart(cart_num,&lib_import_path,true,this);
  if(cart->exec() <0) {
    RDCart *rdcart=new RDCart(cart_num);
    rdcart->remove();
    delete rdcart;
  } 
  else {
    RDListViewItem *item=new RDListViewItem(lib_cart_list);
    item->setText(1,QString().sprintf("%06u",cart_num));
    RefreshLine(item);
    QListViewItemIterator it(lib_cart_list);
    while(it.current()) {
      lib_cart_list->setSelected(it.current(),false);
      ++it;
    }
    lib_cart_list->setSelected(item,true);
    lib_cart_list->ensureItemVisible(item);
  }
  delete cart;
}



void MainWidget::editData()
{
  int sel_count=0;
  QListViewItemIterator *it;

  it=new QListViewItemIterator(lib_cart_list);
  while(it->current()) {
    if (it->current()->isSelected()) {
      sel_count++;
    }
    ++(*it);
  }
  delete it;

  if(sel_count==0) {
    return;
  }
  if(sel_count==1) { //single edit
    it=new QListViewItemIterator(lib_cart_list);
    while(!it->current()->isSelected()) {
      ++(*it);
    }
    RDListViewItem *item=(RDListViewItem *)it->current();

    EditCart *edit_cart=new EditCart(item->text(1).toUInt(),&lib_import_path,
				     false,this,"edit_cart");
    edit_cart->exec();
    RefreshLine(item);
    cartOnItemData(item);
    delete edit_cart;
    delete it;
  }
  else { //multi edit
  //  RDListViewItem *item=(RDListViewItem *)it->current();
    if(lib_user->modifyCarts()) {
      EditCart *edit_cart=new EditCart(0,&lib_import_path,
				     false,this,"edit_cart",lib_cart_list);
    
      edit_cart->exec();
      delete edit_cart;
    
      it=new QListViewItemIterator(lib_cart_list);
      while(it->current()) {
        if (it->current()->isSelected()) {
          RefreshLine((RDListViewItem *)it->current());
        }
        ++(*it);
      }
      delete it;
 //   RefreshLine(item);
    }
  }
}


void MainWidget::deleteData()
{
  QString filename;
  QString sql;
  RDSqlQuery *q;
  QString str;
  int sel_count=0;
  QListViewItemIterator *it;
  bool del_flag;

//  RDListViewItem *item=(RDListViewItem *)lib_cart_list->selectedItem();
//
//  if(item==NULL) {
//    return;
//  }
  it=new QListViewItemIterator(lib_cart_list);
  while(it->current()) {
    if (it->current()->isSelected()) {
      sel_count++;
    }
    ++(*it);
  }
  delete it;

  if(sel_count==0) {
    return;
  }

  str=QString(tr("Are you sure you want to delete cart(s)"));
  if(QMessageBox::question(this,tr("Delete Cart(s)"),str,QMessageBox::Yes,QMessageBox::No)!=
     QMessageBox::Yes) {
    return;
  }
  it=new QListViewItemIterator(lib_cart_list);
  while(it->current()) {
    if (it->current()->isSelected()) {
    del_flag=true;
    RDListViewItem *item=(RDListViewItem *)it->current();
  sql=QString().sprintf("select CUT_NAME from RECORDINGS where \
                         (CUT_NAME like \"%06u_%%\")||(MACRO_CART=%u)",
			item->text(1).toUInt(),item->text(1).toUInt());
  q=new RDSqlQuery(sql);
  if(q->first()) {
      QString str=QString().sprintf(tr("Cart %06u is used in one or more RDCatch events!\n\
Do you still want to delete it?"),item->text(1).toUInt());
      if(QMessageBox::warning(this,tr("RDCatch Event Exists"),str,
			        QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
        del_flag=false;
    }
  }
  delete q;
  if(cut_clipboard!=NULL) {
    if(item->text(1).toUInt()==cut_clipboard->cartNumber()) {
      	QString str=QString().sprintf(tr("Deleting cart %06u will also empty the clipboard.\n\
      	Do you still want to proceed?"),item->text(1).toUInt());
        switch(QMessageBox::question(this,tr("Empty Clipboard"),str,
				  QMessageBox::Yes,
				  QMessageBox::No)) {
	  case QMessageBox::No:
	  case QMessageBox::NoButton:
                del_flag=false;

	  default:
	    break;
      }
      delete cut_clipboard;
      cut_clipboard=NULL;
    }
  }
    if(del_flag && item->text(17).isEmpty()) {
  RDCart *rdcart=new RDCart(item->text(1).toUInt());
  rdcart->remove();
  delete rdcart;
  delete item;
      } 
    else {
      ++(*it);
      } 
    }
    else {
      ++(*it);
     } 
   }
   delete it;
}


void MainWidget::ripData()
{
  QString group=lib_group_box->currentText();
  DiskRipper *dialog=new DiskRipper(&lib_filter_text,&group,
				    this,"disk_ripper");
  if(dialog->exec()==0) {
    for(int i=0;i<lib_group_box->count();i++) {
      if(lib_group_box->text(i)==*group) {
	lib_filter_edit->setText(lib_filter_text);
	lib_group_box->setCurrentItem(i);
	groupActivatedData(lib_group_box->currentText());
      }
    }
  }
  delete dialog;
  RefreshList();
}


void MainWidget::reportsData()
{
  ListReports *lr=
    new ListReports(lib_filter_edit->text(),GetTypeFilter(),
		    lib_group_box->currentText(),this,"lr");
  lr->exec();
  delete lr;
}


void MainWidget::cartOnItemData(QListViewItem *item)
{
  if((!lib_shownotes_box->isChecked())||(item==NULL)) {
    return;
  }
  lib_cart_tip->
    setCartNumber(lib_cart_list->itemRect(item),item->text(1).toUInt());
}


void MainWidget::cartClickedData(QListViewItem *item)
{
  int del_count=0;
  int sel_count=0;
  QListViewItemIterator *it;

  it=new QListViewItemIterator(lib_cart_list);
  while(it->current()) {
    if (it->current()->isSelected()) {
      sel_count++;
      if(it->current()->text(17).isEmpty()) {
        del_count++;
      }
    }
    ++(*it);
  }
  delete it;
  
  if(del_count>0) {
    lib_delete_button->setEnabled(lib_user->deleteCarts());
    } 
  else {
    lib_delete_button->setEnabled(false);
  }
  if(sel_count>1) {
  	if(del_count==0) {
      lib_edit_button->setEnabled(false);
  	  }
    else {
      lib_edit_button->setEnabled(lib_user->modifyCarts());
      }
    } 
  else {
    lib_edit_button->setEnabled(true);
    }

  if(item==NULL) {
    lib_player->setCart(0);
  }
  else {
    lib_player->setCart(item->text(1).toUInt());
  }
}


void MainWidget::cartDoubleclickedData(QListViewItem *,const QPoint &,int)
{
  editData();
}


void MainWidget::audioChangedData(int state)
{
  filterChangedData("");
}


void MainWidget::macroChangedData(int state)
{
  filterChangedData("");
}


void MainWidget::quitMainWidget()
{
  SaveGeometry();
  lib_lock->unlock();
  lib_player->stop();
  exit(0);
}


void MainWidget::closeEvent(QCloseEvent *e)
{
  quitMainWidget();
}


void MainWidget::resizeEvent(QResizeEvent *e)
{
  switch(lib_filter_mode) {
    case RDStation::FilterSynchronous:
      lib_filter_edit->setGeometry(70,10,e->size().width()-170,20);
      break;

    case RDStation::FilterAsynchronous:
      lib_search_button->setGeometry(e->size().width()-180,10,80,50);
      lib_filter_edit->setGeometry(70,10,e->size().width()-260,20);
      break;
  }
  lib_clear_button->setGeometry(e->size().width()-90,10,80,50);
  lib_filter_label->setGeometry(10,10,55,20);
  lib_group_box->setGeometry(70,40,120,20);
  lib_group_label->setGeometry(10,40,55,20);
  lib_codes_box->setGeometry(300,40,120,20);
  lib_codes_label->setGeometry(195,40,100,20);
  lib_showaudio_box->setGeometry(70,67,15,15);
  lib_showaudio_label->setGeometry(90,65,130,20);
  lib_showmacro_box->setGeometry(230,67,15,15);
  lib_showmacro_label->setGeometry(250,65,130,20);
  lib_shownotes_box->setGeometry(390,67,15,15);
  lib_shownotes_label->setGeometry(410,65,130,20);
  lib_cart_list->
    setGeometry(10,90,e->size().width()-20,e->size().height()-155);
  lib_add_button->setGeometry(10,e->size().height()-60,80,50);
  lib_edit_button->setGeometry(100,e->size().height()-60,80,50);
  lib_delete_button->setGeometry(190,e->size().height()-60,80,50);
  lib_selected_label->setGeometry(290,e->size().height()-55,disk_gauge->sizeHint().width(),20);
  disk_gauge->setGeometry(290,e->size().height()-30,
			  disk_gauge->sizeHint().width(),
			  disk_gauge->sizeHint().height());
  lib_rip_button->setGeometry(490,e->size().height()-60,80,50);
  lib_reports_button->setGeometry(590,e->size().height()-60,80,50);
  lib_player->playButton()->setGeometry(690,e->size().height()-60,80,50);
  lib_player->stopButton()->setGeometry(780,e->size().height()-60,80,50);
  lib_close_button->setGeometry(e->size().width()-90,e->size().height()-60,
				80,50);
}


void MainWidget::RefreshList()
{
  RDSqlQuery *q;
  QString sql;
  RDListViewItem *l=NULL;
  QString type_filter;
  QDateTime current_datetime(QDate::currentDate(),QTime::currentTime());
  unsigned cartnum=0;
  RDCart::Validity validity=RDCart::NeverValid;
  QDateTime end_datetime;

  lib_cart_list->clear();

  lib_search_button->setEnabled(false);
  lib_group_box->setEnabled(false);
  lib_showaudio_box->setEnabled(false);
  lib_codes_box->setEnabled(false);
  lib_showmacro_box->setEnabled(false);

  type_filter=GetTypeFilter();
  if(type_filter.isEmpty()) {
    return;
  }

  sql="select CART.NUMBER,CART.FORCED_LENGTH,CART.TITLE,CART.ARTIST,\
       CART.CLIENT,CART.AGENCY,CART.USER_DEFINED,\
       CART.GROUP_NAME,CART.START_DATETIME,CART.END_DATETIME,CART.TYPE,\
       CART.CUT_QUANTITY,CART.LAST_CUT_PLAYED,CART.PLAY_ORDER,\
       CART.ENFORCE_LENGTH,CART.PRESERVE_PITCH,\
       CART.LENGTH_DEVIATION,CART.OWNER,CART.VALIDITY,GROUPS.COLOR, \
       CUTS.LENGTH,CUTS.EVERGREEN,CUTS.START_DATETIME,CUTS.END_DATETIME,\
       CUTS.START_DAYPART,CUTS.END_DAYPART,CUTS.MON,CUTS.TUE,\
       CUTS.WED,CUTS.THU,CUTS.FRI,CUTS.SAT,CUTS.SUN from CART \
       left join GROUPS on CART.GROUP_NAME=GROUPS.NAME \
       left join CUTS on CART.NUMBER=CUTS.CART_NUMBER";
  QString group=lib_group_box->currentText();
  if(group==QString(tr("ALL"))) {
    sql+=QString().
      sprintf(" where %s && %s",
	      (const char *)RDAllCartSearchText(lib_filter_edit->text(),
						lib_user->name()).utf8(),
	      (const char *)type_filter);
  }
  else {
    sql+=QString().
      sprintf(" where %s && %s",
	      (const char *)RDCartSearchText(lib_filter_edit->text(),group).utf8(),
	      (const char *)type_filter);
  }
  QString code=lib_codes_box->currentText();
  if(code!=QString("")) {
  	code+="          ";
    code=code.left(11);
  	sql+=QString().sprintf(" && SCHED_CODES like \"%%%s%%\"",(const char *)code);
    }  
  
  sql+=" order by CART.NUMBER";
  q=new RDSqlQuery(sql);
  int step=0;
  int count=0;
  lib_progress_dialog->setTotalSteps(q->size()/RDLIBRARY_STEP_SIZE);
  lib_progress_dialog->setProgress(0);
  while(q->next()) {
    end_datetime=q->value(9).toDateTime();
    if(q->value(0).toUInt()==cartnum) {
      if((RDCart::Type)q->value(10).toUInt()==RDCart::Macro) {
	validity=RDCart::AlwaysValid;
      }
      else {
	validity=ValidateCut(q,20,validity,current_datetime);
      }
    }
    else {
      //
      // Write availability color
      //
      UpdateItemColor(l,validity,q->value(9).toDateTime(),current_datetime);

      //
      // Start a new entry
      //
      if((RDCart::Type)q->value(10).toUInt()==RDCart::Macro) {
	validity=RDCart::AlwaysValid;
      }
      else {
	validity=ValidateCut(q,20,RDCart::NeverValid,current_datetime);
      }
      l=new RDListViewItem(lib_cart_list);
      switch((RDCart::Type)q->value(10).toUInt()) {
	  case RDCart::Audio:
	    if(q->value(17).isNull()) {
	      l->setPixmap(0,*lib_playout_map);
	    }
	    else {
	      l->setPixmap(0,*lib_track_cart_map);
	    }
	    break;
	    
	  case RDCart::Macro:
	    l->setPixmap(0,*lib_macro_map);
	    l->setBackgroundColor(backgroundColor());
	    break;
	    
	  default:
	    break;
      }
      l->setText(1,QString().sprintf("%06d",q->value(0).toUInt()));
      l->setText(2,q->value(7).toString());
      l->setTextColor(2,q->value(19).toString(),QFont::Bold);
      l->setText(3,RDGetTimeLength(q->value(1).toUInt()));
      l->setText(4,q->value(2).toString());
      l->setText(5,q->value(3).toString());
      if(!q->value(8).toDateTime().isNull()) {
	l->setText(6,q->value(8).toDateTime().toString("MM/dd/yyyy - hh:mm:ss"));
      }
      if(!q->value(9).toDateTime().isNull()) {
	l->setText(7,q->value(9).toDateTime().toString("MM/dd/yyyy - hh:mm:ss"));
      }
      else {
	l->setText(7,"TFN");
      }
      l->setText(8,q->value(4).toString());
      l->setText(9,q->value(5).toString());
      l->setText(10,q->value(6).toString());
      l->setText(11,q->value(11).toString());
      l->setText(12,q->value(12).toString());
      switch((RDCart::PlayOrder)q->value(13).toInt()) {
	  case RDCart::Sequence:
	    l->setText(13,tr("Sequence"));
	    break;
	    
	  case RDCart::Random:
	    l->setText(13,tr("Random"));
	    break;
      }
      l->setText(14,q->value(14).toString());
      l->setText(15,q->value(15).toString());
      l->setText(16,q->value(16).toString());
      l->setText(17,q->value(17).toString());
      if(q->value(14).toString()=="Y") {
	l->setTextColor(3,QColor(RDLIBRARY_ENFORCE_LENGTH_COLOR),QFont::Bold);
      }
      else {
	if((q->value(16).toUInt()>RDLIBRARY_MID_LENGTH_LIMIT)&&
	   (q->value(14).toString()=="N")) {
	  if(q->value(16).toUInt()>RDLIBRARY_MAX_LENGTH_LIMIT) {
	    l->setTextColor(3,QColor(RDLIBRARY_MAX_LENGTH_COLOR),QFont::Bold);
	  }
	  else {
	    l->setTextColor(3,QColor(RDLIBRARY_MID_LENGTH_COLOR),QFont::Bold);
	  }
	}
	else {
	  l->setTextColor(3,QColor(black),QFont::Normal);
	}
      }
    }
    cartnum=q->value(0).toUInt();
    if(count++>RDLIBRARY_STEP_SIZE) {
      lib_progress_dialog->setProgress(++step);
      count=0;
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
    }
  }
  UpdateItemColor(l,validity,end_datetime,current_datetime);
  lib_progress_dialog->reset();
  delete q;

  lib_search_button->setEnabled(true);
  lib_group_box->setEnabled(true);
  lib_showaudio_box->setEnabled(true);
  lib_codes_box->setEnabled(true);
  lib_showmacro_box->setEnabled(true);
  lib_selected_label->setText(QString().sprintf("Filtered  %d Carts",lib_cart_list->childCount()));
}


void SigHandler(int signo)
{
  pid_t pLocalPid;

  switch(signo) {
      case SIGCHLD:
	pLocalPid=waitpid(-1,NULL,WNOHANG);
	while(pLocalPid>0) {
	  pLocalPid=waitpid(-1,NULL,WNOHANG);
	}
	ripper_running=false;
	import_active=false;
	signal(SIGCHLD,SigHandler);
	break;
  }
}


void MainWidget::RefreshLine(RDListViewItem *item)
{
  RDCart::Validity validity=RDCart::NeverValid;
  QDateTime current_datetime(QDate::currentDate(),QTime::currentTime());
  QString sql=QString().sprintf("select CART.FORCED_LENGTH,CART.TITLE,\
                                 CART.ARTIST,CART.CLIENT,\
                                 CART.AGENCY,CART.USER_DEFINED,\
                                 CART.GROUP_NAME,CART.START_DATETIME,\
                                 CART.END_DATETIME,CART.TYPE,\
                                 CART.CUT_QUANTITY,CART.LAST_CUT_PLAYED,\
                                 CART.PLAY_ORDER,CART.ENFORCE_LENGTH,\
                                 CART.PRESERVE_PITCH,\
                                 CART.LENGTH_DEVIATION,CART.OWNER,\
                                 CART.VALIDITY,GROUPS.COLOR,CUTS.LENGTH,\
                                 CUTS.EVERGREEN,CUTS.START_DATETIME,\
                                 CUTS.END_DATETIME,CUTS.START_DAYPART,\
                                 CUTS.END_DAYPART,CUTS.MON,CUTS.TUE,\
                                 CUTS.WED,CUTS.THU,CUTS.FRI,CUTS.SAT,CUTS.SUN \
                                 from CART left join GROUPS on \
                                 CART.GROUP_NAME=GROUPS.NAME left join \
                                 CUTS on CART.NUMBER=CUTS.CART_NUMBER \
                                 where CART.NUMBER=%u",
				item->text(1).toUInt());
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    if((RDCart::Type)q->value(9).toUInt()==RDCart::Macro) {
      validity=RDCart::AlwaysValid;
    }
    else {
      validity=ValidateCut(q,19,validity,current_datetime);
    }
    switch((RDCart::Type)q->value(9).toUInt()) {
	case RDCart::Audio:
	  if(q->value(16).isNull()) {
	    item->setPixmap(0,*lib_playout_map);
	  }
	  else {
	    item->setPixmap(0,*lib_track_cart_map);
	  }
	  if(q->value(0).toUInt()==0) {
	    item->setBackgroundColor(RD_CART_ERROR_COLOR);
	  }
	  else {
	    UpdateItemColor(item,(RDCart::Validity)q->value(17).toUInt(),
			    q->value(8).toDateTime(),current_datetime);
	  }
	  break;

	case RDCart::Macro:
	  item->setPixmap(0,*lib_macro_map);
	  break;

	default:
	  break;
    }
    item->setText(2,q->value(6).toString());
    item->setTextColor(2,q->value(18).toString(),QFont::Bold);
    item->setText(3,RDGetTimeLength(q->value(0).toUInt()));
    item->setText(4,q->value(1).toString());
    item->setText(5,q->value(2).toString());
    item->setText(8,q->value(3).toString());
    item->setText(9,q->value(4).toString());
    item->setText(10,q->value(5).toString());
    if(!q->value(7).toDateTime().isNull()) {
      item->setText(6,q->value(7).toDateTime().
		    toString("MM/dd/yyyy - hh:mm:ss"));
    }
    else {
      item->setText(6,"");
    }
    if(!q->value(8).toDateTime().isNull()) {
      item->setText(7,q->value(8).toDateTime().
		    toString("MM/dd/yyyy - hh:mm:ss"));
    }
    else {
      item->setText(7,tr("TFN"));
    }
    item->setText(11,q->value(10).toString());
    item->setText(12,q->value(11).toString());
    switch((RDCart::PlayOrder)q->value(12).toInt()) {
	case RDCart::Sequence:
	  item->setText(13,tr("Sequence"));
	  break;

	case RDCart::Random:
	  item->setText(13,tr("Random"));
	  break;
    }
    item->setText(14,q->value(13).toString());
    item->setText(15,q->value(14).toString());
    item->setText(16,q->value(15).toString());
    item->setText(17,q->value(16).toString());
    if(q->value(13).toString()=="Y") {
      item->setTextColor(3,QColor(RDLIBRARY_ENFORCE_LENGTH_COLOR),QFont::Bold);
    }
    else {
      if((q->value(15).toUInt()>RDLIBRARY_MID_LENGTH_LIMIT)&&
	 (q->value(13).toString()=="N")) {
	if(q->value(15).toUInt()>RDLIBRARY_MAX_LENGTH_LIMIT) {
	  item->setTextColor(3,QColor(RDLIBRARY_MAX_LENGTH_COLOR),QFont::Bold);
	}
	else {
	  item->setTextColor(3,QColor(RDLIBRARY_MID_LENGTH_COLOR),QFont::Bold);
	}
      }
      else {
	item->setTextColor(3,QColor(black),QFont::Normal);
      }
    }
  }
  delete q;
}


void MainWidget::UpdateItemColor(RDListViewItem *item,
				 RDCart::Validity validity,
				 const QDateTime &end_datetime,
				 const QDateTime &current_datetime)
{
  if(item!=NULL) {
    switch(validity) {
      case RDCart::NeverValid:
	item->setBackgroundColor(RD_CART_ERROR_COLOR);
	break;
	
      case RDCart::ConditionallyValid:
	if(end_datetime.isValid()&&
	   (end_datetime<current_datetime)) {
	  item->setBackgroundColor(RD_CART_ERROR_COLOR);
	}
	else {
	  item->setBackgroundColor(RD_CART_CONDITIONAL_COLOR);
	}
	break;
	
      case RDCart::AlwaysValid:
	item->setBackgroundColor(palette().color(QPalette::Active,
					      QColorGroup::Base));
	break;
	
      case RDCart::EvergreenValid:
	item->setBackgroundColor(RD_CART_EVERGREEN_COLOR);
	break;
    }
  }
}


void MainWidget::SetCaption(QString user)
{
  QString str1;
  QString str2;

  str1=QString(tr("RDLibrary - Host:"));
  str2=QString(tr(", User:"));
  setCaption(QString().sprintf("%s %s, %s %s",
			       (const char *)str1,
			       (const char *)lib_config->stationName(),
			       (const char *)str2,
			       (const char *)user));
  
}


QString MainWidget::GetTypeFilter()
{
  QString type_filter;

  if(lib_showaudio_box->isChecked()) {
    if(lib_showmacro_box->isChecked()) {
      type_filter="((TYPE=1)||(TYPE=2))";
    }
    else {
      type_filter="(TYPE=1)";
    }
  }
  else {
    if(lib_showmacro_box->isChecked()) {
      type_filter="(TYPE=2)";
    }
  }
  return type_filter;
}


void MainWidget::LoadGeometry()
{
  if(getenv("HOME")==NULL) {
    return;
  }
  RDProfile *profile=new RDProfile();
  profile->
    setSource(QString().sprintf("%s/%s",getenv("HOME"),
				RDLIBRARY_GEOMETRY_FILE));
  resize(profile->intValue("RDLibrary","Width",sizeHint().width()),
	 profile->intValue("RDLibrary","Height",sizeHint().height()));
  lib_shownotes_box->
    setChecked(profile->boolValue("RDLibrary","ShowNoteBubbles",true));

  delete profile;
}


void MainWidget::SaveGeometry()
{
  if(getenv("HOME")==NULL) {
    return;
  }
  FILE *file=fopen((const char *)QString().
		   sprintf("%s/%s",getenv("HOME"),RDLIBRARY_GEOMETRY_FILE),
		   "w");
  if(file==NULL) {
    return;
  }
  fprintf(file,"[RDLibrary]\n");
  fprintf(file,"Width=%d\n",geometry().width());
  fprintf(file,"Height=%d\n",geometry().height());
  fprintf(file,"ShowNoteBubbles=");
  if(lib_shownotes_box->isChecked()) {
    fprintf(file,"Yes\n");
  }
  else {
    fprintf(file,"No\n");
  }

  fclose(file);
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  
  //
  // Load Translations
  //
  QTranslator qt(0);
  qt.load(QString(QTDIR)+QString("/translations/qt_")+QTextCodec::locale(),
	  ".");
  a.installTranslator(&qt);

  QTranslator rd(0);
  rd.load(QString(PREFIX)+QString("/share/rivendell/librd_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&rd);

  QTranslator rdhpi(0);
  rdhpi.load(QString(PREFIX)+QString("/share/rivendell/librdhpi_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&rdhpi);

  QTranslator tr(0);
  tr.load(QString(PREFIX)+QString("/share/rivendell/rdlibrary_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&tr);

  //
  // Start Event Loop
  //
  MainWidget *w=new MainWidget(NULL,"main");
  a.setMainWidget(w);
  w->show();
  return a.exec();
}

