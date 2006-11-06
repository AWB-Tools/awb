/*
 *Copyright (C) 2003-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
  * @file  AnnotationFactory.cpp
  */

#include "AnnotationFactory.h"

AnnotationItem* AnnotationFactory::createAnnItem(AnnToolType type)
{
    AnnotationItem* result = NULL;
    switch (type)
    {
        case ANN_LINE:
        result = new AnnotationLine(0,0,0,0);
        Q_ASSERT(result!=NULL);
        break;

        case ANN_RECTANGLE:
        result = new AnnotationRect(0,0,0,0);
        Q_ASSERT(result!=NULL);
        break;

        case ANN_CIRCLE:
        result = new AnnotationCircle(0,0,0,0);
        Q_ASSERT(result!=NULL);
        break;

        case ANN_AUTOBEZIER:
        result = new AnnotationAutoBezierArrow(0,0,0,0);
        Q_ASSERT(result!=NULL);
        break;

        case ANN_TEXT:
        result = new AnnotationText(0,0,QString::null);
        Q_ASSERT(result!=NULL);
        break;
    }
    return (result);
}

