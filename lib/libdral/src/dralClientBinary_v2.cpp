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
 * @file dralClientBinary_v1.cpp
 * @author Pau Cabre
 * @brief dral client binary version 1 implementation
 */

#include "asim/dralClientBinary_v2.h"

/**
 * Constructor of the ascii specific implementation of the dral client
 * it only invokes the generic implementation constructor
 */
DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS
    ::DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS(
    DRAL_BUFFERED_READ dral_read,DRAL_LISTENER listener) :
    DRAL_CLIENT_IMPLEMENTATION_CLASS (dral_read,listener) {}

/**
 * This is the method that uses the buffered read class. It returns a pointer
 * to the read bytes.
 * Warning: n must be != 0 (it would return NULL)
 * It returns NULL if any error found
 */
void * DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::ReadBytes (UINT32 n)
{
    INT64 k;
    void * b;
    UINT32 n_8 = ((n%8)?(n+8-n%8):n);

    b=dralRead->Read(n_8,&k);
    if (k == 0)
    {
        return NULL;
    }
    else if (k == -1)
    {
        dralListener->Error("Error reading from the file descriptor");
        return NULL;
    }
    return b;
}


/**
 * This method will read from the file descriptor in order to process n
 * event by calling its specific method of the listener.
 * It can be either blocking or non-blocking according to the parameter.
 */
