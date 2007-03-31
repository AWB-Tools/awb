/*
 * Copyright (C) 2004-2006 Intel Corporation
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

/**
 * @file
 * @author Alex Settle
 * @brief Generic cache class that can be configured at runtime.
 */

// ASIM core
#include "asim/cache_dyn.h"

// Initialization of non-const static member variable.  The initialization
// will happen only once and will take effect before main() is called.
UINT32 dyn_cache_class::DEFAULT_CACHE_RANDOM_SEED = 0;

