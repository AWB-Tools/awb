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
  * @file  Main.cpp
  */


#include "Main.h"

// some icons
#include "xpm/selectTool.xpm"
#include "xpm/mwandTool.xpm"
#include "xpm/paddingTool.xpm"
#include "xpm/zoomingTool.xpm"
#include "xpm/distanceTool.xpm"
#include "xpm/shadingTool.xpm"
#include "xpm/help_contents.xpm"
#include "xpm/viewmag+.xpm"
#include "xpm/viewmag-.xpm"
#include "xpm/zhp.xpm"
#include "xpm/zhm.xpm"
#include "xpm/zvp.xpm"
#include "xpm/zvm.xpm"
#include "xpm/resetarto.xpm"
#include "xpm/annRect.xpm"
#include "xpm/annCircle.xpm"
#include "xpm/annLine.xpm"
#include "xpm/annText.xpm"
#include "xpm/fillColor.xpm"
#include "xpm/penColor.xpm"
#include "xpm/textColor.xpm"
#include "xpm/zoomUnlocked.xpm"
#include "xpm/paddingLocked.xpm"
#include "xpm/paddingUnlocked.xpm"
#include "xpm/zoomLocked.xpm"
#include "xpm/shadeLocked.xpm"
#include "xpm/shadeUnlocked.xpm"
#include "xpm/selectLocked.xpm"
#include "xpm/selectUnlocked.xpm"

#include "xpm/fileopen.xpm"
#include "xpm/filesave.xpm"
#include "xpm/fileprint.xpm"

// -----------------------------------------------------------------------------
// -- The Main Constructor
// -----------------------------------------------------------------------------
Main::Main(QWidget* parent, const char* name, WFlags f) :
QMainWindow(parent,name,f)
{
    annotationToolsEnabled=false;
    genericOpenEnabled=true;
    genericSaveEnabled=true;
    genericPrintEnabled=true;
    genericCloseEnabled=true;
    bookmarkIOEnabled=true;

    // install graphical message handler...
#ifndef QT_DEBUG
    QErrorMessage::qtHandler();
#endif

    myStatusBar=NULL;
    menu=NULL;
    file=NULL;
    edit=NULL;
    window=NULL;
    window_toolbar=NULL;
    options=NULL;
    view=NULL;
    zfixed=NULL;
    zoneaxe=NULL;
    tools=NULL;
    pointers=NULL;
    annotation=NULL;
    style=NULL;
    bookmark=NULL;
    annotationsTools=NULL;
    viewTools=NULL;
    pointerTools=NULL;
    fileTools=NULL;
    colorTools=NULL;
    mdiTools=NULL;
    bkMgr=NULL;
    brushColorDlg=NULL;
    penColorDlg=NULL;
    fontColorDlg=NULL;
    asv=NULL;

    zoomLocked=false;
    panLocked=false;
    shadingLocked=true;
    highlightLocked=true;
    annotationLocked=true;
    
    win_annotations_bar_id=-1;
    win_view_bar_id=-1;
    win_tool_bar_id=-1;
    win_file_bar_id=-1;
    win_color_bar_id=-1;
    view_grid_id=-1;
    edit_id=-1;
    copy_id=-1;
    paste_id=-1;
    cut_id=-1;
    remove_id=-1;
    options_id=-1;
    close_id=-1;
    zoomin_id=-1;
    zoomout_id=-1;
    zoominh_id=-1;
    zoominv_id=-1;
    zoomouth_id=-1;
    zoomoutv_id=-1;
    rstartio_id=-1;
    z200_id=-1;
    z100_id=-1;
    z50_id=-1;
    z25_id=-1;
    scaletofit_id=-1;
    showgl_id=-1;
    zoomaxes_id=-1;
    scaleto_id=-1;
    addbookmrk_id=-1;
    mbookmrk_id=-1;
    sbookmrk_id=-1;
    ibookmrk_id=-1;
    pointers_id=-1;

    mdiList = new MDIList();

    create_aboutAgt();
}

void
Main::initApplication()
{
    initStatusBar();

    // create the base graphic widget
    asv = getAScrollViewObject();
    if (asv!=NULL)
    {
        setCentralWidget(asv);
    }
    else
    {
        vb = new QVBox( this );
        vb->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
        ws = new QWorkspace( vb );
        ws->setScrollBarsEnabled( TRUE );
        setCentralWidget( vb );
    }

    // use windows as default
    //do_makeStyle("Windows");

    // --------------------------------------------------------------------
    // Toolbars
    // --------------------------------------------------------------------
    addPointerToolBar();
    addFileToolBar();
    addViewToolBar();
    addAnnotationToolBar();
    addColorToolBar();
    addZoomDialog(viewTools);
    addMDIToolBar();
    
    // --------------------------------------------------------------------
    // Main menu
    // --------------------------------------------------------------------
    menu = menuBar();
    addFileMenu();

    addEditMenu();

    addViewMenu();
    addToolsMenu();
    addBookmarkMenu();
    addOptionsMenu();
    addWindowMenu();
    addStylesMenu();
    addHelpMenu();

    // --------------------------------------------------------------------
    // The model bookmark manager
    // --------------------------------------------------------------------
    bkMgr = new BookmarkMgr(this,bookmark,asv);
    Q_ASSERT(bkMgr!=NULL);

    // --------------------------------------------------------------------
    // The zoom dialog
    // --------------------------------------------------------------------
    //if (asv!=NULL) asv->addZoomDialog(viewTools);
}

