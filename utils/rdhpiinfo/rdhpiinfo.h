// rdhpiinfo.h
//
// A Qt-based application for display info about ASI cards.
//
//   (C) Copyright 2002-2005 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rdhpiinfo.h,v 1.2 2008/03/28 20:00:09 fredg Exp $
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


#ifndef RDHPIINFO_H
#define RDHPIINFO_H

#include <qwidget.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <asihpi/hpi.h>

#define RDHPIINFO_USAGE "\n"


class MainWidget : public QWidget
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  ~MainWidget();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void nameActivatedData(int id);
  void changeModeData();

 private:
  void LoadAdapters();
  QLabel *info_name_label;
  QComboBox *info_name_box;
  QLabel *info_serial_label;
  QLabel *info_istreams_label;
  QLabel *info_ostreams_label;
  QLabel *info_dsp_label;
  QLabel *info_adapter_label;
  QLabel *info_mode_label;
  QPushButton *info_changemode_button;
  HW32 hpi_version;
  QString hpi_name[HPI_MAX_ADAPTERS];
  int name_map[HPI_MAX_ADAPTERS];
  HW16 hpi_ostreams[HPI_MAX_ADAPTERS];
  HW16 hpi_istreams[HPI_MAX_ADAPTERS];
  HW16 hpi_card_version[HPI_MAX_ADAPTERS];
  HW32 hpi_serial[HPI_MAX_ADAPTERS];
  HW16 hpi_type[HPI_MAX_ADAPTERS];
  HW32 hpi_mode[HPI_MAX_ADAPTERS];
};


#endif  // RDHPIINFO_H
