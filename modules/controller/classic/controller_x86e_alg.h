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

#ifndef __CONTROL_X86E_ALG_H__
#define __CONTROL_X86E_ALG_H__

// ASIM modules 
#include "asim/provides/controller.h"

// the local module
#include "control_x86e.h"

//
// The single controller instance
// The x86e controller is derived from the x86 controller,
// which is derived from the classic controller
//
typedef CONTROLLER_X86E_CLASS ACTUAL_CONTROLLER_CLASS;
extern                        ACTUAL_CONTROLLER_CLASS theController;

//
// This is a bit of a hack!!
// In the parent class, control_x86, the following are provided by Asim dynamic parameters,
// but in this class they are knobs.  We need to declare these here so the parent class
// compiles ok.
//
extern Knob<bool>   perfchecker;
extern Knob<uint32> pipetrace_dump_max;
extern Knob<uint32> nproc;
extern Knob<uint32> ncore;
extern Knob<uint32> nthread;

//
// this function needs to be publicized, since it is called by the performance model code
//
extern void ArchlibSupport_SetThreads(uint32 np, uint32 nc, uint32 nt);


#endif /* __CONTROL_X86E_ALG_H__ */
/*
 * INTEL CONFIDENTIAL
 * Copyright 2006 Intel Corporation All Rights Reserved.
 * 
 * The source code contained or described herein and all documents related to the source code
 * ("Material") are owned by Intel Corporation or its suppliers or licensors. Title to the
 * Material remains with Intel Corporation or its suppliers and licensors. The Material may
 * contain trade secrets and proprietary and confidential information of Intel Corporation and
 * its suppliers and licensors, and is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed, or disclosed in any way without
 * Intels prior express written permission. 
 * 
 * No license under any patent, copyright, trade secret or other intellectual property right is
 * granted to or conferred upon you by disclosure or delivery of the Materials, either expressly,
 * by implication, inducement, estoppel or otherwise. Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

/**
 * @file
 * @author Carl Beckmann
 * @brief
 */

#ifndef __CONTROL_X86E_ALG_H__
#define __CONTROL_X86E_ALG_H__

// ASIM modules 
#include "asim/provides/controller.h"

// the local module
#include "control_x86e.h"

//
// The single controller instance
// The x86e controller is derived from the x86 controller,
// which is derived from the classic controller
//
typedef CONTROLLER_X86E_CLASS ACTUAL_CONTROLLER_CLASS;
extern                        ACTUAL_CONTROLLER_CLASS theController;

//
// This is a bit of a hack!!
// In the parent class, control_x86, the following are provided by Asim dynamic parameters,
// but in this class they are knobs.  We need to declare these here so the parent class
// compiles ok.
//
extern Knob<bool>   perfchecker;
extern Knob<uint32> pipetrace_dump_max;
extern Knob<uint32> nproc;
extern Knob<uint32> ncore;
extern Knob<uint32> nthread;

//
// this function needs to be publicized, since it is called by the performance model code
//
extern void ArchlibSupport_SetThreads(uint32 np, uint32 nc, uint32 nt);


#endif /* __CONTROL_X86E_ALG_H__ */
/*
 * INTEL CONFIDENTIAL
 * Copyright 2006 Intel Corporation All Rights Reserved.
 * 
 * The source code contained or described herein and all documents related to the source code
 * ("Material") are owned by Intel Corporation or its suppliers or licensors. Title to the
 * Material remains with Intel Corporation or its suppliers and licensors. The Material may
 * contain trade secrets and proprietary and confidential information of Intel Corporation and
 * its suppliers and licensors, and is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed, or disclosed in any way without
 * Intels prior express written permission. 
 * 
 * No license under any patent, copyright, trade secret or other intellectual property right is
 * granted to or conferred upon you by disclosure or delivery of the Materials, either expressly,
 * by implication, inducement, estoppel or otherwise. Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

/**
 * @file
 * @author Carl Beckmann
 * @brief
 */

#ifndef __CONTROL_X86E_ALG_H__
#define __CONTROL_X86E_ALG_H__

// ASIM modules 
#include "asim/provides/controller.h"

// the local module
#include "control_x86e.h"

//
// The single controller instance
// The x86e controller is derived from the x86 controller,
// which is derived from the classic controller
//
typedef CONTROLLER_X86E_CLASS ACTUAL_CONTROLLER_CLASS;
extern                        ACTUAL_CONTROLLER_CLASS theController;

//
// This is a bit of a hack!!
// In the parent class, control_x86, the following are provided by Asim dynamic parameters,
// but in this class they are knobs.  We need to declare these here so the parent class
// compiles ok.
//
extern Knob<bool>   perfchecker;
extern Knob<uint32> pipetrace_dump_max;
extern Knob<uint32> nproc;
extern Knob<uint32> ncore;
extern Knob<uint32> nthread;

//
// this function needs to be publicized, since it is called by the performance model code
//
extern void ArchlibSupport_SetThreads(uint32 np, uint32 nc, uint32 nt);


#endif /* __CONTROL_X86E_ALG_H__ */
