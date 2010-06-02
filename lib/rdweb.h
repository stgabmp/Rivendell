// rdweb.h
//
// Functions for interfacing with web components using the
// Common Gateway Interface (CGI) Standard 
//
//   (C) Copyright 1996-2007 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef RDWEB_H
#define RDWEB_H

#include <map>

#include <qstring.h>
#include <qhostaddress.h>
#include <qstringlist.h>

//
// Data Structure Sizes
//
#define CGI_ACCUM_SIZE 1024
#define RD_WEB_SESSION_TIMEOUT 900

//
// Function Prototypes
//
extern int RDReadPost(char *,int);
extern int RDPutPostString(char *,char *,char *,int);
extern int RDFindPostString(char *,char *,char *,int);
extern int RDGetPostString(char *,char *,char *,int);
extern int RDGetPostInt(char *,char *,int *);
extern int RDGetPostLongInt(char *,char *,long int *);
extern int RDPurgePostString(char *,char *,int);
extern int RDEncodeString(char *,int);
extern int RDEncodeSQLString(char *,int);
extern int RDDecodeString(char *);
extern int RDPutPlaintext(char *,int);
extern int RDPurgePlaintext(char *,int);
extern void RDCgiError(const char *);
extern int RDBufferDiff(char *,int,int,int);
extern void RDPruneAmp(char *);
extern int RDEscapeQuotes(const char *src,char *dest,int maxlen);
extern long int RDAuthenticateLogin(const QString &username,
				    const QString &passwd,
				    const QHostAddress &addr);
extern QString RDAuthenticateSession(long int session_id,
				     const QHostAddress &addr);
extern void RDLogoutSession(long int session_id,const QHostAddress &addr);
extern QString RDEscapeWebString(const QString &str);

extern bool RDParsePost(std::map<QString,QString> *vars);

#endif  // RDWEB_H
