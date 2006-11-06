/*
 * *****************************************************************
 * *                                                               *
 *Copyright (C) 2002-2006 Intel Corporation
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
 * @file
 * @author Artur Klauser
 *
 * @brief This file contains a null implementation for instruction addresses
 */

#ifndef _INST_ADDR_
#define _INST_ADDR_

// generic
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"

typedef class IADDR_CLASS *IADDR;
class IADDR_CLASS {
  private:
    // variables
    // - nada -
  public: 
    // constructors / destructors / initializers
    /// Default constructor clears address.
    IADDR_CLASS() { }

    // compare operators
    bool operator == (const IADDR_CLASS & rhs) { return true; }
    bool operator != (const IADDR_CLASS & rhs) { return false; }

    // Dummy functions
    IADDR_CLASS Next (void) const { return IADDR_CLASS(); }
    IADDR_CLASS Prev (void) const { return IADDR_CLASS(); }
    UINT64 GetBundleAddr (void) const { return 0; }
    UINT32 GetSyllableIndex (void) const { return 0; }

    // UINT64 -> IADDR_CLASS
    IADDR_CLASS (UINT64 a) { }
};

// output operator
std::ostream & operator << (std::ostream & os, const IADDR_CLASS & ia);

#endif //_INST_ADDR_
