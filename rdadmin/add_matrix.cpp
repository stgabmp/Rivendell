// add_matrix.cpp
//
// Add a Rivendell Matrix
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: add_matrix.cpp,v 1.25.2.1.2.2 2010/05/08 23:01:57 cvs Exp $
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

#include <rd.h>
#include <rdmatrix.h>
#include <rddb.h>

#include <edit_user.h>
#include <add_matrix.h>
#include <rdpasswd.h>


AddMatrix::AddMatrix(QString station,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  add_station=station;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  setCaption(tr("Add Switcher"));

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);

  //
  // Matrix Number
  //
  add_matrix_box=new QSpinBox(this,"add_matrix_box");
  add_matrix_box->setGeometry(165,11,30,19);
  add_matrix_box->setRange(0,MAX_MATRICES-1);
  QLabel *label=new QLabel(add_matrix_box,tr("&New Matrix Number:"),this,
			   "matrix_label");
  label->setGeometry(10,11,150,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Matrix Type
  //
  add_type_box=new QComboBox(this,"add_type_box");
  add_type_box->setGeometry(165,36,200,19);
  add_type_box->insertItem(tr("Local GPIO"));
  add_type_box->insertItem(tr("Generic GPO"));
  add_type_box->insertItem(tr("Generic Serial"));
  add_type_box->insertItem("SAS 32000");
  add_type_box->insertItem("SAS 64000");
  add_type_box->insertItem("Wegener Unity 4000");
  add_type_box->insertItem("BroadcastTools SS8.2");
  add_type_box->insertItem("BroadcastTools 10x1");
  add_type_box->insertItem("SAS 64000-GPI");
  add_type_box->insertItem("BroadcastTools 16x1");
  add_type_box->insertItem("BroadcastTools 8x2");
  add_type_box->insertItem("BroadcastTools ACS82");
  add_type_box->insertItem("SAS User Serial Interface");
  add_type_box->insertItem("BroadcastTools 16x2");
  add_type_box->insertItem("BroadcastTools SS12.4");
  add_type_box->insertItem(tr("Local Audio Adapter"));
  add_type_box->insertItem(tr("Logitek vGuest"));
  add_type_box->insertItem(tr("BroadcastTools SS16.4"));
  add_type_box->insertItem(tr("StarGuide III"));
  add_type_box->insertItem(tr("BroadcastTools SS4.2"));
  add_type_box->insertItem(tr("Axia LiveWire"));
  add_type_box->insertItem(tr("Quartz Type 1"));
  add_type_box->insertItem(tr("BroadcastTools SS4.4"));
  add_type_box->insertItem(tr("BroadcastTools SRC-8 III"));
  add_type_box->insertItem(tr("BroadcastTools SRC-16"));
  label=new QLabel(add_type_box,tr("&Switcher Type:"),this,
		   "matrix_label");
  label->setGeometry(10,36,150,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  //  Ok Button
  //
  QPushButton *ok_button=new QPushButton(this,"ok_button");
  ok_button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  ok_button->setDefault(true);
  ok_button->setFont(font);
  ok_button->setText(tr("&OK"));
  connect(ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  QPushButton *cancel_button=new QPushButton(this,"cancel_button");
  cancel_button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			     80,50);
  cancel_button->setFont(font);
  cancel_button->setText(tr("&Cancel"));
  connect(cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Assign Next Free Matrix
  //
  int n=GetNextMatrix();
  if(n>=0) {
    add_matrix_box->setValue(n);
  }
}


QSize AddMatrix::sizeHint() const
{
  return QSize(400,130);
} 


QSizePolicy AddMatrix::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void AddMatrix::okData()
{
  int inputs;
  int outputs;
  int gpis;
  int gpos;

  QString sql=QString().sprintf("select MATRIX from MATRICES \
                                 where STATION_NAME=\"%s\" && MATRIX=%d",
				(const char *)add_station,
				add_matrix_box->value());
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    delete q;
    QMessageBox::warning(this,tr("Invalid Matrix"),
			 tr("Matrix already exists!"));
    return;
  }
  delete q;
  switch((RDMatrix::Type)add_type_box->currentItem()) {
      case RDMatrix::BtSs82:
	inputs=8;
	outputs=2;
	gpis=16;
	gpos=8;
	break;

      case RDMatrix::Bt10x1:
	inputs=10;
	outputs=1;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::Bt16x1:
	inputs=16;
	outputs=1;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::Bt8x2:
	inputs=8;
	outputs=2;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::BtAcs82:
	inputs=8;
	outputs=2;
	gpis=16;
	gpos=16;
	break;

      case RDMatrix::SasUsi:
	inputs=0;
	outputs=0;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::Bt16x2:
	inputs=16;
	outputs=2;
	gpis=16;
	gpos=16;
	break;

      case RDMatrix::BtSs124:
	inputs=12;
	outputs=4;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::LocalAudioAdapter:
	inputs=0;
	outputs=0;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::LogitekVguest:
	inputs=0;
	outputs=0;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::BtSs164:
	inputs=16;
	outputs=4;
	gpis=24;
	gpos=24;
	break;

      case RDMatrix::StarGuideIII:
	inputs=0;
	outputs=6;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::BtSs42:
	inputs=4;
	outputs=2;
	gpis=16;
	gpos=8;
	break;

      case RDMatrix::LiveWire:
	inputs=0;
	outputs=0;
	gpis=0;
	gpos=0;
	break;

      case RDMatrix::BtSs44:
	inputs=4;
	outputs=4;
	gpis=16;
	gpos=8;
	break;

      case RDMatrix::BtSrc8III:
	inputs=0;
	outputs=0;
	gpis=8;
	gpos=8;
	break;

      case RDMatrix::BtSrc16:
	inputs=0;
	outputs=0;
	gpis=16;
	gpos=16;
	break;

      default:
	inputs=0;
	outputs=0;
	gpis=0;
	gpos=0;
	break;
  }
  sql=QString().sprintf("insert into MATRICES set \
                         STATION_NAME=\"%s\",\
                         NAME=\"New Switcher\",\
                         MATRIX=%d,\
                         PORT=0,\
                         GPIO_DEVICE=\"%s\",\
                         TYPE=%d,\
                         INPUTS=%d,\
                         OUTPUTS=%d,\
                         GPIS=%d,\
                         GPOS=%d",
			(const char *)add_station,
			add_matrix_box->value(),
			RD_DEFAULT_GPIO_DEVICE,
			add_type_box->currentItem(),
			inputs,
			outputs,
			gpis,
			gpos);
  q=new RDSqlQuery(sql);
  delete q;
  done(add_matrix_box->value());
}


void AddMatrix::cancelData()
{
  done(-1);
}


int AddMatrix::GetNextMatrix()
{
  int n=0;

  QString sql=QString().sprintf("select MATRIX from MATRICES\
                                 where STATION_NAME=\"%s\" order by MATRIX",
				(const char *)add_station);
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    if(n!=q->value(0).toInt()) {
      delete q;
      return n;
    }
    n++;
  }
  delete q;
  if(n<MAX_MATRICES) {
    return n;
  }
  return -1;
}
