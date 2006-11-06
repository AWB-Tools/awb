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

lib_LIBRARIES = libasim.a

libasim_a_SOURCES =	src/mesg.cpp \
			src/profile.cpp \
			src/stripchart.cpp \
			src/utils.cpp \
			src/event.cpp \
			src/disasm.cpp \
			src/module.cpp \
			src/atoi.cpp \
			src/xmlout.cpp \
			src/registry.cpp \
			src/thread.cpp \
			src/xcheck.cpp \
			src/except.cpp \
			src/stackdump.cpp \
			src/trace.cpp \
			src/ioformat.cpp \
			src/port.cpp \
			src/stateout.cpp \
			src/trackmem.cpp \
			src/arch_register.cpp \
			src/clockserver.cpp \
			src/clockable.cpp \
			src/atomic.cpp \
			src/smp.cpp \
			src/regexobj.cpp \
			src/cache_manager.cpp 

# Inside ASIM we reuse AGT definitions (UINT64 and so on)
WARNFLAGS = -Wall -W -Wpointer-arith -Wredundant-decls -Wcast-align \
            -Wno-long-long -Wno-unused -Wno-unknown-pragmas

AM_CXXFLAGS += ${STLPORTINC} -I./include -I../libdral/include ${XML_CFLAGS} \
               -DASIM_ENABLE_EVENTS -DASIM_ENABLE_TRACE @asim_pthreads@

AM_LDFLAGS += ${XML_LFLAGS}

# pkgconfig configuration file
pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = pkgconfig/libasim.pc

# pkgconfig uninstalled configuration file
pkgconfig_uninstalled_data=libasim-uninstalled.pc
pkgconfig_uninstalled_dir=${top_srcdir}/pkgconfig
pkgconfig_uninstalled=${pkgconfig_uninstalled_dir}/${pkgconfig_uninstalled_data}
all-local : ${pkgconfig_uninstalled}
${pkgconfig_uninstalled} : pkgconfig/${pkgconfig_uninstalled_data} ${lib_LIBRARIES}
	${mkinstalldirs} ${pkgconfig_uninstalled_dir}
	${INSTALL_DATA} pkgconfig/${pkgconfig_uninstalled_data} ${pkgconfig_uninstalled}

CLEANFILES=${pkgconfig_uninstalled}

DIST_SUBDIRS=include
SUBDIRS=include
