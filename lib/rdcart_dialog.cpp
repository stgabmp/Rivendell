// rdcart_dialog.cpp
//
// A widget to select a Rivendell Cart.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcart_dialog.cpp,v 1.40.2.2 2009/11/25 17:19:24 cvs Exp $
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
#include <qsqlquery.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qeventloop.h>

#include <rdconf.h>
#include <rdcart_dialog.h>
#include <rdcart_search_text.h>
#include <rdtextvalidator.h>
#include <rddb.h>

//
// Icons
//
#include "../icons/play.xpm"
#include "../icons/rml5.xpm"


RDCartDialog::RDCartDialog(QString *filter,QString *group,
			   int audition_card,int audition_port,
			   unsigned start_cart,unsigned end_cart,RDCae *cae,
			   RDRipc *ripc,RDStation *station,
			   const QString &edit_cmd,
			   QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());

  cart_cartnum=NULL;
  cart_type=RDCart::All;
  cart_group=group;
  cart_edit_cmd=edit_cmd;
#ifdef WIN32
  cart_filter_mode=RDStation::FilterSynchronous;
#else
  cart_filter_mode=station->filterMode();
#endif  // WIN32

  if(filter==NULL) {
    cart_filter=new QString();
    local_filter=true;
  }
  else {
    cart_filter=filter;
    local_filter=false;
  }

  setCaption(tr("Select Cart"));

  // 
  // Create Fonts
  //
  QFont button_font=QFont("Helvetica",12,QFont::Bold);
  button_font.setPixelSize(12);
  QFont progress_font=QFont("Helvetica",16,QFont::Bold);
  progress_font.setPixelSize(16);

  //
  // Create Icons
  //
  cart_playout_map=new QPixmap(play_xpm);
  cart_macro_map=new QPixmap(rml5_xpm);

  //
  // Text Validator
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator",true);

  //
  // Progress Dialog
  //
  cart_progress_dialog=
    new QProgressDialog(tr("Please Wait..."),"Cancel",10,this,
			"cart_progress_dialog",false,
			Qt::WStyle_Customize|Qt::WStyle_NormalBorder);
  cart_progress_dialog->setCaption(" ");
  QLabel *label=new QLabel(tr("Please Wait..."),cart_progress_dialog);
  label->setAlignment(AlignCenter);
  label->setFont(progress_font);
  cart_progress_dialog->setLabel(label);
  cart_progress_dialog->setCancelButton(NULL);
  cart_progress_dialog->setMinimumDuration(2000);

  //
  // Filter Selector
  //
  cart_filter_edit=new QLineEdit(this,"cart_filter_edit");
  cart_filter_edit->setValidator(validator);
  cart_filter_label=new QLabel(cart_filter_edit,tr("Cart Filter:"),
		   this,"cart_filter_label");
  cart_filter_label->setAlignment(AlignRight|AlignVCenter);
  cart_filter_label->setFont(button_font);
  connect(cart_filter_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(filterChangedData(const QString &)));

  //
  // Filter Search Button
  //
  cart_search_button=new QPushButton(this,"cart_search_button");
  cart_search_button->setText(tr("&Search"));
  cart_search_button->setFont(button_font);
  connect(cart_search_button,SIGNAL(clicked()),this,SLOT(filterSearchedData()));

  //
  // Filter Clear Button
  //
  cart_clear_button=new QPushButton(this,"cart_clear_button");
  cart_clear_button->setText(tr("C&lear"));
  cart_clear_button->setFont(button_font);
  connect(cart_clear_button,SIGNAL(clicked()),this,SLOT(filterClearedData()));

  //
  // Group Selector
  //
  cart_group_box=new RDComboBox(this,"cart_group_box");
  cart_group_label=new QLabel(cart_group_box,tr("Group:"),
			   this,"cart_group_label");
  cart_group_label->setAlignment(AlignRight|AlignVCenter);
  cart_group_label->setFont(button_font);
  connect(cart_group_box,SIGNAL(activated(const QString &)),
	  this,SLOT(groupActivatedData(const QString &)));

  //
  // Cart List
  //
  cart_cart_list=new RDListView(this,"cart_cart_list");
  cart_cart_list->setSelectionMode(QListView::Single);
  cart_cart_list->setAllColumnsShowFocus(true);
  cart_cart_list->setItemMargin(5);
  connect(cart_cart_list,SIGNAL(clicked(QListViewItem *)),
	  this,SLOT(clickedData(QListViewItem *)));
  connect(cart_cart_list,
	  SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	  this,
	  SLOT(doubleClickedData(QListViewItem *,const QPoint &,int)));
  cart_cart_label=new QLabel(cart_cart_list,"Carts",this,"cart_cart_label");
  cart_cart_label->setFont(button_font);
  cart_cart_list->addColumn("");
  cart_cart_list->setColumnAlignment(0,Qt::AlignHCenter);
  cart_cart_list->addColumn(tr("NUMBER"));
  cart_cart_list->setColumnAlignment(1,Qt::AlignHCenter);
  cart_cart_list->addColumn(tr("LENGTH"));
  cart_cart_list->setColumnAlignment(2,Qt::AlignRight);
  cart_cart_list->addColumn(tr("TITLE"),200);
  cart_cart_list->setColumnAlignment(3,Qt::AlignLeft);
  cart_cart_list->setColumnWidthMode(3,QListView::Manual);
  cart_cart_list->addColumn(tr("ARTIST"));
  cart_cart_list->setColumnAlignment(4,Qt::AlignLeft);
  cart_cart_list->addColumn(tr("GROUP"));
  cart_cart_list->setColumnAlignment(5,Qt::AlignLeft);
  cart_cart_list->addColumn(tr("CLIENT"));
  cart_cart_list->setColumnAlignment(6,Qt::AlignLeft);
  cart_cart_list->addColumn(tr("AGENCY"));
  cart_cart_list->setColumnAlignment(7,Qt::AlignLeft);
  cart_cart_list->addColumn(tr("USER DEF"));
  cart_cart_list->setColumnAlignment(8,Qt::AlignLeft);
  cart_cart_list->addColumn(tr("START"));
  cart_cart_list->setColumnAlignment(9,Qt::AlignLeft);
  cart_cart_list->addColumn(tr("END"));
  cart_cart_list->setColumnAlignment(10,Qt::AlignLeft);

  //
  // Audition Player
  //
