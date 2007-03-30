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

// generic C
#include <glob.h>
#include <stdlib.h>
#include <unistd.h>

// generic (C++)
#include <vector>
#include <string>
#include <iostream>

// local
#include "workspace.h"
#include "util.h"

// gcc 3.2.2 complains about these lines inside the class def.
static const char* const DefaultBenchmarkDir   = "/proj/asim/benchmarks";
static const char* const DefaultBuildDir       = "build/default";
static const char* const DefaultSourceTreePath = "asim";
static const char* const DefaultCompiler       = "GCC";
static const char* const DefaultParallel       = "FALSE";
static const char* const DefaultBuildType      = "DEBUG";
static const char* const DefaultMakeFlags      = "";

/**
 * Setup the Workspace information. We figure out where the workspace
 * directory is and create a workspace object from it. In addition
 * a few sanity checks are run on the resulting workspace to make sure
 * it is usable.
 *
 * @return pointer to valid Workspace object or NULL
 */
Workspace *
Workspace::Setup (void)
{
    char * awbLocal = getenv ("AWBLOCAL");
    string workspaceDir = "";
    Workspace * workspace = NULL;


    // find the correct workspaceDir to use

    // Try environment variable $AWBLCOAL
    if (awbLocal) {
        workspaceDir = awbLocal;
    }

    // Then try looking up directory tree
    if (workspaceDir.empty()) {
        string trydir = GetCWD();
        
        while (trydir.length() > 1) {
            if (FileExists(FileJoin(trydir, "awb.config"))) {
                workspaceDir = trydir;
                break;
            }
            trydir = FileHead(trydir);
        }
    }

    // Then try ~/.asim/asimrc file
    if (workspaceDir.empty()) {
        IniFile * asimrc = new IniFile(ExpandTildeUser("~/.asim/asimrc"));

        if (asimrc) {
            workspaceDir = asimrc->Get("Global", "Workspace");
            delete asimrc;
        }
    }


    if (workspaceDir.empty()) {
        cerr << "ERROR: Could not determine a workspace. Tried looking in:" << endl;
        cerr << "ERROR:      1) Environment variable $AWBLOCAL" << endl;
        cerr << "ERROR:      2) Current directory and its parents" << endl;
        cerr << "ERROR:      3) ~/.asim/asimrc" << endl;
        exit (1);
    }

    // instantiate the mother of all workspace information
    workspace = new Workspace(workspaceDir);

    string buildDir = workspace->GetDirectory(BuildDir);
    if ( ! FileExists(buildDir)) {
        MakeDir (buildDir);
    }

    return workspace;
}

/**
 * Create a new workspace object, holding all sorts of globally visible
 * state. The state is filled in from the workspace config file located in
 * the workspaceDir.
 */
