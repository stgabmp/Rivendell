// local_gpio.h
//
// A Rivendell switcher driver for MeasurementComputing GPIO cards.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: local_gpio.h,v 1.9 2008/09/30 18:19:43 fredg Exp $
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

#ifndef LOCAL_GPIO_H
#define LOCAL_GPIO_H

#include <qobject.h>

#include <rdgpio.h>
#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdoneshot.h>

class LocalGpio : public QObject
{
 Q_OBJECT
 public:
  LocalGpio(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~LocalGpio();
  void processCommand(RDMacro *cmd);

 private slots:
  void gpiChangedData(int line,bool state);
  void gpoChangedData(int line,bool state);
  void gpiOneshotData(void *data);

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpiChanged(int matrix,int line,bool state);
  void gpoChanged(int matrix,int line,bool state);

 private:
  RDGpio *gpio_gpio;
  RDOneShot *gpio_gpi_oneshot;
  int gpio_matrix;
  int gpio_gpis;
  int gpio_gpos;
  bool gpio_open;
  bool gpio_gpi_mask[GPIO_MAX_LINES];
};


#endif  // LOCAL_GPIO_H
