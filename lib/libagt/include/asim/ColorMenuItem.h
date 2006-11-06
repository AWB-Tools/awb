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
  * @file   ColorMenuItem.h
  * @brief  Helper class to color selection/apply widgets.
  */
  
#ifndef _COLORMENUITEM_H
#define _COLORMENUITEM_H

#include <qcolor.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qmenudata.h>

#include "agt_syntax.h"

/**
  * Helper class to color selection/apply widgets.
  *
  * This is a specialization of QCustomMenuItem class.
  * @see http://doc.trolltech.com/3.0/qcustommenuitem.html
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class ColorMenuItem : public QCustomMenuItem
{
    public:

    /**
      * Constructor
      */
        ColorMenuItem (
                      QColor                ///< Initial color
                      );

    /**
      * Destructor
      */
        virtual
        ~ColorMenuItem(){};

    /**
      * Function description
      */
        virtual void
        paint (
              QPainter * p,                 ///< Painter where to draw on
              const QColorGroup & cg,       ///< Used by QT for skins...
              bool act,                     ///< TRUE if active item
              bool enabled,                 ///< TRUE if enabled item
              int x,                        ///< x coord. for drawing the item     
              int y,                        ///< y coord. for drawing the item
              int w,                        ///< width for drawing the item
              int h                         ///< height for drawing the item
              );

    /**
      * Inherited redefined QT method for managing widget size.
      * @see http://doc.trolltech.com/3.0/qwidget.html
      */
        virtual QSize
        sizeHint ();

    /**
      * Sets the item color
      */
        inline void
        setColor (
                 QColor c                             ///< New item color
                 );

    /**
      * Gets back item color
      */
        inline QColor       /// @return item color
        getColor();

    protected:
        QColor myColor;
};


void
ColorMenuItem::setColor (QColor c)
{
    myColor = c;
}

QColor
ColorMenuItem::getColor()
{
    return myColor;
}

#endif
