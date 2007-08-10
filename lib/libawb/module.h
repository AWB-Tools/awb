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
 * @brief ASIM Module Information
 */

#ifndef _MODULE_
#define _MODULE_ 1

// generic (C++)
#include <string>
#include <vector>
#include <map>

// local
#include "workspace.h"
#include "modparam.h"

using namespace std;

/**
 * @brief ASIM Module (abstract)
 *
 * This class encapsulates all information that is known about an abstract
 * ASIM module.
 * @note This is <b>not</b> an instance of a module, rather this is the
 * meta-object that describes the module itself. @see ModuleInstance for
 * the module instance.
 */
class Module {
  public:
    // types
    /// Interface type for container of module parameters
    typedef vector<ModParam*> ModParamList;
    /// Interface type for container of strings
    typedef vector<string> StringList;

  private:
    // members
    const Workspace & workspace;  ///< workspace to use

    string name;                  ///< module name
    string desc;                  ///< module description
    string provides;              ///< module's provides type
    StringList requiresList;      ///< list of module's requires types
    string location;              ///< location of module config file
    StringList publicFileList;    ///< list of module's public files
    StringList privateFileList;   ///< list of module's private files
    StringList libraryFileList;   ///< list of module's library files
    StringList includeFileList;   ///< list of module's include directories
    StringList includeOptionsList; ///< list of module's include directories
    StringList sysIncludeFileList; ///< list of module's system include directories
    StringList sysLibraryFileList; ///< list of module's system libraries
    ModParamList paramList;       ///< list of module's parameters
    StringList attributeList;     ///< list of module's attributes
    string baseDir;               ///< base directory of module config file
    string fullFileName;          ///< full filename of module config file
    StringList makefileList;      ///< list for module's makefile and config file
    StringList targetList;        ///< list of module's targets

  public:
    // constructors/destructors
    /// Create a new module
    Module (const Workspace & theWorkspace);
    /// Delete module
    ~Module();

    /// Parse moduleFileName into module object.
    bool Parse (const string & moduleFileName);

    // accessors / modifiers
    const string & GetName (void) const { return name; }
    void SetName (const string & theName) { name = theName; }
    //
    const string & GetDesc (void) const { return desc; }
    void SetDesc (const string & theDesc) { desc = theDesc; }
    //
    const string & GetProvides (void) const { return provides; }
    void SetProvides (const string & theProvides) { provides = theProvides; }
    //
    const StringList & GetRequires (void) const { return requiresList; }
    void AddRequires (const string & requires)
        { requiresList.push_back (requires); }
    //
    const string & GetLocation (void) const { return location; }
    void SetLocation (const string & theLocation) { location = theLocation; }
    //
    const StringList & GetPublic (void) const { return publicFileList; }
    void AddPublic (const string & thePublic)
        { publicFileList.push_back (thePublic); }
    //
    const StringList & GetPrivate (void) const { return privateFileList; }
    void AddPrivate (const string & thePrivate)
        { privateFileList.push_back (thePrivate); }
    //
    const StringList & GetLibrary (void) const { return libraryFileList; }
    void AddLibrary (const string & theLibrary)
        { libraryFileList.push_back (theLibrary); }
    //
    const StringList & GetInclude (void) const { return includeFileList; }
    void AddInclude (const string & theInclude)
        { includeFileList.push_back (theInclude); }

    const StringList & GetIncludeOptions (void) const { return includeOptionsList; }
    void AddIncludeOption (const string & theIncludeOption)
        { includeOptionsList.push_back (theIncludeOption); }
    //
    const StringList & GetSysInclude (void) const { return sysIncludeFileList; }
    void AddSysInclude (const string & theSysInclude)
        { sysIncludeFileList.push_back (theSysInclude); }
    //
    const StringList & GetSysLibrary (void) const { return sysLibraryFileList; }
    void AddSysLibrary (const string & theSysLibrary)
        { sysLibraryFileList.push_back (theSysLibrary); }
    //
    const ModParam * FindParam (const string & name) const;
    void AddParam (ModParam * const param) { paramList.push_back (param); }
    const ModParamList & GetParam (void) const { return paramList; }
    //
    void AddAttribute (const string & attribute);
    bool HasAttribute (const string & attribute);
    //
    const string & GetBaseDir (void) const { return baseDir; }
    void SetBaseDir (const string & theBaseDir) { baseDir = theBaseDir; }
    //
    const string & GetFileName (void) const { return fullFileName; }
    void SetFileName (const string & theFile) { fullFileName = theFile; }
    //
    const StringList & GetMakefile (void) const { return makefileList; }
    void AddMakefile (const string & theMakefile)
        { makefileList.push_back (theMakefile); }
    //
    const StringList & GetTarget (void) const { return targetList; }
    void AddTarget (const string & target) { targetList.push_back (target); }

