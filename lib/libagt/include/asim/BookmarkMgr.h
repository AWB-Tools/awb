// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file   BookmarkMgr.h
  * @brief  BookmarkMgr class wraps all the functionality related with event matrix bookmarks
  */

#ifndef BookMgr_H
#define BookMgr_H

#include <stdio.h>

#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qinputdialog.h>

#include "agt_syntax.h"
#include "AScrollView.h"
#include "bookmarkImpl.h"

/**
  * This class wraps all the functionality related with event matrix bookmarks.
  *
  * We keep just position and scalling factors (and a description) on each
  * entry. How to save/restore bookmark information is not managed by this
  * class (depends on AGT concrete application).
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class BookmarkMgr : public QObject
{
    Q_OBJECT

    public:

        /**
          * Constructor
          */
        BookmarkMgr (
                        QWidget* parent,          ///< Parent widget
                        QPopupMenu* pmenu,        ///< Reference to the popup menu where to add bookmark entries
                        AScrollView* asv = NULL
                     );

        /**
          * Destructor
          */
        ~BookmarkMgr();

        /**
          * Method to add a bookmark in a given point and scalling factor.
          */
        void
        addBookmark (
                        double sx,   ///< horizontal scalling factor
                        double sy,   ///< vertical scalling factor
                        int x,       ///< x axis
                        int y        ///< y axis
                        );
        /**
          * Method to add a bookmark in a given point a scalling factor and a string
          */
        void
        addBookmark (
                      double sx,       ///< horizontal scalling factor
                      double sy,       ///< vertical scalling factor
                      int x,           ///< x axis
                      int y,           ///< y axis
                      QString text     ///< bookmark description
                      );

        /**
          * Import bookmarks from a file
          */
        void importBookmarks();
        void importBookmarks(QFile* file);

        /**
          * Export bookmarks to a file
          */
        void exportBookmarks();
        void exportBookmarks(QFile* file);

        /**
          * Show modal manager dialog
          */
        void
        runDialog();

        /**
          * clean up method
          */
        void reset();

        void setAScrollView (AScrollView* asv);
        AScrollView* getAScrollView();
         
    protected:
        BookContents findOnMap(QString desc,QMap<int, BookContents>* map);
        
    private slots:

        /**
          * This method is triggered when a bookmark is choosen.
          */
        void 
        bookmChosen (
                     int          ///< bookmark index ('vector' position)
                    );

    private:

        QWidget* myParentWidget;
        QPopupMenu* myMenu;
        AScrollView* myAsv;

        QStringList bookmarks;
        QMap<int, BookContents> mBookmarks;

        // management dialog
        bookmarkImpl* bmd;


};


#endif
