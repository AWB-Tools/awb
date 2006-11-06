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


#ifndef __dralServerImplementation_h
#define __dralServerImplementation_h

#include "asim/dralWrite.h"

/*
 * Interface of the dral server implementation
 */
class DRAL_SERVER_IMPLEMENTATION_CLASS
{
  public:

    DRAL_SERVER_IMPLEMENTATION_CLASS(UINT16 buffer_size, bool compression);
    
    void SetFileDescriptor (int fd);

    virtual ~DRAL_SERVER_IMPLEMENTATION_CLASS(void);

    virtual void NewNode (
        UINT16 node_id, const char name[], UINT16 name_len,
        UINT16 parent_id, UINT16 instance)=0;

    virtual void NewEdge (
        UINT16 edge_id, UINT16 source_node, UINT16 destination_node,
        UINT32 bandwidth, UINT32 latency, const char name[], UINT16 name_len)=0;

    virtual void SetNodeLayout(
        UINT16 node_id,  UINT16 dimensions, UINT32 capacity [])=0;

    virtual void SetNodeTag(
        UINT16 node_id, const char tag_name [], UINT16 tag_name_len,
        UINT64 value, UINT16 level, UINT32 lst [])=0;

    virtual void SetNodeTag(
        UINT16 node_id, const char tag_name [], UINT16 tag_name_len, UINT16 n,
        UINT64 set [], UINT16 level, UINT32 lst [])=0;

    virtual void SetNodeTag(
        UINT16 node_id, const char tag_name [], UINT16 tag_name_len,
        const char str [], UINT16 str_len, UINT16 level, UINT32 lst [])=0;

    virtual void Cycle (UINT64 n)=0;

    virtual void NewItem (UINT32 item_id)=0;

    virtual void SetItemTag (
        UINT32 item_id, const char tag_name[], UINT16 tag_name_len,
        UINT64 value)=0;

    virtual void SetItemTag (
        UINT32 item_id, const char tag_name[], UINT16 tag_name_len,
        const char str[], UINT16 str_len)=0;

    virtual void SetItemTag (
        UINT32 item_id, const char tag_name[], UINT16 tag_name_len,
        UINT32 nval, UINT64 value[])=0;

    virtual void MoveItems (
        UINT16 edge_id, UINT32 n, UINT32 item_id[], UINT32 position [])=0;

    virtual void EnterNode (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position[])=0;

    virtual void ExitNode (
        UINT16 node_id, UINT32 item_id, UINT16 dimensions, UINT32 position[])=0;

    virtual void DeleteItem (UINT32 item_id)=0;

    virtual void Comment (
        UINT32 magic_num, const char comment [], UINT32 length)=0;

    virtual void CommentBin (
        UINT16 magic_num, const char comment [], UINT32 comment_len)=0;

    virtual void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len, UINT64 value)=0;

    virtual void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len,
        const char str[], UINT16 str_len)=0;

    virtual void SetCycleTag (
        const char tag_name[], UINT16 tag_name_len,
        UINT32 nval, UINT64 value[])=0;

    virtual void SetNodeInputBandwidth (UINT16 nodeId, UINT32 bandwidth)=0;

    virtual void SetNodeOutputBandwidth (UINT16 nodeId, UINT32 bandwidth)=0;

    virtual void StartActivity (UINT64 firstActivityCycle)=0;

    virtual void SetTagDescription (
        const char tag [], UINT16 tag_len,
        const char description [], UINT16 desc_len)=0;

    virtual void SetNodeClock (UINT16 nodeId, UINT16 clockId)=0;
    
    virtual void NewClock (
        UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions,
        const char name [], UINT16 nameLen)=0;

    virtual void Cycle (UINT16 clockId, UINT64 n, UINT16 phase)=0;

    /*
     * Public method used to send the version
     * The version will be sent to the file descriptor when the implementation
     * class is created.
     */
    virtual void Version (void)=0;
    
    /*
     * Public method used to flush the write buffer.
     * The buffer will be flushed when turning off the server and when
     * destroying the implementation class.
     */
    void Flush (void);

  protected:

    /*
     * A pointer to the class used to perform buffered write
     */
    DRAL_BUFFERED_WRITE dralWrite;

};
typedef DRAL_SERVER_IMPLEMENTATION_CLASS * DRAL_SERVER_IMPLEMENTATION;


#endif /* __dralServerImplementation_h */
