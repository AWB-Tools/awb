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
  * @file   UpdColorInterface.h
  * @brief  Helper interface for AColorDialog used to register target methods for internal events.
  */
  
#ifndef _UPDCOLORITERFACE_H
#define _UPDCOLORITERFACE_H

#include "agt_syntax.h"

/**
  * @interface UpdColorInterface
  * @brief Helper interface for AColorDialog used to register target methods for internal events.
  *
  * The idea is that you want to be informed when the "default" color changes or the
  * user pressed the "apply" button. Usually the apply means do something with currently
  * selected object.
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  *
  * @see AColorDialog
  */
class UpdColorIterface
{
    public:

    /**
      * Triggered method when default color changes.
      */
        virtual void
        colorChanged (
                     QColor color,    ///< the new default color (or undefined if transparent)
                     bool nocolor,    ///< when transparent color is selected this parameter is true
                     int id           ///< Widget ID (pen, brush, text ? ...)
                     ) = 0;

    /**
      * Triggered method when apply button is pressed.
      */
        virtual void
        colorApply (
                   QColor color,           ///< The color to be applied (or undefined if transparent)
                   bool nocolor,           ///< when transparent color is selected this parameter is true
                   int id                  ///< Widget ID (pen, brush, text ? ...)
                   ) = 0;
};

#endif

