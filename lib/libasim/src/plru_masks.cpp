/*
 *Copyright (C) 2004-2007 Intel Corporation
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

#include "asim/plru_masks.h"
const UINT64 PLRU_MaskInner<2>::mask[2] = { 0x01, 0x01 };
const UINT64 PLRU_MaskInner<2>::compare[2] = { 0x00, 0x01 };
const UINT64 PLRU_MaskInner<3>::mask[3] ={ 0x03, 0x05, 0x06 };
const UINT64 PLRU_MaskInner<3>::compare[3] = { 0x00, 0x01, 0x06 };
const UINT64 PLRU_MaskInner<4>::mask[4]    = { 0x05, 0x05, 0x06, 0x06 };
const UINT64 PLRU_MaskInner<4>::compare[4] = { 0x00, 0x01, 0x04, 0x06 };
const UINT64 PLRU_MaskInner<6>::mask[6]    = { 0x019, 0x019, 0x02a, 0x02a, 0x034, 0x034 };
const UINT64 PLRU_MaskInner<6>::compare[6] = { 0x000, 0x001, 0x008, 0x00a, 0x030, 0x034 };
const UINT64 PLRU_MaskInner<8>::mask[8]   = { 0x00b, 0x00b, 0x013, 0x013, 
					      0x025, 0x025, 0x045, 0x045 };
const UINT64 PLRU_MaskInner<8>::compare[8] = { 0x000, 0x008, 0x002, 0x012, 
					       0x001, 0x021, 0x005, 0x045 };

const UINT64 PLRU_MaskInner<12>::mask[12]    = { 0x0605, 0x0605, 0x0606, 0x0606, 0x0a28, 0x0a28,
						 0x0a30, 0x0a30, 0x0d40, 0x0d40, 0x0d80, 0x0d80 };

const UINT64 PLRU_MaskInner<12>::compare[12] = { 0x0000, 0x0001, 0x0004, 0x0006, 0x0200, 0x0208,
						 0x0220, 0x0230, 0x0c00, 0x0c40, 0x0d00, 0x0d80 };

const UINT64 PLRU_MaskInner<16>::mask[16] = { 0x04045, 0x04045, 0x04046, 0x04046, 
					      0x04068, 0x04068, 0x04070, 0x04070, 
					      0x06280, 0x06280, 0x06300, 0x06300,
					      0x07400, 0x07400, 0x07800, 0x07800 };

const UINT64 PLRU_MaskInner<16>::compare[16] = { 0x00000, 0x00001, 0x00004, 0x00006,
						 0x00040, 0x00048, 0x00060, 0x00070, 
						 0x04000, 0x04080, 0x04200, 0x04300,
						 0x06000, 0x06400, 0x07000, 0x07800 };

const UINT64 PLRU_MaskInner<24>::mask[24]   = { 0x0600045, 0x0600045, 0x0600046, 0x0600046, 
						0x0600068, 0x0600068, 0x0600070, 0x0600070, 
						0x0a02280, 0x0a02280, 0x0a02300, 0x0a02300,
						0x0a03400, 0x0a03400, 0x0a03800, 0x0a03800,
						0x0d14000, 0x0d14000, 0x0d18000, 0x0d18000,
						0x0da0000, 0x0da0000, 0x0dc0000, 0x0dc0000 };

const UINT64 PLRU_MaskInner<24>::compare[24] = { 0x0000000, 0x0000001, 0x0000004, 0x0000006,
						 0x0000040, 0x0000048, 0x0000060, 0x0000070,
						 0x0200000, 0x0200080, 0x0200200, 0x0200300,
						 0x0202000, 0x0202400, 0x0203000, 0x0203800,
						 0x0c00000, 0x0c04000, 0x0c10000, 0x0c18000,
						 0x0d00000, 0x0d20000, 0x0d80000, 0x0dc0000 };