Workspace::Workspace (
    const string & workspaceDir)  ///< workspace directory
  : workspaceConfig(workspaceDir + "/awb.config")
{
    //
    // top level dir of this workspace
    //
    SetDirectory (WorkspaceDir, workspaceDir);

    //
    // BENCHMARKDIR - root of the benchmark directory tree
    //
    SetDirectory (BenchmarkDir,
        ExpandPath (
            workspaceConfig.Get ("Paths", "BENCHMARKDIR", DefaultBenchmarkDir),
            workspaceDir));

    //
    // BUILDDIR - fallback directory where performance models are built
    //
    SetDirectory (BuildDir,
        ExpandPath (
            workspaceConfig.Get ("Paths", "BUILDDIR", DefaultBuildDir),
            workspaceDir));

    //
    // Source Tree Path - directory search path for sources
    //
    string sourceTreePath =
        workspaceConfig.Get ("Paths", "SEARCHPATH", DefaultSourceTreePath);

    // generate a StringList from the split sourceTreePath for use as
    // input to ExpandPath
    StringList sourceTreePathUnexpanded;
    SplitString splitsourceTreePath(sourceTreePath, ": \t");
    FOREACH (SplitString, it, splitsourceTreePath) {
        sourceTreePathUnexpanded.push_back (*it);
    }

    // expand search tree paths
    StringList sourceTreePathList;
    ExpandPath (sourceTreePathUnexpanded, workspaceDir, sourceTreePathList);

    // setup source tree union directory
    sourceTree = new
        UnionDir(sourceTreePathList.begin(), sourceTreePathList.end(),
            UnionDir::Recursive);

    //
    // BUILDCOMPILER - compiler suite to be used for generating the model
    // (GEM or GNU)
    //
    SetBuildEnv (BuildEnvCompiler,
        workspaceConfig.Get ("Build", "COMPILER", DefaultCompiler));

    //
    // PARALLEL - use parallel make
    //
    SetBuildEnvFlag (BuildEnvFlagParallel,
        StringToBool (
            workspaceConfig.Get ("Build", "PARALLEL", DefaultParallel)));

    //
    // BUILDTYPE - build OPTIMIZE or DEBUG
    //
    string buildType =
        workspaceConfig.Get ("Build", "BUILDTYPE", DefaultBuildType);

    //------------------------------------------------------------------------
    // end parsing awb.config file
    //------------------------------------------------------------------------

    //
    // directories - Commonly used directories
    //

    // following directories are relative to SourceTree
    SetDirectory (RelSourceBaseDir,   "base");
    SetDirectory (RelSourceConfigDir, "config");
    SetDirectory (RelSourceModelDir,  "pm");

    // following directories are relative to the build directory
    SetDirectory (RelBuildSourceDir,  "src");
    SetDirectory (RelBuildIncludeDir,
        FileJoin (GetDirectory (RelBuildSourceDir), "include"));

    //
    // buildEnv -
    //     Default build environment; most of it is defined in
    //     Makefile.config

    SetBuildEnv (BuildEnvMake, "gmake");
    SetBuildEnv (BuildEnvMakeFlags,
        workspaceConfig.Get ("Build", "MAKEFLAGS", DefaultMakeFlags));
    if (StringCmpNocase (GetBuildEnv (BuildEnvCompiler), "GNU") == 0) {
        SetBuildEnvFlag (BuildEnvFlagGnu, true);
    } else {
        SetBuildEnvFlag (BuildEnvFlagGnu, false);
    }

    // for historic reasons, we currently only support debug-nooptimize
    // and nodebug-optimize combinations; there is no good reason to
    // couple them other than backward compatibility;
    if (StringCmpNocase (buildType, "DEBUG") == 0) {
        SetBuildEnvFlag (BuildEnvFlagDebug, true);
        SetBuildEnvFlag (BuildEnvFlagOptimize, false);
    } else {
        SetBuildEnvFlag (BuildEnvFlagDebug, false);
        SetBuildEnvFlag (BuildEnvFlagOptimize, true);
    }

    // following flags are supported in the build environment, but no
    // option is available to set them in the configuration file
    SetBuildEnvFlag (BuildEnvFlagStatic, false);
    SetBuildEnvFlag (BuildEnvFlagWarn, true);
}

/**
 * Release any memory we might have dynamically allocated for this object.
 */
Workspace::~Workspace()
{
    if (sourceTree) {
        delete sourceTree;
    }
}

/**
 * Get the item varName from the Vars group in the awb.config file. It the
 * item is not found, returns the default (__varName_undefined__) string
 * instead.
 *
 * @return variable contents
 */
string
Workspace::GetConfVar (
    const string & varName)
const
{
    // get item varName from Vars group
    return workspaceConfig.Get("Vars", varName,
               string("__") + varName + string("_undefined__"));
}

/**
 * Wrapper to return single value (1st value of result list) for
 * ExpandPath of a single path expression.
 *
 * @return first item of expanded list
 */
string
Workspace::ExpandPath (
    const string & path,     ///< path to expand
    const string & baseDir)  ///< base directory for relative paths
const
{
    StringList pathList;
    StringList expanded;

    pathList.push_back(path);
    ExpandPath(pathList, baseDir, expanded);
    if (expanded.empty()) {
        return "";
    } else {
        return expanded[0];
    }
}

/**
 * Workspace configuration files support a rather complicated syntax for
 * path expressions. This method handles the expansion of the path
 * expression to a path of absolute directory names.
 *
 * BUGS: this is not exactly coded for high performance :)
 */
void
Workspace::ExpandPath (
    const StringList & pathList, ///< path to expand
    const string & baseDir,      ///< base directory for relative paths
    StringList & expanded)       ///< returns expanded directory list
