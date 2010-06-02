// sas64000gpi.h
//
// A Rivendell switcher driver for the SAS64000 connected via
//   a GPI-1600
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: sas64000gpi.h,v 1.5 2008/09/30 18:19:45 fredg Exp $
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

#ifndef SAS64000GPI_H
#define SAS64000GPI_H

#include <qobject.h>

#include <rd.h>
#include <rdmatrix.h>
#include <rdmacro.h>
#include <rdtty.h>
#include <rdoneshot.h>

class Sas64000Gpi : public QObject
{
 Q_OBJECT
 public:
  Sas64000Gpi(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~Sas64000Gpi();
  void processCommand(RDMacro *cmd);

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpoChanged(int matrix,int line,bool state);

 private slots:
  void gpoOneshotData(void *data);

 private:
  RDTTYDevice *sas_device;
  RDOneShot *sas_gpo_oneshot;
  int sas_matrix;
  int sas_inputs;
  int sas_outputs;
  int sas_gpis;
  int sas_gpos;
};


#endif  // SAS64000_H
