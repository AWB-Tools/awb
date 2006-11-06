/*****************************************************************************
 *
 * @brief Header file for Chip Component
 *
 * @author Pritpal Ahuja
 *
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

#ifndef _CHIP_COMPONENT_
#define _CHIP_COMPONENT_

// ASIM core
#include "asim/syntax.h"
#include "asim/module.h"

// ASIM public modules


typedef class ASIM_CHIP_COMPONENT_CLASS *ASIM_CHIP_COMPONENT;

class ASIM_CHIP_COMPONENT_CLASS : public ASIM_MODULE_CLASS
{
  public:

    ASIM_CHIP_COMPONENT_CLASS(ASIM_MODULE parent, const char* const name);

    ~ASIM_CHIP_COMPONENT_CLASS();

    // This is an abstract class
    virtual void Clock(const UINT64 cycle) =0;
};

inline 
ASIM_CHIP_COMPONENT_CLASS::ASIM_CHIP_COMPONENT_CLASS(
    ASIM_MODULE parent, 
    const char* const name)
    : ASIM_MODULE_CLASS(parent, name)
{}

inline 
ASIM_CHIP_COMPONENT_CLASS::~ASIM_CHIP_COMPONENT_CLASS()
{}

#endif /* _CHIP_COMPONENT_ */
