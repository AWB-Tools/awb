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
 * @file dralClientBinary_v0_1.h
 * @author Pau Cabre 
 * @brief dral client binary defines for dral versions 0 and 1
 */

#ifndef DRAL_CLIENT_BINARY_V0_1_H
#define DRAL_CLIENT_BINARY_V0_1_H

#include "asim/dralClientImplementation.h"

#define DRAL0_CYCLE       0
#define DRAL0_NEWITEM     1
#define DRAL0_MOVEITEMS   2
#define DRAL0_DELETEITEM  3
#define DRAL0_SETTAG      5
#define DRAL0_ENTERNODE   6
#define DRAL0_EXITNODE    7
#define DRAL0_ADDNODE     8
#define DRAL0_ADDEDGE     9
#define DRAL0_SETCAPACITY 10
#define DRAL0_SETHIGHWATERMARK 11
#define DRAL0_COMMENT     12
//13 is DRAL_VERSION. Already defined.
#define DRAL0_SETTAGNODE  14
#define DRAL0_SINGLEVALUE 0
#define DRAL0_STRING      1
#define DRAL0_SET         2

/**
 * BINARY client version 1 implementation class
 */
class DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS
    : public DRAL_CLIENT_IMPLEMENTATION_CLASS
{
  public:

    DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS (
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
    virtual bool SetTag (void * b);
    virtual bool MoveItems (void * b);
    virtual bool AddNode (void * b);
    virtual bool AddEdge (void * b);
    bool SetCapacity (void * b);
    bool SetHighWaterMark (void * b);
    virtual bool Comment (void * b);
    bool Error (void * b);

    bool firstCycle;
};

/**
 * BINARY client version 1 implementation class
 * Note: in fact, this class is a modification of the binary version 1
 * implementation that allow to read files generated with the version 0 server.
 * So, it is a derived class of the binary version 1, and only redefine the
 * methods that need to be changed.
 */
class DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS
    : public DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS
{
  public:

    DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS (
        DRAL_BUFFERED_READ dral_read, DRAL_LISTENER dralListener);
    
  private:

    /**
     * Private methods to proccess theses specific events
     */

    bool SetTag (void * b);
    bool MoveItems (void * b);
    bool AddNode (void * b);
    bool AddEdge (void * b);
    bool Comment (void * b);
};

#endif /* DRAL_CLIENT_BINARY_V0_1_H */
