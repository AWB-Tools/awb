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

// generic (C)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// generic (C++)
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

// local
#include "model_builder.h"
#include "util.h"

// gcc 3.2.2 complains about these lines inside the class def.
static const char * const IncludeExtension = "\\.(h|H|hh|hpp|hxx|def)$";
static const char * const SourceExtension =  "\\.(c|C|cc|cpp|cxx)$";
static const char * const ConfigExtension =  "\\.(pack)$";
static const char * const DoxygenExtension = "\\.(dox|doxy)$";
static const char * const RawExtension =     "\\.(tcl|cfg|r)$";

/**
 *
 */
ModelBuilder::ModelBuilder (
    const Workspace & theWorkspace,
    const Model & theModel,
    const string & theBuildDir)
  : workspace(theWorkspace),
    sourceTree(workspace.GetSourceTree()),
    model(theModel),
    buildDir(theBuildDir)
{
    persist_configureOpt = 0;
    dirWalkDepth = 0;
}

/**
 *
 */
ModelBuilder::~ModelBuilder()
{
    // nada
}

/**
 * This is the top-level driver that oversees and sequences the creation
 * of the build tree.
 */
bool ///< returns true for success, false otherwise
ModelBuilder::CreateBuildTree (int persist)
{
    persist_configureOpt = persist;

    // during this build tree configuration process, several methods
    // collect information for later methods to use; so before we start we
    // need to clear all this information;
    makefile.clear();
    dynamicParams.clear();

    // go through all steps of creating a build tree
    //
    // Note: the order of these calls is significant, since the early
    // calls collect per-file/module information into makefile and
    // dynamicParams, and the later calls consume this information;
    //
    // Note: the following is short-circuit evaluated, ie. we bail out
    // after the first error;
    return (
        CreateBuildTreeForBase() &&
        CreateBuildTreeForModel() &&
        CreateDynamicParams() &&
        CreateSimConfig() &&
        CreateMakefiles()
    );
}

/**
 * Remove all files in the build tree - nuke!
 */
bool
ModelBuilder::NukeBuildTree (void)
{
    if ( ! FileExists (buildDir)) {
        // nothing to do if build tree is missing altogether
        return true;
    }

    // remove all files in buildRoot (but not buildRoot itself)
    bool success = RemoveDir (buildDir, false);
    if ( ! success) {
        cerr << "Error nuking build tree" << endl;
        return false;
    }

    return true;
}

/**
 * Given an existing build tree, this will run make on that tree.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::RunMake (
    const string & makeOptions, ///< extra options for make command line
    const string & target)      ///< target for make
{
    bool success = true;
    //
    // check that there is a makefile to execute
    //
    if ( ! FileExists ( FileJoin (buildDir, "Makefile"))) {
        cerr << "Error: Can't build model " << model.GetName() << endl;
        cerr << "       No Makefile found in build directory " << buildDir
             << endl;
        return false;
    }

    //
    // Determine parallel make flags
    //
    string parallelOptions;
    if (workspace.GetBuildEnvFlag (Workspace::BuildEnvFlagParallel)) {
        // FIXME: harcoding 4 is a bad idea!
        parallelOptions = "-j 4";
    }

    //
    // Determine default make flags
    //
    string makeFlags = workspace.GetBuildEnv (Workspace::BuildEnvMakeFlags);
    
    //
    // good to go - now make
    //
    string cwd = GetCWD();
    if (chdir (buildDir.c_str())) {
        cerr << "Error: Can't cd to build directory " << buildDir << endl;
        return false;
    }
    string cmd = workspace.GetBuildEnv (Workspace::BuildEnvMake)
        + " -f Makefile " + parallelOptions + " " + makeFlags + " " + makeOptions + " " + target;
    if (system (cmd.c_str()) != 0) {
        cerr << "Error: Build of model " << model.GetName() << " failed"
             << endl;
        success = false;
    }
    if (chdir (cwd.c_str()) && success) {
        cerr << "Error: Can't cd back to previous working directory "
             << cwd << endl;
        return false;
    }

    return success;
}


/**
 * Support routine to create apporpriately formated string for a Makefile
 * variable from a StringList.
 *
 * @return formatted string
 */
string
ModelBuilder::MakefileString (
    const StringList & stringList) ///< StringList to format into string
{
    ostringstream os;

    copy (stringList.begin(), stringList.end(),
        ostream_iterator<string>(os, " "));
    return StringLineWrap (StringTrim (os.str()), 74, "\\", "  ");
}

/**
 * Handle Makefile creation.
 *
 * Implicit top-level build assumptions:
 *   - top-level Makefile is in pm/Makefile.template in source tree
 *   - top-level Makefile config is in pm/Makefile.config
 *   - top-level build tree will contain (verbatim) copy of 
 *     Makefile.config and a Makefile derived from
 *     Makefile.template by substituting all $RELACE$FOO strings
 *     with this ASIM builders' notion of FOO
 */
