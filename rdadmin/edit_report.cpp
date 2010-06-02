// edit_report.cpp
//
// Edit a Rivendell Report
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_report.cpp,v 1.19 2007/10/04 20:53:29 dmills Exp $
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
#include <rdtextvalidator.h>

#include <edit_report.h>
#include <test_import.h>
#include <autofill_carts.h>
#include <edit_svc_perms.h>


EditReport::EditReport(QString rptname,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  QString sql;
  RDSqlQuery *q;
  QString str;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  edit_report=new RDReport(rptname);
  str=QString(tr("Edit Report"));
  setCaption(QString().sprintf("%s - %s",(const char *)str,
			       (const char *)rptname));

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont check_font=QFont("Helvetica",12,QFont::Normal);
  check_font.setPixelSize(12);
  QFont section_font=QFont("Helvetica",14,QFont::Bold);
  section_font.setPixelSize(14);

  //
  // Text Validator
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator");

  //
  // Report Description
  //
  edit_description_edit=new QLineEdit(this,"edit_description_edit");
  edit_description_edit->setGeometry(170,10,sizeHint().width()-180,19);
  edit_description_edit->setMaxLength(64);
  QLabel *label=new QLabel(edit_description_edit,tr("&Report Description:"),
			   this,"edit_description_label");
  label->setGeometry(10,10,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Export Filter Type
  //
  edit_filter_box=new QComboBox(this,"edit_filter_box");
  edit_filter_box->setGeometry(170,31,sizeHint().width()-180,19);
  for(int i=0;i<(int)RDReport::LastFilter;i++) {
    edit_filter_box->
      insertItem(RDReport::filterText((RDReport::ExportFilter)i));
  }
  label=new QLabel(edit_description_edit,tr("Export &Filter:"),this,
		   "edit_filter_label");
  label->setGeometry(10,31,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Station ID
  //
  edit_stationid_edit=new QLineEdit(this,"edit_stationid_edit");
  edit_stationid_edit->setGeometry(170,52,180,19);
  edit_stationid_edit->setMaxLength(16);
  edit_stationid_edit->setValidator(validator);
  label=new QLabel(edit_stationid_edit,
		   tr("Station ID:"),this,
		   "edit_stationid_label");
  label->setGeometry(10,52,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Cart Number Parameters
  //
  label=new QLabel(tr("Cart Number Parameters:"),this,"edit_cartdigits_label");
  label->setGeometry(10,73,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  
  edit_cartzeros_box=new QCheckBox(this,"edit_cartzeros_box");
  edit_cartzeros_box->setGeometry(170,75,15,15);
  connect(edit_cartzeros_box,SIGNAL(toggled(bool)),
	  this,SLOT(leadingZerosToggled(bool)));
  label=new QLabel(edit_cartzeros_box,
		   tr("Use Leading Zeros"),this,
		   "edit_cartzeros_label");
  label->setGeometry(187,73,120,19);
  label->setFont(check_font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  edit_cartdigits_spin=new QSpinBox(this,"edit_cartdigits_spin");
  edit_cartdigits_spin->setGeometry(350,73,40,19);
  edit_cartdigits_spin->setRange(1,6);
  edit_cartdigits_label=new QLabel(edit_cartdigits_spin,
				   tr("Digits:"),this,
				   "edit_cartdigits_label");
  edit_cartdigits_label->setGeometry(300,73,45,19);
  edit_cartdigits_label->setFont(check_font);
  edit_cartdigits_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Station Type
  //
  edit_stationtype_box=new QComboBox(this,"edit_stationtype_box");
  edit_stationtype_box->setGeometry(170,94,70,19);
  for(int i=0;i<RDReport::TypeLast;i++) {
    edit_stationtype_box->
      insertItem(RDReport::stationTypeText((RDReport::StationType)i));
  }
  label=new QLabel(edit_stationtype_box,tr("Station Type:"),
		   this,"edit_stationtype_label");
  label->setGeometry(10,94,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Lines Per Page
  //
  edit_linesperpage_spin=new QSpinBox(this,"edit_linesperpage_spin");
  edit_linesperpage_spin->setGeometry(360,94,50,19);
  edit_linesperpage_spin->setRange(10,200);
  label=new QLabel(edit_linesperpage_spin,tr("Lines per Page:"),
		   this,"edit_stationid_label");
  label->setGeometry(255,94,100,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  edit_linesperpage_spin->hide();
  label->hide();

  //
  // Service Name
  //
  edit_servicename_edit=new QLineEdit(this,"edit_servicename_edit");
  edit_servicename_edit->setGeometry(170,115,sizeHint().width()-180,19);
  edit_servicename_edit->setMaxLength(64);
  label=new QLabel(edit_servicename_edit,tr("Ser&vice Name:"),
		   this,"edit_servicename_label");
  label->setGeometry(10,115,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Station Format
  //
  edit_stationformat_edit=new QLineEdit(this,"edit_stationformat_edit");
  edit_stationformat_edit->setGeometry(170,136,sizeHint().width()-180,19);
  edit_stationformat_edit->setMaxLength(64);
  label=new QLabel(edit_stationformat_edit,tr("Station &Format:"),
		   this,"edit_stationformat_label");
  label->setGeometry(10,136,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Linux Export Path
  //
  edit_path_edit=new QLineEdit(this,"edit_path_edit");
  edit_path_edit->setGeometry(170,157,sizeHint().width()-180,19);
  edit_path_edit->setMaxLength(255);
  edit_path_edit->setValidator(validator);
  label=new QLabel(edit_path_edit,
		   tr("Linux Export Path:"),this,
		   "edit_path_label");
  label->setGeometry(10,157,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Windows Export Path
  //
  edit_winpath_edit=new QLineEdit(this,"edit_winpath_edit");
  edit_winpath_edit->setGeometry(170,178,sizeHint().width()-180,19);
  edit_winpath_edit->setMaxLength(255);
  label=new QLabel(edit_winpath_edit,
		   tr("Windows Export Path:"),this,
		   "edit_winpath_label");
  label->setGeometry(10,178,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Type Selectors
  //
  label=new QLabel(tr("Export Event Types:"),this,"export_types_label");
  label->setGeometry(10,199,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  edit_traffic_box=new QCheckBox(this,"edit_traffic_box");
  edit_traffic_box->setGeometry(170,200,15,15);
  edit_traffic_label=new QLabel(tr("Traffic"),this,"export_traffic_label");
  edit_traffic_label->setGeometry(187,199,80,19);
  edit_traffic_label->setFont(check_font);
  edit_traffic_label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
  
  edit_music_box=new QCheckBox(this,"edit_music_box");
  edit_music_box->setGeometry(270,201,15,15);
  edit_music_label=new QLabel(tr("Music"),this,"export_music_label");
  edit_music_label->setGeometry(287,199,80,19);
  edit_music_label->setFont(check_font);
  edit_music_label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  edit_generic_box=new QCheckBox(this,"edit_generic_box");
  edit_generic_box->setGeometry(370,201,15,15);
  label=new QLabel(tr("All"),this,"export_generic_label");
  label->setGeometry(387,199,80,19);
  label->setFont(check_font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
  connect(edit_generic_box,SIGNAL(toggled(bool)),
	  this,SLOT(genericEventsToggledData(bool)));

  //
  // Force Event Source Selectors
  //
  label=new QLabel(tr("Export Events From:"),
		   this,"export_forcetypes_label");
  label->setGeometry(10,220,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  edit_forcetraffic_box=new QCheckBox(this,"edit_forcetraffic_box");
  edit_forcetraffic_box->setGeometry(170,222,15,15);
  label=new QLabel(tr("Traffic Log"),this,"export_forcetraffic_label");
  label->setGeometry(187,220,80,19);
  label->setFont(check_font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
  
  edit_forcemusic_box=new QCheckBox(this,"edit_forcemusic_box");
  edit_forcemusic_box->setGeometry(270,222,15,15);
  label=new QLabel(tr("Music Log"),this,"export_forcemusic_label");
  label->setGeometry(287,220,80,19);
  label->setFont(check_font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Include Only On Air Events
  //
  edit_onairflag_box=new QComboBox(this,"edit_onairflag_box");
  edit_onairflag_box->setGeometry(170,241,60,19);
  edit_onairflag_box->insertItem(tr("No"));
  edit_onairflag_box->insertItem(tr("Yes"));
  label=new QLabel(edit_onairflag_box,tr("Include Only OnAir Events:"),
		   this,"edit_onairflag_label");
  label->setGeometry(10,241,155,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Service Selector
  //
  edit_service_sel=new RDListSelector(this,"edit_service_sel");
  edit_service_sel->
    setGeometry((sizeHint().width()-edit_service_sel->sizeHint().width())/2,
		268,edit_service_sel->sizeHint().width(),
		edit_service_sel->sizeHint().height());
  edit_service_sel->sourceSetLabel(tr("Available Services"));
  edit_service_sel->destSetLabel(tr("Source Services"));
  
  //
  // Station Selector
  //
  edit_station_sel=new RDListSelector(this,"edit_station_sel");
  edit_station_sel->
    setGeometry((sizeHint().width()-edit_station_sel->sizeHint().width())/2,
		378,edit_station_sel->sizeHint().width(),
		edit_station_sel->sizeHint().height());
  edit_station_sel->sourceSetLabel(tr("Available Hosts"));
  edit_station_sel->destSetLabel(tr("Source Hosts"));

  //
  //  Ok Button
  //
  QPushButton *button=new QPushButton(this,"ok_button");
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
  // Populate Fields
  //
  edit_description_edit->setText(edit_report->description());
  edit_filter_box->setCurrentItem((int)edit_report->filter());
  edit_stationid_edit->setText(edit_report->stationId());
  edit_cartzeros_box->setChecked(edit_report->useLeadingZeros());
  leadingZerosToggled(edit_cartzeros_box->isChecked());
  edit_cartdigits_spin->setValue(edit_report->cartDigits());
  edit_stationtype_box->setCurrentItem((int)edit_report->stationType());
  edit_servicename_edit->setText(edit_report->serviceName());
  edit_stationformat_edit->setText(edit_report->stationFormat());
  edit_linesperpage_spin->setValue(edit_report->linesPerPage());
  edit_path_edit->setText(edit_report->exportPath(RDReport::Linux));
  edit_winpath_edit->setText(edit_report->exportPath(RDReport::Windows));
  edit_traffic_box->
    setChecked(edit_report->exportTypeEnabled(RDReport::Traffic));
  edit_music_box->
    setChecked(edit_report->exportTypeEnabled(RDReport::Music));
  edit_generic_box->
    setChecked(edit_report->exportTypeEnabled(RDReport::Generic));
  edit_forcetraffic_box->
    setChecked(edit_report->exportTypeForced(RDReport::Traffic));
  edit_forcemusic_box->
    setChecked(edit_report->exportTypeForced(RDReport::Music));
  genericEventsToggledData(edit_generic_box->isChecked());
  if(edit_report->filterOnairFlag()) {
    edit_onairflag_box->setCurrentItem(1);
  }
  else {
    edit_onairflag_box->setCurrentItem(0);
  }

  sql=QString().sprintf("select SERVICE_NAME from REPORT_SERVICES \
                         where REPORT_NAME=\"%s\"",
			(const char *)edit_report->name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    edit_service_sel->destInsertItem(q->value(0).toString());
  }
  delete q;

  sql=QString().sprintf("select NAME from SERVICES");
  q=new RDSqlQuery(sql);
  while(q->next()) {
    if(edit_service_sel->destFindItem(q->value(0).toString())==0) {
      edit_service_sel->sourceInsertItem(q->value(0).toString());
    }
  }
  delete q;

  sql=QString().sprintf("select STATION_NAME from REPORT_STATIONS \
                         where REPORT_NAME=\"%s\"",
			(const char *)edit_report->name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    edit_station_sel->destInsertItem(q->value(0).toString());
  }
  delete q;

  sql=QString().sprintf("select NAME from STATIONS");
  q=new RDSqlQuery(sql);
  while(q->next()) {
    if(edit_station_sel->destFindItem(q->value(0).toString())==0) {
      edit_station_sel->sourceInsertItem(q->value(0).toString());
    }
  }
  delete q;
}


EditReport::~EditReport()
{
}


QSize EditReport::sizeHint() const
{
  return QSize(500,559);
} 


QSizePolicy EditReport::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void EditReport::leadingZerosToggled(bool state)
{
  edit_cartdigits_label->setEnabled(state);
  edit_cartdigits_spin->setEnabled(state);
}


void EditReport::genericEventsToggledData(bool state)
{
  edit_traffic_label->setDisabled(state);
  edit_traffic_box->setDisabled(state);
  edit_music_label->setDisabled(state);
  edit_music_box->setDisabled(state);
}


void EditReport::okData()
{
  QString sql;
  RDSqlQuery *q;

  edit_report->setDescription(edit_description_edit->text());
  edit_report->
    setFilter((RDReport::ExportFilter)edit_filter_box->currentItem());
  edit_report->setStationId(edit_stationid_edit->text());
  edit_report->setCartDigits(edit_cartdigits_spin->value());
  edit_report->setUseLeadingZeros(edit_cartzeros_box->isChecked());
  edit_report->setLinesPerPage(edit_linesperpage_spin->value());
  edit_report->
    setStationType((RDReport::StationType)edit_stationtype_box->currentItem());
  edit_report->setServiceName(edit_servicename_edit->text());
  edit_report->setStationFormat(edit_stationformat_edit->text());
  edit_report->setExportPath(RDReport::Linux,edit_path_edit->text());
  edit_report->setExportPath(RDReport::Windows,edit_winpath_edit->text());
  edit_report->
    setExportTypeEnabled(RDReport::Traffic,edit_traffic_box->isChecked());
  edit_report->
    setExportTypeEnabled(RDReport::Music,edit_music_box->isChecked());
  edit_report->
    setExportTypeForced(RDReport::Traffic,edit_forcetraffic_box->isChecked());
  edit_report->
    setExportTypeForced(RDReport::Music,edit_forcemusic_box->isChecked());
  edit_report->
    setExportTypeEnabled(RDReport::Generic,edit_generic_box->isChecked());
  edit_report->setFilterOnairFlag(edit_onairflag_box->currentItem()==1);

  //
  // Add New Services
  //
  for(unsigned i=0;i<edit_service_sel->destCount();i++) {
    sql=QString().sprintf("select SERVICE_NAME from REPORT_SERVICES \
where REPORT_NAME=\"%s\" && SERVICE_NAME=\"%s\"",
			  (const char *)edit_report->name(),
			  (const char *)edit_service_sel->destText(i));
    q=new RDSqlQuery(sql);
    if(q->size()==0) {
      delete q;
      sql=QString().
	sprintf("insert into REPORT_SERVICES (REPORT_NAME,SERVICE_NAME) \
                 values (\"%s\",\"%s\")",
			    (const char *)edit_report->name(),
			    (const char *)edit_service_sel->destText(i));
      q=new RDSqlQuery(sql);
    }
    delete q;
  }

  //
  // Delete Old Services
  //
  sql=QString().sprintf("delete from REPORT_SERVICES where REPORT_NAME=\"%s\"",
			(const char *)edit_report->name());
  for(unsigned i=0;i<edit_service_sel->destCount();i++) {
    sql+=QString().sprintf(" && SERVICE_NAME<>\"%s\"",
			   (const char *)edit_service_sel->destText(i));
  }
  q=new RDSqlQuery(sql);
  delete q;



  //
  // Add New Stations
  //
  for(unsigned i=0;i<edit_station_sel->destCount();i++) {
    sql=QString().sprintf("select STATION_NAME from REPORT_STATIONS \
where REPORT_NAME=\"%s\" && STATION_NAME=\"%s\"",
			  (const char *)edit_report->name(),
			  (const char *)edit_station_sel->destText(i));
    q=new RDSqlQuery(sql);
    if(q->size()==0) {
      delete q;
      sql=QString().
	sprintf("insert into REPORT_STATIONS (REPORT_NAME,STATION_NAME) \
                 values (\"%s\",\"%s\")",
			    (const char *)edit_report->name(),
			    (const char *)edit_station_sel->destText(i));
      q=new RDSqlQuery(sql);
    }
    delete q;
  }

  //
  // Delete Old Stations
  //
  sql=QString().sprintf("delete from REPORT_STATIONS where REPORT_NAME=\"%s\"",
			(const char *)edit_report->name());
  for(unsigned i=0;i<edit_station_sel->destCount();i++) {
    sql+=QString().sprintf(" && STATION_NAME<>\"%s\"",
			   (const char *)edit_station_sel->destText(i));
  }
  q=new RDSqlQuery(sql);
  delete q;


  done(0);
}


void EditReport::cancelData()
{
  done(-1);
}
