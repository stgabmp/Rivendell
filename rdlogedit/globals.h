// globals.h
//
// Global Variable Declarations for RDLogEdit
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: globals.h,v 1.7 2007/12/31 18:49:11 fredg Exp $
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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <rdstation.h>
#include <rduser.h>
#include <rdripc.h>
#include <rdconfig.h>
#include <rdlogedit_conf.h>
#include <rdcart_dialog.h>
#ifndef WIN32
#include <rdcae.h>
#endif  // WIN32

//
// Global Resources
//
extern RDStation *rdstation_conf;
extern RDUser *rduser;
extern RDRipc *rdripc;
extern RDConfig *log_config;
extern RDLogeditConf *rdlogedit_conf;
extern RDCartDialog *log_cart_dialog;
extern bool import_running;
#ifndef WIN32
extern RDCae *rdcae;
#endif  // WIN32

#endif  // GLOBALS_H
