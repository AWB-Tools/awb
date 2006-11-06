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
 * @file dralListenerConverter.h
 * @author Guillem Sole
 * @brief Converts the new indexed call-backs to the old non-indexed ones.
 */

#ifndef _DRALLISTENERCONVERTER_H
#define _DRALLISTENERCONVERTER_H

#include <vector>
using namespace std;

#include "asim/dral_syntax.h"
#include "asim/dralListener.h"
#include "asim/dralListenerOld.h"

/**
  * @brief Class used to convert the new dral callbacks to the old ones.
  *
  * This class is used as a wrapper that converts the modified callbacks of
  * the dralListener to the old callbacks. Thanks to this class, the old
  * dral clients don't need to be changed (almost).
  */
class DRAL_LISTENER_CONVERTER_CLASS : public DRAL_LISTENER_CLASS
{
  public:

    DRAL_LISTENER_CONVERTER_CLASS(DRAL_LISTENER_OLD listener);
    virtual ~DRAL_LISTENER_CONVERTER_CLASS();

    /**
      * @name CallBacks for commands found in both DRAL 1.0 and DRAL 2.0
      */
    /*@{*/

    /**
      * @brief Notifies a new time pint into the dral trace
      * @param cycle The current simulation cycle
      */
    virtual void Cycle (UINT64 cycle);

    /**
      * @brief Notifies the declaration of a new item
      * @param item_id The new item identifier
      */
    virtual void NewItem (UINT32 item_id);

    /**
      * @brief Notifies that one or more items have been moved throught an edge
      * @param edge_id The edge identifier
      * @param numOfItems The number of items that have been moved
      * @param items A set with the item identifiers of the moved items
      */
    virtual void MoveItems (
        UINT16 edge_id, UINT32 numOfItems, UINT32 * items);

    /**
      * @brief Notifies that one or more items have been moved throught an edge
      * specifying their position within the edge
      * @param edge_id The edge identifier
      * @param numOfItems The number of items that have been moved
      * @param items A set with the item identifiers of the moved items
      * @param positions A set with the positions of the moved items within the
      * edge
      */
    virtual void MoveItemsWithPositions (
        UINT16 edge_id, UINT32 numOfItems,
        UINT32 * items, UINT32 * positions);

    /**
      * @brief Notifies that an item has been deleted
      * @param item_id The item identifier
      * Further references to this item should not be found
      */
    virtual void DeleteItem (UINT32 item_id);

    /**
      * @brief Notifies that the dral trace has ended
      *
      * This callback is not the result of a DRAL command.
      * The Dral Server invokes this callback when it can not
      * read more data from the file descriptor (usually because the
      * end of file has been reached or an error has been found)
      */
    virtual void EndSimulation (void);

    /**
      * @brief Notifies an error
      *
      * This callback is not the result of a DRAL command. It is invoked by
      * the Dral Client when it finds some error. After invoking this callback,
      * the Dral Client will invoke the EndSiumlation callback, since the Dral
      * Client will not process more events
      * @param error The description of the error found
      */
    virtual void Error (char * error);

    /**
      * @brief Notifies a non critical error
      *
      * This callback is not the result of a DRAL command. It is invoked by
      * the Dral Client when it finds some non critical error. The Dral Client
      * will still be able to process the rest of events.
      * @param error The description of the non critical error found
      */
    virtual void NonCriticalError (char * error);

    /**
      * @brief Notifies the DRAL version used in the dral trace
      *
      * This callback must be the first one invoked by the Dral Client
      * @param version The version
      */
    virtual void Version (UINT16 version);

    /*@}*/ // End of the block: CallBacks for commands found in
           //                   both DRAL 1.0 and DRAL 2.0



    /**
      * @name CallBacks for DRAL 2.0 commands that are not found in DRAL 1.0
      */
    /*@{*/

    /**
      * @brief Notifies the declaration of a new node
      * @param node_id The new node identifier
      * @param node_name The new node name
      * @param parent_id The parent node identifier
      * @param instance The instance number
      */
    virtual void NewNode (
        UINT16 node_id, char * node_name,UINT16 parent_id, UINT16 instance);

