// rddatedecode.cpp
//
// Decode Rivendell Date Macros
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rddatedecode.cpp,v 1.5.6.1 2008/11/06 16:32:29 fredg Exp $
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

#include <rddatedecode.h>


QString RDDateDecode(QString str,QDate date)
{
  QString string;
  int yearnum;
  int dow;
  bool upper_case=false;

  for(unsigned i=0;i<str.length();i++) {
    if(str.at(i)!='%') {
      string+=str.at(i);
    }
    else {
      i++;
      if(((const char *)str)[i]=='^') {
	upper_case=true;
	i++;
      }
      switch(((const char *)str)[i]) {
	  case 'a':   // Abbreviated weekday name
	    if(upper_case) {
	      string+=QDate::shortDayName(date.dayOfWeek()).upper();
	    }
	    else {
	      string+=QDate::shortDayName(date.dayOfWeek()).lower();
	    }
	    break;

	  case 'A':   // Full weekday name
	    if(upper_case) {
	      string+=QDate::longDayName(date.dayOfWeek()).upper();
	    }
	    else {
	      string+=QDate::longDayName(date.dayOfWeek()).lower();
	    }
	    break;

	  case 'b':   // Abbreviated month name
	  case 'h':
	    if(upper_case) {
	      string+=QDate::shortMonthName(date.month()).upper();
	    }
	    else {
	      string+=QDate::shortMonthName(date.month()).lower();
	    }
	    break;

	  case 'B':   // Full month name
	    if(upper_case) {
	      string+=QDate::longMonthName(date.month()).upper();
	    }
	    else {
	      string+=QDate::longMonthName(date.month()).lower();
	    }
	    break;

	  case 'C':   // Century
	    string+=QString().sprintf("%02d",date.year()/100);
	    break;

	  case 'd':   // Day (01 - 31)
	    string+=QString().sprintf("%02d",date.day());
	    break;

	  case 'D':   // Date (mm-dd-yy)
	    string+=date.toString("dd-MM-yy");
	    break;

	  case 'F':   // Date (yyyy-mm-dd)
	    string+=date.toString("yyyy-MM-dd");
	    break;

	  case 'g':   // Two digit year number (as per ISO 8601)
	    date.weekNumber(&yearnum);
	    string+=QString().sprintf("%02d",yearnum-2000);
	    break;

	  case 'G':   // Two digit year number (as per ISO 8601)
	    date.weekNumber(&yearnum);
	    string+=QString().sprintf("%04d",yearnum);
	    break;

	  case 'j':   // Day of year
	    string+=QString().sprintf("%03d",date.dayOfYear());
	    break;

	  case 'm':   // Month (01 - 12)
	    string+=QString().sprintf("%02d",date.month());
	    break;
	    
	  case 'u':   // Day of week (numeric, 1..7, 1=Monday)
	    string+=QString().sprintf("%d",date.dayOfWeek());
	    break;
	    
	  case 'V':   // Week number (as per ISO 8601)
	  case 'W':
	    string+=QString().sprintf("%d",date.weekNumber());
	    break;

	  case 'w':   // Day of week (numeric, 0..6, 0=Sunday)
	    dow=date.dayOfWeek();
	    if(dow==7) {
	      dow=0;
	    }
	    string+=QString().sprintf("%d",dow);
	    break;
	    
	  case 'y':   // Year (yy)
	    string+=QString().sprintf("%02d",date.year()-2000);
	    break;

	  case 'Y':   // Year (yyyy)
	    string+=QString().sprintf("%04d",date.year());
	    break;

	  case '%':   // Literal '%'
	    string+=QString("%");
	    break;
      }
    }
  }
  return string;
}


QString RDDateTimeDecode(QString str,QDateTime datetime)
{
  QString string;
  int yearnum;
  int dow;

  for(unsigned i=0;i<str.length();i++) {
    if(str.at(i)!='%') {
      string+=str.at(i);
    }
    else {
      i++;
      switch(((const char *)str)[i]) {
	  case 'a':   // Abbreviated weekday name
	    string+=QDate::shortDayName(datetime.date().dayOfWeek()).lower();
	    break;

	  case 'A':   // Full weekday name
	    string+=QDate::longDayName(datetime.date().dayOfWeek()).lower();
	    break;

	  case 'b':   // Abbreviated month name
	  case 'h':
	    string+=QDate::shortMonthName(datetime.date().month()).lower();
	    break;

	  case 'B':   // Full month name
	    string+=QDate::longMonthName(datetime.date().month()).lower();
	    break;

	  case 'C':   // Century
	    string+=QString().sprintf("%02d",datetime.date().year()/100);
	    break;

	  case 'd':   // Day (01 - 31)
	    string+=QString().sprintf("%02d",datetime.date().day());
	    break;

	  case 'D':   // Date (mm-dd-yy)
	    string+=datetime.date().toString("MM-dd-yy");
	    break;

	  case 'F':   // Date (yyyy-mm-dd)
	    string+=datetime.date().toString("yyyy-MM-dd");
	    break;

	  case 'g':   // Two digit year number (as per ISO 8601)
	    datetime.date().weekNumber(&yearnum);
	    string+=QString().sprintf("%02d",yearnum-2000);
	    break;

	  case 'G':   // Two digit year number (as per ISO 8601)
	    datetime.date().weekNumber(&yearnum);
	    string+=QString().sprintf("%04d",yearnum);
	    break;

	  case 'H':   // Hour (HH)
	    string+=QString().sprintf("%02d",datetime.time().hour());
	    break;

	  case 'j':   // Day of year
	    string+=QString().sprintf("%03d",datetime.date().dayOfYear());
	    break;

	  case 'M':   // Minute (MM)
	    string+=QString().sprintf("%02d",datetime.time().minute());
	    break;

	  case 'm':   // Month (01 - 12)
	    string+=QString().sprintf("%02d",datetime.date().month());
	    break;
	    
	  case 'S':   // Second (SS)
	    string+=QString().sprintf("%02d",datetime.time().second());
	    break;

	  case 'u':   // Day of week (numeric, 1..7, 1=Monday)
	    string+=QString().sprintf("%d",datetime.date().dayOfWeek());
	    break;
	    
	  case 'V':   // Week number (as per ISO 8601)
	  case 'W':
	    string+=QString().sprintf("%d",datetime.date().weekNumber());
	    break;

	  case 'w':   // Day of week (numeric, 0..6, 0=Sunday)
	    dow=datetime.date().dayOfWeek();
	    if(dow==7) {
	      dow=0;
	    }
	    string+=QString().sprintf("%d",dow);
	    break;
	    
	  case 'y':   // Year (yy)
	    string+=QString().sprintf("%02d",datetime.date().year()-2000);
	    break;

	  case 'Y':   // Year (yyyy)
	    string+=QString().sprintf("%04d",datetime.date().year());
	    break;

	  case '%':   // Literal '%'
	    string+=QString("%");
	    break;
      }
    }
  }
  return string;
}
