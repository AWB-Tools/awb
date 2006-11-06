/*
 *Copyright (C) 2004-2006 Intel Corporation
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
  * @file  AnnotationContainer.cpp
  */

#include "AnnotationContainer.h"

AnnotationContainer::AnnotationContainer(AnnotationItem* _parent) : AnnotationItem(_parent)
{
    //printf("AnnotationContainer::AnnotationContainer, my @=0x%x, parent=0x%x\n",this,parent);fflush(stdout);
}

AnnotationContainer::~AnnotationContainer()
{
    reset();
}

void
AnnotationContainer::reset()
{
    children.clear();
    setbbox= QRect();
}

bool
AnnotationContainer::innerPoint(double x, double y,double scf_x,double scf_y)
{
    //printf("AnnotationContainer::innerPoint on 0x%x, selectable=%d, setbbox_valid=%d,contains=%d\n",this,(int)selectable,(int)(setbbox.isValid()),(int)(setbbox.contains((int)x,(int)y)));fflush(stdout);
    if (!selectable) return false;
    if (!setbbox.isValid()) return false;
    if (!setbbox.contains((int)x,(int)y)) return false;

    //printf("AnnotationContainer::passed basic checks!\n");fflush(stdout);
    
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        //printf("AnnotationContainer::innerPoint checking against 0x%x\n",ann);fflush(stdout);
        if (ann->innerPoint(x,y,scf_x,scf_y)) return true;
    }
    return false;
}

bool
AnnotationContainer::innerNodePoint(double x, double y,double scf_x,double scf_y)
{
    return false;
}

void
AnnotationContainer::boundingRect(double* rx1, double* ry1, double* w, double* h) const
{
    *rx1 = setbbox.x();
    *ry1 = setbbox.y();
    *w = setbbox.width();
    *h = setbbox.height();
}

void
AnnotationContainer::draw(QPainter* p, double scf_x, double scf_y)
{
    QBrush old = p->brush();

    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->draw(p,scf_x,scf_y);
    }
    
    if (visible && selected)
    {
        double leftX,upperY,rightX,bottomY;
        double node_gap_v = NODE_GAP;
        double node_gap_h = NODE_GAP;
        node_gap_v /= scf_y;
        node_gap_h /= scf_x;

        leftX   = (double)setbbox.left();
        upperY  = (double)setbbox.top();
        rightX  = (double)setbbox.right();
        bottomY = (double)setbbox.bottom();
        
        drawSelectionNode(floor(leftX-node_gap_h),floor(upperY-node_gap_v),p,scf_x,scf_y);
        drawSelectionNode(floor(leftX-node_gap_h),floor(bottomY+node_gap_v),p,scf_x,scf_y);
        drawSelectionNode(floor(rightX+node_gap_h),floor(bottomY+node_gap_v),p,scf_x,scf_y);
        drawSelectionNode(floor(rightX+node_gap_h),floor(upperY-node_gap_v),p,scf_x,scf_y);
    }

    p->setBrush(old);
    
}

void
AnnotationContainer::drawRubber(int px, int py, int x, int y, QPainter* p)
{
    //printf("AnnotationContainer::drawRubber\n");fflush(stdout);
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->drawRubber(px,py,x,y,p);
    }
}

void
AnnotationContainer::mouseWorldPressEvent(int vx, int vy, double wx, double wy)
{
    //x1=wx;
    //y1=wy;
    //printf ("AnnLine::staring point at %f,%f\n",x1,y1);
}

void
AnnotationContainer::mouseWorldReleaseEvent(int vx, int vy, double wx, double wy)
{
    //x2=wx;
    //y2=wy;
    //printf ("AnnLine::ending point at %f,%f\n",x2,y2);
}

void
AnnotationContainer::mouseWorldMoveEvent(int vx, int vy, double wx, double wy)
{
}

AnnotationItem* AnnotationContainer::clone()
{
    AnnotationContainer* nl = new AnnotationContainer(parent);
    Q_ASSERT(nl!=NULL);
    AnnotationItem* ann;
    QPtrList<AnnotationItem> cpyList = QPtrList<AnnotationItem>(children); 
    for ( ann = cpyList.first(); ann; ann = cpyList.next() )
    {
        //printf("AnnotationContainer::clone clonning obj=0x%x\n",ann);fflush(stdout);
        AnnotationItem* cann = ann->clone();
        //printf("AnnotationContainer::clone clonned obj=0x%x\n",cann);fflush(stdout);
        cann->reparent(nl);
    }
    //printf("AnnotationContainer::clone done, this=0x%x, clone=0x%x\n",this,nl);fflush(stdout);
    nl->setBuilt();
    return nl;
}

