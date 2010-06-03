## automake.am
##
## Automake.am for rivendell/lib
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.110.2.9 2010/01/21 17:11:47 cvs Exp $
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" @QT_CXXFLAGS@
MOC = @QT_MOC@
CWRAP = ../helpers/cwrap

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

# The cwrap dependency
html_%.cpp:	%.html
	$(CWRAP) -o $@ $<

instdir = @LOCAL_PREFIX@/lib

# I18N Stuff
install-exec-local:	
	mkdir -p $(DESTDIR)$(prefix)/share/rivendell
	cp librd_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/librd_*.qm

all:	
	@QT_BIN@/lupdate lib.pro
	@QT_BIN@/lrelease lib.pro

lib_LTLIBRARIES = librd.la
dist_librd_la_SOURCES = rduser.cpp rduser.h\
                   rdsvc.cpp rdsvc.h\
                   rdcart.cpp rdcart.h\
                   rdcart_search_text.cpp rdcart_search_text.h\
                   rdgroup.cpp rdgroup.h\
                   rdcut.cpp rdcut.h\
                   rdcart_dialog.cpp rdcart_dialog.h\
                   rdcut_dialog.cpp rdcut_dialog.h\
                   rdstation.cpp rdstation.h\
                   rdtty.cpp rdtty.h\
                   rddeck.cpp rddeck.h\
                   rdlog.cpp rdlog.h\
                   rdlog_event.cpp rdlog_event.h\
                   rdlog_line.cpp rdlog_line.h\
                   rdcreate_log.cpp rdcreate_log.h\
                   rdrecording.cpp rdrecording.h\
                   rdlibrary_conf.cpp rdlibrary_conf.h\
                   rdsettings.cpp rdsettings.h\
                   rdpasswd.cpp rdpasswd.h\
                   rdripc.cpp rdripc.h\
                   rdcae.cpp rdcae.h\
                   rdcardselector.cpp rdcardselector.h\
                   rdgpioselector.cpp rdgpioselector.h\
                   rdttyout.cpp rdttyout.h\
                   rdaudio_port.cpp rdaudio_port.h\
                   rdcut_path.cpp rdcut_path.h\
                   rdmixer.cpp rdmixer.h\
                   rdplay_deck.cpp rdplay_deck.h\
                   rdaudio_exists.cpp rdaudio_exists.h\
                   rdversion.cpp rdversion.h\
                   rdcheck_version.cpp rdcheck_version.h\
                   rdcheck_daemons.cpp rdcheck_daemons.h\
                   rdairplay_conf.cpp rdairplay_conf.h\
                   rdcatch_connect.cpp rdcatch_connect.h\
                   rdlistview.cpp rdlistview.h\
                   rdlistviewitem.cpp rdlistviewitem.h\
                   rdtimeedit.cpp rdtimeedit.h\
                   rdmacro_event.cpp rdmacro_event.h\
                   rdmacro.cpp rdmacro.h\
                   rdmatrix.cpp rdmatrix.h\
                   rdevent.cpp rdevent.h\
                   rdevent_line.cpp rdevent_line.h\
                   rdclock.cpp rdclock.h\
                   rdgrid.cpp rdgrid.h\
                   rdpanel_button.cpp rdpanel_button.h\
                   rdbutton_panel.cpp rdbutton_panel.h\
                   rdbutton_dialog.cpp rdbutton_dialog.h\
                   rdsound_panel.cpp rdsound_panel.h\
                   rddatedecode.cpp rddatedecode.h\
                   rdconfig.cpp rdconfig.h\
                   rddebug.cpp rddebug.h\
                   rdadd_log.cpp rdadd_log.h\
                   rdtextvalidator.cpp rdtextvalidator.h\
                   rdgroup_list.cpp rdgroup_list.h\
                   rdreport.cpp rdreport.h\
                   export_deltaflex.cpp export_textlog.cpp\
                   export_bmiemr.cpp export_technical.cpp\
                   export_soundex.cpp export_radiotraffic.cpp\
                   rdmarker_button.cpp rdmarker_button.h\
                   rdmarker_edit.cpp rdmarker_edit.h\
                   rdcombobox.cpp rdcombobox.h\
                   rdexception_dialog.cpp rdexception_dialog.h\
                   rdcmd_cache.cpp rdcmd_cache.h\
                   rdexport_settings_dialog.cpp rdexport_settings_dialog.h\
                   rdescape_string.cpp rdescape_string.h\
                   rdlogedit_conf.cpp rdlogedit_conf.h\
                   rdwavepainter.cpp rdwavepainter.h\
                   rdgain_envelope.cpp rdgain_envelope.h\
                   rdcmd_switch.cpp rdcmd_switch.h\
                   rdadd_cart.cpp rdadd_cart.h\
                   rdlist_groups.cpp rdlist_groups.h\
                   rdevent_player.cpp rdevent_player.h\
                   rdedit_audio.cpp rdedit_audio.h\
                   rdimport_audio.cpp rdimport_audio.h\
                   rdtextfile.cpp rdtextfile.h\
                   rddbheartbeat.cpp rddbheartbeat.h\
                   rdsimpleplayer.cpp rdsimpleplayer.h\
                   rdget_ath.cpp rdget_ath.h\
                   rdgetpasswd.cpp rdgetpasswd.h\
                   rdlist_logs.cpp rdlist_logs.h\
                   rdcatch_conf.cpp rdcatch_conf.h\
                   schedruleslist.cpp schedruleslist.h\
                   schedcartlist.cpp schedcartlist.h\
                   rddropbox.cpp rddropbox.h\
                   rdedit_panel_name.cpp rdedit_panel_name.h\
                   rdaudiosettings_dialog.cpp rdaudiosettings_dialog.h\
                   rdaudiosettings.cpp rdaudiosettings.h\
                   rdcddblookup.cpp rdcddblookup.h\
                   rdcddbrecord.cpp rdcddbrecord.h\
                   rdcdplayer.cpp rdcdplayer.h\
                   rdcodetrap.cpp rdcodetrap.h\
                   rdconf.cpp rdconf.h\
                   rddatedialog.cpp rddatedialog.h\
                   rddatepicker.cpp rddatepicker.h\
                   rdgpio.cpp rdgpio.h\
                   rdinstancelock.cpp rdinstancelock.h\
                   rdlabel.cpp rdlabel.h\
                   rdlicense.cpp rdlicense.h\
                   rdlineedit.cpp rdlineedit.h\
                   rdlistselector.cpp rdlistselector.h\
                   rdplaymeter.cpp rdplaymeter.h\
                   rdprofile.cpp rdprofile.h\
                   rdprofileline.cpp rdprofileline.h\
                   rdprofilesection.cpp rdprofilesection.h\
                   rdpushbutton.cpp rdpushbutton.h\
                   rdringbuffer.cpp rdringbuffer.h\
                   rdsegmeter.cpp rdsegmeter.h\
                   rdslider.cpp rdslider.h\
                   rdsocket.cpp rdsocket.h\
                   rdstereometer.cpp rdstereometer.h\
                   rdtimeengine.cpp rdtimeengine.h\
                   rdtimeevent.cpp rdtimeevent.h\
                   rdtransportbutton.cpp rdtransportbutton.h\
                   rdttydevice.cpp rdttydevice.h\
                   rdurl.cpp rdurl.h\
                   rdwavedata.cpp rdwavedata.h\
                   rdwavefile.cpp rdwavefile.h\
                   rdfeed.cpp rdfeed.h\
                   rdpodcast.cpp rdpodcast.h\
                   rdcreateauxfieldstable.cpp rdcreateauxfieldstable.h\
                   rd.h rdpaths.h dbversion.h html_gpl2.cpp gpio.h\
                   rdweb.cpp rdweb.h\
                   rdlivewire.cpp rdlivewire.h\
                   rdlivewiresource.cpp rdlivewiresource.h\
                   rdlivewiredestination.cpp rdlivewiredestination.h\
                   rdmeteraverage.cpp rdmeteraverage.h\
		   rddb.h rddb.cpp\
                   rdfeedlog.cpp rdfeedlog.h\
                   rdintegeredit.cpp rdintegeredit.h\
                   rdintegerdialog.cpp rdintegerdialog.h\
                   rdencoder.cpp rdencoder.h\
                   rdencoderlist.cpp rdencoderlist.h\
                   rdoneshot.cpp rdoneshot.h\
                   rdnownext.cpp rdnownext.h\
                   rdsystem.cpp rdsystem.h\
                   rdcastsearch.cpp rdcastsearch.h\
                   rdformpost.cpp rdformpost.h

