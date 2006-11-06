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
  * @file  AScrollView.h
  * @brief Defines the class AScrollView and the enumeration PointerType.
  */

#ifndef _ASCROLLVIEW_H
#define _ASCROLLVIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qscrollview.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qbrush.h>
#include <qnamespace.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qstyle.h>
#include <qfont.h>
#include <qcursor.h>
#include <qprogressbar.h>
#include <qtoolbar.h>
#include <qtooltip.h>

#include "agt_syntax.h"
#include "UpdColorInterface.h"
#include "AnnotationCtrl.h"
#include "AnnotationFactory.h"
#include "ColumnShadows.h"

#define MIN_SCALE_FACTOR 0.001
#define MAX_SCALE_FACTOR 100.0
#define MIN_PROGRESS_SCALE_FACTOR 0.50

/**
  * @typedef PointerType
  * @brief Enumeration to reference all sorts of pointers supported by base AScrollView.
  *
  * @warning Leave annotation tools always at the end of the list!
  */
typedef enum
{
    POINTER_SELECT,           ///< Select Pointer
    POINTER_MWAND,            ///< Magic Wand Pointer
    POINTER_PANNING,          ///< Panning Hand Pointer (Acrobat Reader like)
    POINTER_ZOOMING,          ///< Rectangle Zoom Pointer
    POINTER_DISTANCE,         ///< Generic Distance Pointer
    POINTER_SHADING,          ///< Generic Shading Pointer
    POINTER_ZOOMING_NWIN,     ///< Rectangle Zoom Pointer on a new window

    // annotation tools:
    POINTER_DRAWLINE,         ///< Simple Line Annotation tool
    POINTER_DRAWRECTANGLE,    ///< Simple Rectangle Annotation tool
    POINTER_DRAWCIRCLE,       ///< Simple Circle/Ellipse Annotation tool
    POINTER_INSERTTEXT        ///< Simple Text Box Annotation tool
} PointerType;


/**
  *  This is a helper class for dynamic tips used on some tools like shading and distance
  */
class AGTTipLabel : public QLabel
{
    Q_OBJECT
public:
    AGTTipLabel( QWidget* parent, const QString& text) : QLabel( parent, "AGTtoolTipTip",
	     WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM )
    {
	setMargin(1);
	setAutoMask( FALSE );
	setFrameStyle( QFrame::Plain | QFrame::Box );
	setLineWidth( 1 );
	setAlignment( AlignAuto | AlignTop );
	setIndent(0);
	polish();
	setText(text);
	adjustSize();
    }
    void setWidth( int w ) { resize( sizeForWidth( w ) ); }
    
protected:
    QSize sizeForWidth( int w ) const;
};


