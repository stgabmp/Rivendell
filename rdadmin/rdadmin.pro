// rdadmin.pro
//
// The QMake project file for RDAdmin.
//
// (C) Copyright 2003-2005 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdadmin.pro,v 1.6.2.3 2010/01/21 17:11:47 cvs Exp $
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

TEMPLATE = app

x11 {
  SOURCES += add_group.cpp
  SOURCES += add_hostvar.cpp
  SOURCES += add_matrix.cpp
  SOURCES += add_report.cpp
  SOURCES += add_station.cpp
  SOURCES += add_svc.cpp
  SOURCES += add_user.cpp
  SOURCES += autofill_carts.cpp
  SOURCES += createdb.cpp
  SOURCES += edit_audios.cpp
  SOURCES += edit_backup.cpp
  SOURCES += edit_decks.cpp
  SOURCES += edit_encoder.cpp
  SOURCES += edit_endpoint.cpp
  SOURCES += edit_gpi.cpp
  SOURCES += edit_group.cpp
  SOURCES += edit_hostvar.cpp
  SOURCES += edit_matrix.cpp
  SOURCES += edit_nownextplugin.cpp
  SOURCES += edit_now_next.cpp
  SOURCES += edit_rdairplay.cpp
  SOURCES += edit_rdlibrary.cpp
  SOURCES += edit_report.cpp
  SOURCES += edit_station.cpp
  SOURCES += edit_svc.cpp
  SOURCES += edit_svc_perms.cpp
  SOURCES += edit_ttys.cpp
  SOURCES += edit_user.cpp
  SOURCES += edit_vguest_resource.cpp
  SOURCES += info_dialog.cpp
  SOURCES += list_encoders.cpp
  SOURCES += list_endpoints.cpp
  SOURCES += list_gpis.cpp
  SOURCES += list_groups.cpp
  SOURCES += list_hostvars.cpp
  SOURCES += list_matrices.cpp
  SOURCES += list_reports.cpp
  SOURCES += list_stations.cpp
  SOURCES += list_svcs.cpp
  SOURCES += list_users.cpp
  SOURCES += login.cpp
  SOURCES += mysql_login.cpp
  SOURCES += opendb.cpp
  SOURCES += rdadmin.cpp
  SOURCES += rename_group.cpp
  SOURCES += test_import.cpp
  SOURCES += view_adapters.cpp
  SOURCES += xpm_info_banner1.cpp
  SOURCES += xpm_info_banner2.cpp
}

x11 {
  HEADERS += add_group.h
  HEADERS += add_hostvar.h
  HEADERS += add_matrix.h
  HEADERS += add_report.h
  HEADERS += add_station.h
  HEADERS += add_svc.h
  HEADERS += add_user.h
  HEADERS += autofill_carts.h
  HEADERS += createdb.h
  HEADERS += edit_audios.h
  HEADERS += edit_backup.h
  HEADERS += edit_decks.h
  HEADERS += edit_encoder.h
  HEADERS += edit_endpoint.h
  HEADERS += edit_gpi.h
  HEADERS += edit_group.h
  HEADERS += edit_hostvar.h
  HEADERS += edit_matrix.h
  HEADERS += edit_nownextplugin.h
  HEADERS += edit_now_next.h
  HEADERS += edit_rdairplay.h
  HEADERS += edit_rdlibrary.h
  HEADERS += edit_report.h
  HEADERS += edit_station.h
  HEADERS += edit_svc.h
  HEADERS += edit_svc_perms.h
  HEADERS += edit_ttys.h
  HEADERS += edit_user.h
  HEADERS += edit_vguest_resource.h
  HEADERS += info_dialog.h
  HEADERS += list_encoders.h
  HEADERS += list_endpoints.h
  HEADERS += list_gpis.h
  HEADERS += list_groups.h
  HEADERS += list_hostvars.h
  HEADERS += list_matrices.h
  HEADERS += list_reports.h
  HEADERS += list_stations.h
  HEADERS += list_svcs.h
  HEADERS += list_users.h
  HEADERS += login.h
  HEADERS += mysql_login.h
  HEADERS += opendb.h
  HEADERS += rdadmin.h
  HEADERS += rename_group.h
  HEADERS += test_import.h
  HEADERS += view_adapters.h
}

TRANSLATIONS += rdadmin_de.ts
TRANSLATIONS += rdadmin_es.ts
TRANSLATIONS += rdadmin_fr.ts
TRANSLATIONS += rdadmin_nb.ts
TRANSLATIONS += rdadmin_nn.ts
