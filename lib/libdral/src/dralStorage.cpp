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

#include "asim/dralStorage.h"

void DRAL_STORAGE_CLASS::ChangeImplementation(DRAL_SERVER_IMPLEMENTATION impl)
{
    implementation=impl;
}


DRAL_STORAGE_CLASS::DRAL_STORAGE_CLASS(DRAL_SERVER_IMPLEMENTATION impl)
{
    implementation=impl;
}

DRAL_STORAGE_CLASS::~DRAL_STORAGE_CLASS(void)
{
    for(
        list<DRAL_COMMAND_STORAGE>::const_iterator i=allCommandsList.begin();
        i != allCommandsList.end();
        ++i)
    {
        delete (*i);
    }
    allCommandsList.clear();
    numInstanceList.clear();
    partialCommandList.clear();
}

void DRAL_STORAGE_CLASS::ResetPartialList(void)
{
    partialCommandList.clear();
}

void DRAL_STORAGE_CLASS::Store(DRAL_COMMAND_STORAGE command, bool bothLists)
{
    allCommandsList.push_back(command);
    if (bothLists)
    {
        partialCommandList.push_back(command);
    }
}

void DRAL_STORAGE_CLASS::Store(
    DRAL_NEWNODE_STORAGE newNodeCommand,bool bothLists)
{
    Store(dynamic_cast<DRAL_COMMAND_STORAGE>(newNodeCommand),bothLists);
    numInstanceList.push_front(newNodeCommand);
}

void DRAL_STORAGE_CLASS::DumpPartialList(void)
{
    for(
        list<DRAL_COMMAND_STORAGE>::const_iterator i=partialCommandList.begin();
        i != partialCommandList.end();
        ++i)
    {
        (*i)->Notify(implementation);
    }
}

void DRAL_STORAGE_CLASS::DumpAllCommandsList(void)
{
    for(
        list<DRAL_COMMAND_STORAGE>::const_iterator i=allCommandsList.begin();
        i != allCommandsList.end();
        ++i)
    {
        (*i)->Notify(implementation);
    }
}

UINT16
DRAL_STORAGE_CLASS::GetInstance (const char node_name [])
{
    for(
        list<DRAL_NEWNODE_STORAGE>::const_iterator i=numInstanceList.begin();
        i != numInstanceList.end();
        ++i)
    {
        if (!strcmp((*i)->GetNodeName(), node_name))
        {
            return ((*i)->GetInstance()+1);
        }
    }
    return 0;
}
