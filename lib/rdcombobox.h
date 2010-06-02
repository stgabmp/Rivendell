// rdcombobox.h
//
// A Combo Box widget for Rivendell.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcombobox.h,v 1.4 2007/07/09 15:47:09 fredg Exp $
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

#ifndef RDCOMBOBOX_H
#define RDCOMBOBOX_H

#include <qcombobox.h>


class RDComboBox : public QComboBox
{
  Q_OBJECT
 public:
  RDComboBox(QWidget *parent=0,const char *name=0);
  void insertItem(const QString &str,bool unique=false);
  void setSetupMode(bool state);

 signals:
  void setupClicked();

 protected:
  void mousePressEvent(QMouseEvent *e);

 private:
  bool IsItemUnique(const QString &str);
  bool combo_setup_mode;
};


#endif  // RDCOMBOBOX_H
