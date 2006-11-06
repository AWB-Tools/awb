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
 * @file dralClientAscii.cpp
 * @author Pau Cabre 
 * @brief dral client ascii implementation
 */


#include "asim/dralClientAscii_v0_1.h"

/**
 * Constructor of the ascii specific implementation of the dral client
 * it only invokes the generic implementation constructor
 */
DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS(
    DRAL_BUFFERED_READ dral_read,DRAL_LISTENER listener) :
    DRAL_CLIENT_IMPLEMENTATION_CLASS (dral_read,listener), firstCycle(false) {}


/**
 * Private method used to fill the buffer with a line read
 * from the file descriptor
 * All the lines with a command have to end with a '\n' character
 * It returns false if any error found
 */
bool
DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::Get_line (
    char ** buffer, bool * end_of_file)
{
    char * c;
    INT64 r;
    ostringstream sout;

    c=(char *)dralRead->Read(1,&r);
    while (c!= NULL && *c!='\n')
    {
        sout << *c;
        c=(char *)dralRead->Read(1,&r);
    }

    if (r == 0)
    {
        *end_of_file=true;
    }
    else if (r == -1)
    {
        dralListener->Error("Error reading the file descriptor");
        return false;
    }

    sout << ends;
    *buffer=(char *)sout.str().data();
    return true;
}


/**
 * Private method used to convert the time_span string
 * format from the string format to the unsigned byte format
 */
unsigned char
DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::TimeSpanStrToByte(
    char * s)
{
    char * buffer[3];
    int i;
    unsigned char u=0;
    char * c="|";  // used as delimiter when calling strtok
    i=0;
    if ((buffer[0]=strtok(s,c)))
    {
        i++;
        if ((buffer[1]=strtok((char *)NULL,c)))
        {
            i++;
            if ((buffer[2]=strtok((char *)NULL,c)))
            {
                i++;
            }
        }
    }

    for (int j=0;j<i;j++)
    {
        if (!strcmp(buffer[j],"FLG_IMMUTABLE"))
        {
            u|=DRAL_FLG_IMMUTABLE;
        }
        else if (!strcmp(buffer[j],"FLG_PAST"))
        {
            u|=DRAL_FLG_PAST;
        }
        else if (!strcmp(buffer[j],"FLG_CURRENT"))
        {
            u|=DRAL_FLG_CURRENT;
        }
        else if (!strcmp(buffer[j],"FLG_FUTURE"))
        {
            u|=DRAL_FLG_FUTURE;
        }
    }
    return u;
}

/**
 * This private method  counts how many times the character c is
 * found in the string.
 * It is used to know how many elements are in a command
 */
UINT16
DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::HowManyElements(
    char * buffer, char c)
{
    UINT16 i=0;
    UINT16 set_len=0;

    while (buffer[i] != '\n' && buffer[i] != '\0')
    {
        if (buffer[i] == c)
        {
            set_len++;
        }
        i++;
    }
    return set_len;
}


/**
 * This method will read from the file descriptor in order to process a
 * event by calling its specific method of the listener.
 * It can be either blocking or non-blocking according to the parameter.
 */
