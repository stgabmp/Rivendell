// rdlibrary.h
//
// The Production Utility for Rivendell.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdlibrary.h,v 1.43.2.2 2010/01/31 22:24:55 cvs Exp $
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


#ifndef RDLIBRARY_H
#define RDLIBRARY_H

#include <qdialog.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qsqldatabase.h>
#include <rdlistview.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qprogressdialog.h>
#include <qtimer.h>

#include <rdinstancelock.h>
#include <rdstation.h>
#include <rdcart.h>
#include <rduser.h>
#include <rdripc.h>
#include <rdlibrary_conf.h>
#include <rdlistviewitem.h>
#include <rdconfig.h>
#include <rdsimpleplayer.h>

#include <disk_gauge.h>
#include <cart_tip.h>

#define RDLIBRARY_GEOMETRY_FILE ".rdlibrary"
#define RDLIBRARY_STEP_SIZE 5000

//
// Cut Length Deviation Values
//
#define RDLIBRARY_MID_LENGTH_LIMIT 500
#define RDLIBRARY_MID_LENGTH_COLOR darkYellow
#define RDLIBRARY_MAX_LENGTH_LIMIT 1500
#define RDLIBRARY_MAX_LENGTH_COLOR red
#define RDLIBRARY_ENFORCE_LENGTH_COLOR blue


class MainWidget : public QWidget
{
 Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void connectedData(bool state);
  void userData();
  void filterChangedData(const QString &str);
  void searchClickedData();
  void clearClickedData();
  void groupActivatedData(const QString &str);
  void addData();
  void editData();
  void deleteData();
  void ripData();
  void reportsData();
  void cartOnItemData(QListViewItem *item);
  void cartClickedData(QListViewItem *item);
  void cartDoubleclickedData(QListViewItem *,const QPoint &,int);
  void audioChangedData(int state);
  void macroChangedData(int state);
  void quitMainWidget();

 protected:
  void resizeEvent(QResizeEvent *e);
  void closeEvent(QCloseEvent *e);
  
 private:
  void RefreshList();
  void RefreshLine(RDListViewItem *item);
  void UpdateItemColor(RDListViewItem *item,RDCart::Validity validity,
		       const QDateTime &end_datetime,
		       const QDateTime &current_datetime); 
  void SetCaption(QString user);
  QString GetTypeFilter();
  void LoadGeometry();
  void SaveGeometry();
  RDListView *lib_cart_list;
  CartTip *lib_cart_tip;
  QString lib_filter_text;
  QString lib_search_text;
  QPixmap *lib_playout_map;
  QPixmap *lib_macro_map;
  QPixmap *lib_track_cart_map;
  QLineEdit *lib_filter_edit;
  QLabel *lib_filter_label;
  QComboBox *lib_group_box;
  QLabel *lib_group_label;
  QComboBox *lib_codes_box;
  QLabel *lib_codes_label;
  QPushButton *lib_search_button;
  QPushButton *lib_clear_button;
  QPushButton *lib_add_button;
  QPushButton *lib_edit_button;
  QPushButton *lib_delete_button;
  QPushButton *lib_rip_button;
  QPushButton *lib_reports_button;
  QPushButton *lib_close_button;
  QCheckBox *lib_showaudio_box;
  QLabel *lib_showaudio_label;
  QCheckBox *lib_showmacro_box;
  QLabel *lib_shownotes_label;
  QCheckBox *lib_shownotes_box;
  QLabel *lib_showmacro_label;
  QString lib_default_group;
  QString lib_import_path;
  RDInstanceLock *lib_lock;
  QPixmap *lib_rivendell_map;
  RDStation::FilterMode lib_filter_mode;
  QProgressDialog *lib_progress_dialog;
  QLabel *lib_selected_label;
  RDSimplePlayer *lib_player;
};


#endif 
