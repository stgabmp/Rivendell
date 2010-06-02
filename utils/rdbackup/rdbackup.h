// rdbackup.h
//
// A Database Backup Tool for Rivendell
//
//   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdbackup.h,v 1.4 2007/02/14 21:59:12 fredg Exp $
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


#ifndef RDBACKUP_H
#define RDBACKUP_H

#include <list>

#include <qobject.h>
#include <qsqldatabase.h>

#include <rdconfig.h>
#include <rdcmd_switch.cpp>

#define RDBACKUP_USAGE "--destination-dir=<dir> [--purge-after=<days>]\n\nBackup the Rivendell database and (optionally) copy it to other hosts\nand/or purge old backups.\n\n"

//
// Global Variables
//
RDConfig *rdconfig;


class MainObject : public QObject
{
 public:
  MainObject(QObject *parent=0,const char *name=0);

 private:
  bool DumpDatabase(QString *filename);
  void PurgeFiles(const QString &path);
  void PushFile(const QString &filename);
  QString GetOutputName(const QString &path);
  QString destination_dir;
  QString output_name;
  unsigned purge_after_days;
};


#endif  // RDBACKUP_H
