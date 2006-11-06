// ==================================================
//Copyright (C) 2004-2006 Intel Corporation
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
  * @file  AnnotationContainer.h
  * @brief Line annotation tool class.
  */
  
#ifndef _ANNOTATION_CONTAINER_H
#define _ANNOTATION_CONTAINER_H

#include <stdio.h>
#include <math.h>

#include <qptrlist.h>

#include "agt_syntax.h"
#include "AnnotationItem.h"

/**
  * Line annotation tool class.
  * Put long explanation here
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AnnotationContainer : public AnnotationItem
{
    Q_OBJECT
    public:
       /**
        * Function description
        */
        AnnotationContainer (
                       AnnotationItem* parent=NULL
                       );

       /**
        * Function description
        */
        virtual
        ~AnnotationContainer();

        // abstract methods

        virtual void newChild(AnnotationItem* child);
        virtual void detachChild(AnnotationItem* child);

        virtual void setSelectable(bool value);
        virtual void setEditable(bool value);
        virtual void setBrush(QBrush b);
        virtual void setPen(QPen p);
        virtual void setVisible(bool value);
        virtual void setBuilt();
        
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
        void recomputeBBox();
        void applyNewObject(AnnotationItem* ann);
        void reset();
                   
    protected:
        QPtrList<AnnotationItem> children;
        QRect setbbox;
};

#endif

