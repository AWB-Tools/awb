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
  * @file  AScrollView.cpp
  */
#include <iostream>
#include <string.h>
using namespace std;

#include <qapplication.h>
#include <qtooltip.h>

#include "AScrollView.h"

// icons
#include "xpm/zoomingToolBar.xpm"
#include "xpm/zoomingCursor.xpm"
#include "xpm/panningCursor.xpm"
#include "xpm/shadingCursor.xpm"
#include "xpm/mwandCursor.xpm"

AScrollView::AScrollView(QWidget* parent, int width, int height) :
QScrollView(parent,"display", WStaticContents|WResizeNoErase|WRepaintNoErase)
{
    /*
    printf ("starting asv constructor w=%d, h=%d...\n",width,height);
    printf ("sizeof bool=%d\n",sizeof(bool));
    printf ("sizeof qbrush=%d\n",sizeof(QBrush));
    printf ("sizeof qpen=%d\n",sizeof(QPen));
    */

    redrawing = false;
    pi = 4.0*atan(1.0);

    myHorizontalOffset=0;
    myVerticalOffset=0;

    currentPointerType = POINTER_SELECT;
    released_mx=0;
    released_my=0;
    pressed_mx=0;
    pressed_my=0;
    selected_mx=0;
    selected_my=0;
    current_mx=0;
    current_my=0;
    released_wx=0;
    released_wy=0;
    pressed_wx=0;
    pressed_wy=0;
    mdiNumber=-1;
    panning = false;
    zooming = false;
    measuring = false;
    wasScrollByMatrix=false;
    wasScrollByZooming=false;
    shading = false;
    
    lastShaded_mx=-1;
    scf_x = 1.0;
    scf_y = 1.0;
    myZX=100;
    myZY=100;
    bgColor = QColor(220,220,220);

    //printf ("resizing...\n");

    realContentsWidth  = width;
    realContentsHeight = height;
    resizeContents(realContentsWidth,realContentsHeight);

    //printf ("resizing done\n");

    setResizePolicy(Manual);
    viewport()->setBackgroundMode(FixedColor);
    viewport()->setBackgroundColor(bgColor);
    offscr.setOptimization(QPixmap::BestOptim);

    // I want to receive mouse movements...
    viewport()->setMouseTracking(true);

    // default colors
    currentBrush        = QBrush(blue);
    currentPen          = QPen(black,1);
    currentTextColor    = black;
    penColorChId=-1;
    brushColorChId=-1;
    textColorChId=-1;

    gridSize = 10;
    annCtrl = new AnnotationCtrl(this,gridSize,true);
    Q_ASSERT(annCtrl!=NULL);
    currentAnnItem=NULL;
    pressAnnItem=NULL;

    showVGridLines = false;
    showHGridLines = false;

    // default shader colors
    shaderBrush = QBrush(QColor(180,180,180));
    shaderPen = QPen(QColor(100,100,100),1);

    sTFitKeepARatio=true;
    rectZoomKeepARatio=true;

    cshadows=NULL;
    currentPainter=NULL;
    
    tipLabel = NULL;

    zoomLocked=false;
    panLocked=false;
    shadingLocked=true;
    annotationLocked=true;
    
    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(contentsMoving(int,int)));
    
    //tip = new AGTTip(this);
    
    user_min_scale_factor = MIN_SCALE_FACTOR;
    user_max_scale_factor = MAX_SCALE_FACTOR;
   
    mdiLocked = true;
    //printf ("asv constructor done...\n");
}

AScrollView::~AScrollView()
{
    if (cshadows!=NULL)
    {
        delete cshadows;
    }
}

void
AScrollView::initShadeFlags()
{
    int wwidth = (int)ceil(((double)realContentsWidth+1) / QMAX(1.0,getBaseElementSize()));
    if (cshadows!=NULL)
    {
        cshadows->resize(wwidth);
    }
    else
    {
        cshadows = new ColumnShadows (wwidth);
    }
    Q_ASSERT(cshadows!=NULL);
}

void
AScrollView::scaleToFit()
{
    int vw = visibleWidth();
    int vh = visibleHeight();
    int rw = realContentsWidth;
    int rh = realContentsHeight;
    rw = QMAX(rw,1);
    rh = QMAX(rh,1);

    double sx = (double)vw/(double)rw;
    double sy = (double)vh/(double)rh;

    double sc;

    if (sTFitKeepARatio)
    {
        sc = sx < sy ? sx : sy;
        setScallingFactor(sc);
    }
    else
    {
        setScallingFactorX(sx);
        setScallingFactorY(sy);
    }
}

void
AScrollView::scaleToFitHorizontal()
{
    int vw = visibleWidth();
    int rw = realContentsWidth;
    rw = QMAX(rw,1);
    double sx = (double)vw/(double)rw;
    sx -= 0.01;
    if (sTFitKeepARatio)
    {
        setScallingFactor(sx);
    }
    else
    {
        setScallingFactorX(sx);
    }
}

void
AScrollView::scaleToFitVertical()
{
    int vh = visibleHeight();
    int rh = realContentsHeight;
    rh = QMAX(rh,1);
    double sy = (double)vh/(double)rh;
    sy -= 0.01;

    if (sTFitKeepARatio)
    {
        setScallingFactor(sy);
    }
    else
    {
        setScallingFactorY(sy);
    }
}

void
AScrollView::updateScallingFactor (double sx, double sy)
{
    wasScrollByZooming=true;    
    if ((sx < MIN_PROGRESS_SCALE_FACTOR) || (sy < MIN_PROGRESS_SCALE_FACTOR) )
    {
        emit progress_show();
        //if (progressBar)progressBar->setFixedWidth(80);
    }
    else
    {
        emit progress_hide();
    }


    viewport()->setUpdatesEnabled( FALSE );

    // get current center point
    double centerx = (((double)contentsX()+(double)visibleWidth()/2.0)/(double)scf_x);
    double centery = (((double)contentsY()+(double)visibleHeight()/2.0)/(double)scf_y);

    scf_x = sx;
    scf_y = sy;

    centerx *= scf_x;
    centery *= scf_y;

    resizeContents((int)((double)realContentsWidth*scf_x),(int)((double)realContentsHeight*scf_y));
    center((int)centerx,(int)centery);
    viewport()->setUpdatesEnabled( TRUE );
    viewport()->update();

    updateZoomStatusBar();

    emit asvNewScallingFactors(scf_x,scf_y);
    // Since we have recentered the contents we propagate an event for that
    //double ox = ((double)contentsX()/scf_x/getBaseElementSize());
    //double oy = ((double)contentsY()/scf_y/getBaseElementSize());
    //emit asvCentered(ox,oy); 
}

void
AScrollView::updateZoomStatusBar()
{
    int zx = (int)(100.0*scf_x);
    int zy = (int)(100.0*scf_y);
    myZX=zx;
    myZY=zy;
    emit zoomStatusChanged(zx,zy);
}


void
AScrollView::drawPseudoScalledText(QPainter *p, QString str,int flags, int x, int y, int w,int h)
{
    // only works if drawContens has been already called!
    int efx=x;
    int efy=y;
    int efw=w;
    int efh=h;
    QWMatrix mbkp = p->worldMatrix();
    int fsize = (int)floor(QMIN(scf_x,scf_y)*(double)w);
    fsize-=2; // margin

    //QFont fneo = p->font();
    //fneo.setPointSize(fsize);
    QFont fneo = QFont("Courier",fsize,QFont::Light);
    p->setFont(fneo);
    p->setWorldMatrix(preScaleWorldMatrix);
    
    efx = (int)((double)efx*scf_x);
    efy = (int)((double)efy*scf_y);
    efw = (int)((double)efw*scf_x);
    efh = (int)((double)efh*scf_y);

    /*
    cout << "AScrollView::drawPseudoScalledText on x=" << x << " y=" << y ;
    cout << " efx=" << efx << " efy=" << efy << "\n";
    cout << " efw=" << efw << " efh=" << efh << "\n";
    cout << "fsize=" << fsize << " scf_x="<<scf_x << " scf_y=" << scf_y << "\n";
    */
    p->drawText(efx,efy,efw,efh,flags,str);
    p->setWorldMatrix(mbkp);               
}

