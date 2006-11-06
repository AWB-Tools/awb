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
  * @file   AnnotationCtrl.h
  * @brief  AnnotationCtrl class wraps the event handling routines related with annotation tools.
  */

#ifndef __ANNOTATIONCTRL_H
#define __ANNOTATIONCTRL_H

#include <math.h>

#include <qevent.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qscrollview.h>

#include "agt_syntax.h"
#include "AnnotationItem.h"
#include "AnnotationItemDB.h"

#ifndef MIN
#define MIN(a,b) a < b ? a : b
#endif

#ifndef MAX
#define MAX(a,b) a < b ? b : a
#endif

/**
  * This class wraps the event handling routines related with annotation tools.
  * Put long explanation here
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AnnotationCtrl : public QObject
{
    Q_OBJECT

    public:

        /**
         * Function description
         */
        AnnotationCtrl (
                       QScrollView* view,
                       double gridS,
                       bool snap
                       );

       /**
         * Function description
         */
        ~AnnotationCtrl();

        /**
         * Function description
         */
        void clone(AnnotationCtrl* parent);

        void annotationRemove(AnnotationItem*);
        AnnotationItem* annotationPaste(AnnotationItem*);
        
        /**
         * Function description
         */
        void
        reset ();

        /**
         * Function description
         */
        inline void
        setBaseSize (
                    double bsize
                    );
    
        /**
         * Function description
         */
        inline void
        setGridSize (
                    double size
                    );
    
        /**
         * Function description
         */
        inline void
        setSnapToGrid (
                      bool
                      );
    
        /**
         * Function description
         */
        inline bool
        getSnapToGrid();
    
        /**
         * Function description
         */
        void
        mouseWorldPressEvent (
                             QMouseEvent*,
                             AnnotationItem*,
                             double,
                             double,
                             double,
                             double
                             );
    
        /**
         * Function description
         */
        void
        mouseWorldReleaseEvent (
                               QMouseEvent*,
                               AnnotationItem*,
                               double,
                               double,
                               double,
                               double
                               );
    
        /**
         * Function description
         */
        void
        mouseWorldMoveEvent (
                            QMouseEvent*,
                            AnnotationItem*,
                            double,
                            double,
                            double,
                            double
                            );
    
        /**
         * Function description
         */
        void
        repaintAnnItem (
                       AnnotationItem*,
                       double,
                       double
                       );
    
        /**
         * Function description
         */
        void drawArea (
             QPainter*,
             double,
             double,
             double,
             double,
             double,
             double
             );
    
        /**
         * Function description
         */
        AnnotationItem*
        lookAnnItemAt (
                      double,
                      double,
                      double,
                      double
                      );
    
        /**
         * Function description
         */
        AnnotationItem*
        lookAnnItemNodeAt (
                          double x,
                          double y,
                          double scf_x,
                          double scf_y
                          );
    
        /**
         * Function description
         */
        inline void
        addItem (
                AnnotationItem*
                );

        void
        addPropagatedItem (
                AnnotationItem*
                );

        void removePropagatedItem(AnnotationItem*);
        AnnotationItem* lookForItemWithBBox(double x, double y, double w, double h);
        void changePropagatedItem(AnnotationItem* newitem, AnnotationItem* olditem);
        
        inline QPtrDictIterator<AnnotationItem> getIterator();
        
    signals:
        
        void annotationAdded(AnnotationItem*);
        void annotationRemoved(AnnotationItem*);
        void annotationChanged(AnnotationItem*,AnnotationItem*);
        
        protected:
    
        /**
         * Function description
         */
        void
        creatingRubberUpdate (
                             int x,
                             int y,
                             AnnotationItem* item
                             );
    
        /**
         * Function description
         */
        void
        movingSelectionUpdate (
                              int,
                              int,
                              AnnotationItem*,
                              double,
                              double
                              );
    
        /**
         * Function description
         */
        void
        modifingSelectionUpdate (
                                int,
                                int,
                                AnnotationItem*,
                                double,
                                double
                                );
    
        /**
         * Function description
         */
        void
        drawRubber (
                   int x,
                   int y,
                   AnnotationItem* item
                   );
    
        /**
         * Function description
         */
        inline void
        snapCoordinates (
                        int x,
                        int y,
                        int& rx,
                        int& ry,
                        double scf_x,
                        double scf_y
                        );
    
        /**
         * Function description
         */
        inline bool
        innBox (
               double,
               double,
               double,
               double,
               double,
               double
               );

    private:
        // the annotation items database
        AnnotationItemDB* annDB;

        // viewport reference
        QWidget* myViewport;
        QScrollView* myView;
        double baseSize;
        double gridSize;
    
        bool creatingObject;
        bool movingObject;
        bool modifingObject;
        bool annToolDragging;
        bool snapToGrid;
    
        int released_x;    // viewport coordinates
        int released_y;
        int pressed_x;
        int pressed_y;
        int current_x;
        int current_y;
    
        double released_wx;    // world coordinates
        double released_wy;
        double pressed_wx;
        double pressed_wy;
        double current_wx;
        double current_wy;
    
        // snapped viewport coordinates
        int pressed_x_snapped;
        int pressed_y_snapped;
        int released_x_snapped;
        int released_y_snapped;
        int current_x_snapped;
        int current_y_snapped;
    
        // on dragging, last catched coordinate (snaped viewport coordinates)
        int lastAnnTool_x;
        int lastAnnTool_y;
    
        // for item move:
        AnnotationItem* draggingAnnItem;

        // for cut & paste    
        AnnotationItem* copyAnnItem;

};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Inlined routines
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * Check if a point is inside or not a given rect. area (bounding box)
 */