bool ///< returns true for success, false otherwise
ModelBuilder::CreateMakefiles (void)
{
    bool success;

    //
    // setup include dir for base header files
    //
    // find the name of the public include directories for ASIM includes
    string relDestFileName = 
        MakePath (DestFileRel, "", "", "foo", Public);
    string relDestDirName = FileHead (relDestFileName);
    string relIncDir = string("-I") + relDestDirName;
    //
    // setup replacement strings for all Makefiles
    //
    makefile.AddReplacement("PmName", model.GetName());
    makefile.AddReplacement("PmDesc", model.GetDesc());

    makefile.AddReplacement("ASIM", true);
    makefile.AddReplacement("GNU",    workspace.GetBuildEnvFlag(
                                      Workspace::BuildEnvFlagGnu));
    makefile.AddReplacement("OPT",    workspace.GetBuildEnvFlag(
                                      Workspace::BuildEnvFlagOptimize));
    makefile.AddReplacement("DEBUG",  workspace.GetBuildEnvFlag(
                                      Workspace::BuildEnvFlagDebug));
    makefile.AddReplacement("WARN",   workspace.GetBuildEnvFlag(
                                      Workspace::BuildEnvFlagWarn));
    makefile.AddReplacement("STATIC", workspace.GetBuildEnvFlag(
                                      Workspace::BuildEnvFlagStatic));
    makefile.AddReplacement("PAR",    workspace.GetBuildEnvFlag(
                                      Workspace::BuildEnvFlagParallel));
    makefile.AddReplacement("INCS", MakefileString (makefile.incs));
    makefile.AddReplacement("SRCS", MakefileString (makefile.srcs));
    makefile.AddReplacement("OBJS", MakefileString (makefile.objs));
    makefile.AddReplacement("LOCAL_LFLAGS", MakefileString (makefile.localLibs));
    makefile.AddReplacement("LOCAL_CFLAGS", MakefileString (makefile.localIncs));
    makefile.AddReplacement("COMMON_LFLAGS", MakefileString (makefile.commonLibs));
    makefile.AddReplacement("VPATH", MakefileString (makefile.vpath));
    makefile.AddReplacement("BUILDABSROOT", buildDir);

    //
    // Handle sub-Makefiles:
    //
    const ModuleInstance * rootModuleInstance = model.GetRootModule();
    const Module & rootModule = rootModuleInstance->GetModule();
    const string & rootModuleLocation = rootModule.GetLocation();
    const string rootModulePath = "system";
    const string rootModuleShortPath = "system";
    string rootMakefileConfig = "Makefile.config";
    Module::StringList rootMakefileConfigList = rootModule.GetMakefile();
    if (rootMakefileConfigList.size() > 1) {
	rootMakefileConfig = rootMakefileConfigList[1];
	// copy other files on the %makefile argument list
	for (unsigned i = 2; i < rootMakefileConfigList.size(); ++i) {
	    if (rootMakefileConfigList[i] != "Makefile") // don't overwrite template
		FileCopy (sourceTree.FullName(FileJoin(workspace.GetDirectory 
						       (Workspace::RelSourceModelDir),
						       rootMakefileConfigList[i])), 
			  buildDir);
	    else {
		cout << "WARNING: %makefile argument \"" 
		     << rootMakefileConfigList[i]
		     << "\" not copied." 
		     << endl;
	    }
	}
    }
    
    success = CreateSubMakefiles (*rootModuleInstance,
        rootModulePath, rootModuleShortPath);
    if ( ! success) {
        return false;
    }

    //
    // Handle top-level Makefile:
    //
    // reset replacement strings that might have been modified by submakes
    // or were not set yet;
    //
    makefile.AddReplacement("BUILDROOT", "");
    makefile.AddReplacement("INCDIRS", relIncDir);
    makefile.AddReplacement("SUBDIRS", MakefileString (makefile.subDirs));
    makefile.AddReplacement("SUBTARGETS", MakefileString (makefile.subTargets));
    makefile.AddReplacement("MAKEFILECONFIG", rootMakefileConfig);
    string makefileConfigFileName = MakePath (SourceFile, rootModuleLocation, rootModuleShortPath,
					      rootMakefileConfig, Source);
    if (makefileConfigFileName.empty()) {
	cerr << "ERROR: can't find user specified top-level Config Makefile: "
	     << rootMakefileConfig << endl;
	cerr << "       specified in Module '" << rootModule.GetFileName()
	     << "' named '" << rootModule.GetName() << endl;
	return false;
    }
    
    // copy generic makefile config to build tree
    // look for it in directory where awb is
    string makefileConfigSource = sourceTree.FullName ( 
						       FileJoin (rootModuleLocation, 
								 rootMakefileConfig));
    FileCopy (makefileConfigSource, buildDir);
    

    //
    // determine the source and destination makefile paths
    //
    string makefileSourceFileName;
    if (rootModule.GetMakefile().size()!=0) {
        makefileSourceFileName =
            MakePath (SourceFile, rootModuleLocation, rootModuleShortPath,
		      rootModule.GetMakefile().front(), Source);
        if ( makefileSourceFileName == "" ) {
            cerr << "ERROR: can't find user specified top-level Makefile "
                 << rootModule.GetMakefile().front() << endl;
            cerr << "       specified in Module '" << rootModule.GetFileName()
                 << "' named '" << rootModule.GetName() << endl;
            return false;
        }
    } else {
        makefileSourceFileName =
            FileJoin (workspace.GetDirectory (Workspace::RelSourceModelDir),
                      "Makefile.template");
    }

    // copy makefile template to built tree and substitute placeholders
    string makefileTemplate = sourceTree.FullName ( makefileSourceFileName );
    string makefileDest = FileJoin (buildDir, "Makefile");

    // allow overriding of top-level target name as well
    // (just for symmetry reasons with other uses of %makefile & %target)
    Module::StringList rootModuleTarget = rootModule.GetTarget();
    if (rootModuleTarget.begin() != rootModuleTarget.end()) {
        // note: we implicityly only use the first target name (in case
        // there are more than one) and silently drop the others
        makefile.AddReplacement("TARGET", *(rootModuleTarget.begin()));
    } else {
        makefile.AddReplacement("TARGET", model.GetFileName());
    }

    // now substitute placeholders and copy file
    SubstitutePlaceholders (makefileTemplate, makefileDest);

    return true;
}

/**
 * Substitute all replacement strings in Makefile templates. Makefile
 * templates can contain special strings of the form $REPLACE$foo
 * which will be replaced here with the contents of the replacement string
 * that has been set up for 'foo'.
 */
void
ModelBuilder::SubstitutePlaceholders (
    const string & inputFile,  ///< name of input Makefile template
    const string & outputFile) ///< name of output Makefile
{
    // open input file
    ifstream in(inputFile.c_str());
    if ( ! in) {
        cerr << "Error: SubstitutePlaceholder can't open input file "
             << inputFile << endl;
        exit(1);
    }

    // open output file
    ofstream out(outputFile.c_str());
    if ( ! out) {
        cerr << "Error: SubstitutePlaceholder can't open output file "
             << outputFile << endl;
        exit(1);
    }

    // copy and substitute placeholders
    string line;
    const char * const replaceRegexp = "\\$REPLACE\\$([^[:space:]]+)";
    while ( ! in.eof()) {
        getline (in, line);
        if (line.empty() && in.eof()) {
            break; // also eof
        }
        MatchString matchLine(line);
        MatchString::MatchArray matchArray;

        if ( ! matchLine.Match(replaceRegexp, matchArray).empty()) {
            const string & token = matchArray[1];
            string replacement;

            if (makefile.replace.find(token) != makefile.replace.end()) {
                replacement = makefile.replace[token];
            } else {
                replacement = string("undefined token ") + token;
            }
            line = matchLine.Substitute (replaceRegexp, 0, replacement);
        }
        out << line << endl;
    }

    // cleanup
    in.close();
    out.close();
}


/**
 * Append a new string to an existing string list, iff the new string is
 * not already in the string list.
 */
void
ModelBuilder::AppendUnique (
    StringList & container,
    const string & newValue)
{
    if (find (container.begin(), container.end(), newValue)
          == container.end())
    {
        container.push_back (newValue);
    }
}

