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
 * @brief ASIM module Information
 */

// generic (C)

// generic (C++)
#include <iostream>
#include <stdlib.h>
#include <fstream>

// local
#include "module.h"
#include "util.h"

//----------------------------------------------------------------------------
// class Module
//----------------------------------------------------------------------------

/**
 * Create a new module object.
 */
Module::Module (
    const Workspace & theWorkspace) ///< workspace this module belongs to
  : workspace(theWorkspace)
{
    // nada
}

/**
 * Destroy this module object.
 */
Module::~Module()
{
    // nada
}

/**
 * Parse the file moduleFile into the internal data structures of this
 * module.
 *
 * @return true on success, false otherwise
 */
bool
Module::Parse (
    const string & moduleFileName)  ///< path to the module file to parse
{
    // do some error checking first
    if (moduleFileName.empty()) {
        cerr << "Module::Parse: Empty module file name" << endl;
        return false;
    }

    UnionDir & sourceTree = workspace.GetSourceTree();
    if (sourceTree.IsDirectory (moduleFileName)) {
        cerr << "Module::Parse: module points to directory instead of file - "
             << moduleFileName << endl;
        return false;
    }

    string prefix = sourceTree.GetPrefix (moduleFileName);
    if (prefix.empty()) {
        cerr << "Module::Parse: Module file not found under source tree - "
             << moduleFileName << endl;

//        Just leaving this a warning - since GetPrefix is broken
//        return false;
    }

    SetBaseDir (prefix);
    string dirName = FileDirName (moduleFileName);
    SetLocation (dirName);

    string fullName = sourceTree.FullName (moduleFileName);
    SetFileName (fullName);

    if ( ! sourceTree.IsFile (moduleFileName)) {
        // if we can't find the config file, figure out what is the first
        // directory in its path that can't be found and supply a pointed
        // error message for that
        string missingDir;
        if ( ! sourceTree.Exists (dirName)) {
            while ( ! sourceTree.Exists (dirName)) {
                missingDir = dirName;
                dirName = FileDirName (dirName);
                if (dirName == missingDir) {
                    cerr << "Module::Parse: internal error" << endl;
                    exit(1);
                }
            }
            
            missingDir = string("  non-existent directory ")
                + missingDir + '\n';
        }
        cerr << "Module::Parse:" << endl << missingDir
             << "  Module file " << moduleFileName << " not found" << endl;
        return false;
    }

    // FIXME: module cache stuff
    /*
    if { [info exists module_cache($fullname)] && [lindex $module_cache($fullname) 0] == [file mtime $fullname] } {
        # module has not changed since last parsed - return cached copy
        return [lindex $module_cache($fullname) 1]
    }
    */

    //
    // now we are ready to parse the module file
    //
    bool inAwbRegion = false;
    string line;

    // open module file
    ifstream moduleFile(fullName.c_str());
    if (!moduleFile) {
        cerr << "Error: Can't open " << fullName << " for read" << endl;
        return false;
    }
 
    // parse module file
    while (! moduleFile.eof()) {
        getline (moduleFile, line);
        if (line.empty() && moduleFile.eof()) {
            break; // also eof
        }
        MatchString matchLine(line);
        MatchString::MatchArray matchArray;

        //
        // If we find "%AWB_START" on a line, then we are entering
        // an awb definition region. If we find "%AWB_END" then
        // we are done with the file.
        //
        if ( ! matchLine.Match ("%AWB_START").empty()) {
            if (inAwbRegion) {
                cerr << "Nested %AWB_START in line: " << line << endl;
                return false;
            }
            inAwbRegion = true;
            continue;
        } else if ( ! matchLine.Match ("%AWB_END").empty()) {
            if ( ! inAwbRegion) {
                cerr << "Unmatched %AWB_END in line: " << line << endl;
                return false;
            }
            inAwbRegion = false;
            break;
        } else if ( ! inAwbRegion) {
            // ignore everything outside AWB region
            continue;
        }

        // invariant: we are inside AWB region (inAwbRegion = true)

        //
        // Look for special directives and collect the information
        //

        // %name
        if ( ! matchLine.Match (
              "%name[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            if ( ! GetName().empty()) {
                cerr << "Multiple %name in file " << moduleFileName
                     << endl;
                return false;
            }
            SetName (matchArray[1]);
            // the name is implicitly also an attribute of the module
            AddAttribute (matchArray[1]);
        // %desc
        } else if ( ! matchLine.Match (
              "%desc[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            if ( ! GetDesc().empty()) {
                cerr << "Multiple %desc in file " << moduleFileName
                     << endl;
                return false;
            }
            SetDesc (matchArray[1]);
        // %provides
        } else if ( ! matchLine.Match (
              "%provides[[:space:]]+([^[:space:]]+)",
              matchArray).empty())
        {
            if ( ! GetProvides().empty()) {
                cerr << "Multiple %provides in file " << moduleFileName
                     << endl;
                return false;
            }
            SetProvides (matchArray[1]);
        // %requires
        } else if ( ! matchLine.Match (
              "%requires[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddRequires (*it);
                }
            }
        // %attributes
        } else if ( ! matchLine.Match (
              "%attributes[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            AddAttribute (matchArray[1]);
        // %public
        } else if ( ! matchLine.Match (
              "%public[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddPublic (*it);
                }
            }
        // %private
        } else if ( ! matchLine.Match (
              "%private[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddPrivate (*it);
                }
            }
        // %library
        } else if ( ! matchLine.Match (
              "%library[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddLibrary (*it);
                }
            }
        // %include
        } else if ( ! matchLine.Match (
              "%include[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddInclude (*it);
                }
            }
        // %ifile_opt -- support options with file arguments. This
	// implementation is specific - a generic method to feed in special
	// compiler flags and options with or without file args would be
	// useful.
        } else if ( ! matchLine.Match (
              "%ifile_opt[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
	    AddIncludeOption (matchArray[1]);
        // %syslibrary
        } else if ( ! matchLine.Match (
              "%syslibrary[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddSysLibrary (*it);
                }
            }
        // %sysinclude
        } else if ( ! matchLine.Match (
              "%sysinclude[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddSysInclude (*it);
                }
            }

        // %param , %export , %const
        } else if ( ! matchLine.Match (
              "(%(param|export|const)[[:space:]]+.*)",
              matchArray).empty())
        {
            ModParam * param = new ModParam;
            if (param->Parse (matchArray[0])) {
                AddParam (param);
            } else {
                // param parsing error
                cerr << "Module::Parse: could not parse param in file " 
                     << moduleFileName << endl;
                return false;
            }
        // %makefile
        } else if ( ! matchLine.Match (
              "%makefile[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddMakefile (*it);
                }
            }
            //if ( ! GetMakefile().empty()) {
            //    cerr << "Multiple %makefile in file " << moduleFileName
            //         << endl;
            //    return false;
            //}
            //SetMakefile (matchArray[1]);
        // %consript
        } else if ( ! matchLine.Match (
              "%conscript[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            SplitString split(matchArray[1], " \t");
            FOREACH (SplitString, it, split) {
                if ( ! (*it).empty()) {
                    AddConscript (*it);
                }
            }
        // %target
        } else if ( ! matchLine.Match (
              "%target[[:space:]]+([^[:space:]].+[^[:space:]])",
              matchArray).empty())
        {
            AddTarget (matchArray[1]);
        // anything else
        } else {
            // can't give error message since anything else MUST be
            // treated as comment to be backward compatible - yikes
            continue;

            // cerr << "Module::Parse: unknown directive in file " 
            //      << moduleFileName << endl
            //      << line << endl;
            // return false;
        }
    }
    moduleFile.close();

    if (GetProvides().empty()) {
        cerr << "Module::Parse: module does not 'provide' anything - "
             << moduleFileName << endl;
        return false;
    }
    // FIXME: module cache stuff
    /*
    # store parsed module in cache w/ file modification timestamp
    set module_cache($fullname) [list [file mtime $fullname] $mod]
    */

    return true;
}


