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
 * @file dralWrite.h
 * @author Pau Cabre 
 * @brief buffered write interface
 */


#ifndef DRAL_WRITE_H
#define DRAL_WRITE_H

#include <zlib.h>
#include <stdio.h>

#include "asim/dral_syntax.h"

/*
 * This class performs the buffered writing to the file descriptor
 */
class DRAL_BUFFERED_WRITE_CLASS
{

  public:

    DRAL_BUFFERED_WRITE_CLASS (UINT16 buffer_size, bool compression);
    
    void SetFileDescriptor (int fd);
    
    ~DRAL_BUFFERED_WRITE_CLASS(void);
    
    void Write (const void * buf, UINT32 num_bytes);
    
    void Flush (void);

  private:

    UINT16 buf_size;  // the buffer size
    
    char * buffer;
    
    bool buffered;
    
    UINT16 available;  // available bytes in the buffer
    
    UINT16 pos;  // position of the begining of the free area in the buffer
    
    /*
     * Private method that performs the writing of the buffer to the file
     * descriptor
     */
    void WriteFD (const void * buf, UINT32 num_bytes);

    char zeros [8];
    
    gzFile file;
    FILE * uncompressed_file;

    bool compress;
};
typedef DRAL_BUFFERED_WRITE_CLASS * DRAL_BUFFERED_WRITE;

#endif /* DRAL_WRITE_H */
