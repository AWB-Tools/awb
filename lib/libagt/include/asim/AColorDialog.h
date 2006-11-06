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
  * @file AColorDialog.h 
  * @brief Defines AColorDialog and some helper classes for "Office-like" color selection/apply widgets.
  */
#ifndef _ACOLORDIALOG_H
#define _ACOLORDIALOG_H

#include <qwidget.h>
#include <qtoolbutton.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qcolordialog.h>
#include <qpixmap.h>
#include <qlabel.h>

#include "UpdColorInterface.h"
#include "ColorMenuItem.h"

/**
  * @enum StdColors
  * Predefined QT colors for Color Dialog
  */
typedef enum
{
    black,
    white,
    darkGray,
    gray,
    lightGray,
    red,
    green,
    blue,
    cyan,
    magenta,
    yellow,
    darkRed,
    darkGreen,
    darkBlue,
    darkCyan,
    darkMagenta,
    darkYellow,
    NUM_STD_COLORS
} StdColors;

/**
  * Helper class to keep the current (Color Dialog) color rectangle.
  */
class AColorRect : public QLabel
{
    public:

       /**
        * Function description
        */
        inline
        AColorRect (
                   QWidget* parent,
                   QColor
                   );

       /**
        * Function description
        */
        inline void
        setColor (
                 QColor
                 );

       /**
        * Function description
        */
        inline QColor
        getColor (
                 QColor
                 );

       /**
        * Function description
        */
        virtual QSize
        sizeHint() const;

       /**
        * Function description
        */
        inline void
        setNoColor (
                   bool
                   );

    protected:
       /**
        * Function description
        */
        virtual void
        drawContents (
                     QPainter * p
                     );

    private:
        QColor myColor;
        bool noColorFlag;

};

/**
  * Helper class which is a tool-like button used in the Color Dialog
  */
class AToolButton : public QToolButton
{
    public:
       /**
        * Function description
        */
        AToolButton ( QWidget * parent, const char * name = 0 ):
        QToolButton (parent,name){};

    protected:
       /**
        * Function description
        */
        inline bool uses3D () const { return false; }
};

/**
  * QT ad-hoc widget to mimic color selection/apply widget used in office applications.
  * Put long desc. here...
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  *
  * @see UpdColorInterface
  */
class AColorDialog : public QHBox
{
    Q_OBJECT

public:
    /**
     * Function description
     */
    AColorDialog (
                 QColor defColor = black,
                 QWidget *parent = 0,
                 const char *name = 0,
                 QPixmap* icon=NULL
                 );

    /**
     * Function description
     */
    ~AColorDialog ();

    /**
     * Function description
     */
    inline QColor
    getCurrentColor();

    /**
     * Function description
     */
    inline bool
    getNoColor();

    /**
     * Function description
     */
    inline void
    setEnabled (
               bool
               );

    /**
     * Function description
     */
    inline int
    registerApplyMethod (
                        UpdColorIterface*
                        );

public slots:
    /**
     * Function description
     */
    void
    colorApply ();

    /**
     * Function description
     */
    void
    popupShow  ();

    /**
     * Function description
     */
    void
    popMenuActivated (
                     int id
                     );

    /**
     * Function description
     */
    void
    stdColorActivated();

private:
    static QColor stdColors[];
    static int static_id;


protected:
    AToolButton    *bt_app;
    AToolButton    *bt_pop;
    QPopupMenu    *pop;

private:
    ColorMenuItem* stdColorItems[NUM_STD_COLORS];
    int lastActivatedItem;
    QColor currentColor;
    int noColorId;
    int moreColorsId;
    bool noColorFlag;
    UpdColorIterface* myInterfaceObj;
    QPixmap downIcon;
    AColorRect* acr;
    int myId;

};


// ---------------------------------------------------------
// ---------------------------------------------------------
// AColorDialog methods
// ---------------------------------------------------------
// ---------------------------------------------------------
void
AColorDialog::setEnabled(bool v)
{
    bt_app->setEnabled(v);
    //bt_pop->setEnabled(v);
}

bool 
AColorDialog::getNoColor()
{
    return noColorFlag;
}

QColor 
AColorDialog::getCurrentColor()
{
    return currentColor; 
}

int 
AColorDialog::registerApplyMethod(UpdColorIterface* obj)
{
    myInterfaceObj = obj;
    return (myId);
}


// ---------------------------------------------------------
// ---------------------------------------------------------
// AColorRect methods
// ---------------------------------------------------------
// ---------------------------------------------------------
AColorRect::AColorRect(QWidget* parent, QColor c) : QLabel(parent)
{
    myColor = c;
    noColorFlag = false;
}

void 
AColorRect::setColor(QColor c)
{
    myColor = c;
    repaint();
}

QColor
AColorRect::getColor(QColor)
{
    return myColor; 
}

void
AColorRect::setNoColor(bool v)
{
    noColorFlag = v; 
}

#endif


