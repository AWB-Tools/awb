// ==================================================
//Copyright (C) 2004-2006 Intel Corporation
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
  * @file  IconFactory.h
  */

#ifndef _ICONFACTORY_H
#define _ICONFACTORY_H

// QT library
#include <qstring.h>
#include <qiconset.h>
#include <qiconview.h>
#include <qpixmap.h>

/**
  * This class holds DRAL Graph and configuration information.
  *
  * @author Federico Ardanaz
  * @date started at 2004-01-14
  * @version 0.1
  */
class IconFactory 
{
    public:
        static IconFactory* getInstance ();
        static void destroy();

    protected:
        IconFactory();
        virtual ~IconFactory();

    public:
        QPixmap image0;
        QPixmap image1;
        QPixmap image2;
        QPixmap image3;
        QPixmap image4;
        QPixmap image5;
        QPixmap image6;
        QPixmap image7;
        QPixmap image8;
        QPixmap image9;
        QPixmap image10;
        QPixmap image11;
        QPixmap image12;
        QPixmap image13;
        QPixmap image14;
        QPixmap image15;
        QPixmap image16;
        QPixmap image17;
        QPixmap image18;
        QPixmap image19;
        QPixmap image20;
        QPixmap image21;
        QPixmap image22;
        QPixmap image23;
        QPixmap image24;
        QPixmap image25;
        QPixmap image26;
        QPixmap image27;
        QPixmap image28;
        QPixmap image29;
        QPixmap image30;
        QPixmap image31;
        QPixmap image32;
        QPixmap image33;
        QPixmap image34;
        QPixmap image35;
        QPixmap image36;
        QPixmap image37;
        QPixmap image38;
        QPixmap image39;
        QPixmap image40;
        QPixmap image41;

        QPixmap image42;
        QPixmap image43;
        QPixmap image44;
        QPixmap image45;
        QPixmap image46;
        QPixmap image47;

    private:
       static IconFactory* _myInstance;
};



#endif

