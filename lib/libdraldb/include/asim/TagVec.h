// ==================================================
// Copyright (C) 2003-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file TagVec.h
  */

#ifndef _DRALDB_TAGVEC_H
#define _DRALDB_TAGVEC_H

#include <stdio.h>

#include <qstring.h>

#include "asim/draldb_syntax.h"
#include "asim/DRALTag.h"
#include "asim/ZipObject.h"

/**
  * @brief
  * Abstract class to hide encoding formats.
  *
  * @description
  * This class is an interface of different vector implementations.
  * Each implementation is used to know the value of a tag in a
  * precise moment. Each version differs from each other by the way
  * this matching is done : only matches if the tag was set in the
  * same cycle that is requested, if was set before, ...
  *
  * @version 0.1
  * @date started at 2003-05-27
  * @author Federico Ardanaz
  */

typedef enum
{
    TVEType_ITEMIDX,                // for moveitems, enter/exitnodes
    TVEType_DENSE_ITEMIDX,
    TVEType_DENSE_SHORT_ITEMIDX,
    TVEType_DICTIONARY,             // for node tags
    TVEType_DENSE_DICTIONARY,
    TVEType_DICTIONARY_NF,          // for cycle tags
    TVEType_DENSE_DICTIONARY_NF
} TagVecEncodingType;

class TagVec : public ZipObject
{
    public:
        inline TagVec(){ we=true; pendingCnt=0; }

        virtual bool getTagValue(INT32 cycle, UINT64*  value, UINT32* atcycle) = 0;
        virtual bool getTagValue(INT32 cycle, SOVList** value, UINT32* atcycle) = 0;

        virtual bool addTagValue(INT32 cycle, UINT64   value) = 0;
        virtual bool addTagValue(INT32 cycle, QString  value) = 0;
        virtual bool addTagValue(INT32 cycle, SOVList* value) = 0;

        virtual void dumpCycleVector() = 0;
        virtual TagVecEncodingType getType() = 0;

        inline virtual bool isWriteEnabled();
        inline virtual void setWriteEnabled(bool);
        inline virtual void incPendingCnt();
        inline virtual void decPendingCnt();

   protected:
        bool  we;
        INT32 pendingCnt;
};

/**
 * Returns the value of the we flag.
 *
 * @return we flag.
 */
bool
TagVec::isWriteEnabled()
{
    return we;
}

/**
 * Sets the we flag.
 *
 * @return void.
 */
void
TagVec::setWriteEnabled(bool value)
{
    we = value;
}

/**
 * Increments pending counter.
 *
 * @return void.
 */
void
TagVec::incPendingCnt()
{
    ++pendingCnt;
    we=true;
    //printf("TagVec::incPendingCnt called => %d\n",pendingCnt);
}

/**
 * Decrements the pending counter.
 *
 * @return void.
 */
void
TagVec::decPendingCnt()
{
    --pendingCnt;
    //printf("TagVec::decPendingCnt called => %d\n",pendingCnt);
    if (pendingCnt==0)
    {
        we = false;
    }
}

#endif
