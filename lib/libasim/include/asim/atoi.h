/*
 * Copyright (C) 2005-2006 Intel Corporation
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef _ATOI_H_
# define _ATOI_H_

#include <string>

// ASIM core
#include "asim/syntax.h"

extern INT64 atoi_general(const char* buf, int mul);
extern INT64 atoi_general(const char* buf);
extern INT64 atoi_general(const std::string& str);

extern UINT64 atoi_general_unsigned(const char* buf, int mul);
extern UINT64 atoi_general_unsigned(const char* buf);
extern UINT64 atoi_general_unsigned(const std::string& str);

#endif
/*
% Local Variables:
% pref: "../../src/atoi.cpp"
% End:
*/
