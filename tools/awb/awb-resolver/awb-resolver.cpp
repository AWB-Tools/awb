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

// generic (C)
#include <popt.h>
#include <stdio.h>

// generic (c++)
#include <iostream>

// local
#include "awb-resolver.h"
#include "libawb/util.h"

/**
 * Create a new awb resolver
 */
AWB_RESOLVER::AWB_RESOLVER(void)
{
    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Error: Workspace creation failed!" << endl;
        exit (1);
    }
    sourceTree = & workspace->GetSourceTree();
}

/**
 * Destroy this awb resolver
 */
AWB_RESOLVER::~AWB_RESOLVER()
{
    // make sure we delete objects in reverse creation order, since later
    // allocated objects can depend on earlier ones;
    sourceTree = NULL;
    if (workspace) {
        delete workspace;
    }
}

/**
 * Process an awb resolver command line
 */
void
AWB_RESOLVER::ProcessCommandLine (
    int argc,
    char ** argv)
{
    poptContext optContext;   // context for parsing command-line options
    char cmd;
    char * arg = NULL;
    int quiet = 0;

    struct poptOption helpOptionsTable[] = {
        { "help", 'h', POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH,
          NULL, CMD_HELP,
          "show this help message", NULL },
        { "usage", '\0', POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH,
          NULL, CMD_USAGE,
          "display brief usage message", NULL },
        POPT_TABLEEND
    };

    struct poptOption optionsTable[] = {
        { "config", '\0', POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,
          &arg, CMD_CONFIG,
          "print configuration parameter", "<param name>" },
        { "glob", '\0', POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,
          &arg, CMD_GLOB,
          "glob of union directory", "<glob expression>" },
        { "prefix", '\0', POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,
          &arg, CMD_PREFIX,
          "print prefix of file in union directory", "<file>" },
        { "suffix", '\0', POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,
          &arg, CMD_SUFFIX,
          "print unresolved suffix of file in union directory", "<file>" },
        { "quiet", 'q', POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH,
          &quiet, 0,
          "suppress printing of some messages", "" },
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
          "Help options:", NULL },
        POPT_TABLEEND
    };

    optContext = poptGetContext ("awb-resolver", argc,
        const_cast<const char**>(argv), optionsTable, 0);
    poptSetOtherOptionHelp (optContext, "[OPTION] [<file>]");
    poptReadDefaultConfig(optContext, 0);

    //
    // now process options
    //
    int numCmds = 0;
    while ((cmd = poptGetNextOpt(optContext)) >= 0) {
        // all commands are mutually exclusive - check that
        if (numCmds++ > 0) {
            if ( ! quiet ) {
                cerr << "Error: more than one command given" << endl;
                cerr << endl;
                poptPrintUsage(optContext, stderr, 0);
            }
            exit (1);
        }

        switch (cmd) {
          case CMD_HELP: 
            PrintHelp (optContext);
            exit (0);
            break;
          case CMD_USAGE:
            poptPrintUsage(optContext, stderr, 0);
            exit (0);
            break;
          case CMD_CONFIG:
            {
              string config = arg;
              if (config == "awblocal" || config == "workspace") {
                  cout << workspace->GetDirectory(Workspace::WorkspaceDir)
                       << endl;
              } else if (config == "asimdir") {
                  // we don't support it anymore, but we won't exit(1) either
                  if ( ! quiet ) {
                      cerr << "Warning: -config asimdir not supported anymore"
                           << endl;
                  }
                  // note: this leaves "nothing" on stdout! which can be
                  // should be OK for old programs using stdout as ASIMDIR
              } else if (config == "benchmarkdir") {
                  cout << workspace->GetDirectory(Workspace::BenchmarkDir)
                       << endl;
              } else if (config == "builddir") {
                  cout << workspace->GetDirectory(Workspace::BuildDir)
                       << endl;
              } else if (config == "searchpath") {
                  PrintSearchPath(cout, "");
              } else if (config == "compiler") {
                  cout << workspace->GetBuildEnv(Workspace::BuildEnvCompiler)
                       << endl;
              } else if (config == "debug") {
                  cout << workspace->GetBuildEnvFlag(
                          Workspace::BuildEnvFlagDebug)
                       << endl;
              } else if (config == "optimize") {
                  cout << workspace->GetBuildEnvFlag(
                          Workspace::BuildEnvFlagOptimize)
                       << endl;
              } else if (config == "parallel") {
                  cout << workspace->GetBuildEnvFlag(
                          Workspace::BuildEnvFlagParallel)
                       << endl;
              } else {
                  if ( ! quiet ) {
                      cerr << "Error: -config unknown parameter "
                           << "'" << config << "'" << endl;
                      cerr << endl;
                      PrintHelp(optContext);
                  }
                  exit (1);
              }

              break;
            }
          case CMD_GLOB:
            {
              UnionDir::StringList globList;

              sourceTree->Glob(arg, globList);
              FOREACH_CONST (UnionDir::StringList, it, globList) {
                  cout << *it << endl;
              }

              break;
            }
          case CMD_PREFIX:
            {
              string prefix;

              prefix = sourceTree->GetPrefix(arg);
              if (prefix.empty()) {
                  if ( ! quiet ) {
                      cerr << "Can't find '" << arg << "' in your SEARCHPATH:"
                           << endl;
                      PrintSearchPath(cerr, "    ");
                  }
                  exit (1);
              } else {
                  cout << prefix << endl;
              }

              break;
            }
          case CMD_SUFFIX:
            {
              string suffix;

              suffix = sourceTree->GetSuffix(arg);
              if (suffix.empty()) {
                  if ( ! quiet ) {
                      cerr << "Can't find '" << arg << "' in your SEARCHPATH:"
                           << endl;
                      PrintSearchPath(cerr, "    ");
                  }
                  exit (1);
              } else {
                  cout << suffix << endl;
              }

              break;
            }
          default:
            if ( ! quiet ) {
                cerr << "Error: unknown command given" << endl;
                cerr << endl;
                poptPrintUsage(optContext, stderr, 0);
            }
            exit (1);
            break;
        }
    }
    if (cmd < -1) {
        // an error occurred during option processing
        cerr << poptBadOption(optContext, POPT_BADOPTION_NOALIAS) << " : "
             << poptStrerror(cmd) << endl;
        cerr << endl;
        poptPrintUsage(optContext, stderr, 0);
        exit (1);
    }

    // if we have not yet processed a command, the remaining arguments
    // default to a 'fullname' command
    if (numCmds == 0) {
        string fileName;
        string fullName;

        // get leftover args
        const char ** arguments = poptGetArgs(optContext);
        if (arguments) {
            fileName = arguments[0];
        } else {
            fileName = "";
        }

        fullName = sourceTree->FullName(fileName);
        if (fullName.empty()) {
            if ( ! quiet ) {
                cerr << "Can't find '" << fileName << "' in your SEARCHPATH:"
                     << endl;
                PrintSearchPath(cerr, "    ");
            }
            exit (1);
        } else {
            cout << fullName << endl;
        }
    }

    poptFreeContext(optContext);
}

/**
 * Print help message
 */
void
AWB_RESOLVER::PrintHelp (
    const poptContext & optContext) ///< command-line parsing context
{
    poptPrintHelp(optContext, stderr, 0);
    cerr << endl;
    cerr << "All options (other than --quiet) are mutually exclusive." << endl;
    cerr << "The default operation is to convert the file name given on" << endl;
    cerr << "the command line to an absolute path." << endl;
    cerr << endl;
    cerr << "parameters that can be queried with the -config command:" << endl;
    cerr << "    workspace, benchmarkdir, builddir, searchpath,"<< endl;
    cerr << "    compiler, debug, optimize, parallel" << endl;
    cerr << endl;
}

void
AWB_RESOLVER::PrintSearchPath (
    ostream & out,
    const string & prefix)
{
    const UnionDir::StringList * searchPath = & sourceTree->GetSearchPath();

    FOREACH_CONST (UnionDir::StringList, it, *searchPath) {
        out << prefix << *it << endl;
    }
}

//----------------------------------------------------------------------------

int main (int argc, char ** argv)
{
    AWB_RESOLVER awb_resolver;

    awb_resolver.ProcessCommandLine (argc, argv);
}
