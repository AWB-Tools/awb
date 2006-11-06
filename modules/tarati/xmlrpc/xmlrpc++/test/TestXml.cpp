/*
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

// TestXml.cpp : Test XML encoding and decoding.
// The characters <>&'" are illegal in xml and must be encoded.


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <iostream>
// If you are using MSVC++6, you should update <string> to fix
// BUG: getline Template Function Reads Extra Character
#include <string>
#include <assert.h>
#include <stdlib.h>

#include "XmlRpcUtil.h"

using namespace XmlRpc;


int main(int argc, char* argv[])
{
  // Basic tests
  std::string empty;
  assert(empty == XmlRpcUtil::xmlEncode(empty));
  assert(empty == XmlRpcUtil::xmlDecode(empty));
  assert(empty == XmlRpcUtil::xmlEncode(""));
  assert(empty == XmlRpcUtil::xmlDecode(""));

  std::string raw("<>&'\"");
  assert(XmlRpcUtil::xmlDecode(XmlRpcUtil::xmlEncode(raw)) == raw);
  
  std::cout << "Basic tests passed.\n";

  // Interactive tests
  std::string s;
  for (;;) {
    std::cout << "\nEnter line of raw text to encode:\n";
    std::getline(std::cin, s);
    if (s.empty()) break;

    std::cout << XmlRpcUtil::xmlEncode(s) << std::endl;
  }

  for (;;) {
    std::cout << "\nEnter line of xml-encoded text to decode:\n";
    std::getline(std::cin, s);
    if (s.empty()) break;

    std::cout << XmlRpcUtil::xmlDecode(s) << std::endl;
  }

  return 0;
}