void
AScrollView::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
    QHBox* qhb;

    if (redrawing)
    { return; }
  
    // add the raw marging if any
    int ocx=cx;
    int ocy=cy;
    cy-=myVerticalOffset;
    cx-=myHorizontalOffset;
  
    /*
    cerr << "drawContents called on ocx " << ocx << " ocy " << ocy << " cw " << cw << " ch " << ch << endl;
    cerr << "\t vOff " << myVerticalOffset << " hOff " << myHorizontalOffset << endl; 
    cerr << "\t cx " << cx << " cy " << cy << endl;
    */

    //blockSignals(true);
    redrawing = true;
    currentPainter = p;

    double ncx = ((double)cx/scf_x);
    double ncy = ((double)cy/scf_y);
    double ncw = ((double)cw/scf_x);
    double nch = ((double)ch/scf_y);

    // for very INT64 redrawing process we put also a progress bar

    double mw = ncw/QMAX(1.0,getBaseElementSize());
    double mh = nch/QMAX(1.0,getBaseElementSize());
    numElementsToRefresh = (int)(mw*mh);
    
    if (numElementsToRefresh>10000)
    {
        emit progress_reinit((int)mw);
    }

    // create the fake painter for double buffering
    QPainter painter;
    ensureOffScrSize(cw+myHorizontalOffset,ch+myVerticalOffset);
    painter.begin(&offscr,true);
    painter.setBackgroundColor(bgColor);
    painter.fillRect(0,0,cw,ch,bgColor);
    painter.translate(-(double)QMAX(0,cx),-(double)QMAX(0,cy));
    preScaleWorldMatrix = painter.worldMatrix();
    painter.scale(scf_x,scf_y);

    // compute column area
    int x1 = (int)ncx - ((int)ncx%(int)QMAX(1.0,getBaseElementSize()));
    int x2 = (int)ncx + (int)ncw+2*((int)ncx%(int)QMAX(1.0,getBaseElementSize()));
    int y1 = (int)ncy - ((int)ncy%(int)QMAX(1.0,getBaseElementSize()));
    int y2 = (int)ncy + (int)nch+2*((int)ncy%(int)QMAX(1.0,getBaseElementSize()));
    x1 -= 10;x2 += 10;y1 -= 10;y2 += 10;
    if (x1<0) x1=0;
    if (y1<0) y1=0;
    if (x2>=realContentsWidth) x2=realContentsWidth-1;
    if (y2>=realContentsHeight) y2=realContentsHeight-1;

    // column shading
    drawShadingColumns(x1,x2,y1,y2,&painter);

    // call implementation dependent repaint routine
    bool rok = drawWorldCoordinatesContents(&painter,(int)floor(ncx),
            (int)floor(ncy), (int)ceil(ncw),(int)ceil(nch));

    if (rok)
    {
        // grid line
        if (showVGridLines)
        {
            drawVGridLines(&painter,ncx,ncy,ncw,nch);
        }

        if (showHGridLines)
        {
            drawHGridLines(&painter,ncx,ncy,ncw,nch);
        }

        // redraw annotation tools if any
        annCtrl->drawArea(&painter,ncx,ncy,ncw,nch,scf_x,scf_y);

        // bit copy of pixmap inside the original widget
        painter.end();
        int rox=ocx;
        int roy=ocy;
        if (ocx<myHorizontalOffset)
        {
            rox = myHorizontalOffset;
        }
        if (ocy<myVerticalOffset)
        {
            //cerr << "adjusting paint origin: init_roy " << roy;
            roy = myVerticalOffset;
        }
        p->drawPixmap(rox,roy,offscr,0,0,cw,ch);
    }

    // ensure background on original painter (this is needed if headers have been set)
    if (myVerticalOffset)
    {
        p->fillRect(0,0,cw,myVerticalOffset,bgColor);
    }
    if (myHorizontalOffset)
    {
        p->fillRect(0,0,myHorizontalOffset,ch,bgColor);
    }

    emit progress_hide();
    redrawing = false;
    //blockSignals(false);
}

void
AScrollView::drawShadingColumns(int x1,int x2, int y1, int y2, QPainter* painter)
{
    //printf ("drawShadingColumns called (%d,%d)->(%d,%d)\n",x1,y1,x2,y2);

    int mcol0,mcol1,mcol;
    int sz = (int)QMAX(1.0,getBaseElementSize());
    int heigh = y2-y1+1;
    painter->setPen(shaderPen);
    painter->setBrush(shaderBrush);

    mcol0 = x1/sz;
    mcol1 = x2/sz;
    mcol  = mcol0;

    while (mcol<=mcol1)
    {
        if (getShadeFlag(mcol)) 
        {
            painter->drawRect(mcol*sz,y1,sz,heigh);
        }    
        ++mcol;
    }
}

void 
AScrollView::drawVGridLines(QPainter* painter,double ncx, double ncy, double ncw, double nch)
{
    RasterOp bkpOp = painter->rasterOp();

    painter->setPen(QPen(black,0,DotLine));
    painter->setBrush(NoBrush);
    painter->setRasterOp(NotROP);

    int gridStep = (int)(getBaseElementSize()*gridSize);
    
    // round
    double dx = floor(ncx);
    double dy = floor(ncy);

    // compute nearest grid lines to the left/up
    int bsize = (int)floor((double)gridStep);
    int offX  = ((int)dx)%bsize;
    int offY  = ((int)dy)%bsize;
    int gx1 = (int)dx - offX;
    int gy1 = (int)dy - offY;


    int i=gx1;
    while (i<(ncx+ncw))
    {
        painter->drawLine(i,gy1,i,(int)(ncy+nch+10.0));
        i+=gridStep;
    }
    painter->setRasterOp(bkpOp);
}

void 
AScrollView::drawHGridLines(QPainter* painter,double ncx, double ncy, double ncw, double nch)
{
    RasterOp bkpOp = painter->rasterOp();

    painter->setPen(QPen(black,0,DotLine));
    painter->setBrush(NoBrush);
    painter->setRasterOp(NotROP);

    int gridStep = (int)(getBaseElementSize()*gridSize);

    // round
    double dx = floor(ncx);
    double dy = floor(ncy);

    // compute nearest grid lines to the left/up
    int bsize = (int)floor((double)gridStep);
    int offX  = ((int)dx)%bsize;
    int offY  = ((int)dy)%bsize;
    int gx1 = (int)dx - offX;
    int gy1 = (int)dy - offY;

    int j=gy1;
    while (j<(ncy+nch))
    {
        painter->drawLine(gx1,j,(int)(ncx+ncw+10.0),j);
        j+=gridStep;
    }
    painter->setRasterOp(bkpOp);
}

// -------------------------------------------------------------------
// -------------------------------------------------------------------
// Mouse press related functions
// -------------------------------------------------------------------
// -------------------------------------------------------------------

