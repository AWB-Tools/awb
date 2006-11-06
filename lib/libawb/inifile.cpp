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

// generic C
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

// generic C++
#include <fstream>
#include <iostream>

// local
#include "inifile.h"
#include "util.h"

// gcc 3.2.2 complains about these lines inside the class def.
static const char* const IncludeRegexp ="^#include[ \t]+\"([^\"]*[^ \t])\"";
static const char* const CommentRegexp = "^[ \t]*(#.*)?$";
static const char* const GroupRegexp = "^\\[([A-Za-z0-9/()+ :;,_-]*)\\]$";
static const char* const ItemRegexp  = "^([A-Za-z0-9_-]*) *= *(.*) *$";

/**
 * Initialize from an existing Ini file.
 */
IniFile::IniFile (
    const string & theFileName) ///< file name of the .ini file
{
    bool status;

    fileName = theFileName;
    status = Parse (fileName); // parse fileName (and all its includes)
    if (!status) {
        exit(1);
    }
}

/**
 * Release data structures
 */
IniFile::~IniFile()
{
    // all members (objects) will themselves release their memory
    // nada
}

/**
 * Parse an existing .ini file into the internal data structures of this
 * class. Each group in the file will be represented as an entry in the
 * GroupMap. Each name/value pair will be represented as an entry in the
 * NameMap corresponding to its particular group.
 *
 * @return true on success
 */
bool
IniFile::Parse (
    const string & parseFileName) ///< file name to parse
{
    NameMap * currentNameMap = NULL;
    string line;

    // open ini file
    ifstream ini(parseFileName.c_str());
    if (!ini) {
        cerr << "Error: Can't open " << parseFileName << " for read" << endl;
        return false;
    }
 
    // parse ini file
    while (! ini.eof()) {
        getline (ini, line);
        if (line.empty() && ini.eof()) {
            break; // also eof
        }
        MatchString matchLine(line);
        MatchString::MatchArray matchArray;
        if (! matchLine.Match(IncludeRegexp, matchArray).empty()) {
            // we need to temporarily cd to the directory of parseFileName
            // in order to make relative includes work right
            string cwd = GetCWD();
            chdir (FileHead (parseFileName).c_str());

            bool status = Parse(matchArray[1]);
            if (!status) {
                return false;
            }

            // cd back to where we were before
            chdir (cwd.c_str());
        } else if (line.empty() ||
                   ! matchLine.Match(CommentRegexp).empty())
        {
            continue;
        } else if (! matchLine.Match(GroupRegexp, matchArray).empty()) {
            pair<string, NameMap> entry(matchArray[1], NameMap());
            pair<GroupMap::iterator, bool> result;
            result = groupMap.insert(entry);
            // the following syntactic mess gets a reference to the
            // NameMap corresponding to the group name
            currentNameMap = &((&(*(result.first)))->second);
        } else if (! matchLine.Match(ItemRegexp, matchArray).empty()) {
            if (! currentNameMap) {
                cout << "Name=Value pair outside of any group ignored!" << endl;
	        cout << "line --->" << line << "<---" << endl;
            } else {
                pair<string, string> entry(matchArray[1], matchArray[2]);
                currentNameMap->insert(entry);
            }
        } else {
	    cout << "Inifile Warning: unclassified line in file "
                 << parseFileName << endl;
	    cout << "line --->" << line << "<---" << endl;
        }
    }
    ini.close();

    return true; // success
}

/**
 * Search for itemName within groupName and return its value. If it is not
 * found, but envName is defined in the environment, return the contents
 * of that environment variable. Otherwise return defaultValue.
 *
 * @return value of item in group
 */
string 
IniFile::Get (
    const string & groupName,    ///< group to search in
    const string & itemName,     ///< item to search for
    const string & defaultValue, ///< default value fallback
    const string & envName)      ///< environment variable name fallback
const
{
    GroupMap::const_iterator groupIt = groupMap.find(groupName);
    if (groupIt != groupMap.end()) {
        NameMap::const_iterator nameIt = (*groupIt).second.find(itemName);
        if (nameIt != (*groupIt).second.end()) {
            return (*nameIt).second; // the value corresponding to itemName
        }
    }

    // invariant: either group or item was not found

    // check environment
    if ( ! envName.empty()) {
        char * env = getenv (envName.c_str());
        if (env) {
            return env;
        }
    }

    // invariant: neither ini file nor environment hold what we want
    return defaultValue;
}

/**
 * Search for itemName within groupName and set its value. If it is not
 * found, add a new enty for it.
 */
