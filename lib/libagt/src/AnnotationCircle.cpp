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
  * @file  AnnotationCircle.cpp
  */

#include "AnnotationCircle.h"

AnnotationCircle::AnnotationCircle(double vx1, double vy1, double vx2, double vy2,AnnotationItem* _parent) : AnnotationItem(_parent)
{
    x1=vx1;
    x2=vx2;
    y1=vy1;
    y2=vy2;
    if (parent!=NULL)
    {
        parent->newChild(this);
    }
}

AnnotationCircle::~AnnotationCircle()
{
}

bool 
AnnotationCircle::innerPoint(double x, double y,double scf_x,double scf_y)
{
    if (!selectable) return false;

    double leftX,upperY,rightX,bottomY;

    leftX   = x1<x2 ? x1 : x2;
    rightX  = x1<x2 ? x2 : x1;
    upperY  = y1<y2 ? y1 : y2;
    bottomY = y1<y2 ? y2 : y1;

    // compute rad1 & rad2 for the given ellipse
    double rad1 = (rightX-leftX)/2.0;
    double rad2 = (bottomY-upperY)/2.0;
    double cx = leftX+rad1;
    double cy = upperY+rad2;
    //printf (">>x=%f, y=%f, rad1=%f, rad2=%f,cx=%f, cy=%f\n",x,y,rad1,rad2,cx,cy);
    // compute angle
    double qx = x-cx;
    double qy = y-cy;
    double ange = atan2(qy,qx);
    double ang  = atan((rad1/rad2)*tan(ange));
    if (fabs(ang)/ang !=fabs(ange)/ange)
    {
        ang = ang+pi;
    }

    //printf ("ange=%f\n",180.0/pi*ange);
    //printf ("ang=%f\n",180.0/pi*ang);

    // calc intersectin point
    double px = cx + rad1*cos(ang);
    double py = cy + rad2*sin(ang);
    //printf (">>px=%f, py=%f\n",px,py);

    if (myBrush.style()==NoBrush)
    {
        // compute distance between px (intersec) and given point
        double vx = x-px;
        double vy = y-py;
        double mod = sqrt(vx*vx + vy*vy);
        //printf (">>v=(%f,%f), mod=%f\n",vx,vy,mod);
        return (mod <= DELTA_MARGIN);
    }
    else
    {
        // compute distance between px (intersec) respect to center
        double v1x = px-cx;
        double v1y = py-cy;
        double d1 = sqrt (v1x*v1x + v1y*v1y);
        // compute distance between give point respect to center
        double d2 = sqrt (qx*qx + qy*qy);

        return (d2<=d1);
    }
}


bool 
AnnotationCircle::innerNodePoint(double x, double y,double scf_x,double scf_y)
{
    //printf ("innerNodePoint called x=%f,y=%f\n",x,y);fflush(stdout);
    //printf ("\t line is (%f,%f)->(%f,%f)\n",x1,y1,x2,y2);fflush(stdout);
    if (!selected)
    {
        return (false);
    }

    // check area arround node selectors
    double leftX,upperY,rightX,bottomY;
    leftX   = x1<x2 ? x1 : x2;
    rightX  = x1<x2 ? x2 : x1;
    upperY  = y1<y2 ? y1 : y2;
    bottomY = y1<y2 ? y2 : y1;

    double node_gap_v = NODE_GAP;
    double node_gap_h = NODE_GAP;
    node_gap_v /= scf_y;
    node_gap_h /= scf_x;
    bool c1 = innerNodePointArround(x,y,scf_x,scf_y,floor(leftX-node_gap_h),floor(upperY-node_gap_v));
    bool c2 = innerNodePointArround(x,y,scf_x,scf_y,floor(rightX+node_gap_h),floor(bottomY+node_gap_v));
    bool c3 = innerNodePointArround(x,y,scf_x,scf_y,floor(leftX-node_gap_h),floor(bottomY+node_gap_v));
    bool c4 = innerNodePointArround(x,y,scf_x,scf_y,floor(rightX+node_gap_h),floor(upperY-node_gap_v));
    return (c1||c2||c3||c4);
}


void 
AnnotationCircle::boundingRect(double* rx1, double* ry1, double* w, double* h) const
{
    if (x1<x2)
    {
        *rx1 = x1;
    }
    else
    {
        *rx1 = x2;
    }

    if (y1<y2)
    {
        *ry1 = y1;
    }
    else
    {
        *ry1 = y2;
    }

    *w  = fabs(x2-x1+1);
    *h  = fabs(y2-y1+1);
}

