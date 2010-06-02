// add_encoder.h
//
// Add a Rivendell Encoder
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: add_encoder.h,v 1.1 2008/09/18 19:02:10 fredg Exp $
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

#ifndef ADD_ENCODER_H
#define ADD_ENCODER_H

#include <qdialog.h>
#include <qcheckbox.h>
#include <qlineedit.h>


class AddEncoder : public QDialog
{
  Q_OBJECT
 public:
  AddEncoder(QString *encname,const QString &stationname,
	     QWidget *parent=0,const char *name=0);
  ~AddEncoder();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void okData();
  void cancelData();

 private:
  QLineEdit *encoder_name_edit;
  QString *encoder_name;
  QString encoder_stationname;
};


#endif  // ADD_ENCODER_H
