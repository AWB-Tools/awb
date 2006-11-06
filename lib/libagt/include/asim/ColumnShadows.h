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
  * @file  ColumnShadows.h
  * @brief Defines the class AScrollView and the enumeration PointerType.
  */

#ifndef _COLUMNSHADOWS_H
#define _COLUMNSHADOWS_H

// AGT
#include "agt_syntax.h"

// QT
#include <qfile.h>
#include <qtextstream.h>
#include <qcolor.h>
#include <qmessagebox.h>

#define SHD_MAGIC_NUMBER 0xfedefed0

class ColumnShadows
{
    public:

        /**
          * Constructor
          */
        ColumnShadows (
                      int numColumns         ///< Number of columns that can be shaded
                    );
        /**
          * Destructor
          */
        virtual
        ~ColumnShadows();

        virtual void resize(int newsize);

        virtual void shadeColumn(INT32 col,bool shade);
        virtual bool getShadeFlag(INT32 col);

        virtual void clear();
        virtual INT32 countShadowedColumns();

        virtual void saveShadows(QFile* file);
        virtual void loadShadows(QFile* file);

        virtual int getNumCols() { return cols; }
        
    protected:
        int    cols;
        BYTE*  shadeFlagVector;

};
#endif