void 
AnnotationCircle::draw(QPainter* p, double scf_x, double scf_y)
{
    if (!visible)
    {
        return;
    }

    //printf ("AnnotationCircle::draw called, x1=%f,y1=%f,x2=%f,y2=%f\n",x1,y1,x2,y2);

    double leftX,upperY,rightX,bottomY;
    leftX   = x1<x2 ? x1 : x2;
    rightX  = x1<x2 ? x2 : x1;
    upperY  = y1<y2 ? y1 : y2;
    bottomY = y1<y2 ? y2 : y1;

    QBrush old = p->brush();

    p->setBrush(myBrush);
    p->setPen(myPen);

    // draw the base line
    p->drawChord((int)floor(leftX),(int)floor(upperY),
    (int)floor(rightX-leftX+1.0),(int)floor(bottomY-upperY+1.0),0,(16*360));

    // draw (if needed)
    if (selected)
    {
        double leftX,upperY,rightX,bottomY;

        leftX   = x1<x2 ? x1 : x2;
        rightX  = x1<x2 ? x2 : x1;
        upperY  = y1<y2 ? y1 : y2;
        bottomY = y1<y2 ? y2 : y1;

        double node_gap_v = NODE_GAP;
        double node_gap_h = NODE_GAP;
        node_gap_v /= scf_y;
        node_gap_h /= scf_x;

        drawSelectionNode((int)floor(leftX-node_gap_h),(int)floor(upperY-node_gap_v),p,scf_x,scf_y);
        drawSelectionNode((int)floor(leftX-node_gap_h),(int)floor(bottomY+node_gap_v),p,scf_x,scf_y);
        drawSelectionNode((int)floor(rightX+node_gap_h),(int)floor(bottomY+node_gap_v),p,scf_x,scf_y);
        drawSelectionNode((int)floor(rightX+node_gap_h),(int)floor(upperY-node_gap_v),p,scf_x,scf_y);
    }

    p->setBrush(old);
}

void
AnnotationCircle::drawRubber(int px, int py, int x, int y, QPainter* p)
{
    p->drawChord(px,py,x-px,y-py,0,(16*360));
}

void
AnnotationCircle::mouseWorldPressEvent(int vx, int vy, double wx, double wy)
{
    x1=wx;
    y1=wy;
    //printf ("AnnCircle::staring point at %f,%f\n",x1,y1);
}

void
AnnotationCircle::mouseWorldReleaseEvent(int vx, int vy, double wx, double wy)
{
    x2=wx;
    y2=wy;
    //printf ("AnnCircle::ending point at %f,%f\n",x2,y2);
}

void
AnnotationCircle::mouseWorldMoveEvent(int vx, int vy, double wx, double wy)
{
}

AnnotationItem*
AnnotationCircle::clone()
{
    AnnotationCircle* nl = new AnnotationCircle(x1,y1,x2,y2,parent);
    Q_ASSERT(nl!=NULL);
    nl->setPen(myPen);
    nl->setBrush(myBrush);
    return (nl);
}

void
AnnotationCircle::clone(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationCircle* line = (AnnotationCircle*) item;
    line->x1=x1;
    line->y1=y1;
    line->x2=x2;
    line->y2=y2;
    line->myPen = myPen;
    line->myBrush = myBrush;
}

void
AnnotationCircle::cloneGeometry(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationCircle* line = (AnnotationCircle*) item;
    line->x1=x1;
    line->y1=y1;
    line->x2=x2;
    line->y2=y2;
}

void
AnnotationCircle::move(double x, double y)
{
    x1 += x;
    x2 += x;
    y1 += y;
    y2 += y;
}

void
AnnotationCircle::move(double vx, double vy, double dis)
{
    // normalize the vector
    double mod = sqrt(vx*vx +  vy*vy);
    if (mod == 0.0)
    {
        return;
    }
        
    vx /= mod;
    vy /= mod;
    x1 += dis*vx;
    y1 += dis*vy;
    x2 += dis*vx;
    y2 += dis*vy;
}

void
AnnotationCircle::moveNode(double nx, double ny, double x, double y, double scf_x, double scf_y)
{
    // look for the right node (only two in this case)
    double leftX,upperY,rightX,bottomY;

    leftX   = x1<x2 ? x1 : x2;
    rightX  = x1<x2 ? x2 : x1;
    upperY  = y1<y2 ? y1 : y2;
    bottomY = y1<y2 ? y2 : y1;
    bool swapH = x2<=x1;
    bool swapV = y2<=y1;

    double node_gap_v = NODE_GAP;
    double node_gap_h = NODE_GAP;
    node_gap_v /= scf_y;
    node_gap_h /= scf_x;

    bool c1 = innerNodePointArround(nx,ny,scf_x,scf_y,floor(leftX-node_gap_h),floor(upperY-node_gap_v));
    bool c2 = innerNodePointArround(nx,ny,scf_x,scf_y,floor(rightX+node_gap_h),floor(bottomY+node_gap_v));
    bool c3 = innerNodePointArround(nx,ny,scf_x,scf_y,floor(leftX-node_gap_h),floor(bottomY+node_gap_v));
    bool c4 = innerNodePointArround(nx,ny,scf_x,scf_y,floor(rightX+node_gap_h),floor(upperY-node_gap_v));

    /** @todo review this stupid way of implementig this...*/
    if (c1)
    {
        if (swapH)
            x2+=x;
        else
            x1+=x;

        if (swapV)
            y2+=y;
        else
            y1+=y;
    }
    else if (c2)
    {
        if (swapH)
            x1+=x;
        else
            x2+=x;

        if (swapV)
            y1+=y;
        else
            y2+=y;
    }
    else if (c3)
    {
        if (swapH)
            x2+=x;
        else
            x1+=x;

        if (swapV)
            y1+=y;
        else
            y2+=y;
    }
    else if (c4)
    {
        if (swapH)
            x1+=x;
        else
            x2+=x;

        if (swapV)
            y2+=y;
        else
            y1+=y;
    }
}

void
AnnotationCircle::snapToGrid(int gridSize, bool onlyCorner)
{
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
}


