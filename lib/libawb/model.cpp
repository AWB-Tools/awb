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

// generic (C++)
#include <iostream>

// local
#include "model.h"
#include "util.h"

//----------------------------------------------------------------------------
// class Model
//----------------------------------------------------------------------------

/**
 * Create a new model object.
 */
Model::Model (
    const Workspace & theWorkspace) ///< workspace this module belongs to
  : workspace(theWorkspace)
{
    rootModule = NULL;
    saveAllParams = false;
}

/**
 * Destroy this model object.
 */
Model::~Model()
{
    // nada
}

/**
 * Parse the file modelFile into the internal data structures of this
 * model.
 *
 * @return true on success, false on failure
 */
bool
Model::Parse (
    const string & modelFileName) ///< file name of model config file
{
    // check if this is the right file type that we can parse
    if ( !(    modelFileName.size() >= 4
            && modelFileName.substr(modelFileName.size()-4) == ".apm"))
    {
        cerr << "Model::Parse: can't parse file " << modelFileName << endl
             << "  I can only parse .apm files" << endl;
        return false;
    }
    // invariant: correct file type; ready to parse now

    UnionDir & sourceTree = workspace.GetSourceTree();
    if (sourceTree.IsDirectory (modelFileName)) {
        cerr << "Parsemodel: model points to directory instead of file - "
             << modelFileName << endl;
        return false;
    }

    string fullName = sourceTree.FullName (modelFileName);
    SetFileName (TranslateFileName (modelFileName));
    SetConfigFile (fullName);

    // .apm files have Ini file structure;
    // get an IniFile object for this model configuration
    IniFile modelIni(fullName);

    //
    // Check version number
    //
    string version = modelIni.Get ("Global", "Version", "unspecified");
    if (version == "1.0") {
	cerr << "Outdated version of model config file " << modelFileName
             << endl << "Run update utility 'cvs-config-to-apm'" << endl;
        return false;
    } else if (version[0] != '2') {
        cerr << "Unimplemented configuration version " << version << endl
             << "for model config file " << modelFileName << endl;
        return false;
    }

    //
    // Fetch global attributes from configuration file
    //
    string file = modelIni.Get ("Global", "File");
    // check file name for consistency with modelFileName
    char * temp  = strdup(modelFileName.c_str());
    string baseName = basename (temp);
    baseName =  baseName.substr(0, baseName.size()-4);
    free (temp);
    temp = NULL;
    if (file != baseName) {
        cerr << "Model::Parse: inconsistency in model file!" << endl
             << "  file read at " << modelFileName
             << " but claims to be File=" << file << endl;
        return false;
    }

    SetName (modelIni.Get ("Global", "Name"));
    SetDesc (modelIni.Get ("Global", "Description"));
    SetSaveAllParams (
        StringToBool (modelIni.Get ("Global", "SaveParameters", "TRUE")));

    //
    // Get model level attributes from configuration file
    //
    SplitString splitDefaultAttr (modelIni.Get (
          "Model", "DefaultAttributes"), " \t");
    FOREACH (SplitString, it, splitDefaultAttr) {
        if ( ! (*it).empty()) {
            AddDefaultAttribute (*it);
        }
    }

    //
    // Find module name of the root module from configuration file, and
    // then go parse that module instance and recursively its submodules.
    //
    string rootName = modelIni.Get ("Model", "model");
    SetRootModule (ParseModuleInstance (modelIni, rootName));

    return true;
}

/**
 * Parse and create the named module instance in the model configuration.
 * Also, recursively parse the submodules of this module instance.
 *
 * @return module instance of root module
 */
