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

tool_PROGRAMS = amc
amc_SOURCES = amc.cpp
AM_CPPFLAGS = -I$(top_srcdir)/lib
if X86_64_LIBTOOL_HACK
amc_LDADD = $(top_builddir)/lib/libawb/.libs/libawb.a -lpopt
else
amc_LDADD = $(top_builddir)/lib/libawb/libawb.la -lpopt
endif

EXTRA_DIST = doxygen.config
#-----------------------------------------------------------------------------
# Generate documentation with Doxygen
#-----------------------------------------------------------------------------
.PHONY: dox
dox: doxygen.config
	doxygen doxygen.config
	@echo "Doxygen output is at `pwd`/dox/html/index.html"

clean-local:
	@rm -rf dox

# maybe explicitly link this, avoiding use of broken libtool:
if X86_64_LIBTOOL_HACK
amc: $(amc_OBJECTS)
	$(CXXLD) -o amc $(ARCHFLAGS) $(amc_OBJECTS) $(amc_LDADD)
endif