void 
AScrollView::mouseWorldPressEvent(QMouseEvent* e, double x, double y)
{
    setFocus();
    
    pressed_wx = x;
    pressed_wy = y;
    pressed_mx = (int)(x/QMAX(1.0,getBaseElementSize()));
    pressed_my = (int)(y/QMAX(1.0,getBaseElementSize()));

    // snap to column:
    double unsc_x = (double)(pressed_mx*getBaseElementSize())*scf_x;
    int cy;
    contentsToViewport((int)unsc_x,0,pressed_snaped_x,cy);

    //printf ("press event world coordinates v=(%d,%d), w=(%f,%f), m=(%d,%d), snapped_x=%d\n",e->x(),e->y(),pressed_wx,pressed_wy,pressed_mx,pressed_my,pressed_snaped_x);

    // check pointer types:
    switch (currentPointerType)
    {
        case POINTER_SELECT:
        mouseWorldPressEvent_Select(e,x,y);
        break;

        case POINTER_MWAND:
        mouseWorldPressEvent_MWand(e,x,y);
        break;

        case POINTER_PANNING:
        mouseWorldPressEvent_Panning(e,x,y);
        break;

        case POINTER_ZOOMING:
        case POINTER_ZOOMING_NWIN:
        mouseWorldPressEvent_Zooming(e,x,y);
        break;

        case POINTER_DISTANCE:
        mouseWorldPressEvent_Distance(e,x,y);
        break;

        case POINTER_SHADING: 
        mouseWorldPressEvent_Shading(e,x,y);
        break;

        default:    // annotation tools
        mouseWorldPressEvent_Annotation(e,x,y);
    }

}


void 
AScrollView::mouseWorldPressEvent_Select(QMouseEvent* e, double x, double y)
{
    // check for selection of annotation items
    pressAnnItem = annCtrl->lookAnnItemAt(x,y,scf_x,scf_y);
    if (pressAnnItem!=NULL)
    {
        annCtrl->mouseWorldPressEvent(e, pressAnnItem, x, y,scf_x,scf_y);
    }
    else
    {
        // check if we gonna dragge an item node
        //printf ("checking for node selectors...\n");fflush(stdout);
        pressAnnItem = annCtrl->lookAnnItemNodeAt(x,y,scf_x,scf_y);
        if ((currentAnnItem!=NULL)&&(pressAnnItem==currentAnnItem))
        {
            //printf ("I gonna dragg a node selector...\n");fflush(stdout);
            annCtrl->mouseWorldPressEvent(e, pressAnnItem, x, y,scf_x,scf_y);
        }
    }
}

void 
AScrollView::mouseWorldPressEvent_MWand(QMouseEvent* e, double x, double y)
{}

void 
AScrollView::mouseWorldPressEvent_Panning(QMouseEvent* e, double x, double y)
{
    lastPan_x = e->x();
    lastPan_y = e->y();
    panning = true;
}

void 
AScrollView::mouseWorldPressEvent_Zooming(QMouseEvent* e, double x, double y)
{
    // we use viewport coordinates:
    lastZooming_x = e->x();
    lastZooming_y = e->y();
    zooming = true;
}

void 
AScrollView::mouseWorldPressEvent_Distance(QMouseEvent* e, double x, double y)
{
    // we use viewport coordinates:
    lastDistance_x = pressed_snaped_x;
    lastDistance_y = e->y();
    lastDistance_mx = pressed_mx;
    measuring = true;
}

void 
AScrollView::mouseWorldPressEvent_Shading(QMouseEvent* e, double x, double y)
{
    // we use viewport coordinates:
    lastShading_mx = pressed_mx;
    shading = true;
    switchShadeFlag(pressed_mx);
}

void 
AScrollView::mouseWorldPressEvent_Annotation(QMouseEvent* e, double x, double y)
{
        if (currentAnnItem!=NULL)
        {
            currentAnnItem->setSelected(false);
            annCtrl->repaintAnnItem(currentAnnItem,scf_x,scf_y);
        }

        currentAnnItem = newAnnItem();
        currentAnnItem->setBrush(currentBrush);
        currentAnnItem->setPen(currentPen);
        annCtrl->mouseWorldPressEvent(e, currentAnnItem, x, y,scf_x,scf_y);
        return;
}


// -------------------------------------------------------------------
// -------------------------------------------------------------------
// Mouse release related functions
// -------------------------------------------------------------------
// -------------------------------------------------------------------

void 
AScrollView::mouseWorldReleaseEvent(QMouseEvent* e, double x, double y)
{
    released_wx = x;
    released_wy = y;
    released_mx = (int)(x/QMAX(1.0,getBaseElementSize()));
    released_my = (int)(y/QMAX(1.0,getBaseElementSize()));

    //printf ("release event world coordinates (%f,%f)\n",x,y);

    // check pointer types:
    switch (currentPointerType)
    {
        case POINTER_SELECT:
        mouseWorldReleaseEvent_Select(e,x,y);
        break;

        case POINTER_MWAND:
        mouseWorldReleaseEvent_MWand(e,x,y);
        break;

        case POINTER_PANNING:
        mouseWorldReleaseEvent_Panning(e,x,y);
        break;

        case POINTER_ZOOMING:
        mouseWorldReleaseEvent_Zooming(e,x,y);
        break;

        case POINTER_DISTANCE:
        mouseWorldReleaseEvent_Distance(e,x,y);
        break;

        case POINTER_SHADING:
        mouseWorldReleaseEvent_Shading(e,x,y);
        break;

        case POINTER_ZOOMING_NWIN:
        mouseWorldReleaseEvent_ZoomingNWin(e,x,y);
        break;

        default:    // annotation tools
        mouseWorldReleaseEvent_Annotation(e,x,y);
    }
    
    // remove potential tips 
    cleanTip();
}

void
AScrollView::mouseWorldReleaseEvent_Select (QMouseEvent* e, double x, double y)
{
    bool adding  = e->state() & ControlButton;
    bool shifted = e->state() & ShiftButton;

    // check for selection/movement of annotiation items
    if (pressAnnItem!=NULL)
    {
        //printf ("select/move over @=0x%lx\n",pressAnnItem);
        annCtrl->mouseWorldReleaseEvent(e,pressAnnItem,x,y,scf_x,scf_y);
        // check selection changed
        if (pressAnnItem!=currentAnnItem)
        {
            if (currentAnnItem!=NULL)
            {
                currentAnnItem->setSelected(false);
                annCtrl->repaintAnnItem(currentAnnItem,scf_x,scf_y);
            }
            currentAnnItem = pressAnnItem;  // enable/switch selection
            currentAnnItem->setSelected(true);
            annCtrl->repaintAnnItem(currentAnnItem,scf_x,scf_y);
            if (!adding)
            {
                contentsSelectEvent(-1,-1,false,false);
            }
            emit currentAnnItemChanged(currentAnnItem);
        }
    }
    else if ((released_mx == pressed_mx) && (released_my == pressed_my) )
    {
        // unselect any selected annotation...
        if (currentAnnItem!=NULL)
        {
            currentAnnItem->setSelected(false);
            annCtrl->repaintAnnItem(currentAnnItem,scf_x,scf_y);
            currentAnnItem = NULL;
            emit currentAnnItemChanged(currentAnnItem);
            if (hasContentsOn(pressed_mx,released_my))
            {
                // this depends on specific purpose code
                contentsSelectEvent(pressed_mx,released_my,adding,shifted);
            }
        }
        else
        {
            // this depends on specific purpose code
            contentsSelectEvent(pressed_mx,released_my,adding,shifted);
        }
    }

    // clean annotation flags in any case
    pressAnnItem = NULL;
}

void
AScrollView::mouseWorldReleaseEvent_MWand (QMouseEvent* e, double x, double y)
{
    bool c1 = fabs(pressed_wx-released_wx)<3.0;
    bool c2 = fabs(pressed_wy-released_wy)<3.0;
    if (c1&&c2)
    {
        applyMagicWand(pressed_mx,pressed_my);
    }    
}

void 
AScrollView::mouseWorldReleaseEvent_Panning (QMouseEvent* e, double x, double y)
{
    panning = false; 
}

