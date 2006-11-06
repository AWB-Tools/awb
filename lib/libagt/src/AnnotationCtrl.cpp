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
  * @file  AnnotationCtrl.cpp
  */


#include "AnnotationCtrl.h"
#include "AScrollView.h"

AnnotationCtrl::AnnotationCtrl(QScrollView* view, double gridS, bool snap)
{
    Q_ASSERT(view!=NULL);

    myView = view;
    myViewport = view->viewport();

    creatingObject=false;
    movingObject=false;
    modifingObject=false;
    annToolDragging=false;

    // grid
    snapToGrid=snap;
    gridSize = gridS;

    // default ann tool db is empty
    annDB = new AnnotationItemDB();
    Q_ASSERT(annDB!=NULL);

    draggingAnnItem = NULL;
    copyAnnItem = NULL;
}

AnnotationCtrl::~AnnotationCtrl()
{
    delete annDB;
}

void
AnnotationCtrl::clone(AnnotationCtrl* parent)
{
    annDB->reset();
    QPtrDictIterator<AnnotationItem> it = parent->annDB->getIterator();
    AnnotationItem* item;
    while (it.current())
    {
        item = (AnnotationItem*) it.current();
        annDB->add(item->clone());
        ++it;
    }
}

// during creation of a new object:
void
AnnotationCtrl::creatingRubberUpdate(int x, int y, AnnotationItem* item)
{
    // erase old rubber
    drawRubber(lastAnnTool_x,lastAnnTool_y, item);

    // and draw the new one
    lastAnnTool_x = x;
    lastAnnTool_y = y;

    drawRubber(lastAnnTool_x,lastAnnTool_y, item);
}

// during modification of an already created object:
void
AnnotationCtrl::movingSelectionUpdate(int x, int y, AnnotationItem* item, double scf_x, double scf_y)
{
    Q_ASSERT(item!=NULL);

    ////printf ("mov update called over 0x%lx, x=%d, y=%d",item,x,y);

    // if dragging object is null create a new one
    if (draggingAnnItem == NULL)
    {
        //printf ("\n\nclone on movingSelectionUpdate\n");fflush(stdout);
        draggingAnnItem = item->clone();
        draggingAnnItem->setPen(QPen(QColor(0,0,0),1,DotLine));
        draggingAnnItem->setBrush(NoBrush);
        //printf("AnnotationCtrl::movingSelectionUpdate calling add..\n");
        annDB->add(draggingAnnItem);
    }
    else
    {
        draggingAnnItem->setVisible(false);
        repaintAnnItem(draggingAnnItem,scf_x,scf_y);
    }

    // compute distance between original and current position...
    double dx =  ((double)(x-lastAnnTool_x))/scf_x;
    double dy =  ((double)(y-lastAnnTool_y))/scf_y;
    draggingAnnItem->move(dx,dy);
    if (snapToGrid)
    {
        draggingAnnItem->snapToGrid((int)floor(gridSize),true);
    }

    draggingAnnItem->setVisible(true);
    repaintAnnItem(draggingAnnItem,scf_x,scf_y);

    lastAnnTool_x = x;
    lastAnnTool_y = y;
}


void
AnnotationCtrl::mouseWorldPressEvent(QMouseEvent* e, AnnotationItem* item,
    double x, double y, double scf_x, double scf_y)
{
    // viewport coordinates
    pressed_x = e->x();
    pressed_y = e->y();
    // world coordinates
    pressed_wx = x;
    pressed_wy = y;

    // if no snap to grip snaped coordinates are the same than the normal ones
    snapCoordinates(pressed_x,pressed_y,pressed_x_snapped,pressed_y_snapped,scf_x,scf_y);

    // check pointer type (creating / selecting / modifing)
    annToolDragging = true;

    lastAnnTool_x = pressed_x_snapped;
    lastAnnTool_y = pressed_y_snapped;

    if (item->getBuilding())
    {
        creatingObject = true;
        // call the item dependent routine
        item->mouseWorldPressEvent(pressed_x_snapped, pressed_y_snapped, pressed_wx,pressed_wy);
    }
    else if (item->isSelected())
    {
        // check for node dragging
        //if (item->innerNodePoint(x,y,scf_x,scf_y))
        //{
        //    modifingObject = true;
            //printf("modifing object activated!\n");fflush(stdout);
        //}
        //else
        //{
            movingObject = true;
        //}    

    }
    else
    {
        movingObject = true;
    }    

}