/**
 * Find a module parameter by parameter name.
 *
 * @return pointer to the module parameter object or NULL if not found
 */
const ModParam *
Module::FindParam (
    const string & paramName) ///< module parameter name to search for
const
{
    FOREACH_CONST (ModParamList, it, paramList) {
        if ((*it)->GetName() == paramName) {
            return (*it); // found
        }
    }

    return NULL; // not found
}

/**
 * Add an attribute string to this module.
 */
void
Module::AddAttribute (
    const string & attribute) ///< attribute to add
{
    attributeList.push_back (attribute);
}

/**
 * Check if this module has the given attribute.
 *
 * @return true if module has given attribute
 */
bool
Module::HasAttribute (
    const string & attribute) ///< attribute to search for
{
    FOREACH_CONST (StringList, it, attributeList) {
        if (attribute == *it) {
            return true; // found
        }
    }

    return false; // not found
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
Module::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "Module::" << endl;
    out << prefix << "  Name: " << GetName() << endl;
    out << prefix << "  Desc: " << GetDesc() << endl;
    out << prefix << "  BaseDir: " << GetBaseDir() << endl;
    out << prefix << "  Location: " << GetLocation() << endl;
    out << prefix << "  FileName: " << GetFileName() << endl;
    out << prefix << "  Provides: " << GetProvides() << endl;
    out << prefix << "  Requires:" << endl;
    FOREACH_CONST (StringList, it, requiresList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Public:" << endl;
    FOREACH_CONST (StringList, it, publicFileList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Private:" << endl;
    FOREACH_CONST (StringList, it, privateFileList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Library:" << endl;
    FOREACH_CONST (StringList, it, libraryFileList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Include:" << endl;
    FOREACH_CONST (StringList, it, includeFileList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  sysLibrary:" << endl;
    FOREACH_CONST (StringList, it, sysLibraryFileList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  sysInclude:" << endl;
    FOREACH_CONST (StringList, it, sysIncludeFileList) {
        out << prefix << "    " << *it << endl;
    }

    out << prefix << "  Attribute:" << endl;
    FOREACH_CONST (StringList, it, attributeList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Makefile:" << endl;
    FOREACH_CONST (StringList, it, makefileList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Target:" << endl;
    FOREACH_CONST (StringList, it, targetList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  Param:" << endl;
    FOREACH_CONST (ModParamList, it, paramList) {
        (*it)->Dump(out, prefix + "    ");
    }

    return out;
}

//----------------------------------------------------------------------------
// class ModuleDB
//----------------------------------------------------------------------------

/**
 * Construct a new module database object.
 */
ModuleDB::ModuleDB (
    const Workspace & theWorkspace) ///< workspace this module belongs to
  : workspace(theWorkspace)
{
    dynamicParams = true;
}

/**
 * Destroy this module database object.
 */
ModuleDB::~ModuleDB()
{
}

/**
 * Collect all modules found in the associated workspace.
 *
 * Cleanup the list of known modules and reconstruct all the information
 * from scratch.
 */
void
ModuleDB::CollectAllModules (void)
{
    Clear();

    const char * const dirName = ""; // everything starting from ASIM root

    int numModules = 0;
    numModules = CountModules (dirName);
    CollectModules (dirName);
}

/**
 * Collect all modules found in the file system (sourceTree) subtree rooted
 * at the given directory name. If countOnly is true, we skip the actual
 * module creation step and just count how many modules we find.
 *
 * @return number of modules found
 */
int
ModuleDB::CollectModules (
    const string & dirName,      ///< directory name to start searching at
    bool countOnly)              ///< if true, skip module creation
{
    UnionDir & sourceTree = workspace.GetSourceTree();
    UnionDir::StringList fileList;
    int count = 0;

    string pattern = FileJoin (dirName, "*");
    sourceTree.Glob (pattern, fileList);
    FOREACH_CONST (UnionDir::StringList, it, fileList) {
        const string & fileName = *it;

        // don't examine CVS directories or *~ files...
        if (sourceTree.IsDirectory(fileName)) {
            if ((fileName != "CVS") && (fileName != ".svn")){
                // collect subdirectory recursively
                count += CollectModules (fileName, countOnly);
            }
        } else if (    fileName.size() >= 4
                    && fileName.substr(fileName.size()-4) == ".awb")
        {
            // if we are only counting, skip the actual module generation
            if ( ! countOnly ) {
                Module * module = new Module(workspace);
                if (module->Parse (fileName)) {
                    ModuleMapValue entry (module->GetProvides(), module);
                    modules.insert (entry);
                } else {
                    cerr << "Error collecting module " << fileName << endl;
                    delete module;
                }
            }
            count++;
        }
    }

    return count;
}

/**
 * Find all modules that provide a given type. Returns an iterator pair.
 *
 * @return iterator pair (begin/end) to modules providing given type
 */
const ModuleDB::Modules
ModuleDB::FindModules (
    const string & providesType) ///< provides type to search for
const
{
    return modules.equal_range (providesType);
}

/**
 * Find the module that provides the specified type and also has the
 * specified attribute.
 *
 * Current algorithm:
 *   - If there is only one module that matches the provides type,
 *     return that module
 *   - else, for each attribute, if exactly one module matches the
 *     provides type and has that attribute then return that module. 
 *   - else, return NULL
 *
 * @return pointer to the matching module or NULL
 */
const Module *
ModuleDB::FindDefaultModule (
    const string & providesType,      ///< the provides type
    const StringList & attributeList) ///< attribute list to check
const
{
    Modules providers = FindModules (providesType);

    // Check if there is only one module
    ModuleMapIterator it = providers.first;
    it++;
    if (it == providers.second) {
        return (*(providers.first)).second;
    }

    // more than 1 provider
    FOREACH_CONST (StringList, attrIt, attributeList) {
        Module* found = NULL;
        FOREACH_ITER (ModuleMapIterator, modIt,
                      providers.first, providers.second)
        {
            Module * module = (*modIt).second;
            if (module->HasAttribute (*attrIt)) {
                if (found) {
                    // We've found more than one so return null
                    return NULL;
                } else {
                    found = module;
                }
            }
        }
        if (found) {
            // found exactly one provider matching this attribute
            return found;
        }
    }

    return NULL;
}

/**
 * Clear the database - all existing modules are discarded.
 */
void
ModuleDB::Clear (void)
{
  // FIXME: we need to make sure we avoid memory leaks with this,
  // i.e. figure out who own the module object
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
ModuleDB::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print in front of each line
const
{
    out << prefix << "ModuleDB::" << endl;
    out << prefix << "  dynamicParams: " << dynamicParams << endl;
    out << prefix << "  Modules: " << endl;
    FOREACH_CONST (ModuleMap, it, modules) {
//        out << prefix << "    provides " << (*it).first << endl;
        (*it).second->Dump(out, prefix + "    ");
    }

    return out;
}

//----------------------------------------------------------------------------
// class ModuleInstance
//----------------------------------------------------------------------------

/**
 * Create a new module instance. The module's parameters are also
 * instantiated with their default values. The module's submodules are not
 * yet instantiated, but left unspecified.
 */
ModuleInstance::ModuleInstance (
    const Module & theModule) ///< abstract module that this is an instance of
  : module(theModule)
{
    // instantiate each module parameter with the default param value
    FOREACH_CONST (Module::ModParamList, it, module.GetParam()) {
        ModParam * modParam = *it;
        ModParamInstance * mpi = new ModParamInstance(*modParam);
        paramInstanceList.push_back (mpi);
    }

    // submodules are not instantiated at module instantiation time, but
    // we fill the submodule list with null pointers for consistency
    FOREACH_CONST (Module::StringList, it, module.GetRequires()) {
        moduleInstanceList.push_back (NULL);
    }
}

/**
 * Destroy a module instance and release all associated storage.
 */
ModuleInstance::~ModuleInstance()
{
    // delete parameter instances
    FOREACH_CONST (ModParamInstanceList, it, paramInstanceList) {
        if (*it) {
            delete *it;
        }
    }
 
    // delete submodule instances
    FOREACH_CONST (ModuleInstanceList, it, moduleInstanceList) {
        if (*it) {
            delete *it;
        }
    }
}

/**
 * Check if a module instance is complete, i.e. if all its submodules are
 * instantiated.
 *
 * @return true if complete, false otherwise
 */
bool
ModuleInstance::IsComplete (
    bool recursive) ///< if true, also check submodules recursively
const
{
    // check if number of abstract and concrete submodules matches; if it
    // does not, we assume the concrete module is not complete
    const Module::StringList & modRequires = module.GetRequires();
    if (modRequires.size() != moduleInstanceList.size()) {
        return false;
    }
    
    // check the actual submodule instances against the requires list;
    // note that we assume that the abstract and concrete lists must be in
    // the same order;
    int idx = 0;
    FOREACH_CONST (ModuleInstanceList, it, moduleInstanceList) {
        if ( ! *it) {
            return false; // uninstantiated submodule
        }
        const string & subModuleProvides = (*it)->GetModule().GetProvides();
        if (modRequires.at(idx) != subModuleProvides) {
            return false; // wrong requires-provides connection
        }
        idx++;

        if (recursive) {
            if ( ! (*it)->IsComplete(recursive)) {
                return false; // submodule incomplete in recursive check
            }
        }
    }

    return true;
}

/**
 * Set submodule at specified index to new module instance.
 */
void
ModuleInstance::SetSubModule (
    const int idx,              ///< submodule index to set
    ModuleInstance * subModule) ///< submodule instance to set it to
{
    moduleInstanceList.at(idx) = subModule;
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
ModuleInstance::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print in front of each line
const
{
    out << prefix << "ModuleInstance::" << endl;

    // dump abstract module
    module.Dump(out, prefix + "  ");

    // dump paramter instances
    out << prefix << "  ParameterInstances:" << endl;
    FOREACH_CONST (ModParamInstanceList, it, paramInstanceList) {
        if (*it) {
            (*it)->Dump(out, prefix + "    ");
        }
    }
 
    // dump submodules
    out << prefix << "  Submodules:" << endl;
    FOREACH_CONST (ModuleInstanceList, it, moduleInstanceList) {
        if (*it) {
            (*it)->Dump(out, prefix + "    ");
        }
    }

    return out;
}

//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

void TestModule (int argc, char ** argv)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
        exit (1);
    } else {
        Module module(*workspace);

        if ( ! module.Parse (argv[1])) {
            cerr << "Module parsing error!" << endl;
        }
        module.Dump(cout);
    }

    delete workspace;
}

void TestModuleDB (int argc, char ** argv)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
        exit (1);
    } else {
        ModuleDB moduleDB(*workspace);

        moduleDB.CollectAllModules();
        moduleDB.Dump(cout);
    }

    delete workspace;
}

void TestModuleInstance (int argc, char ** argv)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
        exit (1);
    } else {
        ModuleDB moduleDB(*workspace);

        moduleDB.CollectAllModules();

        // find all modules of specified type and instantiate the first one
        ModuleDB::Modules providers(moduleDB.FindModules (argv[1]));
        if (providers.first == providers.second) {
            cerr << "no " << argv[1] << " modules found" << endl;
        } else {
            ModuleInstance theInstance(*((*(providers.first)).second));
            theInstance.Dump(cout);
        }
    }

    delete workspace;
}

int main (int argc, char ** argv)
{
#if 0
    if (argc >= 2) {
        TestModule (argc, argv);
    }
#endif

#if 0
    if (argc >= 1) {
        TestModuleDB (argc, argv);
    }
#endif

#if 0
    if (argc >= 2) {
        TestModuleInstance (argc, argv);
    }
#endif
}

#endif // TESTS 