/**
  * This is a \em generic base class inteded to ease graphic tools development.
  *
  * This is the cornerstone class of AGT library.
  * It provides a bunch of functionality so that build a drawing-like application
  * on top of QT is easy, fast and lightweight (memory footprint).
  *
  * This is a kind of replacement for QCanvas and QCanvasItem object from
  * standard QT library. @see http://doc.trolltech.com/3.0/qcanvas.html
  *
  * Provided functions are:
  * - Automatic scrolling bars
  * - Scalling factor management (independent horizontal and vertical)
  * - Panning hand tool (Acrobat Reader like)
  * - Distance tool (in user specified units)
  * - Shading tool  (to mark interesting columns)
  * - Zooming by rectangle
  * - Zoom in/out by a given factor
  * - Generic Scale to fit
  * - Generic scalling and "reset aspect ratio" operations
  * - Basic "Status Bar" management
  * - Basic grid lines and snap to grip options (see AnnotationCtrl)
  * - Low level QT event handling
  * - Automatic system coordinates transformations
  *
  * This class directly interacts with most important AGT classes and structures.
  * @see Main
  * @see AnnotationCtrl
  * @see AnnotationItem
  * @see AnnotationFactory
  * @see UpdColorInterface
  * @see PointerType
  *
  * @version 0.2
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class AScrollView :  public QScrollView, public UpdColorIterface
{
    Q_OBJECT
public:

    friend class AGTTip;
    
    /**
      * Constructor
      */
    AScrollView (
                QWidget* parent,         ///< Parent widget
                int width,               ///< Initial real contents width
                int height               ///< Initial real contents height
                );

    /**
      * Destructor
      */
    virtual
    ~AScrollView();


    virtual void setLockPan(bool locked); 
    virtual void setLockZoom(bool locked);
    virtual void setLockShading(bool locked);
    virtual void setLockAnnotation(bool locked);

    virtual int getZX() { return myZX; }
    virtual int getZY() { return myZY; }
    virtual int getMDINumber() { return mdiNumber; }    
    virtual void setMDINumber(int v) { mdiNumber=v; }    

    virtual bool getMDILocked()       { return mdiLocked; }    
    virtual void setMDILocked(bool v) { mdiLocked = v; }    

    virtual void addHorizontalHeaderOffset(int);
    virtual void addVerticalHeaderOffset(int);

    // --------------------------------------------------------------
    // -- UpdColorInterface methods
    // --------------------------------------------------------------

    /**
      * Triggered method when default color changes.
      */
    virtual void
    colorChanged (
                 QColor color,    ///< the new default color (or undefined if transparent)
                 bool nocolor,    ///< when transparent color is selected this parameter is true
                 int id           ///< Widget ID (pen, brush, text ? ...)
                 );

    /**
      * Triggered method when apply button is pressed.
      */
    virtual void
    colorApply (
               QColor color,           ///< The color to be applied (or undefined if transparent)
               bool nocolor,           ///< when transparent color is selected this parameter is true
               int id                  ///< Widget ID (pen, brush, text ? ...)
               );
    // --------------------------------------------------------------
    // --------------------------------------------------------------


    /**
      * We define the concept of "base elemnt size" so that some
      * auxiliar tools (as shading columns) can be implemented in
      * a generic way. Any AScrollView object \em MUST implement
      * this method. If you don't care about it just return 1.
      */
    virtual double
    getBaseElementSize()=0;

    /**
      * Low level QT event triggered when any mouse button is pressed.
      */
    virtual void
    viewportMousePressEvent (
                            QMouseEvent* e
                            );

    /**
      * Low level QT event triggered when any mouse button is released.
      */
    virtual void
    viewportMouseReleaseEvent (
                              QMouseEvent* e
                              );

    /**
      * Low level QT event triggered when mouse moved.
      */
    virtual void
    viewportMouseMoveEvent (
                           QMouseEvent* e
                           );

    /**
      * Low level QT event triggered when a double-click is detected.
      */
    virtual void
    contentsMouseDoubleClickEvent (
                                  QMouseEvent * e
                                  );

    /**
      * Zoom in by a given factor
      */
    virtual void
    ZoomIn (
           double factor=1.5      ///< zoom in scalling factor. 1.0 does nothing
           );

    /**
      * Zoom in X axis by a given factor
      */
    virtual void
    ZoomInX (
            double factor=1.5     ///< zoom in scalling factor. 1.0 does nothing
            );

    /**
      * Zoom in Y axis by a given factor
      */
    virtual void
    ZoomInY (
            double factor=1.5     ///< zoom in scalling factor. 1.0 does nothing
            );

    /**
      * Zoom Out by a given factor
      */
    virtual void
    ZoomOut (
            double factor=1.5     ///< zoom in scalling factor. 1.0 does nothing
            );

    /**
      * Zoom Out X axis by a given factor
      */
    virtual void
    ZoomOutX (
             double factor=1.5     ///< zoom in scalling factor. 1.0 does nothing
             );

    /**
      * Zoom Out Y axis by a given factor
      */
    virtual void
    ZoomOutY (
             double factor=1.5     ///< zoom in scalling factor. 1.0 does nothing
             );

    /**
      * Set scalling factor (both horizontal and vertical) to a given value.
      */
    virtual void
    setScallingFactor (
                      double factor          ///< new (x and y) scalling factors.
                      );

    /**
      * Set scalling factor (horizontal) to a given value.
      */
    virtual void
    setScallingFactorX (
                       double factor     ///< new (horizontal) scalling factor.
                       );

    /**
      * Set scalling factor (vertical) to a given value.
      */
    virtual void
    setScallingFactorY (
                       double factor   ///< new (vertical) scalling factor.
                       );

    /**
      * Set user defined limit for scalling factors 
      */
    virtual void
    setMinScallingFactor (
                       double factor   ///< new (vertical) scalling factor.
                       );
    /**
      * Set user defined limit for scalling factors 
      */
    virtual void
    setMaxScallingFactor (
                       double factor   ///< new (vertical) scalling factor.
                       );
    /**
      * Scale to fit method (based on real contents size)
      */
    virtual void
    scaleToFit();

    /**
      * Scale to fit method (based on real contents size)
      */
    virtual void
    scaleToFitVertical();

    /**
      * Scale to fit method (based on real contents size)
      */
    virtual void
    scaleToFitHorizontal();

    /**
      * Put view is used to recover bookmarked positions and scalling factors.
      */
    virtual void
    putView (
            int x,       ///< x position
            int y,       ///< y position
            double sx,   ///< horizontal scalling factor
            double sy    ///< vertical scalling factor
            );

    virtual void
    scrollByMatrix(double x,double y);
            
    /**
      * scroll the contents to a given column matrix
      */
    virtual void goToColumn (
                            INT32 column
                            );

    /**
      * scroll the contents to a given row matrix
      */
    virtual void goToRow (
                            INT32 row
                            );

    /**
      * Change real contents size (in pixels) specifications.
      */
    virtual void
    setRealContentsSize (
                        int w,    ///< new real contents width
                        int h     ///< new real contents height
                        );

    /**
      * Change current pointer type
      * @see PointerType
      */
    virtual void
    setPointerType (
                   PointerType type    ///< New pointer type
                   );

    /**
      * Change current pointer type
      * @see PointerType
      */
    virtual PointerType 
    getPointerType ();

                   /**
      * Method to set the reference to application status bar.
    virtual void
    setStatusBar (
                 QStatusBar* value  ///< reference to application status bar object
                 );
      */

    /**
      * Gets back current horizontal scalling factor.
      * @return current horizontal scalling factor
      */
    virtual  double       
    getScallingFactorX();

    /**
      * Gets back current vertical scalling factor.
      * @return current vertical scalling factor
      */
    virtual  double
    getScallingFactorY();

    /**
      * Set current brush (color, pattern, etc)
      */
    virtual  void
    setCurrentBrush (
                    QBrush        ///< new default brush
                    );

    /**
      * Set current pen (color, width, etc)
      */
    virtual  void
    setCurrentPen (
                  QPen          ///< new default pen
                  );

    /**
      * Set current text color (for annotation tools)
      */
    virtual  void
    setCurrentTextColor (
                        QColor      ///< new text color
                        );

    /**
      * Registers Widget ID for Pen Color Change events
      * @see AColorDialog
      */
    virtual  void
    setPenColorChgId (
                     int chId
                     );

    /**
      * Registers Widget ID for Brush Color Change events
      * @see AColorDialog
      */
    virtual  void
    setBrushColorChgId (
                       int chId
                       );

    /**
      * Registers Widget ID for text Color Change events
      * @see AColorDialog
      */
    virtual  void
    setTextColorChgId (
                      int chId
                      );

    /**
      * Switchs on and off grid lines visualization.
      */
    virtual  void
    setShowGridLines (
                     bool  ///< TRUE means show grid lines
                     );

    virtual  void
    setShowVGridLines (
                     bool  ///< TRUE means show grid lines
                     );

    virtual  void
    setShowHGridLines (
                     bool  ///< TRUE means show grid lines
                     );

    /**
      * Sets "gap" between grid lines
      * @see snapDialogImpl
      */
    virtual void
    setGridLinesSize (
                     int
                     );

    /**
      * Switches on and off snap to grid behavior of annotation tools
      * @see snapDialogImpl
      */
    virtual void
    setSnapToGrid (
                  bool
                  );

    /**
      * Gets back show grid lines flag.
      * @return show grid lines flag
      */
    virtual bool
    getShowVGridLines();

    /**
      * Gets back show grid lines flag.
      * @return show grid lines flag
      */
    virtual bool
    getShowHGridLines();

    /**
      * Gets back "gap" between grid lines
      * @return "gap" between grid lines
      */
    virtual int         
    getGridLinesSize();

    /**
      * Gets back snap to grid flag.
      * @return snap to grid flag
      */
    virtual bool       
    getSnapToGrid();

    /**
      * Switches on and off the "Locked" aspect ratio on scale to fit operations.
      */
    virtual void
    setSTFitKeepARatio (
                       bool    ///< TRUE means keep aspect ratio on scale to fit
                       );

    /**
      * Switches on and off the "Locked" aspect ratio on rectangle-zoom operations.
      */
    virtual void
    setRectZoomKeepARatio (
                          bool  ///< TRUE means keep aspect ratio on rectangle-zoom operations
                          );

    //virtual void addZoomDialog(QToolBar* parent);
    virtual void setBgColor(QColor);

    virtual void shadeColumn(INT32 col,bool shade);
    virtual void setShadingColor(QColor c);
    virtual ColumnShadows* getColumnShadows();
    virtual void canceledDrawRefresh();
    virtual void mouseWorldReleaseEvent_ZoomingNWinCallback (double fx,double fy,int cx, int cy);
                                   
    virtual void copyColumnShadows(AScrollView* parent);
    inline virtual AnnotationCtrl* getAnnotationCtrl() { return annCtrl; }
    
    /**
      * Method to draw text with minimized pixelation effect
      */
    void drawPseudoScalledText (
                        QPainter *p, 
                        QString str,
                        int flags,
                        int x, 
                        int y, 
                        int w,
                        int h
                        );