void 
AScrollView::mouseWorldReleaseEvent_Zooming (QMouseEvent* e, double x, double y)
{
    //cerr << "AScrollView::mouseWorldReleaseEvent_Zooming called, old scf_x " << scf_x << " scf_y " << scf_y << endl;
    // I need to remove the "last" rubber
    drawZoomRubber(e->x(),e->y());
    zooming = false;
    int cx,cy;
    computeZoomRect((int)pressed_wx,(int)pressed_wy,(int)released_wx,(int)released_wy,&scf_x,&scf_y,&cx,&cy);
    //cerr << "\t new scf_x " << scf_x << " scf_y " << scf_y << endl;
    wasScrollByZooming=true;
    resizeContents((int)((double)realContentsWidth*scf_x),(int)((double)realContentsHeight*scf_y));
    emit asvNewScallingFactors(scf_x,scf_y);
    center(cx,cy);
    viewport()->setUpdatesEnabled( TRUE );
    viewport()->update();
    updateZoomStatusBar();

    // Since we have recentered the contents we propagate an event for that
    //double ox = ((double)contentsX()/scf_x/getBaseElementSize());
    //double oy = ((double)contentsY()/scf_y/getBaseElementSize());
    //emit asvCentered(ox,oy); 
}

void 
AScrollView::mouseWorldReleaseEvent_ZoomingNWin (QMouseEvent* e, double x, double y)
{
    //printf("AScrollView::mouseWorldReleaseEvent_ZoomingNWin this=0x%x\n",(void*)this);
    // I need to remove the "last" rubber
    drawZoomRubber(e->x(),e->y());
    zooming = false;
    int cx,cy;
    double nscf_x,nscf_y;
    computeZoomRect((int)pressed_wx,(int)pressed_wy,(int)released_wx,(int)released_wy,&nscf_x,&nscf_y,&cx,&cy);
    emit newZoomWindow((int)pressed_wx,(int)pressed_wy,(int)released_wx,(int)released_wy,nscf_x,nscf_y,cx,cy);
}

void 
AScrollView::mouseWorldReleaseEvent_ZoomingNWinCallback (double fx,double fy,int cx, int cy)
{
    //printf("AScrollView::mouseWorldReleaseEvent_ZoomingNWinCallback this=0x%x\n",(void*)this);
    scf_x=fx;
    scf_y=fy;
    resizeContents((int)((double)realContentsWidth*fx),(int)((double)realContentsHeight*fy));
    center(cx,cy);
    viewport()->setUpdatesEnabled( TRUE );
    viewport()->update();
    updateZoomStatusBar();
}

void 
AScrollView::mouseWorldReleaseEvent_Distance (QMouseEvent* e, double x, double y)
{
    bool shifted = e->state() & ShiftButton;
    
    // I need to remove the "last" rubber line
    drawDstRubber(lastDistance_x,lastDistance_y,lastDistance_mx);
    
    measuring = false;
    showDistance(pressed_mx,released_mx);
    if (shifted)
    {
        createDistanceAnnotation(pressed_mx,pressed_my,released_mx,released_my);
    }
}

void 
AScrollView::mouseWorldReleaseEvent_Shading (QMouseEvent* e, double x, double y)
{
    /*
    printf ("shading detected pressed on (%d,%d)\n",pressed_mx,pressed_my);
    printf ("shading detected released on (%d,%d)\n",released_mx,released_my);
    printf ("lastShaded_mx=%d\n",lastShaded_mx);
    */
    
    shading=false;

    // if shift key  on
    if ( (e->state() & ShiftButton) && (lastShaded_mx>=0) )
    {
        if (lastShaded_mx==released_mx) return;
        
        if (lastShaded_mx < released_mx)
        {
            for (int i=lastShaded_mx+1;i<released_mx;i++)
            {
                switchShadeFlag(i);
            }
        }
        else
        {
            for (int i=lastShaded_mx-1;i>released_mx;i--)
            {
                switchShadeFlag(i);
            }
        }
        lastShaded_mx=released_mx;
        
    }
    else
    {
        lastShaded_mx = lastShading_mx;
    }
}

void 
AScrollView::mouseWorldReleaseEvent_Annotation (QMouseEvent* e, double x, double y)
{
    annCtrl->mouseWorldReleaseEvent(e, currentAnnItem, x, y,scf_x,scf_y);
    currentAnnItem->setSelected(true);

    // repaint the affected area:
    if (pressed_wx > released_wx) 
    {
        double tmp = pressed_wx;
        pressed_wx = released_wx;
        released_wx = tmp;
    }
    
    if (pressed_wy > released_wy) 
    {
        double tmp = pressed_wy;
        pressed_wy = released_wy;
        released_wy = tmp;
    }
    
    pressed_wx *= scf_x;
    pressed_wy *= scf_y;
    released_wx *= scf_x;
    released_wy *= scf_y;
    repaintContents((int)(pressed_wx-(5.0*scf_x)),(int)(pressed_wy-(5.0*scf_y)),
    (int)(released_wx-pressed_wx+(10.0*scf_x)),
    (int)(released_wy-pressed_wy+(10.0*scf_y)),
    false);
}



// -------------------------------------------------------------------
// -------------------------------------------------------------------
// Mouse move related functions
// -------------------------------------------------------------------
// -------------------------------------------------------------------

void
AScrollView::mouseWorldMoveEvent(QMouseEvent* e, double x, double y)
{
    current_mx = (int)(x/QMAX(1.0,getBaseElementSize()));
    current_my = (int)(y/QMAX(1.0,getBaseElementSize()));
    current_wx = x;
    current_wy = y;

    //cerr << "mx " << current_mx << " my " << current_my << endl;

    // update cycle / row stuff
    updateCycleBar(x);
    updateRowBar(x,y);
    //tip->tipNow(QPoint(e->x(),e->y()));
    updateTip(e);
    
    // check pointer types:
    switch (currentPointerType)
    {
        case POINTER_SELECT:
        mouseWorldMoveEvent_Select(e,x,y);
        break;

        case POINTER_MWAND:
        mouseWorldMoveEvent_MWand(e,x,y);
        break;

        case POINTER_PANNING:
        mouseWorldMoveEvent_Panning(e,x,y);
        break;

        case POINTER_ZOOMING:
        case POINTER_ZOOMING_NWIN:
        mouseWorldMoveEvent_Zooming(e,x,y);
        break;

        case POINTER_DISTANCE:
        mouseWorldMoveEvent_Distance(e,x,y);
        break;

        case POINTER_SHADING: 
        mouseWorldMoveEvent_Shading(e,x,y);
        break;

        default:    // annotation tools
        mouseWorldMoveEvent_Annotation(e,x,y);
    }
}


void 
AScrollView::mouseWorldMoveEvent_Select (QMouseEvent* e, double x, double y)
{
    if (pressAnnItem!=NULL)
    {
        //printf ("asv: move with ann item...\n");
        annCtrl->mouseWorldMoveEvent(e, pressAnnItem, x, y,scf_x,scf_y);
    }
}

void 
AScrollView::mouseWorldMoveEvent_MWand (QMouseEvent* e, double x, double y)
{}

void 
AScrollView::mouseWorldMoveEvent_Panning (QMouseEvent* e, double x, double y)
{
    if (panning)
    {
        int ex = e->x();
        int ey = e->y();

        // compute distance width last point
        int dx = ex - lastPan_x;
        int dy = ey - lastPan_y;

        lastPan_x = ex;
        lastPan_y = ey;

        // get current center point
        int centerx = (contentsX()+QMAX(visibleWidth(),1)/2);
        int centery = (contentsY()+QMAX(visibleHeight(),1)/2);

        centerx -= (int)((double)dx);
        centery -= (int)((double)dy);
        center(centerx,centery);
        
        // Since we have recentered the contents we propagate an event for that
        //double ox = ((double)contentsX()/scf_x/getBaseElementSize());
        //double oy = ((double)contentsY()/scf_y/getBaseElementSize());
        //emit asvCentered(ox,oy); 
    }
}

