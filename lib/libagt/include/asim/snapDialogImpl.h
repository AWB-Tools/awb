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
  * @file   snapDialogImpl.h
  * @brief  QT dialog for configuring snap-to-grid functionality.
  */
  
#ifndef SNAPDIALOGIMPL_H
#define SNAPDIALOGIMPL_H

#include <qvariant.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>

#include "agt_syntax.h"
#include "snapDialog.h"


/**
  * This is a QT dialog for configuring snap-to-grid functionality.
  *
  * Basically you can switch on and off the "snap to grid"
  * behaviour of annotation tools and change the gap 
  * between grid lines.
  *
  * @version 0.1
  * @date started at 2002-04-01
  * @author Federico Ardanaz
  */
class snapDialogImpl : public Form1
{
    Q_OBJECT

public:
    /**
      * Constructor
      */
    snapDialogImpl (
                   QWidget* parent = 0,      ///< Widget parent
                   const char* name = 0,     ///< Internal name
                   bool modal = FALSE,       ///< modal dialog? (stay on top and block other windows)
                   WFlags fl = 0             ///< QT Widget flags (stay on top, etc.)
                   );

    /**
      * Destructor
      */
    ~snapDialogImpl();

    /**
      * Query whether snap to grip is enabled or not.
      */
    bool         /// @return TRUE is snap to grid is enabled.
    snap();

    /**
      * Switch on and off snap to grid. 
      */
    void
    setSnap (
            bool v        ///< TRUE is on, FALSE if off
            );

    /**
      * Get back the "gap" between grid lines.
      */
    int              /// @return the "gap" between grid lines.
    getSnapSize();

    /**
      * Set the "gap" between grid lines.
      */
    void
    setSnapSize (
                int size      ///< the "gap" between grid lines (in pixels).
                );

    /**
      * Inherited method redefinition related with QT widget size management.
      * @see http://doc.trolltech.com/3.0/qwidget.html
      */
    virtual QSize
    sizeHint () const;



public slots:
    /**
      * This method is invoked when the Size slider is moved.
      */
    void
    Slider1_sliderMoved (
                        int    ///< New slider value
                        );

    /**
      * This method is invoked when the accept button is clicked.
      */
    void
    acceptClicked();

    /**
      * This method is invoked when cancel button is clicked
      */
    void
    cancelClicked();

    /**
      * This method is invoked when user change the text edit field (for size)
      */
    void
    textChanged (
                const QString &     ///< Current text value
                );

private:
    bool doSnap;
    int  snapSize;

};

#endif // SNAPDIALOGIMPL_H

