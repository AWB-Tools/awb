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
 * @brief This file contains information about instruction addresses.
 */

// generic
#include <ostream>

// ASIM core
#include "asim/ioformat.h"

// ASIM local module
#include "null-iaddr.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

// output operator
ostream &
operator << (ostream & os, const IADDR_CLASS & ia)
{
    os << "0x0";

    return os;
}
