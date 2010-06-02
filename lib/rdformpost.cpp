// rdformpost.cpp
//
// Handle data from an HTML form.
//
//   (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdformpost.cpp,v 1.1.2.2 2009/07/01 21:52:58 cvs Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <rdformpost.h>

RDFormPost::RDFormPost(RDFormPost::Encoding encoding,unsigned maxsize,
		       bool auto_delete)
{
  char tempdir[PATH_MAX];
  bool ok=false;

  post_encoding=encoding;
  post_error=RDFormPost::ErrorNotInitialized;
  post_auto_delete=auto_delete;

  //
  // Verify Transfer Type
  //
  if(getenv("REQUEST_METHOD")==NULL) {
    post_error=RDFormPost::ErrorNotPost;
    return;
  }
  if(QString(getenv("REQUEST_METHOD")).lower()!="post") {
    post_error=RDFormPost::ErrorNotPost;
    return;
  }

  //
  // Verify Size
  //
  if(getenv("CONTENT_LENGTH")==NULL) {
    post_error=RDFormPost::ErrorPostTooLarge;
    return;
  }
  post_content_length=QString(getenv("CONTENT_LENGTH")).toUInt(&ok);
  if((!ok)||((maxsize>0)&&(post_content_length>maxsize))) {
    post_error=RDFormPost::ErrorPostTooLarge;
    return;
  }

  //
  // Initialize Temp Directory Path
  //
  if(getenv("TMPDIR")!=NULL) {
    strcpy(tempdir,getenv("TMPDIR"));
  }
  else {
    strcpy(tempdir,"/tmp");
  }
  strcat(tempdir,"/rivendellXXXXXX");
  post_tempdir=mkdtemp(tempdir);
  if(post_tempdir.isNull()) {
    post_error=RDFormPost::ErrorNoTempDir;
    return;
  }

  switch(post_encoding) {
  case RDFormPost::UrlEncoded:
    LoadUrlEncoding();
    break;

  case RDFormPost::MultipartEncoded:
    LoadMultipartEncoding();
    break;
  }
}


RDFormPost::~RDFormPost()
{
  if(post_auto_delete) {
    for(std::map<QString,bool>::const_iterator ci=post_filenames.begin();
	ci!=post_filenames.end();ci++) {
      if(ci->second) {
	unlink(post_values[ci->first].toString());
      }
    }
    if(!post_tempdir.isNull()) {
      rmdir(post_tempdir);
    }
  }
}


RDFormPost::Error RDFormPost::error() const
{
  return post_error;
}


QStringList RDFormPost::names() const
{
  QStringList list;
  for(std::map<QString,QVariant>::const_iterator ci=post_values.begin();
      ci!=post_values.end();ci++) {
    list.push_back(ci->first);
  }
  return list;
}


QVariant RDFormPost::value(const QString &name,bool *ok)
{
  QVariant v;
  if(post_values.count(name)>0) {
    v=post_values[name];
  }
  if(ok!=NULL) {
    *ok=(post_values.count(name)>0);
  }
  return v;
}


bool RDFormPost::getValue(const QString &name,QString *str)
{
  if(post_values.count(name)>0) {
    *str=post_values[name].toString();
    return true;
  }
  return false;
}


bool RDFormPost::getValue(const QString &name,int *n)
{
  if(post_values.count(name)>0) {
    *n=post_values[name].toInt();
    return true;
  }
  return false;
}


bool RDFormPost::getValue(const QString &name,long *n)
{
  if(post_values.count(name)>0) {
    *n=post_values[name].toLongLong();
    return true;
  }
  *n=0;
  return false;
}


bool RDFormPost::isFile(const QString &name)
{
  return post_filenames[name];
}


void RDFormPost::dump()
{
  printf("Content-type: text/html\n\n");
  printf("<table cellpadding=\"5\" cellspacing=\"0\" border=\"1\">\n");
  printf("<tr>\n");
  printf("<td colspan=\"3\" align=\"center\"><strong>RDFormPost Data Dump</strong></td>\n");
  printf("</tr>\n");

  printf("<tr>\n");
  printf("<th align=\"center\">NAME</th>\n");
  printf("<th align=\"center\">VALUE</th>\n");
  printf("<th align=\"center\">FILE</th>\n");
  printf("</tr>\n");
  
  for(std::map<QString,QVariant>::const_iterator ci=post_values.begin();
      ci!=post_values.end();ci++) {
    printf("<tr>\n");
    printf("<td align=\"left\">|%s|</td>\n",(const char *)ci->first);
    printf("<td align=\"left\">|%s|</td>\n",(const char *)ci->second.toString());
    if(post_filenames[ci->first]) {
      printf("<td align=\"center\">Yes</td>\n");
    }
    else {
      printf("<td align=\"center\">No</td>\n");
    }
    printf("</tr>\n");
  }

  printf("</table>\n");
}


