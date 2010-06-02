// import_audio.h
//
// CD Ripper Dialog for Rivendell
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdimport_audio.h,v 1.8.2.1 2010/02/09 21:43:53 cvs Exp $
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

#ifndef IMPORT_AUDIO_H
#define IMPORT_AUDIO_H

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
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <rdwavedata.h>
#include <rdconfig.h>
#include <rdsettings.h>
#include <rdexport_settings_dialog.h>
#include <rdcut.h>

//
// Global Variables
//
extern bool ripper_running;

//
// Widget Settings
//
#define IMPORT_BAR_INTERVAL 500
#define IMPORT_TEMP_BASENAME "rdlib"


class RDImportAudio : public QDialog
{
 Q_OBJECT
 public:
 RDImportAudio(QString cutname,QString *path,RDSettings *settings,
	       bool *import_metadata,RDWaveData *wavedata,
	       RDCut *clipboard,RDStation *station,bool *running,
	       RDConfig *config,QWidget *parent=0,const char *name=0);
  ~RDImportAudio();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  void enableAutotrim(bool state);
  void setAutotrimLevel(int lvl);
  void enableNormalization(bool state);
  void setNormalizationLevel(int lvl);
  void setChannels(int chans);

 public slots:
  int exec(bool enable_import,bool enable_export);

 private slots:
  void modeClickedData(int id);
  void filenameChangedData(const QString &str);
  void normalizeCheckData(bool state);
  void autotrimCheckData(bool state);
  void barTimerData();
  void selectInputFileData();
  void selectOutputFileData();
  void selectOutputFormatData();
  void importData();
  void cancelData();

 protected:
  void paintEvent(QPaintEvent *e);
  void closeEvent(QCloseEvent *e);

 private:
  void Import();
  void ImportProgress();
  void Export();
  void ExportProgress();
  void AutoTrim();
  unsigned GetFileSize(const QString &filename);
  RDConfig *import_config;
  RDSettings *import_default_settings;
  RDStation *import_station;
  RDCut *import_clipboard;
  bool *import_running;
  QButtonGroup *import_mode_group;
  QRadioButton *import_importmode_button;
  QRadioButton *import_exportmode_button;
  QLabel *import_in_filename_label;
  QLineEdit *import_in_filename_edit;
  QCheckBox *import_in_metadata_box;
  QPushButton *import_in_selector_button;
  QLabel *import_channels_label;
  QLabel *import_out_filename_label;
  QLineEdit *import_out_filename_edit;
  QCheckBox *import_out_metadata_box;
  QPushButton *import_out_selector_button;
  QLabel *import_format_label;
  QLineEdit *import_format_edit;
  QPushButton *import_out_format_button;
  QProgressBar *import_bar;
  QTimer *import_bar_timer;
  QCheckBox *import_normalize_box;
  QSpinBox *import_normalize_spin;
  QLabel *import_normalize_label;
  QLabel *import_normalize_unit;
  QCheckBox *import_autotrim_box;
  QSpinBox *import_autotrim_spin;
  QLabel *import_autotrim_label;
  QLabel *import_autotrim_unit;
  QComboBox *import_channels_box;
  QPushButton *import_cancel_button;
  QPushButton *import_import_button;
  QString *import_path;
  QString import_file_filter;
  bool import_multipass;
  QString import_cutname;
  QString import_dest_filename;
  QFile bar_temp_file;
  pid_t import_script_pid;
  bool import_import_aborted;
  unsigned import_temp_length;
  unsigned import_finished_length;
  bool *import_import_metadata;
  RDSettings *import_settings;
  RDWaveData *import_wavedata;
  QString import_tempwav_name;
  QString import_tempdat_name;
};


#endif

