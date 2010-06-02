//   rdcddblookup.h
//
//   A Qt class for accessing the FreeDB CD Database.
//
//   (C) Copyright 2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rdcddblookup.h,v 1.1 2007/09/14 14:06:24 fredg Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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
//

#ifndef RDCDDBLOOKUP_H
#define RDCDDBLOOKUP_H

#include <qobject.h>
#include <qsocket.h>
#include <rdcddbrecord.h>

//
// Default Settings
//
#define RDCDDBLOOKUP_DEFAULT_PORT 8880
#define RDCDDBLOOKUP_DEFAULT_USER "libradio"
#define RDCDDBLOOKUP_DEFAULT_HOSTNAME "linux"

/**
 * @short Lookup CD Data from the FreeDB CD Database
 * @author Fred Gleason <fredg@paravelsystems.com>
 *
 * This class implements an object for accessing a remote FreeDB CD
 * database server.  
 **/

class RDCddbLookup : public QObject
{
  Q_OBJECT
  public:
   enum Result {ExactMatch=0,PartialMatch=1,NoMatch=2,
		ProtocolError=3,NetworkError=4};
   RDCddbLookup(QObject *parent=0,const char *name=0);
   ~RDCddbLookup();
   void setCddbRecord(RDCddbRecord *);
   void lookupRecord(QString cdda_dir,QString cdda_dev,QString hostname,
		     Q_UINT16 port=RDCDDBLOOKUP_DEFAULT_PORT,
		     QString username="",
		     QString appname=PACKAGE_NAME,QString ver=VERSION);

  private slots:
   void connectedData();
   void readyReadData();
   void errorData(int);

  signals:
   void done(RDCddbLookup::Result);

  private:
   QString DecodeString(QString &str);
   void ParsePair(QString *line,QString *tag,QString *value,int *index);
   int GetIndex(QString *tag);
   bool GetCddaData(QString &cdda_dir,QString &cdda_dev);
   RDCddbRecord *lookup_record;
   QSocket *lookup_socket;
   int lookup_state;
   QString lookup_username;
   QString lookup_appname;
   QString lookup_appver;
   QString lookup_hostname;
};

#endif  // RDCDDBLOOKUP_H
