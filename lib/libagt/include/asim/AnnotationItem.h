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
  * @file AnnotationItem.h
  * @brief AnnotationItem class is the root of the annotation items hierarchy.
  */
  
#ifndef _ANNOTATION__H
#define _ANNOTATION__H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <qobject.h>
#include <qbrush.h>
#include <qpen.h>
#include <qrect.h>
#include <qpainter.h>

#include "agt_syntax.h"

// tolerance for selections
#ifndef DELTA_MARGIN
#define DELTA_MARGIN 2.6
#endif

// fp-error tolerance used in comparisions (for selection)
#ifndef FP_TOLERANCE
#define FP_TOLERANCE 0.6
#endif

// modifing nodes size (viewport system)
#define NODE_SIZE 7.0;

// distance between object boundaries and node selectors (for rectangles, arcs,..)
//#define NODE_GAP 8
#define NODE_GAP 0.0

// fp-error tolerance used to determine horizontal/vertical lines
#ifndef FP_VH_TOLERANCE
#define FP_VH_TOLERANCE 0.01
#endif

/**
  * This is the root class of annotation items hierarchy.
  *
  * Annotation items are used as "decorations" to AScrollView
  * This was thought to very simple drawing tools (lines, boxes, etc)
  * and a very small amount of object (no more than 100).
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AnnotationItem : public QObject
{
    Q_OBJECT
    public:

       /**
        * Constructor
        */
        AnnotationItem(AnnotationItem* _parent=NULL);

       /**
        * Destructor
        */
        virtual
        ~AnnotationItem();

        virtual void newChild(AnnotationItem* child);
        virtual void detachChild(AnnotationItem* child);
        virtual void reparent(AnnotationItem*);
        
       /**
        * Gets back item associated brush
        * @return item associated brush
        */
        inline virtual QBrush
        getBrush() const;

       /**
        * Gets back item associated brush
        * @return item associated brush
        */
        inline virtual QPen
        getPen() const;

       /**
        * Gets back a flag to know whether an object is being set for the very first time or not
        * @return build in progress flag
        */
        inline virtual bool
        getBuilding();

       /**
        * Mark object as "done", which means that further actions will modify instead of create...
        */
        inline virtual void
        setBuilt();

        inline virtual void setSelectable(bool);
        inline virtual bool getSelectable();
        
        inline virtual void setEditable(bool);
        inline virtual bool getEditable();

        /**
        * Sets item's brush
        */
        inline virtual void
        setBrush (
                 QBrush b
                 );

       /**
        * Sets item's pen
        */
        inline virtual void
        setPen (
               QPen p
               );

       /**
        * Mark object as visible
        */
        inline virtual void
        setVisible (
                   bool yes
                   );

       /**
        * Check whether an object is visible or not
        * @return visible flag
        */
        inline virtual bool
        isVisible() const;

       /**
        * Switches on and off the "selected" object flag
        */
        inline virtual void
        setSelected (
                    bool yes
                    );

       /**
        * Check whether the object is selected or not
        * @return selection flag
        */
        inline virtual bool
        isSelected() const;

       /**
        * Computes de graphical center of the item.
        * The default implementation computes the
        * center of the object's bounding box.
        */
        virtual void
        centerPosition (
                       double* x,
                       double* y
                       ) const;

       /**
        * Produce a clone object from current one.
        * @return a new instance of the same Annotation item subclass with identical field values.
        */
        virtual AnnotationItem*
        clone() ;

       /**
        * Copy 'this' fields to the given AnnotationItem object
        */
        virtual void
        clone (
              AnnotationItem* item  ///< Target object to put fields on.
              ) ;

       /**
        * Copy 'this' geometry related fields to the given AnnotationItem object
        */
        virtual void
        cloneGeometry (
                      AnnotationItem* item    ///< Target object to put fields on.
                      ) ;

       /**
        * Apply a position shifting to the item
        */
        virtual void
        move (
             double vx,    ///< X component of direction vector
             double vy,    ///< Y component of direction vector
             double dis    ///< Distance (direction vector gets normalized)
             ) ;

       /**
        * Apply a position shifting to the item
        */
        virtual void
        move (
             double x,     ///< absolute shifting value to X coord.
             double y      ///< absolute shifting value to Y coord.
             ) ;

       /**
        * Apply an offset to a "edition node"
        */
        virtual void
        moveNode (
                 double nx,             ///< "Clicked" X axis
                 double ny,             ///< "Clicked" Y axis
                 double x,              ///< offset for X
                 double y,              ///< offset for Y
                 double scf_x,          ///< current horizontal scalling factor
                 double scf_y           ///< current vertical scalling factor
                 ) ;

       /**
        * Checks if a given point is 'inner' to the object
        * @return TRUE if the given point is inner to the object
        */
        virtual bool
        innerPoint (
                   double x,        ///< "clicked" X
                   double y,        ///< "clicked" Y
                   double scf_x,    ///< Current horizontal scalling factor
                   double scf_y     ///< Current vertical scalling factor
                   );

       /**
        * Check if a given point is 'inner' to any "edition node" for this object
        */
        virtual bool
        innerNodePoint (
                       double x,            ///< "clicked" X
                       double y,            ///< "clicked" Y
                       double scf_x,        ///< Current horizontal scalling factor
                       double scf_y         ///< Current vertical scalling factor
                       ) ;

       /**
        * Computes a bounding box for this object
        */
        virtual void
        boundingRect (
                     double* x,             ///< left X
                     double* y,             ///< upper Y
                     double* w,             ///< width
                     double* h              ///< height
                     ) const  ;

       /**
        * Repaint the object on the given QT painter object
        */
        virtual void
        draw (
             QPainter* p,                   ///< QT Painter where to draw on
             double scf_x,                  ///< horizontal scalling factor
             double scf_y                   ///< vertical scalling factor
             ) ;

       /**
        * Paint rubber band (during object creation)
        */
        virtual void
        drawRubber (
                   int px,                 ///< starting point x
                   int py,                 ///< starting point y
                   int x,                  ///< ending point x
                   int y,                  ///< ending point y
                   QPainter* p             ///< QT Painter where to draw on
                   ) ;

       /**
        * What the heal this does?
        * @todo check is this is deprecated code...
        */
        virtual void
        mouseWorldPressEvent (
                             int vx,
                             int vy,
                             double wx,
                             double wy
                             ) ;

       /**
        * What the heal this does?
        * @todo check is this is deprecated code...
        */
        virtual void
        mouseWorldReleaseEvent (
                               int vx,
                               int vy,
                               double wx,
                               double wy
                               ) ;

       /**
        * What the heal this does?
        * @todo check is this is deprecated code...
        */
        virtual void
        mouseWorldMoveEvent (
                            int vx,
                            int vy,
                            double wx,
                            double wy
                            ) ;

       /**
        * Method to snap object coordinates to the grid.
        */
        virtual void
        snapToGrid (
                   int gridSize,            ///< current "gap" between gridlines
                   bool onlyCorner          ///< TRUE is used during object movements
                   ) ;


    protected:

       /**
        * Drawing of a selection node
        */
        void
        drawSelectionNode (
                          double x,         ///< node X coord.
                          double y,         ///< node Y coord.
                          QPainter* p,      ///< QT painter where to draw on
                          double scf_x,     ///< horizontal scalling factor
                          double scf_y      ///< vertical scalling factor
                          );

       /**
        * Check whether a clicked point is 'inner' to a given edition node
        * @return TRUE means inner
        */
        bool
        innerNodePointArround (
                             double x,      ///< Edition Node X
                             double y,      ///< Edition Node Y
                             double scf_x,  ///< horizontal scalling factor
                             double scf_y,  ///< vertical scalling factor
                             double px,     ///< 'clicked' X
                             double py      ///< 'clicked' Y
                             );

       /**
        * Modifies a given point so it's snapped to the grid lines
        */
        void
        snapToGridPoint (
                        int gridSize,    ///< "gap" between grid lines
                        double &x,       ///< X coord. 
                        double &y        ///< Y coord. 
                        );

       /**
        * Checks wheter a given point is near to a linear segment.
        */
        bool
        pointNearSegment (
                         double x,       ///< "Clicked" X
                         double y,       ///< "Clicked" Y
                         double x1,      ///< Segment starting point X
                         double y1,      ///< Segment starting point Y
                         double x2,      ///< Segment ending point X
                         double y2,      ///< Segment ending point Y
                         double scf_x,   ///< Horizontal scalling factor
                         double scf_y
                         ) const;

       /**
        * Function description
        */
        double
        vecAngle (
                 double vx,
                 double vy
                 ) const;

    protected:
        QPen    myPen;
        QBrush  myBrush;
        bool    visible;
        bool    selected;
        bool    selectable;
        bool    editable;
        bool    building;
        AnnotationItem * parent;
        double    pi;
};

void
AnnotationItem::setVisible(bool yes)
{
    visible = yes;
}

bool
AnnotationItem::isVisible() const
{
    return visible;
}

void
AnnotationItem::setSelected(bool yes)
{
    selected=yes;
}

bool
AnnotationItem::isSelected() const
{
    return selected;
}

QBrush
AnnotationItem::getBrush() const
{
    return myBrush;
}

QPen
AnnotationItem::getPen() const
{
    return myPen;
}

void
AnnotationItem::setBrush(QBrush b)
{
    myBrush = b;
}

void
AnnotationItem::setPen(QPen p)
{
    myPen = p;
}

bool
AnnotationItem::getBuilding()
{
    return building;
}

void
AnnotationItem::setBuilt()
{
    building = false;
}

void
AnnotationItem::setSelectable(bool value)
{
    selectable = value;
}

bool
AnnotationItem::getSelectable()
{
    return selectable;
}

void
AnnotationItem::setEditable(bool value)
{ editable = value; }

bool
AnnotationItem::getEditable()
{ return editable; }

#endif


