// edit_macro.h
//
// Edit a Rivendell Macro
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_macro.h,v 1.6 2007/02/14 21:55:07 fredg Exp $
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

#ifndef EDIT_MACRO_H
#define EDIT_MACRO_H

#include <qdialog.h>
#include <qlineedit.h>

#include <rdcart.h>
#include <rdmacro.h>


class EditMacro : public QDialog
{
  Q_OBJECT
 public:
  EditMacro(RDMacro *cmd,QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void okData();
  void cancelData();

 protected:
  void closeEvent(QCloseEvent *e);

 private:
  QLineEdit *edit_macro_edit;
  RDMacro *edit_macro;
};


#endif

