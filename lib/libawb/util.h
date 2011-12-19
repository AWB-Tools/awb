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
 * @brief Utilities for ASIM tools.
 */

#ifndef _UTIL_
#define _UTIL_ 1

// generic C
#include <regex.h>
#include <assert.h>

// generic (C++)
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// -----------------------------------------------------------------------------
// -------------------------- DEBUGGING FOR SPEED ------------------------------
// -----------------------------------------------------------------------------
#define SLECHTA_DEBUG 0
#if SLECHTA_DEBUG
#include <sys/time.h>
#include <time.h> 
extern int _indent;
#define SPEED_DEBUG(X)                                                        \
{                                                                             \
    time_t t = time(NULL);                                                    \
    struct tm _tm = *localtime(&t);                                           \
    struct timeval _tim;                                                      \
    gettimeofday(&_tim, NULL);                                                \
    fprintf(stderr, "%02d:%02d:%02d.%02d  ", _tm.tm_hour, _tm.tm_min, _tm.tm_sec, _tim.tv_usec/10000); \
    fflush(stderr);                                                           \
    for (int _i = 0; _i < _indent; ++_i) {                                    \
        cerr << "  ";                                                         \
    }                                                                         \
    cerr << X << endl;                                                        \
} 
#define SPEED_DEBUGN(N,X)                                                     \
if ((N) > 0)                                                                  \
{                                                                             \
    SPEED_DEBUG(X);                                                           \
    _indent += (N);                                                           \
}                                                                             \
else                                                                          \
{                                                                             \
    _indent += (N);                                                           \
    SPEED_DEBUG(X);                                                           \
}
#else
#define SPEED_DEBUG(X)
#define SPEED_DEBUGN(N,X)
#endif
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Iterator Macros
//----------------------------------------------------------------------------
/// Generic foreach loop on a container using iterators.
#define FOREACH(type,it,container) \
  for (type::iterator (it) = (container).begin(); \
       (it) != (container).end(); \
       (it)++)

/// Generic foreach loop on a container using constant iterators.
#define FOREACH_CONST(type,it,container) \
  for (type::const_iterator (it) = (container).begin(); \
       (it) != (container).end(); \
       (it)++)

/// Generic foreach loop on a container using explicit begin/end iterators.
#define FOREACH_ITER(type,it,begin,end) \
  for (type (it) = (begin); \
       (it) != (end); \
       (it)++)

//----------------------------------------------------------------------------
// Assertion Macros
//----------------------------------------------------------------------------
/// Assertion macro with user supplied additional message
#define ASSERT(condition,mesg) \
  if ( ! (condition)) { \
      cerr << "Assertion failure in " << __FILE__ << ": " << __LINE__ << endl; \
      cerr << mesg; \
      assert (false); \
  }

/// Simple assertion macro
#define ASSERTX(condition) \
  if ( ! (condition)) { \
      cerr << "Assertion failure in " << __FILE__ << ": " << __LINE__ << endl; \
      assert (false); \
  }

//----------------------------------------------------------------------------
// SplitString
//----------------------------------------------------------------------------

// need forward declaration for iterator class
class SplitString;

/**
 * @brief String splitting support (iterators).
 *
 * @see SplitString
 */
class SplitStringIterator {
  public:
    // types
    /// Interface type for constructor
    typedef string::size_type size_type;

    //consts
    /// End iterator position is encoded as npos
    static const size_type npos = string::npos;

  private:
    // members
    SplitString * splitString; ///< reference to associated SplitString
    string currentSubstr;      ///< substring this is currently pointing to
    size_type begin;           ///< index of next substring in SplitString

  public:
    // constructors / destructors
    /// Regular constructor.
    SplitStringIterator (
        SplitString * const st, ///< iterate over this SplitString
        const size_type idx)    ///< start position of iterator (npos == end)
    {
        splitString = st;
        begin = idx;
        if (idx != npos) {
            Next(); // initialize to first element
        }
    }

    /// Copy constructor
    SplitStringIterator (
        const SplitStringIterator & sti)  ///< iterator to copy
      : splitString(sti.splitString),
        currentSubstr(sti.currentSubstr),
        begin(sti.begin)
    { /* nada */ }

