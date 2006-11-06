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
  * @file  AnnotationAutoBezierArrow.h
  * @brief Line annotation tool class.
  */
  
#ifndef _ANNOTATION_AB_ARROW__H
#define _ANNOTATION_AB_ARROW__H

#include <stdio.h>
#include <math.h>

#include "agt_syntax.h"
#include "AnnotationItem.h"

/**
  * Line annotation tool class.
  * Put long explanation here
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AnnotationAutoBezierArrow : public AnnotationItem
{
    Q_OBJECT
    public:
       /**
        * Function description
        */
        AnnotationAutoBezierArrow (
                       double vx1,
                       double vy1,
                       double vx2,
                       double vy2,
                       AnnotationItem* parent=NULL
                       );

       /**
        * Function description
        */
        virtual
        ~AnnotationAutoBezierArrow();

        // abstract methods

       /**
        * Function description
        */
        virtual
        AnnotationItem*
        clone();

       /**
        * Function description
        */
        virtual void
        clone (
              AnnotationItem*
              );

       /**
        * Function description
        */
        virtual void
        cloneGeometry (
                      AnnotationItem* item
                      );

       /**
        * Function description
        */
        virtual void
        move (
             double vx,
             double vy,
             double dis
             );

       /**
        * Function description
        */
        virtual void
        move (
             double x,
             double y
             );

       /**
        * Function description
        */
        virtual void
        moveNode (
                 double nx,
                 double ny,
                 double x,
                 double y,
                 double scf_x,
                 double scf_y
                 );

       /**
        * Function description
        */
        virtual bool
        innerPoint (
                   double x,
                   double y,
                   double scf_x,
                   double scf_y
                   );

       /**
        * Function description
        */
        virtual bool
        innerNodePoint (
                       double x,
                       double y,
                       double scf_x,
                       double scf_y
                       );

       /**
        * Function description
        */
        virtual void
        boundingRect (
                     double*,
                     double*,
                     double*,
                     double*
                     ) const;

       /**
        * Function description
        */
        virtual void
        draw (
             QPainter*,
             double ,
             double
             );

       /**
        * Function description
        */
        virtual void
        drawRubber (
                   int px,
                   int py,
                   int x,
                   int y,
                   QPainter* p
                   );

       /**
        * Function description
        */
        virtual void
        mouseWorldPressEvent (
                             int vx,
                             int vy,
                             double wx,
                             double wy
                             );

       /**
        * Function description
        */
        virtual void
        mouseWorldReleaseEvent (
                               int vx,
                               int vy,
                               double wx,
                               double wy
                               );

       /**
        * Function description
        */
        virtual void
        mouseWorldMoveEvent (
                            int vx,
                            int vy,
                            double wx,
                            double wy
                            );

       /**
        * Function description
        */
        virtual void
        snapToGrid (
                   int gridSize,
                   bool onlyCorner
                   );

    protected:
        double x1,x2,y1,y2;
        QPointArray parray;
        double arrx0,arrx1,arry0,arry1;
        static BYTE flipping;
};

#endif