void
Main::initStatusBar()
{
    myStatusBar = statusBar();

    // create the widgets

    currentCycleLbl = new QLabel(myStatusBar);
    Q_ASSERT(currentCycleLbl!=NULL);
    currentRowLbl = new QLabel(myStatusBar);
    Q_ASSERT(currentRowLbl!=NULL);

    // to ensure enought space....
    currentCycleLbl->setText("Cycle : ");
    currentCycleLbl->setMinimumWidth (200);
    currentRowLbl->setText("Row : ");
    currentRowLbl->setMinimumWidth (250);
    //currentRowLbl->setMaximumWidth (2000);

    // add all my related widgets
    myStatusBar->addWidget(currentCycleLbl,0,true);
    myStatusBar->addWidget(currentRowLbl,0,true);
    //value->setSizeGripEnabled (false);

    // progress bar for redrawing
    progressBar = new QProgressBar(0,myStatusBar);
    Q_ASSERT(progressBar!=NULL);
    progressBar->setPercentageVisible(false);
    progressBar->hide();
    myStatusBar->addWidget(progressBar,0,true);

    myStatusBar->message(getWelcomeMessage(),10000);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addPointerToolBar()
{
    pointerTools = new QToolBar( this, "tools" );Q_ASSERT(pointerTools!=NULL);
    pointerTools->setLabel( "Tools" );

    selectToolIcon = QPixmap( selectTool );
    QToolButton* selectToolBtn = new QToolButton( selectToolIcon, "Select", QString::null,
               this, SLOT(do_selectToolEnabled()), pointerTools, "select" );Q_ASSERT(selectToolBtn!=NULL);

    mwandToolIcon = QPixmap( mwandTool );

    QToolButton* mwandToolBtn = new QToolButton( IconFactory::getInstance()->image19, "Magic Wand", QString::null,
               this, SLOT(do_mwandToolEnabled()), pointerTools, "magic wand" );Q_ASSERT(mwandToolBtn!=NULL);

    paddingToolIcon = QPixmap( paddingTool );
    QToolButton* paddingToolBtn = new QToolButton( paddingToolIcon, "Panning", QString::null,
               this, SLOT(do_paddingToolEnabled()), pointerTools, "panning" );Q_ASSERT(paddingToolBtn!=NULL);

    QToolButton* zoomingToolBtn = new QToolButton( IconFactory::getInstance()->image11, "Zooming", QString::null,
               this, SLOT(do_zoomingToolEnabled()), pointerTools, "zooming" );Q_ASSERT(zoomingToolBtn!=NULL);

    QToolButton* zoomingWToolBtn = new QToolButton( IconFactory::getInstance()->image40, "Zooming Window", QString::null,
               this, SLOT(do_zoomingWToolEnabled()), pointerTools, "zooming window" );

               distanceToolIcon = QPixmap( distanceTool );
    QToolButton* distanceToolBtn = new QToolButton( distanceToolIcon, "Distance", QString::null,
               this, SLOT(do_distanceToolEnabled()), pointerTools, "distance" );Q_ASSERT(distanceToolBtn!=NULL);

    QToolButton* shadingToolBtn = new QToolButton( IconFactory::getInstance()->image5, "Shading", QString::null,
               this, SLOT(do_shadingToolEnabled()), pointerTools, "shading" );Q_ASSERT(shadingToolBtn!=NULL);

    connect( pointerTools, SIGNAL(visibilityChanged(bool)), SLOT(toolsBarVisibilityChanged(bool)) );

    moveDockWindow(pointerTools,DockLeft);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addFileToolBar()
{
    fileTools = new QToolBar( this, "file" );Q_ASSERT(fileTools!=NULL);
    fileTools->setLabel( "File" );

    if (genericOpenEnabled)
    {
        QToolButton* openFileBtn = new QToolButton( IconFactory::getInstance()->image1, "Open", QString::null,
                   this, SLOT(do_genericOpen()), fileTools, "Open" );Q_ASSERT(openFileBtn!=NULL);
    }

    if (genericSaveEnabled)
    {
        QToolButton* saveFileBtn = new QToolButton( IconFactory::getInstance()->image4, "Save", QString::null,
                   this, SLOT(do_genericSave()), fileTools, "Save" );Q_ASSERT(saveFileBtn!=NULL);
    }

    if (genericPrintEnabled)
    {
        printIcon = QPixmap( fileprint );
        QToolButton* printBtn = new QToolButton( printIcon, "Print", QString::null,
                   this, SLOT(do_genericPrint()), fileTools, "Print" );Q_ASSERT(printBtn!=NULL);
    }
    connect( fileTools, SIGNAL(visibilityChanged(bool)), SLOT(fileBarVisibilityChanged(bool)) );
    moveDockWindow(fileTools,DockTop);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void 
Main::addViewToolBar()
{
    viewTools = new QToolBar( this, "view" );Q_ASSERT(viewTools!=NULL);
    viewTools->setLabel( "View" );

    QToolButton* openZInBtn = new QToolButton( IconFactory::getInstance()->image8, "Zoom In", QString::null,
               this, SLOT(do_zoomIn()), viewTools, "zin" );Q_ASSERT(openZInBtn!=NULL);

    QToolButton* openZOutBtn = new QToolButton( IconFactory::getInstance()->image9, "Zoom Out", QString::null,
               this, SLOT(do_zoomOut()), viewTools, "zout" );Q_ASSERT(openZOutBtn!=NULL);

    zhpIcon = QPixmap( zhp );
    QToolButton* openZHPBtn = new QToolButton( zhpIcon, "Zoom In Horizontal", QString::null,
               this, SLOT(do_zoomInH()), viewTools, "zhp" );Q_ASSERT(openZHPBtn!=NULL);

    zhmIcon = QPixmap( zhm );
    QToolButton* openZHMBtn = new QToolButton( zhmIcon, "Zoom Out Horizontal", QString::null,
               this, SLOT(do_zoomOutH()), viewTools, "zhm" );Q_ASSERT(openZHMBtn!=NULL);

    zvpIcon = QPixmap( zvp );
    QToolButton* openZVPBtn = new QToolButton( zvpIcon, "Zoom In Vertical", QString::null,
               this, SLOT(do_zoomInV()), viewTools, "zvp" );Q_ASSERT(openZVPBtn!=NULL);

    zvmIcon = QPixmap( zvm );
    QToolButton* openZVMBtn = new QToolButton( zvmIcon, "Zoom Out Vertical", QString::null,
               this, SLOT(do_zoomOutV()), viewTools, "zvm" );Q_ASSERT(openZVMBtn!=NULL);

    resetartoIcon = QPixmap(resetarto);
    QToolButton* openARTOBtn = new QToolButton( resetartoIcon, "Reset Aspect Ratio", QString::null,
               this, SLOT(do_resetAspectRatio()), viewTools, "arto" );Q_ASSERT(openARTOBtn!=NULL);

    connect( viewTools, SIGNAL(visibilityChanged(bool)), SLOT(viewBarVisibilityChanged(bool)) );

    moveDockWindow( viewTools,DockTop);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addAnnotationToolBar()
{
    annotationsTools = new QToolBar( this, "annotation" );Q_ASSERT(annotationsTools!=NULL);
    annotationsTools->setLabel( "Annotation" );

    annLineIcon = QPixmap( annLine );
    QToolButton* annLineBtn = new QToolButton( annLineIcon, "Draw Line", QString::null,
               this, SLOT(do_annToolLineEnabled()), annotationsTools, "drawline" );Q_ASSERT(annLineBtn!=NULL);

    annRectIcon = QPixmap( annRect );
    QToolButton* annRectBtn = new QToolButton( annRectIcon, "Draw Rectangle", QString::null,
               this, SLOT(do_annToolRectEnabled()), annotationsTools, "drawrect" );Q_ASSERT(annRectBtn!=NULL);

    annCircleIcon = QPixmap( annCircle );
    QToolButton* annArcBtn = new QToolButton( annCircleIcon, "Draw Circle", QString::null,
               this, SLOT(do_annToolCircleEnabled()), annotationsTools, "drawarc" );Q_ASSERT(annArcBtn!=NULL);

    annTextIcon = QPixmap( annText );
    QToolButton* annTxtBtn = new QToolButton( annTextIcon, "Insert Text", QString::null,
               this, SLOT(do_insertText()), annotationsTools, "instext" );Q_ASSERT(annTxtBtn!=NULL);

    connect( annotationsTools, SIGNAL(visibilityChanged(bool)), SLOT(annotationsBarVisibilityChanged(bool)) );

    moveDockWindow( annotationsTools,DockLeft);

    if (!annotationToolsEnabled) annotationsTools->hide();
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addColorToolBar()
{
    int chId;

    colorTools = new QToolBar( this, "colors" );Q_ASSERT(colorTools!=NULL);
    colorTools->setLabel( "Colors" );

    penColorIcon = QPixmap(penColor);
    penColorDlg = new AColorDialog(black, colorTools, "Pen Color", &penColorIcon);Q_ASSERT(penColorDlg!=NULL);
    chId = penColorDlg->registerApplyMethod(asv);
    if (asv!=NULL) asv->setPenColorChgId(chId);
    colorTools->addSeparator();

    fillColorIcon = QPixmap( fillColor );
    brushColorDlg = new AColorDialog(blue, colorTools, "Brush Color", &fillColorIcon);Q_ASSERT(brushColorDlg!=NULL);
    chId = brushColorDlg->registerApplyMethod(asv);
    if (asv!=NULL) asv->setBrushColorChgId(chId);
    colorTools->addSeparator();

    textColorIcon = QPixmap( textColor );
    fontColorDlg = new AColorDialog(black, colorTools, "Font Color", &textColorIcon);Q_ASSERT(fontColorDlg!=NULL);
    chId = fontColorDlg->registerApplyMethod(asv);
    if (asv!=NULL) asv->setTextColorChgId(chId);

    connect( colorTools, SIGNAL(visibilityChanged(bool)), SLOT(colorsBarVisibilityChanged(bool)) );
    moveDockWindow(colorTools,DockTop);
    if (!annotationToolsEnabled) colorTools->hide();
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addMDIToolBar()
{
    paddingLockedIcon = QPixmap(paddingLocked);
    paddingUnlockedIcon = QPixmap(paddingUnlocked);
    zoomLockedIcon = QPixmap(zoomLockedIcn);
    zoomUnlockedIcon = QPixmap(zoomUnlocked);
    shadingLockedIcon = QPixmap(shadeLocked);
    shadingUnlockedIcon = QPixmap(shadeUnlocked);
    highlightLockedIcon = QPixmap(selectLocked);
    highlightUnlockedIcon = QPixmap(selectUnlocked);
    annotationLockedIcon = QPixmap(selectLocked);
    annotationUnlockedIcon = QPixmap(selectUnlocked);
    
    mdiTools = new QToolBar( this, "mdi" );
    Q_ASSERT(mdiTools!=NULL);
    mdiTools->setLabel( "MDI" );

    zoomLockBtn = new QToolButton( zoomUnlockedIcon, "Lock/Unlock Zoom", QString::null,this, SLOT(do_lockUnlockZoom()), mdiTools, "luzoom" );
    zoomLockBtn->setToggleButton(true);
    Q_ASSERT(zoomLockBtn!=NULL);

    panLockBtn = new QToolButton( paddingUnlockedIcon, "Lock/Unlock Panning", QString::null,this, SLOT(do_lockUnlockPan()), mdiTools, "lupan" );
    panLockBtn->setToggleButton(true);
    Q_ASSERT(panLockBtn!=NULL);
    
    shadingLockBtn = new QToolButton( shadingLockedIcon, "Lock/Unlock Shading", QString::null,this, SLOT(do_lockUnlockShading()), mdiTools, "lushading" );
    shadingLockBtn->setToggleButton(true);
    shadingLockBtn->setOn(true);
    Q_ASSERT(shadingLockBtn!=NULL);

    highlightLockBtn = new QToolButton( highlightLockedIcon, "Lock/Unlock Highlighting", QString::null,this, SLOT(do_lockUnlockHighlight()), mdiTools, "luhigh" );
    highlightLockBtn->setToggleButton(true);
    highlightLockBtn->setOn(true);
    Q_ASSERT(highlightLockBtn!=NULL);

    annotationLockBtn = new QToolButton( annotationLockedIcon, "Lock/Unlock Annotations", QString::null,this, SLOT(do_lockUnlockAnnotation()), mdiTools, "luann" );
    annotationLockBtn->setToggleButton(true);
    annotationLockBtn->setOn(true);
    Q_ASSERT(annotationLockBtn!=NULL);

    // disabled by default
    mdiTools->hide();
}

void 
Main::do_lockUnlockAnnotation()
{
    annotationLocked=!annotationLocked;

    // update toolbar icon
    if (annotationLocked)
        annotationLockBtn->setIconSet(annotationLockedIcon);
    else
        annotationLockBtn->setIconSet(annotationUnlockedIcon);
        
    // propagate new mode to MDI clients
    MDIWindow* cmdi;
    AScrollView* casv;
    for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
    {
        casv=cmdi->getAScrollView();
        if(casv) casv->setLockAnnotation(annotationLocked);
    }
}

void 
Main::do_lockUnlockZoom()
{
    zoomLocked=!zoomLocked;

    // update toolbar icon
    if (zoomLocked)
        zoomLockBtn->setIconSet(zoomLockedIcon);
    else
        zoomLockBtn->setIconSet(zoomUnlockedIcon);
        
    // propagate new mode to MDI clients
    MDIWindow* cmdi;
    AScrollView* casv;
    for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
    {
        casv=cmdi->getAScrollView();
        if(casv) casv->setLockZoom(zoomLocked);
    }
}

void 
Main::do_lockUnlockShading()
{
    shadingLocked=!shadingLocked;

    // update toolbar icon
    if (shadingLocked)
        shadingLockBtn->setIconSet(shadingLockedIcon);
    else
        shadingLockBtn->setIconSet(shadingUnlockedIcon);
        
    // propagate new mode to MDI clients
    MDIWindow* cmdi;
    AScrollView* casv;
    for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
    {
        casv=cmdi->getAScrollView();
        if(casv) casv->setLockShading(shadingLocked);
    }
}

void 
Main::do_lockUnlockPan()
{
    panLocked=!panLocked;

    // update toolbar icon
    if (panLocked)
        panLockBtn->setIconSet(paddingLockedIcon);
    else
        panLockBtn->setIconSet(paddingUnlockedIcon);
        
    // propagate new mode to MDI clients
    MDIWindow* cmdi;
    AScrollView* casv;
    for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
    {
        casv=cmdi->getAScrollView();
        if(casv) casv->setLockPan(panLocked);
    }
}

void 
Main::do_lockUnlockHighlight()
{
    highlightLocked=!highlightLocked;

    // update toolbar icon
    if (highlightLocked)
        highlightLockBtn->setIconSet(highlightLockedIcon);
    else
        highlightLockBtn->setIconSet(highlightUnlockedIcon);
        
    // propagate new mode to MDI clients
    MDIWindow* cmdi;
    AScrollView* casv;
    for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
    {
        casv=cmdi->getAScrollView();
        if(casv) casv->setLockPan(highlightLocked);
    }
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addFileMenu()
{
    file = new QPopupMenu( menu );Q_ASSERT(file!=NULL);
    if (genericOpenEnabled) { file->insertItem("&Open", this, SLOT(do_genericOpen()), CTRL+Key_O); }
    if (genericSaveEnabled) { file->insertItem("&Save", this, SLOT(do_genericSave()), CTRL+Key_S); }
    if (genericCloseEnabled) { close_id = file->insertItem("&Close", this, SLOT(do_genericClose()), CTRL+Key_W); }
    if (genericPrintEnabled) { file->insertItem("&Print", this, SLOT(do_genericPrint()), CTRL+Key_P); }
    file->insertSeparator();
    file->insertItem("E&xit", this, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem("&File", file);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addEditMenu()
{
    edit = new QPopupMenu( menu );Q_ASSERT(edit!=NULL);

    copy_id = edit->insertItem("&Copy", this, SLOT(do_copy()), CTRL+Key_C);
    edit->setItemEnabled(copy_id,annotationToolsEnabled);

    paste_id = edit->insertItem("&Paste", this, SLOT(do_paste()), CTRL+Key_V);
    edit->setItemEnabled(paste_id,annotationToolsEnabled);

    cut_id = edit->insertItem("Cu&t", this, SLOT(do_cut()), CTRL+Key_X);
    edit->setItemEnabled(cut_id,annotationToolsEnabled);

    remove_id = edit->insertItem("&Remove", this, SLOT(do_remove()),Key_Delete );
    edit->setItemEnabled(remove_id,annotationToolsEnabled);

    edit->insertSeparator();
    edit_id = menu->insertItem("&Edit", edit);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addViewMenu()
{
    view = new QPopupMenu( menu );Q_ASSERT(view!=NULL);
    view->insertSeparator();

    //QAction ( const QString & text, const QIconSet & icon, const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0, bool toggle = FALSE )

    actionZoomIn = new Q2DMAction ("zoomin",IconFactory::getInstance()->image8,"Zoom &In",CTRL+Key_Plus,this,"zoomin");
    actionZoomIn->addTo(view);
	zoomin_id = actionZoomIn->getMenuId();
    connect(actionZoomIn, SIGNAL(activated()), this, SLOT( do_zoomIn()));

    actionZoomOut = new Q2DMAction ("zoomout",IconFactory::getInstance()->image9,"Zoom &Out",CTRL+Key_Minus,this,"zoomout");
    actionZoomOut->addTo(view);
	zoomout_id = actionZoomOut->getMenuId();
    connect(actionZoomOut, SIGNAL(activated()), this, SLOT( do_zoomOut()));
    view->insertSeparator();

    zoneaxe = new QPopupMenu( view );Q_ASSERT(zoneaxe!=NULL);

    actionZoomInH = new Q2DMAction ("zoominh",zhpIcon,"Zoom In Horizontal",CTRL+ALT+Key_Plus,this,"zoominh");
    actionZoomInH->addTo(zoneaxe);
	zoominh_id = actionZoomInH->getMenuId();
    connect(actionZoomInH, SIGNAL(activated()), this, SLOT( do_zoomInH()));

    actionZoomInV = new Q2DMAction ("zoominv",zvpIcon,"Zoom In Vertical",CTRL+SHIFT+Key_Plus,this,"zoominv");
    actionZoomInV->addTo(zoneaxe);
	zoominv_id = actionZoomInV->getMenuId();
    connect(actionZoomInV, SIGNAL(activated()), this, SLOT( do_zoomInV()));

    actionZoomOutH = new Q2DMAction ("zoomOutH",zhmIcon,"Zoom Out Horizontal",CTRL+ALT+Key_Minus,this,"zoomOutH");
    actionZoomOutH->addTo(zoneaxe);
	zoomouth_id = actionZoomOutH->getMenuId();
    connect(actionZoomOutH, SIGNAL(activated()), this, SLOT( do_zoomOutH()));

    actionZoomOutV = new Q2DMAction ("zoomOutV",zvmIcon,"Zoom Out Vertical",CTRL+SHIFT+Key_Minus,this,"zoomOutV");
    actionZoomOutV->addTo(zoneaxe);
	zoomoutv_id = actionZoomOutV->getMenuId();
    connect(actionZoomOutV, SIGNAL(activated()), this, SLOT( do_zoomOutV()));

    zoomaxes_id = view->insertItem("Zoom Axes ...", zoneaxe);

    actionResetARatio = new Q2DMAction ("resetAspectRatio",resetartoIcon,"&Reset Aspect Ratio",CTRL+Key_R,this,"resetAspectRatio");
    actionResetARatio->addTo(view);
	rstartio_id = actionResetARatio->getMenuId();
    connect(actionResetARatio, SIGNAL(activated()), this, SLOT( do_resetAspectRatio()));

    view->insertSeparator();

    zfixed = new QPopupMenu (view);Q_ASSERT(zfixed!=NULL);
    z200_id = zfixed->insertItem("&200%", this, SLOT(do_z200()));
    z100_id = zfixed->insertItem("&100%", this, SLOT(do_z100()));
    z50_id = zfixed->insertItem("&50%", this, SLOT(do_z50()));
    z25_id = zfixed->insertItem("&25%", this, SLOT(do_z25()));

    scaletofit_id = view->insertItem("&Scale To Fit", this, SLOT(do_scaleToFit()));
    scaleto_id = view->insertItem("Scale To ...", zfixed);
    view->insertSeparator();
    showgl_id = view_grid_id = view->insertItem("Show &Grid Lines", this, SLOT(do_switchShowGridLines()));
    view->setItemChecked(view_grid_id,false);

    menu->insertItem("&View", view);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addToolsMenu()
{
    tools = new QPopupMenu(menu);Q_ASSERT(tools!=NULL);

    pointers = new QPopupMenu(tools);Q_ASSERT(pointers!=NULL);

    actionSelectPointer = new Q2DMAction ("selectToolEnabled",selectToolIcon,"&Select Pointer",CTRL+Key_1,this,"selectToolEnabled");
    actionSelectPointer->addTo(pointers);
    connect(actionSelectPointer, SIGNAL(activated()), this, SLOT( do_selectToolEnabled()));

    actionMWandPointer = new Q2DMAction ("mwandToolEnabled",IconFactory::getInstance()->image19,"&Magic Wand Pointer",CTRL+Key_2,this,"mwandToolEnabled");
    actionMWandPointer->addTo(pointers);
    connect(actionMWandPointer, SIGNAL(activated()), this, SLOT( do_mwandToolEnabled()));

    actionPanningPointer = new Q2DMAction ("paddingToolEnabled",paddingToolIcon,"&Panning Pointer",CTRL+Key_3,this,"paddingToolEnabled");
    actionPanningPointer->addTo(pointers);
    connect(actionPanningPointer, SIGNAL(activated()), this, SLOT( do_paddingToolEnabled()));

    actionZoomingPointer = new Q2DMAction ("zoomingToolEnabled",IconFactory::getInstance()->image11,"&Zooming Pointer",CTRL+Key_4,this,"zoomingToolEnabled");
    actionZoomingPointer->addTo(pointers);
    connect(actionZoomingPointer, SIGNAL(activated()), this, SLOT( do_zoomingToolEnabled()));

    actionZoomingWPointer = new Q2DMAction ("zoomingWToolEnabled",IconFactory::getInstance()->image40,"&Zooming on Window Pointer",CTRL+Key_5,this,"zoomingWToolEnabled");
    actionZoomingWPointer->addTo(pointers);
    connect(actionZoomingWPointer, SIGNAL(activated()), this, SLOT( do_zoomingToolEnabled()));

    actionDistancePointer = new Q2DMAction ("distanceToolEnabled",distanceToolIcon,"&Distance Pointer",CTRL+Key_6,this,"distanceToolEnabled");
    actionDistancePointer->addTo(pointers);
    connect(actionDistancePointer, SIGNAL(activated()), this, SLOT( do_distanceToolEnabled()));

    actionShadingPointer = new Q2DMAction ("shadingToolEnabled",IconFactory::getInstance()->image5,"S&hading Pointer",CTRL+Key_7,this,"shadingToolEnabled");
    actionShadingPointer->addTo(pointers);
    connect(actionShadingPointer, SIGNAL(activated()), this, SLOT( do_shadingToolEnabled()));

    pointers_id = tools->insertItem("&Pointers",pointers);

    if (annotationToolsEnabled)
    {
        addAnnotationMenu();
    }

    menu->insertItem("&Tools", tools);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addAnnotationMenu()
{
    annotation = new QPopupMenu(tools);Q_ASSERT(annotation!=NULL);
    annotation->insertItem(annLineIcon,"Draw &Line", this, SLOT(do_annToolLineEnabled()));
    annotation->insertItem(annRectIcon,"Draw &Rectangle", this, SLOT(do_annToolRectEnabled()));
    annotation->insertItem(annCircleIcon,"Draw &Circle", this, SLOT(do_annToolCircleEnabled()));
    annotation->insertItem(annTextIcon,"Insert &Text", this, SLOT(do_insertText()));
    tools->insertItem("&Annotation tools",annotation);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addBookmarkMenu()
{
    bookmark = new QPopupMenu(menu);Q_ASSERT(bookmark!=NULL);
    actionAddBM = new Q2DMAction ("addBookmark",IconFactory::getInstance()->image14,"&Add Bookmark Here",CTRL+Key_B,this,"addBookmark");
    actionAddBM->addTo(bookmark);
	addbookmrk_id = actionAddBM->getMenuId();  
    connect(actionAddBM, SIGNAL(activated()), this, SLOT( do_addBookmark()));

    mbookmrk_id = bookmark->insertItem(IconFactory::getInstance()->image15,"&Manage Bookmarks", this, SLOT(do_manageBookmark()));
    if (bookmarkIOEnabled)
    {
        bookmark->insertSeparator();
        ibookmrk_id = bookmark->insertItem("&Import Bookmarks", this, SLOT(do_importBookmarks()));
        sbookmrk_id = bookmark->insertItem("&Save Bookmarks", this, SLOT(do_saveBookmarks()));
    }
    bookmark->insertSeparator();
    menu->insertItem("&Bookmarks", bookmark);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addOptionsMenu()
{
    options = new QPopupMenu( menu );Q_ASSERT(options!=NULL);
    options->insertItem("&Grid Lines...", this, SLOT(do_snapDialog()));
    options_id = menu->insertItem("&Options",options);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addWindowMenu()
{
    window = new QPopupMenu(menu);Q_ASSERT(window!=NULL);

    // -------------------------------------------------------
    // toolbars stuff
    // -------------------------------------------------------
    window_toolbar = new QPopupMenu(window);Q_ASSERT(window_toolbar!=NULL);

    win_tool_bar_id = window_toolbar->insertItem("&Tools", this, SLOT(do_switchToolsBar()));
    window_toolbar->setItemChecked(win_tool_bar_id,true);

    win_file_bar_id = window_toolbar->insertItem("&File", this, SLOT(do_switchFileBar()));
    window_toolbar->setItemChecked(win_file_bar_id,true);

    win_view_bar_id = window_toolbar->insertItem("&View", this, SLOT(do_switchViewBar()));
    window_toolbar->setItemChecked(win_view_bar_id,true);

    if (annotationToolsEnabled)
    {
        win_annotations_bar_id = window_toolbar->insertItem("&Annotations", this, SLOT(do_switchAnnotationsBar()));
        window_toolbar->setItemChecked(win_annotations_bar_id,true);

        win_color_bar_id = window_toolbar->insertItem("&Colors", this, SLOT(do_switchColorsBar()));
        window_toolbar->setItemChecked(win_color_bar_id,true);
    }

    window->insertItem("&Toolbars...",window_toolbar);
    //window->insertSeparator();
    menu->insertItem("&Window",window);
    window->setCheckable( TRUE );
}

 
// --------------------------------------------------------------------
// Styles stuff
// --------------------------------------------------------------------
void 
Main::addStylesMenu()
{
    style = new QPopupMenu(menu);Q_ASSERT(style!=NULL);
    style->setCheckable( TRUE );
    options->insertItem("&Style",style);
    style->setCheckable( TRUE );

    QActionGroup *ag = new QActionGroup( menu, 0 );Q_ASSERT(ag!=NULL);
    ag->setExclusive(TRUE);
    QSignalMapper *styleMapper = new QSignalMapper( this );Q_ASSERT(styleMapper!=NULL);
    connect( styleMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( do_makeStyle(const QString&)));
    QStringList list = QStyleFactory::keys();
    list.sort();
    QDict<int> stylesDict( 17, FALSE );

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        QString styleStr = *it;
        QString styleAccel = styleStr;
        if ( stylesDict[styleAccel.left(1)] )
        {
            for ( uint i = 0; i < styleAccel.length(); i++ )
            {
                if ( !stylesDict[styleAccel.mid( i, 1 )] )
                {
                    stylesDict.insert(styleAccel.mid( i, 1 ), (const int *)1);
                    styleAccel = styleAccel.insert( i, '&' );
                    break;
                }
            }
        }
        else
        {
            stylesDict.insert(styleAccel.left(1), (const int *)1);
            styleAccel = "&"+styleAccel;
        }

        QAction *a = new QAction( styleStr, QIconSet(), styleAccel, 0, ag, 0, ag->isExclusive() );Q_ASSERT(a!=NULL);
        connect( a, SIGNAL( activated() ), styleMapper, SLOT(map()) );
        styleMapper->setMapping( a, a->text() );
    }
    ag->addTo(style);
}

// --------------------------------------------------------------------
//
// --------------------------------------------------------------------
void
Main::addHelpMenu()
{
    menu->insertSeparator();
    QPopupMenu* help = new QPopupMenu( menu );Q_ASSERT(help!=NULL);
    help->insertItem("About &QT", this, SLOT(do_about_qt()));
    help->insertItem("About &AGT", this, SLOT(do_about_agt()));
    help->insertItem(IconFactory::getInstance()->image18,"&Contents", this, SLOT(do_help()), Key_F1);
    menu->insertItem("&Help",help);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::do_switchToolsBar()
{
    bool s = !window->isItemChecked(win_tool_bar_id);
    window->setItemChecked(win_tool_bar_id,s);
    if (s)
    {
        pointerTools->show();
    }    
    else
    {
        pointerTools->hide();
    }    
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::do_switchViewBar()
{
    bool s = !window->isItemChecked(win_view_bar_id);
    window->setItemChecked(win_view_bar_id,s);
    if (s)
    {
        viewTools->show();
    }    
    else
    {
        viewTools->hide();
    }    
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::do_switchAnnotationsBar()
{
    bool s = !window->isItemChecked(win_annotations_bar_id);
    window->setItemChecked(win_annotations_bar_id,s);
    if (s)
    {
        annotationsTools->show();
    }    
    else
    {
        annotationsTools->hide();
    }    
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::do_switchFileBar()
{
    bool s = !window->isItemChecked(win_file_bar_id);
    window->setItemChecked(win_file_bar_id,s);
    if (s)
    {
        fileTools->show();
    }    
    else
    {
        fileTools->hide();
    }    
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::do_switchColorsBar()
{
    bool s = !window->isItemChecked(win_color_bar_id);
    window->setItemChecked(win_color_bar_id,s);
    if (s)
    {
        colorTools->show();
    }    
    else
    {
        colorTools->hide();
    }    
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::do_switchShowGridLines()
{
    bool s = !view->isItemChecked(view_grid_id);
    view->setItemChecked(view_grid_id,s);
    // update AScrollView object state:
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->setShowGridLines(s);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void 
Main::toolsBarVisibilityChanged(bool visible)
{
    window->setItemChecked(win_tool_bar_id,visible); 
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::fileBarVisibilityChanged(bool visible)
{
    window->setItemChecked(win_file_bar_id,visible); 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::viewBarVisibilityChanged(bool visible)
{
    window->setItemChecked(win_view_bar_id,visible); 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::annotationsBarVisibilityChanged(bool visible)
{
    window->setItemChecked(win_annotations_bar_id,visible);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::colorsBarVisibilityChanged(bool visible)
{
    window->setItemChecked(win_color_bar_id,visible); 
}


void
Main::do_selectToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_SELECT);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_SELECT);
        }
    }
}

void 
Main::do_mwandToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_MWAND);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_MWAND);
        }
    }
}

void 
Main::do_paddingToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_PANNING);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_PANNING);
        }
    }
}

void
Main::do_zoomingToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_ZOOMING);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_ZOOMING);
        }
    }
}

void
Main::do_zoomingWToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_ZOOMING_NWIN);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_ZOOMING_NWIN);
        }
    }
}

