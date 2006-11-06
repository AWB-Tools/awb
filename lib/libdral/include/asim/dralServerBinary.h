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

#include "asim/dralStringMapping.h"
#include "asim/dralServerBinaryDefines.h"
#include "asim/dralClientBinary_v3.h"

// Stats macros
#ifdef DRAL_STATS
    #define STATS(code) code
#else
    #define STATS(code)
#endif

/*
 * This class is derived from the dral server implemetation class,
 * and performs the binary version implementation of the dral server.
 */
class DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS : public DRAL_SERVER_IMPLEMENTATION_CLASS
{

  public:

    DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS(
        UINT16 buffer_size, bool compression);

    ~DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS();

    void NewNode (UINT16 node_id, const char name[], UINT16 name_len,
        UINT16 parent_id, UINT16 instance);

    void NewEdge (
        UINT16 edge_id, UINT16 source_node, UINT16 destination_node,
        UINT32 bandwidth, UINT32 latency, const char name[], UINT16 name_len);

    void SetNodeLayout(UINT16 node_id, UINT16 dimensions, UINT32 capacity []);

    void SetNodeTag(
        UINT16 node_id, const char tag_name [], UINT16 tag_name_len,
        UINT64 value, UINT16 level, UINT32 list []);

    void SetNodeTag(
        UINT16 node_id, const char tag_name [], UINT16 tag_name_len,
        UINT16 n, UINT64 set [], UINT16 level, UINT32 list []);

    void SetNodeTag(
        UINT16 node_id, const char tag_name [], UINT16 tag_name_len,
        const char str [], UINT16 str_len, UINT16 level, UINT32 list []);

    void Cycle (UINT64 n);

    void NewItem (UINT32 item_id);

    void SetItemTag (
        UINT32 item_id, const char tag_name[], UINT16 tag_name_len, UINT64 value);

    void SetItemTag (
        UINT32 item_id, const char tag_name[], UINT16 tag_name_len,
        const char str[], UINT16 str_len);

    void SetItemTag (
        UINT32 item_id, const char tag_name[], UINT16 tag_name_len,
        UINT32 nval, UINT64 value[]);

    void MoveItems (
        UINT16 edge_id, UINT32 n, UINT32 item_id[], UINT32 position []);

    void EnterNode (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position []);

    void ExitNode (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position []);

    void DeleteItem (UINT32 item_id);

    void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len, UINT64 value);

    void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len,
        const char str[], UINT16 str_len);

    void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len,
        UINT32 nval, UINT64 value[]);

    void Comment (UINT32 magic_num, const char comment [], UINT32 length);

    void CommentBin (UINT16 magic_num, const char comment [], UINT32 comment_len);
    
    void SetNodeInputBandwidth (UINT16 nodeId, UINT32 bandwidth);

    void SetNodeOutputBandwidth (UINT16 nodeId, UINT32 bandwidth);

    void StartActivity (UINT64 firstActivityCycle);

    void SetTagDescription (
        const char tag [], UINT16 tag_len,
        const char description [], UINT16 desc_len);

    void SetNodeClock (UINT16 nodeId, UINT16 clockId);
    
    void NewClock (UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions,
    const char name [], UINT16 nameLen);

    void Cycle (UINT16 clockId, UINT64 n, UINT16 phase);
    
    void Version (void);

  private:
    DRAL_STRING_MAPPING_CLASS tag_map;     ///< Mapping of tags.
    DRAL_STRING_MAPPING_CLASS str_val_map; ///< Mapping of strings values.

    inline UINT8 getTagIndex(const char * tag, UINT16 tag_len);
    inline UINT16 getStringIndex(const char * str, UINT16 str_len);

    inline DRAL3_VALUE_SIZE getValSize(UINT64 val);
    inline DRAL3_VALUE_SIZE getRangeSize(INT32 delta);

    DRAL3_VALUE_SIZE getMaxBitsVector(UINT32 n, UINT32 * values, INT32 last_value);

    INT32 last_item; ///< Contains the last item id.
    INT32 last_node; ///< Contains the last node id.
    INT32 last_edge; ///< Contains the last edge id.

    UINT16 lastClockId; ///< Clock id of the last cycle command.
    UINT16 lastPhase;   ///< Phase of the lasat cycle command.
    UINT64 lastCycle;   ///< Cycle of the lasat cycle command.

    STATS
    (
        void clearMoveItems();
        void computeMoveItems();

        UINT32 move_item_edges[65536];
        UINT64 total_move_item_edges[65536];
        UINT64 max_edge;

        void checkRange(INT32 id, INT32 * last_id, UINT32 * ranges);

        void clearItems();

        UINT32 item_range[4];

        void clearNodes();

        UINT32 node_range[4];

        void clearEdges();

        UINT32 edge_range[4];

        INT32 numOccupancy;
        INT32 numState;
    )

    STATS
    (
        void dumpStats() const;

        UINT64 cycle;
        UINT64 cycleSize;
        UINT64 newItem;
        UINT64 newItemSize;
        UINT64 moveItem;
        UINT64 moveItemSize;
        UINT64 enterNode;
        UINT64 enterNodeSize;
        UINT64 exitNode;
        UINT64 exitNodeSize;
        UINT64 deleteItem;
        UINT64 deleteItemSize;
        UINT64 newNode;
        UINT64 newNodeSize;
        UINT64 newEdge;
        UINT64 newEdgeSize;
        UINT64 setNodeLayout;
        UINT64 setNodeLayoutSize;
        UINT64 comment;
        UINT64 commentSize;
        UINT64 version;
        UINT64 versionSize;
        UINT64 setItemTagVal;
        UINT64 setItemTagValSize;
        UINT64 setItemTagStr;
        UINT64 setItemTagStrSize;
        UINT64 setItemTagSOV;
        UINT64 setItemTagSOVSize;
        UINT64 setNodeTagVal;
        UINT64 setNodeTagValSize;
        UINT64 setNodeTagStr;
        UINT64 setNodeTagStrSize;
        UINT64 setNodeTagSOV;
        UINT64 setNodeTagSOVSize;
        UINT64 setCycleTagVal;
        UINT64 setCycleTagValSize;
        UINT64 setCycleTagStr;
        UINT64 setCycleTagStrSize;
        UINT64 setCycleTagSOV;
        UINT64 setCycleTagSOVSize;
        UINT64 setNodeInput;
        UINT64 setNodeInputSize;
        UINT64 setNodeOutput;
        UINT64 setNodeOutputSize;
        UINT64 startActivity;
        UINT64 startActivitySize;
        UINT64 setTagDescription;
        UINT64 setTagDescriptionSize;
        UINT64 setNodeClock;
        UINT64 setNodeClockSize;
        UINT64 newClock;
        UINT64 newClockSize;

    )
} ;

