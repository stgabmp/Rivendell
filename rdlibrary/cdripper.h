// cdripper.h
//
// CD Ripper Dialog for Rivendell
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: cdripper.h,v 1.11.2.2 2009/08/04 20:56:49 cvs Exp $
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

#ifndef CDRIPPER_H
#define CDRIPPER_H

#include <sys/types.h>
#include <unistd.h>
#include <qdialog.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qprogressbar.h>
#include <qtimer.h>
#include <qfile.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtextedit.h>

#include <rdtransportbutton.h>
#include <rdcdplayer.h>
#include <rdcddbrecord.h>
#include <rdcddblookup.h>

#include <rdlibrary_conf.h>

//
// Global Variables
//


class CdRipper : public QDialog
{
  Q_OBJECT
  public:
   CdRipper(QString cutname,RDCddbRecord *rec,RDLibraryConf *conf,
	    QWidget *parent=0,const char *name=0);
   ~CdRipper();
   QSize sizeHint() const;
   QSizePolicy sizePolicy() const;

  private slots:
   void trackSelectionChangedData();
   void ejectButtonData();
   void playButtonData();
   void stopButtonData();
   void ripTrackButtonData();
   void ejectedData();
   void mediaChangedData();
   void playedData(int);
   void stoppedData();
   void cddbDoneData(RDCddbLookup::Result);
   void normalizeCheckData(bool);
   void autotrimCheckData(bool);
   void barTimerData();
   void closeData();

 protected:
   void closeEvent(QCloseEvent *e);

  private:
   void AutoTrim();
   RDLibraryConf *rip_conf;
   RDCdPlayer *rip_cdrom;
   RDCddbRecord *rip_cddb_record;
   RDCddbLookup *rip_cddb_lookup;
   QListView *rip_track_list;
   QPushButton *rip_rip_button;
   bool rip_rip_aborted;
   QPushButton *rip_close_button;
   QLineEdit *rip_album_edit;
   QLineEdit *rip_artist_edit;
   QTextEdit *rip_other_edit;
   QCheckBox *rip_apply_box;
   QLabel *rip_apply_label;
   RDTransportButton *rip_eject_button;
   RDTransportButton *rip_play_button;
   RDTransportButton *rip_stop_button;
   QString rip_cutname;
   QString rip_wavefile;
   QProgressBar *rip_bar;
   QCheckBox *rip_normalize_box;
   QSpinBox *rip_normalize_spin;
   QLabel *rip_normalize_label;
   QLabel *rip_normalize_unit;
   QComboBox *rip_channels_box;
   QTimer *rip_bar_timer;
   QFile *bar_file;
   QFile *bar_temp_file;
   int rip_temp_length;
   int rip_finished_length;
   pid_t rip_ripper_pid;
   int rip_track;
   QCheckBox *rip_autotrim_box;
   QSpinBox *rip_autotrim_spin;
   QLabel *rip_autotrim_label;
   QLabel *rip_autotrim_unit;   
   bool rip_done;
};


#endif

