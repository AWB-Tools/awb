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
 * @file dralRead.cpp
 * @author Pau Cabre
 * @brief
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "asim/dralRead.h"

#include <stdio.h>
#include <iostream>
using namespace std;

DRAL_BUFFERED_READ_CLASS::DRAL_BUFFERED_READ_CLASS (
    int file_descriptor, DRAL_LISTENER dral_listener, UINT32 buffer_size)
{
    fd=file_descriptor;
    numBytesRead=0;    
    bufferSize=buffer_size;
    dralListener=dral_listener;
    errorFound=false;

    if (bufferSize != 0)
    {
        buffer = (void *) new char [bufferSize + 1];
        if (buffer == NULL)
        {
            dralListener->Error("Not enought memory to allocate the buffer.");
            dralListener->EndSimulation();
            errorFound=true;
        }
        else
        {
            available = 0;
            pos = 0;
            file=gzdopen(dup(fd),"r");
            if (file == NULL)
            {
                dralListener->Error(strerror(errno));
                dralListener->EndSimulation();
                errorFound=true;
            }
        }
        buffer = (void *) (((char *) buffer) + 1);
    }
    else
    {
        dralListener->Error(
            "No valid buffer size");
        dralListener->EndSimulation();
        errorFound=true;
    }
}

DRAL_BUFFERED_READ_CLASS::~DRAL_BUFFERED_READ_CLASS ()
{
    if (buffer != NULL)
    {
        buffer = (void *) (((char *) buffer) - 1);
        delete [] (char *)buffer;
    }
    
    if (file != NULL)
    {
        gzclose(file);
    }
}


INT64 DRAL_BUFFERED_READ_CLASS::ReadFD(void * buf, UINT32 n)
{
    UINT32 i=0;
    INT64 r=0;

    while (r <= (n-i))
    {
        r=gzread(file,(char *)buf+i,n-i);

        if (r == -1)
        {
            int err;
            dralListener->Error((char *)gzerror(file,&err));
            if (err == Z_ERRNO)
            {
                dralListener->Error(strerror(errno));
            }
            return r;
        }
        else if (r == 0)
        {
            return i;
        }
        else
        {
            i+=r;
        }
    }
    return i;
}



void * DRAL_BUFFERED_READ_CLASS::Read(UINT32 n, INT64 * r)
{
    // n must be != 0
    INT64 tmp;
    
    if (errorFound)
    {
        *r=0;
        return NULL;
    }

    if (n > bufferSize)
    {
        //We want more bytes than the buffer size. That's not possible with
        //the current size, so we change the buffer size to n
        void * buffer_tmp = (void *) new char [n + 1];
        if (buffer_tmp == NULL)
        {
            dralListener->Error(
                "The buffer has been resized and we have run out of memory"
                " in that operation");
            dralListener->EndSimulation();
            errorFound=true;
            *r=0;
            return NULL;
        }
        memcpy(buffer_tmp,(char *)buffer+pos-1,available+1);
        pos=0;
        bufferSize=n;
        delete [] (((char *) buffer) - 1);
        buffer = (void *) (((char *) buffer_tmp) + 1);
    }

    if (n <= available)
    {
        pos+=n;
        available-=n;
        *r=n;
        numBytesRead+=n;
        return ((char *)buffer+pos-n);
    }
    else
    {
        memcpy((char *) buffer - 1,(char *) buffer + pos - 1, available + 1);
        tmp=ReadFD((void *)((char *)buffer+available),bufferSize-available);
        if (tmp == -1 || tmp == 0)
        {
            *r=tmp;
            return NULL;
        }
        pos=n;
        if ((available+tmp) < n)  // we wanted more bytes than the file has
        {
            *r=0;
            dralListener->Error(
                "Unexpected end of file\n"
                "Please report this bug to dral@bssad.intel.com attaching "
                "input files (if any)"
                );
            errorFound=true;
            return NULL;
        }
        available=available+tmp-n;
        *r=(INT64)n;
        numBytesRead+=n;
        return (buffer);
    }
}


INT64  /* It will return the file size if known or -1 otherwise */
DRAL_BUFFERED_READ_CLASS::GetFileSize (void)
{
    bool compressed = false;
    INT64 n;
    UINT32 size;
    
    INT64 current_byte = lseek(fd,0,SEEK_CUR);
    if (current_byte != -1)
    {
        if (lseek(fd,0,SEEK_SET) == -1)
        {
            dralListener->NonCriticalError(strerror(errno));
            return (-1);
        }
        else
        {
            unsigned char begining [2];
            n=read(fd,begining,2);
            if (n!=2)
            {
                dralListener->Error(strerror(errno));
                dralListener->EndSimulation();
                errorFound=true;
                return (-1);
            }
            if ((begining[0] == 0x1f) && (begining[1] == 0x8b))
            {
                compressed=true;
            }
        }
        
        if (compressed)
        {
            if (lseek(fd,-4,SEEK_END) == -1)
            {
                dralListener->Error(strerror(errno));
                dralListener->EndSimulation();
                errorFound=true;
                return (-1);
            }
            n=read(fd,(void *)&size,4);
            if (n!=4)
            {
                dralListener->Error(strerror(errno));
                dralListener->EndSimulation();
                errorFound=true;
                return (-1);
            }
        }
        else
        {
            struct stat s;
            if (fstat(fd,&s))
            {
                dralListener->Error(strerror(errno));
                dralListener->EndSimulation();
                errorFound=true;
                return (-1);
            }
            size=s.st_size;
        }

        if (lseek(fd,current_byte,SEEK_SET)==-1)
        {
            dralListener->Error(strerror(errno));
            dralListener->EndSimulation();
            errorFound=true;
            return (-1);
        }
        
        return size;
    }
    else
    {
        dralListener->NonCriticalError(strerror(errno));
        return (-1);
    }
}

UINT64 DRAL_BUFFERED_READ_CLASS::GetNumBytesRead (void)
{
    return numBytesRead;
}

UINT32 DRAL_BUFFERED_READ_CLASS::AvailableBytes (void)
{
    return available;
}