void
IniFile::Set (
    const string & groupName,  ///< group to search in
    const string & itemName,   ///< item to search for
    const string & value)      ///< value to set
{
    GroupMap::iterator groupIt = groupMap.find(groupName);
    if (groupIt == groupMap.end()) {
        // group does not exist - add it
        pair<string, NameMap> entry(groupName, NameMap());
        pair<GroupMap::iterator, bool> result;
        result = groupMap.insert(entry);
        // now we've got an iterator for it
        groupIt = result.first;
    }

    // insert name=value
    (*groupIt).second[itemName] = value;
}

/**
 * Set item itemName within group groupName to value and rewrite the
 * inifile (on disk).
 */
void
IniFile::Put (
      const string groupName,  ///< group to search in
      const string itemName,   ///< item to search for
      const string value)      ///< value to set
{
    // set the new value, so Get() will see it.
    Set (groupName, itemName, value);

    string line;

    // open input file
    ifstream in(fileName.c_str());
    if (!in) {
        cerr << "Error: Can't open " << fileName << " for read" << endl;
        exit(1);
    }
    // open output file
    string outFileName(fileName + ".new");
    ofstream out(outFileName.c_str());
    if (!out) {
        cerr << "Error: Can't open " << fileName << ".new for write" << endl;
        exit(1);
    }
 
    // try to find groupName and copy in -> out as we go along
    bool foundGroup = false;
    bool itemWritten = false;
    while (! in.eof()) {
        // read and copy
        getline (in, line);
        if (line.empty() && in.eof()) {
            break; // also eof
        }
        out << line << endl;

        MatchString matchLine(line);
        MatchString::MatchArray matchArray;

        if (! matchLine.Match(GroupRegexp, matchArray).empty() &&
            matchArray[1] == groupName)
        {
            foundGroup = true;
            break;
        }
    }

    // rewrite or add the item to the group
    if (foundGroup) {
        while (! in.eof()) {
            // read
            getline( in, line);
            if (line.empty() && in.eof()) {
                break; // also eof
            }
            MatchString matchLine(line);
            MatchString::MatchArray matchArray;

            if (! matchLine.Match(GroupRegexp, matchArray).empty()) {
                // found a new group - add item before it
                out << itemName << "=" << value << endl;
                out << line << endl;
                itemWritten = true;
                break;
            } else if (! matchLine.Match(ItemRegexp, matchArray).empty() &&
                matchArray[1] == itemName)
            {
                // found item - rewrite it
                out << itemName << "=" << value << endl;
                itemWritten = true;
                break;
            } else {
                // copy
                out << line << endl;
            }
        }
    }

    // copy rest of file
    while (! in.eof()) {
        // read and copy
        getline (in, line);
        if (line.empty() && in.eof()) {
            break; // also eof
        }
        out << line << endl;
    }

    // add group and item at end if it was not found earlier
    if ( ! itemWritten) {
        out << "[" << groupName << "]" << endl;
        out << itemName << "=" << value << endl;
    }

    // clean up
    in.close();
    out.close();

    // rename files
    if (rename (outFileName.c_str(), fileName.c_str())) {
        cerr << "IniFile::Put rename error" << endl
              << strerror(errno) << endl;
        exit (1);
    }
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
IniFile::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "IniFile::" << endl;
    out << prefix << "  Filename: " << fileName << endl;
    out << prefix << "  Data:" << endl;
    FOREACH_CONST (GroupMap, groupIt, groupMap) {
        out << prefix << "    [" << groupIt->first << "]" << endl;
        FOREACH_CONST (NameMap, nameIt, groupIt->second) {
            out << prefix << "    " 
                << nameIt->first << "=" << nameIt->second << endl;
        }
        out << endl;
    }

    return out;
}

//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

void TestIniFile (char * fileName, char * group = NULL, char * item = NULL)
{
    IniFile ini(fileName);
    ini.Dump(cout);

    if (group && item) {
        string value = ini.Get(group, item, "not found");
        cout << group << "::" << item << " = " << value << endl;

        ini.Set(group, item, "myNewValue");
        //ini.Put(group, item, "myNewValue");

        value = ini.Get(group, item, "not found");
        cout << group << "::" << item << " = " << value << endl;
    }
}

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
        if (string(argv[1]) == string("--read")) {
            if (argc == 3) {
                TestIniFile (argv[2]);
            } else {
                ParseError (argv);
            }
        } else if (string(argv[1]) == string("--set")) {
            if (argc == 5) {
                TestIniFile (argv[2], argv[3], argv[4]);
            } else {
                ParseError (argv);
            }
        } else {
            cerr << "can't parse " << argv[0]
                 << " argument " << argv[1] << endl;
            exit(1);
        }
    }
}

#endif // TESTS 