void 
AScrollView::mouseWorldMoveEvent_Zooming (QMouseEvent* e, double x, double y)
{
    if (zooming)
    {
        int ex = e->x();
        int ey = e->y();

        // erase old rubber
        drawZoomRubber(lastZooming_x,lastZooming_y);

        // and draw the new one
        lastZooming_x = ex;
        lastZooming_y = ey;

        drawZoomRubber(lastZooming_x,lastZooming_y);
    }
}

void 
AScrollView::mouseWorldMoveEvent_Distance (QMouseEvent* e, double x, double y)
{
    if (measuring)
    {
        // erase old rubber
        drawDstRubber(lastDistance_x,lastDistance_y,lastDistance_mx);

        // and draw the new one
        
        // snap to column:
        double unsc_x = (double)(current_mx*getBaseElementSize())*scf_x;
        int cy;
        contentsToViewport((int)unsc_x,0,lastDistance_x,cy);
        lastDistance_y = e->y();
        lastDistance_mx = current_mx;

        drawDstRubber(lastDistance_x,lastDistance_y,lastDistance_mx);

        // update bar
        showDistance(pressed_mx,current_mx);
    }
}

void 
AScrollView::mouseWorldMoveEvent_Shading (QMouseEvent* e, double x, double y)
{
    if (shading)
    {
        if (lastShading_mx==current_mx)
        {
            return;
        }
        else
        {
            lastShading_mx = current_mx;
            switchShadeFlag(current_mx);
            lastShaded_mx=current_mx;
        }
    }
}

void 
AScrollView::mouseWorldMoveEvent_Annotation (QMouseEvent* e, double x, double y)
{
    annCtrl->mouseWorldMoveEvent(e, currentAnnItem, x, y,scf_x,scf_y); 
}


void 
AScrollView::switchShadeFlag(int col)
{
    //printf ("switchShade on col %d\n",col);
    //printf ("current state %d\n",getShadeFlag(col));

    shadeColumn(col, !getShadeFlag(col));
    showLastShadded(col);
    
    if (mdiLocked) emit toggledShadowCol(col,getShadeFlag(col));
}


void
AScrollView::showDistance(int pressed_mx, int released_mx)
{
    emit status_msg_changed(QString("From ")+QString::number((int)pressed_mx)+QString(" to ")+
        QString::number((int)released_mx)+QString(" : ")+
        QString::number(abs(released_mx-pressed_mx)) +
        QString(" cycles"));
}

void
AScrollView::showLastShadded(int mx)
{
    emit status_msg_changed(QString("Cycle ")+QString::number(mx));
}


// -------------------------------------------------------------------
// -------------------------------------------------------------------
// Rubber band functions
// -------------------------------------------------------------------
// -------------------------------------------------------------------

void 
AScrollView::drawZoomRubber( int x, int y )
{
    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );

    style().drawPrimitive(QStyle::PE_FocusRect, &p,
              QRect( pressed_x, pressed_y,x-pressed_x+1,y-pressed_y+1),
              colorGroup(), QStyle::Style_Default,
              QStyleOption(colorGroup().base()));
    p.end();
}

void
AScrollView::drawDstRubber(int x, int y, int mx)
{
    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush);
    p.setFont(QFont("Helvetica"));

    //printf ("AScrollView::drawDstRubber called with x=%d,y=%d,mx=%d,pressed_snaped_x=%d,pressed_y=%d\n",x,y,mx,pressed_snaped_x,pressed_y);
    // compute the vector
    double vx = (double)(x-pressed_snaped_x);
    double vy = (double)(y-pressed_y);

    // normalize:
    double mod = sqrt(vx*vx + vy*vy);
    if (mod < 4.0)
    {
        p.end();
        return;
    }

    // central line:
    p.drawLine(pressed_snaped_x,pressed_y,x,y);

    vx /= mod;
    vy /= mod;

    // rotate 90
    double rvy = vx;
    double rvx = -vy;

    // cota marks
    p.drawLine(pressed_snaped_x,pressed_y,pressed_snaped_x+(int)(rvx*10.0),pressed_y+(int)(rvy*10.0));
    p.drawLine(pressed_snaped_x,pressed_y,pressed_snaped_x-(int)(rvx*10.0),pressed_y-(int)(rvy*10.0));

    p.drawLine(x,y,x+(int)(rvx*10.0),y+(int)(rvy*10.0));
    p.drawLine(x,y,x-(int)(rvx*10.0),y-(int)(rvy*10.0));

    if (mod < 16.0)
    {
        p.end();
        return;
    }

    // put the current measure:
    QFontMetrics fm = p.fontMetrics();
    QRect tbr = fm.boundingRect(QString("Distance: ")+QString::number(abs(mx-pressed_mx)));

    double b = (double)(x-pressed_snaped_x);
    double a = (double)(y-pressed_y);

    bool inv = (b<0.0);
    if  (inv)
    {
        b = -b;
        a = -a;
    }

    double px,py;
    px = pressed_snaped_x-(rvx*(double)tbr.height()*2.0);
    py = pressed_y-(rvy*(double)tbr.height()*2.0);
    px += (vx*mod*0.5);
    py += (vy*mod*0.5);
    px -= (vx*(double)tbr.width()*0.5);
    py -= (vy*(double)tbr.width()*0.5);

    double rpx,rpy;
    QWMatrix matx;
    matx.rotate(-(atan2(a,b)*180.0/pi));
    matx.map(px,py,&rpx,&rpy);

    if (inv)
    {
        p.translate(vx*(double)tbr.width(),vy*(double)tbr.width());
    }

    p.rotate((atan2(a,b)*180.0/pi));
    p.drawText ((int)floor(rpx),(int)floor(rpy),
    QString("Distance: ")+QString::number(abs((int)(mx - pressed_mx))));

    p.end();
}


// -------------------------------------------------------------------
// -------------------------------------------------------------------
//
// -------------------------------------------------------------------
// -------------------------------------------------------------------

void 
AScrollView::computeZoomRect(int pmx, int pmy, int rmx, int rmy, double* nscf_x, double* nscf_y, int* ncenterx, int* ncentery)
{
    viewport()->setUpdatesEnabled( FALSE );
    double mscf_x = scf_x;
    double mscf_y = scf_y;
    
    // check order
    if (pmx > rmx)
    {
        int sw = pmx;
        pmx = rmx;
        rmx = sw;
    }
    if (pmy > rmy)
    {
        int sw = pmy;
        pmy = rmy;
        rmy = sw;
    }

    // get current center point
    int centerx = pmx + (rmx-pmx)/2;
    int centery = pmy + (rmy-pmy)/2;

    // compute needed scalling factors:
    int wwidth  = rmx - pmx;
    int wheight = rmy - pmy;

    if (wwidth==0) wwidth=1;
    if (wheight==0) wheight = 1;

    int vwidth  = visibleWidth();
    int vheight = visibleHeight();

    double prop_x = (double)vwidth /(double)wwidth;
    double prop_y = (double)vheight /(double)wheight;

    if (rectZoomKeepARatio)
    {
        double prop = prop_x < prop_y ? prop_x : prop_y;
        mscf_x = prop;
        mscf_y = prop;
    }
    else
    {
        mscf_x = prop_x;
        mscf_y = prop_y;
    }

    if (mscf_x < MIN_SCALE_FACTOR) mscf_x = MIN_SCALE_FACTOR;
    if (mscf_x > MAX_SCALE_FACTOR) mscf_x = MAX_SCALE_FACTOR;
    if (mscf_y < MIN_SCALE_FACTOR) mscf_y = MIN_SCALE_FACTOR;
    if (mscf_y > MAX_SCALE_FACTOR) mscf_y = MAX_SCALE_FACTOR;

    // user defined limits
    if (mscf_x < user_min_scale_factor) mscf_x = user_min_scale_factor;
    if (mscf_x > user_max_scale_factor) mscf_x = user_max_scale_factor;
    if (mscf_y < user_min_scale_factor) mscf_y = user_min_scale_factor;
    if (mscf_y > user_max_scale_factor) mscf_y = user_max_scale_factor;
    
    centerx = (int)((double)centerx*mscf_x);
    centery = (int)((double)centery*mscf_y);

    // put results
    *ncenterx = centerx;
    *ncentery = centery;
    *nscf_x = mscf_x;
    *nscf_y = mscf_y;
    
    viewport()->setUpdatesEnabled( TRUE );    
}


