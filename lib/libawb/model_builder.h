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
 * @brief ASIM build tree configuration: support for building models
 */

#ifndef _MODEL_BUILDER_
#define _MODEL_BUILDER_ 1

// generic (C++)
#include <string>
#include <vector>
#include <map>

// local
#include "workspace.h"
#include "module.h"
#include "model.h"

using namespace std;

/**
 * @brief ASIM build tree configuration: support for building models
 *
 * This class holds (temporary) state and methods needed for configuring
 * the build tree of a model.
 */
class ModelBuilder {
  public:
    // types
    enum ExportType {
        Source,
        Public,
        Synthesized,
        Restricted,
        Base,
        Doxygen,
        Raw
    };

    enum ResultPath {
        SourceFile,
        DestFile,
        DestFileRel
    };

  private:
    // types
    enum TargetStructure {
        Tree,
        Verbatim
    };
    typedef vector<string> StringList;
    typedef vector<const ModParamInstance*> ModParamInstanceList;
    typedef map<string, string> StringMap;

    // consts
    //
    // Extensions we recognize when looking for source and header
    // (including doxygen source and configuration files)
    // gcc 3.2.2 complains about these lines inside the class def.
//    static const char * const IncludeExtension;
//    static const char * const SourceExtension;
//    static const char * const ConfigExtension;
//    static const char * const DoxygenExtension;
//    static const char * const RawExtension;

    /// How should the build target directory hierarchy be structured.
    /// @note that only "Tree" is fully supported at this time.
    static const TargetStructure targetStructure = Tree;

    // members
    const Workspace & workspace; ///< workspace this builder runs in
    const UnionDir & sourceTree; ///< we need this many times, so we cache it
    const Model & model;         ///< model to build
    const string buildDir;       ///< directory to build in
    
    ModParamInstanceList dynamicParams;

    /**
     * @brief Temporary information for generating Makefiles
     *
     * Temporary information for Makefile creation collected during
     * recursive module traversal. This information is only valid
     * during certain processing steps after it has been collected by
     * adequate recursive module traversal of a model.
     */
    struct Makefile {
        StringList srcs;
        StringList objs;
        StringList vpath;
        StringList incs;
        StringList incDirs;
        StringList subDirs;
        StringList subTargets;
        StringList localLibs;
        StringList localIncs;
        StringList commonLibs;
        StringMap  replace;

        void clear (void)
        {
            srcs.clear();
            objs.clear();
            vpath.clear();
            incs.clear();
            incDirs.clear();
            localLibs.clear();
            localIncs.clear();
            commonLibs.clear();
            replace.clear();
        }
        void AddReplacement (const string & name, const string & value)
        {
            replace[name] = value;
        }
        void AddReplacement (const string & name, const char * value)
        {
            replace[name] = value;
        }
        void AddReplacement (const string & name, bool value)
        {
            replace[name] = (value ? "1" : "0");
        }
    } makefile;

    // create hard links during model configure if set
    int persist_configureOpt;
    
    int dirWalkDepth;
    
  public:
    // constructors / destructors
    ModelBuilder(const Workspace & theWorkspace, const Model & theModel,
        const string & theBuildDir);
    ~ModelBuilder();

    // top level driver methods
    /// Create the complete build tree (aka. 'configure').
    bool CreateBuildTree (int persist = 0);
    /// Remove all files in the build tree.
    bool NukeBuildTree (void);
    /// Run make on an existing build tree.
    bool RunMake (const string & compileOptions = "",
        const string & target = "");

  private:
    // methods
    void SubstitutePlaceholders (const string & inputFile,
        const string & outputFile);
    void AppendUnique (StringList & container, const string & newValue);
    bool CopyFileToBuildTree (const string & source, const string & dest);
    bool HardCopyFileToBuildTree (const string & source, const string & dest);
    bool HardCopyDirToBuildTree (const string & source, const string & dest);
    string MakePath (const ResultPath resultPath,
        const string & sourceModule, const string & modulePath,
        const string & fileName, const ExportType exportType);
    bool CreateBuildTreeForBase (void);
    bool CreateBuildTreeForModel (void);
    bool CreateBuildTreeForModule (const ModuleInstance & moduleInstance,
        const string & modulePath, const string & moduleShortPath,
        const ModParamInstanceList & inheritedParams);
    bool CreateMakefiles (void);
    bool CreateSubMakefiles ( const ModuleInstance & moduleInstance,
        const string & modulePath, const string & moduleShortPath);
    /// Create the short module path name from the long one
    string CreateShortName (const string & modPath,
        const string & modShortPath, const Module & module);
    string MakefileString (const StringList & stringList);
    bool CreateDefaultWorkbench (const Module & workbench);
    bool CreateSimConfig (void);
    bool CreateSimConfigForModule (const ModuleInstance & moduleInstance,
        ostream & simConfigFile);
    bool CreateModuleHeader (const ModuleInstance & moduleInstance,
        const ModParamInstanceList & inheritedParams);
    bool CreateDynamicParams (void);

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

#endif // _MODEL_BUILDER_ 
