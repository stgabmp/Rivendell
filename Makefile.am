## automake.am
##
## Top level automake.am for Rivendell
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.60.2.6 2009/08/03 13:33:10 cvs Exp $
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

if PAM_RD_AM
  PAM_RD_OPT = pam_rd
endif
if HPI_RD_AM
  HPI_RD_OPT = rdhpi
endif
if ALSA_RD_AM
  ALSACONFIG_RD_OPT = rdalsaconfig
endif
SUBDIRS = icons helpers toolame $(HPI_RD_OPT) lib rlm scripts cae conf docs \
          debian xdg rdadmin ripcd rdlogin rdlibrary rdcatch rdcatchd \
          rdlogedit rdlogmanager rdpanel rdairplay rdcastmanager utils web \
          tests importers $(PAM_RD_OPT)

install-exec-local:	
	./install-init.sh $(DESTDIR)$(prefix)

install-debian:	
	touch ./building-debian
	make install
	rm -f ./building-debian

uninstall-exec:
	rm -f /etc/init.d/rivendell
	rm -f /etc/sysconfig/rivendell

rpm:	dist
	cp rivendell-$(VERSION).tar.gz $(RPM_ROOT)/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux rivendell.spec
	mv $(RPM_ROOT)/RPMS/$(ARCH)/rivendell-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/RPMS/$(ARCH)/rivendell-importers-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/RPMS/$(ARCH)/rivendell-web-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/RPMS/$(ARCH)/rivendell-devel-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/SRPMS/rivendell-$(VERSION)-@RPM_RELEASE@.src.rpm $(top_srcdir)/
	rm $(RPM_ROOT)/SOURCES/rivendell-$(VERSION).tar.gz
	rm -rf $(RPM_ROOT)/BUILD/rivendell-$(VERSION)

slack:	all
	./make_slack

slax:	all
	rm -f rivendell-$(VERSION)-$(ARCH)-$(RPM_RELEASE).mo
	./make_slack
	tgz2mo rivendell-$(VERSION)-$(ARCH)-$(RPM_RELEASE).tgz rivendell-$(VERSION)-$(ARCH)-$(RPM_RELEASE).mo

zip:	dist
	tar -zxf rivendell-@VERSION@.tar.gz
	zip -r rivendell-@VERSION@.zip rivendell-@VERSION@
	rm -rf rivendell-@VERSION@

doxygen: 
	cd lib && doxygen doxygen.conf 
	cd cae && doxygen doxygen.conf &
	cd rdairplay && doxygen doxygen.conf &
	cd rdadmin && doxygen doxygen.conf &
	cd rdcatch && doxygen doxygen.conf &
	cd rdcatchd && doxygen doxygen.conf &
	cd rdlibrary && doxygen doxygen.conf &
	cd rdlogedit && doxygen doxygen.conf &
	cd rdlogin && doxygen doxygen.conf &
	cd rdlogmanager && doxygen doxygen.conf &
	cd ripcd && doxygen doxygen.conf &

EXTRA_DIST = autogen.sh SupportedCards ToDo rivendell-suse.in\
             rivendell.spec.in rivendell.sys rivendell.pro rivendell-gentoo\
             install-init.sh make_slack.in slack-desc.in slack_doinst.sh\
             CODINGSTYLE PACKAGE_VERSION build_win32.bat rivendell.ism\
             get_target.sh get_distro.sh

CLEANFILES = *~ *.tar.gz *.tgz *.mo *.rpm *.zip
DISTCLEANFILES = rivendell.spec rivendell make_slack slack-desc ltmain.sh config.guess config.sub
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz\
                       config.guess config.status config.sub install-sh\
                       libtool ltmain.sh missing mkinstalldirs depcomp\
                       rivendell.spec rivendell-suse
