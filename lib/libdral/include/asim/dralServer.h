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
 * @file dralServer.h
 * @author Pau Cabre 
 * @brief dral server interface
 */



#ifndef __dralServer_h
#define __dralServer_h

#include <sstream>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <set>

#include "asim/dral_syntax.h"
#include "asim/dralServerImplementation.h"
#include "asim/dralStorage.h"
#include "asim/dralServerDefines.h"
#include "asim/dralCommonDefines.h"

/*
 * Autocompress stuff, Federico Ardanaz @ BSSAD, November 2004.
 *
 */
typedef map <UINT16, UINT64> Nodetagcache_tag_map; 
typedef map <string, Nodetagcache_tag_map*> Nodetagcache_map;


/*
 * liveItems typedefs
 */
typedef set<UINT32> liveItemsList;
//typedef HASH_NAMESPACE::hash_set<UINT32> liveItemsList;

/**
 * @class DRAL_SERVER_CLASS
 * This class mission is to help developers with the generation of DRAL 2.0 trace files.
 * Every object of this class writes dral events into a single file. Although one program may 
 * have more than one dralServer object.
 * 
 * A Dral server does not only provide a list of calls that get 
 * translated into dral 2.0 command bit encodings, it has some goodies:
 * - Use of buffered writing to minimize write system calls
 * - Automatic compression of output files
 * - Start and stop writing at user convenience (Turn on and off a dralServer)
 * - Possibility of closing the output file and continue writing into a new file (only when the server is turned off) 
 * - Dral server is able to remember dral commands used when its turned off, and the write them 
 *   All when it is turned on again (only for those commands that are persistent)
 * For more explanation about DRAL 2.0 \see Dral 2.0 Documentation.
 * 
 * @brief Dral server mission is to write DRAL-Language trace files. 
 */
class DRAL_SERVER_CLASS
{

  public:

     /**
     * Constructor with the file name that will be used to dump event traces.
     * Dral servers are turned off by default when they are created. So, nothing is written to the 
     * output file until \c TurnOn() method is called.
     * @brief Constructor 
     * @param fileName String holding the name of the file that will be used to write the events
     * @param bufferSize size of the internal buffer. (Dral server is always buffered) 
     * @param avoidRep if true then dral server accepts nodes with the same name and instance number.
     */
     DRAL_SERVER_CLASS (
        char * fileName,           // the file name
        UINT16 bufferSize = 4096,  // buffer size
        bool avoidRep = false,     // shall the dral server specify different
                                   // instance numbers for nodes with the same name
        bool compression = true,   // compress the output
        bool ebededTarFile = true); //automaticaly add the embeded tar file

    /**
     * Constructor with the file descriptor that will be used to dump event traces.
     * Dral servers are turned off by default when they are created. So, nothing is written to the 
     * output file until \c TurnOn() method is called.
     * @note The file descriptor will not be closed
     * @brief Constructor 
     * @param fd file descriptor that will be used to write the events
     * @param bufferSize size of the internal buffer. (Dral server is always buffered) 
     * @param avoidRep if true then dral server accepts nodes with the same name and instance number.
     */
    DRAL_SERVER_CLASS (
        int fd,                    // the file descriptor
        UINT16 bufferSize = 4096,  // buffer size
        bool avoidRep = false,     // shall the dral server specify different
                                   // instance numbers for nodes with the same name
        bool compression = true,   // compress the output
        bool ebededTarFile = true); //automaticaly add the embeded tar file

    /**
     * @note the output file will be only closed if dral server was created using a file name instead of a file descriptor. 
     * @brief Void destructor. Used to free memory, close the file descriptor....
     */
    ~DRAL_SERVER_CLASS();

    /**
    * Method used to enable the writing to the dral output file.
    * When dral server is on, all command calls to the dral server are dumped into the output file.
    * But, while turned off, the persistent commands are stored internally. 
    * So when \c TurnOn() is called, all the persistent commands generated since the server was turned off
    * (or since it was created if it has never been turned on) are dumped into the output trace file.
    * If the dral output file has changed using \c ChangeFileName, the next time the server will be turned on,
    * all the persistent commands will be dumped.
    * No files will be opened or created if the server is not turned on.
    * @brief Turns on the dral event dumping into a file.
    */
    void TurnOn();

