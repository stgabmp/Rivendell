// rdcut_path.cpp
//
// Get the full Cut Path for a given Cut Name.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdcut_path.cpp,v 1.5 2007/02/14 21:48:41 fredg Exp $
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

#include <rdcut.h>
#include <rdcart.h>
#include <rdcut_path.h>


QString RDCutPath(QString cutname)
{
  QString path;

  RDCut *cut=new RDCut(cutname); 
  if(!cut->exists()) {
    delete cut;
    return QString(QT_TR_NOOP("UNKNOWN CUT"));
  }
  RDCart *cart=new RDCart(cut->cartNumber());

  if(!cart->exists()) {
    delete cart;
    delete cut;
    return QString("UNKNOWN CUT");
  }
  if((cart->title().isEmpty())&&(cut->description().isEmpty())) {
    path=QString();
  }
  else {
    path=QString().sprintf("%s->%s",(const char *)cart->title(),
			   (const char *)cut->description());
  }
  delete cart;
  delete cut;
  return path;
}