#ifndef WIN32
  if((cae==NULL)||(audition_card<0)||(audition_port<0)) {
    cart_player=NULL;
  }
  else {
    cart_player=new RDSimplePlayer(cae,ripc,audition_card,audition_port,
				   start_cart,end_cart,this,"cart_player");
    cart_player->playButton()->setDisabled(true);
    cart_player->stopButton()->setDisabled(true);
    cart_player->stopButton()->setOnColor(red);
  }
#endif  // WIN32

  //
  // Send to Editor Button
  //
  cart_editor_button=
    new QPushButton(tr("Send to\n&Editor"),this,"cart_editor_button");
  cart_editor_button->setFont(button_font);
  connect(cart_editor_button,SIGNAL(clicked()),this,SLOT(editorData()));
  if(edit_cmd.isEmpty()) {
    cart_editor_button->hide();
  }

  //
  // OK Button
  //
  cart_ok_button=new QPushButton(tr("&OK"),this,"cart_ok_button");
  cart_ok_button->setFont(button_font);
  connect(cart_ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  // Cancel Button
  //
  cart_cancel_button=new QPushButton(tr("&Cancel"),this,"cart_cancel_button");
  cart_cancel_button->setFont(button_font);
  connect(cart_cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));
}


RDCartDialog::~RDCartDialog()
{
  if(local_filter) {
    delete cart_filter;
  }
#ifndef WIN32
  if(cart_player!=NULL) {
    delete cart_player;
  }
#endif  // WIN32
  delete cart_playout_map;
  delete cart_macro_map;
}


QSize RDCartDialog::sizeHint() const
{
  return QSize(550,370);
}


