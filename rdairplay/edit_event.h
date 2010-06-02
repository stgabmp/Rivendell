// edit_event.h
//
// Event Editor for RDAirPlay
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_event.h,v 1.21.2.2 2010/01/20 22:22:50 cvs Exp $
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

#ifndef EDIT_EVENT_H
#define EDIT_EVENT_H

#include <qdialog.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qdatetimeedit.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qtimer.h>

#include <rdtransportbutton.h>
#include <rdslider.h>
#include <rdcae.h>
#include <rdplay_deck.h>
#include <rdmarker_edit.h>
#include <rdpushbutton.h>
#include <rdtimeedit.h>

#include <marker_bar.h>
#include <log_play.h>

class EditEvent : public QDialog
{
  Q_OBJECT
 public:
  EditEvent(LogPlay *log,QWidget *parent=0,const char *name=0);
  ~EditEvent();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 public slots:
  int exec(int line);

 private slots:
  void timeChangedData(const QTime &);
  void timeToggledData(bool state);
  void graceClickedData(int id);
  void sliderPressedData();
  void sliderReleasedData();
  void sliderChangedData(int pos);
  void auditionButtonData();
  void pauseButtonData();
  void stopButtonData();
  void stateChangedData(int id,RDPlayDeck::State state);
  void positionData(int id,int msecs);
  void startClickedData();
  void endClickedData();
  void auditionTimerData();
  void okData();
  void cancelData();
  virtual void wheelEvent(QWheelEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);

 protected:
  void closeEvent(QCloseEvent *e);

 private:
  void SetStartMode(bool state);
  void SetEndMode(bool state);
  void ShowAudioControls(bool state);
  void Playing(int id);
  void Paused(int id);
  void Stopped(int id);
  void UpdateCounters();
  void ClearChannel();
  LogPlay *edit_log;
  RDLogLine *edit_logline;
  int edit_line;
  RDPlayDeck *edit_play_deck;
  RDTimeEdit *edit_time_edit;
  RDSlider *edit_slider;
  bool edit_time_changed;
  QLabel *edit_up_label;
  QLabel *edit_down_label;
  QCheckBox *edit_timetype_box;
  QLabel *edit_timetype_label;
  QButtonGroup *edit_grace_group;
  RDTimeEdit *edit_grace_edit;
  QLabel *edit_time_label;
  QComboBox *edit_transtype_box;
  QLabel *edit_transtype_label;
  QCheckBox *edit_overlap_box;
  QLabel *edit_overlap_label;
  QFont normal_font;
  RDTransportButton *edit_audition_button;
  RDTransportButton *edit_pause_button;
  RDTransportButton *edit_stop_button;
  QPushButton *edit_ok_button;
  QPushButton *edit_cancel_button;
  int edit_height;
  bool edit_slider_pressed;
  QPalette edit_play_color;
  QPalette edit_start_color;
  int edit_start_pos;
  QLabel *edit_position_label;
  MarkerBar *edit_position_bar;
  RDPushButton *edit_start_button;
  RDPushButton *edit_end_button;
  bool edit_shift_pressed;
  bool edit_right_click_stop;
  QTimer *edit_audition_timer;
};


#endif