void 
Main::do_distanceToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_DISTANCE);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_DISTANCE);
        }
    }
}

void
Main::do_shadingToolEnabled()
{
    if (asv!=NULL)
    {
        asv->setPointerType(POINTER_SHADING);
    }
    else
    {
        MDIWindow* cmdi;
        AScrollView* casv;
        for ( cmdi = mdiList->first(); cmdi; cmdi = mdiList->next() )
        {
            casv=cmdi->getAScrollView();
            if(casv) casv->setPointerType(POINTER_SHADING);
        }
    }
}

void 
Main::do_annToolLineEnabled()
{
    qWarning("Feature unstable");
    qApp->beep();
    if (asv!=NULL) asv->setPointerType(POINTER_DRAWLINE);
}

void
Main::do_annToolRectEnabled()
{
    qWarning("Feature unstable");
    qApp->beep();
    if (asv!=NULL) asv->setPointerType(POINTER_DRAWRECTANGLE);
}

void 
Main::do_annToolCircleEnabled()
{
    qWarning("Feature unstable");
    qApp->beep();
    if (asv!=NULL) asv->setPointerType(POINTER_DRAWCIRCLE);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
Main::~Main()
{
    if (annotationsTools!=NULL) delete annotationsTools;
    if (viewTools!=NULL) delete viewTools;
    if (pointerTools!=NULL) delete pointerTools;
    if (fileTools!=NULL) delete fileTools;
    if (colorTools!=NULL) delete colorTools;

    if (file!=NULL) delete file;
    if (edit!=NULL) delete edit;
    if (window!=NULL) delete window;
    if (options!=NULL)  delete options;
    if (view!=NULL) delete view;
    if (zfixed!=NULL) delete zfixed;
    if (zoneaxe!=NULL) delete zoneaxe;
    if (tools!=NULL) delete tools;
    if (pointers!=NULL) delete pointers;
    if (annotation!=NULL) delete annotation;
    if (style!=NULL) delete style;
    if (bookmark!=NULL) delete bookmark;

    if (bkMgr!=NULL) delete bkMgr;
    if (myStatusBar!=NULL) delete myStatusBar;
    if (menu!=NULL) delete menu;
    if (asv!=NULL) delete asv;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_help()
{
    // just tmp:
    QString home = QString("example.html");
    HelpWindow *help = new HelpWindow(home, ".", NULL, "Help Viwer");Q_ASSERT(help!=NULL);
    help->setCaption("Helpviewer");

    if ( QApplication::desktop()->width() > 400 && QApplication::desktop()->height() > 500 )
    {
        help->show();
    }
    else
    {
        help->showMaximized();
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_about_agt()
{
    about->show();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::create_aboutAgt()
{
    QString stros(STR_OS);
#ifdef PLATFORM_IA32_WINDOWS_VS
    stros = "Microsoft Windows";
    WindowsVersion wver = qApp->winVersion();
    char* strwver;
    switch (wver)
    {
        case WV_95:
        strwver = "95";
        break;

        case WV_98:
        strwver = "98";
        break;

        case WV_Me:
        strwver = "Millennium Edition";
        break;

        case WV_NT:
        strwver = "NT";
        break;

        case WV_2000:
        strwver = "2000";
        break;

        case WV_XP:
        strwver = "XP";
        break;

        default;
        strwver = "(Unknown version)";
    }
#endif

    QString msg="";
    msg = msg + "<h2>Architecture Graphic Tools (AGT) project</h2>" ;
    msg = msg + "<h3>AGT is framework aimed to ease the development of <br>";
    msg = msg + "graphic tools for Computer Architecture.<br></h3>";
    msg = msg + "<h3>This library was developed in BSSAD,<br>Intel Labs Barcelona</h3>";
    msg = msg + "<h4>Build information: </h4>";
    msg = msg + "<ul>";
    msg = msg + "<li>Library Version: " STR_AGT_MAJOR_VERSION "." STR_AGT_MINOR_VERSION ;
    msg = msg + "<li>Platform: " STR_PLATFORM ;
    msg = msg + "<li>OS: " + stros;
#ifdef PLATFORM_IA32_WINDOWS_VS
    msg = msg + " " + strwver;
#endif
    msg = msg + "</ul><br>" ;
    msg = msg + "</ul>";
    about = new QMessageBox("AGT Information",msg,QMessageBox::Information,1, 0, 0, this, 0, FALSE);
    Q_ASSERT(about!=NULL);
    about->setButtonText( 1, "Dismiss" );
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_about_qt()
{
    QMessageBox::aboutQt(this,"QT Library Information");
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_zoomIn()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->ZoomIn();
}

void
Main::do_zoomOut()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->ZoomOut();
}

void
Main::do_zoomInH()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->ZoomInX();

}

void
Main::do_zoomOutH()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->ZoomOutX();
}

void
Main::do_zoomInV()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->ZoomInY();
}

void
Main::do_zoomOutV()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->ZoomOutY();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_resetAspectRatio()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv)
    {
        double x,y,r;
        x = casv->getScallingFactorX();
        y = casv->getScallingFactorY();
        r = x < y ? x : y;
        casv->setScallingFactor(r);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_scaleToFit()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->scaleToFit();
}

void
Main::do_z200()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->setScallingFactor(2.0);
}

