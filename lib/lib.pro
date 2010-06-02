# lib.pro
#
# The lib/ QMake project file for Rivendell.
#
# (C) Copyright 2003-2007 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: lib.pro,v 1.32.2.7 2010/01/25 18:56:16 cvs Exp $
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

TEMPLATE = lib

win32 {
  DEFINES += WIN32
  DEFINES += VERSION=\"$$[VERSION]\"
  DEFINES += PACKAGE=\"rivendell\" 
  DEFINES += PACKAGE_VERSION=\"$$[VERSION]\" 
  DEFINES += PACKAGE_NAME=\"rivendell\"
  DEFINES += PACKAGE_BUGREPORT=\"fredg@paravelsystems.com\"
}

SOURCES += rdcreate_log.cpp
SOURCES += rdescape_string.cpp
SOURCES += rduser.cpp
SOURCES += rdripc.cpp
SOURCES += rdlibrary_conf.cpp
SOURCES += rdlog_line.cpp
SOURCES += rdconfig.cpp
SOURCES += rdlog.cpp
SOURCES += rdlistview.cpp
SOURCES += rdlog_event.cpp
SOURCES += rdstation.cpp
SOURCES += rdcart_dialog.cpp
SOURCES += rdcart_search_text.cpp
SOURCES += rdcart.cpp
SOURCES += rdmacro.cpp
SOURCES += rdlistviewitem.cpp
SOURCES += rdevent.cpp
SOURCES += rdevent_line.cpp
SOURCES += rdclock.cpp
SOURCES += rdmacro_event.cpp
SOURCES += rdsvc.cpp
SOURCES += rdcut.cpp
SOURCES += rddatedecode.cpp
SOURCES += rdadd_log.cpp
SOURCES += rddebug.cpp
SOURCES += rdgroup_list.cpp
SOURCES += rdtextvalidator.cpp
SOURCES += rdreport.cpp
SOURCES += rdcombobox.cpp
SOURCES += rdexception_dialog.cpp
SOURCES += export_deltaflex.cpp
SOURCES += export_textlog.cpp
SOURCES += export_bmiemr.cpp
SOURCES += export_technical.cpp
SOURCES += export_soundex.cpp
SOURCES += export_radiotraffic.cpp
SOURCES += rdcmd_switch.cpp
SOURCES += rdlogedit_conf.cpp
SOURCES += rdtextfile.cpp
SOURCES += rddbheartbeat.cpp
SOURCES += rdget_ath.cpp
SOURCES += rdgetpasswd.cpp
SOURCES += rddropbox.cpp
SOURCES += schedruleslist.cpp
SOURCES += schedcartlist.cpp
SOURCES += rdclock.cpp
SOURCES += rdsegmeter.cpp
SOURCES += rdstereometer.cpp
SOURCES += rdtransportbutton.cpp
SOURCES += rdplaymeter.cpp
SOURCES += rdslider.cpp
SOURCES += rdsocket.cpp
SOURCES += rdlabel.cpp
SOURCES += rdlistselector.cpp
SOURCES += rdtimeengine.cpp
SOURCES += rdpushbutton.cpp
SOURCES += rdlineedit.cpp
SOURCES += rdaudiosettings.cpp
SOURCES += rdaudiosettings_dialog.cpp
SOURCES += rdlistviewitem.cpp
SOURCES += rdconf.cpp
SOURCES += rdprofile.cpp
SOURCES += rdprofileline.cpp
SOURCES += rdprofilesection.cpp
SOURCES += rddatepicker.cpp
SOURCES += rddatedialog.cpp
SOURCES += rdlicense.cpp
SOURCES += rdwavedata.cpp
SOURCES += rddb.cpp
SOURCES += rdintegeredit.cpp
SOURCES += rdintegerdialog.cpp
SOURCES += rdencoder.cpp
SOURCES += rdencoderlist.cpp
SOURCES += rdoneshot.cpp
SOURCES += rdsystem.cpp
SOURCES += rdtimeedit.cpp
win32 {
  SOURCES += rdwin32.cpp
  SOURCES += rdttydevice_win32.cpp
  SOURCES += html_gpl2_win32.cpp
}
x11 {
  SOURCES += rdairplay_conf.cpp
  SOURCES += rdaudio_exists.cpp
  SOURCES += rdaudio_port.cpp
  SOURCES += rdbutton_dialog.cpp
  SOURCES += rdbutton_panel.cpp
  SOURCES += rdcae.cpp
  SOURCES += rdcardselector.cpp
  SOURCES += rdcatch_connect.cpp
  SOURCES += rdcheck_daemons.cpp
  SOURCES += rdcheck_version.cpp
  SOURCES += rdcut_dialog.cpp
  SOURCES += rdcut_path.cpp
  SOURCES += rddeck.cpp
  SOURCES += rdexport_settings_dialog.cpp
  SOURCES += rdgpioselector.cpp
  SOURCES += rdgrid.cpp
  SOURCES += rdgroup.cpp
  SOURCES += rdmarker_button.cpp
  SOURCES += rdmarker_edit.cpp
  SOURCES += rdmatrix.cpp
  SOURCES += rdpanel_button.cpp
  SOURCES += rdpasswd.cpp
  SOURCES += rdplay_deck.cpp
  SOURCES += rdrecording.cpp
  SOURCES += rdsettings.cpp
  SOURCES += rdsound_panel.cpp
  SOURCES += rdtimeedit.cpp
  SOURCES += rdtty.cpp
  SOURCES += rdttyout.cpp
  SOURCES += rdversion.cpp
  SOURCES += rdadd_cart.cpp
  SOURCES += rdedit_audio.cpp
  SOURCES += rdimport_audio.cpp
  SOURCES += rdsimpleplayer.cpp
  SOURCES += rdwavefile.cpp
  SOURCES += rdcddblookup.cpp
  SOURCES += rdcdplayer.cpp
  SOURCES += rdcddbrecord.cpp
  SOURCES += rdttydevice.cpp
  SOURCES += html_gpl2.cpp
  SOURCES += rdcmd_cache.cpp
  SOURCES += rdedit_panel_name.cpp
  SOURCES += rdlist_groups.cpp
  SOURCES += rdlist_logs.cpp
}

