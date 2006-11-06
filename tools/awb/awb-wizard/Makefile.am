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

tool_PROGRAMS= awb-wizard

awb_wizard_SOURCES = awb-wizard.pro       \
                     awb_wizard_head.pl   \
                     awb_wizard.ui        \
                     awb_wizard.ui.h      \
                     awb_wizard_about.ui  \
                     awb_wizard_write.pl  \
                     awb_wizard_images.pm \
                     awb_wizard_main.pl   \
                     ${IMAGES}


IMAGES= images/filenew.png      \
        images/fileopen.png     \
        images/filesave.png     \
        images/filebuild.png    \
        images/whatsthis.png




awb-wizard$(EXEEXT): awb_wizard_head.pl   \
                     awb_wizard_about.ui.pm  \
                     awb_wizard.ui.pm        \
                     awb_wizard_write.pl  \
                     awb_wizard_images.pm \
                     awb_wizard_main.pl
	cat awb_wizard_head.pl awb_wizard_about.ui.pm awb_wizard.ui.pm awb_wizard_write.pl awb_wizard_images.pm awb_wizard_main.pl >awb-wizard
	chmod +x awb-wizard


awb_wizard.ui.pm: awb_wizard.ui awb_wizard.ui.h

awb_wizard_about.ui.pm: awb_wizard_about.ui

awb_wizard_images.pm: awb-wizard.pro ${IMAGES}
	puic -o awb_wizard_images.pm -embed awb-wizard.pro ${IMAGES}

%.ui.pm: %.ui
	puic -o $@ $<

CLEANFILES = *.ui.pm awb_wizard_images.pm
