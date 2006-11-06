/*
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

/*
 * *****************************************************************
 * *                                                               *

/**
 * @file
 * @author Artur Klauser
 *
 * @brief A standalone CPU_INST for APE
 */

// ASIM public modules
#include "asim/provides/cpu_inst.h"

ASIM_MM_DEFINE(CPU_INST_CLASS, MAX_CPU_INST);

ASIM_MM_DEFINE(CPU_BLOCKADDR_CLASS, MAX_BLOCKADDR);
UINT64 CPU_BLOCKADDR_CLASS::uniqueBlockId;
