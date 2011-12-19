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
 * @brief ASIM module Parameter Information
 */

#ifndef _MODPARAM_
#define _MODPARAM_ 1

// generic (C++)
#include <string>

// local
#include "util.h"

using namespace std;

/**
 * @brief ASIM module parameter (abstract)
 *
 * This class encapsulates all information that is known about an abstract
 * ASIM module parameter. @note This is the abstract parameter object, not
 * an instance of this parameter.  @see ModParamInstance
 */
class ModParam {
  public:
    // types
    /**
     * @brief Where is a parameter visible in the model
     *
     * The visibility setting tells us how a parameter has to be exported
     * to the module hierarchy. The paramerter can either be only visible 
     * to the module that declared it, or to all modules in the subtree
     * rooted at the module that declared it.
     */
    enum Visibility {
        Local,  ///< parameter visible only in module declaring it
        Subtree ///< parameter visible in module declaring it and all 
                ///< its submodules
    };
    /**
     * @brief Location where a parameter is defined / redefinable
     *
     * The idea of the location setting is to give an indication as to how
     * much flexiliblity the code can deal with with respect to redefining
     * a parameter. The enum is really an ordered list, where each level
     * is binding the parameter value later than the previous level. E.g.
     * if a parameter has Location == ModParam::Module, it can only be
     * defined in the Module config file. If a parameter has Location ==
     * ModParam::Model, its default is still defined in the Module, but
     * the value can be redefined in the Model config file, ie. its value
     * can be bound later. ModParam::Startup and ModParam::Runtime can
     * bind even later than Model.
     *
     * @note Bindings up to ModParam::Model result in the bound "values"
     * being compiled into the code and can not be changed later. They are
     * called "static". Bindings of ModParam::Startup and higher result in
     * "variables" for the parametes to be compiled in, and are called
     * "dynamic". These variables can be changed at startup / runtime.
     *
     * The location setting is used for two things:
     *  - to define the latest possible binding location of a parameter value
     *  - to define the actual binding location of a parameter value,
     *    which must obviously be <= to the latest possible location
     */
    enum Location {
        Module,  ///< definition in module .awb file
        Model,   ///< definition in model .apm file
        Startup, ///< definition dynamic at simulator invocation
        Runtime  ///< definition can change during runtime
    };

  private:
    // members
    string name;            ///< parameter name
    string desc;            ///< parameter description
    string type;            ///< parameter type
    string defaultValue;    ///< default value
    Visibility visibility;  ///< where is this param visible
    Location   mutability;  ///< where can this param be changed

  public:
    // constructors/destructors
    //ModParam();
    //~ModParam();

    // accessors / modifiers
    const string & GetName (void) const { return name; }
    void SetName (const string & theName) { name = theName; }
    //
    const string & GetDesc (void) const { return desc; }
    void SetDesc (const string & theDesc) { desc = theDesc; }
    //
    const string & GetType (void) const { return type; }
    void SetType (const string & theType) { type = theType; }
    //
    const string & GetDefault (void) const { return defaultValue; }
    void SetDefault (const string & theDefault) { defaultValue = theDefault; }
    //
    Visibility GetVisibility (void) const { return visibility; }
    void SetVisibility (const Visibility theVis) { visibility = theVis; }
    //
    Location GetMutability (void) const { return mutability; }
    void SetMutability (const Location theMut) { mutability = theMut; }
    /**
     * @brief Is this parameter a "dynamic" parameter (changable after build)
     * @return true if param is dynamic, false otherwise
     */
    bool
    IsDynamic (void) const
        { return (mutability == Startup || mutability == Runtime); }
    
    /**
     * @brief Is this parameter a "string" parameter
     * @return true if param is of "string" type, false otherwise
     */
    bool
    IsString (void) const
        { return (type == "string"); }
    
    // methods
    /// Parse a .awb param declaration/definition line
    bool Parse (const string & line);
    /// Set .awb param information from parsed declaration/definition line
    bool SetParsed (const string & line,
        const MatchString::MatchArray & matchArray);

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const; 
};

/**
 * @brief ASIM module parameter (instance)
 *
 * This class represents a module parameter instance. @see ModParam for
 * the abstract parameter object.
 */
class ModParamInstance {
  public:
    // types
    /// Interface type for (re)setting values.
    typedef ModParam::Location Location;

  private:
    const ModParam & param;  ///< the abstract parameter of this instance
    string value;            ///< current value
    Location source;         ///< where can this param was defined

  public:
    // constructors / destructors
    /// Create a new instance associated with given abstract parameter.
    ModParamInstance(const ModParam & theParam);
    //~ModParamInstance();

    // accessors / modifiers
    /// Get abstract module parameter for this concrete module parameter.
    const ModParam & GetModParam (void) const { return param; }
    /// Get source, ie. where this param instance value was defined.
    Location GetSource (void) const { return source; }
    /// Get current value of this param instance.
    const string & GetValue (void) const { return value; }
    /// Set the value and source of this parameter instance.
    void SetValue (const string & theValue, const Location theSource);

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const; 
};

#endif // _MODPARAM_ 
