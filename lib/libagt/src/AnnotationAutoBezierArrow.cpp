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
  * @file  AnnotationAutoBezierArrow.cpp
  */

#include "AnnotationAutoBezierArrow.h"

BYTE AnnotationAutoBezierArrow::flipping = 0;

AnnotationAutoBezierArrow::AnnotationAutoBezierArrow(double vx1, double vy1, double vx2, double vy2,AnnotationItem* _parent) : AnnotationItem(_parent)
{
    bool flipped = false;

    x1=vx1;
    x2=vx2;
    y1=vy1;
    y2=vy2;

    double cx0,cx1,cx2,cx3;
    double cy0,cy1,cy2,cy3;

    cx0 = (double)vx1;
    cy0 = (double)vy1;
    cx3 = (double)vx2;
    cy3 = (double)vy2;

    parray.resize(4);

    // 1) compute vector
    double vx,vy;
    vx = vx2-vx1;
    vy = vy2-vy1;

    // 2) length
    double vlen = sqrt(vx*vx + vy*vy);
    //printf ("vlen=%g\n",vlen);
    // 3) normalize vector
    vx /= vlen;
    vy /= vlen;

    // 4) ortogonal vector
    double nvx,nvy;
    nvy = vx;
    nvx = -vy;
    bool isVertical = (fabs(nvx)<FP_VH_TOLERANCE)&&(fabs(fabs(nvy)-1.0)<FP_VH_TOLERANCE);
    //printf ("nvx=%g, nvy=%g, isVertical=%d\n",nvx,nvy,(int)isVertical);
    if (isVertical)
    {
        flipping = (flipping+1)%2;
    }
    if ((vy1<vy2) || (isVertical && flipping) )
    {
        flipped = true;
        QWMatrix rot180;
        rot180.rotate(180);
        double tmpx,tmpy;
        rot180.map(nvx,nvy,&tmpx,&tmpy);
        nvx = tmpx;
        nvy = tmpy;
    }


    // 5) compute midle point
    double pcx,pcy;
    pcx = vx1 + (vx*vlen/2.0);
    pcy = vy1 + (vy*vlen/2.0);

    // 6) shift it
    double spcx,spcy;
    if (vlen<=10)
    {
        spcx = pcx + (nvx*vlen*2.5);
        spcy = pcy + (nvy*vlen*2.5);
    }
    else if (vlen<=30)
    {
        spcx = pcx + (nvx*vlen);
        spcy = pcy + (nvy*vlen);
    }
    else if (vlen>75)
    {
        spcx = pcx + (nvx*30.0);
        spcy = pcy + (nvy*30.0);
    }
    else
    {
        spcx = pcx + (nvx*vlen/2.5);
        spcy = pcy + (nvy*vlen/2.5);
    }
    // 7) make translation to (0,0)
    double tspcx,tspcy;
    QWMatrix mtrans;
    mtrans.translate(-pcx,-pcy);
    mtrans.map(spcx,spcy,&tspcx,&tspcy);

    // 8) rotations
    double acx1,acx2,acy1,acy2;
    QWMatrix matx1;
    QWMatrix matx2;
    matx1.rotate(45);
    matx1.map(tspcx,tspcy,&acx1,&acy1);
    matx2.rotate(-45);
    matx2.map(tspcx,tspcy,&acx2,&acy2);

    // 9) translation back to original position
    mtrans.reset();
    mtrans.translate(pcx,pcy);
    mtrans.map(acx1,acy1,&cx1,&cy1);
    mtrans.map(acx2,acy2,&cx2,&cy2);

    // 10) set the QT object
    parray.setPoint(0,(int)cx0,(int)cy0);
    if (flipped)
    {
        parray.setPoint(2,(int)cx1,(int)cy1);
        parray.setPoint(1,(int)cx2,(int)cy2);
    }
    else
    {
        parray.setPoint(1,(int)cx1,(int)cy1);
        parray.setPoint(2,(int)cx2,(int)cy2);
    }
    parray.setPoint(3,(int)cx3,(int)cy3);

    // 11) compute arrow segments
    double avx,avy;
    if (flipped)
    {
        avx = cx1-cx3;
        avy = cy1-cy3;
    }
    else
    {
        avx = cx2-cx3;
        avy = cy2-cy3;
    }
    vlen = sqrt(avx*avx + avy*avy);
    avx /= vlen;
    avy /= vlen;
    double apx,apy,tapx,tapy;
    apx = cx3 + (avx*4.0);
    apy = cy3 + (avy*4.0);
    mtrans.reset();
    mtrans.translate(-cx3,-cy3);
    matx1.reset();
    matx2.reset();
    matx1.rotate(35);
    matx2.rotate(-35);
    mtrans.map(apx,apy,&tapx,&tapy);
    double tarrx0,tarrx1,tarry0,tarry1;
    matx1.map(tapx,tapy,&tarrx0,&tarry0);
    matx2.map(tapx,tapy,&tarrx1,&tarry1);
    mtrans.reset();
    mtrans.translate(cx3,cy3);
    mtrans.map(tarrx0,tarry0,&arrx0,&arry0);
    mtrans.map(tarrx1,tarry1,&arrx1,&arry1);

    if (parent!=NULL)
    {
        parent->newChild(this);
    }
}

