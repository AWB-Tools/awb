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
  * @file bookmarkImpl.cpp
  */


#include <string.h>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "bookmarkImpl.h"

BookContentsClass::BookContentsClass(int _x, int _y, double _sx, double _sy, QString _desc)
{
    x = _x;
    y = _y;
    sx = _sx;
    sy = _sy;
    desc = _desc;
}

BookContentsClass::~BookContentsClass()
{
}

void
BookContentsClass::save(QFile* file)
{
    QTextStream ostream(file);
    ostream << repSpaces(desc) << "\n" << x << "\n" << y << "\n" << sx << "\n" << sy << "\n";
}

void
BookContentsClass::load(QFile* file)
{
    QTextStream istream(file);
    istream >> desc >> x >> y >> sx >> sy;
    desc.replace(QRegExp("_")," ");
}

QString
BookContentsClass::repSpaces(QString str)
{
    str.replace(QRegExp("\\s"),"_");
    return (str);
}

/*
 *  Constructs a bookmarkImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
bookmarkImpl::bookmarkImpl(QStringList *mBookmarks, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : BookMarkForm( parent, name, modal, fl )
{
    booklist=mBookmarks;
    // connect all the buttons
    connect ( UpButton, SIGNAL(clicked()), this, SLOT(UpButton_clicked()) );
    connect ( DownButton, SIGNAL(clicked()), this, SLOT(DownButton_clicked()) );
    connect ( RemoveButton, SIGNAL(clicked()), this, SLOT(RemoveButton_clicked()) );
    connect ( CloseButton, SIGNAL(clicked()), this, SLOT(CloseButton_clicked()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
bookmarkImpl::~bookmarkImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

void
bookmarkImpl::refresh()
{
    BookMarkList->clear();

    QStringList::Iterator it;
    for ( it = booklist->begin(); it != booklist->end(); ++it )
    {
        BookMarkList->insertItem(*it);
    }
}

void
bookmarkImpl::UpButton_clicked()
{
    int pos = BookMarkList->currentItem();
    if (pos<0)
    {
        qApp->beep();
        return;
    }

    QListBoxItem* lastSelected = BookMarkList->item(pos);
    if (lastSelected!=NULL)
    {
        if (pos>0)
        {
            QString prevStr = (BookMarkList->item(pos-1))->text();
            BookMarkList->changeItem(lastSelected->text(),pos-1);
            BookMarkList->changeItem(prevStr,pos);
            BookMarkList->setSelected(pos-1,true);
            BookMarkList->setCurrentItem(pos-1);
            BookMarkList->ensureCurrentVisible();
        }
    }
    else
    {
        qApp->beep();
    }
}

void
bookmarkImpl::DownButton_clicked()
{
    int pos = BookMarkList->currentItem();
    if (pos<0)
    {
        qApp->beep();
        return;
    }

    QListBoxItem* lastSelected = BookMarkList->item(pos);
    if (lastSelected!=NULL)
    {
        int pos = BookMarkList->currentItem();
        if (pos< ((int)BookMarkList->count() - 1))
        {
            QString prevStr = (BookMarkList->item(pos+1))->text();
            BookMarkList->changeItem(lastSelected->text(),pos+1);
            BookMarkList->changeItem(prevStr,pos);
            BookMarkList->setSelected(pos+1,true);
            BookMarkList->setCurrentItem(pos+1);
            BookMarkList->ensureCurrentVisible();
        }
    }
    else
    {
        qApp->beep();
    }
}

void
bookmarkImpl::RemoveButton_clicked()
{
    int ci = BookMarkList->currentItem();
    if (ci>=0)
    {
        BookMarkList->removeItem(ci);
    }
}

void
bookmarkImpl::CloseButton_clicked()
{
    // change the string list according to the elements on BookMarkList
    QListBoxItem *current = BookMarkList->firstItem();
    booklist->clear();
    while (current!=NULL)
    {
        booklist->append(current->text());
        current = current->next();
    }    
    done(1);
}


