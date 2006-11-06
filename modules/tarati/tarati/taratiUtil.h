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
 * @file
 * @author Artur Klauser
 * @brief Utility functions for Tarati.
 */

#ifndef _TARATI_UTIL_
#define _TARATI_UTIL_ 1

// generic
#include <iostream>

// XML-RPC low level transport library
#include <asim/provides/xmlrpc.h>

using namespace XmlRpc;

namespace Tarati {

// a simple error implementation
#define TARATI_ERROR(a) \
  std::cerr << "Tarati::Error " << __FILE__ << ":" << __LINE__ << std::endl; \
  std::cerr << "  " << a << std::endl; \
  exit(1);

/**
 * Error handler replacement for XmlRpc errors.
 */
class TaratiXmlRpcErrorHandler : public XmlRpcErrorHandler
{
  public:
    void error(const char* msg);
};

extern TaratiXmlRpcErrorHandler taratiXmlRpcErrorHandler;

} // namespace Tarati

#endif // _TARATI_UTIL_
