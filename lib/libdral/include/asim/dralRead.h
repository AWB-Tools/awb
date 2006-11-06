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
 * @file dralRead.h
 * @author Pau Cabre 
 * @brief
 */


#ifndef DRAL_READ_H
#define DRAL_READ_H

#include "asim/dral_syntax.h"
#include <zlib.h>
#include <pthread.h>
#include "asim/dralListener.h"

/**
 * This class performs the buffered read to the dral client
 */
class DRAL_BUFFERED_READ_CLASS
{

  public:

    /**
     * Constructor with the file descriptor, the dral listener (to comunicate
     * errors) and the buffer size
     */
    DRAL_BUFFERED_READ_CLASS (
        int file_descriptor, DRAL_LISTENER dralListener,
        UINT32 buffer_size = 4096);
    
    ~DRAL_BUFFERED_READ_CLASS();
    
    /**
     * This is the public method used to read bytes from the buffer.
     * It returns a pointer to the data and in 'r' the number of bytes read.
     * 'r' will be == to num_bytes if everything goes right.
     * If there is an error, 'r' will be 0 (end of file found) of -1 (error
     * reading the file descriptor). Note: this class will use the dral listener
     * to comunicate the error.
     * The parameter num_bytes should be != 0 (if it is 0, a NULL pointer will
     * be returned)
     */
    void * Read (UINT32 num_bytes, INT64 * r);

    /**
     * Public method used to know the size of the file associated to the file
     * descriptor.
     * It will return -1 if the size is impossible to know.
     */
    INT64 GetFileSize (void);
    
    /**
     * Public method used to know the number of bytes passed to the dral client.
     */
    UINT64 GetNumBytesRead (void);


    /**
     * Public method used to know how many unread bytes are still in the buffer
     */
    UINT32 AvailableBytes(void);

  private:

    /**
     * Private method that performs the read form the file descriptor to the
     * buffer.
     */
    INT64 ReadFD (void * buf, UINT32 num_bytes);

    /*
     * Private variables
     */
    int fd;
    gzFile file;
    UINT32 bufferSize;
    UINT32 available;
    UINT32 pos;
    UINT64 numBytesRead;
    void * buffer;
    DRAL_LISTENER dralListener;
    bool errorFound;

};
typedef DRAL_BUFFERED_READ_CLASS * DRAL_BUFFERED_READ; 

#endif /* DRAL_READ_H */
