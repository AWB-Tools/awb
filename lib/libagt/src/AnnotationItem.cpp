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
  * @file  AnnotationItem.cpp
  */

#include "AnnotationItem.h"

AnnotationItem::AnnotationItem(AnnotationItem* _parent)
{
    visible = true;
    selected = false;
    building = true;
    selectable=true;
    editable=true;
    pi = 4.0*atan(1.0);
    parent=_parent;
}

AnnotationItem::~AnnotationItem()
{}

void
AnnotationItem::drawSelectionNode(double x, double y, QPainter* p,double scf_x,double scf_y)
{
    double ns = NODE_SIZE;

    p->scale(1.0/scf_x,1.0/scf_y);
    p->setBrush(QBrush(QColor(255,255,255)));
    p->setPen(QPen(QColor(0,0,0),1));

    p->drawRect((int)floor(x*scf_x - (ns/2.0)),
        (int)floor(y*scf_y - (ns/2.0)),
        (int)floor(ns+1.0),
        (int)floor(ns+1.0));

    p->scale(scf_x,scf_y);
}

bool 
AnnotationItem::innerNodePointArround(double x, double y,
    double scf_x,double scf_y, double px,double py)
{
    double vx,vy,mod,ns;
    ns = NODE_SIZE;
    vx = x-px;
    vy = y-py;
    //printf ("innNodePArround node=(%f,%f), clicked=(%f,%f), v=(%f,%f)\n",px,py,x,y,vx,vy);
    vx *= scf_x;
    vy *= scf_y;
    mod = sqrt(vx*vx + vy*vy);
    //printf ("\tafter scalling v=(%f,%f), mod=%f\n",vx,vy,mod);
    return (mod<=ns);
}

void 
AnnotationItem::centerPosition(double* x, double* y) const
{
    // get bounding box:
    double bx,by,bw,bh;
    this->boundingRect(&bx,&by,&bw,&bh);

    // compute center of given bounding box
    *x = bx+(bw/2.0);
    *y = by+(bh/2.0);
}


void 
AnnotationItem::snapToGridPoint(int gridSize, double &x,double &y)
{

    //printf ("## snapping wc(%f,%f)\n",x,y);

    // round
    int dx = (int)floor(x);
    int dy = (int)floor(y);
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
    dx = ( (abs(gx1-dx)) > (abs(gx2-dx)) ) ? gx2 : gx1;
    dy = ( (abs(gy1-dy)) > (abs(gy2-dy)) ) ? gy2 : gy1;

    //printf ("## choosen wc(%f,%f)\n",dx,dy);
    x = (double)dx;
    y = (double)dy;

}

bool
AnnotationItem::pointNearSegment (double x, double y,
    double x1, double y1, double x2, double y2,double scf_x,double scf_y) const
{
    //printf ("inner at %f,%f, line is (%f,%f)->(%f,%f)\n",x,y,x1,y1,x2,y2);
    double leftX,upperY,rightX,bottomY;

    leftX   = x1<x2 ? x1 : x2;
    rightX  = x1<x2 ? x2 : x1;
    upperY  = y1<y2 ? y1 : y2;
    bottomY = y1<y2 ? y2 : y1;

    // check for ~vertical lines
    if (fabs(x1-x2)<FP_TOLERANCE)
    {
        return ((fabs(x1-x)<DELTA_MARGIN/scf_x) && (y>=upperY) && (y<=bottomY));
    }

    // check for ~horizontal lines
    if (fabs(y1-y2)<FP_TOLERANCE)
    {
        return ((fabs(y1-y)<DELTA_MARGIN/scf_x) && (x>=leftX) && (x<=rightX));
    }

    // any other line

    // check bounds
    if ((x<leftX)||(x>rightX)) 
    {
        return (false);
    }
    
    if ((y<upperY)||(y>bottomY)) 
    {
        return (false);
    }    

    // compute vector
    double vx = x2-x1;
    double vy = y2-y1;

    // normalize
    double mod = sqrt(vx*vx + vy*vy);
    Q_ASSERT (mod != 0.0);
    vx /= mod;
    vy /= mod;

    // compute perpendicular to such vector
    double pvx = -vy;
    double pvy = vx;

    //printf ("v=(%f,%f) vp=(%f,%f)\n",vx,vy,pvx,pvy);

    // compute intersection between such lines ...
    double a1 = vy/vx;
    double b1 = y1 - a1*x1;
    double a2 = pvy/pvx;
    double b2 = y - a2*x;
    double intX = (b2-b1)/(a1-a2);
    double intY = a2*intX + b2;

    //printf ("a1=%f, b1=%f\n",a1,b1);
    //printf ("a2=%f, b2=%f\n",a2,b2);
    //printf ("intX=%f, intY=%f\n",intX,intY);

    // now compute vector between the given point and the
    // intersection point
    double dvx = x-intX;
    double dvy = y-intY;

    //printf ("dv=(%f,%f)\n",dvx,dvy);

    // scale
    dvx *= scf_x;
    dvy *= scf_y;

    //printf ("scalled dv=(%f,%f)\n",dvx,dvy);

    // compute the size of such vector
    mod = sqrt(dvx*dvx + dvy*dvy);
    //printf ("mod=%f\n",mod);

    return (mod < DELTA_MARGIN);
}

void 
AnnotationItem::newChild(AnnotationItem* child)
{
}

void 
AnnotationItem::detachChild(AnnotationItem* child)
{
}


void 
AnnotationItem::reparent(AnnotationItem *newparent)
{
    //printf("AnnotationItem::reparent this=0x%x, oldparent=0x%x, newparent=0x%x \n",this,parent,newparent);
    if (parent!=NULL)
    {
        parent->detachChild(this);
    }
    
    parent=newparent;
    if (parent) parent->newChild(this);
}

bool
AnnotationItem::innerPoint(double x, double y,double scf_x,double scf_y)
{
    return false;
}

bool
AnnotationItem::innerNodePoint(double x, double y,double scf_x,double scf_y)
{
    return false;
}


void
AnnotationItem::boundingRect(double* rx1, double* ry1, double* w, double* h) const
{
}

void
AnnotationItem::draw(QPainter* p, double scf_x, double scf_y)
{
}

void
AnnotationItem::drawRubber(int px, int py, int x, int y, QPainter* p)
{
}

void
AnnotationItem::mouseWorldPressEvent(int vx, int vy, double wx, double wy)
{
}

void
AnnotationItem::mouseWorldReleaseEvent(int vx, int vy, double wx, double wy)
{
}

void
AnnotationItem::mouseWorldMoveEvent(int vx, int vy, double wx, double wy)
{
}

AnnotationItem* AnnotationItem::clone()
{
    return NULL;
}

void
AnnotationItem::clone(AnnotationItem* item)
{
}

void
AnnotationItem::cloneGeometry(AnnotationItem* item)
{
}

void
AnnotationItem::move(double x, double y)
{
}

void
AnnotationItem::move(double vx, double vy, double dis)
{
}

void
AnnotationItem::moveNode(double nx, double ny, double x, double y, double scf_x, double scf_y)
{
}

void
AnnotationItem::snapToGrid(int gridSize, bool onlyCorner)
{
}