const
{
    //
    // round 1: expand variables and make option crossproducts
    //
    // invariants:
    //   - the paths we need to expand are in pathList
    //   - the result paths will get collected in round1OutPathList
    //
    StringList round1OutPathList;

    FOREACH_CONST (StringList, it, pathList) {
        string path = *it;

        // invariant: the 1 path we need to expand is in path

        // expand all variables (supports symbolic variable expansion, ie.
        // variables containing variable names)
        const char* varMatch = "\\$[{(]([^})]+)[})]";
        while (true) {
            MatchString matchString(path);
            MatchString::MatchArray matchArray;
            if (matchString.Match(varMatch, matchArray).empty()) {
                break;
            } else {
                string var = GetConfVar(matchArray[1]);
                path = matchString.Substitute(varMatch, 0, var);
            }
        }

        // expand {} option specifier; this can turn 1 path into many
        // invariants:
        //   - the 1 path we need to expand is in path
        //   - the result paths will be collected in round1OutPathList
        const char* optionMatch = "[{]([^}]*)[}]";
        MatchString matchString(path);
        MatchString::MatchArray matchArray;
        if (matchString.Match(optionMatch, matchArray).empty()) {
            // no option cross product to expand - out = in
            round1OutPathList.push_back(path);
        } else {
            StringList localPathList;
            SplitString splitString(matchArray[1], ",");
            FOREACH (SplitString, it2, splitString) {
                localPathList.push_back (
                    matchString.Substitute(optionMatch, 0, *it2));
            }
 
            // now of course dir might have more {} - recurse
            // FIXME: we should do this with local iteration instead
            ExpandPath(localPathList, "", round1OutPathList);
        }
    }

    // invariant: result is in round1OutPathList

    //
    // round 2: expand ~user and relative paths
    //
    // invariants:
    //   - the input paths are in round2InPathList
    //   - the result paths will get collected in round2OutPathList
    //
    StringList & round2InPathList = round1OutPathList;
    StringList round2OutPathList;
    FOREACH_CONST (StringList, it, round2InPathList) {
        // expand ~user to home directory
        string path = ExpandTildeUser(*it);

        // prefix relative paths with basedir
        if (! baseDir.empty() && IsRelativePath(path)) {
            path = baseDir + "/" + path;
        }
        round2OutPathList.push_back(path);
    }

    // invariant: result is in round2OutPathList

    //
    // round 3: expand * and ? globs, but only take directories
    //
    // invariants:
    //   - the input paths are in round3InPathList
    //   - the result paths will get collected in round3OutPathList
    //
    StringList & round3InPathList = round2OutPathList;
    StringList & round3OutPathList = expanded;

    FOREACH_CONST (StringList, it, round3InPathList) {
        // expand * and ?
        const char* globMatch = "[*?]";
        MatchString matchString(*it);
        if (matchString.Match(globMatch).empty()) {
            // no globbing needed
            round3OutPathList.push_back(*it);
        } else {
            // glob for directory names
            glob_t globbuf;  // interface to libc glob

            glob((*it).c_str(), 0, NULL, &globbuf);

            for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
                char* fileName = globbuf.gl_pathv[i];

                if (FileIsDirectory(fileName)) {
                    round3OutPathList.push_back(fileName);
                }
            }
        }
    }

    // invariant: result is in round3OutPathList
    // invariant: round3OutPathList == expanded
}

/**
 * Search for abstract directory name in the directory map and return
 * its value if found. Otherwise return empty string.
 *
 * @return the concrete directory name
 */
string
Workspace::GetDirectory (
    const DirectoryName name)  ///< name of the directory to look for
const
{
    ASSERT (name < LAST_DIR, "range overflow in directory access");
    return directory[name];
}

/**
 * Set abstract directory name to value.
 */
void
Workspace::SetDirectory (
    const DirectoryName name,  ///< name of the directory to set
    const string & value)      ///< value to set this directory name to
{
    ASSERT (name < LAST_DIR, "range overflow in directory access");
    directory[name] = value;
}

/**
 * Search for abstract build environment name and return its value if
 * found. Otherwise return empty string.
 *
 * @return the concrete build environment string
 */
string
Workspace::GetBuildEnv (
    const BuildEnvName name) ///< name of build environment to look for
const
{
    ASSERT (name < LAST_BUILD_ENV, "range overflow in buildEnv access");
    return buildEnv[name];
}

/**
 * Set abstract build environment name to value.
 */
void
Workspace::SetBuildEnv (
    const BuildEnvName name,  ///< name of the build environment to set
    const string & value)     ///< value to set this build environment to
{
    ASSERT (name < LAST_BUILD_ENV, "range overflow in buildEnv access");
    buildEnv[name] = value;
}

/**
 * Search for abstract build environment boolean flag name and return its
 * value. Otherwise return false.
 *
 * @return the concrete build environment flag
 */
bool
Workspace::GetBuildEnvFlag (
    const BuildEnvFlagName name) ///< name of the build environment flag 
                                 ///< to look for
