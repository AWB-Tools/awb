/**************************************************************************
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
 * @file dralInterface.cpp
 * @author Julio Gago
 * @brief dral low-level-buy-pretty interface
 *
 * Please refer to header file for comments and usage.
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
using namespace std;

#include "asim/dralInterface.h"

UINT32 DRAL_ITEM::current_id = 1;
UINT32 DRAL_NODE::current_id = 1;
UINT32 DRAL_EDGE::current_id = 1;

/*
 * A default DRAL server. Used when the user does not explicitly create and
 * provide one or more (most of the times).
 */
DRAL_SERVER_CLASS defaultDralServer("events.drl", 4096, false, true, false);

/*
 * Several shortcuts to access the default DRAL server
 *
 */
void DralStartActivity()
{
    defaultDralServer.StartActivity();
}

void DralTurnOn(void)
{
    defaultDralServer.TurnOn();
}

void DralCycle(UINT64 n)
{
    defaultDralServer.Cycle(n);
}

void DralTurnOff(void)
{
    defaultDralServer.TurnOff();
}

/*
 * Syntax sugar to create name/value TAG pairs using a single and compact
 * function name, but without the use of a DRAL_TAG objects, which is in any
 * case recommended :-).
 */
DRAL_uTAG dTag(const char tag_name[], UINT64 value)       { return *(new DRAL_uTAG(tag_name, value)); }
DRAL_uTAG dTag(const char tag_name[], INT64 value)        { return *(new DRAL_uTAG(tag_name, (UINT64) value)); } 
DRAL_uTAG dTag(const char tag_name[], UINT32 value)       { return *(new DRAL_uTAG(tag_name, (UINT64) value)); }
DRAL_uTAG dTag(const char tag_name[], INT32 value)        { return *(new DRAL_uTAG(tag_name, (UINT64) value)); }
DRAL_cTAG dTag(const char tag_name[], char value)         { return *(new DRAL_cTAG(tag_name, value)); }
DRAL_sTAG dTag(const char tag_name[], const char value[]) { return *(new DRAL_sTAG(tag_name, value)); }

