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
  * @file  ColorMenuItem.cpp
  */

#include "ColorMenuItem.h"

ColorMenuItem::ColorMenuItem(QColor c): QCustomMenuItem()
{
    this->myColor = c; 
}

void
ColorMenuItem::paint ( QPainter * p, const QColorGroup & cg, bool act, bool enabled, int x, int y, int w, int h )
{
    //p->fillRect(x+1,y+1,w-8,h-3,myColor);
    p->setPen(QPen(QColor(0,0,0),1));
    p->setBrush(QBrush(myColor));
    p->drawRect(x+1,y+1,w-8,h-3);
}

QSize
ColorMenuItem::sizeHint ()
{
    return QSize(25,12); 
}