HEADERS += rdescape_string.h
HEADERS += rduser.h
HEADERS += rdripc.h
HEADERS += rdlibrary_conf.h
HEADERS += rdlog_line.h
HEADERS += rdconfig.h
HEADERS += rdlog.h
HEADERS += rdlistview.h
HEADERS += rdlog_event.h
HEADERS += rdstation.h
HEADERS += rdcreate_log.h
HEADERS += rdcart_dialog.h
HEADERS += rdcae.h
HEADERS += rdcart_search_text.h
HEADERS += rd.h
HEADERS += rdcart.h
HEADERS += rdmacro.h
HEADERS += rdlistviewitem.h
HEADERS += rdevent.h
HEADERS += rdevent_line.h
HEADERS += rdclock.h
HEADERS += rdmacro_event.h
HEADERS += rdsvc.h
HEADERS += rdcut.h
HEADERS += rddatedecode.h
HEADERS += rdadd_log.h
HEADERS += rddebug.h
HEADERS += rdgroup_list.h
HEADERS += rdtextvalidator.h
HEADERS += rdreport.h
HEADERS += rdcombobox.h
HEADERS += rdexception_dialog.h
HEADERS += rdcmd_switch.h
HEADERS += rdlogedit_conf.h
HEADERS += rdtextfile.h
HEADERS += rddbheartbeat.h
HEADERS += rdget_ath.h
HEADERS += rdgetpasswd.h
HEADERS += rddropbox.h
HEADERS += schedruleslist.h
HEADERS += schedcartlist.h
HEADERS += rdclock.h
HEADERS += rdsegmeter.h
HEADERS += rdstereometer.h
HEADERS += rdtransportbutton.h
HEADERS += rdplaymeter.h
HEADERS += rdslider.h
HEADERS += rdsocket.h
HEADERS += rdlabel.h
HEADERS += rdlistselector.h
HEADERS += rdtimeengine.h
HEADERS += rdpushbutton.h
HEADERS += rdlineedit.h
HEADERS += rdaudiosettings.h
HEADERS += rdaudiosettings_dialog.h
HEADERS += rdlistviewitem.h
HEADERS += rdconf.h
HEADERS += rdprofile.h
HEADERS += rdprofileline.h
HEADERS += rdprofilesection.h
HEADERS += rdttydevice.h
HEADERS += rddatepicker.h
HEADERS += rddatedialog.h
HEADERS += rdlicense.h
HEADERS += rdwavedata.h
HEADERS += rddb.h
HEADERS += rdintegeredit.h
HEADERS += rdintegerdialog.h
HEADERS += rdencoder.h
HEADERS += rdencoderlist.h
HEADERS += rdoneshot.h
HEADERS += rdsystem.h
HEADERS += rdtimeedit.h
win32 {
  HEADERS += rdwin32.h
}
x11 {
  HEADERS += rdairplay_conf.h
  HEADERS += rdaudio_exists.h
  HEADERS += rdaudio_port.h
  HEADERS += rdbutton_dialog.h
  HEADERS += rdbutton_panel.h
  HEADERS += rdcae.h
  HEADERS += rdcardselector.h
  HEADERS += rdcatch_connect.h
  HEADERS += rdcheck_daemons.h
  HEADERS += rdcheck_version.h
  HEADERS += rdcut_dialog.h
  HEADERS += rdcut_path.h
  HEADERS += rddeck.h
  HEADERS += rdexport_settings_dialog.h
  HEADERS += rdgpioselector.h
  HEADERS += rdgrid.h
  HEADERS += rdgroup.h
  HEADERS += rdmarker_button.h
  HEADERS += rdmarker_edit.h
  HEADERS += rdmatrix.h
  HEADERS += rdpanel_button.h
  HEADERS += rdpasswd.h
  HEADERS += rdplay_deck.h
  HEADERS += rdrecording.h
  HEADERS += rdsettings.h
  HEADERS += rdsound_panel.h
  HEADERS += rdtimeedit.h
  HEADERS += rdtty.h
  HEADERS += rdttyout.h
  HEADERS += rdversion.h
  HEADERS += rdadd_cart.h
  HEADERS += rdedit_audio.h
  HEADERS += rdimport_audio.h
  HEADERS += rdsimpleplayer.h
  HEADERS += rdgpio.h
  HEADERS += rdcddblookup.h
  HEADERS += rdcdplayer.h
  HEADERS += rdcddbrecord.h
  HEADERS += rdpaths.h
  HEADERS += rdcmd_cache.h
  HEADERS += rdedit_panel_name.h
  HEADERS += rdlist_groups.h
  HEADERS += rdlist_logs.h
}

INCLUDEPATH += ..\..\libradio\radio

CONFIG += qt staticlib

TRANSLATIONS += librd_de.ts
TRANSLATIONS += librd_es.ts
TRANSLATIONS += librd_fr.ts
TRANSLATIONS += librd_nb.ts
TRANSLATIONS += librd_nn.ts