void
AnnotationCtrl::mouseWorldReleaseEvent(QMouseEvent* e, AnnotationItem* item,
    double x, double y, double scf_x, double scf_y)
{
    // viewport coordinates
    released_x = e->x();
    released_y = e->y();
    // world coordinates
    released_wx = x;
    released_wy = y;

    // if no snap to grip snaped coordinates are the same than the normal ones
    snapCoordinates(released_x,released_y,released_x_snapped,released_y_snapped,scf_x,scf_y);

    // check pointer type (creating / selecting / modifing)
    annToolDragging = false;

    // if I was creating an object remove the las rubber
    if (creatingObject)
    {
        drawRubber(released_x_snapped,released_y_snapped, item);

        // call the item dependent routine
        item->mouseWorldReleaseEvent(released_x_snapped,released_y_snapped, released_wx,released_wy);

        // add the new item to the item db
        //printf("AnnotationCtrl::mouseReleaseEvent calling add..\n");
        annDB->add(item);

        // clean-up all ongoing-task flags
        creatingObject=false;
        item->setBuilt();
        item->setSelected(true);
        if (snapToGrid)
        {
            item->snapToGrid((int)floor(gridSize),false);
        }    
        repaintAnnItem(item,scf_x,scf_y);

    }
    else if ((movingObject)||(modifingObject) )
    {
        //printf ("((movingObject)||(modifingObject)) detected !\n");fflush(stdout);
        // apply movement (if any)
        if(draggingAnnItem!=NULL)
        {
            // propagate event
            emit annotationChanged(draggingAnnItem,item);
            
            // clean old position
            item->setVisible(false);
            repaintAnnItem(item,scf_x,scf_y);
            QBrush oldb = item->getBrush();
            QPen oldp = item->getPen();
            draggingAnnItem->cloneGeometry(item);
            item->setBrush(oldb);
            item->setPen(oldp);
            if (snapToGrid)
            {
                item->snapToGrid((int)floor(gridSize),movingObject);
            }    
            bool rok = annDB->remove(draggingAnnItem,true);
            Q_ASSERT (rok);
            draggingAnnItem=NULL;
        }

        // clean-up all ongoing-task flags
        movingObject=false;
        modifingObject=false;

        // repaint
        item->setSelected(true);
        item->setVisible(true);
        repaintAnnItem(item,scf_x,scf_y);
    }
    else
    {
        Q_ASSERT(false);
    }

}


void
AnnotationCtrl::mouseWorldMoveEvent(QMouseEvent* e, AnnotationItem* item,
    double x, double y, double scf_x, double scf_y)
{
    // viewport coordinates
    current_x = e->x();
    current_y = e->y();
    // wolrd coordinates
    current_wx = x;
    current_wy = y;

    // if no snap to grip snaped coordinates are the same than the normal ones
    snapCoordinates(current_x,current_y,current_x_snapped,current_y_snapped,scf_x,scf_y);

    if (!annToolDragging)
    {
        return;
    }    

    if (creatingObject)
    {
        creatingRubberUpdate(current_x_snapped,current_y_snapped,item);
    }    
    else if (movingObject)
    {
        movingSelectionUpdate(current_x_snapped,current_y_snapped,item,scf_x,scf_y);
    }    
    else if (modifingObject)
    {
        modifingSelectionUpdate(current_x_snapped,current_y_snapped,item,scf_x,scf_y);
    }    
    else
    {
        Q_ASSERT(false);
    }    
}

void
AnnotationCtrl::drawRubber(int x, int y, AnnotationItem* item)
{
    QPainter p;
    p.begin( myViewport );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush);

    item->drawRubber(pressed_x_snapped,pressed_y_snapped,x,y,&p);

    p.end();
}

// scalling factors requiered due to  node points drawing (for modification of selected objects)
void 
AnnotationCtrl::drawArea(QPainter* painter, double ncx, double ncy, double ncw, double nch, double scf_x, double scf_y)
{
    // no clipping at all, review this!!!

    ////printf ("drawAnnTools called\n");

    QPtrDictIterator<AnnotationItem> it = annDB->getIterator();
    for( ; it.current(); ++it )
    {
        AnnotationItem* item = (AnnotationItem*) it.currentKey();
        //printf ("drawAnnTools drawing obj=0x%x\n",(void*)item);fflush(stdout);
        item->draw(painter,scf_x,scf_y);
    }
}

