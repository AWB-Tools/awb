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
  * @file  HelpWindow.cpp
  */

#include "HelpWindow.h"

#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <ctype.h>

// icons
#include "xpm/help_forward.xpm"
#include "xpm/help_back.xpm"
#include "xpm/help_home.xpm"

HelpWindow::HelpWindow( const QString& home_, const QStringList& _path,QWidget* parent, const char *name ) :
QMainWindow( parent, name, WDestructiveClose ),pathCombo( 0 ), selectedURL()
{
    myParent=parent;
    readHistory();
    readBookmarks();

    browser = new QTextBrowser( this );Q_ASSERT(browser!=NULL);
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    browser->mimeSourceFactory()->setFilePath( _path );
    //browser->mimeSourceFactory()->setExtensionType("html", "text/html");

    connect( browser, SIGNAL( textChanged() ),this,SLOT( textChanged() ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
    {
        browser->setSource( home_ );
    }

    connect( browser, SIGNAL( highlighted( const QString&) ),
         statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );Q_ASSERT(file!=NULL);
    file->insertItem( tr("&New Window"), this, SLOT( newWindow() ), CTRL+Key_N );
    file->insertItem( tr("&Print"), this, SLOT( print() ), CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( tr("&Close"), this, SLOT( close() ), CTRL+Key_Q );

    // The same three icons are used twice each.

    QPixmap icon_back = QPixmap(help_back) ;
    QPixmap icon_forward = QPixmap(help_forward);
    QPixmap icon_home = QPixmap(help_home);

    QPopupMenu* go = new QPopupMenu( this );Q_ASSERT(go!=NULL);
    backwardId = go->insertItem( icon_back,
                 tr("&Backward"), browser, SLOT( backward() ),
                 CTRL+Key_Left );
    forwardId = go->insertItem( icon_forward,
                tr("&Forward"), browser, SLOT( forward() ),
                CTRL+Key_Right );
    go->insertItem( icon_home, tr("&Home"), browser, SLOT( home() ) );

    hist = new QPopupMenu( this );Q_ASSERT(hist!=NULL);
    QStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
    {
        mHistory[ hist->insertItem( *it ) ] = *it;
    }

    connect( hist, SIGNAL( activated( int ) ),
         this, SLOT( histChosen( int ) ) );

    bookm = new QPopupMenu( this );Q_ASSERT(bookm!=NULL);
    bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertSeparator();

    QStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
    {
        mBookmarks[ bookm->insertItem( *it2 ) ] = *it2;
    }

    connect( bookm, SIGNAL( activated( int ) ),
         this, SLOT( bookmChosen( int ) ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );
    menuBar()->insertItem( tr( "History" ), hist );
    menuBar()->insertItem( tr( "Bookmarks" ), bookm );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);

    connect( browser, SIGNAL( backwardAvailable(bool)),this, SLOT(setBackwardAvailable(bool)));
    connect( browser, SIGNAL( forwardAvailable(bool)),this, SLOT(setForwardAvailable(bool)));

    QToolBar* toolbar = new QToolBar( this );Q_ASSERT(toolbar!=NULL);
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( icon_back, tr("Backward"), "", browser, SLOT(backward()), toolbar );
    Q_ASSERT(button!=NULL);
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_forward, tr("Forward"), "", browser, SLOT(forward()), toolbar );
    Q_ASSERT(button!=NULL);
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_home, tr("Home"), "", browser, SLOT(home()), toolbar );
    Q_ASSERT(button!=NULL);

    toolbar->addSeparator();

    pathCombo = new QComboBox( TRUE, toolbar );Q_ASSERT(pathCombo!=NULL);
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
         this, SLOT( pathSelected( const QString & ) ) );

    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( TRUE );
    setDockEnabled( DockLeft, FALSE );
    setDockEnabled( DockRight, FALSE );

    pathCombo->insertItem( home_ );
    browser->setFocus();

}


void
HelpWindow::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void
HelpWindow::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void
HelpWindow::textChanged()
{
    if ( browser->documentTitle().isNull() )
    {
        setCaption( "Helpviewer - " + browser->context() );
    }
    else
    {
        setCaption( "Helpviewer - " + browser->documentTitle() ) ;
    }

    selectedURL = browser->context();

    if ( !selectedURL.isEmpty() && pathCombo )
    {
        bool exists = FALSE;
        int i;
        for ( i = 0; i < pathCombo->count(); ++i )
        {
            if ( pathCombo->text( i ) == selectedURL )
            {
                exists = TRUE;
                break;
            }
        }
        if ( !exists )
        {
            pathCombo->insertItem( selectedURL, 0 );
            pathCombo->setCurrentItem( 0 );
            mHistory[ hist->insertItem( selectedURL ) ] = selectedURL;
        }
        else
        {
            pathCombo->setCurrentItem( i );
        }
        selectedURL = QString::null;
    }
}

