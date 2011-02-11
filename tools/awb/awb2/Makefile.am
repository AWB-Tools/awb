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

tool_PROGRAMS= awb2

awb2_SOURCES = awb_head.pl \
               colored_list_view_item.pm \
               awb_util.pm \
               awb_main.pl \
               awb_dialog.ui awb_dialog.ui.h \
               awb_about.ui \
               awb_runlog.ui awb_runlog.ui.h

awb2$(EXEEXT): awb_head.pl colored_list_view_item.pm awb_about.ui.pm awb_runlog.ui.pm awb_util.pm awb_dialog.ui.pm awb_main.pl
	cat awb_head.pl colored_list_view_item.pm awb_about.ui.pm awb_runlog.ui.pm awb_util.pm awb_dialog.ui.pm awb_main.pl >awb2
	chmod +x awb2

awb_dialog.ui.pm: awb_dialog.ui awb_dialog.ui.h

awb_about.ui.pm: awb_about.ui

awb_runlog.ui.pm: awb_runlog.ui awb_runlog.ui.h

%.ui.pm: %.ui
	puic -o $@ $<

#
# Convert perl documentation to man page and install
#

PERLRUN = perl
MAN1EXT = 1
MANPERM = 644
MAN1DIR=$(mandir)/man1
POD2MAN_EXE = $(PERLRUN) "-MExtUtils::Command::MM" -e pod2man "--"
POD2MAN = $(POD2MAN_EXE) --section=$(MAN1EXT) --perm_rw=$(MANPERM)

# put in link between old name (awb) and new name (awb2)
install-data-hook:
	(cd $(DESTDIR)$(tooldir); rm -f awb; ln -s awb2 awb)
	$(mkinstalldirs) $(DESTDIR)$(MAN1DIR)
	$(POD2MAN) awb2 $(DESTDIR)$(MAN1DIR)/awb2.$(MAN1EXT)
	$(POD2MAN) awb2 $(DESTDIR)$(MAN1DIR)/awb.$(MAN1EXT)


CLEANFILES = *.ui.pm