const
{
    ASSERT (name < LAST_BUILD_ENV_FLAG,
            "range overflow in buildEnvFlag access");
    return buildEnvFlag[name];
}

/**
 * Set abstract build environment flag name to value.
 */
void
Workspace::SetBuildEnvFlag (
    const BuildEnvFlagName name, ///< name of build environment flag to set
    const bool value)     ///< value to set this build environment flag to
{
    ASSERT (name < LAST_BUILD_ENV_FLAG,
            "range overflow in buildEnvFlag access");
    buildEnvFlag[name] = value;
}


/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
Workspace::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    int count;

    out << prefix << "Workspace::" << endl;

    out << prefix << "  Directory:" << endl;
    count = 0;
    out << prefix << "    BenchmarkDir: "
        << GetDirectory (BenchmarkDir) << endl;
    count++;
    out << prefix << "    BuildDir: " 
        << GetDirectory (BuildDir) << endl;
    count++;
    out << prefix << "    RelSourceBaseDir: " 
        << GetDirectory (RelSourceBaseDir) << endl;
    count++;
    out << prefix << "    RelSourceConfigDir: " 
        << GetDirectory (RelSourceConfigDir) << endl;
    count++;
    out << prefix << "    RelSourceModelDir: " 
        << GetDirectory (RelSourceModelDir) << endl;
    count++;
    out << prefix << "    RelBuildSourceDir: " 
        << GetDirectory (RelBuildSourceDir) << endl;
    count++;
    out << prefix << "    RelBuildIncludeDir: " 
        << GetDirectory (RelBuildIncludeDir) << endl;
    count++;
    if (count != LAST_DIR) {
        out << prefix << "    WARNING: missing data for Directory" << endl;
    }

    out << prefix << "  BuildEnv:" << endl;
    count = 0;
    out << prefix << "    Compiler: "
        << GetBuildEnv (BuildEnvCompiler) << endl;
    count++;
    out << prefix << "    Make: "
        << GetBuildEnv (BuildEnvMake) << endl;
    count++;
    if (count != LAST_BUILD_ENV) {
        out << prefix << "    WARNING: missing data for BuildEnv" << endl;
    }

    out << prefix << "  BuildEnvFlag:" << endl;
    count = 0;
    out << prefix << "    Parallel: "
        << GetBuildEnvFlag (BuildEnvFlagParallel) << endl;
    count++;
    out << prefix << "    Gnu: "
        << GetBuildEnvFlag (BuildEnvFlagGnu) << endl;
    count++;
    out << prefix << "    Debug: "
        << GetBuildEnvFlag (BuildEnvFlagDebug) << endl;
    count++;
    out << prefix << "    Optimize: "
        << GetBuildEnvFlag (BuildEnvFlagOptimize) << endl;
    count++;
    out << prefix << "    Static: "
        << GetBuildEnvFlag (BuildEnvFlagStatic) << endl;
    count++;
    out << prefix << "    Warn: "
        << GetBuildEnvFlag (BuildEnvFlagWarn) << endl;
    count++;
    if (count != LAST_BUILD_ENV_FLAG) {
        out << prefix << "    WARNING: missing data for BuildEnvFlag" << endl;
    }

    out << prefix << "  SourceTree:" << endl;
    sourceTree->Dump (out, prefix + "    ");

    return out;
}

//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

void TestExpandPath (char * workspaceDir, char * path, char * baseDir)
{
    Workspace workspace(workspaceDir);
    Workspace::StringList pathList;
    Workspace::StringList expandedList;

    pathList.push_back(path);

    workspace.ExpandPath(pathList, baseDir, expandedList);

    cout << "path: " << path << endl;
    FOREACH (Workspace::StringList, it, expandedList) {
        cout << "expanded: " << *it << endl;
    }
}

void TestConstructor (char * workspaceDir)
{
    Workspace workspace(workspaceDir);

    workspace.Dump(cout);
}

void TestSetup (void)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if (workspace) {
        workspace->Dump(cout);
        delete workspace;
    } else {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
    }

    cout << "Workspace::Setup:: done" << endl;
}

int main (int argc, char ** argv)
{
#if 0
    if (argc >= 4) {
        TestExpandPath (argv[1], argv[2], argv[3]);
    }
#endif

    if (argc >= 2) {
        TestConstructor (argv[1]);
    }

#if 0
    if (argc >= 1) {
        TestSetup();
    }
#endif
}

#endif // TESTS 