    /**
      * @brief Notifies the declaration of a new edge
      * @param sourceNode The source node identifier
      * @param destNode The destination node identifier
      * @param edge_id The new edge identifier
      * @param bandwidth The new edge bandwidth
      * @param latency The new edge latency
      * @param name The new edge name
      */
    virtual void NewEdge (
        UINT16 sourceNode, UINT16 destNode, UINT16 edge_id,
        UINT32 bandwidth, UINT32 latency, char * name);

    /**
      * @brief Notifies the definition of the layout structure of a given node
      * @param node_id The node identifier
      * @param capacity The total capacity of the node in elements
      * @param dim The number of dimensions of the layout
      * @param capacities A set containing the capacity for each dimension of
      * the layout
      */
    virtual void SetNodeLayout (
        UINT16 node_id, UINT32 capacity, UINT16 dim, UINT32 capacities []);

    /**
      * @brief Notifies the flow of an item into a given node
      *
      * A single position list is used to specify which slot the item has
      * entered
      * @param node_id The node identifier
      * @param item_id The item identifier
      * @param dim The number of dimensions of the single position list
      * @param position A set containing the values for each dimension of the
      * single position list 
      */
    virtual void EnterNode (
        UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position []);

    /**
      * @brief Notifies the flow of an item exiting a given node
      *
      * A single position list is used to specify which slot the item has
      * exited
      * @param node_id The node identifier
      * @param item_id The item identifier
      * @param dim The number of dimensions of the single position list
      * @param position A set containing the values for each dimension of the
      * single position list 
      */
    virtual void ExitNode (
        UINT16 node_id, UINT32 item_id, UINT16 dim, UINT32 position []);

    /**
      * @brief Notifies that the current cycle has been set a single value tag
      * @param tag_name The tag name
      * @param value The value of the tag
      */
    virtual void SetCycleTag(UINT32 tag_idx, UINT64 value);

    /**
      * @brief Notifies that the current cycle has been set a string tag
      * @param tag_name The tag name
      * @param str The value of the tag
      */
    virtual void SetCycleTagString(UINT32 tag_idx, UINT32 str_idx);

    /**
      * @brief Notifies that the current cycle has been set a set tag
      * @param tag_name The tag name
      * @param nval The set size
      * @param set The set
      */
    virtual void SetCycleTagSet(UINT32 tag_idx, UINT32 nval, UINT64 set []);

    /**
      * @brief Notifies that an item has been set a single value tag
      * @param item_id The item identifier
      * @param tag_name The tag name
      * @param value The value of the tag
      */
    virtual void SetItemTag(
        UINT32 item_id, UINT32 tag_idx, UINT64 value);

    /**
      * @brief Notifies that an item has been set a string tag
      * @param item_id The item identifier
      * @param tag_name The tag name
      * @param str The value of the tag
      */
    virtual void SetItemTagString(
        UINT32 item_id, UINT32 tag_idx, UINT32 str_idx);

    /**
      * @brief Notifies that an item has been set a set tag
      * @param item_id The item identifier
      * @param tag_name The tag name
      * @param nval The set size
      * @param set The set
      */
    virtual void SetItemTagSet(
        UINT32 item_id, UINT32 tag_idx, UINT32 nval, UINT64 set []);

    /**
      * @brief Notifies that one or more slots within a node have been set a
      * single value tag
      * @param node_id The node identifier
      * @param tag_name The tag name
      * @param value The value of the tag
      * @param level The number of dimensions of the multiple position list
      * @param list A set containing the values for each dimension of the
      * multiple position list
      */
    virtual void SetNodeTag(
        UINT16 node_id, UINT32 tag_idx, UINT64 value,
        UINT16 level, UINT32 list []);

    /**
      * @brief Notifies that one or more slots within a node have been set a
      * string tag
      * @param node_id The node identifier
      * @param tag_name The tag name
      * @param str The string
      * @param level The number of dimensions of the multiple position list
      * @param list A set containing the values for each dimension of the
      * multiple position list
      */
    virtual void SetNodeTagString(
        UINT16 node_id, UINT32 tag_idx, UINT32 str_idx,
        UINT16 level, UINT32 list []);