/**
 * Hard copy a file from source to destination.  
 * Must handle non-directory source files.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::HardCopyFileToBuildTree(const string & sourceFileName, 
				      const string & destFileName)
{
    int nlevels = 0, size = 0;
    char buffer[256];	    
    string symlinkFileName;
    string sourceFileFullName = sourceFileName;
    
    while (FileIsSymLink(sourceFileFullName)) {
	if (++nlevels > 8) {
	    cout << " WARNING: Creating hard link failed : Symbolic link too deep on ";
	    cout << " src: " << sourceFileFullName.c_str() << "'" << ", dest: '" 
		 << destFileName.c_str() << "'" <<endl;
	    return false;
	}
	size = readlink(sourceFileFullName.c_str(), buffer, 256);
	if (size < 0) {
	    perror(" WARNING: Creating hard link failed : Cannot resolve symbolic link ");
	    cout << " src: " << sourceFileFullName.c_str() << "'" << ", dest: '" 
		 << destFileName.c_str() << "'" <<endl;
	    return false;
	}
	else if (size >= 256) {
	    cout << " WARNING: Creating hard link failed : Link name too long on ";
	    cout << " src: " << sourceFileFullName.c_str() << "'" << ", dest: '" 
		 << destFileName.c_str() << "'" <<endl;
	    return false;
	}
	buffer[size] = '\0';
	symlinkFileName = buffer;
	if (IsAbsolutePath(symlinkFileName)) {
	    sourceFileFullName = symlinkFileName;
	}
	else {
	    sourceFileFullName = FileJoin(FileDirName(sourceFileFullName), symlinkFileName);
	}
    }

    if (link(sourceFileFullName.c_str(), destFileName.c_str()) == -1) {
	perror(" WARNING: Creating hard link failed "); 
	symlink (sourceFileFullName.c_str(), destFileName.c_str());
	cout << " Created symbolic link between src: '" << sourceFileFullName.c_str() << "'" << ", dest: '" 
	     << destFileName.c_str() << "'" << endl;
    }
    return true;
}

/**
 * Copy a dir from source to destination and create destination
 * directory if necessary.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::HardCopyDirToBuildTree(const string & sourceFileName, 
				     const string & destFileName)
{
    // Is recursion too deep?
    struct _depthChecker {
	int &depth;
	_depthChecker(int &d) : depth(d){
	    depth++;
	}
	~_depthChecker() {
	    depth--;
	}
    } depthChecker(dirWalkDepth);
    if (dirWalkDepth > 16) {
	cout << "ERROR: Cannot descend into src '" << sourceFileName << "', to create dest: '"
	     << destFileName << "'.  Recursion too deep." << endl;
	return false;
    }
    ////////////////////

    DIR *dir;
    struct dirent *dirEnt;

    if ((dir = opendir(sourceFileName.c_str())) == 0) {
	perror(" WARNING: Creating hard link failed : Cannot open dir to read ");
	cout << " src: " << sourceFileName << "'" << ", dest: '" 
	     << destFileName.c_str() << "'" <<endl;
	return false;
    }

    MakeDir(destFileName);
    
    while ((dirEnt = readdir(dir)) != 0) {
	string thisSource = FileJoin(sourceFileName, dirEnt->d_name);
	string thisDest = FileJoin(destFileName, dirEnt->d_name);
	if (FileIsDirectory(thisSource)) {
	    if ((strcmp(dirEnt->d_name, ".") == 0) || (strcmp(dirEnt->d_name, "..") == 0) || (strcmp(dirEnt->d_name, ".svn") == 0)) {
		continue;
	    }
	    HardCopyDirToBuildTree(thisSource, thisDest);
	}
	else {
	    HardCopyFileToBuildTree(thisSource, thisDest);
	}
    }
    closedir(dir);
    
    return true;
}

/**
 * Copy a file from source to destination and create destination
 * directory if necessary.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CopyFileToBuildTree (
    const string & sourceFileName, ///< source file name
    const string & destFileName)   ///< destination file name
{
    MakeDir (FileHead (destFileName));
    unlink (destFileName.c_str());

    string sourceFileFullName = sourceTree.FullName (sourceFileName);

    bool fileIsEmpty = sourceFileFullName.empty();
    bool fileExists = FileExists (sourceFileFullName);

    if (fileExists) {
	if (persist_configureOpt) {
	    if (FileIsDirectory(sourceFileFullName)) {	// creating hard links to directories may not work
		return (HardCopyDirToBuildTree(sourceFileFullName, destFileName));
	    }
	    else {
		HardCopyFileToBuildTree(sourceFileFullName, destFileName);
	    }
	}
	else {
	    symlink (sourceFileFullName.c_str(), destFileName.c_str());
	}
        return true;
    } else {
        cout << "WARNING: can not copy source file" << endl;
        cout << "         " << sourceFileName << endl;
        cout << "         to build tree." << endl;
        
        if (fileIsEmpty) {
            cout << "         Source file not found in source tree." << endl;
        } else if (! fileExists) {
            cout << "         Source file" << endl;
            cout << "         " << sourceFileFullName << endl;
            cout << "         does not exist." << endl;
        }
        return false;
    }
}

/**
 * Make source and destination paths for module files, depending on what
 * our target structure should look like.
 * This method makes sure we have a consistent view of how files move
 * between source and build tree. Every time you need to find out about
 * the location of a file, query this method. DO NOT make up your own
 * paths to files anywhere else, since locations might move around in
 * future versions of the builder and only this method is guaranteed to
 * give the right location.
 *
 * Supported taget structures:
 *   - Verbatim: verbatim copy of source structure
 *   - Tree:     tree structure based on requires-provides relationship
 *
 * @note: Only the Tree target structure is really supported for build
 * purposes. The Verbatim structure solely remains here for debugging
 * purposes.
 *
 * @return the generated path
 */
