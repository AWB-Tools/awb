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
  * @file  HelpWindow.h
  * @brief This is a small HTML-based help viewer with basic navigation functionality.
  */
  
#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>

#include "agt_syntax.h"

class QComboBox;
class QPopupMenu;

/**
  * This is a small HTML-based help viewer with basic navigation functionality.
  * We are able to keep the history and explicit added bookmarks.
  *
  * @todo This is based on a QT-example source, review copyright implications of this !?
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class HelpWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
      * Constructor
      */
    HelpWindow (
               const QString& home_,    ///< Starting "home page"
               const QStringList& path,
               QWidget* parent = 0,
               const char *name=0
               );

    /**
      * Destructor
      */
    ~HelpWindow();

public:
    inline QTextBrowser* getTextBrowser() { return browser; }

private slots:

    /**
      * Switches on and off the back arrow
      */
    void
    setBackwardAvailable (
                         bool
                         );

    /**
      *  Switches on and off the forward arrow
      */
    void
    setForwardAvailable (
                        bool
                        );

    /**
      * This method is triggered when for some reason the path
      * text field changes.
      */
    void
    textChanged();

    /**
      * You can keep multiple windows with different view.
      * This method is triggered when a new view must be
      * created.
      */
    void
    newWindow();

    /**
      * This method is triggered when the current view should be printed out.
      */
    void
    print();

    /**
      * This method is triggered when some "old" path is selected from
      * the related combo box widget.
      */
    void
    pathSelected (
                 const QString &
                 );

    /**
      * This method is triggered when some "old" path is selected from
      * the history list.
      */
    void
    histChosen (
               int
               );

    /**
      * This method is triggered when some "old" path is selected from
      * the bookmark list.
      */
    void
    bookmChosen (
                int
                );

    /**
      * Makes current position gets saved as a (help) bookmark.
      */
    void
    addBookmark();

private:

    /**
      * Try to read History from current directory
      */
    void
    readHistory();

    /**
      * Try to read Bookmakrs from current directory
      */
    void
    readBookmarks();

private:
    QTextBrowser* browser;
    QComboBox *pathCombo;
    int backwardId, forwardId;
    QString selectedURL;
    QStringList history, bookmarks;
    QMap<int, QString> mHistory, mBookmarks;
    QPopupMenu *hist, *bookm;
    QWidget* myParent;

};

#endif

