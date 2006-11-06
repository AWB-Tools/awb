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
  * @file   AnnotationFactory.h
  * @brief  AnnotationFactory class know all the annotation subclasses and create such objects.
  */
  
#ifndef __ANNOTATIONFACTORY_H
#define __ANNOTATIONFACTORY_H


#include "agt_syntax.h"

#include "AnnotationItem.h"
#include "AnnotationContainer.h"
#include "AnnotationLine.h"
#include "AnnotationRect.h"
#include "AnnotationCircle.h"
#include "AnnotationAutoBezierArrow.h"
#include "AnnotationText.h"

/**
  * @typedef AnnToolType
  * Currently known annotation item subclasses.
  */
typedef enum
{
    ANN_LINE,
    ANN_RECTANGLE,
    ANN_CIRCLE,
    ANN_AUTOBEZIER,
    ANN_TEXT
} AnnToolType;

/**
  * This class know all the annotation subclasses and create such objects.
  * Put long explanation here
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  * @see AnnToolType
  */
class AnnotationFactory
{
    public:

       /**
        * Function description
        */
        static AnnotationItem*
        createAnnItem (
                      AnnToolType
                      );
};

#endif