string
ModelBuilder::MakePath (
    const ResultPath resultPath, ///< type of result path to return
    const string & sourceDir,    ///< relative path from sourceTree to 
                                 ///< directory containing source file
    const string & moduleDir,    ///< relative path from buildDir to directory
                                 ///< of module sources in build tree
                                 ///< (encodes requires-provides hierarchy)
    const string & fileName,     ///< basename of file to handle
    const ExportType exportType) ///< determines where in the build tree the
                                 ///< file will be located
{
    string sourceFile;
    string destDirRel;
    string destFileRel;
    string destFile;

    if (targetStructure == Tree) {
        // tree structure based on requires-provides relationship
        sourceFile = sourceTree.FullName (FileJoin (sourceDir, fileName));

        switch (exportType) {
          case Source: // ie. a C++ (.cpp) file
            destDirRel = FileJoin (
                workspace.GetDirectory (Workspace::RelBuildSourceDir),
                moduleDir);
            break;
          case Public: // ie. a public header file
            destDirRel = workspace.GetDirectory (Workspace::RelBuildIncludeDir);
            break;
          case Synthesized: // ie. a synthesized header file
            destDirRel = FileJoin (
                workspace.GetDirectory (Workspace::RelBuildIncludeDir),
                "asim/provides");
            break;
          case Restricted: // ie. a private header file
            destDirRel = FileJoin (
                workspace.GetDirectory (Workspace::RelBuildIncludeDir),
                "asim/restricted");
            break;
          case Base: // ie. a file in ASIM's base directory
            destDirRel = FileJoin (
                workspace.GetDirectory (Workspace::RelBuildIncludeDir),
                "asim");
            break;
          case Doxygen: // ie. a doxygen config file
            // For generating destination directory, Doxygen files are
            // like source files
            destDirRel = FileJoin (
                workspace.GetDirectory (Workspace::RelBuildSourceDir),
                moduleDir);
            break;
          case Raw:
            // For generating destination directory, raw files are like
            // source files
            destDirRel = FileJoin (
                workspace.GetDirectory (Workspace::RelBuildSourceDir),
                moduleDir);
            break;
          default:
            cerr << "MakePath: unknown export type " << exportType << endl;
            exit (1);
        }

        destFileRel = FileJoin (destDirRel, fileName);
        destFile = FileJoin (buildDir, destFileRel);

    } else if (targetStructure == Verbatim) {
        // destination uses same directory hierarchy as found in source tree
        sourceFile = sourceTree.FullName (FileJoin (sourceDir, fileName));
        destDirRel = FileJoin (
            workspace.GetDirectory (Workspace::RelBuildSourceDir), sourceDir);
        destFileRel = FileJoin (destDirRel, fileName);
        destFile = FileJoin (buildDir, destFileRel);
    } else {
        cerr << "MakePath: unknown target structure " << targetStructure
             << endl;
        exit (1);
    }

    switch (resultPath) {
      case SourceFile:  return sourceFile;  break;
      case DestFile:    return destFile;    break;
      case DestFileRel: return destFileRel; break;
      default:
        cerr << "MakePath: unknown result path type " << resultPath << endl;
        exit (1);
    }

    return ""; // always return something to keep compiler warnings happy
}


