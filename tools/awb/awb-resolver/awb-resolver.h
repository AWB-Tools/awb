/**************************************************************************
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
 * @author Artur Klauser
 * @brief AWB Resolver - filename resolution and queries of union dirs
 */

#ifndef _AWB_RESOLVER_
#define _AWB_RESOLVER_ 1

// local
#include "libawb/workspace.h"

using namespace std;

class AWB_RESOLVER {
  private:
    Workspace * workspace;
    UnionDir * sourceTree;

    /// command line flags are these command types
    enum command_types {
        CMD_HELP     = 1,
        CMD_USAGE,
        CMD_CONFIG,
        CMD_GLOB,
        CMD_PREFIX,
        CMD_SUFFIX
    };

  public:
    // constructors / destructors
    AWB_RESOLVER();
    ~AWB_RESOLVER();

    void ProcessCommandLine (int argc, char ** argv);

  private:
    void PrintHelp (const poptContext & optContext);
    void PrintSearchPath (ostream & out, const string & prefix);
};

#endif // _AWB_RESOLVER_
