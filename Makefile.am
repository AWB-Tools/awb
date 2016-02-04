#
# Copyright (C) 2002-2006 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#

SUBDIRS = lib libperl tools

#
# we have non-standard sub-targets
#
.phony:  install-indep install-arch install-exec-hook install-config install-etc install-package install-src print-release-name

#
# Targets used when making a .deb file
#    WARNING: These targets assume that the $(DESTDIR) starts empty!!!
#

install-indep:

#
# Do install and
#   - install application menu stuff
#
install-arch: install
	$(INSTALL) -D -m 644 share/awb.desktop       $(DESTDIR)$(datadir)/applications/awb.desktop
	$(INSTALL) -D -m 644 share/awb.svg           $(DESTDIR)$(datadir)/icons/awb.svg
	$(INSTALL) -D -m 644 share/apm-edit.desktop  $(DESTDIR)$(datadir)/applications/apm-edit.desktop
	$(INSTALL) -D -m 644 share/apm-edit.svg      $(DESTDIR)$(datadir)/icons/apm-edit.svg


#
# Target for install
#

install-exec-hook: install-config install-etc
	@echo ""
	@echo "*******************************************************"
	@echo ""
	@echo "Remember to do the following:"
	@echo ""
	@echo "    Update PATH to include \"$(DESTDIR)$(bindir)\""
	@echo "    Update MANPATH to include \"$(DESTDIR)$(mandir)\""
	@echo "    Update PERL5LIB to include \"$(DESTDIR)$(libdir)/perl5/site_perl\""
	@echo ""
	@echo "*******************************************************"
	@echo ""
	@echo "Done."

#
# This creates a simple default workspace for this version of asim
#
install-config:
	$(INSTALL) -D -m 644 share/awb.config.template  $(DESTDIR)$(configdir)/awb.config
	$(mkinstalldirs)  $(DESTDIR)$(configdir)/src/awb


#
# This installs the /etc files
#   Note: don't do this if creating a .deb file
#
INSTALL_RC = $(top_srcdir)/scripts/install-rcfile

install-etc:
	$(INSTALL) -D -m 644 $(srcdir)/etc/asim.pack $(DESTDIR)$(sysconfdir)/asim/asim.pack
	cd $(srcdir)/etc/repositories.d; \
	for i in * ; do \
	    $(INSTALL) -D -m 644 $$i $(DESTDIR)$(sysconfdir)/asim/repositories.d/$$i; \
	done
	$(INSTALL) -D -m 644 $(srcdir)/etc/awb-shell $(DESTDIR)$(sysconfdir)/bash_completion.d/awb-shell
	export PERL5LIB=$(top_srcdir)/libperl/Asim/lib; \
	$(INSTALL_RC) -D -m 644 $(srcdir)/etc/asimrc $(DESTDIR)$(sysconfdir)/asim/asimrc

#
# This horrible sequence copies just the modules and other files needed to build 
# models to $(codedir)
# It also removes the .svn and/or .git subdirectories in the installed tree.
#
install-package:
	@rm -rf  $(DESTDIR)$(codedir)
	$(mkinstalldirs)  $(DESTDIR)$(codedir)
	rsync -av --exclude .svn --exclude .git admin   $(DESTDIR)$(codedir)
	rsync -av --exclude .svn --exclude .git base    $(DESTDIR)$(codedir)
	rsync -av --exclude .svn --exclude .git modules $(DESTDIR)$(codedir)
	rsync -av --exclude .svn --exclude .git scripts $(DESTDIR)$(codedir)

#
# This horrible sequence copies the source to $(codedir) and
# tries to make the ownership and permissions reasonable
# It also removes the .svn and/or .git subdirectories in the installed tree.
#
install-src:
	$(mkinstalldirs)  $(DESTDIR)$(codedir)
	rm -rf  $(DESTDIR)$(codedir)/*
	cp -r . $(DESTDIR)$(codedir)
	chgrp --recursive --reference $(datadir) $(DESTDIR)$(codedir)
	if [ `ls -ld $(datadir) | cut -b 6` = w ]; then chmod -R g+w $(DESTDIR)$(codedir); fi
	make -C $(DESTDIR)$(codedir) clean
	find    $(DESTDIR)$(codedir) -name .svn -depth -exec rm -rf {} \;
	find    $(DESTDIR)$(codedir) -name .git -depth -exec rm -rf {} \;

#
# Install the source tree, and make it world-readable
#
install-src-public: install-src
	chmod -R o+r $(DESTDIR)$(codedir)
	find         $(DESTDIR)$(codedir) -type d -exec chmod o+x {} \;


#
# Utility target to print the release being built
#
print-release-name:
	@echo ${release}
