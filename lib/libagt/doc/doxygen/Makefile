#
# Copyright (C) 2005-2006 Intel Corporation
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

# makefile to produce the documentation using doxygen
# Federico Ardanaz 2005.02.15

all: html

html: agt.doxygen
	doxygen agt.doxygen

update: agt.doxygen
	doxygen -u agt.doxygen

clean:
	rm -rf ./doxygen_output
	mkdir ./doxygen_output
	 	
