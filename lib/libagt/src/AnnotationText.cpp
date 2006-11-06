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
  * @file  AnnotationText.cpp
  */

#include "AnnotationText.h"

AnnotationText::AnnotationText(double vx, double vy, QString vtxt,AnnotationItem* _parent) : AnnotationItem(_parent)
{
    x=vx;
    y=vy;
    txt=vtxt;
    if (parent!=NULL)
    {
        parent->newChild(this);
    }
}

AnnotationText::AnnotationText (QRect bbox,QString vtxt,AnnotationItem* _parent) : AnnotationItem(_parent)
{
    //printf("AnnotationText::AnnotationText on rect left=%d,top=%d,width=%d,height=%d\n",bbox.left(),bbox.top(),bbox.width(),bbox.height());
    x=0;
    y=0;
    txtbbox=bbox;
    txt=vtxt;
    if (parent!=NULL)
    {
        parent->newChild(this);
    }
}

AnnotationText::~AnnotationText()
{
}


bool
AnnotationText::innerPoint(double x, double y,double scf_x,double scf_y)
{
    return false;
    /*
    if (!selectable) return false;
    if (txtbbox.isValid())
    {
        return txtbbox.contains((int)x,(int)y);
    }
    else
    {
        return false;
    }
    */
}

bool
AnnotationText::innerNodePoint(double x, double y,double scf_x,double scf_y)
{
    /// @todo fix this
    return false;
    
    //printf ("innerNodePoint called x=%f,y=%f\n",x,y);fflush(stdout);
    //printf ("\t line is (%f,%f)->(%f,%f)\n",x1,y1,x2,y2);fflush(stdout);
    /*
    if (!selected) 
    {
        return (false);
    }
    */    
    // check area arround node selectors
    //bool c1 = innerNodePointArround(x,y,scf_x,scf_y,x1,y1);
    //bool c2 = innerNodePointArround(x,y,scf_x,scf_y,x2,y2);
    //printf("\t c1=%d, c2=%d\n",(INT32)c1,(INT32)c2);fflush(stdout);
    //return (c1||c2);
}

void
AnnotationText::boundingRect(double* rx1, double* ry1, double* w, double* h) const
{
    if (txtbbox.isValid())
    {
        *rx1 = (double)txtbbox.left(); 
        *ry1 = (double)txtbbox.top(); 
        *w = (double)txtbbox.width(); 
        *h = (double)txtbbox.height(); 
    }
    else
    {
        *w=-1.0; // signal invalid bbox...
        *h=-1.0;
    }
}

void
AnnotationText::draw(QPainter* p, double scf_x, double scf_y)
{
    if (!visible) 
    {
        return;
    }    


    QBrush old = p->brush();

    p->setBrush(myBrush);
    p->setPen(myPen);

    // draw the base line
    if (txtbbox.isValid())
    {
        //printf ("AnnotationText::draw called, bbox x=%d,y=%d, w=%d, h=%d\n",txtbbox.x(),txtbbox.y(),txtbbox.width(),txtbbox.bottom());
        p->drawText(txtbbox, Qt::AlignHCenter|Qt::AlignVCenter,txt);
    }
    else
    {
        //printf ("AnnotationText::draw called, x1=%f,y1=%f\n",x,y);
        p->drawText((int)floor(x),(int)floor(y),txt);
    }

    // draw (if needed)
    if (selected)
    {
        //drawSelectionNode(x1,y1,p,scf_x,scf_y);
        //drawSelectionNode(x2,y2,p,scf_x,scf_y);
    }

    p->setBrush(old);
}

void
AnnotationText::drawRubber(int px, int py, int x, int y, QPainter* p)
{
    //p->drawLine(px,py,x,y);
}

void
AnnotationText::mouseWorldPressEvent(int vx, int vy, double wx, double wy)
{
    //x1=wx;
    //y1=wy;
    //printf ("AnnLine::staring point at %f,%f\n",x1,y1);
}

void
AnnotationText::mouseWorldReleaseEvent(int vx, int vy, double wx, double wy)
{
    //x2=wx;
    //y2=wy;
    //printf ("AnnLine::ending point at %f,%f\n",x2,y2);
}

void
AnnotationText::mouseWorldMoveEvent(int vx, int vy, double wx, double wy)
{
}

AnnotationItem* AnnotationText::clone()
{
    AnnotationText* nl=NULL;
    if (txtbbox.isValid())
    {
        nl = new AnnotationText(txtbbox,txt,parent);
    }
    else
    {
        nl = new AnnotationText(x,y,txt,parent);
    }
    Q_ASSERT(nl!=NULL);
    nl->setPen(myPen);
    nl->setBrush(myBrush);
    return (nl);
}

void
AnnotationText::clone(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationText* line = (AnnotationText*) item;
    line->x=x;
    line->y=y;
    line->txt=txt;
    line->myPen = myPen;
    line->myBrush = myBrush;
}

void
AnnotationText::cloneGeometry(AnnotationItem* item)
{
    Q_ASSERT(item!=NULL);
    AnnotationText* line = (AnnotationText*) item;
    line->x=x;
    line->y=y;
}

void
AnnotationText::move(double dx, double dy)
{
    if (txtbbox.isValid())
    {
        txtbbox.moveBy((int)dx,(int)dy);
    }
    else
    {
        x += dx;
        y += dy;
    }
}

void
AnnotationText::move(double vx, double vy, double dis)
{
    // normalize the vector
    double mod = sqrt(vx*vx +  vy*vy);
    if (mod == 0.0) return;
    vx /= mod;
    vy /= mod;

    if (txtbbox.isValid())
    {
        txtbbox.moveBy((int)(dis*vx),(int)(dis*vy));
    }
    else
    {
        x += dis*vx;
        y += dis*vy;
    }
}

void
AnnotationText::moveNode(double nx, double ny, double x, double y, double scf_x, double scf_y)
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
AnnotationText::snapToGrid(int gridSize, bool onlyCorner)
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