void
AScrollView::updatePointerBitmap()
{
    QPixmap zoomingCursorIcon = QPixmap( zoomingCursor );
    QPixmap panningCursorIcon = QPixmap( panningCursor );
    QPixmap shadingCursorIcon = QPixmap( shadingCursor );
    QPixmap mwandCursorIcon   = QPixmap( mwandCursor );

    switch (currentPointerType)
    {
        case POINTER_SELECT:
        viewport()->setCursor(QCursor(ArrowCursor));
        break;

        case POINTER_MWAND:
        viewport()->setCursor(QCursor(mwandCursorIcon,8,9));
        break;

        case POINTER_PANNING:
        viewport()->setCursor(QCursor(panningCursorIcon));
        break;

        case POINTER_ZOOMING:
        case POINTER_ZOOMING_NWIN:
        viewport()->setCursor(QCursor(zoomingCursorIcon,6,5));
        break;

        case POINTER_DISTANCE:
        viewport()->setCursor(QCursor(CrossCursor));
        break;

        case POINTER_SHADING:
        viewport()->setCursor(QCursor(shadingCursorIcon,4,19));
        break;

        // all annotation tools:
        default:
        viewport()->setCursor(QCursor(CrossCursor));
    }
}


AnnotationItem* 
AScrollView::newAnnItem()
{

    // translate pointer type into ann tool class...
    AnnToolType annType = ANN_LINE; // just to avoid compiler warning

    switch (currentPointerType)
    {
        case POINTER_DRAWLINE:
        annType = ANN_LINE;
        break;

        case POINTER_DRAWRECTANGLE:
        annType = ANN_RECTANGLE;
        break;

        case POINTER_DRAWCIRCLE:
        annType = ANN_CIRCLE;
        break;

        case POINTER_INSERTTEXT:
        annType = ANN_TEXT;
        break;

        default:
        Q_ASSERT(false);
    }

    AnnotationItem* result = AnnotationFactory::createAnnItem (annType);
    return (result);
}

void
AScrollView::ZoomIn(double factor)
{
    if (scf_x > MAX_SCALE_FACTOR) { return; }
    if (scf_y > MAX_SCALE_FACTOR) { return; }
    if (scf_x > user_max_scale_factor) { return; }
    if (scf_y > user_max_scale_factor) { return; }

    if (mdiLocked) emit asvZoomed(factor,factor);
    updateScallingFactor(scf_x*factor,scf_y*factor);
}

void
AScrollView::ZoomInX(double factor)
{
    if (scf_x > MAX_SCALE_FACTOR) { return; }
    if (scf_x > user_max_scale_factor) { return; }

    if (mdiLocked) emit asvZoomed(factor,1.0);
    updateScallingFactor(scf_x*factor,scf_y);
}

void
AScrollView::ZoomInY(double factor)
{
    if (scf_y > MAX_SCALE_FACTOR) { return; }    
    if (scf_y > user_max_scale_factor) { return; }    
    
    if (mdiLocked) emit asvZoomed(1.0,factor);
    updateScallingFactor(scf_x,scf_y*factor);
}

void
AScrollView::ZoomOut(double factor)
{
    if (scf_x < MIN_SCALE_FACTOR) { return; }
    if (scf_y < MIN_SCALE_FACTOR) { return; }
    if (scf_x < user_min_scale_factor) { return; }
    if (scf_y < user_min_scale_factor) { return; }
    
    if (mdiLocked) emit asvZoomed(1.0/factor,1.0/factor);
    updateScallingFactor(scf_x/factor,scf_y/factor);
}

void
AScrollView::ZoomOutX(double factor)
{
    if (scf_x > MAX_SCALE_FACTOR) { return; }    
    if (scf_x > user_max_scale_factor) { return; }

    if (mdiLocked) emit asvZoomed(1.0/factor,1.0);
    updateScallingFactor(scf_x/factor,scf_y);
}

void 
AScrollView::ZoomOutY(double factor)
{
    if (scf_y > MAX_SCALE_FACTOR) { return; }    
    if (scf_y > user_max_scale_factor) { return; }

    if (mdiLocked) emit asvZoomed(1.0,1.0/factor);
    updateScallingFactor(scf_x,scf_y/factor);
}

void 
AScrollView::setScallingFactor (double factor)
{
    if (factor < MIN_SCALE_FACTOR) { factor = MIN_SCALE_FACTOR; }
    if (factor > MAX_SCALE_FACTOR) { factor = MAX_SCALE_FACTOR; }    
    if (factor < user_min_scale_factor) { factor = user_min_scale_factor; }
    if (factor > user_max_scale_factor) { factor = user_max_scale_factor; }    

    updateScallingFactor(factor,factor);
}


void 
AScrollView::setScallingFactorX(double factor)
{
    if (factor < MIN_SCALE_FACTOR) { factor = MIN_SCALE_FACTOR; }    
    if (factor > MAX_SCALE_FACTOR) { factor = MAX_SCALE_FACTOR; }    
    if (factor < user_min_scale_factor) { factor = user_min_scale_factor; }    
    if (factor > user_max_scale_factor) { factor = user_max_scale_factor; }    

    updateScallingFactor(factor,scf_y);
}

void 
AScrollView::setScallingFactorY(double factor)
{
    if (factor < MIN_SCALE_FACTOR) { factor = MIN_SCALE_FACTOR; }    
    if (factor > MAX_SCALE_FACTOR) { factor = MAX_SCALE_FACTOR; }    
    if (factor < user_min_scale_factor) { factor = user_min_scale_factor; }    
    if (factor > user_max_scale_factor) { factor = user_max_scale_factor; }    
    
    updateScallingFactor(scf_x,factor);
}

void 
AScrollView::updateCycleBar(double x)
{
    if (x<0)
    {
        x=0;
    }

    if (x>=realContentsWidth)
    {
        x = realContentsWidth-1;
    }

    int cycle = (int)(x/QMAX(1.0,getBaseElementSize()));
    emit cycle_msg_changed(QString(" Cycle : ")+QString::number((int)cycle)+QString("    "));
}

void
AScrollView::updateRowBar(double x,double y)
{
    if (y<0)
    {
        y=0;
    }

    if (y>=realContentsHeight)
    {
        y = realContentsHeight-1;
    }

    int row = (int)(y/QMAX(1.0,getBaseElementSize()));
    emit row_msg_changed(QString(" Row : ")+QString::number((int)row)+QString(" "));
}

void
AScrollView::ensureOffScrSize( int osw, int osh )
{
    if ( osw > offscr.width() || osh > offscr.height() )
    {
        offscr.resize(QMAX(osw,offscr.width()),QMAX(osh,offscr.height()));
    }
    else if ( offscr.width() == 0 || offscr.height() == 0 )
    {
        offscr.resize( QMAX( offscr.width(), 1),QMAX( offscr.height(), 1 ) );
    }    
}

