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
  * @file  AnnotationLine.cpp
  */

#include "AnnotationLine.h"

AnnotationLine::AnnotationLine(double vx1, double vy1, double vx2, double vy2,AnnotationItem* _parent) : AnnotationItem(_parent)
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

AnnotationLine::~AnnotationLine()
{
}


bool
AnnotationLine::innerPoint(double x, double y,double scf_x,double scf_y)
{
    if (!selectable) return false;
    return (pointNearSegment(x,y,x1,y1,x2,y2,scf_x,scf_y));
}

bool
AnnotationLine::innerNodePoint(double x, double y,double scf_x,double scf_y)
{
    if (parent!=NULL) return parent->innerNodePoint(x,y,scf_x,scf_y);

    //printf ("innerNodePoint called x=%f,y=%f\n",x,y);fflush(stdout);
    //printf ("\t line is (%f,%f)->(%f,%f)\n",x1,y1,x2,y2);fflush(stdout);
    if (!selected) 
    {
        return (false);
    }
        
    // check area arround node selectors
    bool c1 = innerNodePointArround(x,y,scf_x,scf_y,x1,y1);
    bool c2 = innerNodePointArround(x,y,scf_x,scf_y,x2,y2);
    //printf("\t c1=%d, c2=%d\n",(INT32)c1,(INT32)c2);fflush(stdout);
    return (c1||c2);
}


void
AnnotationLine::boundingRect(double* rx1, double* ry1, double* w, double* h) const
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
AnnotationLine::draw(QPainter* p, double scf_x, double scf_y)
{
    if (!visible) 
    {
        return;
    }    

    //printf ("AnnotationLine::draw called, x1=%f,y1=%f,x2=%f,y2=%f\n",x1,y1,x2,y2);

    QBrush old = p->brush();

    p->setBrush(myBrush);
    p->setPen(myPen);

    // draw the base line
    p->drawLine((int)floor(x1),(int)floor(y1),(int)floor(x2),(int)floor(y2));

    // draw (if needed)
    if (selected)
    {
        drawSelectionNode(x1,y1,p,scf_x,scf_y);
        drawSelectionNode(x2,y2,p,scf_x,scf_y);
    }

    p->setBrush(old);
}

void
AnnotationLine::drawRubber(int px, int py, int x, int y, QPainter* p)
{
    p->drawLine(px,py,x,y);
}

void
AnnotationLine::mouseWorldPressEvent(int vx, int vy, double wx, double wy)
{
    if (parent!=NULL)
    {
        parent->mouseWorldPressEvent(vx,vy,wx,wy);
        return;
    }

    x1=wx;
    y1=wy;
    //printf ("AnnLine::staring point at %f,%f\n",x1,y1);
}

void
AnnotationLine::mouseWorldReleaseEvent(int vx, int vy, double wx, double wy)
{
    if (parent!=NULL)
    {
        parent->mouseWorldReleaseEvent(vx,vy,wx,wy);
        return;
    }

    x2=wx;
    y2=wy;
    //printf ("AnnLine::ending point at %f,%f\n",x2,y2);
}

void
AnnotationLine::mouseWorldMoveEvent(int vx, int vy, double wx, double wy)
{
}

AnnotationItem* AnnotationLine::clone()
{
    AnnotationLine* nl = new AnnotationLine(x1,y1,x2,y2,parent);
    Q_ASSERT(nl!=NULL);
    nl->setPen(myPen);
    nl->setBrush(myBrush);
    return (nl);
}

void
AnnotationLine::clone(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationLine* line = (AnnotationLine*) item;
    line->x1=x1;
    line->y1=y1;
    line->x2=x2;
    line->y2=y2;
    line->myPen = myPen;
    line->myBrush = myBrush;
}

void
AnnotationLine::cloneGeometry(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationLine* line = (AnnotationLine*) item;
    line->x1=x1;
    line->y1=y1;
    line->x2=x2;
    line->y2=y2;
}

void
AnnotationLine::move(double x, double y)
{
    x1 += x;
    x2 += x;
    y1 += y;
    y2 += y;
}

void
AnnotationLine::move(double vx, double vy, double dis)
{
    // normalize the vector
    double mod = sqrt(vx*vx +  vy*vy);
    if (mod == 0.0) return;
    vx /= mod;
    vy /= mod;
    x1 += dis*vx;
    y1 += dis*vy;
    x2 += dis*vx;
    y2 += dis*vy;
}

void
AnnotationLine::moveNode(double nx, double ny, double x, double y, double scf_x, double scf_y)
{
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
}

void
AnnotationLine::snapToGrid(int gridSize, bool onlyCorner)
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

