// edit_marker.h
//
// Edit a Rivendell Log Entry
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_marker.h,v 1.8.2.1 2010/02/08 23:50:14 cvs Exp $
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

#ifndef EDIT_MARKER_H
#define EDIT_MARKER_H

#include <qdialog.h>
#include <qdatetimeedit.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include <rdlog_line.h>
#include <rdtimeedit.h>

class EditMarker : public QDialog
{
  Q_OBJECT
 public:
  EditMarker(RDLogLine *,QWidget *parent=0,const char *name=0);
  ~EditMarker();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  
 private slots:
  void timeChangedData(const QTime &);
  void timeToggledData(bool state);
  void graceClickedData(int id);
  void selectTimeData(int);
  void okData();
  void cancelData();

 protected:
  void closeEvent(QCloseEvent *e);

 private:
  RDLogLine *edit_logline;
  RDTimeEdit *edit_time_edit;
  QLabel *edit_time_label;
  QCheckBox *edit_timetype_box;
  QLabel *edit_timetype_label;
  QCheckBox *edit_time_box;
  QComboBox *edit_transtype_box;
  QLineEdit *edit_comment_edit;
  QLineEdit *edit_label_edit;
  QButtonGroup *edit_grace_group;
  QFont normal_font;
  QTimeEdit *edit_grace_box;
};


#endif