void
AScrollView::viewportMousePressEvent(QMouseEvent* e)
{
    int cx,cy;

    if (e->button()!=LeftButton)
    {
        // possible context menu
        rightClickPressEvent(e);
        return;
    }

    pressed_x = e->x();
    pressed_y = e->y();

    // move offset if any
    pressed_x -= myHorizontalOffset;
    pressed_y -= myVerticalOffset;
    
    // apply contents shifting...
    viewportToContents (pressed_x,pressed_y,cx,cy);
    mouseWorldPressEvent(e,((double)cx/scf_x),((double)cy/scf_y));
}

void 
AScrollView::viewportMouseReleaseEvent(QMouseEvent* e)
{
    int cx,cy;

    if (e->button()!=LeftButton)
    {
        // possible context menu
        rightClickReleaseEvent(e);
        return;
    }

    released_x = e->x();
    released_y = e->y();

    // move offset if any
    released_x -= myHorizontalOffset;
    released_y -= myVerticalOffset;
    
    // apply contents shifting...
    viewportToContents (released_x,released_y,cx,cy);
    mouseWorldReleaseEvent(e,((double)cx/scf_x),((double)cy/scf_y));
}

void 
AScrollView::viewportMouseMoveEvent(QMouseEvent* e)
{
    int cx,cy;

    // apply contents shifting...
    viewportToContents (e->x()-myHorizontalOffset,e->y()-myVerticalOffset,cx,cy);
    mouseWorldMoveEvent(e,((double)cx/scf_x),((double)cy/scf_y));
}

void 
AScrollView::contentsMouseDoubleClickEvent (QMouseEvent* e)
{
    mouseWorldDoubleClickEvent(e,((double)e->x()/scf_x),((double)e->y()/scf_y));
}

void
AScrollView::putView(int x,int y, double sx, double sy)
{
    updateScallingFactor(sx,sy);
    setContentsPos(x,y);
}

void
AScrollView::scrollByMatrix(double x,double y)
{
    /*
    static double lastXRem=0.0;
    static double lastYRem=0.0;
    double lostRemX;
    double lostRemY;
    x = x*scf_x;
    y = y*scf_y;
    wasScrollByMatrix=true;
    int dx = (int)floor(floor(x)+lastXRem);    
    int dy = (int)floor(floor(y)+lastYRem);
    lostRemX = (floor(x)+lastXRem)-(double)dx;    
    lostRemY = (floor(y)+lastYRem)-(double)dy;    
    scrollBy(dx,dy);
    lastXRem=x-floor(x);
    lastYRem=y-floor(y);
    lastXRem+=lostRemX; 
    lastYRem+=lostRemY;
    */
    x = x*scf_x;
    y = y*scf_y;
    wasScrollByMatrix=true;
    scrollBy((int)rint(x),(int)rint(y));
}

void
AScrollView::goToColumn(INT32 column)
{
    INT32 cw   = (INT32)((double)visibleWidth()/getBaseElementSize()/scf_x);
    INT32 ch   = (INT32)((double)visibleHeight()/getBaseElementSize()/scf_y);
    INT32 cx   = (INT32)((double)contentsX()/getBaseElementSize()/scf_x);
    // if is already visible dont move!
    if ((column>=cx) && (column<=(cx+cw)) ) return;

    int x = (int)floor((double)column*QMAX(1.0,getBaseElementSize())*scf_x);
    setContentsPos(x-visibleWidth()/2,contentsY());
}

void
AScrollView::goToRow(INT32 row)
{
    INT32 cw   = (INT32)((double)visibleWidth()/getBaseElementSize()/scf_x);
    INT32 ch   = (INT32)((double)visibleHeight()/getBaseElementSize()/scf_y);
    INT32 cy   = (INT32)((double)contentsY()/getBaseElementSize()/scf_y);
    // if is already visible dont move!
    if ((row>=cy) && (row<=(cy+ch)) ) return;

    int y = (int)floor((double)row*QMAX(1.0,getBaseElementSize())*scf_y);
    setContentsPos(contentsX(),y-visibleHeight()/2);
}

void
AScrollView::setSTFitKeepARatio(bool v)
{
    sTFitKeepARatio = v;
}

void
AScrollView::setRectZoomKeepARatio(bool v)
{
    rectZoomKeepARatio = v;
}

void 
AScrollView::shadeColumn(INT32 col,bool shade)
{
    cshadows->shadeColumn(col,shade);
    repaintContents((int)((double)col*scf_x*(double)QMAX(1.0,getBaseElementSize())-(2*scf_x)),
    0, (int)(scf_x*(double)QMAX(1.0,getBaseElementSize())+(4*scf_x)),
    (int)(scf_y*(double)realContentsHeight),false);
}

void
AScrollView::setPenColorChgId(int chId)
{
    penColorChId = chId;
}

void
AScrollView::setBrushColorChgId(int chId)
{
    brushColorChId = chId;
}

void
AScrollView::setTextColorChgId(int chId)
{
    textColorChId = chId;
}

void
AScrollView::setShowGridLines(bool v)
{
    showVGridLines = v;
    showHGridLines = v;
    viewport()->repaint(true);
}

void
AScrollView::setShowVGridLines(bool v)
{
    showVGridLines = v;
    viewport()->repaint(true);
}

void
AScrollView::setShowHGridLines(bool v)
{
    showHGridLines = v;
    viewport()->repaint(true);
}

void
AScrollView::setGridLinesSize(int v)
{
    gridSize = v;
    annCtrl->setGridSize(v);
    viewport()->repaint(false);
}

bool
AScrollView::getShowVGridLines()
{
    return showVGridLines;
}

bool
AScrollView::getShowHGridLines()
{
    return showHGridLines;
}

int
AScrollView::getGridLinesSize()
{
    return gridSize;
}

void
AScrollView::setSnapToGrid(bool v)
{
    annCtrl->setSnapToGrid(v);
}

bool
AScrollView::getSnapToGrid()
{
    return annCtrl->getSnapToGrid();
}

bool
AScrollView::getShadeFlag(int pos)
{ return cshadows->getShadeFlag(pos); }

double
AScrollView::getScallingFactorX()
{
    return scf_x;
}

double
AScrollView::getScallingFactorY()
{
    return scf_y;
}

void
AScrollView::setRealContentsSize(int w, int h)
{
    realContentsWidth  = w;
    realContentsHeight = h;
    resizeContents((int)((double)w*scf_x),(int)((double)h*scf_y));
}

void
AScrollView::setPointerType(PointerType type)
{
    currentPointerType = type;
    updatePointerBitmap();
}


void
AScrollView::setCurrentBrush(QBrush b)
{
    this->currentBrush = b;
}

void
AScrollView::setCurrentPen(QPen p)
{
    this->currentPen = p;
}

void
AScrollView::setCurrentTextColor (QColor c)
{
    this->currentTextColor = c;
}

void
AScrollView::colorChanged (QColor color, bool nocolor, int chId)
{
    if (chId==penColorChId)
    {
        if (nocolor)
        {
            currentPen.setStyle(NoPen);
        }
        else
        {
            if (currentPen.style()==NoPen) 
            {
                currentPen.setStyle(SolidLine);
            }    
            currentPen.setColor(color);
        }
    }
    else if (chId==brushColorChId)
    {
        if (nocolor)
        {
            currentBrush.setStyle(NoBrush);
        }
        else
        {
            if (currentBrush.style()==NoBrush) 
            {
                currentBrush.setStyle(SolidPattern);
            }
            currentBrush.setColor(color);
        }
    }
    else if (chId==textColorChId)
    {
        currentTextColor = color;
    }
}

