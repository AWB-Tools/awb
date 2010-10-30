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

# tool programs for libexec
tool_SCRIPTS = awb-batch awb-benchmark awb-run awb-shell plot-shell regression.launcher regression.verifier regression.cleanup sum-simpoints sum-unweighted summarize-stats model-coverage

# pod2man_scripts
pod2man_list = $(tool_SCRIPTS) asimstarter

# install asimstarter in bindir
nodist_bin_SCRIPTS = asimstarter

# asimstarter needs post-install processing
# create soft links to old tool names
# create man files

PERLRUN = perl
MAN1EXT = 1
MANPERM = 644
MAN1DIR=$(mandir)/man1
POD2MAN_EXE = $(PERLRUN) "-MExtUtils::Command::MM" -e pod2man "--"
POD2MAN = $(POD2MAN_EXE) --section=$(MAN1EXT) --perm_rw=$(MANPERM)


install-data-hook: install-man
	@(cd $(DESTDIR)$(bindir); ./asimstarter -install)
	@(cd $(DESTDIR)$(tooldir); $(LN_S) -f awb-batch     asim-batch)
	@(cd $(DESTDIR)$(tooldir); $(LN_S) -f awb-benchmark asim-benchmark)
	@(cd $(DESTDIR)$(tooldir); $(LN_S) -f awb-run       asim-run)
	@(cd $(DESTDIR)$(tooldir); $(LN_S) -f awb-shell     asim-shell)

install-man:
	$(mkinstalldirs) $(DESTDIR)$(MAN1DIR)
	for i in $(pod2man_list); do \
	    $(POD2MAN) $$i $(DESTDIR)$(MAN1DIR)/$$i.$(MAN1EXT); \
	done

# cores should get cleaned
MOSTLYCLEANFILES = core *.core