public slots:    
    virtual void contentsMoving ( int x, int y );
    
signals:
    void status_msg_changed(QString);
    void cycle_msg_changed(QString);
    void row_msg_changed(QString);
    void progress_cnt_changed(int);
    void progress_reinit(int);
    void progress_show();
    void progress_hide();
    void zoomStatusChanged(int,int);
    void asvMoved(double,double); 
    void asvNewTLCorner(double,double); 
    void asvZoomed(double,double); 
    void asvNewScallingFactors(double,double); 
    void newZoomWindow(int,int,int,int,double,double,int,int);
    void toggledShadowCol(int,bool);
    void currentAnnItemChanged(AnnotationItem*);
    void contentSelected(int,int,bool,bool);

protected:
    // -------------------------------------------------------------------------
    // 2D world coordinates mouse events
    // -------------------------------------------------------------------------

    /**
      * This method is triggered when any mouse button is pressed.
      */
    virtual void
    mouseWorldPressEvent (
                         QMouseEvent* e,        ///< Original QT Event
                         double x,              ///< World Coord. X
                         double y               ///< World Coord. Y
                         );

    /**
      * This method is triggered left button is pressed and "Select" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_Select (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                );

    /**
      * This method is triggered left button is pressed and "Select" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_MWand (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                               );

    /**
      * This method is triggered left button is pressed and "Panning" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_Panning (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );

    /**
      * This method is triggered left button is pressed and "Rect-Zooming" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_Zooming (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );

    /**
      * This method is triggered left button is pressed and "Distance" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_Distance (
                                  QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );

    /**
      * This method is triggered left button is pressed and "Shading" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_Shading (
                                  QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );


                                 /**
      * This method is triggered left button is pressed and \em any "Annotation Tools" pointer is active.
      */
    virtual void
    mouseWorldPressEvent_Annotation (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                    );

    /**
      * This method is triggered when any mouse button is released.
      */
    virtual void
    mouseWorldReleaseEvent (
                             QMouseEvent* e,        ///< Original QT Event
                             double x,              ///< World Coord. X
                             double y               ///< World Coord. Y
                           );

    /**
      * This method is triggered left button is released and "Select" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_Select (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                  );

    /**
      * This method is triggered left button is released and "Magic Wind" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_MWand (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );

    /**
      * This method is triggered left button is released and "Panning" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_Panning (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                   );

    /**
      * This method is triggered left button is released and "Zooming" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_Zooming (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                   );

    /**
      * This method is triggered left button is released and "Zooming" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_ZoomingNWin (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                   );

    /**
      * This method is triggered left button is released and "Distance" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_Distance (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                    );

    /**
      * This method is triggered left button is released and "Shading" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_Shading (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                   );

    /**
      * This method is triggered left button is released \em any "Annotation Tools" pointer is active.
      */
    virtual void
    mouseWorldReleaseEvent_Annotation (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                      );


    /**
      * This method is triggered when a mouse movement is detected.
      */
    virtual void
    mouseWorldMoveEvent (
                         QMouseEvent* e,        ///< Original QT Event
                         double x,              ///< World Coord. X
                         double y               ///< World Coord. Y
                        );

    /**
      * This method is triggered when a mouse movement is detected and "Select" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_Select (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                               );

    /**
      * This method is triggered when a mouse movement is detected and "Magic Wand" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_MWand (
                             QMouseEvent* e,        ///< Original QT Event
                             double x,              ///< World Coord. X
                             double y               ///< World Coord. Y
                              );

    /**
      * This method is triggered when a mouse movement is detected and "Panning" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_Panning (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                );

    /**
      * This method is triggered when a mouse movement is detected and "Rect-Zooming" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_Zooming (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                );

    /**
      * This method is triggered when a mouse movement is detected and "Distance" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_Distance (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );

    /**
      * This method is triggered when a mouse movement is detected and "Distance" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_Shading (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                                 );


                                 /**
      * This method is triggered when a mouse movement is detected \em any "Annotation Tool" pointer is active.
      */
    virtual void
    mouseWorldMoveEvent_Annotation (
                                     QMouseEvent* e,        ///< Original QT Event
                                     double x,              ///< World Coord. X
                                     double y               ///< World Coord. Y
                                   );

    /**
      * This method is triggered when a double-click event is detected.
      */
    virtual void
    mouseWorldDoubleClickEvent (
                                 QMouseEvent* e,        ///< Original QT Event
                                 double x,              ///< World Coord. X
                                 double y               ///< World Coord. Y
                               ) = 0;


    /**
      * This method is triggered when the right button is pressed.
      */
    virtual void
    rightClickPressEvent (
                         QMouseEvent* e         ///< Original QT Event
                         ) = 0;

    /**
      * This method is triggered when the right button is released.
      */
    virtual void
    rightClickReleaseEvent (
                           QMouseEvent* e       ///< Original QT Event
                           ) = 0;

    // -------------------------------------------------------------------------

    /**
      * Initialize structures for column shading functionality.
      */
    virtual void
    initShadeFlags();

    /**
      * Gets back the shading on/off flag for a given column
      * @return the shading on/off flag for the given column
      */
    virtual  bool             
    getShadeFlag (
                 int pos      ///< the target column
                 );

    /**
      * This method is invoken (by QT event handler) when a given drawing area must be repainted
      */
    virtual void
    drawContents (
                 QPainter * p,      ///< Painter where to draw on
                 int clipx,         ///< left X coordinate (QScrollView coordinates system)
                 int clipy,         ///< upper Y coordinate (QScrollView coordinates system)
                 int clipw,         ///< redrawing area width (QScrollView coordinates system)
                 int cliph          ///< redrawing area height (QScrollView coordinates system)
                 );

    /**
      * This method is invoken to redraw the shaded columns in a given area
      */
    virtual void
    drawShadingColumns (
                       int x1,           ///< left X coordinate
                       int x2,           ///< right X coordinate
                       int y1,           ///< upper Y coordinate
                       int y2,           ///< bottom Y coordinate
                       QPainter* painter ///< Painter where to draw on
                       );

    /**
      * This method is invoken to redraw the grid lines in a given area
      */
    void drawVGridLines (
                       QPainter* painter,    ///< Painter where to draw on
                       double ncx,           ///< left X coordinate
                       double ncy,           ///< upper Y coordinate
                       double ncw,           ///< redrawing area width
                       double nch            ///< redrawing area height
                       );

    /**
      * This method is invoken to redraw the grid lines in a given area
      */
    void drawHGridLines (
                       QPainter* painter,    ///< Painter where to draw on
                       double ncx,           ///< left X coordinate
                       double ncy,           ///< upper Y coordinate
                       double ncw,           ///< redrawing area width
                       double nch            ///< redrawing area height
                       );

    /**
      * This method is invoken (by AScrollViewer) when a given drawing area must be repainted
      */
    virtual bool
    drawWorldCoordinatesContents (
                                 QPainter* p,     ///< Painter where to draw on
                                 int cx,          ///< left X coordinate (World Coordinates System)
                                 int cy,          ///< upper Y coordinate (World Coordinates System)
                                 int cw,          ///< redrawing area width (World Coordinates System)
                                 int ch           ///< redrawing are height (World Coordinates System)
                                 )=0;

    /**
      * This method is used to ensure a minimum size on the off screen pixmap.
      * This off screen pixmap is used to implement double buffering on screen refresh.
      */
    void
    ensureOffScrSize (
                     int osw,  ///< minimum width (in pixels)
                     int osh   ///< minimum height (in pixels)
                     );

    /**
      * Query method to know is some valid data is present in a given matrix <column,row> position
      * @return TRUE means there is valid data in the given position.
      */
    virtual bool             
    hasContentsOn (
                  int col,   ///< Target matrix column
                  int row    ///< Target matrix row
                  ) = 0;

    /**
      * Method triggered when some "implementation specific" object was selected (press+release)
      */
    virtual void
    contentsSelectEvent (
                        int col,                   ///< Matrix Column
                        int row,                   ///< Matrix Row
                        bool adding,               ///< TRUE if Control button pressed
                        bool shifted               ///< RUE if Shift button pressed
                        ) = 0;

    /**
      * Sets new scalling factors (horizontal and vertical) an refresh screen accordingly.
      */
    virtual void
    updateScallingFactor (
                         double sx,           ///< New horizontal scalling factor
                         double sy            ///< New vertical scalling factor
                         );

    /**
      * Method triggered when somehow a scalling factor changes
      */
    virtual void
    updateZoomStatusBar ();

    /**
      * Method triggered when the matrix column pointed by the mouse changes
      */
    virtual void
    updateCycleBar (
                   double x   ///< X coordinate (World Coord. System)
                   );

    /**
      * Method triggered when the matrix row pointed by the mouse changes
      */
    virtual void
    updateRowBar (
                 double x,    ///< X coordinate (World Coord. System)
                 double y     ///< Y coordinate (World Coord. System)
                 );

    /**
      * Method called to draw a rectangle rubber band for rect-zooming tool.
      */
    virtual void
    drawZoomRubber (
                   int x,     ///< X coord. (QScrollView coordinates system)
                   int y      ///< Y coord. (QScrollView coordinates system)
                   );

    /**
      * Method that computes new scalling factors using the final rectangle obtained
      * from the rect-Zoom tool
      */
    virtual void
    computeZoomRect (
                    int pmx,      ///< 'Pressed' X coord. (World Coord. System)
                    int pmy,      ///< 'Pressed' Y coord. (World Coord. System)
                    int rmx,      ///< 'Released' X coord. (World Coord. System)
                    int rmy,      ///< 'Released' Y coord. (World Coord. System)
                    double* nscf_x,
                    double* nscf_y,
                    int* centerx, 
                    int* centery
                    );

    /**
      * Method used to draw the rubber band for the distance tool.
      */
    virtual void
    drawDstRubber (
                  int x,    ///< current (end point) X coord. (QScrollView coordinates system)
                  int y,    ///< current (end point) Y coord. (QScrollView coordinates system)
                  int mx    ///< current (end point) Matrix Column
                  );

    /**
      * Method used to update the gauge distance (matrix columns) on the status bars
      */
    virtual void
    showDistance (
                 int pressed_mx,        ///< Staring point matrix column
                 int released_mx        ///< Ending point matrix column
                 );

    virtual void
    showLastShadded (
                    int mx
                    );

    /**
      * Switchs on/off the shading of a given matrix column and repaint the affected area.
      */
    virtual void
    switchShadeFlag (
                    int col        ///< Target Matrix Column
                    );

    /**
      * Creates an annotation item (depending on the kind of currently active pointer)
      * @see AnnotationFactory
      * @return the right annotation item subclass
      */
    AnnotationItem*           
    newAnnItem();

    /**
      * Changes the cursor bitmap to the right shape (pointer type change)
      */
    void
    updatePointerBitmap();

    virtual void
    applyMagicWand (
                   int mx,
                   int my
                   );

     virtual QString 
     getTipString(bool eventOnly=false);

     virtual void 
     updateTip( QMouseEvent* e );
     
     virtual void 
     cleanTip();
     
     virtual void
     createDistanceAnnotation(int,int,int,int);
     