    /**
    * Method used to disable the writing to the dral output file.
    * When dral server is turned off non-persistent command calls to dral server are lost.
    * But persistent commands are stored internally by the dral server. Then, if eventually the
    * the dral server is turned on again, all these command will be dumped into the output file.
    * @brief Turns off the dral event dumping into a file.
    */
    void TurnOff();

    /**
      * This functions turns on and off the autocompression mechanism for setnodetags
      * While activated, if a setnodetag is performed with the same value than previous
      * command for the same slot, the second (redundant) command is dropped.
      */
    void setNoteTagAutocompress(bool value);
    
    /**
    * Every time that cycle command is called sets a new time stamp for the forthcoming of the commands.
    * DRAL language aspects that the cycles are monotonic increasing, but they do not need to be consecutive. The 
    * existence of time gaps is accepted.
    * @brief Sets a new time point into the dral trace. 
    * @param n Number of the new cycle that has just started.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void Cycle (UINT64 n, bool persistent=false);

    /** 
    * Nodes and edges are the building blocks of dral structure.
    * Nodes need an unique identifier that will be used for the rest of the commands that require a node.
    * It is highly recommended to create a node before using it in other functions. And although it is not 
    * required, create nodes before starting the creation edges and items is recommended.
    * @brief Function used to declare a new node.
    * @return Returns an unique identifier for that node. Such identifier 
    *         must be used in other dral command when referring to this node.
    *         The dral server has an internal counter to provide this value. This counter is increased
    *         each time this method is called.
    *         This counter is not affected by the \p nodeId paramter of the other \c NewNode method.
    *         So the user must be carefull when using both methods to delcare nodes.
    * @param name String containing the name of the node. 
    * @param parentId Node identifier of a parent node. It may be used to 
    *        specify hierarchy between nodes, like a sub-node \em contained in a parent node. 
    * @param instance Used to differentiate between nodes when there is more than one node with the same \p name.         
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    * @param autoflush All the items entered in the node will be automatically exited in the following cycle
    * @param relaxed Allow explicit calls to ExitNode in autoflush node
    */
    UINT16  
    NewNode (const char name[], UINT16 parentId, UINT16 instance=0,
             bool persistent=true, bool autoflush=false, bool relaxed=false);
    
    /** 
    * Nodes and edges are the building blocks of dral structure.
    * Nodes need an unique identifier that will be used for the rest of the commands that require a node.
    * It is highly recommended to create a node before using it in other functions. And although it is not 
    * required, create all nodes before starting the creation edges and items is recommended.
    * @brief Function used to declare a new node. The user chooses an unique identifier for the node.
    * @param nodeId Unique identifier for that node. Such identifier 
    *         must be used in other dral command when referring to this node.  
    * @param name String containing the name of the node. 
    * @param parentId Node identifier of a parent node. It may be used to 
    *        specify hierarchy between nodes, like a sub-node \em contained in a parent node. 
    * @param instance Used to differentiate between nodes when there is more than one node with the same \p name.         
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    * @param autoflush All the items entered in the node will be automatically exited in the following cycle
    * @param relaxed Allow explicit calls to ExitNode in autoflush node
   */
    void NewNode (
        UINT16 nodeId, const char name[], UINT16 parentId, UINT16 instance=0,
	bool persistent=true, bool autoflush=false, bool relaxed=false);

    /** 
    * Nodes and edges are the building blocks of dral structure.
    * Edges need an unique identifier that will be used for the rest of the commands that require a node.
    * It is highly recommended to create an edge before using it in other functions. And although it is not 
    * required, create all edges before starting the creation items is recommended.
    * @brief Function used to declare a new edge.
    * @return Returns an unique identifier for that edge. Such identifier 
    *         must be used in other dral command when referring to this edge.  
    *         The dral server has an internal counter to provide this value. This counter is increased
    *         each time this method is called.
    *         This counter is not affected by the \p edgeId paramter of the other \c NewEdge method.
    *         So the user must be carefull when using both methods to delcare edges.
    * @param sourceNode node identifier where the edge originates.
    * @param destinationNode node identifier where the edge ends.
    * @param bandwidth Maximum number of items per cycle that may flow in this node
    * @param latency Travel time for an item to go from the \p sourceNode to the \p destinationNode.
    * @param name String containing the name of the edge. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    UINT16  /* it returns the edgeId */
    NewEdge (UINT16 sourceNode, UINT16 destinationNode,
        UINT32 bandwidth, UINT32 latency, const char name[], bool persistent=true);