void
AnnotationContainer::clone(AnnotationItem* item)
{
    Q_ASSERT(false);
}

void
AnnotationContainer::cloneGeometry(AnnotationItem* item)
{
    //printf("AnnotationContainer::clone geometry called this=0x%x, item=0x%x\n",this,item);fflush(stdout);
    AnnotationContainer* cont = (AnnotationContainer*)item;
    cont->reset();
    AnnotationItem* ann;
    QPtrList<AnnotationItem> cpyList = QPtrList<AnnotationItem>(children); 
    for ( ann = cpyList.first(); ann; ann = cpyList.next() )
    {
        AnnotationItem* cann = ann->clone();
        cann->reparent(cont);
    }
}

void 
AnnotationContainer::newChild(AnnotationItem* child)
{
    //printf("AnnotationContainer::newChild on 0x%x, with child 0x%x\n",this,child);fflush(stdout);
    if (children.contains(child)==0)
    {
        children.append(child);
        // set bbox update 
        applyNewObject(child);
        myBrush=child->getBrush();
        myPen=child->getPen();
    }
}


void 
AnnotationContainer::detachChild(AnnotationItem* child)
{
    //printf ("AnnotationContainer::detachChild this=0x%x, child=0x%x \n",this,child);fflush(stdout);
    if (children.contains(child)!=0)
    {
        children.remove(child);
        recomputeBBox();
    }
}

void
AnnotationContainer::move(double x, double y)
{
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->move(x,y);
    }
    recomputeBBox();
}

void
AnnotationContainer::move(double vx, double vy, double dis)
{
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->move(vx,vy,dis);
    }   
    recomputeBBox();
}

void
AnnotationContainer::moveNode(double nx, double ny, double x, double y, double scf_x, double scf_y)
{
    /*
    // look for the right node (only two in this case)
    bool c1 = innerNodePointArround(nx,ny,scf_x,scf_y,x1,y1);
    bool c2 = innerNodePointArround(nx,ny,scf_x,scf_y,x2,y2);
    if (c1)
    {
        x1+=x;
        y1+=y;
    }
    else if (c2)
    {
        x2+=x;
        y2+=y;
    }
    */
}

void
AnnotationContainer::snapToGrid(int gridSize, bool onlyCorner)
{
    /*
    double bx1 = x1;
    double by1 = y1;

    snapToGridPoint(gridSize,x1,y1);
    if (!onlyCorner)
    {
        snapToGridPoint(gridSize,x2,y2);
    }
    else
    {
        x2 -= (bx1-x1);
        y2 -= (by1-y1);
    }
    */
}

void
AnnotationContainer::recomputeBBox()
{
    //printf("AnnotationContainer::recomputeBBox on 0x%x \n",this);fflush(stdout);
    setbbox = QRect();
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        applyNewObject(ann);
    }   
}

void
AnnotationContainer::applyNewObject(AnnotationItem* ann)
{
    //printf("AnnotationContainer::applyNewObject on 0x%x, with child 0x%x\n",this,ann);fflush(stdout);
    double annx,anny,annw,annh,annx2,anny2;
    annw=-1.0;annh=-1.0;
    ann->boundingRect (&annx,&anny,&annw,&annh);
    annx2=annx+annw;
    anny2=anny+annh;
    if ((annw>=0.0)&&(annh>=0.0))  // valid & significative bbox
    {
        if (setbbox.isValid())
        {
            // update set bbox with current object bbox
            setbbox.setLeft(QMIN(setbbox.left(),(int)annx));
            setbbox.setRight(QMAX(setbbox.right(),(int)annx2));
            setbbox.setTop(QMIN(setbbox.top(),(int)anny));
            setbbox.setBottom(QMAX(setbbox.bottom(),(int)anny2));
        }
        else
        {
            // init set bbox with current object bbox
            setbbox = QRect((int)annx,(int)anny,(int)annw,(int)annh);
        }
    }
}

void 
AnnotationContainer::setSelectable(bool value)
{
    selectable=value;
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->setSelectable(value);
    }
}

void 
AnnotationContainer::setEditable(bool value)
{
    editable=value;
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->setEditable(value);
    }
}

void
AnnotationContainer::setBrush (QBrush b)
{
    myBrush=b;
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->setBrush(b);
    }
}

void
AnnotationContainer::setPen (QPen p)
{
    myPen=p;
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->setPen(p);
    }
}

void
AnnotationContainer::setVisible (bool value)
{
    visible=value;
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->setVisible(value);
    }
}

void
AnnotationContainer::setBuilt()
{
    building = false;
    AnnotationItem* ann;
    for ( ann = children.first(); ann; ann = children.next() )
    {
        ann->setBuilt();
    }
}

