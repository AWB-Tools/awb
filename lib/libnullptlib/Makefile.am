lib_LIBRARIES = libnullptlib.a

libnullptlib_a_SOURCES = \
	src/ptlib.cpp

# Inside ASIM we reuse AGT definitions (UINT64 and so on)
WARNFLAGS = -pedantic -W -Wall -Wcast-align -Wno-unused -Wno-parentheses -Wno-long-long
AM_CXXFLAGS += ${STLPORTINC} -I./include 
# pkgconfig configuration file
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pkgconfig/libnullptlib.pc

# pkgconfig uninstalled configuration file
pkgconfig_uninstalled_data=libnullptlib-uninstalled.pc
pkgconfig_uninstalled_dir=${top_srcdir}/pkgconfig
pkgconfig_uninstalled=${pkgconfig_uninstalled_dir}/${pkgconfig_uninstalled_data}
all-local : ${pkgconfig_uninstalled}
${pkgconfig_uninstalled} : pkgconfig/${pkgconfig_uninstalled_data} ${lib_LIBRARIES}
	${mkinstalldirs} ${pkgconfig_uninstalled_dir}
	${INSTALL_DATA} pkgconfig/${pkgconfig_uninstalled_data} ${pkgconfig_uninstalled}

CLEANFILES=${pkgconfig_uninstalled}

DIST_SUBDIRS=include
SUBDIRS=include
