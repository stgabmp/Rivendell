// rdformpost.h
//
// Handle POST data from an HTML form.
//
//   (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdformpost.h,v 1.1.2.1 2009/05/21 17:32:17 cvs Exp $
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

#ifndef RDFORMPOST_H
#define RDFORMPOST_H

#include <map>

#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

class RDFormPost
{
 public:
  enum Encoding {UrlEncoded=0,MultipartEncoded=1};
  enum Error {ErrorOk=0,ErrorNotPost=1,ErrorNoTempDir=2,ErrorMalformedData=3,
	      ErrorPostTooLarge=4,ErrorInternal=5,ErrorNotInitialized=6};
  RDFormPost(RDFormPost::Encoding encoding,unsigned maxsize=0,
	     bool auto_delete=true);
  ~RDFormPost();
  RDFormPost::Error error() const;
  QStringList names() const;
  QVariant value(const QString &name,bool *ok=NULL);
  bool getValue(const QString &name,QString *str);
  bool getValue(const QString &name,int *n);
  bool getValue(const QString &name,long *n);
  bool isFile(const QString &name);
  void dump();
  static QString errorString(RDFormPost::Error err);

 private:
  void LoadUrlEncoding();
  void LoadMultipartEncoding();
  QString UrlDecode(const QString &str) const;
  RDFormPost::Encoding post_encoding;
  RDFormPost::Error post_error;
  std::map<QString,QVariant> post_values;
  std::map<QString,bool> post_filenames;
  QString post_tempdir;
  bool post_auto_delete;
  unsigned post_content_length;
};


#endif  // RDFORMPOST_H
