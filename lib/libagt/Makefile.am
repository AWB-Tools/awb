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

lib_LIBRARIES = libagt.a

libagt_a_SOURCES =  src/AColorDialog.cpp \
        src/Q2DMAction.cpp \
		src/AnnotationItem.cpp \
		src/ColorMenuItem.cpp \
		src/snapDialog.cpp \
		src/AScrollView.cpp \
		src/AnnotationItemDB.cpp \
		src/HelpWindow.cpp \
		src/snapDialogImpl.cpp \
		src/AnnotationCircle.cpp \
		src/AnnotationLine.cpp \
		src/AnnotationText.cpp \
		src/AnnotationContainer.cpp \
		src/AnnotationAutoBezierArrow.cpp \
		src/Main.cpp \
		src/AnnotationCtrl.cpp \
		src/AnnotationRect.cpp \
		src/bookmark.cpp \
		src/AnnotationFactory.cpp \
		src/BookmarkMgr.cpp \
		src/bookmarkImpl.cpp \
		src/ColumnShadows.cpp \
		src/MDIWindow.cpp \
		src/IconFactory.cpp 

nodist_libagt_a_SOURCES = src/AColorDialog_moc.cpp \
			src/AnnotationItem_moc.cpp \
            src/Q2DMAction_moc.cpp \
			src/snapDialog_moc.cpp \
			src/AScrollView_moc.cpp \
			src/HelpWindow_moc.cpp \
			src/snapDialogImpl_moc.cpp \
			src/AnnotationCircle_moc.cpp \
			src/AnnotationLine_moc.cpp \
			src/AnnotationContainer_moc.cpp \
			src/AnnotationText_moc.cpp \
            src/AnnotationAutoBezierArrow_moc.cpp \
			src/Main_moc.cpp \
			src/AnnotationCtrl_moc.cpp \
			src/AnnotationRect_moc.cpp \
			src/bookmark_moc.cpp \
			src/BookmarkMgr_moc.cpp \
			src/MDIWindow_moc.cpp \
			src/bookmarkImpl_moc.cpp 

libagt_a_DIALOGS= src/bookmark.ui \
		src/snapDialog.ui 

src/%_moc.cpp: include/asim/%.h src/%.cpp
	$(QTMOCDIR)/moc $< -o $@ 

# Note: QT .h files produce tons of warning if -Wall is present
# so we redefine the warning flags to an intermediate level:
WARNFLAGS = -ansi -pedantic -W -Wall -Wcast-align -Wformat=2 -Wconversion -Winline -Wno-unused -Wno-parentheses -Wno-long-long
AM_CXXFLAGS += -I./include/asim $(QTINCDIR) $(OPTFLAGS)

CLEANFILES = src/*moc* src/*.lo
DIST_SUBDIRS=include
SUBDIRS=include
