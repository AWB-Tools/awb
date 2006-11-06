/*
 *Copyright (C) 2004-2006 Intel Corporation
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
  * @file IconFactory.cpp
  */

#include "IconFactory.h"
#include "icons.cpp"

IconFactory* IconFactory::_myInstance = NULL;

IconFactory*
IconFactory::getInstance()
{
    if (_myInstance==NULL)
        _myInstance = new IconFactory();

    Q_ASSERT(_myInstance!=NULL);
    return _myInstance;
}

void
IconFactory::destroy()
{
    if (_myInstance!=NULL) { delete _myInstance; }
}

IconFactory::IconFactory()
{
    image0 = QPixmap(image0_data);
    image1 = QPixmap(image1_data);
    image2 = QPixmap(image2_data);
    image3 = QPixmap(image3_data);
    image4 = QPixmap(image4_data);
    image5 = QPixmap(image5_data);
    image6 = QPixmap(image6_data);
    image7 = QPixmap(image7_data);
    image8 = QPixmap(image8_data);
    image9 = QPixmap(image9_data);
    image10 = QPixmap(image10_data);
    image11 = QPixmap(image11_data);
    image12 = QPixmap(image12_data);
    image13 = QPixmap(image13_data);
    image14 = QPixmap(image14_data);
    image15 = QPixmap(image15_data);
    image16 = QPixmap(image16_data);
    image17 = QPixmap(image17_data);
    image18 = QPixmap(image18_data);
    image19 = QPixmap(image19_data);
    image20 = QPixmap(image20_data);
    image21 = QPixmap(image21_data);
    image22 = QPixmap(image22_data);
    image23 = QPixmap(image23_data);
    image24 = QPixmap(image24_data);
    image25 = QPixmap(image25_data);
    image26 = QPixmap(image26_data);
    image27 = QPixmap(image27_data);
    image28 = QPixmap(image28_data);
    image29 = QPixmap(image29_data);
    image30 = QPixmap(image30_data);
    image31 = QPixmap(image31_data);
    image32 = QPixmap(image32_data);
    image33 = QPixmap(image33_data);
    image34 = QPixmap(image34_data);
    image35 = QPixmap(image35_data);
    image36 = QPixmap(image36_data);
    image37 = QPixmap(image37_data);
    image38 = QPixmap(image38_data);
    image39 = QPixmap(image39_data);
    image40 = QPixmap(image40_data);
    image41 = QPixmap(image41_data);

    image42 = QPixmap(image42_data);
    image43 = QPixmap(image43_data);
    image44 = QPixmap(image44_data);
    image45 = QPixmap(image45_data);
    image46 = QPixmap(image46_data);
    image47 = QPixmap(image47_data);
}

IconFactory::~IconFactory()
{
}