ModuleInstance *
Model::ParseModuleInstance (
    const IniFile & modelIni,  ///< ini file object of this model config file
    const string & moduleName) ///< module name to parse from ini file
{
    //
    // Get name of file for the module, and parse the module itself, and
    // put name of file in module object
    //
    string moduleFileName = modelIni.Get (moduleName, "File");
    //
    // If the module is really a submodel then read in the submodel
    // and return the root module
    //
    if (moduleFileName.find(".apm") != string::npos) {
        Model * submodel = new Model(workspace);

        if (! submodel->Parse(moduleFileName)) {
            cerr << "can't parse submodel" << moduleFileName << endl;
            return NULL;
        }

        ModuleInstance * submodelRootModule = submodel->GetRootModule();
        // we are just interested in the modules, not the model; since we
        // don't keep track of the submodel we created, we have to clean
        // it up here as well;
        delete submodel;

        return  submodelRootModule;
    }
            

    //FIXME: we don't really want to create a new abstract module for this
    // file name, but we do it now to test things out; this should query the
    // ModuleDB instead;
    Module * module = new Module(workspace);
    if ( ! module->Parse (moduleFileName)) {
        cerr << "can't instantiate module " << moduleFileName << endl;
        return NULL;
    }
    ModuleInstance * moduleInstance = new ModuleInstance (*module);

    //
    // For each parameter of this module,
    // update those that are specified in the model config file
    //
    string paramGroup = moduleName + "/Params";

    FOREACH_CONST (ModuleInstance::ModParamInstanceList, it,
        moduleInstance->GetParam())
    {
        ModParamInstance & modParamInstance = **it;
        const string & paramName = modParamInstance.GetModParam().GetName();
        string modelParamValue = modelIni.Get (paramGroup, paramName);
        if ( ! modelParamValue.empty()) {
            if (modParamInstance.GetModParam().IsString()) {
                // it better is quoted
                if (modelParamValue[0] != '"' ||
                    modelParamValue[modelParamValue.length() - 1] != '"')
                {
                    cerr << "Error: string param '" << paramName
                         << "' in apm file lacks quote ("
                         << modelParamValue << ")" << endl;
                    return NULL;
                }
                // strip the quotes from the new value...
                modelParamValue =
                    modelParamValue.substr(1, modelParamValue.length() - 2);
            }
            modParamInstance.SetValue (modelParamValue, ModParam::Model);
        }
    }

    //
    // For each module type that this module requires,
    // find the name of the module that is being used, and
    // parse it recursively
    //
    string requiresGroup = moduleName + "/Requires";

    int idx = 0;
    FOREACH_CONST (Module::StringList, it, module->GetRequires()) {
        const string & requiresType = *it;
        string requiresName = modelIni.Get (requiresGroup, requiresType);
        ModuleInstance * subModule = NULL;
        if ( ! requiresName.empty()) {
            // parse submodule instance recursively
            subModule = ParseModuleInstance (modelIni, requiresName);
        } else {
            cerr <<
            cerr << "Can't find required module '" << requiresType
                 << "' in model '" << moduleName << "'" << endl;
        }

        if (subModule &&
            subModule->GetModule().GetProvides() != requiresType)
        {
            cerr << "Model::ParseModuleInstance: Module type mismatch:" << endl
                 << " wanted " << requiresType << " got "
                 << subModule->GetModule().GetProvides() << endl;
            subModule = NULL;
        }
        // now add the submodule instance to this module instance
        moduleInstance->SetSubModule (idx, subModule);
        idx++;
    }

    return moduleInstance;
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
Model::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "Model::" << endl;
    out << prefix << "  Name: " << GetName() << endl;
    out << prefix << "  Desc: " << GetDesc() << endl;
    out << prefix << "  FileName: " << GetFileName() << endl;
    out << prefix << "  ConfigFile: " << GetConfigFile() << endl;
    out << prefix << "  DefaultAttributes:" << endl;
    FOREACH_CONST (StringList, it, defaultAttributeList) {
        out << prefix << "    " << *it << endl;
    }
    out << prefix << "  SaveAllParams: " << IsSaveAllParams() << endl;
    if (GetRootModule()) {
        out << prefix << "  RootModule:" << endl;
        GetRootModule()->Dump(out, prefix + "    ");
    } else {
        out << prefix << "  RootModule: (NULL)" << endl;
    }

    return out;
}

//----------------------------------------------------------------------------
// class ModelDB
//----------------------------------------------------------------------------

/**
 * Create a model database object
 */
ModelDB::ModelDB (
    const Workspace & theWorkspace) ///< workspace this module belongs to
  : workspace(theWorkspace)
{
    // nada
}

/**
 * Destroy this module database object
 */
ModelDB::~ModelDB()
{
    // nada
}

/**
 * Collect all models found in the current workspace.
 *
 * @return true for success, false otherwise
 */
bool
ModelDB::CollectAllModels (void)
{
    return false; // FIXME
}

/**
 * Collect all models found in the file system (sourceTree) subtree rooted
 * at the given directory name.
 *
 * @return true for success, false otherwise
 */
bool
ModelDB::CollectModels (
    const string & dirName) ///< root of directory tree to collect
{
    return false; // FIXME
    return (dirName.empty()); // keep compiler happy (unused variable)
}

//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

void TestModel (int argc, char ** argv)
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
        model.Dump(cout);
    }

    delete workspace;
}

int main (int argc, char ** argv)
{
#if 0
    if (argc >= 2) {
        TestModel (argc, argv);
    }
#endif

#if 0
    if (argc >= 1) {
        TestModelDB (argc, argv);
    }
#endif
}

#endif // TESTS 
