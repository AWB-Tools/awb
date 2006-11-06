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
  * @file ColumnShadows.cpp
  * @brief
  */

#include "ColumnShadows.h"

ColumnShadows::ColumnShadows (int numColumns)
{
    // create shading flag vectors
    int fsz = (numColumns+1)/sizeof(BYTE);
    cols = numColumns;
    shadeFlagVector = new BYTE[fsz];
    Q_ASSERT(shadeFlagVector!=NULL);
    bzero((void*)shadeFlagVector,(size_t)fsz);
}

ColumnShadows::~ColumnShadows()
{
    delete [] shadeFlagVector;
}

void
ColumnShadows::resize(int newsize)
{
    int i;
    BYTE* tmpVector;
    int fsz = (newsize+1)/sizeof(BYTE);
    tmpVector = new BYTE[fsz];
    Q_ASSERT(tmpVector!=NULL);
    bzero((void*)tmpVector,(size_t)fsz);
    int cmin = QMIN(cols,newsize);
    for (i=0;i<cmin;i++)
    {
        if (getShadeFlag(i))
            tmpVector[i/8] |= (1<<(7 - i%8));
    }
    delete [] shadeFlagVector;
    shadeFlagVector = tmpVector;
    cols = newsize;
}

void
ColumnShadows::shadeColumn(INT32 col,bool shade)
{
    if (col >= cols) { return; }

    int v = (int)shade;
    BYTE mask = ~(1<<(7 - col%8));
    shadeFlagVector[col/8] &= mask;
    shadeFlagVector[col/8] |= (v<<(7 - col%8));
}

bool
ColumnShadows::getShadeFlag(INT32 pos)
{ return (shadeFlagVector[pos/8]>>(7 - pos%8) & 1 ); }

void
ColumnShadows::clear()
{
    int fsz = cols/sizeof(BYTE);
    bzero((void*)shadeFlagVector,(size_t)fsz);
}

INT32
ColumnShadows::countShadowedColumns()
{
    INT32 i;
    INT32 acc = 0;
    for (i=0;i<cols;i++)
        if (getShadeFlag(i)) acc++;

    return acc;
}

void
ColumnShadows::saveShadows(QFile* file)
{
    int i;
    int cnt = countShadowedColumns();
    QTextStream ostream (file);

    unsigned mgc = SHD_MAGIC_NUMBER;
    ostream << mgc << "\n";
    ostream << cnt << "\n";

    for (i=0;i<cols;i++)
    {
        if (getShadeFlag(i))
        {
            ostream << i << "\n";
        }
    }
}

void
ColumnShadows::loadShadows(QFile* file)
{
    int i,cnt;
    unsigned mgc;
    {
        QTextStream istream (file);
        istream >> mgc;
        istream >> cnt;
    }

    if ((mgc!=SHD_MAGIC_NUMBER) || (cnt<0) )
    {
        QMessageBox::critical (NULL,"Shadows Error",
        "Unrecognized format.",
        QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
        return;
    }

    QTextStream istream (file);
    int pos;
    for (int i=0;i<cnt;i++)
    {
        istream >> pos;
        shadeColumn(pos,true);
    }
}