    /** 
    * Nodes and edges are the building blocks of dral structure.
    * Edges need an unique identifier that will be used for the rest of the commands that require a node.
    * It is highly recommended to create an edge before using it in other functions. And although it is not 
    * required, create all edges before starting the creation items is recommended.
    * @brief Function used to declare a new edge. The user chooses an unique identifier for the edge.
    * @param edgeId Unique identifier for that edge. Such identifier 
    *         must be used in other dral command when referring to this edge.  
    * @param sourceNode node identifier where the edge originates.
    * @param destinationNode node identifier where the edge ends.
    * @param bandwidth Maximum number of items per cycle that may flow in this node
    * @param latency Travel time for an item to go from the \p sourceNode to the \p destinationNode.
    * @param name String containing the name of the edge. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void NewEdge (
        UINT16 edgeId, UINT16 sourceNode, UINT16 destinationNode,
        UINT32 bandwidth, UINT32 latency, const char name[], bool persistent=true);
 
    /**
    * Items are the dynamic pieces of the DRAL world. Every item need an unique identifier within whole simulation,
    * even after it is deleted.
    * It is highly recommended to create an item before using it in other functions.
    * @brief Creates a new Item.
    * @return Returns an unique identifier for the new item created.
    *         The dral server has an internal counter to provide this value. This counter is increased
    *         each time this method is called.
    *         This counter is not affected by the \p itemId paramter of the other \c NewItem method.
    *         So the user must be carefull when using both methods to delcare new items.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    UINT32  /* it returns the itemId */
    NewItem (bool persistent=false);

