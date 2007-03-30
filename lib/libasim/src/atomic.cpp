/*****************************************************************************
 *
 * @brief Atomic counter type.  Useful for threadsafe reference counting
 *
 * @author Kenneth C. Barr. 
 * @brief string-ify the atomic types 
 * 
 *Copyright (C) 2004-2006 Intel Corporation
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

#include "asim/atomic.h"
#include <iostream>

std::ostream & operator<< (std::ostream & os, ATOMIC32_CLASS a){
    os << a.val;
    return os;
}

std::ostream & operator<< (std::ostream & os, ATOMIC64_CLASS a){
    os << __read_val((_Atomic64 *)(&(a.val)));
    return os;
}