/**
 * Create the build tree for the ASIM base directory. All models need
 * these files.
 *
 * @note: This method also collects information about its module that is
 * required for later processing steps (e.g. Makefile creation). The
 * information is collected in 'global' member variables of this
 * ModelBuilder instance.
 *
 * @note: This method does not depend on any particular model getting
 * build, as all models use the same base directory infrastructure.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateBuildTreeForBase (void)
{
    bool success;

    UnionDir::StringList baseDirList;
    sourceTree.Glob (FileJoin (
          workspace.GetDirectory(Workspace::RelSourceBaseDir), "*"),
          baseDirList);

    FOREACH_CONST (UnionDir::StringList, it, baseDirList) {
        string fileName = FileTail (*it);
        MatchString matchFileName(fileName);
        ExportType exportType;

        // determine the appropriate export type based on extension
        if ( ! matchFileName.Match(IncludeExtension).empty()) {
            // header files
            exportType = Base;
        } else if ( ! matchFileName.Match(SourceExtension).empty()) {
            // C/C++ source files
            exportType = Source;
        } else if ( ! matchFileName.Match(DoxygenExtension).empty()) {
            // Dyxygen data and config files
            exportType = Doxygen;
        } else if ( ! matchFileName.Match(RawExtension).empty()) {
            exportType = Raw;
        } else {
            // happens e.g. for CVS directory in base
            continue; // next file
        }

        // absolute filename to source
        string sourceFileName = 
            MakePath (SourceFile, "base", "base",
                      fileName, exportType);
        if (sourceFileName.empty()) {
            cout << "WARNING: source file " << fileName
                 << "does not exist in base" << endl;
            cout << "         ... not copying" << endl;
            continue; // next file
        }
        // absolute filename to destination
        string destFileName =
            MakePath (DestFile, "base", "base",
                      fileName, exportType);
        // relative filename to destination
        string destRelFileName =
            MakePath (DestFileRel, "base", "base",
                      fileName, exportType);
        // relative path to destination directory
        string destRelDirName = FileHead (destRelFileName);

        //
        // finally we can copy source to destination
        //
        success = CopyFileToBuildTree (sourceFileName, destFileName);
        if ( ! success) {
            cout << "WARNING: could not copy file " << endl;
            cout << "         " << fileName << endl;
            cout << "from base to build tree" << endl;
            cout << endl;
            continue; // next file
        }

        //
        // collect information this file for Makefile
        //
        if (exportType == Source) {
            const string & source = destRelFileName;
            const string object = FileRoot (FileTail (source)) + ".o";

            makefile.srcs.push_back (source);
            makefile.objs.push_back (object);
            AppendUnique (makefile.vpath, destRelDirName);
        } else if (exportType == Base) {
            makefile.incs.push_back (destRelFileName);
            AppendUnique (makefile.incDirs, string("-I") + destRelDirName);
        } else if (exportType == Doxygen) {
            // Once the doxygen file has been copied, we don't want to add it 
            // to the list of source, object, or include files in the makefile
            continue; // next file
        } else if (exportType == Raw) {
            continue; // next file
        } else {
            cerr << "CreateBuildTree: unknown export type " << exportType
                 << endl;
            return false;
        }
    }

    return true;
}


/**
 * Create the build tree for the model. All modules are handled
 * recursively, starting with the root module.
 *
 * @note: This method also collects information about its module that is
 * required for later processing steps (e.g. Makefile creation). The
 * information is collected in 'global' member variables of this
 * ModelBuilder instance.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateBuildTreeForModel (void)
{
    ModParamInstanceList noParams;
    const ModuleInstance * rootModule = model.GetRootModule();

    if ( ! rootModule) {
        cerr << "Error: no root module found in model " << model.GetName()
             << endl;
        cerr << "       Can't create build tree" << endl;
        return false;
    }

    if ( ! rootModule->IsComplete(true)) {
        cerr << "Error: incompletly instantiated model " << model.GetName()
             << endl;
        cerr << "       Can't create build tree" << endl;
        return false;
    }

    // root module is located in system and has no inherited params;
    // go create root module and sub-modules recursively
    return CreateBuildTreeForModule (*rootModule, "system", "system", noParams);
}


/**
 * Create the build tree for the specified module (and recursively for all
 * its submodules).
 *
 * @note: This method also collects information about its module that is
 * required for later processing steps (e.g. Makefile creation). The
 * information is collected in 'global' member variables of this
 * ModelBuilder instance.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateBuildTreeForModule (
    const ModuleInstance & moduleInstance,  ///< module that is handled here
    const string & modulePath,
    const string & moduleShortPath,
    const ModParamInstanceList & inheritedParams) ///< params from ancestors
{
    const Module & module = moduleInstance.GetModule();
    bool success;

    //
    // Create the header for this module. The header's name is
    // determined by what the module provides.
    //
    success = CreateModuleHeader (moduleInstance, inheritedParams);
    if ( ! success) {
        return false;
    }

    //
    // copy all the header and source files.
    //
    StringList files = module.GetPublic();
    const Module::StringList & privateFiles = module.GetPrivate();
    files.insert (files.end(), privateFiles.begin(), privateFiles.end());

    FOREACH_CONST (StringList, it, files) {
        string fileName = *it;
        MatchString matchFileName(fileName);
        ExportType exportType;

        // determine the appropriate export type based on extension
        if ( ! matchFileName.Match(IncludeExtension).empty()) {
            // header files
            exportType = Restricted;
        } else if ( ! matchFileName.Match(SourceExtension).empty()) {
            // C/C++ source files
            exportType = Source;
        } else if ( ! matchFileName.Match(ConfigExtension).empty()) {
            // certain config files are not processed any further; they
            // are not supposed to be needed during the build process;
            return true;
        } else if ( ! matchFileName.Match(DoxygenExtension).empty()) {
            // If the doxgen files are always in base, we don't need to
            // process them here, but if they appear in a private or
            // public for some module, they need to be copied to the build
            // tree.
            exportType = Doxygen;
        } else if ( ! matchFileName.Match(RawExtension).empty()) {
            exportType = Raw;
        } else {
            cerr << "CreateBuildTreeForModule: Unknown file extension on file: "
                 << fileName << endl;
            return false;
        }

        //
        // create all file names we need later
        //
        // absolute filename to source
        const string & moduleLocation = module.GetLocation();
        string sourceFileName = 
            MakePath (SourceFile, moduleLocation, moduleShortPath,
                      fileName, exportType);
        if (sourceFileName.empty()) {
            cout << "WARNING: source file " << fileName << endl;
            cout << "           does not exist in module '"
                 << module.GetName() << "'" << endl;
            cout << "           ... not copying" << endl;
            cout << "         Probable error in module config file" << endl;
            cout << "           " << module.GetFileName() << endl;
            cout << "           %private or %public directives" << endl;
            cout << endl;
            continue; // next file
        }

        // strip potential subdirectory from file name; (this is OK since
        // we already have created an absolute path to the source file)
        // we need to do this stripping since we do not want to get such
        // subdirectories in the build tree;
        fileName = FileTail (fileName);
        // absolute filename to destination
        string destFileName =
            MakePath (DestFile, moduleLocation, moduleShortPath,
                      fileName, exportType);
        // relative filename to destination
        string destRelFileName =
            MakePath (DestFileRel, moduleLocation, moduleShortPath,
                      fileName, exportType);
        // relative path to destination directory
        string destRelDirName = FileHead (destRelFileName);

        //
        // finally we can copy source to destination
        //
        success = CopyFileToBuildTree (sourceFileName, destFileName);
        if ( ! success) {
            cout << "WARNING: could not copy file " << endl;
            cout << "         " << fileName << endl;
            cout << "         of module '" << module.GetName()
                 << "' to build tree" << endl;
            cout << endl;
            continue; // next file
        }

        //
        // Create a second copy of each private (ie. restricted) header
        // file in the owner module, so it can be accessed from the
        // other source and header files in the same module without any
        // prefix in the #include statement. This is just a handy
        // shortcut for increasing the readability of the code, but the
        // same header fiels are also available as #include "restricted/"
        // files. This shortcut relies on the compiler to *always*
        // first look for an include file in the same directory where
        // the file with the #include lives.
        //
        if (exportType == Restricted) {
            // following dest file with export type "source" will put the
            // header file in the same directory where the source files
            // will go.
            string secondDestFileName =
                MakePath (DestFile, moduleLocation, moduleShortPath,
                          fileName, /*export type*/Source);

            success = CopyFileToBuildTree (sourceFileName, secondDestFileName);
            if ( ! success) {
                cout << "WARNING: could not copy file " << endl;
                cout << "         " << fileName << endl;
                cout << "         of module '" << module.GetName()
                     << "' to build tree" << endl;
                cout << endl;
                continue; // next file
            }
        }

        //
        // collect information of this file for Makefile
        //
        if (exportType == Source) {
            const string & source = destRelFileName;
            const string object = FileRoot (FileTail (source)) + ".o";

            makefile.srcs.push_back (source);
            makefile.objs.push_back (object);
            AppendUnique (makefile.vpath, destRelDirName);
        } else if (exportType == Doxygen) {
            // Once the doxygen file has been copied, we don't want to add it 
            // to the list of source, object, or include files in the makefile
            continue; // next file
        } else if (exportType == Raw) {
            continue; // next file
        } else {
            makefile.incs.push_back (destRelFileName);
            AppendUnique (makefile.incDirs, string("-I") + destRelDirName);
        }
    }

    //
    // merge set of inherited paramters and parameters of this module that
    // have declared subtree visibility; we will need the merged set for
    // recursively working on the submodules;
    //
    ModParamInstanceList subModuleParams = inheritedParams;

    FOREACH_CONST (ModuleInstance::ModParamInstanceList, it,
        moduleInstance.GetParam())
    {
        const ModParamInstance * paramInstance = *it;
        const ModParam & param = paramInstance->GetModParam();

        if (param.GetVisibility() == ModParam::Subtree) {
            subModuleParams.push_back (paramInstance);
        }
    }

    StringList libFiles = module.GetLibrary();

    FOREACH_CONST (StringList, it, libFiles) 
    {
        string libFileName = *it;

        string fullName = sourceTree.FullName(libFileName);
        if (!fullName.empty())
        {
            string dirName = FileDirName(fullName);
            AppendUnique (makefile.localIncs, "-I" + dirName + "/include");
            AppendUnique (makefile.localLibs, fullName);
        }
        else
        {
            // strip off the directory and extension 
            string fileName = FileRoot(FileTail(libFileName));
            string libName;
            const string lib("lib");

            string::size_type separator = fileName.find (lib);
            if (separator == string::npos) 
            {
                libName = fileName;
            } 
            else 
            {
                if (fileName.length() == separator + 1) 
                {
                    // separator is last char - tail is empty
                    libName = fileName;
                } 
                else 
                {
                    libName = fileName.substr (separator + lib.size());
                }
            }

            AppendUnique (makefile.commonLibs, "-l" + libName);
            // common include and lib directories will always be generated in
            // the makefile.config, so no need to add it here
	    cout << "INFO: '" << libFileName << "' not found in local directories.\n"
		 << "         Library '" << fileName << "' assumed to be under the common directory specified in Makefile.config. " 
		 << endl;
        }
    }

    // include files
    StringList incFiles = module.GetInclude();

    FOREACH_CONST (StringList, it, incFiles) 
    {
        string incFileName = *it;

        string fullName = sourceTree.FullName(incFileName);
        if (!fullName.empty())
        {
            AppendUnique (makefile.localIncs, "-I" + fullName);
        }
    }

    // system include files
    StringList sysIncFiles = module.GetSysInclude();
    FOREACH_CONST (StringList, it, sysIncFiles) 
    {
        string incFileName = *it;
        AppendUnique (makefile.localIncs, "-I" + incFileName);
    }
    
    // add include options
    StringList incOptions = module.GetIncludeOptions();
    FOREACH_CONST (StringList, it, incOptions) 
    {
	SplitString split(*it, " \t");
	SplitString::iterator sit = split.begin(); 
	
	string thisOption(*sit); sit++;
	string fullName = sourceTree.FullName(*sit);
	thisOption += " " + fullName;
	AppendUnique (makefile.localIncs, thisOption);
    }

    // system library  files
    StringList sysLibFiles = module.GetSysLibrary();
    FOREACH_CONST (StringList, it, sysLibFiles) 
    {
        string libFileName = *it;

        string fullName = sourceTree.FullName(libFileName);
        // if the libFileName is a directory, add "-L" to the front
        if (!fullName.empty() && sourceTree.IsDirectory(fullName))
        {
            AppendUnique (makefile.localLibs, "-L" + fullName);
        }
        // otherwise if it starts with a "-", add it unchanged
        else if (libFileName[0] == '-')
        {
            AppendUnique (makefile.localLibs, libFileName);
        }
    }



    //
    // Recurse into contained modules...
    //
    FOREACH_CONST (ModuleInstance::ModuleInstanceList, it,
        moduleInstance.GetSubModules())
    {
        const ModuleInstance & subModuleInstance = **it;
        const Module & subModule = subModuleInstance.GetModule();

        string subModulePath = FileJoin (modulePath, subModule.GetProvides());
        string subModuleShortPath =
            CreateShortName (modulePath, moduleShortPath, subModule);

        success = CreateBuildTreeForModule (subModuleInstance,
                      subModulePath, subModuleShortPath, subModuleParams);
        if ( ! success) {
            return false;
        }
    }

    return true;
}


