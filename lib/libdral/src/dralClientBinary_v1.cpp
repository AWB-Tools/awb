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

#include "asim/dralClientBinary_v0_1.h"

/**
 * Constructor of the ascii specific implementation of the dral client
 * it only invokes the generic implementation constructor
 */
DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS
    ::DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS(
    DRAL_BUFFERED_READ dral_read,DRAL_LISTENER listener) :
    DRAL_CLIENT_IMPLEMENTATION_CLASS (dral_read,listener),
    firstCycle(true) {}

/**
 * This is the method that uses the buffered read class. It returns a pointer
 * to the read bytes.
 * Warning: n must be != 0 (it would return NULL)
 * It returns NULL if any error found
 */
void * DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::ReadBytes (UINT32 n)
{
    INT64 k;
    void * b;

    b=dralRead->Read(n,&k);
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
DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::ProcessNextEvent (
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
        UINT64 commandCode : 4;
        UINT64 other : 60;
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
          case DRAL0_CYCLE:
            r=Cycle(buffer);
            break;
          case DRAL0_NEWITEM:
            r=NewItem(buffer);
            break;
          case DRAL0_MOVEITEMS:
            r=MoveItems(buffer);
            break;
          case DRAL0_DELETEITEM:
            r=DeleteItem(buffer);
            break;
          case DRAL0_SETTAG:
            r=SetTag(buffer);
            break;
          case DRAL0_ENTERNODE:
            r=EnterNode(buffer);
            break;
          case DRAL0_EXITNODE:
            r=ExitNode(buffer);
            break;
          case DRAL0_ADDNODE:
            r=AddNode(buffer);
            break;
          case DRAL0_ADDEDGE:
            r=AddEdge(buffer);
            break;
          case DRAL0_SETCAPACITY:
            r=SetCapacity(buffer);
            break;
          case DRAL0_SETHIGHWATERMARK:
            r=SetHighWaterMark(buffer);
            break;
          case DRAL0_COMMENT:
            r=Comment(buffer);
            break;
          default:
            r=Error(buffer);
            break;
        }

        if (!r)
        {
            dralListener->EndSimulation();
            errorFound=true;
            break;
        }
    }
    return n_events;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::Error(void * )
{
    dralListener->Error(
        "Unknown command code\n"
        "Please report this bug to dral@bssad.intel.com attaching "
        "input files (if any)"
        );
    return false;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::Cycle (void * buffer)
{
    struct cycleFormat
    {
        UINT64 commandCode  : 4;
        UINT64 n            : 60;
    } * commandCycle;

    commandCycle=(cycleFormat *)buffer;

    if (firstCycle)
    {
        dralListener->StartActivity(commandCycle->n);
        firstCycle=false;
    }

    dralListener->Cycle(commandCycle->n);
    
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::NewItem (void * buffer)
{
    struct newItemFormat
    {
        UINT64 commandCode  : 4;
        UINT64 reserved     : 28;
        UINT64 item_id      : 32;
    } * commandNewItem;

    commandNewItem=(newItemFormat *)buffer;

    dralListener->NewItem(commandNewItem->item_id);
    
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::MoveItems (void * buffer)
{
    struct moveItemsFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 6;
        UINT64 positionsPresent : 1;
        UINT64 n : 5;
        UINT64 edge_id : 16;
        UINT64 item_0_or_reserved2: 32; // it will be item_0 if moving items
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
        UINT16 roundUp;

        if (n % 2)
        {
            roundUp=0;
        }
        else
        {
            roundUp=1;
        }

        if (n > 1)
        {
            items = (UINT32 *)ReadBytes((n-1+roundUp)*sizeof(UINT32));
            if(items == NULL)
            {
                return false;
            }

            if (buffer == (items-2)) //has the buffer changed?
            {
                items -= 1; // items[0] //No. So the data is reused
            }
            else
            {
                //Yes. So we need to copy all the array
                temp = new UINT32 [n-1+roundUp];
                temp[0]=item_0_or_reserved2;
                for (UINT16 j=1; j<(n-1+roundUp); j++)
                {
                    temp[j]=items[j-1];
                }
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

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::DeleteItem (void * buffer)
{
    struct deleteItemFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 28;
        UINT64 item_id : 32;
    } * commandDeleteItem;

    commandDeleteItem=(deleteItemFormat *)buffer;

    dralListener->DeleteItem(commandDeleteItem->item_id);
    
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::SetTag (void * buffer)
{
    char tag_name [9];
    UINT32 size;
    char * buffer2;
    UINT64 * value;
    struct setTagFormat
    {
        UINT64 commandCode : 4;
        UINT64 variant : 2;
        UINT64 flags : 4;
        UINT64 reserved : 6;
        UINT64 n : 16;
        UINT64 item_id : 32;
        UINT64 tag_name : 64;
    } * commandSetTag;
    UINT32 n;
    UINT32 item_id;
    UINT16 variant;
    UBYTE flags;
    
    commandSetTag=(setTagFormat *)buffer;

    //the values must be saved because '*buffer' may change if full
    n=commandSetTag->n;
    item_id=commandSetTag->item_id;
    variant=commandSetTag->variant;
    flags=commandSetTag->flags;

    buffer2 = (char *) ReadBytes(8);  // reading the tag name
    if (buffer2 == NULL)
    {
        return false;
    }

    memcpy(tag_name,buffer2,8);//copy the tag name, because '*buffer' may change
    tag_name[8]='\0';

    UINT32 tag_idx = getTagIndex(tag_name, strlen(tag_name));
    UINT32 str_idx;

    switch (variant)
    {
      case DRAL0_SINGLEVALUE:

        value = (UINT64 *)ReadBytes(sizeof(*value)); //the value
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetTagSingleValue(item_id, tag_idx, * value, flags);
        break;

      case DRAL0_STRING:

        if (n % 8)
        {
            size=n+8-(n % 8);
        }
        else
        {
            size=n;
        }

        buffer2 = (char *) ReadBytes (size);  // read the string
        if (buffer2 == NULL)
        {
            return false;
        }

        str_idx = getStringIndex(buffer2, strlen(buffer2));
        dralListener->SetTagString(item_id, tag_idx, str_idx, flags);
        break;

      case DRAL0_SET:

        value = (UINT64 *) ReadBytes(n*sizeof(UINT64)); //read the set
        if (value == NULL)
        {
            return false;
        }
        dralListener->SetTagSet(item_id, tag_idx, n, value, flags);
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

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::EnterNode (void * buffer)
{
    UINT32 * buffer2;
    struct enterNodeFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 12;
        UINT64 node_id : 16;
        UINT64 item_id : 32;
    } * commandEnterNode;
    UINT16 node_id;
    UINT32 item_id;

    commandEnterNode=(enterNodeFormat *)buffer;

    //the values must be saved because '*buffer' may change if full    
    node_id=commandEnterNode->node_id;
    item_id=commandEnterNode->item_id;

    buffer2 = (UINT32 *) ReadBytes(8);  // read the slot
    if (buffer2 == NULL)
    {
        return false;
    }
    
    dralListener->EnterNode(node_id,item_id,buffer2[1]);
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::ExitNode (void * buffer)
{
    struct exitNodeFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 12;
        UINT64 node_id : 16;
        UINT64 slot : 32;
    } * commandExitNode;

    commandExitNode=(exitNodeFormat *)buffer;

    dralListener->ExitNode(
            commandExitNode->node_id,
            commandExitNode->slot);
    
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::AddNode (void * buffer)
{
    char * buffer2;
    UINT32 size;
    struct addNodeFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 4;
        UINT64 name_len : 8;
        UINT64 node_id : 16;
        UINT64 parent_id: 16;
        UINT64 instance : 16;
    } * commandAddNode;
    UINT16 node_id;
    UINT16 parent_id;
    UINT16 instance;

    commandAddNode=(addNodeFormat *)buffer;

    if (commandAddNode->name_len == 0)
    {
        dralListener->NonCriticalError(
            "Found a addnode command with node name length == 0");
        return true;
    }

    //the values must be saved because '*buffer' may change if full    
    node_id=commandAddNode->node_id;
    parent_id=commandAddNode->parent_id;
    instance=commandAddNode->instance;

    if ( commandAddNode->name_len % 8 )
    {
        size=commandAddNode->name_len + 8 - (commandAddNode->name_len % 8);
    }
    else
    {
        size=commandAddNode->name_len;
    }

    buffer2 = (char *) ReadBytes(size); // read the node name
    if (buffer2 == 0) 
    {
        return false;
    }

    dralListener->AddNode(node_id,buffer2,parent_id,instance);
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::AddEdge (void * buffer)
{
    char * buffer2;
    UINT32 size;
    UINT32 * buffer3;
    struct addEdgeFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 4;
        UINT64 name_len : 8;
        UINT64 edge_id : 16;
        UINT64 src : 16;
        UINT64 dest : 16;
    } * commandAddEdge;
    UINT16 edge_id;
    UINT16 src;
    UINT16 dest;
    UINT32 bandwidth;
    UINT32 latency;

    commandAddEdge=(addEdgeFormat *)buffer;

    if (commandAddEdge->name_len == 0)
    {
        dralListener->NonCriticalError(
            "Found a addedge command with edge name length == 0");
        return true;
    }

    //the values must be saved because '*buffer' may change if full    
    edge_id=commandAddEdge->edge_id;
    src=commandAddEdge->src;
    dest=commandAddEdge->dest;

    if (commandAddEdge->name_len % 8)
    {
        size=commandAddEdge->name_len + 8 - (commandAddEdge->name_len % 8);
    }
    else
    {
        size=commandAddEdge->name_len;
    }

    buffer3 = (UINT32 *) ReadBytes(8); //read bandwidth and latency
    if (buffer3 == NULL)  
    {
        return false;
    }
    bandwidth=buffer3[0];
    latency=buffer3[1];

    buffer2 = (char *) ReadBytes(size);
    if (buffer2 == NULL)
    {
        return false;
    }

    dralListener->AddEdge(src,dest,edge_id,bandwidth,latency,buffer2);
    return true;
}


bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::SetCapacity (void * buffer)
{
    UINT32 capacity;
    UINT32 * capacities;
    UINT32 size;
    struct setCapacityFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 12;
        UINT64 node_id : 16;
        UINT64 reserved2 : 16;
        UINT64 dimensions : 16;
    } * commandSetCapacity;
    UINT16 node_id;
    UINT16 dimensions;

    capacity=1;

    commandSetCapacity=(setCapacityFormat *)buffer;

    //the values must be saved because '*buffer' may change if full    
    node_id=commandSetCapacity->node_id;
    dimensions=commandSetCapacity->dimensions;

    if (dimensions == 0)
    {
        dralListener->NonCriticalError(
            "Found a setcapacity command with 0 dimensions");
        return true;
    }

    size = dimensions;
    if ((commandSetCapacity->dimensions % 2) != 0)
    {
        size++;  // to read the zeros
    }

    capacities = (UINT32 *)ReadBytes(sizeof(UINT32)*size);
    if (capacities == NULL)
    {
        return false;
    }

    for (UINT16 i=0; i<dimensions; i++)
    {
        capacity*=capacities[i];
    }

    dralListener->SetCapacity(node_id,capacity,capacities,dimensions);

    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::SetHighWaterMark (void * buffer)
{
    struct setHighWaterMarkFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 12;
        UINT64 node_id : 16;
        UINT64 mark : 32;
    } * commandSetHighWaterMark;

    commandSetHighWaterMark=(setHighWaterMarkFormat *)buffer;

    dralListener->SetHighWaterMark(
        commandSetHighWaterMark->node_id,commandSetHighWaterMark->mark);
    
    return true;
}

bool DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS::Comment (void * buffer)
{
    char * buffer2;
    UINT32 size;
    struct commentFormat
    {
        UINT64 commandCode : 4;
        UINT64 reserved : 12;
        UINT64 strlen : 16;
        UINT64 reserved2 : 32;
    } * commandComment;

    commandComment=(commentFormat *)buffer;

    if (commandComment->strlen == 0)
    {
        dralListener->NonCriticalError(
            "Found a comment command with comment length == 0");
        return true;
    }

    if (commandComment->strlen % 8)
    {
        size=commandComment->strlen + 8 - (commandComment->strlen % 8);
    }
    else
    {
        size=commandComment->strlen;
    }

    buffer2 = (char *) ReadBytes(size);
    if (buffer2 == NULL)
    {
        return false;
    }

    dralListener->Comment(buffer2);

    return true;
}