int RDCartDialog::exec(int *cartnum,RDCart::Type type,QString *svcname,
		       int svc_quan)
{
  cart_cartnum=cartnum;
  cart_type=type;
  cart_service=svcname;
  cart_service_quan=svc_quan;
  switch(cart_type) {
    case RDCart::All:
    case RDCart::Audio:
      if(cart_edit_cmd.isEmpty()) {
	cart_editor_button->hide();
      }
      else {
	cart_editor_button->show();
      }
#ifndef WIN32
      if(cart_player!=NULL) {
	cart_player->playButton()->show();
	cart_player->stopButton()->show();
      }
#endif  // WIN32
      break;

    case RDCart::Macro:
      cart_editor_button->hide();
#ifndef WIN32
      if(cart_player!=NULL) {
	cart_player->playButton()->hide();
	cart_player->stopButton()->hide();
      }
#endif  // WIN32
      break;
  }
  if(*cart_cartnum==0) {
    cart_ok_button->setDisabled(true);
  }
  switch(cart_filter_mode) {
    case RDStation::FilterAsynchronous:
      cart_search_button->setDefault(true);
      break;

    case RDStation::FilterSynchronous:
      cart_ok_button->setDefault(true);
      cart_search_button->hide();
  }
  BuildGroupList();
  cart_filter_edit->setText(*cart_filter);
  RefreshCarts();
  RDListViewItem *item=(RDListViewItem *)cart_cart_list->firstChild();
  while(item!=NULL) {
    if(item->text(1).toInt()==*cartnum) {
      cart_cart_list->setSelected(item,true);
      cart_cart_list->ensureItemVisible(item);
      clickedData(item);
      return QDialog::exec();
    }
    item=(RDListViewItem *)item->nextSibling();
  }
  return QDialog::exec();
}


QSizePolicy RDCartDialog::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void RDCartDialog::filterChangedData(const QString &str)
{
  cart_search_button->setEnabled(true);
  switch(cart_filter_mode) {
    case RDStation::FilterSynchronous:
      filterSearchedData();
      break;

    case RDStation::FilterAsynchronous:
      break;
  }
}


void RDCartDialog::filterSearchedData()
{
  if(cart_filter_edit->text().isEmpty()) {
    cart_clear_button->setDisabled(true);
  }
  else {
    cart_clear_button->setEnabled(true);
  }
  RefreshCarts();
}


void RDCartDialog::filterClearedData()
{
  cart_filter_edit->clear();
  filterChangedData("");
}


void RDCartDialog::groupActivatedData(const QString &group)
{
  filterChangedData("");
  if(cart_group!=NULL) {
    *cart_group=group;
  }
}


void RDCartDialog::clickedData(QListViewItem *item)
{
  RDListViewItem *i=(RDListViewItem *)item;
  if (i==NULL) {
    return;
  }
  cart_ok_button->setEnabled(true);
  bool audio=((RDCart::Type)i->id())==RDCart::Audio;
#ifndef WIN32
  if(cart_player!=NULL) {
    cart_player->playButton()->setEnabled(audio);
    cart_player->stopButton()->setEnabled(audio);
    cart_player->setCart(i->text(1).toUInt());
  }
#endif  // WIN32
  cart_editor_button->setEnabled(audio);
}


void RDCartDialog::doubleClickedData(QListViewItem *,const QPoint &,int)
{
  okData();
}


void RDCartDialog::editorData()
{
#ifndef WIN32
  RDListViewItem *item=(RDListViewItem *)cart_cart_list->currentItem();
  if(item==NULL) {
    return;
  }

  QString sql;
  RDSqlQuery *q;

  sql=QString().sprintf("select CUTS.CUT_NAME,CUTS.LENGTH,CART.GROUP_NAME,\
                         CART.TITLE,CART.ARTIST,CART.ALBUM,CART.YEAR,\
                         CART.LABEL,CART.CLIENT,CART.AGENCY,CART.COMPOSER,\
                         CART.PUBLISHER,CART.USER_DEFINED \
                         from CUTS left join CART \
                         on CUTS.CART_NUMBER=CART.NUMBER \
                         where (CUTS.CART_NUMBER=%u)&&(CUTS.LENGTH>0)",
			item->text(1).toUInt());
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return;
  }
  QString cmd=cart_edit_cmd;
  cmd.replace("%f",RDCut::pathName(q->value(0).toString()));
  cmd.replace("%n",QString().sprintf("%06u",item->text(1).toUInt()));
  cmd.replace("%h",QString().sprintf("%d",q->value(1).toInt()));
  cmd.replace("%g",q->value(2).toString());
  cmd.replace("%t",q->value(3).toString());
  cmd.replace("%a",q->value(4).toString());
  cmd.replace("%l",q->value(5).toString());
  cmd.replace("%y",q->value(6).toString());
  cmd.replace("%b",q->value(7).toString());
  cmd.replace("%c",q->value(8).toString());
  cmd.replace("%e",q->value(9).toString());
  cmd.replace("%m",q->value(10).toString());
  cmd.replace("%p",q->value(11).toString());
  cmd.replace("%u",q->value(12).toString());
  delete q;

  if(fork()==0) {
    system(cmd+" &");
    exit(0);
  }
