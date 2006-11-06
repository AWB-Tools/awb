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
 * @file dralClientBinary_v0.cpp
 * @author Pau Cabre
 * @brief dral client binary version 0 implementation
 */

#include "asim/dralClientImplementation.h"
#include "asim/dralClientBinary_v0_1.h"

DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS
    ::DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS(
    DRAL_BUFFERED_READ dral_read,DRAL_LISTENER listener) :
    DRAL_CLIENT_BINARY_1_IMPLEMENTATION_CLASS (dral_read,listener) {}


bool DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS::MoveItems (void * buffer)
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
    UINT32 * temp2=NULL;
    UINT32 * items;
    bool mustDelete=false;
    bool mustDelete2=false;

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
            temp2 = (UINT32 *)ReadBytes((n-1+roundUp)*sizeof(UINT32));
            if(temp2 == NULL)
            {
                return false;
            }

            if (buffer == (temp2-2)) //has the buffer changed?
            {
                temp2 -= 1; // items[0] //No. So the data is reused
            }
            else
            {
                //Yes. So we need to copy all the array
                temp = new UINT32 [n-1+roundUp];
                mustDelete=true;
                temp[0]=item_0_or_reserved2;
                for (UINT16 j=1; j<(n-1+roundUp); j++)
                {
                    temp[j]=temp2[j-1];
                }
                temp2=temp;
            }

            items = new UINT32 [n-1+roundUp];
            mustDelete2=true;

            items[0]=temp2[0];

            for(UINT16 i=1;i<n;i+=2)
            {
                if ((i+1)<n)
                {
                    memcpy(
                        &items[i+1],
                        &temp2[i],
                        sizeof(UINT32));
                }
                memcpy(&items[i],&temp2[i+1],sizeof(UINT32));
            }            

        }
        else
        {
            // only one item. We do not need to read more data
            items=(UINT32 *)((UINT32 *)buffer+1);
        }

        dralListener->MoveItems(edge_id,n,items);

        
        if (mustDelete2)
        {
            delete [] temp2;
            if (mustDelete)
            {
                delete [] temp;
            }
        }

    }
    else
    {
        //MoveItems with positions
        
        //Reading items and positions, all in one
        temp = (UINT32 *)ReadBytes (n*sizeof(UINT32)*2);
        if (temp == NULL)
        {
            dralListener->EndSimulation();
            errorFound=true;
            return false;
        }

        items = new UINT32 [n];
        UINT32 * positions = new UINT32 [n];

        for (UINT16 i=0;i<n;i++)
        {
            items[i]=temp[2*i];
            positions[i]=temp[2*i+1];
        }
        dralListener->MoveItemsWithPositions(edge_id,n,items,positions);
        delete [] items;
        delete [] positions;
    }
    return true;
}


bool DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS::SetTag (void * buffer)
{
    char tag_name [9];
    UINT32 size;
    char * buffer2;
    char * str;
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
        dralListener->SetTagSingleValue(item_id, tag_idx,*value,flags);
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
        
        str = new char [size+1]; // +1 for the '\0'
        
        strncpy(str,buffer2,n);
        str[n]='\0';

        str_idx = getStringIndex(str, strlen(str));

        dralListener->SetTagString(item_id, tag_idx, str_idx, flags);
        
        delete [] str;
        
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


bool DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS::AddNode (void * buffer)
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

    char * str = new char [size+1];
    strncpy(str,buffer2,size);
    str[size]='\0';

    dralListener->AddNode(node_id,str,parent_id,instance);
    delete [] str;
    return true;
}

bool DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS::AddEdge (void * buffer)
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
    char * str = new char [size+1];
    strncpy(str,buffer2,size);
    str[size]='\0';

    dralListener->AddEdge(src,dest,edge_id,bandwidth,latency,str);
    delete [] str;
    return true;
}

bool DRAL_CLIENT_BINARY_0_IMPLEMENTATION_CLASS::Comment (void * buffer)
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
    char * str = new char [size+1];
    strncpy(str,buffer2,size);
    str[size]='\0';

    dralListener->Comment(str);
    delete [] str;

    return true;
}
