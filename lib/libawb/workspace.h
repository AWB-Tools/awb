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
 * @brief ASIM workspace information.
 */

#ifndef _WORKSPACE_
#define _WORKSPACE_ 1

// generic (C++)
#include <string>
#include <map>
#include <vector>

// local
#include "uniondir.h"
#include "inifile.h"

using namespace std;

/**
 * @brief ASIM workspace information.
 *
 * This class manages the (global) workspace information.
 * The initial information is loaded from the workspace configuration file
 * in the constructor. After that, it can be used with a set of accessors
 * and modifiers.
 *
 * The form of the awb.config file is like a windows ini file.
 * Path specifiers allow relative paths, which are interpreted as relative
 * from workspaceDir. Path specifiers also allow * and ? file globs and
 * {,} cartesian products. Paths can reference variables declared in the
 * Vars section, which are string substituted for their definition. Variable
 * syntax is $(varname) or ${varname}. Globs and cartesian products used
 * for scalar path variables will get truncated after the first element.
 * Path lists allow collon, space, or tab separators.
 *        
 * <pre>
 *   [Global]
 *   
 *   [Vars]
 *
 *   [Paths]
 *   # Directory containing actual benchmarks
 *   BENCHMARKDIR=/work/benchmarks
 *   
 *   # Directory to do model builds in
 *   BUILDDIR=build
 *   
 *   # Directory search path for modules
 *   SEARCHPATH=asim
 *   
 *   [Build]
 *   # Compiler (GEM | GNU)
 *   COMPILER=GEM
 *   
 *   # Buildtype (DEBUG | OPTIMIZE)
 *   BUILDTYPE=DEBUG
 *   
 *   # Do parallel make (1) or not (0)
 *   PARALLEL=0
 *   
 *   # Build binary with (1) or without (0) events
 *   EVENTS=1
 * </pre>        
 */

class Workspace {
  public:
    // types
    /// Interface type for container of strings.
    typedef vector<string> StringList;

    /// Abstract directory names
    enum DirectoryName {
        // the following is the top level dir
        WorkspaceDir = 0,    ///< top level workspace dir
        // following directories are from the workbench config file
        BenchmarkDir,        ///< benchmark base directory
        BuildDir,            ///< fallback build and run base directory
        // following directories are relative to SourceTree and represent
        // conventions that have to be followed - not configurable
        RelSourceBaseDir,    ///< base directory in source tree
        RelSourceConfigDir,  ///< config directory in source tree
        RelSourceModelDir,   ///< model directory in source tree
        // following directories are relative to BuildDir and represent
        // conventions that have to be followed - not configurable
        RelBuildSourceDir,   ///< source directory in build dir
        RelBuildIncludeDir,  ///< include directory in build dir
        LAST_DIR             ///< sentinel
    };

    /// Abstract build environment names
    enum BuildEnvName {
        BuildEnvCompiler = 0,///< compiler type to use in build process
        BuildEnvMake,        ///< make program to use in build process
        BuildEnvMakeFlags,   ///< arbitrary flags to pass for make program
        LAST_BUILD_ENV       ///< sentinel
    };

    /// Abstract build environment flag names
    enum BuildEnvFlagName {
        BuildEnvFlagParallel, ///< parallel build requested
        BuildEnvFlagGnu,      ///< build with GNU compiler
        BuildEnvFlagDebug,    ///< build with debug turned on
        BuildEnvFlagOptimize, ///< build with optimizations turned on
        BuildEnvFlagStatic,   ///< build statically linked executable
        BuildEnvFlagWarn,     ///< build with warnings turned on
        BuildEnvFlagEvents,   ///< build with / without events code in the binary
        LAST_BUILD_ENV_FLAG   ///< sentinel
    };

  private:
    // members
    IniFile workspaceConfig;   ///< workspace's awb.config IniFile object
    UnionDir * sourceTree;     ///< the ASIM source tree UnionDir

    // directories
    /// An abstract map of directories, ie. an abstract directory name
    /// like BenchmarkDir is mapped to a concrete directory in the file
    /// system.
    string directory[LAST_DIR]; 

    // build environment
    /// An abstract map of the build environment, ie. an abstract build
    /// environment name like BuildCompiler is mapped to the actual
    ///compiler to be used.
    string buildEnv[LAST_BUILD_ENV];
    /// boolean flags in abstract build environment
    bool buildEnvFlag[LAST_BUILD_ENV_FLAG];

    // methods
    /// Expansion of path expressions to single regular directory.
    string ExpandPath (const string & path, const string & basedir) const;
    /// Expansion of path expressions to list of regular directories.
    void ExpandPath (const StringList & path,
        const string & basedir, StringList & expanded) const;
#ifdef TESTS
    friend void TestExpandPath (char * workspaceDir, char * path,
           char * baseDir);
#endif

  public:
    // workspace factory (bootstrapping a workspace)
    /// Setup a Workspace
    static Workspace * Setup (void);

    // constructors / destructors
    /// @brief Create a new config object representing the configuration
    /// found at the specified workspaceDir.
    Workspace(const string & workspaceDir);
    /// Release allocated memory
    ~Workspace();

    // accessors / modifiers
    /// Get a directory from the abstract directory map.
    string GetDirectory (const DirectoryName name) const;
    /// Set a directory in the abstract directory map.
    void SetDirectory (const DirectoryName name, const string & value);
    //
    /// Get an entry from the abstract build environment.
    string GetBuildEnv (const BuildEnvName name) const;
    /// Set an entry in the abstract build environment.
    void SetBuildEnv (const BuildEnvName name, const string & value);
    //
    /// Get an entry from the abstract build environment flags.
    bool GetBuildEnvFlag (const BuildEnvFlagName name) const;
    /// Set an entry in the abstract build environment flags.
    void SetBuildEnvFlag (const BuildEnvFlagName name, const bool value = true);
    /// Get a config variable from the Vars group
    string GetConfVar (const string & varName) const;

    //
    /// Get a reference to the source tree union dir
    UnionDir & GetSourceTree (void) const { return *sourceTree; }

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

#endif // _WORKSPACE_ 