    // input iterator methods
    /// Post-increment operator
    SplitStringIterator & operator++ (int)
    {
        Next();
        return *this;
    }

    /// Check if two iterators are different
    bool operator!= (const SplitStringIterator & that) const
    {
        // a little warning here: it is assumed that the two iterators
        // actually iterate over the same string, since this is
        // (intentially) not checked here;
        return (this->begin != that.begin);
    }
        
    /// Dereference operator returns current substring reference
    string & operator* (void)
    {
        return currentSubstr;
    }

  private:
    /// Advance to next substring
    void Next (void);
};

/**
 * @brief String splitting support.
 *
 * The main purpose of this class is to provide support for splitting a
 * string into components. Each component is separated from the next by
 * any of a number of "separator" characters. If more than one separator
 * character is specified, the string is split on any of those characters.
 * This support is used for example to emulate the behavior of the "split"
 * function found in scripting languages (e.g. tcl, perl).
 *
 * The approach taken here is to provide a simple base class (SplitString)
 * and an iterator on this class (SplitStringIterator). The base class
 * holds the string and the separator characters and acts as an iterator
 * factory (SplitString::begin(), SplitString::end()). A typical usage
 * example crates a SplitString with the string that is to be split and the
 * separator characters, and then uses an iterator to get the components
 * of the split string.
 *
 * Example:
 * <pre>
 *   string myString, mySeparators;
 *   SplitString split(myString, mySeparators);
 *   for (SplitString::iterator it = split.begin();
 *        it != split.end();
 *        it++)
 *   {
 *       cout << "substr: " << *it << endl;
 *   }
 * </pre>
 */
class SplitString {
  public:
    // types
    /// Interface type for SplitString iterator
    typedef SplitStringIterator iterator;

  private:
    string str;             ///< the string to split
    string separators;      ///< separator characters to split by
    static const iterator endIterator; /// only one end iterator exists

  public:
    // constructors / destructors
    /// Construct from data string and separator char
    SplitString (const string & s, const char sep)
      : str(s),
        separators(1, sep) // string with 1 copy of char sep
    { /* nada */ }

    /// Construct from data string and separator string
    SplitString (const string & s, const string & sep)
      : str(s),
        separators(sep)
    { /* nada */ }
//    friend class iterator;
    friend class SplitStringIterator;
    /// The begin iterator (refereces first element)
    iterator begin(void) { return iterator (this, 0); }
    /// The end iterator (past the last element)
    iterator end(void) { return endIterator; }
};

//----------------------------------------------------------------------------
// MatchString
//----------------------------------------------------------------------------

/**
 * @brief String matching support.
 *
 * The class is a wrapper for the POSIX regex functionality.
 *
 * Example 1: (simple match)
 * <pre>
 *   string myString, myRegexp;
 *   MatchString matchString(myString);
 *   if ( ! matchString.Match(myRegexp).empty()) {
 *       cout << myString << " matches " << myRegexp << endl;
 *   }
 * </pre>
 *
 * Example 2: (match with submatches)
 * <pre>
 *   string myString, myRegexp;
 *   MatchString::MatchArray myMatchArray;
 *   MatchString matchString(myString);
 *   if ( ! matchString.Match(myRegexp, myMatchArray).empty()) {
 *       cout << myString << " matches " << myRegexp << endl;
 *       int i = 0;
 *       for (MatchString::MatchArray::const_iterator it = myMatchArray.begin();
 *           it != myMatchArray.end();
 *           it++, i++)
 *       {
 *           cout << "submatch[" << i << "] = " << *it << endl;
 *       }
 *   }
 * </pre>
 */
class MatchString {
  public:
    // types
    /// Interface type for arrary of submatches
    typedef vector<string> MatchArray;

  private:
    // consts
    static const int MAX_MATCH = 10; ///< max. number of submatches supported

    // members
    string str;  ///< the string to match

    // methods
    /// Print error message
    void Error (const int errcode, regex_t * preg) const;

  public:
    // constructors / destructors
    /// Construct from string
    MatchString (
        const string & s) ///< initializer string
      : str(s)
    { /* nada */ }

