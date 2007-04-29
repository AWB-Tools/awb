#
# Copyright (C) 2003-2006 Intel Corporation
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

lib_LIBRARIES = libdral.a

libdral_a_SOURCES = \
	src/dralServer.cpp \
	src/dralServerBinary.cpp \
	src/dralServerAscii.cpp \
	src/dralServerImplementation.cpp \
	src/dralStorage.cpp \
	src/dralClientBinary_v0.cpp \
	src/dralClientBinary_v1.cpp \
	src/dralClientBinary_v2.cpp \
	src/dralClientBinary_v3.cpp \
	src/dralClientBinary_v4.cpp \
	src/dralClientAscii_v0_1.cpp \
	src/dralClient.cpp \
	src/dralClientImplementation.cpp \
	src/dralStringMapping.cpp \
	src/dralListenerConverter.cpp \
	src/dralWrite.cpp \
	src/dralRead.cpp \
	src/dralInterface.cpp \
	src/dralDesc.cpp \
	src/dralTar.cpp

src/dralTar.cpp : tar_files/tarFile.tar scripts/makeDralTar.pl
	perl scripts/makeDralTar.pl tar_files/tarFile.tar > src/dralTar.cpp

# Inside ASIM we reuse AGT definitions (UINT64 and so on)
WARNFLAGS = -pedantic -W -Wall -Wcast-align -Wno-unused -Wno-parentheses -Wno-long-long
AM_CXXFLAGS += ${STLPORTINC} -I./include 
# pkgconfig configuration file
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pkgconfig/libdral.pc

# pkgconfig uninstalled configuration file
pkgconfig_uninstalled_data=libdral-uninstalled.pc
pkgconfig_uninstalled_dir=${top_srcdir}/pkgconfig
pkgconfig_uninstalled=${pkgconfig_uninstalled_dir}/${pkgconfig_uninstalled_data}
all-local : ${pkgconfig_uninstalled}
${pkgconfig_uninstalled} : pkgconfig/${pkgconfig_uninstalled_data} ${lib_LIBRARIES}
	${mkinstalldirs} ${pkgconfig_uninstalled_dir}
	${INSTALL_DATA} pkgconfig/${pkgconfig_uninstalled_data} ${pkgconfig_uninstalled}

CLEANFILES=${pkgconfig_uninstalled}

DIST_SUBDIRS=include
SUBDIRS=include