/**
 * Search for modules that have their own Makefile and create those
 *
 * @return true for success, false otherwise
 */
bool
ModelBuilder::CreateSubMakefiles (
    const ModuleInstance & moduleInstance,
    const string & modulePath,
    const string & moduleShortPath)
{
    const Module & module = moduleInstance.GetModule();
    const string & moduleLocation = module.GetLocation();
    bool success;

    // if the module has its own Makefile, handle it here;        
    // exception: the top level Makefile (modPath == system) has some        
    // special requirements and is not handled here
    //if (module.GetMakefile() != "" && modulePath != "system") {
    if (module.GetMakefile().size() != 0 && modulePath != "system") {
        //
        // copy the source tree of the module into the "src" directory
        //
        // use the directory that contains the module config file as
        // source directory; if we were directly using its parent
        // directory (ie. '.'), the resolver might find it first in some
        // other package and thus we'd end up with the wrong place in this
        // case;
        string sourceFileName = FileDirName (module.GetFileName());
        if (sourceFileName.empty()) {
            cout << "WARNING: source file " << sourceFileName << endl;
            cout << "           does not exist in module '"
                 << module.GetName() << "'" << endl;
            cout << "           ... not copying" << endl;
            cout << endl;
        }

        string destFileName =
            MakePath (DestFile, moduleLocation, moduleShortPath,
                      "src", Source);

        success = CopyFileToBuildTree (sourceFileName, destFileName);
        if ( ! success) {
            cout << "WARNING: could not copy module " << module.GetName()
                 << " src tree to build tree" << endl;
            cout << endl;
        }

        //
        // determine the source and destination makefile paths
        //
        sourceFileName =
            MakePath (SourceFile, moduleLocation, moduleShortPath,
                      //module.GetMakefile(), Source);
		      module.GetMakefile().front(), Source);
        string destRelFileName =
            MakePath (DestFileRel, moduleLocation, moduleShortPath,
                      "Makefile", Source);
        string destRelDirName = FileHead (destRelFileName);
        destFileName =
            MakePath (DestFile, moduleLocation, moduleShortPath,
                      "Makefile", Source);
        string destDirName = FileHead (destFileName);

        //
        // add modules directory to subdirs of upper level Make
        // and export all module targets to the caller;
        //
        makefile.subDirs.push_back (destRelDirName);
        string modTargets;
        FOREACH_CONST (Module::StringList, it, module.GetTarget()) {
            makefile.subTargets.push_back (FileJoin (destRelDirName, *it));
            if (modTargets.empty()) {
                modTargets = *it;
            } else {
                modTargets += string(" ") + *it;
            }
        }

        //
        // find the name of the public include directory
        //
        destRelFileName =
            MakePath (DestFileRel, "", "", "foo", Public);
        destRelDirName = FileHead (destRelFileName);
        string buildRelDir = FileRelativePath (destDirName, buildDir);
        string relIncDir = string("-I") + 
            FileJoin (buildRelDir, destRelDirName);

	const ModuleInstance * rootModuleInstance = model.GetRootModule();
	const Module & rootModule = rootModuleInstance->GetModule();
	string rootMakefileConfig = "Makefile.config";
	Module::StringList rootMakefileConfigList = rootModule.GetMakefile();

	if (rootMakefileConfigList.size() > 1) {
	    rootMakefileConfig = rootMakefileConfigList[1];
	}
	
        //
        // fix replacement strings for sub-makefiles
        //
        makefile.AddReplacement("BUILDROOT", buildRelDir);
        makefile.AddReplacement("INCDIRS", relIncDir);
	makefile.AddReplacement("MAKEFILECONFIG",
				FileJoin (buildRelDir, rootMakefileConfig));
        makefile.AddReplacement("TARGET", modTargets);

        //
        // copy makefile template to built tree and substitute placeholders
        //
        SubstitutePlaceholders (sourceFileName, destFileName);
    }

    //
    // Recurse into contained modules...
    //
    FOREACH_CONST (ModuleInstance::ModuleInstanceList, it,
        moduleInstance.GetSubModules())
    {
        const ModuleInstance & subModuleInstance = **it;
        const Module & subModule = subModuleInstance.GetModule();

        string subModulePath = FileJoin (modulePath, subModule.GetProvides());
        string subModuleShortPath =
            CreateShortName (modulePath, moduleShortPath, subModule);

        success = CreateSubMakefiles (subModuleInstance,
                      subModulePath, subModuleShortPath);
        if ( ! success) {
            return false;
        }
    }

    return true;
}


/**
 * modPath contains the full module dependence path of the
 * provides-requires dependence chain; since this comes from a flat
 * namespace, it frequently prefixes the name of the previous level to the
 * current provides name, e.g. arana_ibox/arana_ibox_foo; here we try to
 * find the longest such prefix and remove this prefix to create a cleaner
 * directory hierarchy;
 * @note that this is only a heuristic - it might not always work well,
 * depending on the naming strategy that was used in the source code.
 */
