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

// generic (C++)
#include <iostream>

// local
#include "modparam.h"

//----------------------------------------------------------------------------
// class ModParam
//----------------------------------------------------------------------------

/**
 * Parse the string in line into the internal data structures of this
 * object.
 *
 * @return true for success, false for failure
 */
bool
ModParam::Parse (
    const string & line) ///< the line to be parsed
{
    MatchString matchLine(line);
    MatchString::MatchArray matchArray;

    // note: all cases need to produce the following for SetParsed()
    // matchArray[]  0     1      2      3     4     5      6      7    8
    //              all command dummy dynamic name quote default quote desc
    if ( ! matchLine.Match (
          "%(param|export|const)[[:space:]]+(%(dynamic))?[[:space:]]*([^[:space:]]*)[[:space:]]*(\")([^\"]*)(\")[[:space:]]*\"([^\"]*)\"",
          matchArray).empty())
    {
        // quoted default value
        return SetParsed (line, matchArray);
    } else if ( ! matchLine.Match (
          "%(param|export|const)[[:space:]]+(%(dynamic))?[[:space:]]*([^[:space:]]*)[[:space:]]*()([^[:space:]]*)()[[:space:]]*\"([^\"]*)\"",
          matchArray).empty())
    {
        // unquoted default value
        return SetParsed (line, matchArray);
    } else {
        cerr << "ParseParam: unknown statement " << endl
             << line << endl;
        return false;
    }

    return true;
}

/**
 * Setup parameter from a parsed match array. The array has the following
 * layout:
 * <pre>
 * matchArray[]  0     1      2      3     4     5      6      7    8
 *              all command dummy dynamic name quote default quote desc
 * </pre>
 *
 * @return true for success, false for failure
 */
bool
ModParam::SetParsed (
    const string & line, ///< the parsed line (for error messages)
    const MatchString::MatchArray & matchArray) ///< parsed input line
{
    if (matchArray[5] != matchArray[7]) {
        cerr << "ParseParam: double quote mismatch in default value" << endl
             << line << endl;
        return false;
    }
    SetName (matchArray[4]);
    SetDesc (matchArray[8]);
    SetDefault (matchArray[6]);
    if (matchArray[5] == "\"") {
        SetType ("string");
    } else {
        SetType ("uint32"); // constant for now
    }

    //
    // correspondence to current nomenclature:
    //                        visibility   mutability
    //   %param ............. local        model
    //   %param %dynamic .... local        startup
    //   %export ............ subtree      model
    //   %export %dynamic ... subtree      startup
    //   %const ............. subtree      module
    //
    if (matchArray[1] == "param") {
        SetVisibility (Local);
    } else {
        SetVisibility (Subtree);
    }
    if (matchArray[1] != "const") {
        if (matchArray[3].empty()) {
            SetMutability (Model);
        } else {
            SetMutability (Startup);
        }
    } else {
        if (matchArray[3].empty()) {
            SetMutability (Module);
        } else {
            cerr << "ParseParam: can't have %const %dynamic in line"
                 << endl << line << endl;
            return false;
                 
        }
    }

    return true;
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
ModParam::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print in front of each line
const
{
    out << prefix;
    switch (GetVisibility()) {
        case Local:   out << "[Local] ";   break;
        case Subtree: out << "[Subtree] "; break;
        default:      out << "[?vis?] ";
    }
    switch (GetMutability()) {
        case Module:  out << "[Module] ";  break;
        case Model:   out << "[Model] ";   break;
        case Startup: out << "[Startup] "; break;
        case Runtime: out << "[Runtime] "; break;
        default:      out << "[?mut?] ";
    }
    out << GetName() << " = " << GetDefault() << " # " << GetDesc() << endl;

    return out;
}

//----------------------------------------------------------------------------
// class ModParamInstance
//----------------------------------------------------------------------------

/**
 * Create a module parameter instance object and fill value with default value
 * of module paramter object.
 */
ModParamInstance::ModParamInstance(
    const ModParam & theParam) ///< reference to abstract module parameter
  : param(theParam)
{
    SetValue (param.GetDefault(), ModParam::Module);
}

/**
 * Set the value of the parameter instance and the location where this value
 * is from.
 */
void
ModParamInstance::SetValue (
    const string & theValue,  ///< new paramter intstance value
    const Location theSource) ///< location where value is defined
{
    value = theValue;
    source = theSource;
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
ModParamInstance::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print in front of each line
const
{
    param.Dump(out, prefix + "abstract: ");
    out << prefix << "concrete: Value = " << GetValue() << " ";
    switch (GetSource()) {
        case ModParam::Module:  out << "[Module] ";  break;
        case ModParam::Model:   out << "[Model] ";   break;
        case ModParam::Startup: out << "[Startup] "; break;
        case ModParam::Runtime: out << "[Runtime] "; break;
        default:                out << "[?src?] ";
    }
    out << endl;

    return out;
}


//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

void ParseError (char ** argv)
{
    cerr << "can't parse " << argv[0]
         << " command line for " << argv[1] << endl;
    exit(1);
}

int main (int argc, char ** argv)
{
    //
    // very simple argument style: first argument --foo indicates that
    // the rest of the arguments belong to the foo test
    //
    if (argc >= 2) {
        if (string(argv[1]) == string("--nada")) {
        } else {
            cerr << "can't parse " << argv[0]
                 << " argument " << argv[1] << endl;
            exit(1);
        }
    }
}

#endif // TESTS 
