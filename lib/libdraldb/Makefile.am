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

lib_LIBRARIES = libdraldb.a

libdraldb_a_SOURCES = src/Hash6431.cpp \
		      src/Dict2064.cpp \
                    src/StatObj.cpp \
                    src/AMemObj.cpp \
                    src/ItemTagHeap.cpp \
                    src/TagDescVector.cpp \
                    src/StrTable.cpp \
                    src/TrackHeap.cpp \
                    src/TrackVec.cpp \
                    src/TagIdVec.cpp \
                    src/TagVecItemIdx.cpp \
                    src/TagVecDenseItemIdx.cpp \
                    src/TagVecDenseShortItemIdx.cpp \
                    src/TagVecDictionary.cpp \
                    src/TagVecDenseDictionary.cpp \
                    src/TagVecDictionaryNF.cpp \
                    src/TagVecDenseDictionaryNF.cpp \
                    src/LogMgr.cpp \
                    src/DBListener.cpp \
                    src/DBGraphEdge.cpp \
                    src/DBGraphNode.cpp \
                    src/DBGraph.cpp \
                    src/ItemHandler.cpp \
                    src/DralDB.cpp \
                    src/DBConfig.cpp

WARNFLAGS = -ansi -pedantic -W -Wall -Wcast-align -Wformat=2 -Wconversion -Winline -Wno-unused -Wno-parentheses -Wno-long-long
AM_CXXFLAGS += -I./include -I../libdral/include $(QTINCDIR) 

CLEANFILES = *moc*
DIST_SUBDIRS=include
SUBDIRS=include