string
ModelBuilder::CreateShortName (
    const string & modPath,
    const string & modShortPath,
    const Module & module)
{
    const string & provides = module.GetProvides();
    string shortProvides = provides;
    SplitString splitModPath(modPath, "/");
    FOREACH (SplitString, it, splitModPath) {
        if ( ! (*it).empty()) {
            if (provides.find (*it) == 0) {
                shortProvides = StringTrim (
                    provides.substr ((*it).length()), TrimLeft, "_-.");
            }
        }
    }

    return FileJoin (modShortPath, shortProvides);
}

/**
 * Create a header file used to register the simulator configuration
 * as simulator stats, so it can be automatically printed in the stats
 * output. We recursively descend down the module tree and register each
 * module's configuration as we pass by.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateSimConfig (void)
{
    // Open header file for writing and output the header info.
    const string configFileName = 
        MakePath (DestFile, "", "", "sim_config.h", Synthesized);
    MakeDir (FileHead (configFileName));
    ofstream simConfigFile (configFileName.c_str(),
        ios_base::out | ios_base::trunc);

    if (!simConfigFile) {
        cerr << "Error: Can't open " << configFileName << " for write" << endl;
        return false;
    }

    simConfigFile << "//" << endl
                  << "// automatically generated file - DO NOT EDIT" << endl
                  << "//" << endl;

    // register all modules, starting at root module
    const ModuleInstance * rootModule = model.GetRootModule();
    if ( ! rootModule) {
        cerr << "CreateSimConfig: no root module defined for model "
             << model.GetName() << endl
             << "  at location " << model.GetFileName() << endl;
        return false;
    } else {
        if (! CreateSimConfigForModule (*rootModule, simConfigFile)) {
            return false;
        }
    }

    // cleanup
    simConfigFile.close();

    return true;
}

/**
 * Create registration section for simulator configuration of this module
 * and all its submodules.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateSimConfigForModule (
    const ModuleInstance & moduleInstance, ///< current module
    ostream & simConfigFile)               ///< output stream
{
    const Module & module = moduleInstance.GetModule();

    // register the current module
    string moduleProvides = module.GetProvides();
    string moduleName = module.GetName();
    simConfigFile << "Register(\"Module_" << moduleProvides
        << "\",\"provides " << moduleProvides
        << " (" << moduleName << ")\",UINT64,module_" << moduleProvides << ", true)"
        << endl;

    // register all parameters of the current module
    FOREACH_CONST (ModuleInstance::ModParamInstanceList, it,
        moduleInstance.GetParam())
    {
        const ModParamInstance & paramInstance = **it;
        const ModParam & param = paramInstance.GetModParam();
        const string & paramName = param.GetName();
        const string & paramValue = paramInstance.GetValue();
        if (param.IsDynamic()) {
            if (param.IsString()) {
                simConfigFile << "Declare(extern string " << paramName << ")"
                    << endl;
                simConfigFile << "RegisterDyn(\"Param_" << paramName
                    << "\",\"(dynamic) parameter " << paramName << "\",string,"
                    << paramName << ")" << endl;
            } else {
                simConfigFile << "Declare(extern UINT64 " << paramName << ")"
                    << endl;
                simConfigFile << "RegisterDyn(\"Param_" << paramName
                    << "\",\"(dynamic) parameter " << paramName << "\",UINT64,"
                    << paramName << ")" << endl;
            }
        } else {
            if (param.IsString()) {
                simConfigFile << "Register(\"Param_" << paramName
                    << "\",\"parameter " << paramName << "\",string,param_"
                    << paramName << ",\"" << paramValue << "\")" << endl;
            } else {
                simConfigFile << "Register(\"Param_" << paramName
                    << "\",\"parameter " << paramName << "\",UINT64,param_"
                    << paramName << "," << paramValue << ")" << endl;
            }
        }
    }

    // recursively progress down into submodules
    FOREACH_CONST (ModuleInstance::ModuleInstanceList, it,
        moduleInstance.GetSubModules())
    {
        CreateSimConfigForModule (**it, simConfigFile);
    }

    return true;
}

/**
 * Create a header file for module defining it's parameters and
 * including any public headers files.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateModuleHeader (
    const ModuleInstance & moduleInstance, ///< module to create header for
    const ModParamInstanceList & inheritedParams) ///< params from ancestors
{
    const Module & module = moduleInstance.GetModule();
    const string & provides = module.GetProvides();
    const string & name = module.GetName();
    const string & configFile = module.GetFileName();

    // Open header file for writing and output the header info.
    const string headerFileName = 
        MakePath (DestFile, "", "", provides + ".h", Synthesized);
    MakeDir (FileHead (headerFileName));
    ofstream headerFile (headerFileName.c_str(),
        ios_base::out | ios_base::trunc);

    if ( ! headerFile) {
        cerr << "Error: Can't open " << headerFileName << " for write" << endl;
        return false;
    }

    headerFile << "/**************************************************" << endl;
    headerFile << " * This header file is automatically generated" << endl;
    headerFile << " * by ModelBuilder for module \"" << name << "\"" << endl;
    headerFile << " * from file \"" << configFile << "\"" << endl;
    headerFile << " *************************************************/" << endl;
    headerFile << endl;
    headerFile << "#include <string>" << endl;
    headerFile << "#include \"asim/syntax.h\"" << endl;
    headerFile << "#include \"asim/atoi.h\"" << endl;
    headerFile << endl;
    headerFile << "using std::string;" << endl;
    headerFile << endl;
    headerFile << "#ifndef _ASIM_" << StringToUpper(provides) << "_" << endl;
    headerFile << "#define _ASIM_" << StringToUpper(provides) << "_" << endl;
    headerFile << endl;

    // print all inherited parameters first
    FOREACH_CONST (ModParamInstanceList, it, inheritedParams) {
        const ModParamInstance & paramInstance = **it;
        const ModParam & param = paramInstance.GetModParam();
        const string & paramName = param.GetName();
        const string & paramValue = paramInstance.GetValue();

        if (param.IsDynamic()) {
            if (param.IsString()) {
                headerFile << "extern string " << paramName << ";" << endl;
            } else {
                headerFile << "extern UINT64 " << paramName << ";" << endl;
            }
        } else {
            headerFile << "#ifndef " << paramName << endl;
            if (param.IsString()) {
                headerFile << "#define " << paramName << " \""
                           << paramValue << "\"" << endl;
            } else {
                headerFile << "#define " << paramName << " "
                           << paramValue << endl;
            }
            // headerFile << "#define " << paramName << "__STR \""
            //            << paramValue << "\"" << endl;
            headerFile << "#endif" << endl;
        }
    }
    // print the module's own parameters
    FOREACH_CONST (ModuleInstance::ModParamInstanceList, it,
        moduleInstance.GetParam())
    {
        const ModParamInstance & paramInstance = **it;
        const ModParam & param = paramInstance.GetModParam();
        const string & paramName = param.GetName();
        const string & paramValue = paramInstance.GetValue();

        if (param.IsDynamic()) {
            if (param.IsString()) {
                headerFile << "extern string " << paramName << ";" << endl;
            } else {
                headerFile << "extern UINT64 " << paramName << ";" << endl;
            }
            // add to running list of dynamic params that were processed
            dynamicParams.push_back(& paramInstance);
        } else {
            if (param.IsString()) {
                headerFile << "#define " << paramName << " \""
                           << paramValue << "\"" << endl;
                headerFile << "#define " << paramName << "__STR \""
                           << paramValue << "\"" << endl;
            } else {
                headerFile << "#define " << paramName << " " << paramValue << endl;
                headerFile << "#define " << paramName << "__STR \""
                           << paramValue << "\"" << endl;
            }
        }
    }

    // create #include statements for public files in synthesized header
    const string headerDir = FileHead (headerFileName);
    FOREACH_CONST (Module::StringList, it, module.GetPublic()) {
        string includeFileName = 
            MakePath (DestFile, "", "", FileTail(*it), Restricted);
        includeFileName = FileRelativePath (headerDir, includeFileName);
        headerFile << "#include \"" << includeFileName << "\"" << endl;
    }

    headerFile << endl;
    headerFile << "#endif // _ASIM_" << StringToUpper(provides) << "_" << endl;
    headerFile.close();

    return true;
}

