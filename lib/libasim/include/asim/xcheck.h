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

/**
 * @file
 * @author Roger Espasa
 * @brief Cross-check two live simulator runs with each other (for debugging).
 */

#ifndef _XCHECK_
#define _XCHECK_

// generic
#include <errno.h>
#include <string>
#include <sstream>

// ASIM core
#include "asim/syntax.h"

using namespace std;

#define XCHECK_MAX_STRING	80
#define CROSSCHECK(A) {ostringstream os; os << A; CrossCheck(os.str());}

/// Turn on cross checking into specified file.
void ActivateCrossChecking(const char * const file);

/// CrossCheck output method.
void CrossCheck(const string & line);

#endif // _XCHECK_