UINT16
DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::ProcessNextEvent (
    bool , UINT16 num_events)
{

   /*
    * Since this version gets the information from a file
    * the parameter blocking it is not used.
    */


    /*
     * buffer with the data read
     */
    void * buffer;
    
    bool r;

    struct commandFormat
    {
        UINT64 commandCode  : 6;
        UINT64 other        : 58;
    } * command;
    
    UINT16 n_events;

    if (num_events == 0 || errorFound)
    {
        return 0; // if there is an error this method must do nothing
    }

    for (n_events=0;n_events<num_events;n_events++)
    {
        buffer=ReadBytes(8);
        if (buffer == NULL)
        {
            dralListener->EndSimulation();
            errorFound=true;
            break;
        }

        command=(commandFormat *)buffer;
        
        switch (command->commandCode)
        {
          case DRAL2_CYCLE:
            r=Cycle(buffer);
            break;
          case DRAL2_NEWITEM:
            r=NewItem(buffer);
            break;
          case DRAL2_MOVEITEMS:
            r=MoveItems(buffer);
            break;
          case DRAL2_DELETEITEM:
            r=DeleteItem(buffer);
            break;
          case DRAL2_SETITEMTAG:
          case DRAL2_SETITEMTAG_STRING:
          case DRAL2_SETITEMTAG_SET:
            r=SetItemTag(buffer,command->commandCode);
            break;
          case DRAL2_ENTERNODE:
            r=EnterNode(buffer);
            break;
          case DRAL2_EXITNODE:
            r=ExitNode(buffer);
            break;
          case DRAL2_NEWNODE:
            r=NewNode(buffer);
            break;
          case DRAL2_NEWEDGE:
            r=NewEdge(buffer);
            break;
          case DRAL2_SETNODELAYOUT:
            r=SetNodeLayout(buffer);
            break;
          case DRAL2_COMMENT:
            r=Comment(buffer);
            break;
          case DRAL2_COMMENTBIN:
            r=CommentBin(buffer);
            break;
          case DRAL2_SETNODETAG:
          case DRAL2_SETNODETAG_STRING:
          case DRAL2_SETNODETAG_SET:
            r=SetNodeTag(buffer,command->commandCode);
            break;
          case DRAL2_SETCYCLETAG:
          case DRAL2_SETCYCLETAG_STRING:
          case DRAL2_SETCYCLETAG_SET:
            r=SetCycleTag(buffer,command->commandCode);
            break;
          case DRAL2_SETNODEINPUTBANDWIDTH:
            r=SetNodeInputBandwidth(buffer);
            break;
          case DRAL2_SETNODEOUTPUTBANDWIDTH:
            r=SetNodeOutputBandwidth(buffer);
            break;
          case DRAL2_STARTACTIVITY:
            r=StartActivity(buffer);
            break;
          case DRAL2_SETTAGDESCRIPTION:
            r=SetTagDescription(buffer);
            break;
          case DRAL2_SETNODECLOCK:
            r=SetNodeClock(buffer);
            break;
          case DRAL2_NEWCLOCK:
            r=NewClock(buffer);
            break;
          case DRAL2_CYCLEWITHCLOCK:
            r=CycleWithClock(buffer);
            break;
          default:
            r=Error(buffer);
            break;
        }

        if (!r)
        {
            dralListener->Error(
                "Error reading the events file. If it is compressed, "
                "it may have a CRC error");
            dralListener->EndSimulation();
            errorFound=true;
            break;
        }
    }
    return n_events;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::Error(void * )
{
    dralListener->Error(
        "Unknown command code\n"
        "Please report this bug to dral@bssad.intel.com attaching "
        "input files (if any)"
        );
    return false;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::Cycle (void * buffer)
{
    struct cycleFormat
    {
        UINT64 commandCode  : 6;
        UINT64 n            : 58;
    } * commandCycle;

    commandCycle=(cycleFormat *)buffer;

    dralListener->Cycle(commandCycle->n);
    
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::NewItem (void * buffer)
{
    struct newItemFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 26;
        UINT64 item_id      : 32;
    } * commandNewItem;

    commandNewItem=(newItemFormat *)buffer;

    dralListener->NewItem(commandNewItem->item_id);
    
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::MoveItems (void * buffer)
{
    struct moveItemsFormat
    {
        UINT64 commandCode          : 6;
        UINT64 reserved             : 4;
        UINT64 positionsPresent     : 1;
        UINT64 n                    : 5;
        UINT64 edge_id              : 16;
        UINT64 item_0_or_reserved2  : 32; // it will be item_0 if moving items
                                          // without positions and reserved
                                          // if moving intems with positions
    } * commandMoveItems;
    UINT16 n;
    UINT16 edge_id;
    UINT32 item_0_or_reserved2;

    UINT32 * temp=NULL; // =NULL to avoid warning
    UINT32 * items;
    bool mustDelete=false;

    commandMoveItems=(moveItemsFormat *)buffer;
    
    //the values must be saved because '*buffer' may change if full
    n=commandMoveItems->n;
    if (n == 0)
    {
        dralListener->NonCriticalError(
            "Found a move items command with n == 0");
        return true;
    }
    edge_id=commandMoveItems->edge_id;
    item_0_or_reserved2=commandMoveItems->item_0_or_reserved2;

    if (!(commandMoveItems->positionsPresent))
    {
        //MoveItems without positions

        if (n > 1)
        {
            items = (UINT32 *)ReadBytes((n-1)*sizeof(UINT32));
            if(items == NULL)
            {
                return false;
            }

            if (buffer == (items-2)) //has the buffer changed?
            {
                --items; // items[0] //No. So the data is reused
            }
            else
            {
                //Yes. So we need to copy all the array
                // i.e. to add items[0] at the begining.
                temp = new UINT32 [n];
                if (temp == NULL)
                {
                    dralListener->Error("Out of memory");
                    return false;
                }
                temp[0]=item_0_or_reserved2;
                memcpy(temp+1,items,(n-1)*sizeof(UINT32));
                items=temp;
                mustDelete=true;
            }
        }
        else
        {
            // only one item. We do not need to read more data
            items=(UINT32 *)((UINT32 *)buffer+1);
        }

        dralListener->MoveItems(edge_id,n,items);

        
        if (mustDelete)
        {
            //We had to copy the array, so now we free it
            delete [] temp;
        }
    }
    else
    {
        //MoveItems with positions
        
        //Reading items and positions, all in one
        items = (UINT32 *)ReadBytes (n*sizeof(UINT32)*2);
        if (items == NULL)
        {
            dralListener->EndSimulation();
            errorFound=true;
            return false;
        }

        UINT32 * positions = items+n;

        dralListener->MoveItemsWithPositions(edge_id,n,items,positions);
    }
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::DeleteItem (void * buffer)
{
    struct deleteItemFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 26;
        UINT64 item_id      : 32;
    } * commandDeleteItem;

    commandDeleteItem=(deleteItemFormat *)buffer;

    dralListener->DeleteItem(commandDeleteItem->item_id);
    
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetItemTag (
    void * buffer, UINT16 variant)
{
    char * buffer2;
    UINT64 * value;
    UINT16 tag_name_len;
    struct setItemTagFormat
    {
        UINT64 commandCode      : 6;
        UINT64 reserved         : 2;
        UINT64 tag_name_len     : 8;
        UINT64 n                : 16;
        UINT64 item_id          : 32;

    } * commandSetItemTag;
    UINT16 n;
    UINT32 item_id;
    
    commandSetItemTag=(setItemTagFormat *)buffer;

    //the values must be saved because '*buffer' may change if full
    n=commandSetItemTag->n;
    item_id=commandSetItemTag->item_id;
    tag_name_len=commandSetItemTag->tag_name_len;

    buffer2 = (char *) ReadBytes(tag_name_len);  // reading the tag name
    if (buffer2 == NULL)
    {
        return false;
    }

    UINT32 tag_idx = getTagIndex(buffer2, tag_name_len);
    UINT32 str_idx;

    switch (variant)
    {
      case DRAL2_SETITEMTAG:

        value = (UINT64 *)ReadBytes(sizeof(*value)); //the value
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetItemTag(item_id, tag_idx, * value);
        break;

      case DRAL2_SETITEMTAG_STRING:

        buffer2 = (char *) ReadBytes (n);  // read the string
        if (buffer2 == NULL)
        {
            return false;
        }
        str_idx = getStringIndex(buffer2, n);

        dralListener->SetItemTagString(item_id, tag_idx, str_idx);
        break;

      case DRAL2_SETITEMTAG_SET:

        value = (UINT64 *) ReadBytes(n*sizeof(UINT64)); //read the set
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetItemTagSet(item_id, tag_idx, n, value);
        break;

      default:
        dralListener->Error(
            "Unknown SetTag variant\n"
            "Please report this bug to dral@bssad.intel.com attaching "
            "input files (if any)"
            );
        return false;
    }

    return true;
}



bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::EnterNode (void * buffer)
{

    bool mustDelete = false;

    struct enterNodeFormat1
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 10;
        UINT64 node_id      : 16;
        UINT64 item_id      : 32;
    } * commandEnterNode1;

    struct enterNodeFormat2
    {
        UINT64 reserved2    : 16;
        UINT64 dimensions   : 16;
        UINT64 position_0   : 32;
    } * commandEnterNode2;
    
    UINT16 node_id;
    UINT32 item_id;
    UINT16 dimensions;
    UINT32 position_0;
    UINT32 * position = NULL;
    UINT32 * temp = NULL;

    commandEnterNode1=(enterNodeFormat1 *)buffer;

    node_id=commandEnterNode1->node_id;
    item_id=commandEnterNode1->item_id;

    // Read the dimensions and position_0
    commandEnterNode2=(enterNodeFormat2 *) ReadBytes(sizeof(enterNodeFormat2));
    if (commandEnterNode2 == NULL)
    {
        return false;
    }

    dimensions=commandEnterNode2->dimensions;
    position_0=commandEnterNode2->position_0;

    if (dimensions>1)
    {
        position = (UINT32 *)ReadBytes((dimensions-1)*sizeof(UINT32));
        if(position == NULL)
        {
            return false;
        }

        if ((UINT32 *)commandEnterNode2 == (position-2)) //has the buffer changed?
        {
            --position; // items[0] //No. So the data is reused
        }
        else
        {
            //Yes. So we need to copy all the array
            // i.e. to add items[0] at the begining.
            temp = new UINT32 [dimensions];
            if (temp == NULL)
            {
                dralListener->Error("Out of memory");
                return false;
            }
            temp[0]=position_0;
            memcpy(temp+1,position,(dimensions-1)*sizeof(UINT32));
            position=temp;
            mustDelete=true;
        }
    }
    else
    {
        position = (UINT32 *)commandEnterNode2 + 1;
    }

    dralListener->EnterNode(node_id,item_id,dimensions,position);
    if (mustDelete)
    {
        delete [] temp;
    }
    return true;
}


bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::ExitNode (void * buffer)
{

    bool mustDelete = false;

    struct exitNodeFormat1
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 10;
        UINT64 node_id      : 16;
        UINT64 item_id      : 32;
    } * commandExitNode1;

    struct exitNodeFormat2
    {
        UINT64 reserved2    : 16;
        UINT64 dimensions   : 16;
        UINT64 position_0   : 32;
    } * commandExitNode2;
    
    UINT16 node_id;
    UINT32 item_id;
    UINT16 dimensions;
    UINT32 position_0;
    UINT32 * position = NULL;
    UINT32 * temp = NULL;

    commandExitNode1=(exitNodeFormat1 *)buffer;

    //the values must be saved because '*buffer' may change if full    
    node_id=commandExitNode1->node_id;
    item_id=commandExitNode1->item_id;

    // Read the dimensions and position_0
    commandExitNode2=(exitNodeFormat2 *) ReadBytes(sizeof(exitNodeFormat2));
    if (commandExitNode2 == NULL)
    {
        return false;
    }
    //save the values
    dimensions=commandExitNode2->dimensions;
    position_0=commandExitNode2->position_0;

    if (dimensions>1)
    {
        position = (UINT32 *)ReadBytes((dimensions-1)*sizeof(UINT32));
        if(position == NULL)
        {
            return false;
        }

        if ((UINT32 *)commandExitNode2 == (position-2)) //has the buffer changed?
        {
            --position; // items[0] //No. So the data is reused
        }
        else
        {
            //Yes. So we need to copy all the array
            // i.e. to add items[0] at the begining.
            temp = new UINT32 [dimensions];
            if (temp == NULL)
            {
                dralListener->Error("Out of memory");
                return false;
            }
            temp[0]=position_0;
            memcpy(temp+1,position,(dimensions-1)*sizeof(UINT32));
            position=temp;
            mustDelete=true;
        }
    }
    else
    {
        position = (UINT32 *)commandExitNode2 + 1;
    }

    dralListener->ExitNode(node_id,item_id,dimensions,position);
    if (mustDelete)
    {
        delete [] temp;
    }
    return true;
}



bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::NewNode (void * buffer)
{
    char * buffer2;

    struct newNodeFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 2;
        UINT64 name_len     : 8;
        UINT64 node_id      : 16;
        UINT64 parent_id    : 16;
        UINT64 instance     : 16;
    } * commandNewNode;
    UINT16 node_id;
    UINT16 parent_id;
    UINT16 instance;

    commandNewNode=(newNodeFormat *)buffer;

    if (commandNewNode->name_len == 0)
    {
        dralListener->NonCriticalError(
            "Found a newNode command with node name length == 0");
        return true;
    }

    node_id=commandNewNode->node_id;
    parent_id=commandNewNode->parent_id;
    instance=commandNewNode->instance;

    //read the node name
    buffer2 = (char *) ReadBytes(commandNewNode->name_len);
    if (buffer2 == 0) 
    {
        return false;
    }

    dralListener->NewNode(node_id,buffer2,parent_id,instance);
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::NewEdge (void * buffer)
{
    char * buffer2;
    UINT32 * buffer3;
    struct newEdgeFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 2;
        UINT64 name_len     : 8;
        UINT64 edge_id      : 16;
        UINT64 src          : 16;
        UINT64 dest         : 16;
    } * commandNewEdge;
    UINT16 edge_id;
    UINT16 src;
    UINT16 dest;
    UINT32 bandwidth;
    UINT32 latency;
    UINT16 name_len;

    commandNewEdge=(newEdgeFormat *)buffer;

    if (commandNewEdge->name_len == 0)
    {
        dralListener->NonCriticalError(
            "Found a addedge command with edge name length == 0");
        return true;
    }

    edge_id=commandNewEdge->edge_id;
    src=commandNewEdge->src;
    dest=commandNewEdge->dest;
    name_len=commandNewEdge->name_len;

    buffer3 = (UINT32 *) ReadBytes(8); //read bandwidth and latency
    if (buffer3 == NULL)  
    {
        return false;
    }
    bandwidth=buffer3[0];
    latency=buffer3[1];

    buffer2 = (char *) ReadBytes(name_len);
    if (buffer2 == NULL)
    {
        return false;
    }

    dralListener->NewEdge(src,dest,edge_id,bandwidth,latency,buffer2);
    return true;
}


bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetNodeLayout (void * buffer)
{
    UINT32 capacity;
    UINT32 * capacities;
    struct setNodeLayoutFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 10;
        UINT64 node_id      : 16;
        UINT64 reserved2    : 16;
        UINT64 dimensions   : 16;
    } * commandSetNodeLayout;
    UINT16 node_id;
    UINT16 dimensions;

    capacity=1;

    commandSetNodeLayout=(setNodeLayoutFormat *)buffer;

    node_id=commandSetNodeLayout->node_id;
    dimensions=commandSetNodeLayout->dimensions;

    if (dimensions == 0)
    {
        dralListener->NonCriticalError(
            "Found a setcapacity command with 0 dimensions");
        return true;
    }

    capacities = (UINT32 *)ReadBytes(sizeof(UINT32)*dimensions);
    if (capacities == NULL)
    {
        return false;
    }

    for (UINT16 i=0; i<dimensions; i++)
    {
        capacity*=capacities[i];
    }

    dralListener->SetNodeLayout(node_id,capacity,dimensions,capacities);

    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::Comment (void * buffer)
{
    char * buffer2;
    UINT32 magic_num;
    UINT32 length;
    struct commentFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 10;
        UINT64 strlen       : 16;
        UINT64 magic_num    : 32;
    } * commandComment;

    commandComment=(commentFormat *)buffer;

    if (commandComment->strlen == 0)
    {
        dralListener->NonCriticalError(
            "Found a comment command with comment length == 0");
        return true;
    }

    magic_num=commandComment->magic_num;
    length=commandComment->strlen;
    buffer2 = (char *) ReadBytes(commandComment->strlen);
    if (buffer2 == NULL)
    {
        return false;
    }

    dralListener->Comment(magic_num,buffer2);

    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::CommentBin (void * buffer)
{
    char * buffer2;
    UINT16 magic_num;
    UINT32 length;
    struct commentBinFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 10;
        UINT64 magic_num    : 16;
        UINT64 length       : 32;
    } * commandCommentBin;

    commandCommentBin=(commentBinFormat *)buffer;

    if (commandCommentBin->length == 0)
    {
        dralListener->NonCriticalError(
            "Found a comment command with comment length == 0");
        return true;
    }

    magic_num=commandCommentBin->magic_num;
    length=commandCommentBin->length;
    buffer2 = (char *) ReadBytes(commandCommentBin->length);
    if (buffer2 == NULL)
    {
        return false;
    }

    dralListener->CommentBin(magic_num,buffer2,length);

    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetNodeTag (
    void * buffer, UINT16 variant)
{
    char * buffer2;
    char * tag_name;
    UINT32 * list;
    UINT32 * buffer3 = NULL; // "= NULL" to avoid warning message
    UINT64 * value;
    UINT16 tag_name_len;
    UINT16 n;
    UINT16 node_id;
    UINT16 levels;
    bool mustDeleteList = false;

    struct setNodeTagFormat
    {
        UINT64 commandCode      : 6;
        UINT64 reserved         : 2;
        UINT64 tag_name_len     : 8;
        UINT64 n                : 16;
        UINT64 node_id          : 16;
        UINT64 levels           : 16;

    } * commandSetNodeTag;
    
    commandSetNodeTag=(setNodeTagFormat *)buffer;

    n=commandSetNodeTag->n;
    node_id=commandSetNodeTag->node_id;
    tag_name_len=commandSetNodeTag->tag_name_len;
    levels=commandSetNodeTag->levels;

    buffer2 = (char *) ReadBytes(tag_name_len);  // reading the tag name
    if (buffer2 == NULL)
    {
        return false;
    }

    UINT32 tag_idx = getTagIndex(buffer2, tag_name_len);
    UINT32 str_idx;

    if (levels)
    {
        buffer3 = (UINT32 *) ReadBytes(levels*sizeof(UINT32)); // read the list
        if (buffer3 == NULL)
        {
            return false;
        }
        list = buffer3;
    }
    else
    {
        list=NULL;
    }

    switch (variant)
    {
      case DRAL2_SETNODETAG:

        if (dralRead->AvailableBytes() < sizeof(UINT64))
        {
            if (levels)
            {
                list = new UINT32 [levels];
                memcpy(list,buffer3,levels*sizeof(UINT32));
                mustDeleteList=true;
            }
        }

        value = (UINT64 *)ReadBytes(sizeof(*value)); //the value
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetNodeTag(node_id, tag_idx, * value, levels, list);
        break;

      case DRAL2_SETNODETAG_STRING:

        if (dralRead->AvailableBytes() < n)
        {
            if (levels)
            {
                list = new UINT32 [levels];
                memcpy(list,buffer3,levels*sizeof(UINT32));
                mustDeleteList=true;
            }
        }

        buffer2 = (char *) ReadBytes (n);  // read the string
        if (buffer2 == NULL)
        {
            return false;
        }

        str_idx = getStringIndex(buffer2, n);
        dralListener->SetNodeTagString(node_id, tag_idx, str_idx, levels, list);
        break;

      case DRAL2_SETNODETAG_SET:

        if (dralRead->AvailableBytes() < n*sizeof(UINT64))
        {
            if (levels)
            {
                list = new UINT32 [levels];
                memcpy(list,buffer3,levels*sizeof(UINT32));
                mustDeleteList=true;
            }
        }

        value = (UINT64 *) ReadBytes(n*sizeof(UINT64)); //read the set
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetNodeTagSet(node_id, tag_idx, n, value, levels, list);
        break;

      default:
        dralListener->Error(
            "Unknown SetTag variant\n"
            "Please report this bug to dral@bssad.intel.com attaching "
            "input files (if any)"
            );
        return false;
    }
    
    if (mustDeleteList)
    {
        delete [] list;
    }
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetCycleTag (
    void * buffer, UINT16 variant)
{
    char * tag_name;
    char * buffer2;
    UINT64 * value;
    UINT16 tag_name_len;
    UINT16 n;

    struct setCycleTagFormat
    {
        UINT64 commandCode      : 6;
        UINT64 reserved         : 2;
        UINT64 tag_name_len     : 8;
        UINT64 n                : 16;
        UINT64 reserved2        : 32;
    } * commandSetCycleTag;
    
    commandSetCycleTag=(setCycleTagFormat *)buffer;

    n=commandSetCycleTag->n;
    tag_name_len=commandSetCycleTag->tag_name_len;

    buffer2 = (char *) ReadBytes(tag_name_len);  // reading the tag name
    if (buffer2 == NULL)
    {
        return false;
    }

    UINT32 tag_idx = getTagIndex(buffer2, tag_name_len);
    UINT32 str_idx;

    switch (variant)
    {
      case DRAL2_SETCYCLETAG:

        value = (UINT64 *)ReadBytes(sizeof(*value)); //the value
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetCycleTag(tag_idx, * value);
        break;

      case DRAL2_SETCYCLETAG_STRING:

        buffer2 = (char *) ReadBytes (n);  // read the string
        if (buffer2 == NULL)
        {
            return false;
        }
        str_idx = getStringIndex(buffer2, n);
        dralListener->SetCycleTagString(tag_idx, str_idx);
        break;

      case DRAL2_SETCYCLETAG_SET:

        value = (UINT64 *) ReadBytes(n*sizeof(UINT64)); //read the set
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetCycleTagSet(tag_idx, n, value);
        break;

      default:
        dralListener->Error(
            "Unknown SetTag variant\n"
            "Please report this bug to dral@bssad.intel.com attaching "
            "input files (if any)"
            );
        return false;
    }
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetNodeInputBandwidth (
    void * buffer)
{
    struct setNodeInputBandwidthFormat
    {
        UINT64 commandCode      : 6;
        UINT64 reserved         : 10;
        UINT64 node_id          : 16;
        UINT64 bandwidth        : 32;
    } * commandSetNodeInputBandwidth;

    commandSetNodeInputBandwidth=(setNodeInputBandwidthFormat *)buffer;
    
    dralListener->SetNodeInputBandwidth(
        commandSetNodeInputBandwidth->node_id,
        commandSetNodeInputBandwidth->bandwidth);
    
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetNodeOutputBandwidth (
    void * buffer)
{
    struct setNodeOutputBandwidthFormat
    {
        UINT64 commandCode      : 6;
        UINT64 reserved         : 10;
        UINT64 node_id          : 16;
        UINT64 bandwidth        : 32;
    } * commandSetNodeOutputBandwidth;

    commandSetNodeOutputBandwidth=(setNodeOutputBandwidthFormat *)buffer;
    
    dralListener->SetNodeOutputBandwidth(
        commandSetNodeOutputBandwidth->node_id,
        commandSetNodeOutputBandwidth->bandwidth);
    
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::StartActivity (void * buffer)
{
    struct startActivityFormat
    {
        UINT64 commandCode      : 6;
        UINT64 firstActCycle    : 58;
    } * commandStartActivity;
    
    commandStartActivity=(startActivityFormat *)buffer;
    
    dralListener->StartActivity(commandStartActivity->firstActCycle);

    return true;
}


bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetTagDescription(
    void * buffer)
{
    char * buffer2;
    UINT16 tag_len;
    UINT16 desc_len;

    struct setTagDescriptionFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 26;
        UINT64 taglen       : 16;
        UINT64 desclen      : 16;
    } * commandSetTagDescription;
    
    commandSetTagDescription=(setTagDescriptionFormat *)buffer;

    tag_len=commandSetTagDescription->taglen;
    desc_len=commandSetTagDescription->desclen;
    buffer2 = (char *) ReadBytes(tag_len);
    if (buffer2 == NULL)
    {
        return false;
    }

    UINT32 tag_idx = getTagIndex(buffer2, tag_len);

    buffer2 = (char *) ReadBytes(desc_len);
    if (buffer2 == NULL)
    {
        return false;
    }

    dralListener->SetTagDescription(tag_idx, buffer2);

    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::SetNodeClock(
    void * buffer)
{
    struct setNodeClockFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 26;
        UINT64 nodeId       : 16;
        UINT64 clockId      : 16;
    } * commandSetNodeClockFormat;

    commandSetNodeClockFormat = (setNodeClockFormat *)buffer;

    dralListener->SetNodeClock(
        commandSetNodeClockFormat->nodeId,
        commandSetNodeClockFormat->clockId);
        
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::NewClock(
    void * buffer)
{
    UINT16 clockId;
    UINT16 skew;
    UINT64 freq;
    UINT16 nameLen;
    char * buffer2;

    struct newClockFormat
    {
        UINT64 commandCode  : 6;
        UINT64 reserved     : 10;
        UINT64 clockId      : 16;
        UINT64 skew         : 16;
        UINT64 nameLen      : 16;
    } * commandNewClockFormat;

    commandNewClockFormat = (newClockFormat *)buffer;
    clockId = commandNewClockFormat->clockId;
    skew = commandNewClockFormat->skew;
    nameLen = commandNewClockFormat->nameLen;

    buffer2 = (char *)ReadBytes(sizeof(UINT64));
    if (!buffer2)
    {
        return false;
    }
    freq = *(UINT64 *)buffer2;
    buffer2 = (char *)ReadBytes(nameLen);
    if (!buffer2)
    {
        return false;
    }
    dralListener->NewClock(clockId,freq,skew,1 /* divisions */,buffer2);
    return true;
}

bool DRAL_CLIENT_BINARY_2_IMPLEMENTATION_CLASS::CycleWithClock(
    void * buffer)
{
    struct cycleFormat
    {
        UINT64 commandCode  : 6;
        UINT64 n            : 42;
        UINT64 clockId      : 16;
    } * commandCycleFormat;

    commandCycleFormat = (cycleFormat *)buffer;
    
    dralListener->Cycle(
        commandCycleFormat->clockId,
        commandCycleFormat->n, 0 /* phase */);

    return true;
}
