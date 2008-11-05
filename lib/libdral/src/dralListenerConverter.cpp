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
 * @file dralListenerConverter.cpp
 * @author Guillem Sole
 * @brief Converts the new indexed call-backs to the old non-indexed ones.
 */
 
#include "asim/dralListenerConverter.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

DRAL_LISTENER_CONVERTER_CLASS::DRAL_LISTENER_CONVERTER_CLASS(DRAL_LISTENER_OLD listener)
{
    dralListener = listener;
}

DRAL_LISTENER_CONVERTER_CLASS::~DRAL_LISTENER_CONVERTER_CLASS()
{
    // Frees the tags.
    for(UINT32 i = 0; i < tags.size(); i++)
    {
        free((void *) tags[i]);
    }

    // Frees the strings.
    for(UINT32 i = 0; i < strings.size(); i++)
    {
        free((void *) strings[i]);
    }
}

void
DRAL_LISTENER_CONVERTER_CLASS::Cycle(UINT64 cycle)
{
    dralListener->Cycle(cycle);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NewItem(UINT32 item_id)
{
    dralListener->NewItem(item_id);
}

void
DRAL_LISTENER_CONVERTER_CLASS::MoveItems(UINT16 edge_id, UINT32 numOfItems, UINT32 * items)
{
    dralListener->MoveItems(edge_id, numOfItems, items);
}

void
DRAL_LISTENER_CONVERTER_CLASS::MoveItemsWithPositions(UINT16 edge_id, UINT32 numOfItems, UINT32 * items, UINT32 * positions)
{
    dralListener->MoveItemsWithPositions(edge_id, numOfItems, items, positions);
}

void
DRAL_LISTENER_CONVERTER_CLASS::DeleteItem(UINT32 item_id)
{
    dralListener->DeleteItem(item_id);
}

void
DRAL_LISTENER_CONVERTER_CLASS::EndSimulation(void)
{
    dralListener->EndSimulation();
}

void
DRAL_LISTENER_CONVERTER_CLASS::Error(char * error)
{
    dralListener->Error(error);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NonCriticalError(char * error)
{
    dralListener->NonCriticalError(error);
}

void
DRAL_LISTENER_CONVERTER_CLASS::Version(UINT16 version)
{
    dralListener->Version(version);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NewNode(UINT16 node_id, char * node_name, UINT16 parent_id, UINT16 instance)
{
    dralListener->NewNode(node_id, node_name, parent_id, instance);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NewEdge(UINT16 sourceNode, UINT16 destNode, UINT16 edge_id, UINT32 bandwidth, UINT32 latency, char * name)
{
    dralListener->NewEdge(sourceNode, destNode, edge_id, bandwidth, latency, name);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeLayout(UINT16 node_id, UINT32 capacity, UINT16 dim, UINT32 capacities [])
{
    dralListener->SetNodeLayout(node_id, capacity, dim, capacities);
}

void
DRAL_LISTENER_CONVERTER_CLASS::EnterNode(UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position [])
{
    dralListener->EnterNode(node_id, item_id, dim, position);
}

void
DRAL_LISTENER_CONVERTER_CLASS::ExitNode(UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position [])
{
    dralListener->ExitNode(node_id, item_id, dim, position);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetCycleTag(UINT32 tag_idx, UINT64 value)
{
    dralListener->SetCycleTag(tags[tag_idx], value);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetCycleTagString(UINT32 tag_idx, UINT32 str_idx)
{
    dralListener->SetCycleTagString(tags[tag_idx], strings[str_idx]);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetCycleTagSet(UINT32 tag_idx, UINT32 nval, UINT64 set [])
{
    dralListener->SetCycleTagSet(tags[tag_idx], nval, set);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetItemTag(UINT32 item_id, UINT32 tag_idx, UINT64 value)
{
    dralListener->SetItemTag(item_id, tags[tag_idx], value);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetItemTagString(UINT32 item_id, UINT32 tag_idx, UINT32 str_idx)
{
    dralListener->SetItemTagString(item_id, tags[tag_idx], strings[str_idx]);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetItemTagSet(UINT32 item_id, UINT32 tag_idx, UINT32 nval, UINT64 set [])
{
    dralListener->SetItemTagSet(item_id, tags[tag_idx], nval, set);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeTag(UINT16 node_id, UINT32 tag_idx, UINT64 value, UINT16 level, UINT32 list [])
{
    dralListener->SetNodeTag(node_id, tags[tag_idx], value, level, list);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeTagString(UINT16 node_id, UINT32 tag_idx, UINT32 str_idx, UINT16 level, UINT32 list [])
{
    dralListener->SetNodeTagString(node_id, tags[tag_idx], strings[str_idx], level, list);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeTagSet(UINT16 node_id, UINT32 tag_idx, UINT16 n, UINT64 set [], UINT16 level, UINT32 list [])
{
    dralListener->SetNodeTagSet(node_id, tags[tag_idx], n, set, level, list);
}

void
DRAL_LISTENER_CONVERTER_CLASS::Comment(UINT32 magic_num, char * cont)
{
    dralListener->Comment(magic_num, cont);
}

void
DRAL_LISTENER_CONVERTER_CLASS::CommentBin(UINT16 magic_num, char * cont, UINT32 length)
{
    dralListener->CommentBin(magic_num, cont, length);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeInputBandwidth(UINT16 node_id, UINT32 bandwidth)
{
    dralListener->SetNodeInputBandwidth(node_id, bandwidth);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeOutputBandwidth(UINT16 node_id, UINT32 bandwidth)
{
    dralListener->SetNodeOutputBandwidth(node_id, bandwidth);
}

void
DRAL_LISTENER_CONVERTER_CLASS::StartActivity(UINT64 start_activity_cycle)
{
    dralListener->StartActivity(start_activity_cycle);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetTagDescription(UINT32 tag_idx, char description [])
{
    dralListener->SetTagDescription(tags[tag_idx], description);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetNodeClock(UINT16 nodeId, UINT16 clockId)
{
    dralListener->SetNodeClock(nodeId, clockId);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NewClock(UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions, const char name [])
{
    dralListener->NewClock(clockId, freq, skew, divisions, name);
}

void
DRAL_LISTENER_CONVERTER_CLASS::Cycle(UINT16 clockId, UINT64 cycle, UINT16 phase)
{
    dralListener->Cycle(clockId, cycle, phase);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetTagSingleValue(UINT32 item_id, UINT32 tag_idx, UINT64 value, UBYTE time_span_flags)
{
    dralListener->SetTagSingleValue(item_id, tags[tag_idx], value, time_span_flags);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetTagString(UINT32 item_id, UINT32 tag_idx, UINT32 str_idx, UBYTE time_span_flags)
{
    dralListener->SetTagString(item_id, tags[tag_idx], strings[str_idx], time_span_flags);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetTagSet(UINT32 item_id, UINT32 tag_idx, UINT32 set_size, UINT64 * set, UBYTE time_span_flags)
{
    dralListener->SetTagSet(item_id, tags[tag_idx], set_size, set, time_span_flags);
}

void
DRAL_LISTENER_CONVERTER_CLASS::EnterNode(UINT16 node_id, UINT32 item_id, UINT32 slot)
{
    dralListener->EnterNode(node_id, item_id, slot);
}

void
DRAL_LISTENER_CONVERTER_CLASS::ExitNode(UINT16 node_id, UINT32 slot)
{
    dralListener->ExitNode(node_id, slot);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetCapacity(UINT16 node_id, UINT32 capacity, UINT32 capacities [], UINT16 dimensions)
{
    dralListener->SetCapacity(node_id, capacity, capacities, dimensions);
}

void
DRAL_LISTENER_CONVERTER_CLASS::SetHighWaterMark(UINT16 node_id, UINT32 mark)
{
    dralListener->SetHighWaterMark(node_id, mark);
}

void
DRAL_LISTENER_CONVERTER_CLASS::Comment(char * comment)
{
    dralListener->Comment(comment);
}

void
DRAL_LISTENER_CONVERTER_CLASS::AddNode(UINT16 node_id, char * node_name,UINT16 parent_id, UINT16 instance)
{
    dralListener->AddNode(node_id, node_name, parent_id, instance);
}

void
DRAL_LISTENER_CONVERTER_CLASS::AddEdge(UINT16 sourceNode, UINT16 destNode, UINT16 edge_id, UINT32 bandwidth, UINT32 latency, char * name)
{
    dralListener->AddEdge(sourceNode, destNode, edge_id, bandwidth, latency, name);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NewTag(UINT32 tag_idx, const char * tag_name, INT32 tag_name_len)
{
    assert(tag_idx == tags.size());
    tags.push_back((char *) malloc(tag_name_len));
    memcpy(tags[tag_idx], tag_name, tag_name_len);
}

void
DRAL_LISTENER_CONVERTER_CLASS::NewString(UINT32 string_idx, const char * str, INT32 str_len)
{
    assert(string_idx == strings.size());
    strings.push_back((char *) malloc(str_len));
    memcpy(strings[string_idx], str, str_len);
}
