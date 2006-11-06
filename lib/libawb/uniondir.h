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

#ifndef _UNIONDIR_
#define _UNIONDIR_ 1

// generic (C++)
#include <vector>
#include <string>

using namespace std;

/**
 * @brief Union directory (like Plan 9 and 4.4 BSD)
 *
 * UnionDir provides an overlayed namespace for directories, like the
 * "union directories" found in Plan 9 (flat) and in 4.4 BSD-Lite
 * (recursive).  The order of overlays is defined by the order of
 * directories in the search path.
 */
class UnionDir {
  public:
    // types
 
    /**
     * @brief Overlay behavior
     *
     * RecursionMode can be either Flat or Recursive:
     * "Flat" is like Plan 9 where only the first directory level
     * is considered; if a directory earlier in the search path
     * overlays that directory later in the search path, then all
     * files in the first directory are seen and non of the files
     * in the second directory are seen;
     * "Recursive" is like 4.4 BSD-Lite and overlays on a
     * file-by-file basis, merging directories to any depth
     * recursively;
     */
    enum RecursionMode {
        Recursive,  ///< overlay directory hierarchy recursively (like 4.4 BSD)
        Flat        ///< overlay only 1st-level directory (like Plan 9)
    };
    /// Interface type for container of strings
    typedef vector<string> StringList;

  private:
    // members
    StringList searchPath;       ///< list of directories to overlay
    RecursionMode recursionMode; ///< overlay behavior setting

  public:
    // constructors / destructors
    /// Constructor for compound search path string
    UnionDir(const string & theSearchPath, const RecursionMode theRecursionMode,
        const char separator = ':');
    /// Constructor for search path as iterator
    template <class InputIterator>
    UnionDir(InputIterator first, InputIterator last,
        RecursionMode theRecursionMode);
    /// Default destructor
    ~UnionDir();

    // methods
    /// Get the prefix (directory) where fileName is found
    string GetPrefix (const string & fileName) const;
    /// Get the suffix (relative path) where fileName can be found
    string GetSuffix (const string & fileName) const;
    /// Get the full (absolute) path where fileName is found
    string FullName (const string & fileName) const;
    /// Check if fileName exists in this union directory
    bool Exists (const string & fileName) const;
    /// Check if fileName is a regular file
    bool IsFile (const string & fileName) const;
    /// Check if fileName is a directory
    bool IsDirectory (const string & fileName) const;
    /// Return all files found in filePattern (shell-like glob, e.g. "foo.*")
    void Glob (const string & filePattern, StringList & result) const;
    /// Return the search path
    const StringList & GetSearchPath(void) const;

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

/**
 * Create a union directory from a sequence of strings (first and last
 * iterators) as search path and the specified recursion behavior.
 *
 * @note Requirements:
 * <ul>
 * <li> Iterators need operator++(int), operator!=(), string operator*().
 * </ul>
 */
template <class InputIterator>
UnionDir::UnionDir(
    InputIterator first, ///< iterator for first string in search path
    InputIterator last,  ///< iterator past last string in search path
    RecursionMode theRecursionMode) ///< recursion behavior for directories
{
    recursionMode = theRecursionMode;

    // add all paths to searchPath
    for ( ; first != last; first++) {
        searchPath.push_back( *first );
    }
}

#endif // _UNIONDIR_ 