bool
AnnotationCtrl::innBox(double bx, double by, double bw, double bh, double x, double y)
{
    return ((x>=bx) && (y>=by) && (x<=(bx+bw-1.0)) && (y<=(by+bh-1.0)));
}



/**
 * Snap to grip (if active) viewport given coordinates
 */
void
AnnotationCtrl::snapCoordinates(int x, int y, int& rx, int& ry, double scf_x, double scf_y)
{
    if (!snapToGrid)
    {
        rx = x;
        ry = y;
    }
    else
    {
        // pass to world coordinates:
        int cx,cy,tx,ty;
        double dx,dy;
        myView->viewportToContents (x,y,cx,cy);
        dx = ((double)cx/scf_x);
        dy = ((double)cy/scf_y);

        //printf ("## snapping vp (%d,%d), wc(%f,%f)\n",x,y,dx,dy);

        // round
        dx = floor(dx);
        dy = floor(dy);
        //printf ("## rounded wc(%f,%f)\n",dx,dy);

        // compute nearest grid lines
        int bsize = (int)floor(gridSize);
        int offX = ((int)dx)%bsize;
        int offY = ((int)dy)%bsize;

        // to the left/up
        int gx1 = (int)dx - offX;
        int gy1 = (int)dy - offY;
        // to the right/down
        int gx2 = (int)dx + (bsize-offX);
        int gy2 = (int)dy + (bsize-offY);

        //printf ("## gx1=%d, gx2=%d, gy1=%d, gy2=%d\n",gx1,gx2,gy1,gy2);

        // choose the smallest distance for each component
        dx = ( (fabs((double)gx1-dx)) > (fabs((double)gx2-dx)) ) ? (double)gx2 : (double)gx1;
        dy = ( (fabs((double)gy1-dy)) > (fabs((double)gy2-dy)) ) ? (double)gy2 : (double)gy1;

        //printf ("## choosen wc(%f,%f)\n",dx,dy);

        // transform again into viewport coordinates
        dx *= scf_x;
        dy *= scf_y;

        //printf ("## choosen wc scaled (%f,%f)\n",dx,dy);

        tx = (int)floor(dx);
        ty = (int)floor(dy);

        myView->contentsToViewport(tx,ty,rx,ry);
        //printf ("## choosen vp no shift (%d,%d)\n",tx,ty);
        //printf ("## choosen vp shifted (%d,%d)\n",rx,ry);
    }
}

void
AnnotationCtrl::addItem(AnnotationItem* item)
{
    annDB->add(item);
    emit annotationAdded(item); 
}

void 
AnnotationCtrl::setBaseSize(double bsize)
{
    baseSize = bsize;
}

void 
AnnotationCtrl::setGridSize(double size)
{
    gridSize = size; 
}

void
AnnotationCtrl::setSnapToGrid(bool v)
{
    snapToGrid = v; 
}

bool
AnnotationCtrl::getSnapToGrid()
{
    return snapToGrid;
}

QPtrDictIterator<AnnotationItem>
AnnotationCtrl::getIterator()
{
    return annDB->getIterator();
}

#endif