AnnotationItem* AnnotationCtrl::lookAnnItemAt(double x, double y,double scf_x,double scf_y)
{
    // this is intended to be used with very few objects !
    // right now this is a complete linear search against the hash table,
    // review clipping techniques !
    double bx,by,bw,bh;
    bool fnd = false;
    QPtrDictIterator<AnnotationItem> it = annDB->getIterator();
    AnnotationItem* item;
    AnnotationItem* prevItem=NULL;

    while (it.current() && !fnd)
    {
        item = (AnnotationItem*) it.currentKey();
        fnd = item->innerPoint(x,y,scf_x,scf_y);
        ++it;
        prevItem = item;
    }

    if (!fnd)
    {
        item = NULL;
    }    
    else
    {
        item = prevItem;
    }    

    return item;
}

AnnotationItem* AnnotationCtrl::lookAnnItemNodeAt(double x, double y,double scf_x,double scf_y)
{
    // this is intended to be used with very few objects !
    // right now this is a complete linear search against the hash table,
    // review clipping techniques !
    bool fnd = false;
    QPtrDictIterator<AnnotationItem> it = annDB->getIterator();
    AnnotationItem* item;
    AnnotationItem* prevItem=NULL;

    while (it.current() && !fnd)
    {
        item = (AnnotationItem*) it.currentKey();
        fnd = item->innerNodePoint(x,y,scf_x,scf_y);
        ++it;
        prevItem = item;
    }

    if (!fnd)
    {
        item = NULL;
    }    
    else
    {
        item = prevItem;
    }    

    return item;
}

void AnnotationCtrl::repaintAnnItem(AnnotationItem* item, double scf_x, double scf_y)
{
    double cx,cy,cw,ch;
    item->boundingRect(&cx,&cy,&cw,&ch);

    // correction for very narrow/very slim objects    ...
    // ...pending

    // scale
    cx *= scf_x;
    cy *= scf_y;
    cw *= scf_x;
    ch *= scf_y;

    double node_gap_h = NODE_GAP+NODE_SIZE;
    double node_gap_v = NODE_GAP+NODE_SIZE;
    //node_gap_h = ceil(node_gap_h*scf_x);
    //node_gap_v = ceil(node_gap_v*scf_y);
    cx -= node_gap_h;
    cy -= node_gap_v;
    cw += 2.1*node_gap_h;
    ch += 2.1*node_gap_v;

    myView->repaintContents((int)floor(cx-1.1),(int)floor(cy-1.1),(int)ceil(cw+2.6),(int)ceil(ch+2.6),false);
}


void AnnotationCtrl::modifingSelectionUpdate(int x,int y,AnnotationItem* item, double scf_x, double scf_y)
{
    Q_ASSERT(item!=NULL);

    //printf ("modSelUpd on 0x%lx, x=%d, y=%d\n",item,x,y);

    // if dragging object is null create a new one
    if (draggingAnnItem == NULL)
    {
        //printf ("\n\nclone on modifingSelectionUpdate\n");fflush(stdout);
        draggingAnnItem = item->clone();
        draggingAnnItem->setPen(QPen(QColor(0,0,0),1,DotLine));
        draggingAnnItem->setBrush(NoBrush);
        printf("AnnotationCtrl::modifingSelectionUpdate calling add..\n");
        annDB->add(draggingAnnItem);
    }
    else
    {
        draggingAnnItem->setVisible(false);
        repaintAnnItem(draggingAnnItem,scf_x,scf_y);
    }

    // compute distance between original and current position...
    double dx =  ((double)(x-lastAnnTool_x))/scf_x;
    double dy =  ((double)(y-lastAnnTool_y))/scf_y;
    draggingAnnItem->moveNode(pressed_wx,pressed_wy,dx,dy,scf_x,scf_y);
    if (snapToGrid)
    {
        draggingAnnItem->snapToGrid((int)floor(gridSize),false);
    }
    pressed_wx += dx;
    pressed_wy += dy;
    draggingAnnItem->setVisible(true);
    repaintAnnItem(draggingAnnItem,scf_x,scf_y);

    lastAnnTool_x = x;
    lastAnnTool_y = y;
}

