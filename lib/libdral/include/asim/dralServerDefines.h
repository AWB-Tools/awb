/**************************************************************************
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

/**
 * @file dralServerDefines.h
 * @author Pau Cabre 
 * @brief dral server defines
 */


#ifndef DRAL_SERVER_DEFINES_H
#define DRAL_SERVER_DEFINES_H

#define DRAL_SERVER_VERSION_MAJOR 4    /**< Interface version */
#define DRAL_SERVER_VERSION_MINOR 0    /**< Interface implementation version */

#include <iostream>
using namespace std;

#define DRAL_ASSERT(condition,msg) \
    if (! (condition)) { \
        cerr << "DRAL error in file " << __FILE__ " line " << __LINE__<< ": " \
        << __PRETTY_FUNCTION__ << ": " << msg << endl; \
        abort(); \
    }

#define DRAL_WARNING(msg) \
    cerr << "Warning: " << msg << endl;


#ifndef DRAL_ANY
#define DRAL_ANY         UINT32_MAX
#endif

#ifndef DRAL_ALL
#define DRAL_ALL         (UINT32_MAX-1)
#endif

#ifndef DRAL_EMPTY
#define DRAL_EMPTY       (UINT32_MAX-2)
#endif

#endif /* DRAL_SERVER_DEFINES_H */

