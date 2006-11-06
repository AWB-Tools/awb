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

/****************************************************************************
** Form implementation generated from reading ui file 'bookmark.ui'
**
** Created: Thu Jan 9 11:13:39 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "bookmark.h"

#include <qvariant.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a BookMarkForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
BookMarkForm::BookMarkForm( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "BookMarkForm" );
    resize( 576, 333 ); 
    setCaption( trUtf8( "Bookmark Manager" ) );
    BookMarkFormLayout = new QGridLayout( this, 1, 1, 11, 6, "BookMarkFormLayout"); 

    BookMarkList = new QListBox( this, "BookMarkList" );
    BookMarkList->setMinimumSize( QSize( 0, 0 ) );

    BookMarkFormLayout->addWidget( BookMarkList, 0, 0 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setFrameShadow( QGroupBox::Sunken );
    GroupBox1->setTitle( trUtf8( "Operations:" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 6 );
    GroupBox1->layout()->setMargin( 11 );
    GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    QSpacerItem* spacer = new QSpacerItem( 0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox1Layout->addItem( spacer );

    Layout8 = new QVBoxLayout( 0, 0, 6, "Layout8"); 

    Layout2 = new QVBoxLayout( 0, 0, 6, "Layout2"); 

    UpButton = new QPushButton( GroupBox1, "UpButton" );
    UpButton->setText( trUtf8( "&Up" ) );
    Layout2->addWidget( UpButton );

    DownButton = new QPushButton( GroupBox1, "DownButton" );
    DownButton->setText( trUtf8( "&Down" ) );
    Layout2->addWidget( DownButton );
    Layout8->addLayout( Layout2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    Layout8->addItem( spacer_2 );

    RemoveButton = new QPushButton( GroupBox1, "RemoveButton" );
    RemoveButton->setText( trUtf8( "&Remove" ) );
    Layout8->addWidget( RemoveButton );
    GroupBox1Layout->addLayout( Layout8 );
    QSpacerItem* spacer_3 = new QSpacerItem( 0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox1Layout->addItem( spacer_3 );
    QSpacerItem* spacer_4 = new QSpacerItem( 0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox1Layout->addItem( spacer_4 );
    QSpacerItem* spacer_5 = new QSpacerItem( 0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox1Layout->addItem( spacer_5 );
    QSpacerItem* spacer_6 = new QSpacerItem( 0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox1Layout->addItem( spacer_6 );

    CloseButton = new QPushButton( GroupBox1, "CloseButton" );
    CloseButton->setText( trUtf8( "&Close" ) );
    GroupBox1Layout->addWidget( CloseButton );

    BookMarkFormLayout->addWidget( GroupBox1, 0, 2 );

    Line1 = new QFrame( this, "Line1" );
    Line1->setFrameShape( QFrame::VLine );
    Line1->setFrameShadow( QFrame::Sunken );
    Line1->setFrameShape( QFrame::VLine );

    BookMarkFormLayout->addWidget( Line1, 0, 1 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BookMarkForm::~BookMarkForm()
{
    // no need to delete child widgets, Qt does it all for us
}