    /**
     * @brief Simple match - no submatches returned
     * @return overall match string or empty string if no match
     */
    string
    Match (
        const string & regexp, ///< (extended) regular expression to match
        int regFlags = 0)      ///< additional regexp flags (see regex.h)
    const
    {
        return Match (regexp, NULL, regFlags);
    }

    /**
     * @brief Match with submatches - match array reference
     * @return overall match string or empty string if no match
     */
    string
    Match (
        const string & regexp,   ///< (extended) regular expression to match
        MatchArray & matchArray, ///< return array for submatche strings
        int regFlags = 0)        ///< additional regexp flags (see regex.h)
    const
    {
        return Match (regexp, & matchArray, regFlags);
    }

    /// Match with submatches - match array  pointer
    string Match (const string & regexp, MatchArray * matchArray,
        int regFlags = 0) const;

    /// Substitution of match string or substring
    string Substitute (const string & regexp, const int idx,
        const string & subst, int regFlags = 0) const;
};

//----------------------------------------------------------------------------
// Misc String Functions
//----------------------------------------------------------------------------

/// Substitute all occurences of a substring.
string StringSubstituteAll (const string & inputString,
    const string & searchString, const string & replacementString);
/// Case-insensitive string compare.
int StringCmpNocase (const string & s1, const string & s2);
/// String to bool conversion.
bool StringToBool (const string & input);
/// Pretty print a long string into multiple lines.
string StringLineWrap (const string & str, string::size_type maxLength,
    const string & postfix, const string & prefix, bool wrapFirstLine = true);
/// Which side of string should be trimmed
enum StringTrimFlags {
  TrimLeft = 1,  ///< trim left side (beginning) of string
  TrimRight = 2, ///< trim right side (end) of string
  TrimBoth = 3   ///< trim both sides of string
};
/// Trim characters from beginning and end of string.
string StringTrim (const string & in, const StringTrimFlags flags = TrimBoth,
    const string & trimChars = " \t");
/// Replace tabs with appropriate number of spaces.
string StringDeTab (const string & in, int tabSize = 8);
/// Convert string to upper case
string StringToUpper (const string & in);
/// Convert string to lower case
string StringToLower (const string & in);
/// Remove CR and LF.  Doesn't check that they are at the end of the line.
string StringRemoveCRLF (const string & in);

//----------------------------------------------------------------------------
// Filename manipulations and test
//----------------------------------------------------------------------------

/// Expand ~user to home directory.
string ExpandTildeUser (const string & original);

/// Canonicalize a file name
bool CanonicalFilename (const string & original, string & canonical,
    const bool fail = true);

/// Join two file names.
string FileJoin (const string & name1, const string & name2);

/**
 * Check if a path is absolute (starting with "/"), or relative.
 *
 * @return true if path is absolute
 */
inline bool
IsAbsolutePath (
    const string & path) ///< the path to check
{
    return path[0] == '/';
}

/**
 * Check if a path is absolute (starting with "/"), or relative.
 *
 * @return true if path is relative
 */
inline bool
IsRelativePath (
    const string & path) ///< the path to check
{
    return ( ! IsAbsolutePath(path));
}

/// Check if a file exists
bool FileExists (const string & fileName);
/// Check if a file is a regular file
bool FileIsFile (const string & fileName);
/// Check if a file is a directory
bool FileIsDirectory (const string & fileName);
/// Check if a file is a symlink.
bool FileIsSymLink (const string & fileName);
/// Get head portion of file name.
string FileHead (const string & fileName);
/// Get tail portion of file name.
string FileTail (const string & fileName);
/// Get extension portion of file name.
string FileExtension (const string & fileName);
/// Get root portion of file name.
string FileRoot (const string & fileName);
/// Get directory name portion of file name.
string FileDirName (const string & fileName);
/// Get base name portion of file name.
string FileBaseName (const string & fileName);
/// Compute the relative path from path1 to path2.
string FileRelativePath (const string & path1, const string & path2);
/// Copy file from source to dest.
void FileCopy (const string & source, const string & dest);
/// Change file mode bits.
bool FileChmod (const string & fileName, mode_t mode);
/// Create a new directory.
void MakeDir (const string & dir, int mode = 0777);
/// Remove a directory.
bool RemoveDir (const string & dir, bool nukeSelf = true);
/// Get the current working directory.
string GetCWD (void);

#endif // _UTIL_ 
