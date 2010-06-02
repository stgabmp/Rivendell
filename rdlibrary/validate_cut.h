// validate_cut.h
//
// Validate a Rivendell Audio Cut
//
//   (C) Copyright 2006 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: validate_cut.h,v 1.3 2007/10/05 13:50:33 fredg Exp $
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

#ifndef VALIDATE_CUT_H
#define VALIDATE_CUT_H

#include <qsqldatabase.h>
#include <qdatetime.h>

#include <rdcart.h>


RDCart::Validity ValidateCut(RDSqlQuery *q,unsigned offset,
			     RDCart::Validity prev_validity,
			     const QDateTime &datetime);

#endif  // VALIDATE_CUT_H