void
Main::do_z100()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->setScallingFactor(1.0) ;
}

void
Main::do_z50()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->setScallingFactor(0.5) ;
}

void
Main::do_z25()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL) casv->setScallingFactor(0.25) ;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void Main::do_makeStyle(const QString &style)
{
    QFont appFont = QApplication::font();

    qApp->setStyle(style);
    if(style == "Platinum")
    {
        QPalette p( QColor( 239, 239, 239 ) );
        qApp->setPalette( p, TRUE );
        qApp->setFont( appFont, TRUE );
    }
    else if(style == "Windows")
    {
        qApp->setFont( appFont, TRUE );
    }
    else if(style == "CDE")
    {
        QPalette p( QColor( 75, 123, 130 ) );
        p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
        p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
        p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
        p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
        p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
        p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
        p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
        p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
        p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
        p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
        p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
        p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
        p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
        p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
        p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
        p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
        p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
        p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );
        qApp->setPalette( p, TRUE );
        qApp->setFont( QFont( "times", appFont.pointSize() ), TRUE );
    }
    else if (style == "Motif" || style == "MotifPlus")
    {
        QPalette p( QColor( 192, 192, 192 ) );
        qApp->setPalette( p, TRUE );
        qApp->setFont( appFont, TRUE );
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_addBookmark()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();

    if (casv!=NULL)
    {
        double scx = casv->getScallingFactorX();
        double scy = casv->getScallingFactorY();
        int x  = casv->contentsX();
        int y  = casv->contentsY();
        bkMgr->addBookmark(scx,scy,x,y);
    }
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_manageBookmark()
{
    if (bkMgr) bkMgr->runDialog();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_resetBookmarks()
{
    if (bkMgr) bkMgr->reset();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_importBookmarks()
{
    if (bkMgr) bkMgr->importBookmarks(); 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_saveBookmarks()
{
    if (bkMgr) bkMgr->exportBookmarks(); 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_insertText()
{
    qWarning("Feature not implemented");
    qApp->beep();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_copy()
{
    qWarning("Feature not implemented");
    qApp->beep();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_paste()
{
    qWarning("Feature not implemented");
    qApp->beep();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_cut()
{
    qWarning("Feature not implemented");
    qApp->beep();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_remove()
{
    qWarning("Feature not implemented");
    qApp->beep();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void
Main::do_snapDialog()
{
    AScrollView* casv = asv;
    if (!casv) casv = getActiveASV();
    if (casv!=NULL)
    {
        snapDialogImpl* sd = new snapDialogImpl(this,"snap dialog",true);
        Q_ASSERT(sd!=NULL);
        sd->setSnap(casv->getSnapToGrid());
        sd->setSnapSize(casv->getGridLinesSize());
    
        int result = sd->exec();
        if (result)
        {
            casv->setGridLinesSize(sd->getSnapSize());
            casv->setSnapToGrid(sd->snap());
        }
    }
}

MDIWindow* 
Main::getActiveMDI()
{ 
    if (!ws) return NULL;
    return (MDIWindow*)(ws->activeWindow());
}

AScrollView* 
Main::getActiveASV()
{ 
    if (asv) return asv;
    
    MDIWindow* mdi;
    if (!ws) return NULL;
    mdi = (MDIWindow*)(ws->activeWindow());
    if (!mdi) return NULL;
    return mdi->getAScrollView();
}

void 
Main::cycle_msg_changed(QString msg)
{
    if (currentCycleLbl) currentCycleLbl->setText(msg);
}

void 
Main::row_msg_changed(QString msg)
{
    if (currentRowLbl) currentRowLbl->setText(msg);
}
    
void 
Main::progress_cnt_changed(int value)
{
    if (progressBar!=NULL) progressBar->setProgress(value);
}
    
void 
Main::progress_reinit(int value)
{
    if (progressBar)
    {
        progressBar->setTotalSteps(value);
        progressBar->adjustSize();
        progressBar->reset();
    }
}
    
void 
Main::progress_show()
{
    if (progressBar!=NULL) progressBar->show();
}

void 
Main::progress_hide()
{
    if (progressBar!=NULL) progressBar->hide();
}

void 
Main::status_msg_changed(QString msg)
{
    myStatusBar->message(msg,10000);
}

void
Main::addZoomDialog(QToolBar* parent)
{
    parent->addSeparator();

    //QHBox* qhb = new QHBox(parent);Q_ASSERT(qhb!=NULL);
    //qhb->setSpacing(10);
    //QPixmap zoomingToolIcon = QPixmap( zoomingToolBar );
    //QLabel* icnLbl = new QLabel(qhb);Q_ASSERT(icnLbl!=NULL);
    //icnLbl->setPixmap(zoomingToolIcon);

    myZoomCombo    = new QComboBox(false,parent,"scaling_factor");Q_ASSERT(myZoomCombo!=NULL);
    QToolTip::add(myZoomCombo,"Scaling Factor");
    
    // current
    myZoomCombo->insertItem( " 100% ");
    // put std zooming values:
    myZoomCombo->insertItem( " 200% ");
    myZoomCombo->insertItem( " 150% ");
    myZoomCombo->insertItem( " 125% ");
    myZoomCombo->insertItem( " 100% ");
    myZoomCombo->insertItem( " 75% ");
    myZoomCombo->insertItem( " 50% ");
    myZoomCombo->insertItem( " 25% ");
    myZoomCombo->insertItem( " 10% ");
    myZoomCombo->insertItem( "  5% ");
    myZoomCombo->insertItem( "  1% ");
    myZoomCombo->insertItem( " Fit ");
    myZoomCombo->insertItem( " Fit Vertical ");
    myZoomCombo->insertItem( " Fit Horizontal ");
    // to ensure enought space....
    myZoomCombo->setCurrentItem(0);
    // Connect the activated SIGNALs of the Comboboxes with SLOTs
    connect( myZoomCombo,SIGNAL(activated(const QString &)),this,SLOT(slotZoomComboActivated(const QString &)));
}
void
Main::slotZoomComboActivated( const QString &s )
{
    if (myZoomCombo==NULL) { return; }

    AScrollView* casv = getActiveASV();
    if (casv)
    {
        switch (myZoomCombo->currentItem())
        {
            case  0: return;
            case  1: casv->setScallingFactor(2.00); break;
            case  2: casv->setScallingFactor(1.50); break;
            case  3: casv->setScallingFactor(1.25); break;
            case  4: casv->setScallingFactor(1.00); break;
            case  5: casv->setScallingFactor(0.75); break;
            case  6: casv->setScallingFactor(0.50); break;
            case  7: casv->setScallingFactor(0.25); break;
            case  8: casv->setScallingFactor(0.10); break;
            case  9: casv->setScallingFactor(0.05); break;
            case 10: casv->setScallingFactor(0.01); break;
            case 11: casv->scaleToFit(); break;
            case 12: casv->scaleToFitVertical(); break;
            case 13: casv->scaleToFitHorizontal(); break;
        }
    }
}

QString
Main::getWelcomeMessage()
{
    return "Put your welcome message here";
}

void
Main::zoomStatusChanged(int zx, int zy)
{
    if (myZoomCombo==NULL) { return; }

    if (fabs(zx-zy)<1)
    {
        myZoomCombo->changeItem(QString::number(zx) + "%  ",0);
    }
    else
    {
        myZoomCombo->changeItem(QString("<") + QString::number(zx) +
        "%," + QString::number(zy) + "%>  ",0);
    }
    myZoomCombo->setCurrentItem(0);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -- The main AGT method
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
int
agtMain(Main* m)
{
    m->setMinimumSize(800,600);
    m->resize(1024,768);
    //qApp->setMainWidget(m);

    m->setCaption(m->getApplicationCaption());

    if ( QApplication::desktop()->width() > m->width() + 10
     && QApplication::desktop()->height() > m->height() + 30 )
    {
        m->show();
    }
    else
    {
        m->showMaximized();
    }

    qApp->setMainWidget(m);
    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
    return qApp->exec();
}