void
AScrollView::colorApply (QColor color, bool nocolor, int chId)
{
    if (chId==penColorChId)
    {
        if (currentAnnItem!=NULL)
        {
            currentAnnItem->setPen(currentPen);
            annCtrl->repaintAnnItem(currentAnnItem,scf_x,scf_y);
        }
    }
    else if (chId==brushColorChId)
    {
        if (currentAnnItem!=NULL)
        {
            currentAnnItem->setBrush(currentBrush);
            annCtrl->repaintAnnItem(currentAnnItem,scf_x,scf_y);
        }
    }
    else if (chId==textColorChId)
    {
        //
    }
}

void
AScrollView::setBgColor(QColor c)
{
    bgColor=c;
    viewport()->setBackgroundColor(bgColor);
}

void
AScrollView::setShadingColor(QColor c)
{
    shaderBrush = QBrush(c);
}

void
AScrollView::applyMagicWand (int mx,int my)
{
}

ColumnShadows*
AScrollView::getColumnShadows()
{ return cshadows; }

void
AScrollView::canceledDrawRefresh()
{
    currentPainter->fillRect(currentPainter->viewport(),QBrush(QColor("#202020"),Qt::DiagCrossPattern));
}

QString
AScrollView::getTipString(bool eventOnly)
{
    QString s = "Cycle ";
    s = s + QString::number(current_mx) + " Row " + QString::number(current_my);
    return s;
}

void 
AScrollView::updateTip( QMouseEvent* e )
{
    static int label_mx=-1;
    static int label_my=-1;
    bool shifted = e->state() & ShiftButton;
    
    //printf ("updateTip called with measuring=%d,shading=%d,zooming=%d,shifted=%d\n",(int)measuring,(int)shading,(int)zooming,(int)shifted);    
    if(shading || zooming || shifted)
    {
        if ( (current_mx!=label_mx)||(current_my!=label_my) )
        {
            cleanTip();

            QString  s = getTipString();
            tipLabel = new AGTTipLabel(NULL,s);
        
            //printf ("updateTip called on pos %d %d with msg=%s\n",current_mx,current_my,s.latin1());

            QPoint global = mapToGlobal(QPoint(e->x()+22,e->y()+22));
            QRect screen = QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen());
            int x = global.x();
            int y = global.y();
            int lwidth = tipLabel->width();
            int lheight = tipLabel->height();
            int width = screen.width();
            int height = screen.height();
            if((x + lwidth) > width)
            {
                global.setX(width - lwidth);
            }
            if((y + lheight) > height)
            {
                global.setY(height - lheight);
            }
            tipLabel->move(global);
            tipLabel->show();
            tipLabel->raise();
            
            label_mx=current_mx;
            label_my=current_my;
        }
    }
    else if(measuring)
    {
        cleanTip();

        QString s = getTipString();
        tipLabel = new AGTTipLabel(NULL,s);

        if(pressed_mx < current_mx)
        {
            tipLabel->move(mapToGlobal(QPoint(pressed_x - tipLabel->width() - 22, e->y() - (tipLabel->height() >> 1))));
        }
        else
        {
            tipLabel->move(mapToGlobal(QPoint(pressed_x + 22, e->y() - (tipLabel->height() >> 1))));
        }
        tipLabel->show();
        tipLabel->raise();
    }
    else
    {
        // clean up any spureous label
        cleanTip();
    }
}

void
AScrollView::cleanTip()
{
    if (tipLabel!=NULL)
    {
        tipLabel->hide();
        delete tipLabel;
        tipLabel=NULL;
    }
}

void 
AScrollView::setLockPan(bool locked)
{
    panLocked = locked;
}

void 
AScrollView::setLockZoom(bool locked)
{
    zoomLocked=locked;
}

void 
AScrollView::setLockShading(bool locked)
{
    shadingLocked=locked;
}

void 
AScrollView::setLockAnnotation(bool locked)
{
    annotationLocked=locked;
}

void 
AScrollView::contentsMoving ( int x, int y )
{
    // use kind of semaphor to avoid infinite recursion 
    if ( (!wasScrollByMatrix) && (!wasScrollByZooming) )
    {
        double dx = (double)(x-contentsX());
        double dy = (double)(y-contentsY());
        // transform this into matrix coordinates
        dx = dx/scf_x;
        dy = dy/scf_y;
        if (mdiLocked) emit asvMoved(dx,dy);                                 // delta movement
        //cerr << "AScrollView::contentsMoving x " << x << " y " << y << " dx " << dx << " dy " << dy << endl;
    }
    //FIXME some coherent policy to which/when evetns are emitted should be set
    //cerr << "AScrollView::contentsMoving x " << x << " y " << y << " scfx " << scf_x << " scfy " << scf_y << endl;
    emit asvNewTLCorner((double)x/scf_x,(double)y/scf_y); // absolute position
    wasScrollByMatrix=false;
    wasScrollByZooming=false;
}

PointerType 
AScrollView::getPointerType()
{ return currentPointerType; }

void 
AScrollView::copyColumnShadows(AScrollView* parent)
{
    ColumnShadows* pshadows = parent->cshadows;
    int wwidth = pshadows->getNumCols();
    for (int col=0;col<wwidth;col++)
    {
        cshadows->shadeColumn(col,pshadows->getShadeFlag(col));
        repaintContents((int)((double)col*scf_x*(double)QMAX(1.0,getBaseElementSize())-(2*scf_x)),
        0, (int)(scf_x*(double)QMAX(1.0,getBaseElementSize())+(4*scf_x)),
        (int)(scf_y*(double)realContentsHeight),false);
    }
}

void
AScrollView::createDistanceAnnotation(int px, int py, int rx, int ry)
{
    int sd= (int)getBaseElementSize();
 
    AnnotationContainer* anngroup = new AnnotationContainer();   
    AnnotationLine *vl1,*vl2,*hl;
    vl1 = new AnnotationLine((double)(px*sd),(double)(QMAX((py-1)*sd,0)),(double)(px*sd),(double)((ry+1)*sd),anngroup);
    vl2 = new AnnotationLine((double)(rx*sd),(double)(QMAX((py-1)*sd,0)),(double)(rx*sd),(double)((ry+1)*sd),anngroup);
    hl  = new AnnotationLine((double)(px*sd),(double)(ry*sd),(double)(rx*sd),(double)(ry*sd),anngroup);
    int wd = (rx-px)*sd - (sd/2);
    int hg = (ry-py)*sd - (sd/2);
    if (wd<sd) wd = sd;
    if (hg<sd) hg = sd;
    AnnotationText *atxt = new AnnotationText(QRect(px*sd+2,QMAX(ry*sd-hg-1,0),wd,hg),QString::number(abs(rx-px)),anngroup);
    anngroup->setBuilt();
    annCtrl->addItem(anngroup);
    repaintContents(false);
}

QSize
AGTTipLabel::sizeForWidth( int w ) const
{
    QRect br;

    int hextra = 2 * frameWidth();
    int vextra = hextra;
    QFontMetrics fm( fontMetrics() );

	if ( w < 0 ) w = 2000;
    
	br = fm.boundingRect( 0, 0, w ,2000, alignment(), text() );
    return QSize( br.width() + hextra, br.height() + vextra );
}

void 
AScrollView::addHorizontalHeaderOffset(int horizontalOffset)
{
    myHorizontalOffset+=horizontalOffset;
}

void 
AScrollView::addVerticalHeaderOffset(int verticalOffset)
{
    //cerr << "AScrollView::addVerticalHeaderOffset w/ " << verticalOffset << endl; 
    myVerticalOffset+=verticalOffset;
}


void
AScrollView::setMinScallingFactor ( double factor )
{
    user_min_scale_factor = factor;    
}

void
AScrollView::setMaxScallingFactor ( double factor )
{
    user_max_scale_factor = factor;    
}