HelpWindow::~HelpWindow()
{
    history.clear();
    QMap<int, QString>::Iterator it = mHistory.begin();

    for ( ; it != mHistory.end(); ++it )
    {
        history.append( *it );
    }

    QFile f( QDir::currentDirPath() + "/.history" );
    f.open( IO_WriteOnly );
    QDataStream s( &f );
    s << history;
    f.close();

    bookmarks.clear();
    QMap<int, QString>::Iterator it2 = mBookmarks.begin();

    for ( ; it2 != mBookmarks.end(); ++it2 )
    {
        bookmarks.append( *it2 );
    }

    QFile f2( QDir::currentDirPath() + "/.bookmarks" );
    f2.open( IO_WriteOnly );
    QDataStream s2( &f2 );
    s2 << bookmarks;
    f2.close();
}

void
HelpWindow::newWindow()
{
    ( new HelpWindow (browser->source(),browser->mimeSourceFactory()->filePath(),
          myParent,"hbrowser") )->show();
}

void
HelpWindow::print()
{
    QPrinter printer;//(QPrinter::HighResolution );
    printer.setFullPage(TRUE);
    if ( printer.setup( this ) )
    {
        QPainter p( &printer );
        QPaintDeviceMetrics metrics(p.device());
        int dpix = metrics.logicalDpiX();
        int dpiy = metrics.logicalDpiY();
        const int margin = 72; // pt

        QRect body(margin*dpix/72, margin*dpiy/72,
               metrics.width()-margin*dpix/72*2,
               metrics.height()-margin*dpiy/72*2 );

        QFont font("times");
        QStringList filePaths = browser->mimeSourceFactory()->filePath();
        QString file;
        QStringList::Iterator it = filePaths.begin();

        for ( ; it != filePaths.end(); ++it )
        {
            file = QUrl( *it, QUrl( browser->source() ).path() ).path();
            if ( QFile::exists( file ) )
            {
                break;
            }
            else
            {
                file = QString::null;
            }
        }

        if ( file.isEmpty() )
        {
            return;
        }

        QFile f( file );
        if ( !f.open( IO_ReadOnly ) )
        {
            return;
        }

        QTextStream ts( &f );
        QSimpleRichText richText( ts.read(), font, browser->context(), browser->styleSheet(),
                      browser->mimeSourceFactory(), body.height() );

        richText.setWidth( &p, body.width() );
        QRect view( body );
        int page = 1;
        do
        {
            richText.draw( &p, body.left(), body.top(), view, colorGroup() );
            view.moveBy( 0, body.height() );
            p.translate( 0 , -body.height() );
            p.setFont( font );

            p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
                view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );

            if ( view.top()  >= richText.height() )
            {
                break;
            }

            printer.newPage();
            page++;
        }
        while (TRUE);
    }
}

void
HelpWindow::pathSelected( const QString &_path )
{
    browser->setSource( _path );
    QMap<int, QString>::Iterator it = mHistory.begin();
    bool exists = FALSE;

    for ( ; it != mHistory.end(); ++it )
    {
        if ( *it == _path )
        {
            exists = TRUE;
            break;
        }
    }

    if ( !exists )
    {
        mHistory[ hist->insertItem( _path ) ] = _path;
    }
}

void
HelpWindow::readHistory()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.history" ) )
    {
        QFile f( QDir::currentDirPath() + "/.history" );
        f.open( IO_ReadOnly );
        QDataStream s( &f );
        s >> history;
        f.close();
        while ( history.count() > 20 )
        {
            history.remove( history.begin() );
        }
    }
}

void
HelpWindow::readBookmarks()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.bookmarks" ) )
    {
        QFile f( QDir::currentDirPath() + "/.bookmarks" );
        f.open( IO_ReadOnly );
        QDataStream s( &f );
        s >> bookmarks;
        f.close();
    }
}

void
HelpWindow::histChosen( int i )
{
    if ( mHistory.contains( i ) )
    {
        browser->setSource( mHistory[ i ] );
    }
}

void
HelpWindow::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
    {
        browser->setSource( mBookmarks[ i ] );
    }
}

void
HelpWindow::addBookmark()
{
    mBookmarks[ bookm->insertItem( caption() ) ] = browser->context();
}


