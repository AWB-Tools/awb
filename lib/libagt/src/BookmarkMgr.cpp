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
   @file BookmarkMgr.h
  */
#include "BookmarkMgr.h"

#include <qmessagebox.h>
#include <qfiledialog.h>

BookmarkMgr::BookmarkMgr(QWidget* parent,QPopupMenu* pmenu,AScrollView* asv)
{
    myMenu = pmenu;
    myParentWidget = parent;
    myAsv = asv;

    bmd = new bookmarkImpl(&bookmarks,parent,"bmd",true);Q_ASSERT(bmd!=NULL);

    connect( pmenu, SIGNAL( activated( int ) ),
         this, SLOT( bookmChosen( int ) ) );

}

BookmarkMgr::~BookmarkMgr()
{
    if (bmd != NULL) delete bmd;
}

void
BookmarkMgr::setAScrollView (AScrollView* asv)
{ myAsv=asv;}

AScrollView*
BookmarkMgr::getAScrollView ()
{ return myAsv; }

void
BookmarkMgr::addBookmark(double sx,double sy, int x, int y)
{
    // get a description for the bookmark
    bool ok = FALSE;
    QString text = QInputDialog::getText(
    "Bookmark management",
    "Please enter a description for the new bookmark:",
    QLineEdit::Normal,
    QString::null, &ok, myParentWidget );

    if (!ok || text.isEmpty())
    {
        return;
    }
    addBookmark(sx,sy,x,y,text);
}

void
BookmarkMgr::addBookmark(double sx,double sy, int x, int y, QString text)
{
    int cnt = (int)bookmarks.contains(text);
    if (cnt>0)
    {
        QMessageBox::critical (NULL,"Bookmark Error",
        "Repeated descriptor not allowed.",
        QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
        return;
    }

    // create the needed struct
    BookContents nb = new BookContentsClass(x,y,sx,sy,text);
    Q_ASSERT(nb!=NULL);

    mBookmarks[myMenu->insertItem(text)]=nb;
    bookmarks.append(text);
}

void
BookmarkMgr::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
    {
        BookContents nb = mBookmarks[i];
        if (myAsv)
            myAsv->putView(nb->getX(),nb->getY(),nb->getSX(),nb->getSY());
    }

}

void BookmarkMgr::runDialog()
{
    bmd->refresh();
    bmd->exec();
    QStringList strListCpy(bookmarks);
    QMap<int, BookContents> mapCpy(mBookmarks);
    reset();

    QStringList::Iterator it;
    for ( it = strListCpy.begin(); it != strListCpy.end(); ++it )
    {
        BookContents bc = findOnMap(*it,&mapCpy);
        if (bc!=NULL)
        {
            mBookmarks[myMenu->insertItem(*it)]=bc;
            bookmarks.append(*it);
        }
    }
}

BookContents
BookmarkMgr::findOnMap(QString desc,QMap<int, BookContents>* map)
{
    bool fnd=false;
    QMap<int, BookContents>::Iterator it = map->begin();
    while (!fnd && (it != map->end()))
    {
        QString str(it.data()->getDesc());
        fnd = (str==desc);
        ++it;
    }
    if (fnd)
    {
        --it;
        return it.data();
    }
    else
    {
        return NULL;
    }
}

void
BookmarkMgr::reset()
{
    bookmarks.clear();

    QMap<int, BookContents>::Iterator it;
    for ( it = mBookmarks.begin(); it != mBookmarks.end(); ++it )
    {
        int menuid = it.key();
        myMenu->removeItem(menuid);
    }

    mBookmarks.clear();
}

void
BookmarkMgr::importBookmarks()
{
    // open file dialog
    QString fileName = QFileDialog::getOpenFileName(QString::null,"AGT Bookmarks (*.abk)");
    if (fileName == QString::null)
    {
        return;
    }

    QFile* file = new QFile(fileName);
    bool ok = file->open(IO_ReadOnly);
    if (!ok)
    {
        QMessageBox::critical (NULL,"Import Error",
        "IO Error reading "+fileName,
        QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
        return;
    }
    importBookmarks(file);
    file->close();
    delete file;
}

void
BookmarkMgr::importBookmarks(QFile* file)
{

    reset();
    int cnt;
    unsigned mgc;
    {
    QTextStream istream (file);
    istream >> mgc;
    istream >> cnt;
    }

    if ((mgc!=BKM_MAGIC_NUMBER) || (cnt<0) )
    {
        QMessageBox::critical (NULL,"Bookmark Error",
        "Unrecognized format.",
        QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
        return;
    }

    for (int i=0;i<cnt;i++)
    {
        BookContents bc = new BookContentsClass();
        bc->load(file);
        mBookmarks[myMenu->insertItem(bc->getDesc())]=bc;
        bookmarks.append(bc->getDesc());
    }
}

void
BookmarkMgr::exportBookmarks()
{
    // save file dialog
    QString fileName = QFileDialog::getSaveFileName(QString::null,"AGT Bookmarks (*.abk)");
    if (fileName == QString::null)
    {
        return;
    }

    QFile* file = new QFile(fileName);
    bool ok = file->open(IO_WriteOnly);
    if (!ok)
    {
        QMessageBox::critical (NULL,"Export Error",
        "IO Error writing "+fileName,
        QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
        return;
    }
    exportBookmarks(file);
    file->close();
    delete file;
}

void
BookmarkMgr::exportBookmarks(QFile* file)
{
    int cnt = bookmarks.count();
    /*
    if (cnt<1)
    {
        QMessageBox::critical (NULL,"Export Error",
        "No bookmarks to export!",
        QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
        return;
    }
    */
    unsigned mgc = BKM_MAGIC_NUMBER;
    {
    QTextStream ostream (file);
    ostream << mgc << "\n";
    ostream << cnt << "\n";
    }

    QStringList::Iterator it;
    for ( it = bookmarks.begin(); it != bookmarks.end(); ++it )
    {
        BookContents bc = findOnMap(*it,&mBookmarks);
        Q_ASSERT(bc!=NULL);
        bc->save(file);
    }
}


