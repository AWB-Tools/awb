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
tool_SCRIPTS = awb-shell plot-shell sum-simpoints sum-unweighted summarize-stats model-coverage

# install asimstarter in bindir
nodist_bin_SCRIPTS = asimstarter

# asimstarter needs post-install processing
# create soft links to old tool names
install-data-hook:
	@(cd $(DESTDIR)$(bindir); ./asimstarter -install)
	@(cd $(DESTDIR)$(tooldir); $(LN_S) -f awb-shell asim-shell)

# cores should get cleaned
MOSTLYCLEANFILES = core *.core
