#
# Copyright (C) 2002-2012 Intel Corporation
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

tool_PROGRAMS= apm-edit

apm_edit_SOURCES = apm_edit.qrc \
                   apm_edit_head.pl \
                   apm_edit.ui \
                   apm_edit.pm \
                   apm_edit_about.ui \
                   apm_edit_about.pm \
                   apm_edit_properties.ui \
                   apm_edit_properties.pm \
                   runlog.ui \
                   runlog.pm \
                   apm_edit_main.pl 

apm-edit$(EXEEXT): apm_edit_head.pl runlog.ui.pm apm_edit_properties.ui.pm apm_edit_about.ui.pm apm_edit.ui.pm apm_edit_main.pl apm_edit_images.pm
	cat apm_edit_head.pl runlog.ui.pm runlog.pm apm_edit_properties.ui.pm apm_edit_properties.pm apm_edit_about.ui.pm apm_edit_about.pm apm_edit.ui.pm apm_edit_images.pm apm_edit.pm apm_edit_main.pl > apm-edit
	chmod +x apm-edit

apm_edit.ui.pm: apm_edit.ui

apm_edit_properties.ui.pm: apm_edit_properties.ui

runlog.ui.pm: runlog.ui

apm_edit_about.ui.pm: apm_edit_about.ui

apm_edit_images.pm: apm_edit.qrc
	prcc4_bin -o apm_edit_images.pm $<

%.ui.pm: %.ui
	sed 's/version=\"3\..\"/version="3.0"/i' $<
	puic4 -o $@ $<

#
# Convert perl documentation to man page and install
#

PERLRUN = perl
MAN1EXT = 1
MANPERM = 644
MAN1DIR=$(mandir)/man1
POD2MAN_EXE = $(PERLRUN) "-MExtUtils::Command::MM" -e pod2man "--"
POD2MAN = $(POD2MAN_EXE) --section=$(MAN1EXT) --perm_rw=$(MANPERM)

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(MAN1DIR)
	$(POD2MAN) apm-edit $(DESTDIR)$(MAN1DIR)/apm-edit.$(MAN1EXT)


CLEANFILES = *.ui.pm apm_edit_images.pm