typedef DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS * DRAL_SERVER_BINARY_IMPLEMENTATION;

UINT8
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::getTagIndex(const char * tag, UINT16 tag_len)
{
    UINT32 index;

    if(tag_map.getMapping(tag, tag_len, &index))
    {
        struct newTagEntryFormat
        {
            UINT8 commandCode : 6;
            UINT8 reserved    : 2;
            UINT8 tag_id;
            UINT8 tag_len;
        } tagEntry;

        tagEntry.commandCode = DRAL3_NEWTAG;
        tagEntry.reserved = 0;
        tagEntry.tag_id = index;
        tagEntry.tag_len = tag_len;

        dralWrite->Write(&tagEntry, sizeof(newTagEntryFormat));
        dralWrite->Write(tag, tag_len);
    }
    return index;
}

UINT16
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::getStringIndex(const char * str, UINT16 str_len)
{
    UINT32 index;

    if(str_val_map.getMapping(str, str_len, &index))
    {
        struct newStringEntryFormat
        {
            UINT8 commandCode : 6;
            UINT8 reserved    : 2;
            UINT16 str_len;
            UINT16 str_id;
        } stringEntry;

        stringEntry.commandCode = DRAL3_NEWSTRINGVALUE;
        stringEntry.reserved = 0;
        stringEntry.str_id = index;
        stringEntry.str_len = str_len;

        dralWrite->Write(&stringEntry, sizeof(newStringEntryFormat));
        dralWrite->Write(str, str_len);
    }
    return index;
}

DRAL3_VALUE_SIZE
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::getValSize(UINT64 val)
{
    if(val & 0xFFFFFFFF00000000ULL)
    {
        return VALUE_64_BITS;
    }
    else
    {
        if(val & 0xFFFF0000)
        {
            return VALUE_32_BITS;
        }
        else
        {
            if(val & 0xFF00)
            {
                return VALUE_16_BITS;
            }
            else
            {
                return VALUE_8_BITS;
            }
        }
    }
}

DRAL3_VALUE_SIZE
DRAL_SERVER_BINARY_IMPLEMENTATION_CLASS::getRangeSize(INT32 delta)
{
    if(delta >= 0)
    {
        if(delta < 128)
        {
            return VALUE_8_BITS;
        }
        else if(delta < 32768)
        {
            return VALUE_16_BITS;
        }
        else
        {
            return VALUE_32_BITS;
        }
    }
    else
    {
        delta = -delta;
        if(delta <= 128)
        {
            return VALUE_8_BITS;
        }
        else if(delta <= 32768)
        {
            return VALUE_16_BITS;
        }
        else
        {
            return VALUE_32_BITS;
        }
    }
}