nodist_librd_la_SOURCES = moc_rdpasswd.cpp moc_rdripc.cpp moc_rdcae.cpp\
                          moc_rdcardselector.cpp moc_rdgpioselector.cpp\
                          moc_rdcart_dialog.cpp moc_rdadd_cart.cpp\
                          moc_rdcut_dialog.cpp moc_rdplay_deck.cpp\
                          moc_rdcatch_connect.cpp\
                          moc_rdlistview.cpp moc_rdtimeedit.cpp\
                          moc_rdmacro_event.cpp moc_rdpanel_button.cpp\
                          moc_rdbutton_dialog.cpp moc_rdsound_panel.cpp\
                          moc_rdsvc.cpp moc_rdadd_log.cpp\
                          moc_rdmarker_edit.cpp moc_rdcombobox.cpp\
                          moc_rdexception_dialog.cpp\
                          moc_rdexport_settings_dialog.cpp\
                          moc_rdevent_player.cpp moc_rdedit_audio.cpp\
                          moc_rdlist_groups.cpp moc_rdimport_audio.cpp\
                          moc_rdsimpleplayer.cpp moc_rddbheartbeat.cpp\
                          moc_rdget_ath.cpp moc_rdgetpasswd.cpp\
                          moc_schedruleslist.cpp moc_schedcartlist.cpp\
                          moc_rdlist_logs.cpp moc_rdedit_panel_name.cpp\
                          moc_rdaudiosettings_dialog.cpp\
                          moc_rdcddblookup.cpp moc_rdcdplayer.cpp\
                          moc_rdcodetrap.cpp\
                          moc_rddatedialog.cpp moc_rddatepicker.cpp\
                          moc_rdgpio.cpp moc_rdlabel.cpp\
                          moc_rdlicense.cpp moc_rdlineedit.cpp\
                          moc_rdlistselector.cpp moc_rdplaymeter.cpp\
                          moc_rdpushbutton.cpp moc_rdsegmeter.cpp\
                          moc_rdslider.cpp moc_rdsocket.cpp\
                          moc_rdstereometer.cpp moc_rdtimeengine.cpp\
                          moc_rdtransportbutton.cpp moc_rddb.cpp\
                          moc_rdlivewire.cpp moc_rdintegeredit.cpp\
                          moc_rdintegerdialog.cpp\
                          moc_rdoneshot.cpp moc_rdfeed.cpp

librd_la_LDFLAGS =  -lmad -ltag -release $(VERSION)
librd_la_CXXFLAGS =  -I/usr/include/taglib

EXTRA_DIST = lib.pro rdwin32.h rdwin32.cpp rdttydevice_win32.cpp\
             gpl2.html html_gpl2_win32.cpp\
             librd_es.ts librd_fr.ts librd_de.ts librd_nb.ts librd_nn.ts\
             rdpaths.h.in

CLEANFILES = *~ moc_* *.qm
DISTCLEANFILES = rdpaths.h
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