    /**
    * Items are the dynamic pieces of the DRAL world. Every item need an unique identifier within whole simulation,
    * even after it is deleted.
    * It is highly recommended to create an item before using it in other functions.
    * @brief Creates a new Item. The user must provide the unique identifier for the item.
    * @param itemId Unique identifier for the new item created.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void NewItem (UINT32 itemId, bool persistent=false);

    /**
    * Items are the dynamic pieces of the DRAL world. Every item need an unique identifier within whole simulation,
    * even after it is deleted.
    * It is highly recommended to delete items as soon as they are not needed by other functions. Keeping quiescent alive
    * items may hurt some dral client tools and viewers performance.
    * @brief Deletes an existing item. 
    * @param itemId Unique identifier for the deleted item.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void DeleteItem (UINT32 itemId, bool persistent=false);

    /**
    * Move items through an edge. With the \p position is possible to specify 
    * the exact position of the edge where items are moving.
    * @brief Move some items through an edge. 
    * @param edgeId Unique identifier of the Edge 
    * @param n number of items to be moved
    * @param itemId Array holding the unique identifiers for the items that are moving. 
    * @param position (optional) array holding values between 0 and edgeBandwidth - 1. 
    *        Denotes the exact position of the edge where items are moving. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void MoveItems (
        UINT16 edgeId, UINT32 n, UINT32 itemId[], UINT32 position [] = NULL, bool persistent=false);

    /**
    * Move an item through an edge. With the \p position is possible to specify 
    * the exact position of the edge where such item is moving.
    * @brief Move an item through an edge. 
    * @param edgeId Unique identifier of the edge 
    * @param itemId unique identifiers for the items that is moving. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void MoveItem (UINT16 edgeId, UINT32 itemId, bool persistent=false)
    {
        MoveItems(edgeId,1,&itemId,NULL,persistent);
    }
 
    /**
    * Move an item through an edge. With the \p position is possible to specify 
    * the exact position of the edge where such item is moving.
    * @brief Move an item through an edge. 
    * @param edgeId Unique identifier of the edge 
    * @param itemId unique identifiers for the items that is moving. 
    * @param position (optional) value between 0 and edgeBandwidth - 1. 
    *        Denotes the exact position of the edge where the item is moving. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void MoveItem (UINT16 edgeId, UINT32 itemId, UINT32 position, bool persistent=false)
    {
        MoveItems(edgeId,1,&itemId,&position,persistent);
    }
 
    /** 
    * Defines the layout structure for a node. Every node should be seen as a multidimensional symmetric array. 
    * @brief Defines the layout structure for a node.
    * @param nodeId Node identifier 
    * @param dimensions Number of dimensions of the node.
    * @param capacity Array containing the size of every dimension.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetNodeLayout(
        UINT16 nodeId,  UINT16 dimensions, UINT32 capacity [],
        bool persistent=true);
    
    /** 
    * @brief Defines the layout structure for a unidimensional node. 
    * @param nodeId Node identifier 
    * @param capacity Number of entries in the first and only dimension of the node. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void
    SetNodeLayout(UINT16 nodeId, UINT32 capacity, bool persistent=true)
    {
        SetNodeLayout(nodeId,1,&capacity,persistent);
    }

    /**
    * Command used to define the flow of an item into a node.  
    * For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    * @brief Define the flow of an item into a given node. 
    * @param nodeId Unique identifier of the node.
    * @param itemId Unique identifier of the item.
    * @param dimensions Length of the \p position parameter array.
    * @param position Array specifying the exact position in the multidimensional 
    *        structure of the node where the item is entering.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void EnterNode (
        UINT16 nodeId, UINT32 itemId, UINT16 dimensions, UINT32 position[], bool persistent=false);

    /**
    * Command used to define the flow of an item into a node.  
    * For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    * @brief Define the flow of an item into a given node. 
    * @param nodeId Unique identifier of the node.
    * @param itemId Unique identifier of the item.
    * @param position Position in the first and only dimension of the node
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void 
    EnterNode (UINT16 nodeId, UINT32 itemId, UINT32 position, bool persistent=false)
    {
        EnterNode(nodeId, itemId, 1, &position, persistent);
    }

    /**
    * Command used to define the flow of an item exiting a node.  
    * For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    * @brief Define the flow of an item exiting a given node. 
    * @param nodeId Unique identifier of the node.
    * @param itemId Unique identifier of the item.
    * @param dimensions Length of the \p position parameter array.
    * @param position Array specifying the exact position in the multidimensional 
    *        structure of the node where the item is exiting from.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void ExitNode (
        UINT16 nodeId, UINT32 itemId, UINT16 dimensions, UINT32 position[], bool persistent=false);

    /**
    * Command used to define the flow of an item exiting a node.  
    * For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    * @brief Define the flow of an item exiting a given node. 
    * @param nodeId Unique identifier of the node.
    * @param itemId Unique identifier of the item.
    * @param position Position in the first and only dimension of the node
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void 
    ExitNode (UINT16 nodeId, UINT32 itemId, UINT32 position, bool persistent=false)
    {
        ExitNode(nodeId, itemId, 1, &position, persistent);
    }

    /**
    * Attaches a TagName-Value couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param value Value for the Tag 
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetNodeTag(
        UINT16 nodeId, const char tagName [], UINT64 value,
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent = false);

 
    /**
    * Attaches a TagName-Value couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param value Value for the Tag 
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], INT64 value,
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        SetNodeTag(nodeId,tagName,UINT64(value),level,lst,persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param value Value for the Tag 
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], UINT32 value,
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        SetNodeTag(nodeId,tagName,UINT64(value),level,lst,persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param value Value for the Tag 
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], INT32 value,
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        SetNodeTag(nodeId,tagName,UINT64(value),level,lst,persistent);
    }

 
    /**
    * Attaches a TagName-MultipleValue couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param n Number of values for the tag.
    * @param set Array containing all the values for the tag.
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetNodeTag(
        UINT16 nodeId, const char tagName [], UINT16 n, UINT64 set [],
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false);

    /**
    * Attaches a TagName-MultipleValue couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param n Number of values for the tag.
    * @param set Array containing all the values for the tag.
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], UINT16 n, INT64 set [],
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        SetNodeTag(
            nodeId,tagName,n,reinterpret_cast<UINT64 *>(set),
            level,lst,persistent);
    }

 
    /**
    * Attaches a TagName-MultipleValue couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param n Number of values for the tag.
    * @param set Array containing all the values for the tag.
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], UINT16 n, UINT32 set [],
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        DRAL_ASSERT(tagName!=NULL,"No tag name provided");
        UINT16 tag_name_len = strlen(tagName);
        DRAL_ASSERT(tag_name_len < 255 && tag_name_len != 0,
            "Parameter tagName " << tagName << " is too long");
        DRAL_ASSERT(n<65535 && n!=0 && set != NULL,
            "The set size is not valid");
        UINT64 * temp = new UINT64 [n];
        for(UINT16 i=0; i<n; ++i)
        {
            temp[i]=UINT64(set[i]);
        }
        SetNodeTag(nodeId,tagName,n,temp,level,lst,persistent);
    }

 
    /**
    * Attaches a TagName-MultipleValue couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param n Number of values for the tag.
    * @param set Array containing all the values for the tag.
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], UINT16 n, INT32 set [],
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        SetNodeTag(
            nodeId,tagName,n,reinterpret_cast<UINT64 *>(set),
            level,lst,persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param str String containing the value of the tag. 
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetNodeTag(
        UINT16 nodeId, const char tagName [], const char str [],
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false);

    /**
    * Attaches a TagName-Value couple to a node, dimension of a node or positions 
    * of a node. For a more detailed explanation:  \see Dral 2.0 Documentation. \see Dral defines values
    *
    * According to dral language semantics, attaching a tag is to a node makes the node to have this tag 
    * for all the simulation. So, set a tag in cycle N for first time , makes that node to have this value since cycle 0 until
    * the simulation ends or the same tag is set again with a different value some cycle in the future.
    * So, SetNodeTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a node.
    * @param nodeId Node identifier.
    * @param tagName String containing the name of the tag. 
    * @param c Character containing the value for the tag 
    * @param level Specifies the depth of the \p list param
    * @param lst List used to specify where in the node the tag is attached.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetNodeTag(
        UINT16 nodeId, const char tagName [], char c,
        UINT16 level = 0, UINT32 lst [] = NULL, bool persistent=false)
    {
        char temp [2];
        temp[0]=c;
        temp[1]='\0';
        SetNodeTag(nodeId,tagName,temp,level,lst,persistent);
    }

    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetItemTag (
        UINT32 itemId, const char tagName[],
        UINT64 value, bool persistent=false);

    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetItemTag (
        UINT32 itemId, const char tagName[],
        INT64 value,bool persistent=false)
    {
        SetItemTag(itemId,tagName,UINT64(value),persistent);
    }

    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetItemTag (
        UINT32 itemId, const char tagName[],
        UINT32 value,bool persistent=false)
    {
        SetItemTag(itemId,tagName,static_cast<UINT64>(value),persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetItemTag (
        UINT32 itemId, const char tagName[],
        INT32 value,bool persistent=false)
    {
        SetItemTag(itemId,tagName,static_cast<UINT64>(value),persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param str String containing New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetItemTag (
        UINT32 itemId, const char tagName[],
        const char str[], bool persistent=false);

  
    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param character Character containing the value for the tag 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetItemTag (
        UINT32 itemId, const char tagName[], char character,
        bool persistent=false)
    {
        char temp[2];
        temp[0]=character;
        temp[1]='\0';
        SetItemTag(itemId,tagName,temp,persistent);
    }
  
    /**
    * Attaches a TagName-Value couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetItemTag (
        UINT32 itemId, const char tagName[], UINT32 nval,
        UINT64 value[], bool persistent=false);
  
    /**
    * Attaches a TagName-MultivalueValue couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetItemTag (
        UINT32 itemId, const char tagName[], UINT32 nval,
        INT64 value[], bool persistent=false)
    {
        SetItemTag(
            itemId,tagName,nval,reinterpret_cast<UINT64 *>(value),persistent);
    }

   
    /**
    * Attaches a TagName-MultivalueValue couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetItemTag (
        UINT32 itemId, const char tagName[], UINT32 nval,
        UINT32 value[], bool persistent=false)
    {
        DRAL_ASSERT(tagName!=NULL,"No tag name provided");
        UINT16 tag_name_len = strlen(tagName);
        DRAL_ASSERT(tag_name_len < 255 && tag_name_len != 0,
            "Parameter tagName " << tagName << " is too long");
        DRAL_ASSERT(nval<65535 && nval!=0 && value != NULL,
            "The set size is not valid");
        UINT64 * temp = new UINT64 [nval];
        for(UINT32 i=0;i<nval;++i)
        {
            temp[i]=UINT64(value[i]);
        }
        SetItemTag(itemId,tagName,nval,temp,persistent);
    }

    
    /**
    * Attaches a TagName-MultivalueValue couple to an item.
    * According to dral language semantics, attaching a tag is to an item makes the item to have this tag 
    * for all its live (from NewItem time to DeleteItem time). So, set a tag in cycle N for first time , makes that node to have this value since NewItem time until
    * the DeleteItem time or the same tag is set again with a different value some cycle in the future.
    * So, SetItemTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to an item.
    * @param itemId Unique item identifier.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
   inline void SetItemTag (
        UINT32 itemId, const char tagName[], UINT32 nval,
        INT32 value[], bool persistent=false)
    {
        SetItemTag(
            itemId,tagName,nval,reinterpret_cast<UINT32 *>(value),persistent);
    }

    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetCycleTag (
        const char tagName[],
        UINT64 value, bool persistent=false);

    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[],
        INT64 value,bool persistent=false)
    {
        SetCycleTag(tagName,UINT64(value),persistent);
    }

    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[],
        UINT32 value,bool persistent=false)
    {
        SetCycleTag(tagName,static_cast<UINT64>(value),persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param value New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[],
        INT32 value,bool persistent=false)
    {
        SetCycleTag(tagName,static_cast<UINT64>(value),persistent);
    }

 
    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param str String containing New/First value for the tag. 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetCycleTag (
        const char tagName[],
        const char str[], bool persistent=false);

  
    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param character Character containing the value for the tag 
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[], char character,
        bool persistent=false)
    {
        char temp[2];
        temp[0]=character;
        temp[1]='\0';
        SetCycleTag(tagName,temp,persistent);
    }
  
    /**
    * Attaches a TagName-Value couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void SetCycleTag (
        const char tagName[], UINT32 nval,
        UINT64 value[], bool persistent=false);
  
    /**
    * Attaches a TagName-MultivalueValue couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[], UINT32 nval,
        INT64 value[], bool persistent=false)
    {
        SetCycleTag(tagName,nval,reinterpret_cast<UINT64 *>(value),persistent);
    }

   
    /**
    * Attaches a TagName-MultivalueValue couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[], UINT32 nval,
        UINT32 value[], bool persistent=false)
    {
        DRAL_ASSERT(tagName!=NULL,"No tag name provided");
        UINT16 tag_name_len = strlen(tagName);
        DRAL_ASSERT(tag_name_len < 255 && tag_name_len != 0,
            "Parameter tagName " << tagName << " is too long");
        DRAL_ASSERT(nval<65535 && nval!=0 && value != NULL,
            "The set size is not valid");
        UINT64 * temp = new UINT64 [nval];
        for(UINT32 i=0;i<nval;++i)
        {
            temp[i]=UINT64(value[i]);
        }
        SetCycleTag(tagName,nval,temp,persistent);
    }

    
    /**
    * Attaches a TagName-MultivalueValue couple to a cycle.
    * According to dral language semantics, attaching a tag is to a cycle makes the cycle to have this tag 
    * for all its live (So, just one cycle). 
    * SetCycleTag may also be used to change the value of a previously attached Tag.
    * @brief Set a tag with a value to a cycle.
    * @param tagName String containing the name of the tag. 
    * @param nval Number of values of the Tag. 
    * @param value Array holding \p nval values for the tag.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    inline void SetCycleTag (
        const char tagName[], UINT32 nval,
        INT32 value[], bool persistent=false)
    {
        SetCycleTag(tagName,nval,reinterpret_cast<UINT32 *>(value),persistent);
    }
   

    /**
    * This method add comments to the file. The client is able to see them
    * as well.
    * @brief used to insert comments into the dral trace
    * @param magicNum number identifying the source of the comments.
    * @param comment String holding the comment itself.
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void Comment (UINT32 magicNum, const char comment [], bool persistent=false);

    /**
    * This method add binary comments to the file. The client is able to see them
    * as well.
    * @brief used to insert binary comments into the dral trace
    * @param magicNum number identifying the source of the comments.
    * @param comment String holding the comment itself.
    * @param length the length of the binary comment
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void CommentBin (UINT16 magicNum, const char comment [], UINT32 length, bool persistent=false);
    
    /**
    * Function used to change the name of the dral output file. 
    * It can only be used when the dral server is turned off and
    * has been created using the constructor that has a file name. 
    * It closes the previous file.
    * All the persistent events generated since the server was created will be dumped the next time the server
    * will be turned on.
    * @brief Changes the name of the dral trace output file.
    * @param fileName Name of the new output file
    */
    void ChangeFileName(const char * fileName);

