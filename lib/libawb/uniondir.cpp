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
 * @brief Implementation of Union Directories
 */

// autoconf
#include "config.h"

// generic C
#include <glob.h>
#include <unistd.h>

// generic C++
#include <set>
#include <iostream>

// local
#include "uniondir.h"
#include "util.h"

/**
 * Create a union directory from a list of directories as search path and
 * the specified recursion behavior. The directories in the list are
 * separated by the "separtor" character (default is colon ":").
 */
UnionDir::UnionDir(
    const string & theSearchPath,         ///< sequence of directories
    const RecursionMode theRecursionMode, ///< recursion behavior
    const char separator)                 ///< separator char for searchPath
{
    recursionMode = theRecursionMode;

    // add all paths to searchPath
    SplitString spSplit(theSearchPath, separator);
    FOREACH (SplitString, it, spSplit) {
         searchPath.push_back(*it);
    }
}

/**
 * Default destructor.
 */
UnionDir::~UnionDir()
{
    // nada
}

/**
 * Find file name prefix, i.e. a directory from searchpath, where the
 * fileName can be found in the union directory.
 *
 * @return prefix where prefix/fileName exists
 */
string
UnionDir::GetPrefix (
    const string & fileName) ///< file name to get prefix for
const
{
    // support "normal" directories as well - uniondirs are always relative

    // This is broken for paths that are equivalent but textually different (jse)
    if (IsAbsolutePath(fileName)) {
        FOREACH_CONST (StringList, it, searchPath) {
            if (fileName.find(*it) == 0) {
                // searchPath is at beginning of fileName
                return *it;
            }
        }
        return "";
    }

    // relative file name
    string myFile;
    CanonicalFilename(fileName, myFile);
    string prefix;
    if (recursionMode == Flat) {
        // flat union dir - first hierarchy level defines which overlay to use
        SplitString split(myFile, '/');
        string dir = *(split.begin());

        FOREACH_CONST (StringList, it, searchPath) {
            if (FileExists(*it + "/" + dir)) {
                if (FileExists(*it + "/" + myFile)) {
                    prefix = *it;
                }
                break;
            }
        }
    } else {
        // recursive UnionDir
        FOREACH_CONST (StringList, it, searchPath) {
            if (FileExists(*it + "/" + myFile)) {
                prefix = *it;
                break;
            }
        }
    }

    return prefix;
}

/**
 * Find file name suffix, i.e. a relative filename path where
 * the file can be found in one of the elements of the searchpath.
 *
 * @return suffix where searchpath-element/suffix exists
 */
string
UnionDir::GetSuffix (
    const string & fileName) ///< file name to get prefix for
const
{
    string fullname;
    string prefix;

    fullname = FullName(fileName);
    if (fullname.empty()) {
        return "";
    }

    prefix = GetPrefix(fileName);
    if (prefix.empty()) {
        return "";
    }

    if (fullname.length() < prefix.length()) {
        return "";
    }

    return fullname.substr(prefix.length()+1);
}

/**
 * Generate the full (absolute) path for a file in the uniondir.
 *
 * @return full path (prefix/fileName)
 */
string
UnionDir::FullName (
    const string & fileName) ///< file name to get full name for
const
{
    // support "normal" directories as well - uniondirs are always relative
    if (IsAbsolutePath(fileName)) {
        return fileName;
    }

    // relative path
    string file;
    CanonicalFilename(fileName, file);
    string prefix = GetPrefix(file);
    bool exists = (prefix != "");

    string fullName;
    if (exists) {
        fullName = prefix + "/" + file;
    }

    return fullName;
}

/**
 * Check if the file exists in the uniondir
 *
 * @return true if file exists
 */
bool
UnionDir::Exists (
    const string & fileName) ///< file name to check
const
{
    string fullName = FullName(fileName);
    if (fullName == "") {
        return false;
    } else {
        return FileExists(fullName);
    }
}

/**
 * Check if fileName is a regular file (ie. not a directory)
 *
 * @return true for regular file
 */
bool
UnionDir::IsFile (
    const string & fileName) ///< file name to check
const
{
    string fullName = FullName(fileName);
    if (fullName == "") {
        return false;
    } else {
        return FileIsFile(fullName);
    }
}

/**
 * Check if fileName is a directory
 *
 * @return true for directory
 */
