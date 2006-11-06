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
 * @brief Windows .ini file reader / writer.
 */

#ifndef _INIFILE_
#define _INIFILE_ 1

// generic C++
#include <string>
#include <map>

using namespace std;

/**
 * @brief Windows .ini file reader / writer.
 *
 * This class provides support for reading and writing windows style ini
 * file with groups of name/value pairs that can be interogated and
 * manipulated by the program.
 *
 * File looks like:
 * <pre>
 * [Group1]
 * NAME1=VALUE1
 * NAME2=VALUE2
 *
 * [Group2]
 * NAME3=VALUE3
 * NAME4=VALUE4
 * </pre>
 */
class IniFile {
  private:
    // types
    typedef map<string, string> NameMap;    ///< map holding name=value pairs
    typedef map<string, NameMap> GroupMap;  ///< map associating group name
                                            ///< to its NameMap
    // consts
    // regular expressions for ini file line parsing
    // gcc 3.2.2 complains about these lines inside the class def.
//    static const char* const IncludeRegexp ="^#include[ \t]+\"([^\"]*[^ \t])\"";
//    static const char* const CommentRegexp = "^[ \t]*(#.*)?$";
//    static const char* const GroupRegexp = "^\\[([A-Za-z0-9/()+ :;,_-]*)\\]$";
//    static const char* const ItemRegexp  = "^([A-Za-z0-9_-]*) *= *(.*) *$";
    
    // members
    string fileName;   ///< the file name this object is associated with
    GroupMap groupMap; ///< top level map holds group-name -> NameMap entries

    // methods
    /// Parse the .ini file into internal data structures.
    bool Parse (const string & parseFileName);

  public:
    // constructors/destructors
    /// Initialize from existing file.
    IniFile(const string & theFileName);
    /// Destructor
    ~IniFile();

    // accessors / modifiers
    /// Get value of item itemName in group groupName.
    string Get (const string & groupName,
        const string & itemName,
        const string & defaultValue = "",
        const string & envName = "") const;
    //
    /// Set value and update .ini file on disk.
    void Put (const string groupName,
        const string itemName,
        const string value);

    /// Set value of item itemName in group groupName.
    void Set (const string & groupName,
        const string & itemName,
        const string & value);

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

#endif // _INIFILE_ 
