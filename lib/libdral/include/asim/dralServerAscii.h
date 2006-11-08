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


#ifndef __dralServerAscii_h
#define __dralServerAscii_h

#include "asim/dralServerImplementation.h"
#include "asim/dralServerDefines.h"

/*
 * This class is derived from the dral server implemetation class,
 * and performs the ascii version implementation of the dral server.
 * The ascii files generated with this class are much bigger than the ones
 * generated by the binary implementation class.
 * So this implementation class should not be used very often.
 * In the future, a tool to translate form dral binary files to ascii binary
 * files will be provided, and this class will be removed from this code
 */
class DRAL_SERVER_ASCII_IMPLEMENTATION_CLASS
    : public DRAL_SERVER_IMPLEMENTATION_CLASS
{


  public:

    DRAL_SERVER_ASCII_IMPLEMENTATION_CLASS(
        UINT16 buffer_size, bool compression);

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

    void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len, UINT64 value);

    void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len,
        const char str[], UINT16 str_len);

    void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len,
        UINT32 nval, UINT64 value[]);

    void MoveItems (
        UINT16 edge_id, UINT32 n, UINT32 item_id[], UINT32 position []);

    void EnterNode (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position []);

    void ExitNode (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position []);

    void DeleteItem (UINT32 item_id);

    void Comment (UINT32 magic_num, const char comment [], UINT32 length);

    void CommentBin (UINT16 magic_num, const char comment [], UINT32 comment_len);

    void SetNodeInputBandwidth (UINT16 nodeId, UINT32 bandwidth);

    void SetNodeOutputBandwidth (UINT16 nodeId, UINT32 bandwidth);

    void StartActivity (UINT64 firstActivityCycle);

    void SetTagDescription (
        const char tag [], UINT16 tag_len,
        const char description [], UINT16 desc_len);

    void SetNodeClock (UINT16 nodeId, UINT16 clockId);
    
    void NewClock (
        UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions,
        const char name [], UINT16 nameLen);

    void Cycle (UINT16 clockId, UINT64 n, UINT16 phase);

    void Version (void);

  private:

    /*
     * Private methods used to convert an array of values
     * to a string with this values in ASCII format
     */
    void Fill_values (ostringstream * value_list,UINT32 nval, UINT64 value[]);
    void Fill_values_and_positions (
        ostringstream * value_list,UINT32 nval,
        UINT32 value[], UINT32 positions []);
    void Write_single_position_list (
        ostringstream * sout,UINT16 dim, UINT32 positions []);
    void Write_multiple_list (
        ostringstream * sout,UINT16 dim, UINT32 positions []);
};
typedef DRAL_SERVER_ASCII_IMPLEMENTATION_CLASS *
    DRAL_SERVER_ASCII_IMPLEMENTATION;

#endif /* __dralServerAscii_h */