    /**
      * @brief Notifies that one or more slots within a node have been set a
      * set tag
      * @param node_id The node identifier
      * @param tag_name The tag name
      * @param n The number of elements of the set
      * @param set The set
      * @param level The number of dimensions of the multiple position list
      * @param list A set containing the values for each dimension of the
      * multiple position list
      */
    virtual void SetNodeTagSet(
        UINT16 node_id, UINT32 tag_idx, UINT16 n, UINT64 set [],
        UINT16 level, UINT32 list []);

    /**
      * @brief Notifies a certain comment in the dral trace
      * @param magic_num The comment identifier
      * @param comment The comment
      */
    virtual void Comment (UINT32 magic_num, char * cont);

    /**
      * @brief Notifies a certain binary comment in the dral trace
      * @param magic_num The comment identifier
      * @param comment The comment
      * @param length The comment length
      */
    virtual void CommentBin (UINT16 magic_num, char * cont, UINT32 length);

    /**
      * @brief Notifies the input bandwidth of a node. The input bandwidth is
      * the sum of the bandwith of the edges that arrive to the node
      * @param node_id The node identifier
      * @param bandwidth The input bandwidth
      */
    virtual void SetNodeInputBandwidth(UINT16 node_id, UINT32 bandwidth);

    /**
      * @brief Notifies the output bandwidth of a node. The output bandwidth is
      * the sum of the bandwith of the edges that departs from the node
      * @param node_id The node identifier
      * @param bandwidth The output bandwidth
      */
    virtual void SetNodeOutputBandwidth(UINT16 node_id, UINT32 bandwidth);

    /**
      * @brief Notifies that the graph definition has finished and the cycle
      * the activity is to start
      * @param start_activity_cycle The cycle the activity is to start
      */
    virtual void StartActivity (UINT64 start_activity_cycle);

    /**
      * @brief Notifies that a tag has been set a description
      * @param tag_name The tag
      * @param description The description
      */
    virtual void SetTagDescription (UINT32 tag_idx, char description []);

    /**
    * @brief Notifies the frequency domain of a node
    * @param nodeId Node identifier
    * @param clockId Frequency domain identifier
    */
    virtual void SetNodeClock (UINT16 nodeId, UINT16 clockId);

    /**
    * @brief Notifies a new frequency domain
    * @param clockId The domain identifier
    * @param frequency The frequency of the domain
    * @param skew The skew of the domain
    * @param divisions The number of divisions to display for this clock id
    * @param name The name of the clock
    */
    virtual void NewClock (
        UINT16 clockId, UINT64 freq, UINT16 skew, UINT16 divisions, const char name []);

    /**
      * @brief Notifies a new time point into the dral trace
      * @param clockId The frequency domain of the cycle
      * @param cycle The current simulation cycle
      * @param phase The phase of the cycle
      */
    virtual void Cycle (UINT16 clockId, UINT64 cycle, UINT16 phase);

    /*@}*/ // End of the block: CallBacks for DRAL 2.0 commands that are not
           //                   found in DRAL 1.0




    /** 
      * @name CallBacks for commands only found in DRAL 1.0
      */
    /*@{*/

    /**
      * @brief Notifies that an item has been set a single value tag
      * @param item_id The item identifier
      * @param tag_name The tag name
      * @param value The value of the tag
      * @param time_span_flags This parameter is deprecated. DRAL 2.0 has this
      * command without this flag
      */
    virtual void SetTagSingleValue (
        UINT32 item_id, UINT32 tag_idx,
        UINT64 value, UBYTE time_span_flags);

    /**
      * @brief Notifies that an item has been set a string tag
      * @param item_id The item identifier
      * @param tag_name The tag name
      * @param str The value of the tag
      * @param time_span_flags This parameter is deprecated. DRAL 2.0 has this
      * command without this flag
      */
    virtual void SetTagString (
        UINT32 item_id, UINT32 tag_idx,
        UINT32 str_idx, UBYTE time_span_flags);

