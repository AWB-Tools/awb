/**************************************************************************
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
 * @file dralClientBinary_v2.h
 * @author Pau Cabre 
 * @brief dral client binary defines for dral version 2
 */

#ifndef DRAL_CLIENT_BINARY_V2_H
#define DRAL_CLIENT_BINARY_V2_H

#include "asim/dralClientImplementation.h"
#include "asim/dralClientDefines.h"     /* We need to include this file because
                                        there is the definition of the
                                        DRAL_VERSION command code */


#define DRAL2_CYCLE                      0
#define DRAL2_NEWITEM                    1
#define DRAL2_MOVEITEMS                  2
#define DRAL2_DELETEITEM                 3
#define DRAL2_SETITEMTAG                 4
#define DRAL2_SETITEMTAG_STRING          5
#define DRAL2_SETITEMTAG_SET             6
#define DRAL2_ENTERNODE                  7
#define DRAL2_EXITNODE                   8
#define DRAL2_NEWNODE                    9
#define DRAL2_NEWEDGE                    10
#define DRAL2_SETNODELAYOUT              11
#define DRAL2_COMMENT                    12
//13 used for DRAL_VERSION. Already defined
#define DRAL2_SETNODETAG                 14
#define DRAL2_SETNODETAG_STRING          15
#define DRAL2_SETNODETAG_SET             16
#define DRAL2_SETCYCLETAG                17
#define DRAL2_SETCYCLETAG_STRING         18
#define DRAL2_SETCYCLETAG_SET            19
#define DRAL2_SETNODEINPUTBANDWIDTH      20
#define DRAL2_SETNODEOUTPUTBANDWIDTH     21
#define DRAL2_STARTACTIVITY              22
#define DRAL2_SETTAGDESCRIPTION          23
#define DRAL2_COMMENTBIN                 24
#define DRAL2_SETNODECLOCK               25
#define DRAL2_NEWCLOCK                   26
#define DRAL2_CYCLEWITHCLOCK             27

/**
 * BINARY client version 2 implementation class
 */
class DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS
    : public DRAL_CLIENT_IMPLEMENTATION_CLASS
{
  public:

    DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS (
        DRAL_BUFFERED_READ dral_read, DRAL_LISTENER dralListener);
    
    UINT16 ProcessNextEvent (bool blocking, UINT16 num_events);

  protected:

    void * ReadBytes (UINT32 num_bytes);

    /**
     * Private methods to proccess theses specific events
     */
    bool Cycle (void * b);
    bool NewItem (void * b);
    bool DeleteItem (void * b);
    bool EnterNode (void * b);
    bool ExitNode (void * b);
    bool SetItemTag (void * b, UINT16 v);
    bool MoveItems (void * b);
    bool NewNode (void * b);
    bool NewEdge (void * b);
    bool SetNodeLayout (void * b);
    bool Comment (void * b);
    bool CommentBin (void * b);
    bool Error (void * b);
    bool SetNodeTag (void * b, UINT16 v);
    bool SetCycleTag (void * b, UINT16 v);
    bool SetNodeInputBandwidth (void * b);
    bool SetNodeOutputBandwidth (void * b);
    bool StartActivity (void * b);
    bool SetTagDescription (void * b);
    bool SetNodeClock (void * b);
    bool NewClock (void * b);
    bool CycleWithClock (void * b);

};


#endif /* DRAL_CLIENT_BINARY_V2_H */