bool
UnionDir::IsDirectory (
    const string & fileName) ///< file name to check
const
{
    string fullName = FullName(fileName);
    if (fullName == "") {
        return false;
    } else {
        return FileIsDirectory(fullName);
    }
}

/**
 * Generate all file names that match the filePattern.
 */
void
UnionDir::Glob (
    const string & filePattern, ///< the file pattern to search for
    StringList & globs)         ///< result strings will be added here
const
{
    glob_t globbuf;  // interface to libc glob

    // support "normal" directories as well - uniondirs are always relative
    if (IsAbsolutePath(filePattern)) {
        glob(filePattern.c_str(), GLOB_BRACE, NULL, &globbuf);

        for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
            globs.push_back(globbuf.gl_pathv[i]);
        }
    } else {
        // relative pattern
        string cwd = GetCWD();

        // count number of directories in pattern
        SplitString patternSplit(filePattern, '/');
        int patternDirs = 0;
        FOREACH (SplitString, it, patternSplit) {
            patternDirs++;
        }

        if (recursionMode == Flat &&
            patternDirs > 1)
        {
            // flat union dir - first hierarchy level defines which
            // overlay to use
            // Note that globs on the first directory level behave like
            // recursive globs and are thus handled in the recursive part
            string dir = *(patternSplit.begin());
            FOREACH_CONST (StringList, it, searchPath) {
                if (FileExists(*it + "/" + dir)) {
                    // found first level in overlay *it, check for whole path
                    chdir((*it).c_str());
                    glob(filePattern.c_str(), GLOB_BRACE, NULL, &globbuf);
                    chdir(cwd.c_str());
                    break;
                }
            }
            for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
                globs.push_back(globbuf.gl_pathv[i]);
            }
        } else {
            // recursive union dir
            set<string> found; // keep track of which paths we've got already

            FOREACH_CONST (StringList, it, searchPath) {
                if (! FileIsDirectory(*it)) {
                    continue;
                }
                chdir((*it).c_str());
                glob(filePattern.c_str(), GLOB_BRACE, NULL, &globbuf);
                for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
                    pair<set<string>::iterator,bool> p;
                    p = found.insert(globbuf.gl_pathv[i]);
                    if (p.second) {
                        // this was a new path
                        globs.push_back(globbuf.gl_pathv[i]);
                    } else {
                        // got this path already
                        // nada
                    }
                }

                chdir(cwd.c_str());
            }
        }
    }

    globfree(&globbuf);
}

/**
 * Return the search path that is used for this union dir.
 * Used only for debugging purposes.
 */
const UnionDir::StringList &
UnionDir::GetSearchPath(void)
const
{
    return searchPath;
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
UnionDir::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "UnionDir::" << endl;
    string modeString;
    switch (recursionMode) {
      case Recursive: modeString = "recursive"; break;
      case Flat:      modeString = "flat"; break;
      default:        modeString = "-unknown-";
    }
    out << prefix << "  RecursionMode: " << modeString << endl;
    out << prefix << "  SearchPath:" << endl;
    FOREACH_CONST (StringList, it, searchPath) {
        out << prefix << "    " << *it << endl;
    }

    return out;
}


//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS
void TestGlob (char * searchPath, char * pattern)
{
    UnionDir unionDir(searchPath, UnionDir::Recursive, ':');
    UnionDir::StringList result;

    unionDir.Glob(pattern, result);
    
    cout << "glob:";
    FOREACH_CONST (UnionDir::StringList, it, result) {
      cout << " " << *it;
    }
    cout << endl;
}

void TestGetPrefix (char * searchPath, char * file)
{
    UnionDir unionDir(searchPath, UnionDir::Recursive, ':');

    cout << "prefix: " << unionDir.GetPrefix(file) << endl;
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
        if (string(argv[1]) == string("--glob")) {
            if (argc == 4) {
                TestGlob (argv[3], argv[2]);
            } else if (argc == 3) {
                TestGlob ("/", argv[2]);
            } else {
                ParseError (argv);
            }
        } else if (string(argv[1]) == string("--getprefix")) {
            if (argc == 4) {
                TestGetPrefix (argv[3], argv[2]);
            } else if (argc == 3) {
                TestGetPrefix ("/", argv[2]);
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