    /**
      * @brief Notifies that an item has been set a set tag
      * @param item_id The item identifier
      * @param tag_name The tag name
      * @param set_size The set size
      * @param set The set
      * @param time_span_flags This parameter is deprecated. DRAL 2.0 has this
      * command without this flag
      */
    virtual void SetTagSet (
        UINT32 item_id, UINT32 tag_idx, UINT32 set_size,
        UINT64 * set, UBYTE time_span_flags);

    /**
      * @brief Notifies the flow of an item into a given node
      *
      * In DRAL 2.0 the slot is not specified by a single value number but
      * with a single position list
      * @param node_id The node identifier
      * @param item_id The item identifier
      * @param slot The slot within the node the item has entered
      */
    virtual void EnterNode (
        UINT16 node_id, UINT32 item_id, UINT32 slot);

    /**
      * @brief Notifies the flow of an item exiting a given node
      *
      * In DRAL 2.0 the slot is not specified by a single value number but
      * with a single position list. The item identifier not present here
      * is also specified
      * @param node_id The node identifier
      * @param slot The slot within the node that has been left
      */
    virtual void ExitNode (UINT16 node_id, UINT32 slot);

    /**
      * @brief Notifies the definition of the layout structure of a given node
      *
      * In DRAL 2.0 this command is replaced by the SetLayout command
      * @param node_id The node identifier
      * @param capacity The node capacity
      * @param capacities A set with the different capacities for each
      * dimension of the node layout
      * @param dimensions The set size (this parameter must also be the number
      * of dimensions of the node layout)
      */
    virtual void SetCapacity (
        UINT16 node_id, UINT32 capacity,
        UINT32 capacities [], UINT16 dimensions);

    /**
      * @brief Notifies that a node in the graph has been set a high water mark
      *
      * In DRAL 2.0 this command is not specified, but one can use the
      * SetTagNode command to notify the same information
      * @param node_id The node identifier
      * @param mark The water mark
      */
    virtual void SetHighWaterMark (UINT16 node_id, UINT32 mark);

    /**
      * @brief Notifies a comment in the session
      *
      * DRAL 2.0 has this command with one extra parameter specifying the magic
      * number of the comment
      * @param comment The comment
      */
    virtual void Comment (char * comment);

    /**
      * @brief Notifies the declaration of a new node
      * @param node_id The new node identifier
      * @param node_name The new node name
      * @param parent_id The parent node identifier
      * @param instance The instance number
      */
    virtual void AddNode (
        UINT16 node_id, char * node_name,UINT16 parent_id, UINT16 instance);

    /**
      * @brief Notifies the declaration of a new edge
      * @param sourceNode The source node identifier
      * @param destNode The destination node identifier
      * @param edge_id The new edge identifier
      * @param bandwidth The new edge bandwidth
      * @param latency The new edge latency
      * @param name The new edge name
      */
    virtual void AddEdge (
        UINT16 sourceNode, UINT16 destNode, UINT16 edge_id,
        UINT32 bandwidth, UINT32 latency, char * name);

    /**
      * @brief Notifies the declaration of a new tag
      * @param tag_idx The index associated to the tag
      * @param tag_name The name of the tag
      * @param tag_name_len The length of the tag
      */
    virtual void NewTag (
        UINT32 tag_idx, const char * tag_name, INT32 tag_name_len);

    /**
      * @brief Notifies the declaration of a new string
      * @param string_idx The index associated to the tag
      * @param str The string
      * @param str_len The length of the string
      */
    virtual void NewString (
        UINT32 string_idx, const char * str, INT32 str_len);

    /*@}*/ // End of the block: CallBacks for commands only found in DRAL 1.0

  private:
    DRAL_LISTENER_OLD dralListener; ///< Pointer to the old listener to whom the callbacks are forwarded.
    vector<char *> tags;            ///< Array of the declared tags.
    vector<char *> strings;         ///< Array of the declared strings.
};

typedef DRAL_LISTENER_CONVERTER_CLASS * DRAL_LISTENER_CONVERTER; /**<Pointer type to
                                                                   * a DRAL_LISTENER_CONVERTER_CLASS
                                                                   */

#endif /* _DRALLISTENERCONVERTER_H */
