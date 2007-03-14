/*
 *Copyright (C) 1999-2006 Intel Corporation
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
 * @author Carl Beckmann
 * @brief
 */

#ifndef __CONTROL_CLASSIC_ALG_H__
#define __CONTROL_CLASSIC_ALG_H__

// ASIM modules 
#include "asim/provides/controller.h"



//
// The single controller instance
// The "Asim classic controller" uses as its implementation the controller
// base class directly, so there is nothing here to override
//
typedef CONTROLLER_CLASS ACTUAL_CONTROLLER_CLASS;
extern                   ACTUAL_CONTROLLER_CLASS theController;


#endif /* __CONTROL_CLASSIC_ALG_H__ */