#endif
}


void RDCartDialog::okData()
{
  RDListViewItem *item=(RDListViewItem *)cart_cart_list->currentItem();
  if(item==NULL) {
    return;
  }

#ifndef WIN32
  if(cart_player!=NULL) {
    cart_player->stop();
  }
#endif  // WIN32
  if(!local_filter) {
    *cart_filter=cart_filter_edit->text();
  }
  *cart_cartnum=item->text(1).toInt();
  done(0);
}


void RDCartDialog::cancelData()
{
#ifndef WIN32
  if(cart_player!=NULL) {
    cart_player->stop();
  }
#endif  // WIN32
  done(-1);
}


void RDCartDialog::resizeEvent(QResizeEvent *e)
{
  cart_filter_label->setGeometry(10,10,85,20);

  cart_search_button->setGeometry(size().width()-160,5,70,30);
  cart_clear_button->setGeometry(size().width()-80,5,70,30);
  cart_group_box->setGeometry(100,35,150,20);
  cart_group_label->setGeometry(10,35,85,20);
  cart_cart_label->setGeometry(15,60,100,20);
  cart_cart_list->setGeometry(10,80,size().width()-20,size().height()-150);
  cart_editor_button->setGeometry(235,size().height()-60,80,50);
  cart_ok_button->setGeometry(size().width()-180,size().height()-60,80,50);
  cart_cancel_button->setGeometry(size().width()-90,size().height()-60,80,50);
  switch(cart_filter_mode) {
    case RDStation::FilterAsynchronous:
      cart_filter_edit->setGeometry(100,10,size().width()-280,20);
      break;

    case RDStation::FilterSynchronous:
      cart_filter_edit->setGeometry(100,10,size().width()-200,20);
      break;
  }
#ifndef WIN32
  if(cart_player!=NULL) {
    cart_player->playButton()->setGeometry(10,size().height()-60,80,50);
    cart_player->stopButton()->setGeometry(100,size().height()-60,80,50);
  }
#endif  // WIN32
}


void RDCartDialog::closeEvent(QCloseEvent *e)
{
#ifndef WIN32
  if(cart_player!=NULL) {
    cart_player->stop();
  }
#endif  // WIN32
  cancelData();
}


