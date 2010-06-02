## automake.am
##
## Automake.am for rivendell/rdadmin
##
## (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.60.2.6 2010/01/21 17:11:47 cvs Exp $
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/lib
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib
MOC = @QT_MOC@
CWRAP = ../helpers/cwrap

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

# I18N Stuff
install-exec-local:	
	mkdir -p $(DESTDIR)$(prefix)/share/rivendell
	cp rdadmin_*.qm $(DESTDIR)$(prefix)/share/rivendell

uninstall:
	rm -f $(DESTDIR)$(prefix)/share/rivendell/rdadmin_*.qm

all:	
	@QT_BIN@/lupdate rdadmin.pro
	@QT_BIN@/lrelease rdadmin.pro

bin_PROGRAMS = rdadmin

dist_rdadmin_SOURCES = rdadmin.cpp rdadmin.h\
                       login.cpp login.h\
                       list_users.cpp list_users.h\
                       edit_user.cpp edit_user.h\
                       add_user.cpp add_user.h\
                       list_groups.cpp list_groups.h\
                       edit_group.cpp edit_group.h\
                       rename_group.cpp rename_group.h\
                       add_group.cpp add_group.h\
                       list_svcs.cpp list_svcs.h\
                       edit_svc.cpp edit_svc.h\
                       importfields.cpp importfields.h\
                       add_svc.cpp add_svc.h\
                       list_stations.cpp list_stations.h\
                       edit_station.cpp edit_station.h\
                       add_station.cpp add_station.h\
                       edit_rdlibrary.cpp edit_rdlibrary.h\
                       edit_decks.cpp edit_decks.h\
                       edit_audios.cpp edit_audios.h\
                       help_audios.cpp help_audios.h\
                       edit_ttys.cpp edit_ttys.h\
                       opendb.cpp opendb.h\
                       createdb.cpp createdb.h\
                       mysql_login.cpp mysql_login.h\
                       edit_rdairplay.cpp edit_rdairplay.h\
                       info_dialog.cpp info_dialog.h\
                       view_adapters.cpp view_adapters.h\
                       list_matrices.cpp list_matrices.h\
                       add_matrix.cpp add_matrix.h\
                       edit_matrix.cpp edit_matrix.h\
                       list_endpoints.cpp list_endpoints.h\
                       edit_endpoint.cpp edit_endpoint.h\
                       list_gpis.cpp list_gpis.h\
                       edit_gpi.cpp edit_gpi.h\
                       test_import.cpp test_import.h\
                       autofill_carts.cpp autofill_carts.h\
                       list_hostvars.cpp list_hostvars.h\
                       add_hostvar.cpp add_hostvar.h\
                       edit_hostvar.cpp edit_hostvar.h\
                       edit_backup.cpp edit_backup.h\
                       edit_svc_perms.cpp edit_svc_perms.h\
                       list_reports.cpp list_reports.h\
                       add_report.cpp add_report.h\
                       edit_report.cpp edit_report.h\
                       edit_now_next.cpp edit_now_next.h\
                       edit_user_perms.cpp edit_user_perms.h\
                       edit_vguest_resource.cpp edit_vguest_resource.h\
                       list_vguest_resources.cpp list_vguest_resources.h\
                       edit_rdlogedit.cpp edit_rdlogedit.h\
                       edit_schedcodes.cpp edit_schedcodes.h\
                       list_schedcodes.cpp list_schedcodes.h\
                       add_schedcodes.cpp add_schedcodes.h\
                       list_dropboxes.cpp list_dropboxes.h\
                       edit_dropbox.cpp edit_dropbox.h\
                       edit_rdpanel.cpp edit_rdpanel.h\
                       list_feeds.cpp list_feeds.h\
                       edit_feed.cpp edit_feed.h\
                       add_feed.cpp add_feed.h\
                       list_aux_fields.cpp list_aux_fields.h\
                       edit_aux_field.cpp edit_aux_field.h\
                       add_aux_field.cpp add_aux_field.h\
                       edit_feed_perms.cpp edit_feed_perms.h\
                       list_nodes.cpp list_nodes.h\
                       edit_node.cpp edit_node.h\
                       view_node_info.cpp view_node_info.h\
                       add_encoder.cpp add_encoder.h\
                       list_encoders.cpp list_encoders.h\
                       edit_encoder.cpp edit_encoder.h\
                       edit_nownextplugin.cpp edit_nownextplugin.h\
                       edit_settings.cpp edit_settings.h\
                       globals.h

nodist_rdadmin_SOURCES = moc_rdadmin.cpp moc_login.cpp moc_list_users.cpp\
                         moc_edit_user.cpp moc_add_user.cpp\
                         moc_list_groups.cpp moc_edit_group.cpp\
                         moc_add_group.cpp moc_list_svcs.cpp\
                         moc_edit_svc.cpp moc_add_svc.cpp\
                         moc_importfields.cpp\
                         moc_list_stations.cpp moc_edit_station.cpp\
                         moc_add_station.cpp moc_edit_rdlibrary.cpp\
                         moc_edit_decks.cpp moc_edit_audios.cpp\
                         moc_help_audios.cpp\
                         moc_rename_group.cpp moc_edit_ttys.cpp\
                         moc_mysql_login.cpp moc_edit_rdairplay.cpp\
                         moc_info_dialog.cpp moc_view_adapters.cpp\
                         moc_list_matrices.cpp moc_add_matrix.cpp\
                         moc_edit_matrix.cpp moc_list_endpoints.cpp\
                         moc_edit_endpoint.cpp moc_list_gpis.cpp\
                         moc_edit_gpi.cpp moc_test_import.cpp\
                         moc_autofill_carts.cpp moc_list_hostvars.cpp\
                         moc_add_hostvar.cpp moc_edit_hostvar.cpp\
                         moc_edit_backup.cpp moc_edit_svc_perms.cpp\
                         moc_list_reports.cpp moc_edit_user_perms.cpp\
                         moc_add_report.cpp moc_edit_report.cpp\
                         moc_edit_now_next.cpp moc_edit_vguest_resource.cpp\
                         moc_list_vguest_resources.cpp moc_edit_rdlogedit.cpp\
                         moc_edit_schedcodes.cpp moc_list_schedcodes.cpp \
                         moc_add_schedcodes.cpp moc_list_dropboxes.cpp\
                         moc_edit_dropbox.cpp moc_edit_rdpanel.cpp\
                         moc_list_feeds.cpp moc_edit_feed.cpp\
                         moc_add_feed.cpp moc_list_aux_fields.cpp\
                         moc_edit_aux_field.cpp moc_add_aux_field.cpp\
                         moc_edit_feed_perms.cpp moc_list_nodes.cpp\
                         moc_edit_node.cpp moc_view_node_info.cpp\
                         moc_list_encoders.cpp moc_edit_encoder.cpp\
                         moc_add_encoder.cpp moc_edit_nownextplugin.cpp\
                         moc_edit_settings.cpp

rdadmin_LDADD = -lqui -lrd -lid3 -lFLAC -lsamplerate @LIBVORBIS@
# rdadmin_LDFLAGS = -static

EXTRA_DIST = info_banner1.xpm info_banner2.xpm rdadmin.pro\
             xpm_info_banner1.cpp xpm_info_banner2.cpp\
             rdadmin_es.ts rdadmin_fr.ts rdadmin_de.ts rdadmin_nb.ts\
             rdadmin_nn.ts

CLEANFILES = *~ moc_* *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
