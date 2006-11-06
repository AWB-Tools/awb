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
  * @file   bookmarkImpl.h
  * @brief  A QT dialog for bookmark management dialog.
  */

#ifndef BOOKMARKIMPL_H
#define BOOKMARKIMPL_H

#include <qstringlist.h>
#include <qfile.h>

#include "agt_syntax.h"
#include "bookmark.h"

#define BKM_MAGIC_NUMBER 0xfedeedef

/**
  * Just a place holder to bookmark entry information.
  */
class BookContentsClass
{
    public:
        BookContentsClass() {};
        BookContentsClass(int _x, int _y, double _sx, double _sy, QString _desc);
        ~BookContentsClass();

        inline int getX() { return x; }
        inline int getY() { return y; }
        inline double getSX() { return sx; }
        inline double getSY() { return sy; }
        inline QString getDesc() { return desc; }

        void save(QFile* file);
        void load(QFile* file);
        
    protected:
        QString repSpaces(QString str);
        
    private:
        int x;                             ///< bookmark X axis
        int y;                             ///< bookmark Y axis
        double sx;                         ///< bookmark X scalling factor
        double sy;                         ///< bookmark Y scalling factor
        QString desc;                      ///< bookmark description
};


/**
  * @typedef BookContents
  * @brief Pointer to BookContentsClass
  */
typedef BookContentsClass* BookContents;



/**
  * A QT dialog for bookmark management dialog.
  *
  * This is intended basically to reorder, remove and rename bookmarks
  *
  * @warning Not fully operative
  * @todo Implement this dialog!
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class bookmarkImpl : public BookMarkForm
{
    Q_OBJECT

    public:
    
        /**
          * Constructor
          */
        bookmarkImpl (
                     QStringList *mBookmarks, 
                     QWidget* parent = 0,   ///< Parent widget
                     const char* name = 0,  ///< Internal name
                     bool modal = FALSE,    ///< Modal if TRUE (block other windows)
                     WFlags fl = 0          ///< QT Flags (stay on top, etc.)
                     );
    
        /**
          * Destructor
          */
        ~bookmarkImpl();
    
        /**
          * Refresh dialog contents before run
          */
        void refresh();
        
    public slots:
    
        /**
          * Method triggered when "bookmark to up" button is pressed.
          */
        void
        UpButton_clicked();
    
        /**
          * Method triggered when "bookmark to up" button is pressed.
          */
        void
        DownButton_clicked();
    
        /**
          * Method triggered when "remove bookmark" button is pressed.
          */
        void
        RemoveButton_clicked();
    
        /**
          * Method triggered when "close" button is pressed.
          */
        void
        CloseButton_clicked();
    
    private:
        QStringList* booklist;
};

#endif // BOOKMARKIMPL_H