void RDCartDialog::RefreshCarts()
{
  QString sql;
  RDSqlQuery *q;
  RDListViewItem *l;
  RDListViewItem *active_item=NULL;

  cart_cart_list->clear();
  QString group=cart_group_box->currentText();
  if(group==QString(tr("ALL"))) {
    group="";
  }
  if(cart_type==RDCart::All) {
    sql=QString().sprintf("select CART.NUMBER,CART.TITLE,CART.ARTIST,\
                           CART.CLIENT,CART.AGENCY,CART.USER_DEFINED,\
                           CART.START_DATETIME,CART.END_DATETIME,CART.TYPE,\
                           CART.FORCED_LENGTH,CART.GROUP_NAME,GROUPS.COLOR \
                           from CART left join GROUPS \
                           on CART.GROUP_NAME=GROUPS.NAME where %s",
		 (const char *)GetSearchFilter(cart_filter_edit->text(),
						group));
  }
  else {
    sql=QString().sprintf("select CART.NUMBER,CART.TITLE,CART.ARTIST,\
                           CART.CLIENT,CART.AGENCY,CART.USER_DEFINED,\
                           CART.START_DATETIME,CART.END_DATETIME,CART.TYPE,\
                           CART.FORCED_LENGTH,CART.GROUP_NAME,GROUPS.COLOR \
                           from CART left join GROUPS \
                           on CART.GROUP_NAME=GROUPS.NAME \
                           where (%s)&&(TYPE=%d)",
		(const char *)GetSearchFilter(cart_filter_edit->text(),group),
			  cart_type);
  }
  q=new RDSqlQuery(sql);
  int step=0;
  int count=0;
  cart_progress_dialog->setTotalSteps(q->size()/RDCART_DIALOG_STEP_SIZE);
  cart_progress_dialog->setProgress(0);
  while(q->next()) {
    l=new RDListViewItem(cart_cart_list);
    l->setId(q->value(8).toUInt());
    switch((RDCart::Type)q->value(8).toUInt()) {
	case RDCart::Audio:
	  l->setPixmap(0,*cart_playout_map);
	  break;

	case RDCart::Macro:
	  l->setPixmap(0,*cart_macro_map);
	  break;

	default:
	  break;
    }
    l->setText(1,QString().sprintf("%06d",q->value(0).toUInt())); // Number
    l->setText(2,RDGetTimeLength(q->value(9).toInt(),false,true)); // Length
    l->setText(3,q->value(1).toString());                       // Title
    l->setText(4,q->value(2).toString());                       // Artist
    l->setText(5,q->value(10).toString());                      // Group
    l->setTextColor(5,q->value(11).toString(),QFont::Bold);
    l->setText(6,q->value(3).toString());                       // Client
    l->setText(7,q->value(4).toString());                       // Agency
    l->setText(8,q->value(5).toString());                       // User Defined
    if(!q->value(6).toDate().isNull()) {
      l->setText(9,q->value(6).toDate().toString("MM/dd/yyyy"));  // Start Date
    }
    if(!q->value(8).toDate().isNull()) {
      l->setText(10,q->value(7).toDate().toString("MM/dd/yyyy"));  // End Date
    }
    else {
      l->setText(10,"TFN");
    }
    if(*cart_cartnum==q->value(0).toInt()) {
      active_item=l;
    }
    if(count++>RDCART_DIALOG_STEP_SIZE) {
      cart_progress_dialog->setProgress(++step);
      count=0;
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
    }
  }
  cart_progress_dialog->reset();
  delete q;
  cart_search_button->setDisabled(true);
}


void RDCartDialog::BuildGroupList()
{
  QString sql;
  RDSqlQuery *q;
  
  cart_group_box->clear();
  cart_group_box->insertItem(tr("ALL"));
  sql="select GROUP_NAME from AUDIO_PERMS";
  if(cart_service_quan>0) {
    sql+=" where ";
    for(int i=0;i<cart_service_quan;i++) {
      if(!cart_service[i].isEmpty()) {
	sql+=QString().sprintf("(SERVICE_NAME=\"%s\")||",
			       (const char *)cart_service[i]);
      }
    }
    sql=sql.left(sql.length()-2);
  }
  sql+=" order by GROUP_NAME";
  q=new RDSqlQuery(sql);
  while(q->next()) {
    cart_group_box->insertItem(q->value(0).toString(),true);
  }
  delete q;

  //
  // Preselect Group
  //
  if(cart_group!=NULL) {
    for(int i=0;i<cart_group_box->count();i++) {
      if(*cart_group==cart_group_box->text(i)) {
	cart_group_box->setCurrentItem(i);
	return;
      }
    }
  }
}


QString RDCartDialog::GetSearchFilter(QString filter,QString group)
{
  QString sql;
  RDSqlQuery *q;

  QString search=RDCartSearchText(filter,group).utf8();

  //
  // Excluded Groups
  //
  sql=QString().sprintf("select NAME from GROUPS where ");
  for(int i=1;i<cart_group_box->count();i++) {
    sql+=QString().sprintf("(NAME!=\"%s\")&&",
			   (const char *)cart_group_box->text(i));
  }
  sql=sql.left(sql.length()-2);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    search+=QString().sprintf("&&(GROUP_NAME!=\"%s\")",
			      (const char *)q->value(0).toString());
  }
  delete q;
  return search;
}

