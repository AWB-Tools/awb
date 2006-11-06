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
  * @file  snapDialogImpl.cpp
  */

#include "snapDialogImpl.h"

/*
 *  Constructs a snapDialogImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
snapDialogImpl::snapDialogImpl(QWidget* parent,  const char* name, bool modal, WFlags fl)
    : Form1( parent, name, modal, fl )
{
    // defaults
    doSnap = true;
    snapSize = 10;

    CheckBoxSnap->setChecked(doSnap);
    LineEditDst->setText(QString::number((int)snapSize));
    SliderDst->setValue(snapSize);

    connect(PushButton1,SIGNAL(clicked()),this,SLOT(acceptClicked()));
    connect(PushButton2,SIGNAL(clicked()),this,SLOT(cancelClicked()));
    connect(LineEditDst,SIGNAL(textChanged(const QString &)),this,SLOT(textChanged(const QString &)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
snapDialogImpl::~snapDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void
snapDialogImpl::Slider1_sliderMoved(int value)
{
    LineEditDst->setText(QString::number((int)value));
}


QSize
snapDialogImpl::sizeHint() const
{
    return QSize(400,150); 
}


bool
snapDialogImpl::snap()
{
    return doSnap;
}

int
snapDialogImpl::getSnapSize()
{
    return snapSize; 
}


void
snapDialogImpl::setSnap(bool v)
{
    doSnap = v;
    CheckBoxSnap->setChecked(doSnap);
}

void
snapDialogImpl::setSnapSize(int v)
{
    snapSize = v;
    LineEditDst->setText(QString::number((int)v));
}

void
snapDialogImpl::acceptClicked()
{
    bool atoi;
    int tmp = LineEditDst->text().toInt(&atoi);
    if ( (!atoi) || (tmp<2) || (tmp>100) )
    {
        // deny ...
        QMessageBox::warning (this,"The introduced text is not a valid",
        "Input error, valid range between 2 and 100",
        QMessageBox::NoButton,QMessageBox::NoButton,QMessageBox::NoButton);

        return;
    }

    doSnap = CheckBoxSnap->isChecked();
    snapSize = tmp;

    // end modal dialog:
    this->done(1);
}

void
snapDialogImpl::cancelClicked()
{
    this->done(0); 
}

void
snapDialogImpl::textChanged ( const QString & str)
{
    //printf ("textChanged called\n");
    bool atoi;
    int tmp = LineEditDst->text().toInt(&atoi);
    if ( (atoi) && (tmp<=100) && (tmp>=2))
    {
        SliderDst->setValue(tmp);
    }
}