/**
 * Create the support file necessary for handling dynamic parameters.
 *
 * @return true on success, false otherwise
 */
bool
ModelBuilder::CreateDynamicParams (void)
{
    // Open param file for writing and output the header info.
    const string paramFileName = 
        MakePath (DestFile, "base", "base", "param.cpp", Source);
    MakeDir (FileHead (paramFileName));
    // we need to unlink the placeholder file, since it is a link to the
    // original and we don't want to overwrite the original
    unlink (paramFileName.c_str());
    ofstream paramFile (paramFileName.c_str(),
        ios_base::out | ios_base::trunc);

    if ( ! paramFile) {
        cerr << "Error: Can't open dynamic parameter file "
             << paramFileName << " for write" << endl;
        return false;
    }

    paramFile << "/**************************************************" << endl;
    paramFile << " * This source file is automatically generated" << endl;
    paramFile << " * by ModelBuilder for defining all parameters." << endl;
    paramFile << " *************************************************/" << endl;
    paramFile << endl;
    paramFile << "#include \"asim/param.h\"" << endl;
    paramFile << "#include \"asim/atoi.h\"" << endl;
    paramFile << endl;

    // declare each parameter and define with default value
    FOREACH_CONST (ModParamInstanceList, it, dynamicParams) {
        const ModParamInstance & paramInstance = **it;
        const ModParam & param = paramInstance.GetModParam();

        if (param.IsString()) {
            paramFile << "string " << param.GetName() << " = \""
                      << paramInstance.GetValue() << "\";" << endl;
        } else {
            paramFile << "UINT64 " << param.GetName() << " = "
                      << paramInstance.GetValue() << ";" << endl;
        }
    }

    // create parser for command line flags for each parameter
    paramFile << endl << endl;
    paramFile << "bool SetParam (char * name, char * value)" << endl;
    paramFile << "{" << endl;
    paramFile << "    bool found = true;" << endl;
    string ifString = "if";
    FOREACH_CONST (ModParamInstanceList, it, dynamicParams) {
        const ModParamInstance & paramInstance = **it;
        const ModParam & param = paramInstance.GetModParam();
        const string & paramName = param.GetName();

        paramFile << "    " << ifString << " (strcmp(name, \""
                  << paramName << "\") == 0) {" << endl;
        if (param.IsString()) {
            paramFile << "        " << paramName << " = value;"
                      << endl;
        } else {
            paramFile << "        " << paramName 
              << " = atoi_general(value);"              //<< " = strtoll(value, NULL, 0);"
                      << endl;
        }
        paramFile << "    }" << endl;
        ifString = "else if";
    }
    // add some error checking
    paramFile << "    " << ifString << " (1) {" << endl;
    paramFile << "        found = false;" << endl;
    paramFile << "    }" << endl;
    paramFile << "    return found;" << endl;
    paramFile << "}" << endl;

    // create lister for dynamic params
    paramFile << endl << endl;
    paramFile << "void ListParams (void)" << endl;
    paramFile << "{" << endl;
    if (dynamicParams.empty()) {
        paramFile << "    cout << "
                  << "\"There are no dynamic parameters registered!\" << endl;"
                  << endl;
    } else {
        paramFile << "    cout << "
                  << "\"The following dynamic parameters are registered:\""
                  << " << endl;" << endl;
        FOREACH_CONST (ModParamInstanceList, it, dynamicParams) {
            const ModParamInstance & paramInstance = **it;
            const ModParam & param = paramInstance.GetModParam();
            const string & paramName = param.GetName();
            const string & paramValue = paramInstance.GetValue();
           
            paramFile << "    cout << \"    " << paramName
                      << "\" << \" = \" << \"";
            if (param.IsString()) {
                // string param values are printed double-quoted
                paramFile << "\\\"";
            }
            paramFile << paramValue;
            if (param.IsString()) {
                // string param values are printed double-quoted
                paramFile << "\\\"";
            }
            paramFile << "\" << endl;" << endl;
        }
    }
    paramFile << "}" << endl;

    paramFile.close();

    return true;
}


/**
 * Dump state of internal data structures to ostream and return ostream
 * reference for operation chaining.
 *
 * @return ostream reference for operation chaining
 */
ostream & 
ModelBuilder::Dump (
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "ModelBuilder::" << endl;
    out << prefix << "  Model = " << model.GetName() << endl;
    out << prefix << "  BuildDir = " << buildDir << endl;

    out << prefix << "  makefile.replace:" << endl;
    FOREACH_CONST (StringMap, it, makefile.replace) {
        out << prefix << "    " << it->first << " = " << it->second << endl;
    }

    return out;
}


//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

#include <list>

void TestBuild (int argc, char ** argv)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
        exit (1);
    } else {
        Model model(*workspace);

        if ( ! model.Parse (argv[1])) {
            cerr << "Model parsing error!" << endl;
        }

        ModelBuilder builder(*workspace, model, "/home/klauser/work/awb/pm");

        bool success = 
            builder.CreateBuildTree();
        if ( ! success) {
            cerr << "Error creating build tree" << endl;
        }
    }

    delete workspace;
}

int main (int argc, char ** argv)
{
#if 0
    if (argc >= 2) {
        TestBuild (argc, argv);
    }
#endif
}

#endif // TESTS 
