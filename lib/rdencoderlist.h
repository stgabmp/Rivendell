// rdencoderlist.h
//
// Abstract a Rivendell Custom Encoder
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdencoderlist.h,v 1.1 2008/09/18 19:02:07 fredg Exp $
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

#ifndef RDENCODERLIST_H
#define RDENCODERLIST_H

#include <vector>

#include <qstring.h>

#include <rdencoder.h>

class RDEncoderList
{
 public:
  RDEncoderList(const QString &stationname);
  ~RDEncoderList();
  unsigned encoderQuantity() const;
  RDEncoder *encoder(unsigned n);

 private:
  std::vector<RDEncoder *> list_encoders;
};


#endif  // RDENCODERLIST_H