    /**
    * Defines the total incoming bandwith of a node. It can be calculated by
    * adding the bandwidth of all the edges whose destination is the node.
    * @brief Sets the input bandwidth of the node
    * @param nodeId Node identifier
    * @param bandwidth Total incoming bandwidth
    * @param persistent Tells if this command should be remembered when the
    * dral server is turned off.
    */
    void SetNodeInputBandwidth(UINT16 nodeId, UINT32 bandwidth, bool persistent=true);
    
    /**
    * Defines the total outgoing bandwith of a node. It can be calculated by
    * adding the bandwidth of all the edges whose source is the node.
    * @brief Sets the output bandwidth of the node
    * @param nodeId Node identifier
    * @param bandwidth Total outgoing bandwidth
    * @param persistent Tells if this command should be remembered when the
    * dral server is turned off.
    */
    void SetNodeOutputBandwidth(UINT16 nodeId, UINT32 bandwidth, bool persistent=true);

    /**
    * This command give assigns a description to a tag
    * @brief Defines the meaning of a tag
    * @param tag The tag
    * @param description The tag description
    * @param persistent Tells if this command should be remembered when the
    * dral server is turned off.
    */
    void SetTagDescription(const char tag [], const char description [], bool persistent=true);

    /**
    * This command specifies the frequency domain associated with a certain node
    * @brief Sets the frequency domain of a node
    * @param nodeId Node identifier
    * @param clockId Frequency domain identifier
    * @param persistent Tells if this command should be remembered when the
    * dral server is turned off.
    */
    void SetNodeClock(UINT16 nodeId, UINT16 clockId, bool persistent=true);

