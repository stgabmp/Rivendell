// edit_matrix.cpp
//
// Edit a Rivendell Matrix
//
//   (C) Copyright 2002-2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_matrix.cpp,v 1.32.2.2.2.2 2010/05/08 23:01:57 cvs Exp $
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
#include <qtextedit.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qsqldatabase.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdtextvalidator.h>
#include <rddb.h>
#include <rdcart_dialog.h>

#include <globals.h>
#include <edit_user.h>
#include <edit_matrix.h>
#include <list_endpoints.h>
#include <list_gpis.h>
#include <list_nodes.h>
#include <list_vguest_resources.h>


EditMatrix::EditMatrix(RDMatrix *matrix,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  QString str;

  edit_matrix=matrix;
  edit_stationname=matrix->station();
  edit_matrix_number=matrix->matrix();

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  setCaption(tr("Edit Switcher"));

  //
  // Create Fonts
  //
  QFont bold_font=QFont("Helvetica",12,QFont::Bold);
  bold_font.setPixelSize(12);
  QFont font=QFont("Helvetica",12,QFont::Normal);
  font.setPixelSize(12);

  //
  // Text Validator
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator");

  //
  // Matrix Number
  //
  QLabel *label=new QLabel(QString().sprintf("%d",edit_matrix_number),
			   this,"edit_matrix_number");
  label->setGeometry(135,10,30,19);
  label->setFont(font);
  label=new QLabel(tr("Matrix Number:"),this,"matrix_label");
  label->setGeometry(10,10,120,19);
  label->setFont(bold_font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Matrix Type
  //
  label=new QLabel(edit_matrix->typeString(),this,"edit_type");
  label->setGeometry(135,30,200,19);
  label->setFont(font);
  label=new QLabel(tr("Switcher Type:"),this,"matrix_label");
  label->setGeometry(10,30,120,19);
  label->setFont(bold_font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Descriptive Name
  //
  edit_name_edit=new QLineEdit(this,"edit_name_edit");
  edit_name_edit->setGeometry(135,50,240,19);
  edit_name_edit->setValidator(validator);
  label=new QLabel(edit_name_edit,tr("Description:"),this,"edit_name_label");
  label->setGeometry(10,50,120,19);
  label->setFont(bold_font);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Primary Connection
  //
  label=new QLabel(tr("Primary Connection"),this);
  label->setGeometry(20,74,130,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  //
  // Primary Type
  //
  edit_porttype_box=new QComboBox(this,"edit_porttype_edit");
  edit_porttype_box->setGeometry(90,96,70,19);
  edit_porttype_label=new QLabel(edit_porttype_box,tr("Type:"),
				 this,"edit_porttype_label");
  edit_porttype_label->setGeometry(15,96,70,19);
  edit_porttype_label->setFont(bold_font);
  edit_porttype_label->setAlignment(AlignRight|AlignVCenter);
  edit_porttype_box->insertItem(tr("Serial"));
  edit_porttype_box->insertItem(tr("TCP/IP"));
  connect(edit_porttype_box,SIGNAL(activated(int)),
	  this,SLOT(portTypeActivatedData(int)));

  //
  // Primary Serial Port
  //
  edit_port_box=new QComboBox(this,"edit_port_box");
  edit_port_box->setGeometry(290,96,90,19);
  edit_port_box->setEditable(false);
  edit_port_label=
    new QLabel(edit_port_box,tr("Serial Port:"),this,"edit_name_label");
  edit_port_label->setGeometry(175,96,90,19);
  edit_port_label->setFont(bold_font);
  edit_port_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Primary IP Address
  //
  edit_ipaddress_edit=new QLineEdit(this,"edit_ipaddress_edit");
  edit_ipaddress_edit->setGeometry(90,118,115,19);
  edit_ipaddress_label=new QLabel(edit_ipaddress_edit,tr("IP Address:"),
				  this,"edit_name_label");
  edit_ipaddress_label->setGeometry(15,118,70,19);
  edit_ipaddress_label->setFont(bold_font);
  edit_ipaddress_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Primary IP Port
  //
  edit_ipport_spin=new QSpinBox(this,"edit_ipport_spin");
  edit_ipport_spin->setGeometry(290,118,65,19);
  edit_ipport_spin->setRange(0,0xFFFF);
  edit_ipport_label=
    new QLabel(edit_ipport_spin,tr("IP Port:"),this,"edit_ipport_label");
  edit_ipport_label->setGeometry(215,118,70,19);
  edit_ipport_label->setFont(bold_font);
  edit_ipport_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Primary Username
  //
  edit_username_edit=new QLineEdit(this,"edit_username_edit");
  edit_username_edit->setGeometry(90,140,115,19);
  edit_username_label=new QLabel(edit_username_edit,tr("Username:"),
				  this,"edit_name_label");
  edit_username_label->setGeometry(15,140,70,19);
  edit_username_label->setFont(bold_font);
  edit_username_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Primary Password
  //
  edit_password_edit=new QLineEdit(this,"edit_password_edit");
  edit_password_edit->setGeometry(290,140,115,19);
  edit_password_edit->setEchoMode(QLineEdit::Password);
  edit_password_label=new QLabel(edit_password_edit,tr("Password:"),
				  this,"edit_name_label");
  edit_password_label->setGeometry(215,140,70,19);
  edit_password_label->setFont(bold_font);
  edit_password_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Primary Start Cart
  //
  edit_start_cart_edit=new QLineEdit(this,"edit_start_cart_edit");
  edit_start_cart_edit->setGeometry(120,164,80,19);
  edit_start_cart_label=new QLabel(edit_start_cart_edit,tr("Startup Cart:"),
				  this,"edit_start_cart_label");
  edit_start_cart_label->setGeometry(15,164,100,19);
  edit_start_cart_label->setFont(bold_font);
  edit_start_cart_label->setAlignment(AlignRight|AlignVCenter);
  edit_start_cart_button=
    new QPushButton(tr("Select"),this,"edit_start_cart_button");
  edit_start_cart_button->setFont(font);
  edit_start_cart_button->setGeometry(205,162,60,24);
  connect(edit_start_cart_button,SIGNAL(clicked()),this,SLOT(startCartData()));

  //
  // Primary Stop Cart
  //
  edit_stop_cart_edit=new QLineEdit(this,"edit_stop_cart_edit");
  edit_stop_cart_edit->setGeometry(120,188,80,19);
  edit_stop_cart_label=new QLabel(edit_stop_cart_edit,tr("Shutdown Cart:"),
				  this,"edit_stop_cart_label");
  edit_stop_cart_label->setGeometry(15,188,100,19);
  edit_stop_cart_label->setFont(bold_font);
  edit_stop_cart_label->setAlignment(AlignRight|AlignVCenter);
  edit_stop_cart_button=
    new QPushButton(tr("Select"),this,"edit_stop_cart_button");
  edit_stop_cart_button->setFont(font);
  edit_stop_cart_button->setGeometry(205,186,60,24);
  connect(edit_stop_cart_button,SIGNAL(clicked()),this,SLOT(stopCartData()));

  //
  // Backup Connection
  //
  label=new QLabel(tr("Backup Connection"),this);
  label->setGeometry(20,221,130,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  //
  // Backup Type
  //
  edit_porttype2_box=new QComboBox(this,"edit_porttype_edit");
  edit_porttype2_box->setGeometry(90,243,70,19);
  edit_porttype2_label=new QLabel(edit_porttype2_box,tr("Type:"),
				 this,"edit_porttype_label");
  edit_porttype2_label->setGeometry(15,243,70,19);
  edit_porttype2_label->setFont(bold_font);
  edit_porttype2_label->setAlignment(AlignRight|AlignVCenter);
  edit_porttype2_box->insertItem(tr("Serial"));
  edit_porttype2_box->insertItem(tr("TCP/IP"));
  edit_porttype2_box->insertItem(tr("None"));
  connect(edit_porttype2_box,SIGNAL(activated(int)),
	  this,SLOT(portType2ActivatedData(int)));

  //
  // Backup Serial Port
  //
  edit_port2_box=new QComboBox(this,"edit_port_box");
  edit_port2_box->setGeometry(290,243,90,19);
  edit_port2_box->setEditable(false);
  edit_port2_label=
    new QLabel(edit_port2_box,tr("Serial Port:"),this,"edit_name_label");
  edit_port2_label->setGeometry(175,243,90,19);
  edit_port2_label->setFont(bold_font);
  edit_port2_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Backup IP Address
  //
  edit_ipaddress2_edit=new QLineEdit(this,"edit_ipaddress_edit");
  edit_ipaddress2_edit->setGeometry(90,265,115,19);
  edit_ipaddress2_label=new QLabel(edit_ipaddress2_edit,tr("IP Address:"),
				  this,"edit_name_label");
  edit_ipaddress2_label->setGeometry(15,265,70,19);
  edit_ipaddress2_label->setFont(bold_font);
  edit_ipaddress2_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Backup IP Port
  //
  edit_ipport2_spin=new QSpinBox(this,"edit_ipport_spin");
  edit_ipport2_spin->setGeometry(290,265,65,19);
  edit_ipport2_spin->setRange(0,0xFFFF);
  edit_ipport2_label=
    new QLabel(edit_ipport2_spin,tr("IP Port:"),this,"edit_ipport_label");
  edit_ipport2_label->setGeometry(215,265,70,19);
  edit_ipport2_label->setFont(bold_font);
  edit_ipport2_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Backup Username
  //
  edit_username2_edit=new QLineEdit(this,"edit_username_edit");
  edit_username2_edit->setGeometry(90,288,115,19);
  edit_username2_label=new QLabel(edit_username2_edit,tr("Username:"),
				  this,"edit_name_label");
  edit_username2_label->setGeometry(15,288,70,19);
  edit_username2_label->setFont(bold_font);
  edit_username2_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Backup Password
  //
  edit_password2_edit=new QLineEdit(this,"edit_password_edit");
  edit_password2_edit->setGeometry(290,288,115,19);
  edit_password2_edit->setEchoMode(QLineEdit::Password);
  edit_password2_label=new QLabel(edit_password2_edit,tr("Password:"),
				  this,"edit_name_label");
  edit_password2_label->setGeometry(215,288,70,19);
  edit_password2_label->setFont(bold_font);
  edit_password2_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Backup Start Cart
  //
  edit_start_cart2_edit=new QLineEdit(this,"edit_start_cart2_edit");
  edit_start_cart2_edit->setGeometry(120,312,80,19);
  edit_start_cart2_label=new QLabel(edit_start_cart2_edit,tr("Startup Cart:"),
				  this,"edit_start_cart2_label");
  edit_start_cart2_label->setGeometry(15,312,100,19);
  edit_start_cart2_label->setFont(bold_font);
  edit_start_cart2_label->setAlignment(AlignRight|AlignVCenter);
  edit_start_cart2_button=
    new QPushButton(tr("Select"),this,"edit_start_cart2_button");
  edit_start_cart2_button->setFont(font);
  edit_start_cart2_button->setGeometry(205,310,60,24);
  connect(edit_start_cart2_button,SIGNAL(clicked()),
	  this,SLOT(startCart2Data()));

  //
  // Backup Stop Cart
  //
  edit_stop_cart2_edit=new QLineEdit(this,"edit_stop_cart2_edit");
  edit_stop_cart2_edit->setGeometry(120,336,80,19);
  edit_stop_cart2_label=new QLabel(edit_stop_cart2_edit,tr("Shutdown Cart:"),
				  this,"edit_stop_cart2_label");
  edit_stop_cart2_label->setGeometry(15,336,100,19);
  edit_stop_cart2_label->setFont(bold_font);
  edit_stop_cart2_label->setAlignment(AlignRight|AlignVCenter);
  edit_stop_cart2_button=
    new QPushButton(tr("Select"),this,"edit_stop_cart2_button");
  edit_stop_cart2_button->setFont(font);
  edit_stop_cart2_button->setGeometry(205,334,60,24);
  connect(edit_stop_cart2_button,SIGNAL(clicked()),this,SLOT(stopCart2Data()));

  //
  // Card Number
  //
  edit_card_box=new QSpinBox(this,"edit_card_box");
  edit_card_box->setGeometry(75,371,50,19);
  edit_card_box->setRange(0,RD_MAX_CARDS-1);
  edit_card_label=
    new QLabel(edit_card_box,tr("Card:"),this,"edit_card_label");
  edit_card_label->setGeometry(10,371,65,19);
  edit_card_label->setFont(bold_font);
  edit_card_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Inputs
  //
  edit_inputs_box=new QSpinBox(this,"edit_inputs_box");
  edit_inputs_box->setGeometry(230,371,50,19);
  edit_inputs_box->setRange(0,MAX_ENDPOINTS);
  edit_inputs_label=
    new QLabel(edit_inputs_box,tr("Inputs:"),this,"edit_inputs_label");
  edit_inputs_label->setGeometry(175,371,50,19);
  edit_inputs_label->setFont(bold_font);
  edit_inputs_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Outputs
  //
  edit_outputs_box=new QSpinBox(this,"edit_outputs_box");
  edit_outputs_box->setGeometry(355,371,50,19);
  edit_outputs_box->setRange(0,MAX_ENDPOINTS);
  edit_outputs_label=
    new QLabel(edit_outputs_box,tr("Outputs:"),this,"edit_outputs_label");
  edit_outputs_label->setGeometry(280,371,70,19);
  edit_outputs_label->setFont(bold_font);
  edit_outputs_label->setAlignment(AlignRight|AlignVCenter);

  //
  // Device
  //
  edit_device_edit=new QLineEdit(this,"edit_device_edit");
  edit_device_edit->setGeometry(75,396,90,19);
  edit_device_edit->setValidator(validator);
  edit_device_label=new QLabel(edit_device_edit,tr("Device:"),
			       this,"edit_name_label");
  edit_device_label->setGeometry(5,396,65,19);
  edit_device_label->setFont(bold_font);
  edit_device_label->setAlignment(AlignRight|AlignVCenter);

  //
  // GPIs
  //
  edit_gpis_box=new QSpinBox(this,"edit_gpis_box");
  edit_gpis_box->setGeometry(230,396,50,19);
  edit_gpis_box->setRange(0,MAX_GPIO_PINS);
  edit_gpis_label=
    new QLabel(edit_gpis_box,tr("GPIs:"),this,"edit_gpis_label");
  edit_gpis_label->setGeometry(175,396,50,19);
  edit_gpis_label->setFont(bold_font);
  edit_gpis_label->setAlignment(AlignRight|AlignVCenter);
  connect(edit_gpis_box,SIGNAL(valueChanged(int)),
	  this,SLOT(gpisChangedData(int)));

  //
  // GPOs
  //
  edit_gpos_box=new QSpinBox(this,"edit_gpos_box");
  edit_gpos_box->setGeometry(355,396,50,19);
  edit_gpos_box->setRange(0,MAX_GPIO_PINS);
  edit_gpos_label=
    new QLabel(edit_gpos_box,tr("GPOs:"),this,"edit_gpos_label");
  edit_gpos_label->setGeometry(280,396,70,19);
  edit_gpos_label->setFont(bold_font);
  edit_gpos_label->setAlignment(AlignRight|AlignVCenter);
  connect(edit_gpos_box,SIGNAL(valueChanged(int)),
	  this,SLOT(gposChangedData(int)));

  //
  // Layer
  //
  edit_layer_box=new QComboBox(this,"edit_layer_box");
  edit_layer_box->setGeometry(75,421,50,19);
  edit_layer_label=
    new QLabel(edit_layer_box,tr("Layer:"),this,"edit_layer_label");
  edit_layer_label->setGeometry(10,421,65,19);
  edit_layer_label->setFont(bold_font);
  edit_layer_label->setAlignment(AlignRight|AlignVCenter);
  edit_layer_box->insertItem("V");
  edit_layer_box->insertItem("A");
  edit_layer_box->insertItem("B");
  edit_layer_box->insertItem("C");
  edit_layer_box->insertItem("D");
  edit_layer_box->insertItem("E");
  edit_layer_box->insertItem("F");
  edit_layer_box->insertItem("G");
  edit_layer_box->insertItem("H");
  edit_layer_box->insertItem("I");
  edit_layer_box->insertItem("J");
  edit_layer_box->insertItem("K");
  edit_layer_box->insertItem("L");
  edit_layer_box->insertItem("M");
  edit_layer_box->insertItem("N");
  edit_layer_box->insertItem("O");

  //
  // Displays
  //
  edit_displays_box=new QSpinBox(this,"edit_displays_box");
  edit_displays_box->setGeometry(355,421,50,19);
  edit_displays_box->setRange(0,1024);
  edit_displays_label=
    new QLabel(edit_displays_box,tr("Displays:"),this,"edit_displays_label");
  edit_displays_label->setGeometry(280,421,70,19);
  edit_displays_label->setFont(bold_font);
  edit_displays_label->setAlignment(AlignRight|AlignVCenter);

  //
  //  Configure Inputs Button
  //
  edit_inputs_button=new QPushButton(this,"inputs_button");
  edit_inputs_button->setGeometry(35,446,80,50);
  edit_inputs_button->setFont(bold_font);
  edit_inputs_button->setText(tr("Configure\n&Inputs"));
  connect(edit_inputs_button,SIGNAL(clicked()),this,SLOT(inputsButtonData()));

  //
  //  Configure Outputs Button
  //
  edit_outputs_button=new QPushButton(this,"outputs_button");
  edit_outputs_button->setGeometry(125,446,80,50);
  edit_outputs_button->setFont(bold_font);
  edit_outputs_button->setText(tr("Configure\n&Outputs"));
  connect(edit_outputs_button,SIGNAL(clicked()),
	  this,SLOT(outputsButtonData()));

  //
  //  Configure Crosspoints Button
  //
  edit_xpoint_button=new QPushButton(this,"xpoints_button");
  edit_xpoint_button->setGeometry(215,446,80,50);
  edit_xpoint_button->setDefault(true);
  edit_xpoint_button->setFont(bold_font);
  edit_xpoint_button->setText(tr("Configure\n&Xpoints"));
  connect(edit_xpoint_button,SIGNAL(clicked()),this,SLOT(xpointsButtonData()));
  edit_xpoint_button->hide();

  //
  //  Configure GPIs Button
  //
  edit_gpis_button=new QPushButton(this,"edit_gpis_button");
  edit_gpis_button->setGeometry(215,446,80,50);
  edit_gpis_button->setDefault(true);
  edit_gpis_button->setFont(bold_font);
  edit_gpis_button->setText(tr("Configure\n&GPIs"));
  connect(edit_gpis_button,SIGNAL(clicked()),this,SLOT(gpisButtonData()));

  //
  //  Configure GPOs Button
  //
  edit_gpos_button=new QPushButton(this,"edit_gpos_button");
  edit_gpos_button->setGeometry(305,446,80,50);
  edit_gpos_button->setDefault(true);
  edit_gpos_button->setFont(bold_font);
  edit_gpos_button->setText(tr("Configure\nG&POs"));
  connect(edit_gpos_button,SIGNAL(clicked()),this,SLOT(gposButtonData()));

  //
  //  LiveWire Nodes Button
  //
  edit_livewire_button=new QPushButton(this,"livewire_button");
  edit_livewire_button->setGeometry(80,506,80,50);
  edit_livewire_button->setFont(bold_font);
  edit_livewire_button->setText(tr("LiveWire\nNodes"));
  connect(edit_livewire_button,SIGNAL(clicked()),
	  this,SLOT(livewireButtonData()));

  //
  //  vGuest switches Button
  //
  edit_vguestrelays_button=new QPushButton(this,"vguestrelays_button");
  edit_vguestrelays_button->setGeometry(170,506,80,50);
  edit_vguestrelays_button->setFont(bold_font);
  edit_vguestrelays_button->setText(tr("vGuest\nSwitches"));
  connect(edit_vguestrelays_button,SIGNAL(clicked()),
	  this,SLOT(vguestRelaysButtonData()));

  //
  //  vGuest Displays Button
  //
  edit_vguestdisplays_button=new QPushButton(this,"vguestdisplays_button");
  edit_vguestdisplays_button->setGeometry(260,506,80,50);
  edit_vguestdisplays_button->setFont(bold_font);
  edit_vguestdisplays_button->setText(tr("vGuest\nDisplays"));
  connect(edit_vguestdisplays_button,SIGNAL(clicked()),
	  this,SLOT(vguestDisplaysButtonData()));

  //
  //  Ok Button
  //
  QPushButton *button=new QPushButton(this,"ok_button");
  button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  button->setDefault(true);
  button->setFont(bold_font);
  button->setText(tr("&OK"));
  connect(button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  button=new QPushButton(this,"cancel_button");
  button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			     80,50);
  button->setFont(bold_font);
  button->setText(tr("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Load Values
  //
  edit_name_edit->setText(edit_matrix->name());
  str=QString(tr("Serial"));
  for(int i=0;i<MAX_TTYS;i++) {
    edit_port_box->insertItem(QString().sprintf("%s%d",(const char *)str,i));
    edit_port2_box->insertItem(QString().sprintf("%s%d",(const char *)str,i));
  }
  edit_porttype_box->
    setCurrentItem((int)edit_matrix->portType(RDMatrix::Primary));
  edit_porttype2_box->
    setCurrentItem((int)edit_matrix->portType(RDMatrix::Backup));
  switch((RDMatrix::PortType)edit_porttype_box->currentItem()) {
      case RDMatrix::TtyPort:
	edit_port_box->setCurrentItem(edit_matrix->port(RDMatrix::Primary));
	edit_port2_box->setCurrentItem(edit_matrix->port(RDMatrix::Backup));
	break;

      case RDMatrix::TcpPort:
	edit_ipaddress_edit->
	  setText(edit_matrix->ipAddress(RDMatrix::Primary).toString());
	edit_ipport_spin->setValue(edit_matrix->ipPort(RDMatrix::Primary));
	edit_ipaddress2_edit->
	  setText(edit_matrix->ipAddress(RDMatrix::Backup).toString());
	edit_ipport2_spin->setValue(edit_matrix->ipPort(RDMatrix::Backup));
	break;

    case RDMatrix::NoPort:
      break;
  }
  edit_card_box->setValue(edit_matrix->card());
  edit_inputs_box->setValue(edit_matrix->inputs());
  edit_outputs_box->setValue(edit_matrix->outputs());
  edit_device_edit->setText(edit_matrix->gpioDevice());
  edit_gpis_box->setValue(edit_matrix->gpis());
  edit_gpos_box->setValue(edit_matrix->gpos());
  edit_username_edit->setText(edit_matrix->username(RDMatrix::Primary));
  edit_password_edit->setText(edit_matrix->password(RDMatrix::Primary));
  edit_username2_edit->setText(edit_matrix->username(RDMatrix::Backup));
  edit_password2_edit->setText(edit_matrix->password(RDMatrix::Backup));
  if(edit_matrix->startCart(RDMatrix::Primary)>0) {
    edit_start_cart_edit->
      setText(QString().sprintf("%06u",
				edit_matrix->startCart(RDMatrix::Primary)));
  }
  if(edit_matrix->stopCart(RDMatrix::Primary)>0) {
    edit_stop_cart_edit->
      setText(QString().sprintf("%06u",
				edit_matrix->stopCart(RDMatrix::Primary)));
  }
  if(edit_matrix->startCart(RDMatrix::Backup)>0) {
    edit_start_cart2_edit->
      setText(QString().sprintf("%06u",
				edit_matrix->startCart(RDMatrix::Backup)));
  }
  if(edit_matrix->stopCart(RDMatrix::Backup)>0) {
    edit_stop_cart2_edit->
      setText(QString().sprintf("%06u",
				edit_matrix->stopCart(RDMatrix::Backup)));
  }
  edit_displays_box->setValue(edit_matrix->displays());
  if(edit_matrix->layer()=='V') {
    edit_layer_box->setCurrentItem(0);
  }
  else {
    edit_layer_box->setCurrentItem(edit_matrix->layer()-'@');
  }
  switch(edit_matrix->type()) {
      case RDMatrix::LocalGpio:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_port_label->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_inputs_button->setDisabled(true);
	edit_outputs_button->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::GenericGpo:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_port_label->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::GenericSerial:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::Unity4000:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_outputs_box->setRange(0,4);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::Sas32000:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::Sas64000:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::BtSs82:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      case RDMatrix::Bt10x1:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::Sas64000Gpi:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setEnabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::Bt16x1:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	break;

      case RDMatrix::Bt8x2:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	break;

      case RDMatrix::BtAcs82:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      case RDMatrix::SasUsi:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_porttype_label->setEnabled(true);
	edit_porttype_box->setEnabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setEnabled(true);
	edit_stop_cart_edit->setEnabled(true);
	edit_start_cart2_edit->setEnabled(true);
	edit_stop_cart2_edit->setEnabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      case RDMatrix::Bt16x2:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      case RDMatrix::BtSs124:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::LocalAudioAdapter:
	edit_port_label->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::LogitekVguest:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_porttype_label->setEnabled(true);
	edit_porttype_box->setEnabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_username_label->setEnabled(true);
	edit_username_edit->setEnabled(true);
	edit_password_label->setEnabled(true);
	edit_password_edit->setEnabled(true);
	edit_displays_label->setEnabled(true);
	edit_displays_box->setEnabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setEnabled(true);
	edit_vguestdisplays_button->setEnabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setEnabled(true);
	edit_stop_cart_edit->setEnabled(true);
	edit_start_cart2_edit->setEnabled(true);
	edit_stop_cart2_edit->setEnabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      case RDMatrix::BtSs164:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::StarGuideIII:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_outputs_box->setRange(0,6);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::BtSs42:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      case RDMatrix::LiveWire:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_port_label->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setEnabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_button->setEnabled(true);
	edit_outputs_button->setEnabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;

      case RDMatrix::Quartz1:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_porttype_label->setEnabled(true);
	edit_porttype_box->setEnabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setEnabled(true);
	edit_porttype2_box->setEnabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_layer_box->setEnabled(true);
	edit_layer_label->setEnabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	break;

      case RDMatrix::BtSs44:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

    case RDMatrix::BtSrc8III:
    case RDMatrix::BtSrc16:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_inputs_button->setDisabled(true);
	edit_outputs_button->setDisabled(true);
	edit_gpis_button->setEnabled(true);
	edit_gpos_button->setEnabled(true);
	break;

      default:
	edit_card_label->setDisabled(true);
	edit_card_box->setDisabled(true);
	edit_porttype_label->setDisabled(true);
	edit_porttype_box->setDisabled(true);
	edit_port_box->setDisabled(true);
	edit_port_label->setDisabled(true);
	edit_port2_label->setDisabled(true);
	edit_port2_box->setDisabled(true);
	edit_porttype2_label->setDisabled(true);
	edit_porttype2_box->setDisabled(true);
	edit_device_label->setDisabled(true);
	edit_device_edit->setDisabled(true);
	edit_xpoint_button->setDisabled(true);
	edit_gpis_button->setDisabled(true);
	edit_gpos_button->setDisabled(true);
	edit_inputs_label->setDisabled(true);
	edit_inputs_box->setDisabled(true);
	edit_outputs_label->setDisabled(true);
	edit_outputs_box->setDisabled(true);
	edit_gpis_label->setDisabled(true);
	edit_gpis_box->setDisabled(true);
	edit_gpos_label->setDisabled(true);
	edit_gpos_box->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipaddress_edit->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_username_label->setDisabled(true);
	edit_username_edit->setDisabled(true);
	edit_password_label->setDisabled(true);
	edit_password_edit->setDisabled(true);
	edit_ipaddress2_label->setDisabled(true);
	edit_ipaddress2_edit->setDisabled(true);
	edit_ipport2_label->setDisabled(true);
	edit_ipport2_spin->setDisabled(true);
	edit_username2_label->setDisabled(true);
	edit_username2_edit->setDisabled(true);
	edit_password2_label->setDisabled(true);
	edit_password2_edit->setDisabled(true);
	edit_displays_label->setDisabled(true);
	edit_displays_box->setDisabled(true);
	edit_inputs_button->setDisabled(true);
	edit_outputs_button->setDisabled(true);
	edit_livewire_button->setDisabled(true);
	edit_vguestrelays_button->setDisabled(true);
	edit_vguestdisplays_button->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_start_cart2_edit->setDisabled(true);
	edit_stop_cart2_edit->setDisabled(true);
	edit_layer_box->setDisabled(true);
	edit_layer_label->setDisabled(true);
	break;
  }
  portTypeActivatedData(edit_porttype_box->currentItem());
  portType2ActivatedData(edit_porttype2_box->currentItem());
}


QSize EditMatrix::sizeHint() const
{
  return QSize(420,636);
} 


QSizePolicy EditMatrix::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void EditMatrix::paintEvent(QPaintEvent *e)
{
  QPainter *p=new QPainter(this);
  p->setPen(QColor(black));
  p->drawRect(10,84,sizeHint().width()-20,134);
  p->drawRect(10,231,sizeHint().width()-20,134);
  p->end();
}


void EditMatrix::portTypeActivatedData(int index)
{
  switch((RDMatrix::PortType)edit_porttype_box->currentItem()) {
      case RDMatrix::TtyPort:
	switch(edit_matrix->type()) {
	    case RDMatrix::LocalGpio:
	    case RDMatrix::LocalAudioAdapter:
	    case RDMatrix::LiveWire:
	    case RDMatrix::None:
	      edit_port_box->setDisabled(true);
	      edit_port_label->setDisabled(true);
	      break;

	    default:
	      edit_port_box->setEnabled(true);
	      edit_port_label->setEnabled(true);
	      break;
	}
	edit_ipaddress_edit->setDisabled(true);
	edit_ipaddress_label->setDisabled(true);
	edit_ipport_spin->setDisabled(true);
	edit_ipport_label->setDisabled(true);
	edit_start_cart_label->setDisabled(true);
	edit_start_cart_edit->setDisabled(true);
	edit_start_cart_button->setDisabled(true);
	edit_stop_cart_label->setDisabled(true);
	edit_stop_cart_edit->setDisabled(true);
	edit_stop_cart_button->setDisabled(true);
	break;
	
      case RDMatrix::TcpPort:
	edit_port_box->setDisabled(true);
	edit_port_label->setDisabled(true);
	edit_ipaddress_edit->setEnabled(true);
	edit_ipaddress_label->setEnabled(true);
	edit_ipport_spin->setEnabled(true);
	edit_ipport_label->setEnabled(true);
	edit_start_cart_label->setEnabled(true);
	edit_start_cart_edit->setEnabled(true);
	edit_start_cart_button->setEnabled(true);
	edit_stop_cart_label->setEnabled(true);
	edit_stop_cart_edit->setEnabled(true);
	edit_stop_cart_button->setEnabled(true);
	break;

    case RDMatrix::NoPort:
      break;
  }
}


void EditMatrix::portType2ActivatedData(int index)
{
  switch(edit_matrix->type()) {
    case RDMatrix::LogitekVguest:
    case RDMatrix::Quartz1:
      switch((RDMatrix::PortType)edit_porttype2_box->currentItem()) {
	case RDMatrix::TtyPort:
	  edit_port2_box->setEnabled(true);
	  edit_port2_label->setEnabled(true);
	  edit_ipaddress2_edit->setDisabled(true);
	  edit_ipaddress2_label->setDisabled(true);
	  edit_ipport2_spin->setDisabled(true);
	  edit_ipport2_label->setDisabled(true);
	  edit_username2_edit->setEnabled(true);
	  edit_username2_label->setEnabled(true);
	  edit_password2_edit->setEnabled(true);
	  edit_password2_label->setEnabled(true);
	  edit_start_cart2_label->setDisabled(true);
	  edit_start_cart2_edit->setDisabled(true);
	  edit_start_cart2_button->setDisabled(true);
	  edit_stop_cart2_label->setDisabled(true);
	  edit_stop_cart2_edit->setDisabled(true);
	  edit_stop_cart2_button->setDisabled(true);
	  break;
		  
	case RDMatrix::TcpPort:
	  edit_port2_box->setDisabled(true);
	  edit_port2_label->setDisabled(true);
	  edit_ipaddress2_edit->setEnabled(true);
	  edit_ipaddress2_label->setEnabled(true);
	  edit_ipport2_spin->setEnabled(true);
	  edit_ipport2_label->setEnabled(true);
	  edit_ipaddress2_edit->setEnabled(true);
	  edit_ipaddress2_label->setEnabled(true);
	  edit_ipport2_spin->setEnabled(true);
	  edit_ipport2_label->setEnabled(true);
	  edit_username2_edit->setEnabled(true);
	  edit_username2_label->setEnabled(true);
	  edit_password2_edit->setEnabled(true);
	  edit_password2_label->setEnabled(true);
	  edit_start_cart2_label->setEnabled(true);
	  edit_start_cart2_edit->setEnabled(true);
	  edit_start_cart2_button->setEnabled(true);
	  edit_stop_cart2_label->setEnabled(true);
	  edit_stop_cart2_edit->setEnabled(true);
	  edit_stop_cart2_button->setEnabled(true);
	  break;
		  
	case RDMatrix::NoPort:
	  edit_port2_box->setDisabled(true);
	  edit_port2_label->setDisabled(true);
	  edit_ipaddress2_edit->setDisabled(true);
	  edit_ipaddress2_label->setDisabled(true);
	  edit_ipport2_spin->setDisabled(true);
	  edit_ipport2_label->setDisabled(true);
	  edit_ipaddress2_edit->setDisabled(true);
	  edit_ipaddress2_label->setDisabled(true);
	  edit_ipport2_spin->setDisabled(true);
	  edit_ipport2_label->setDisabled(true);
	  edit_username2_edit->setDisabled(true);
	  edit_username2_label->setDisabled(true);
	  edit_password2_edit->setDisabled(true);
	  edit_password2_label->setDisabled(true);
	  edit_start_cart2_label->setDisabled(true);
	  edit_start_cart2_edit->setDisabled(true);
	  edit_start_cart2_button->setDisabled(true);
	  edit_stop_cart2_label->setDisabled(true);
	  edit_stop_cart2_edit->setDisabled(true);
	  edit_stop_cart2_button->setDisabled(true);
	  break;
      }
    default:
      break;
  }
}


void EditMatrix::inputsButtonData()
{
  if(!WriteMatrix()) {
    return;
  }
  ListEndpoints *ep=new ListEndpoints(edit_matrix,RDMatrix::Input,this);
  ep->exec();
  delete ep;
}


void EditMatrix::outputsButtonData()
{
  if(!WriteMatrix()) {
    return;
  }
  ListEndpoints *ep=new ListEndpoints(edit_matrix,RDMatrix::Output,this);
  ep->exec();
  delete ep;
}


void EditMatrix::xpointsButtonData()
{
}


void EditMatrix::gpisButtonData()
{
  if(!WriteMatrix()) {
    return;
  }
  ListGpis *ep=new ListGpis(edit_matrix,RDMatrix::GpioInput,this);
  ep->exec();
  delete ep;
}


void EditMatrix::gposButtonData()
{
  if(!WriteMatrix()) {
    return;
  }
  ListGpis *ep=new ListGpis(edit_matrix,RDMatrix::GpioOutput,this);
  ep->exec();
  delete ep;
}


void EditMatrix::gpisChangedData(int value)
{
  if(edit_matrix->type()==RDMatrix::LogitekVguest) {
    edit_gpos_box->setValue(value);
  }
  if(value>0) {
    edit_gpis_button->setEnabled(true);
  }
  else {
    edit_gpis_button->setDisabled(true);
  }
}


void EditMatrix::gposChangedData(int value)
{
  if(edit_matrix->type()==RDMatrix::LogitekVguest) {
    edit_gpis_box->setValue(value);
    if(value>0) {
      edit_gpis_button->setEnabled(true);
    }
    else {
      edit_gpis_button->setDisabled(true);
    }
  }
}


void EditMatrix::livewireButtonData()
{
  ListNodes *dialog=new ListNodes(edit_matrix,this,"dialog");
  dialog->exec();
  delete dialog;
}


void EditMatrix::vguestRelaysButtonData()
{
  ListVguestResources *dialog=
    new ListVguestResources(edit_matrix,RDMatrix::VguestTypeRelay,
			    edit_gpos_box->value(),this,"dialog");
  dialog->exec();
  delete dialog;
}


void EditMatrix::vguestDisplaysButtonData()
{
  ListVguestResources*dialog=
    new ListVguestResources(edit_matrix,RDMatrix::VguestTypeDisplay,
			    edit_displays_box->value(),this,"dialog");
  dialog->exec();
  delete dialog;
}


void EditMatrix::startCartData()
{
  int cartnum=edit_start_cart_edit->text().toUInt();
  if(admin_cart_dialog->exec(&cartnum,RDCart::Macro,NULL,0)==0) {
    if(cartnum>0) {
      edit_start_cart_edit->setText(QString().sprintf("%06u",cartnum));
    }
    else {
      edit_start_cart_edit->setText("");
    }
  }
}


void EditMatrix::stopCartData()
{
  int cartnum=edit_stop_cart_edit->text().toUInt();
  if(admin_cart_dialog->exec(&cartnum,RDCart::Macro,NULL,0)==0) {
    if(cartnum>0) {
      edit_stop_cart_edit->setText(QString().sprintf("%06u",cartnum));
    }
    else {
      edit_stop_cart_edit->setText("");
    }
  }
}


void EditMatrix::startCart2Data()
{
  int cartnum=edit_start_cart2_edit->text().toUInt();
  if(admin_cart_dialog->exec(&cartnum,RDCart::Macro,NULL,0)==0) {
    if(cartnum>0) {
      edit_start_cart2_edit->setText(QString().sprintf("%06u",cartnum));
    }
    else {
      edit_start_cart2_edit->setText("");
    }
  }
}


void EditMatrix::stopCart2Data()
{
  int cartnum=edit_stop_cart2_edit->text().toUInt();
  if(admin_cart_dialog->exec(&cartnum,RDCart::Macro,NULL,0)==0) {
    if(cartnum>0) {
      edit_stop_cart2_edit->setText(QString().sprintf("%06u",cartnum));
    }
    else {
      edit_stop_cart2_edit->setText("");
    }
  }
}


void EditMatrix::okData()
{
  if(!WriteMatrix()) {
    return;
  }
  done(0);
}


void EditMatrix::cancelData()
{
  done(1);
}


bool EditMatrix::WriteMatrix()
{
  QHostAddress addr;
  QHostAddress addr2;

  //
  // Ensure Sane Values
  //
  switch((RDMatrix::PortType)edit_porttype_box->currentItem()) {
    case RDMatrix::TcpPort:
      if(!addr.setAddress(edit_ipaddress_edit->text())) {
	QMessageBox::warning(this,tr("Invalid Address"),
			     tr("The primary IP address is invalid!"));
	return false;
      }
      break;
      
    default:
      break;
  }
  switch(edit_matrix->type()) {
      case RDMatrix::LogitekVguest:
      case RDMatrix::Quartz1:
	  switch((RDMatrix::PortType)edit_porttype2_box->currentItem()) {
	      case RDMatrix::TcpPort:
		  if(!addr2.setAddress(edit_ipaddress2_edit->text())) {
		      QMessageBox::warning(this,tr("Invalid Address"),
					   tr("The backup IP address is invalid!"));
		      return false;
		  }
		  if(edit_porttype_box->currentItem()==RDMatrix::TcpPort) {
		      if((addr==addr2)&&
			 (edit_ipport_spin->value()==edit_ipport2_spin->value())) {
			  QMessageBox::warning(this,tr("Duplicate Connections"),
					       tr("The primary and backup connections must be different!"));
			  return false;
		      }
		  }
		  break;
		  
	      case RDMatrix::TtyPort:
		  if(edit_porttype_box->currentItem()==RDMatrix::TtyPort) {
		      if(edit_port_box->currentItem()==edit_port2_box->currentItem()) {
			  QMessageBox::warning(this,tr("Duplicate Connections"),
					       tr("The primary and backup connections must be different!"));
			  return false;
		      }
		  }
		  break;
		  
	      case RDMatrix::NoPort:
		  break;
	  }
	  break;

      default:
	  break;
  }

  switch((RDMatrix::PortType)edit_porttype_box->currentItem()) {
    case RDMatrix::TtyPort:
      edit_matrix->setPortType(RDMatrix::Primary,RDMatrix::TtyPort);
      edit_matrix->setPort(RDMatrix::Primary,edit_port_box->currentItem());
      edit_matrix->setIpAddress(RDMatrix::Primary,QHostAddress());
      edit_matrix->setIpPort(RDMatrix::Primary,0);
      break;
      
    case RDMatrix::TcpPort:
      edit_matrix->setPortType(RDMatrix::Primary,RDMatrix::TcpPort);
      edit_matrix->setPort(RDMatrix::Primary,-1);
      edit_matrix->setIpAddress(RDMatrix::Primary,addr);
      edit_matrix->setIpPort(RDMatrix::Primary,edit_ipport_spin->value());
      break;
  
    case RDMatrix::NoPort:
      break;
}
  switch((RDMatrix::PortType)edit_porttype2_box->currentItem()) {
    case RDMatrix::TtyPort:
      edit_matrix->setPortType(RDMatrix::Backup,RDMatrix::TtyPort);
      edit_matrix->setPort(RDMatrix::Backup,edit_port2_box->currentItem());
      edit_matrix->setIpAddress(RDMatrix::Backup,QHostAddress());
      edit_matrix->setIpPort(RDMatrix::Backup,0);
      break;
      
    case RDMatrix::TcpPort:
      edit_matrix->setPortType(RDMatrix::Backup,RDMatrix::TcpPort);
      edit_matrix->setPort(RDMatrix::Backup,-1);
      edit_matrix->setIpAddress(RDMatrix::Backup,addr2);
      edit_matrix->setIpPort(RDMatrix::Backup,edit_ipport2_spin->value());
      break;
      
    case RDMatrix::NoPort:
      edit_matrix->setPortType(RDMatrix::Backup,RDMatrix::NoPort);
      break;
  }
  if(edit_layer_box->currentItem()==0) {
    edit_matrix->setLayer('V');
  }
  else {
    edit_matrix->setLayer('@'+edit_layer_box->currentItem());
  }
  
  //
  // Update GPIO Tables
  //
  WriteGpioTable(RDMatrix::GpioInput);
  WriteGpioTable(RDMatrix::GpioOutput);

  edit_matrix->setName(edit_name_edit->text());
  edit_matrix->setCard(edit_card_box->value());
  edit_matrix->setInputs(edit_inputs_box->value());
  edit_matrix->setOutputs(edit_outputs_box->value());
  edit_matrix->setGpioDevice(edit_device_edit->text());
  edit_matrix->setGpis(edit_gpis_box->value());
  edit_matrix->setGpos(edit_gpos_box->value());
  edit_matrix->setUsername(RDMatrix::Primary,edit_username_edit->text());
  edit_matrix->setPassword(RDMatrix::Primary,edit_password_edit->text());
  edit_matrix->setUsername(RDMatrix::Backup,edit_username2_edit->text());
  edit_matrix->setPassword(RDMatrix::Backup,edit_password2_edit->text());
  edit_matrix->setDisplays(edit_displays_box->value());
  edit_matrix->
    setStartCart(RDMatrix::Primary,edit_start_cart_edit->text().toUInt());
  edit_matrix->
    setStopCart(RDMatrix::Primary,edit_stop_cart_edit->text().toUInt());
  edit_matrix->
    setStartCart(RDMatrix::Backup,edit_start_cart2_edit->text().toUInt());
  edit_matrix->
    setStopCart(RDMatrix::Backup,edit_stop_cart2_edit->text().toUInt());

  return true;
}


void EditMatrix::WriteGpioTable(RDMatrix::GpioType type)
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  QString tablename;
  int line_quan=0;
  switch(type) {
    case RDMatrix::GpioInput:
      tablename="GPIS";
      line_quan=edit_gpis_box->value();
      break;

    case RDMatrix::GpioOutput:
      tablename="GPOS";
      line_quan=edit_gpos_box->value();
      break;
  }

  switch(edit_matrix->type()) {
    case RDMatrix::LiveWire:
      // LiveWire manages the GPIO tables dynamically
      break;

    default:
      //
      // Create New Entries
      for(int i=0;i<line_quan;i++) {
	sql=QString().sprintf("select ID from %s where \
                               (STATION_NAME=\"%s\")&&\
                               (MATRIX=%d)&&\
                               (NUMBER=%d)",
			      (const char *)tablename,
			      (const char *)edit_stationname,
			      edit_matrix_number,
			      i+1);
	q=new RDSqlQuery(sql);
	if(!q->first()) {
	  sql=QString().sprintf("insert into %s set \
                                 STATION_NAME=\"%s\",\
                                 MATRIX=%d,\
                                 NUMBER=%d,\
                                 MACRO_CART=0",
				(const char *)tablename,
				(const char *)edit_stationname,
				edit_matrix_number,
				i+1);
	  q1=new RDSqlQuery(sql);
	  delete q1;
	}
	delete q;
      }

      //
      // Purge Stale Entries
      //
      sql=QString().sprintf("delete from %s where \
                             (STATION_NAME=\"%s\")&&\
                             (MATRIX=%d)&&\
                             (NUMBER>%d)",
			    (const char *)tablename,
			    (const char *)edit_stationname,
			    edit_matrix_number,
			    line_quan);
      q=new RDSqlQuery(sql);
      delete q;
      break;
  }
}