    // debug
    /// Dump internal data structures to ostream
    ostream & Dump(ostream & out, const string & prefix = "") const;
};

/**
 * @brief ASIM Module database
 *
 * This class is a (very, very, very simple) database of modules. It
 * keeps track of a set of modules by their provides type.
 */
class ModuleDB {
  public:
    // types
    /// Interface type for container of strings
    typedef vector<string> StringList;
    /// Interface type for container of Modules
    typedef vector<Module&> ModuleList;
    /// Interface type for map of modules (by requires type)
    typedef multimap<string, Module*> ModuleMap;
    /// Interface type for values in a module map
    typedef ModuleMap::value_type ModuleMapValue;
    /// Interface type for const iterator of module map
    typedef ModuleMap::const_iterator ModuleMapIterator;
    /// Interface type for begin/end iterator pair of module map
    typedef pair<ModuleMapIterator, ModuleMapIterator> Modules;

  private:
    // members
    const Workspace & workspace; ///< workspace to use
    ModuleMap modules;  ///< the map holding all modules by provides type
    bool dynamicParams; ///< if false, supress dynamic qualifier on params

    //variable module_cache;

  public:
    // constructors/destructors
    /// Create a new Module database
    ModuleDB (const Workspace & theWorkspace);
    /// Delete a module database
    ~ModuleDB();

    // accessors / modifiers
    // FIXME: (r2r) nobody is using UseDynamicParams right now, and I'm
    // not sure if this class is the right place for it anyway
    /// Turn usage of dynamic parameters on or off
    void UseDynamicParams (bool dyn = true) { dynamicParams = dyn; }
    /// Collect all modules in the associated workspace
    void CollectAllModules (void);
    /// Collect all modules in file system subtree rooted at dirName.
    int CollectModules (const string & dirName, bool countOnly = false);
    /// Count all modules in file system subtree rooted at dirName.
    int CountModules (const string & dirName)
        { return CollectModules (dirName, true); }
    /// Find all modules of given provides type.
    const Modules FindModules (const string & providesType) const;
    /// Find the default for the given requiresType and attribute.
    const Module * FindDefaultModule (const string & requiresType,
        const StringList & attribute) const;
    /// Clear DB (remove all modules)
    void Clear (void);

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

/**
 * @brief ASIM Module (instance)
 *
 * This class represents an ASIM module instance.
 * @see Module for the abstract module, ie. the module meta-object.
 *
 * The module instance contains instances of the module's contained
 * objects, ie. sub-module instances and parameter instances.
 */
class ModuleInstance {
  public:
    // types
    /// Interface type for container of module instances
    typedef vector<ModuleInstance*> ModuleInstanceList;
    /// Interface type for container of module parameter instances
    typedef vector<ModParamInstance*> ModParamInstanceList;

  private:
    const Module & module; ///< the abstract module of this instance
    ModuleInstanceList moduleInstanceList; ///< submodule instances
    ModParamInstanceList paramInstanceList; ///< parameter instances

  public:
    // constructors / destructors
    /// Create a new module instance
    ModuleInstance(const Module & theModule);
    /// Delete module instance
    ~ModuleInstance();

    // accessors / modifiers
    /// Get abstract module reference for this concrete module.
    const Module & GetModule (void) const { return module; }
    /// Check if module has all submodules instantiated.
    bool IsComplete (bool recursive) const;
    /// Get a list of all module parameter instances
    const ModParamInstanceList & GetParam (void) const
        { return paramInstanceList; }
    /// Set a submodule instance to a specific submodule
    void SetSubModule (const int idx, ModuleInstance * subModule);
    /// Get a list of all submodule instances
    const ModuleInstanceList & GetSubModules (void) const
        { return moduleInstanceList; }

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

#endif // _MODULE_ 