    /**
    * Method used to specify a new frequency domain. The dral server will return
    * an identifier for this new domain.
    * @brief Specifies a new frequency domain
    * @param frequency The frequency of the domain
    * @param skew The skew of the domain
    * @param divisions The number of divisions to display
    * @param name The name of the clock
    * @param persistent Tells if this command should be remembered when the
    * dral server is turned off.
    */
    UINT16 NewClock(UINT64 frequency, UINT16 skew, UINT16 divisions, const char name [], bool persistent=true);

    /**
    * Method used to specify a new frequency domain. User must specify the
    * domain id of the new domain.
    * @brief Specifies a new frequency domain
    * @param clockId The frequency domain identifier
    * @param frequency The frequency of the domain (multiples 10Mhz)
    * @param skew The skew of the domain
    * @param divisions The number of divisions to display
    * @param name The name of the clock
    * @param persistent Tells if this command should be remembered when the
    * dral server is turned off.
    */
    void NewClock(UINT16 clockId, UINT64 frequency, UINT16 skew, UINT16 divisions, const char name [], bool persistent=true);

    /**
    * Every time that cycle command is called sets a new time stamp for the forthcoming of the commands.
    * DRAL language aspects that the cycles are monotonic increasing, but they do not need to be consecutive. The 
    * existence of time gaps is accepted.
    * @brief Sets a new time point into the dral trace.
    * @param clockId Frequency domain identifier of that cycle
    * @param cycle Number of the new cycle that has just started.
    * @param phase Number of the phase of that cycle
    * @param persistent Tells if this command should be remembered when the dral server is turned off.
    */
    void Cycle(UINT16 clockId, UINT64 cycle, UINT16 phase, bool persistent=false);

