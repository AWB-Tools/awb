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
  * @file  AColorDialog.cpp
  */


#include "AColorDialog.h"

// icons
#include "xpm/pdown.xpm"

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// AColorRect methods
// ------------------------------------------------------------------
// ------------------------------------------------------------------

QSize
AColorRect::sizeHint() const
{
    return QSize(20,5);
}

void
AColorRect::drawContents ( QPainter * p )
{
    if (!noColorFlag)
    {
        p->setBrush(QBrush(myColor));
        p->setPen(QPen(QColor(0,0,0),1));
    }
    else
    {
        p->setBrush(NoBrush);
        p->setPen(QPen(QColor(150,150,150),1));
    }
    p->drawRect(1,0,19,5);
}



// ------------------------------------------------------------------
// ------------------------------------------------------------------
// AColorDialog methods
// ------------------------------------------------------------------
// ------------------------------------------------------------------
QColor
AColorDialog::stdColors[] =
{
        black,
        white,
        darkGray,
        gray,
        lightGray,
        red,
        green,
        blue,
        cyan,
        magenta,
        yellow,
        darkRed,
        darkGreen,
        darkBlue,
        darkCyan,
        darkMagenta,
        darkYellow
};

int 
AColorDialog::static_id=0;

AColorDialog::AColorDialog (QColor defColor, QWidget *parent, const char *name, QPixmap* icon) :
QHBox( parent, name )
{
    myId = static_id;
    static_id++;

    setMargin(0);
    setSpacing(0);

    // defaults
    lastActivatedItem = -1;
    noColorFlag    = false;
    currentColor = defColor;
    myInterfaceObj = NULL;

    // create std color array
    int i;
    for (i=0;i<NUM_STD_COLORS;i++)
    {
        stdColorItems[i] = new ColorMenuItem(stdColors[i]);
        Q_ASSERT(stdColorItems[i]!=NULL);
    }

    // create the buttom for color apply
    QVBox* vbox = new QVBox(this,"color_v_box");
    Q_ASSERT(vbox!=NULL);
    vbox->setMargin(0);
    vbox->setSpacing(0);
    vbox->setMaximumWidth(20);
    vbox->setMaximumHeight(28);
    bt_app = new AToolButton(vbox);
    Q_ASSERT(bt_app!=NULL);
    bt_app->setAutoRaise(true);

    if (icon!=NULL)
    {
        bt_app->setPixmap(*icon);
    }
    acr = new AColorRect(vbox,currentColor);Q_ASSERT(acr!=NULL);
    connect( bt_app, SIGNAL( clicked() ), this, SLOT( colorApply() ) );

    // create the buttom for menu popup
    downIcon = QPixmap( pdown );
    bt_pop = new AToolButton(this);Q_ASSERT(bt_pop!=NULL);
    bt_pop->setMaximumWidth (16);
    bt_pop->setPixmap(downIcon);
    bt_pop->setAutoRaise(true);
    connect(bt_pop,SIGNAL( pressed() ), this, SLOT( popupShow() ) );

    // and the popup menu
    pop = new QPopupMenu(parent,"qt_color_menu");Q_ASSERT(pop!=NULL);
    connect(pop,SIGNAL(activated(int)),this, SLOT( popMenuActivated(int) ) );

    // now put in all the "standard" colors
    noColorId = pop->insertItem("No color");

    for (i=0;i<NUM_STD_COLORS;i++)
    {
        int id = pop->insertItem(stdColorItems[i]);
        pop->connectItem(id,this,SLOT(stdColorActivated()));
    }

    moreColorsId = pop->insertItem("More colors...");
}

AColorDialog::~AColorDialog ()
{
    int i;
    for (i=0;i<NUM_STD_COLORS;i++)
    {
        delete stdColorItems[i];
    }

    delete acr;
    delete bt_app;
    delete bt_pop;
    delete pop;
}


void 
AColorDialog::colorApply()
{
    // invoke registered call
    if (myInterfaceObj!=NULL)
    {
        myInterfaceObj->colorApply(currentColor,noColorFlag,myId);
    }
}

void
AColorDialog::popupShow()
{
    pop->exec(mapToGlobal(bt_pop->geometry().bottomLeft()));
    bt_pop->setDown(false);
}

void 
AColorDialog::popMenuActivated (int id)
{
    lastActivatedItem = id;

    // check for "more colors dialog"
    if (id==moreColorsId)
    {
        QColor selColor = QColorDialog::getColor();
        if (selColor.isValid())
        {
            currentColor = selColor;
            noColorFlag = false;
            acr->setNoColor(false);
            acr->setColor(currentColor);
            if (myInterfaceObj!=NULL)
            {
                myInterfaceObj->colorChanged(currentColor,noColorFlag,myId);
            }    
        }
    }
    else if (id==noColorId)
    {
        noColorFlag = true;
        acr->setNoColor(true);
        acr->setColor(currentColor);
        if (myInterfaceObj!=NULL)
        {
            myInterfaceObj->colorChanged(currentColor,noColorFlag,myId);
        }    
    }
}

void 
AColorDialog::stdColorActivated()
{
    int pos = pop->indexOf(lastActivatedItem);
    currentColor = stdColorItems[pos-1]->getColor();
    noColorFlag = false;
    acr->setNoColor(false);
    acr->setColor(currentColor);
    if (myInterfaceObj!=NULL)
        myInterfaceObj->colorChanged(currentColor,noColorFlag,myId);
}