UINT16
DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::ProcessNextEvent (
    bool , UINT16 n)
{

   /*
    * Since this version gets the information from a file
    * the parameter blocking it is not used.
    */

    char * buffer;     // it will point to a whole line of the file
    char command[20];  // This array will contain the command name. Since the
                       // largest one is "setTagNodeSingleValue", the array
                       // size is 25

    UINT16 i;
    for (i=0; i<n; i++)
    {
        bool end_of_file=false;

        if (errorFound)
        {
            return i;
        }

        if (!Get_line(&buffer,&end_of_file))
        {
            dralListener->EndSimulation();
            errorFound=true;
            return 0;
        }

        istringstream stream(buffer);

        if (end_of_file)
        {
            dralListener->EndSimulation();
            return 0;
        }

        stream.get(command,sizeof(command),' ');

        if (!strcmp(command,"cycle"))
        {
            UINT64 n;
            stream >> n;
            if (firstCycle)
            {
                dralListener->StartActivity(n);
                firstCycle=false;
            }
            dralListener->Cycle(n);
        }
        else if (!strcmp(command,"newitem"))
        {
            UINT32 item_id;
            stream >> item_id;
            dralListener->NewItem(item_id);
        }
        else if (!strcmp(command,"settagSingleValue"))
        {
            SetTagSingleValue(&stream);
        }
        else if (!strcmp(command,"settagString"))
        {
            SetTagString(&stream);
        }
        else if (!strcmp(command,"settagSet"))
        {
            SetTagSet(&stream,buffer);
        }
        else if (!strcmp(command,"moveitems"))
        {
            MoveItems(&stream,buffer);
        }
        else if (!strcmp(command,"enternode"))
        {
            UINT16 node_id;
            UINT32 item_id;
            UINT32 slot_index;

            stream.ignore(5); // ignore " node"
            stream >> node_id;
            stream.ignore(5); // ignore " item"
            stream >> item_id;
            stream.ignore(5); // ignore " slot"
            stream >> slot_index;

            dralListener->EnterNode(node_id,item_id,slot_index);
        }
        else if (!strcmp(command,"exitnode"))
        {
            UINT16 node_id;
            UINT32 slot_index;

            stream.ignore(5); // ignore " node"
            stream >> node_id;
            stream.ignore(5); // ignore " slot"
            stream >> slot_index;

            dralListener->ExitNode(node_id,slot_index);
        }
        else if (!strcmp(command,"deleteitem"))
        {
            UINT32 item_id;

            stream >> item_id;
            dralListener->DeleteItem(item_id);
        }
        else if (!strcmp(command,"node"))
        {
            UINT16 node_id;
            UINT16 parent_id;
            UINT16 instance;
            stringbuf nodeName;

            stream.ignore(1); //ignore the white space
            stream.get(nodeName,' ');
            nodeName.sputc('\0'); // it adds the null terminator to the node name
            stream.ignore(3); // ignore " id"
            stream >> node_id;
            stream.ignore(7); // ignore " parent"
            stream >> parent_id;
            stream.ignore(9); // ignore " instance"
            stream >> instance;

            dralListener->AddNode(
                node_id,(char *)nodeName.str().data(),parent_id,instance);

        }
        else if (!strcmp(command,"edge"))
        {
            UINT16 src_node;
            UINT16 dest_node;
            UINT16 edge_id;
            UINT32 bandwidth;
            UINT32 latency;
            stringbuf name;

            stream.ignore(1); //ignore the white space
            stream.get(name,' ');
            name.sputc('\0'); // it adds the null terminator to the edge name
            stream.ignore(10); // ignore " from node"
            stream >> src_node;
            stream.ignore(8); // ignore " to node"
            stream >> dest_node;
            stream.ignore(3); // ignore " id"
            stream >> edge_id;
            stream.ignore(3); // ignore " bw"
            stream >> bandwidth;
            stream.ignore(4); // ignore " lat"
            stream >> latency;

            dralListener->AddEdge(
                src_node,dest_node,edge_id,bandwidth,
                latency,(char *)name.str().data());

        }
        else if (!strcmp(command,"setcapacity"))
        {
            SetCapacity(&stream,buffer);
        }
        else if (!strcmp(command,"sethighwatermark"))
        {
            UINT16 node_id;
            UINT32 mark;

            stream.ignore(5); // ignore " node"
            stream >> node_id;
            stream.ignore(5); // ignore " mark"
            stream >> mark;

            dralListener->SetHighWaterMark(node_id,mark);
        }
        else if (!strcmp(command,"comment:"))
        {
            stringbuf comment;
            stream.ignore(1); // ignore white space
            stream.get(comment);
            comment.sputc('\0'); // it adds the null terminator to the node name

            dralListener->Comment((char *)comment.str().data());

        }
        else
        {
            dralListener->Error(
                "Unknown command\n"
                "Please report this bug to dral@bssad.intel.com attaching "
                "input files (if any)"
                );
            dralListener->EndSimulation();
            errorFound=true;
            return false;
        }
    }
    return i;
}


void DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::SetTagSingleValue(
    istringstream * stream)
{
    char tag_name[9]="        ";
    char tagName[9];
    char timeSpanFlags[50];

    UINT32 item_id;
    UINT64 value;
    unsigned char time_span;

    *stream >> item_id;
    (*stream).ignore(1); // ignore the white space

    (*stream).get(timeSpanFlags,sizeof(timeSpanFlags),' ');
    (*stream).ignore(1); // ignore the white space
    (*stream).get(tagName,sizeof(tagName),' ');

    (*stream).ignore(10,'=');
    *stream >> value; // ignore the white space

    strncpy(tag_name,tagName,strlen(tagName)); // strncpy is used because
                                               // the tag name has to have
                                               // exactly 8 characters
    time_span=TimeSpanStrToByte(timeSpanFlags);

    UINT32 tag_idx = getTagIndex(tag_name, strlen(tag_name));

    dralListener->SetTagSingleValue(item_id, tag_idx, value, time_span);
}

void DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::SetTagString(
    istringstream * stream)
{
    char tag_name[9]="        ";
    char tagName[9];
    char timeSpanFlags[50];
    UINT32 item_id;
    unsigned char time_span;
    stringbuf string;

    *stream >> item_id;
    (*stream).ignore(1); // ignore the white space
    (*stream).get(timeSpanFlags,sizeof(timeSpanFlags),' ');
    (*stream).ignore(1); // ignore the white space
    (*stream).get(tagName,sizeof(tagName),' ');
    (*stream).ignore(10,'=');
    (*stream).ignore(1); // ignore the white space
    (*stream).get(string);
    string.sputc('\0'); // it adds the null terminator to the string

    strncpy(tag_name,tagName,strlen(tagName)); // strncpy is used because
                                               // the tag name has to have
                                               // exactly 8 characters
    time_span=TimeSpanStrToByte(timeSpanFlags);

    UINT32 tag_idx = getTagIndex(tag_name, strlen(tag_name));
    UINT32 str_idx = getStringIndex(string.str().data(), string.str().length());

    dralListener->SetTagString(item_id, tag_idx, str_idx, time_span);

}

void DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::SetTagSet(
    istringstream * stream, char * buffer)
{
    char tag_name[9]="        ";
    char tagName[9];
    char timeSpanFlags[50];

    UINT32 item_id;
    unsigned char time_span;
    UINT16 set_len;
    UINT32 j;

    *stream >> item_id;
    (*stream).ignore(1); // ignore the white space
    (*stream).get(timeSpanFlags,sizeof(timeSpanFlags),' ');
    (*stream).ignore(1); // ignore the white space
    (*stream).get(tagName,sizeof(tagName),' ');

    strncpy(tag_name,tagName,strlen(tagName)); // strncpy is used because
                                               // the tag name has to have
                                               // exactly 8 characters
    time_span=TimeSpanStrToByte(timeSpanFlags);

    set_len=HowManyElements(buffer,',')+1; // ir returns the aproximate
                                           // number of elements of the set.
                                           // It is used to allocate
                                           // memory for the set

    UINT64 * values = new UINT64 [set_len]; // the alocation

    (*stream).ignore(20,'{');
    j=0;
    while ( *stream >> values[j])
    {
        (*stream).ignore(1); // ignore ","
        j++;
    }

    UINT32 tag_idx = getTagIndex(tag_name, strlen(tag_name));

    dralListener->SetTagSet(item_id, tag_idx, j, values, time_span);

    delete [] values;
}

void DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::MoveItems(
    istringstream * stream, char * buffer)
{
    UINT16 edge_id;
    UINT16 set_len;
    UINT16 j=1;
    UINT32 * positions;
    char c;

    (*stream).ignore(5); // ignore " edge"
    *stream >> edge_id;
    (*stream).ignore(6); // ignore " items"

    set_len=HowManyElements(buffer,',')+1; // ir returns the aproximate
                                           // number of elements of the set.
                                           // It is used to allocate
                                           // memory for the set

    UINT32 * values = new UINT32 [set_len]; // the alocation



    *stream >> values [0];
    if (*stream >> c)
    {
        switch (c)
        {
          case '@':
            positions = new UINT32 [set_len];
            *stream >> positions [0];
            (*stream).ignore(1); // ignore ","
            while ( *stream >> values[j])
            {
                (*stream).ignore(1); // ignore "@"
                *stream >> positions [j];
                (*stream).ignore(1); // ignore ","
                j++;
            }
            dralListener->MoveItemsWithPositions(edge_id,j,values,positions);
            delete [] positions;
            break;
          case ',':
            while ( *stream >> values[j])
            {
                (*stream).ignore(1); // ignore ","
                j++;
            }
            dralListener->MoveItems(edge_id,j,values);
            break;
        }
    }
    else  // only one value with no position
    {
        dralListener->MoveItems(edge_id,j,values);
    }
    delete [] values;
}

void DRAL_CLIENT_ASCII_IMPLEMENTATION_CLASS::SetCapacity(
    istringstream * stream, char * buffer)
{
    UINT16 node_id;
    UINT32 * capacities;
    UINT32 capacity;
    UINT16 dimensions;

    dimensions=HowManyElements(buffer,'x')+1;

    capacities = new UINT32 [dimensions];

    (*stream).ignore(5); // ignore " node"
    *stream >> node_id;
    (*stream).ignore(4); //ignore " cap"
    *stream >> capacities[0];
    capacity=capacities[0];

    for (UINT16 i=1;i<dimensions;i++)
    {
        (*stream).ignore(2); //ignore " x"
        *stream >> capacities[i];
        capacity*=capacities[i];
    }

    dralListener->SetCapacity(node_id,capacity,capacities,dimensions);

    delete [] capacities;
}
