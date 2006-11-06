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
 * @brief ASIM Model information.
 */

#ifndef _MODEL_
#define _MODEL_ 1

// generic (C++)
#include <string>
#include <vector>

// local
#include "module.h"

using namespace std;

/**
 * @brief ASIM Model information.
 *
 * This class contains all information known about an ASIM model.
 *
 * @note
 * This is always a "model instance". The "abstract model" or model
 * meta-information is built-in. I.e. the Model does not have the same
 * abstract vs. instance split that e.g. Module or ModParam has.
 */
class Model {
  public:
    // types
    /// Interface type for container of strings
    typedef vector<string> StringList;

  private:
    // members
    const Workspace & workspace;     ///< workspace to use

    string fileName;                 ///< file name (redundant)
    string configFile;               ///< what is this??
    string name;                     ///< model name
    string desc;                     ///< model description
    StringList defaultAttributeList; ///< list of default attributes
    bool saveAllParams;             ///< should we save all params?
    ModuleInstance * rootModule;     ///< root of model's module tree

  public:
    // constructors / destructors
    /// Create a new Model.
    Model(const Workspace & theWorkspace);
    /// Delete this Model.
    ~Model();

    // accessors / modifiers
    const string & GetFileName (void) const { return fileName; }
    void SetFileName (const string & theFileName) { fileName = theFileName; }
    //
    const string & GetConfigFile (void) const { return configFile; }
    void SetConfigFile (const string & theConfigFile)
        { configFile = theConfigFile; }
    //
    const string & GetName (void) const { return name; }
    void SetName (const string & theName) { name = theName; }
    //
    const string & GetDesc (void) const { return desc; }
    void SetDesc (const string & theDesc) { desc = theDesc; }
    //
    const StringList & GetDefaultAttributes (void) const
        { return defaultAttributeList; }
    void AddDefaultAttribute (const string & theDefaultAttribute)
        { defaultAttributeList.push_back (theDefaultAttribute); }
    //
    /// Should we save all parameters in model config ?
    const bool IsSaveAllParams (void) const { return saveAllParams; }
    /// Save all parameters in model config (vs. only non-default ones)
    void SetSaveAllParams (const bool theSaveAllParams = true)
        { saveAllParams = theSaveAllParams; }
    //
    ModuleInstance * GetRootModule (void) const { return rootModule; }
    void SetRootModule (ModuleInstance * theRootModule)
        { rootModule = theRootModule; }

    // methods
    /// Parse a model file into internal data structures.
    bool Parse (const string & fileName);
    /// Parse a module instance from the model ini file.
    ModuleInstance * ParseModuleInstance (const IniFile & modelIni,
        const string & moduleName);

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

/**
 * @brief ASIM Model database.
 *
 * This class is a (very, very, very simple) database of models. It
 * keeps track of a set of models by their (real) file name.
 */
class ModelDB {
  public:
    // types

  private:
    // members
    const Workspace & workspace; ///< workspace to use

  public:
    // constructors / destructors
    /// Create a new model database.
    ModelDB(const Workspace & theWorkspace);
    /// Delete this model database.
    ~ModelDB();

    // accessors / modifiers
    bool CollectAllModels (void);
    bool CollectModels (const string & dirName);
};

#endif // _MODEL_ 