    /**
    * This method indicates that the items start moving trough the
    * graph (i.e. the end of the graph definition).
    * @brief Signals the end of the graph definition
    * @param First activity cycle of the clock domain 0 (first one created)
    */
    void StartActivity(UINT64 firstActivityCycle = 0);

    /**
    * Method used to know the maximum bandwidth used in a single cycle for
    * all the edges
    */
    void ComputeEdgeMaxBandwidth();

    /**
    * Returns the edge needed bandwidth so no bandwidth overflow occurs
    * during the simulation.
    * @brief Gets the edge bandwidth
    * @param edgeId requested edge
    */
    UINT32 GetEdgeMaxBandwidth(UINT16 edgeId);

private:

    void Init(UINT16 buffer_size, bool avoid_rep, bool compression);
    void DralEnterNode (UINT16 nodeId, UINT32 itemId, UINT16 dim,
         UINT32 position [], bool persistent);
    void DralExitNode (UINT16 nodeId, UINT32 itemId, UINT16 dim,
         UINT32 position [], bool persistent);
    UINT32 ComputeNodePosition(UINT16 nodeId, UINT16 dim, UINT32 position[]);
    void AutoFlush(UINT64 n);
    void DumpLiveItemIds();
    void UpdateEdgeMaxBandwidth();