AnnotationAutoBezierArrow::~AnnotationAutoBezierArrow()
{
}


bool
AnnotationAutoBezierArrow::innerPoint(double x, double y,double scf_x,double scf_y)
{
    /// @todo implement this method
    return (false);

    //if (!selectable) return false;
    //return (pointNearSegment(x,y,x1,y1,x2,y2,scf_x,scf_y));
}

bool
AnnotationAutoBezierArrow::innerNodePoint(double x, double y,double scf_x,double scf_y)
{
    /// @todo implement this method
    return (false);

    /*
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
    */
}


void
AnnotationAutoBezierArrow::boundingRect(double* rx1, double* ry1, double* w, double* h) const
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
AnnotationAutoBezierArrow::draw(QPainter* p, double scf_x, double scf_y)
{
    if (!visible)
    {
        return;
    }

    //printf ("AnnotationAutoBezierArrow::draw called, x1=%f,y1=%f,x2=%f,y2=%f\n",x1,y1,x2,y2);

    QBrush old = p->brush();

    p->setBrush(myBrush);
    p->setPen(myPen);

    // draw the curve
    p->drawCubicBezier(parray);

    // draw the arrow ending
    p->drawLine(parray[3].x(),parray[3].y(),(int)arrx0,(int)arry0);
    p->drawLine(parray[3].x(),parray[3].y(),(int)arrx1,(int)arry1);

    // draw (if needed)
    if (selected)
    {
        drawSelectionNode(x1,y1,p,scf_x,scf_y);
        drawSelectionNode(x2,y2,p,scf_x,scf_y);
    }

    p->setBrush(old);
}

void
AnnotationAutoBezierArrow::drawRubber(int px, int py, int x, int y, QPainter* p)
{
    /// @todo implement this method
    //p->drawLine(px,py,x,y);
}

void
AnnotationAutoBezierArrow::mouseWorldPressEvent(int vx, int vy, double wx, double wy)
{
    x1=wx;
    y1=wy;
    //printf ("AnnLine::staring point at %f,%f\n",x1,y1);
}

void
AnnotationAutoBezierArrow::mouseWorldReleaseEvent(int vx, int vy, double wx, double wy)
{
    x2=wx;
    y2=wy;
    //printf ("AnnLine::ending point at %f,%f\n",x2,y2);
}

void
AnnotationAutoBezierArrow::mouseWorldMoveEvent(int vx, int vy, double wx, double wy)
{
}

AnnotationItem* AnnotationAutoBezierArrow::clone()
{
    AnnotationAutoBezierArrow* nl = new AnnotationAutoBezierArrow(x1,y1,x2,y2,parent);
    Q_ASSERT(nl!=NULL);
    nl->setPen(myPen);
    nl->setBrush(myBrush);
    return (nl);
}

void
AnnotationAutoBezierArrow::clone(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationAutoBezierArrow* line = (AnnotationAutoBezierArrow*) item;
    line->x1=x1;
    line->y1=y1;
    line->x2=x2;
    line->y2=y2;
    line->myPen = myPen;
    line->myBrush = myBrush;
}

void
AnnotationAutoBezierArrow::cloneGeometry(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationAutoBezierArrow* line = (AnnotationAutoBezierArrow*) item;
    line->x1=x1;
    line->y1=y1;
    line->x2=x2;
    line->y2=y2;
}

void
AnnotationAutoBezierArrow::move(double x, double y)
{
    x1 += x;
    x2 += x;
    y1 += y;
    y2 += y;
}

void
AnnotationAutoBezierArrow::move(double vx, double vy, double dis)
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
AnnotationAutoBezierArrow::moveNode(double nx, double ny, double x, double y, double scf_x, double scf_y)
{
    /// @todo implement this method
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
AnnotationAutoBezierArrow::snapToGrid(int gridSize, bool onlyCorner)
{
    /// @todo implement this method
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