void AnnotationCtrl::reset()
{
    creatingObject=false;
    movingObject=false;
    modifingObject=false;
    annToolDragging=false;

    draggingAnnItem=NULL;

    annDB->reset();
}

void 
AnnotationCtrl::annotationRemove(AnnotationItem* item)
{
    AScrollView* asv = (AScrollView*)myView;
    if (item)
    {
        item->setVisible(false);
        repaintAnnItem(item,asv->getScallingFactorX(),asv->getScallingFactorY());
        emit annotationRemoved(item->clone());
        bool rok = annDB->remove(item,true);
        Q_ASSERT (rok);
    }
}

AnnotationItem*
AnnotationCtrl::annotationPaste(AnnotationItem* item)
{
    AnnotationItem* result = NULL;
    AScrollView* asv = (AScrollView*)myView;
    if (item)
    {
        AnnotationItem* clone = item->clone();
        clone->move(asv->getBaseElementSize()*2.0,asv->getBaseElementSize()*2.0);
        clone->setSelected(true);
        //printf("AnnotationCtrl::annotationPaste calling add..\n");
        annDB->add(clone);
        repaintAnnItem(clone,asv->getScallingFactorX(),asv->getScallingFactorY());
        result=clone;
    }
    return result;
}

AnnotationItem* 
AnnotationCtrl::lookForItemWithBBox(double x, double y, double w, double h)
{
    double cx,cy,cw,ch;
    QPtrDictIterator<AnnotationItem> it = annDB->getIterator();
    AnnotationItem* item;
    AnnotationItem* prevItem=NULL;
    bool fnd=false;
    
    while (it.current() && !fnd)
    {
        item = (AnnotationItem*) it.current();
        item->boundingRect(&cx,&cy,&cw,&ch);
        fnd = (fabs(cx-x)<0.01);
        fnd = fnd && (fabs(cy-y)<0.01);
        fnd = fnd && (fabs(cw-w)<0.01);
        fnd = fnd && (fabs(ch-h)<0.01);
        ++it;
        prevItem = item;
    }

    if (!fnd)
    {
        item = NULL;
    }    
    else
    {
        item = prevItem;
    }    

    return item;
}

void 
AnnotationCtrl::removePropagatedItem(AnnotationItem* item)
{
    //printf("AnnotationCtrl::removePropagatedItem called on 0x%x\n",(void*)item);fflush(stdout);
    // look for an equivalent object in this controler
    double x,y,w,h;
    item->boundingRect(&x,&y,&w,&h);
    AnnotationItem* eq = lookForItemWithBBox(x,y,w,h);
    //printf("AnnotationCtrl::removePropagatedItem eq=0x%x\n",(void*)eq);fflush(stdout);
    if (eq)
    {
        AScrollView* asv = (AScrollView*)myView;
        eq->setVisible(false);
        repaintAnnItem(eq,asv->getScallingFactorX(),asv->getScallingFactorY());
        annDB->remove(eq,true);
    }
    delete item;
}

void
AnnotationCtrl::addPropagatedItem(AnnotationItem* item)
{
    AnnotationItem* cl=item->clone();
    annDB->add(cl);
    AScrollView* asv = (AScrollView*)myView;
    repaintAnnItem(cl,asv->getScallingFactorX(),asv->getScallingFactorY());
}

void
AnnotationCtrl::changePropagatedItem(AnnotationItem* newitem, AnnotationItem* olditem)
{
    double x,y,w,h;
    AScrollView* asv = (AScrollView*)myView;
    olditem->boundingRect(&x,&y,&w,&h);
    AnnotationItem* eq = lookForItemWithBBox(x,y,w,h);
    if (eq)
    {
        QBrush oldb = eq->getBrush();
        QPen oldp = eq->getPen();
        eq->setVisible(false);
        repaintAnnItem(eq,asv->getScallingFactorX(),asv->getScallingFactorY());
        eq->setVisible(newitem->isVisible());
        newitem->cloneGeometry(eq);
        eq->setBrush(oldb);
        eq->setPen(oldp);
        repaintAnnItem(eq,asv->getScallingFactorX(),asv->getScallingFactorY());
    }
}