protected:
    QBrush shaderBrush;
    QPen   shaderPen;
    QColor shadingColor;
    ColumnShadows* cshadows;

    QPixmap offscr;
    QWMatrix preScaleWorldMatrix;

    QColor  bgColor;
    double scf_x;
    double scf_y;
    int realContentsHeight;
    int realContentsWidth;

    bool sTFitKeepARatio;
    bool rectZoomKeepARatio;

    // for pointer & so on
    PointerType currentPointerType;
    int released_mx;    // element-wide (matrix) coordinates
    int released_my;
    int pressed_mx;
    int pressed_my;
    int current_mx;
    int current_my;

    double released_wx;    // world coordinates
    double released_wy;
    double pressed_wx;
    double pressed_wy;
    double current_wx;
    double current_wy;

    int released_x;    // viewport coordinates
    int released_y;
    int pressed_x;
    int pressed_snaped_x;
    int pressed_y;

    int selected_mx;
    int selected_my;

    int lastPan_x;
    int lastPan_y;

    int lastZooming_x;
    int lastZooming_y;

    int lastDistance_x;
    int lastDistance_y;
    int lastDistance_mx;

    int lastShading_mx;
    int lastShaded_mx;
    
    QBrush currentBrush;
    QPen   currentPen;
    QColor currentTextColor;
    int penColorChId;
    int brushColorChId;
    int textColorChId;

    bool panning;
    bool zooming;
    bool measuring;
    bool shading;

    double pi;

    // flag: I am redrawing?
    bool redrawing;

    // for annotation Tools
    AnnotationItem* currentAnnItem;
    AnnotationItem* pressAnnItem;
    AnnotationCtrl* annCtrl;

    // grid lines ?
    bool showHGridLines;
    bool showVGridLines;
    int gridSize;

    QPainter* currentPainter;
    AGTTipLabel* tipLabel;
    
    int myZX;
    int myZY;

    bool zoomLocked;
    bool panLocked;
    bool shadingLocked;
    bool annotationLocked;

    volatile bool wasScrollByMatrix;
    volatile bool wasScrollByZooming;
    bool mdiLocked;
    int mdiNumber;

    int myHorizontalOffset;
    int myVerticalOffset;

    int numElementsToRefresh;

    double user_min_scale_factor;
    double user_max_scale_factor;
};

#endif