    /*
     * Private counters. They are used when creating new items, new nodes,
     * new edges and new clock domains. The value of the counter is returned
     * and incremented.
     */
    UINT32 item_id;
    UINT16 node_id;
    UINT16 edge_id;
    UINT16 clock_id;


    bool nodetagAutocompress;
    Nodetagcache_map nodetagcache;

    /*
     * AutoFlush implementation, Julio Gago @ BSSAD, June 2004.
     *
     */

    /*
     * Keep a 'map' with a couple of details we need to remember for each
     * EnterNode() in case we need to automatically generate an ExitNode. The
     * map is indexed by the final position index in the node contents array.
     */
    typedef struct
    {
        UINT32 *position; /* position in the node layout */
        bool persistent;  /* whether the enternode was persistent */
    } enternode_data;
    typedef map <UINT32, enternode_data> enternode_map;

    /*
     * Keep a couple of 'vectors' with auto-flush information for each created
     * node
     */
    vector<bool> auto_flush;             /* whether 'autoflush' is set for a node */
    typedef struct
    {
        bool relaxed;                    /* relaxed-mode autoflush (allow explicit ExitNode) ? */
        UINT16 dimensions;               /* total dimensions in the node layout */
        UINT32 *capacity;                /* size of each dimension */
        UINT32 *contents;                /* array with contents (item ids) of the node */
        enternode_map current, previous; /* information about enternodes in current/previous clocks */
    } auto_flush_entry;	
    vector<auto_flush_entry> auto_flush_data;

    /*
     * We grow the two defined vectors in chunks of 'chunk_size' elements
     */
    static const int chunk_size = 2048;

    /*
     * We keep also a vector with the node ids created with autoflush set
     * (convenient for quick-iteration every clock)
     */
    vector<UINT16> auto_flush_nodes;

    /*
     * End of Autoflush implementation.
     *
     */


    /*
     * Container used to remember the live items while the dral server is turned
     * off. Once it is turned on, we will lookup this list in order to generate
     * the corresponding NewItem events. Just after the generation of these
     * events, the container will be cleared
     */
    liveItemsList liveItems;

    /*
     * A pointer to the implementation class
     */
    DRAL_SERVER_IMPLEMENTATION implementation;

    DRAL_STORAGE dralStorage;

    /*
     * boolean used to know if the writing to the file descriptor
     * is enabled or disabled
     */
    bool turnedOn;

    /*
     * boolean used to know if the the file descriptor must be closed
     * at the destructor
     */
    bool mustCloseFileDescriptor;
    
    /*
     * The file descriptor
     */
    int file_descriptor;
    
    /*
     * The size of the write buffer
     */
    UINT16 buff_size;

    /*
     * boolean used to know if the output file has been created
     * (i.e. the server has been turned On at least once)
     * Note: when using the constructor with a file descriptor as a first
     * parameter, the dral server assumes that the file has been already opened.
     */
    bool fileOpened;

    /*
     * The file name
     */
    string file_name;

    bool openedWithFileName;

    /*
     * This boolean makes the ascii server change the instance number of a
     * node if its name has been already used
     */
    bool avoid_node_reps;

    /*
     *  boolean used to know if the dral server has to automatically add the
     *  embeded tar file when the StartActivity method is invoked
     */
    bool embeded_tar_file;

    /*
     * Private method that returns the size and a pointer to the embeded tar
     * file
     */
    char * GetTar (UINT32 * size);

    /*
     * Are we computing the max bandwidth for edges
     */
    bool com_edge_bw;

    /*
     * Stores the maximum bandwidth for all the edges
     */
    UINT32 * max_edge_bw;

    /*
     * Actual clock bandwidth used per edge
     */
    map<UINT16, UINT32> edge_bw;

  protected:    
    Nodetagcache_tag_map* getNodeTagMap(const char tag_name[]);

  public:

    void SwitchToDebug(int fd, bool compression);

};
typedef DRAL_SERVER_CLASS * DRAL_SERVER;


 /** \example dralServerExample
  *  This is an example on a simple use of dralServer.
  */

#endif /* __dralServer_h */