QString RDFormPost::errorString(RDFormPost::Error err)
{
  QString str="Unknown error";

  switch(err) {
  case RDFormPost::ErrorOk:
    str="OK";
    break;

  case RDFormPost::ErrorNotPost:
    str="Request is not POST";
    break;

  case RDFormPost::ErrorNoTempDir:
    str="Unable to create temporary directory";
    break;

  case RDFormPost::ErrorMalformedData:
    str="The data is malformed";
    break;

  case RDFormPost::ErrorPostTooLarge:
    str="POST is too large";
    break;

  case RDFormPost::ErrorInternal:
    str="Internal error";
    break;

  case RDFormPost::ErrorNotInitialized:
    str="POST class not initialized";
    break;
  }
  return str;
}


void RDFormPost::LoadUrlEncoding()
{
  char *data=new char[post_content_length+1];
  int n;
  QStringList lines;
  QStringList line;

  if((n=read(0,data,post_content_length))<0) {
    post_error=RDFormPost::ErrorMalformedData;
    return;
  }
  data[post_content_length]=0;
  lines=lines.split("&",data);
  for(unsigned i=0;i<lines.size();i++) {
    line=line.split("=",lines[i]);
    if(line.size()==2) {
      post_values[line[0]]=UrlDecode(line[1]);
      post_filenames[line[0]]=false;
    }
  }

  post_error=RDFormPost::ErrorOk;
  delete data;
}


void RDFormPost::LoadMultipartEncoding()
{
  std::map<QString,QString> headers;
  bool header=true;
  char *data=NULL;
  FILE *f=NULL;
  ssize_t n=0;
  QString sep;
  QString name;
  QString filename;
  QString tempdir;
  int fd=-1;

  if((f=fdopen(0,"r"))==NULL) {
    post_error=RDFormPost::ErrorInternal;
    return;
  }



  /*
  int out=open("/tmp/output.dat",O_WRONLY|O_CREAT);
  while((n=getline(&data,(size_t *)&n,f))>0) {
    write(out,data,n);
  }
  ::close(out);
  printf("Content-type: text/html\n\n");
  printf("DONE!\n");
  */








  if((n=getline(&data,(size_t *)&n,f))<=0) {
    post_error=RDFormPost::ErrorMalformedData;
    return;
  }
  sep=QString(data).stripWhiteSpace();

  //
  // Get message parts
  //
  while((n=getline(&data,(size_t *)&n,f))>0) {
    if(QString(data).stripWhiteSpace().contains(sep)>0) {  // End of part
      if(fd>=0) {
	ftruncate(fd,lseek(fd,0,SEEK_CUR)-2);  // Remove extraneous final CR/LF
	::close(fd);
	fd=-1;
      }
      name="";
      filename="";
      headers.clear();
      header=true;
      continue;
    }
    if(header) {  // Read header
      if(QString(data).stripWhiteSpace().isEmpty()) {
	if(!headers["content-disposition"].isNull()) {
	  QStringList fields;
	  fields=fields.split(";",headers["content-disposition"]);
	  if(fields.size()>0) {
	    if(fields[0].lower().stripWhiteSpace()=="form-data") {
	      for(unsigned i=1;i<fields.size();i++) {
		QStringList pairs;
		pairs=pairs.split("=",fields[i]);
		if(pairs[0].lower().stripWhiteSpace()=="name") {
		  name=pairs[1].stripWhiteSpace();
		  name.replace("\"","");
		}
		if(pairs[0].lower().stripWhiteSpace()=="filename") {
		  filename=post_tempdir+"/"+pairs[1].stripWhiteSpace();
		  filename.replace("\"","");
		  fd=open(filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
		}
	      }
	    }
	  }
	}
	header=false;
      }
      else {
	QStringList hdr;
	hdr=hdr.split(":",QString(data).stripWhiteSpace());
	// Reconcaternate trailing sections so we don't split on the 
	// useless M$ drive letter supplied by IE
	for(unsigned i=2;i<hdr.size();i++) {
	  hdr[1]+=hdr[i];
	}
	headers[hdr[0].lower()]=hdr[1];
      }
    }
    else {  // Read data
      if(filename.isEmpty()) {
	QString str=post_values[name].toString();
	str+=QString(data);
	post_filenames[name]=false;
	post_values[name]=str.stripWhiteSpace();
      }
      else {
	post_filenames[name]=true;
	post_values[name]=filename;
	write(fd,data,n);
      }
    }
  }
  free(data);
  post_error=RDFormPost::ErrorOk;
}


QString RDFormPost::UrlDecode(const QString &str) const
{
  int istate=0;
  unsigned n;
  QString code;
  QString ret;
  bool ok=false;

  for(unsigned i=0;i<str.length();i++) {
    switch(istate) {
    case 0:
      if(str.at(i)==QChar('+')) {
	ret+=" ";
      }
      else {
	if(str.at(i)==QChar('%')) {
	  istate=1;
	}
	else {
	  ret+=str.at(i);
	}
      }
      break;

    case 1:
      n=str.mid(i,1).toUInt(&ok);
      if((!ok)||(n>9)) {
	istate=0;
      }
      code=str.mid(i,1);
      istate=2;
      break;

    case 2:
      n=str.mid(i,1).toUInt(&ok);
      if((!ok)||(n>9)) {
	istate=0;
      }
      code+=str.mid(i,1);
      ret+=QChar(code.toInt(&ok,16));
      istate=0;
      break;
    }
  }

  return ret;
}
