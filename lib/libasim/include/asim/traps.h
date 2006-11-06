/*
 * Copyright (C) 2003-2006 Intel Corporation
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

/*
 * *****************************************************************
 * *                                                               *

/**
 * @file
 * @author ??
 * @brief
 */

// (r2r) what the heck is that?

#define TRAP_RESET	0x0001  /* Immediate */
#define TRAP_ITB_ACV	0x0081  /* PreMap */
#define TRAP_INTERRUPT	0x0101  /* Immediate */
#define TRAP_DTB_MISS	0x0201  /* Exec - basic D-tb miss */
#define TRAP_DDTB_MISS	0x0281  /* Exec - nestetd D-tb miss */
#define TRAP_UNALIGN	0x0301  /* Retire */
#define TRAP_DFAULT	0x0381  /* Retire */
#define TRAP_ITB_MISS	0x0181  /* PreMap - I-tb miss */
#define TRAP_OPCDEC	0x0481  /* Retire */
#define TRAP_MCHK	0x0401  /* Immediate */
#define TRAP_ARITH	0x0501  /* Retire */
#define TRAP_FEN	0x0581  /* Retire */
#define TRAP_PAL	0x2001  /* PreMap */

#define TRAP_DTB_ACV    TRAP_DTB_FAULT
