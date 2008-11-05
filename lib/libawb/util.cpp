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

// autoconf
#include "config.h"

// generic C
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>

// generic C++
#include <list>
#include <fstream>

// local
#include "util.h"

using namespace std;


// -----------------------------------------------------------------------------
// -------------------------- DEBUGGING FOR SPEED ------------------------------
// -----------------------------------------------------------------------------
#if SLECHTA_DEBUG
int _indent = 0;
#endif
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// SplitString
//----------------------------------------------------------------------------

const SplitString::iterator
      SplitString::endIterator(NULL, SplitString::iterator::npos);

/**
 * Generate the next substring that will be returned by the iterator.
 */
void
SplitStringIterator::Next (void)
{
    size_type end;
    size_type oldBegin;
    size_type separatorIdx;

    if (
        // we are in the first iteration
        begin == 0 &&
        // we have an non-empty string (otherwise corner case)
        splitString->str.length() != 0 &&
        // the first char is a separator character
        splitString->separators.find_first_of(splitString->str[0]) != npos)
    {
        // corner case: empty first substring (str starts with separator)
        //currentSubstr = "<empty-begin>";
        currentSubstr = "";
        begin = 1; // advance past first separator
    } else if (begin >= splitString->str.length()) {
        // handle termination or iterator sequence
        if (
            // we have a string longer than 1 char (otherwise corner case)
            splitString->str.length() > 1 &&
            // we are in the last iteration
            begin == splitString->str.length() &&
            // the last char is a separator character
            splitString->separators.find_first_of(
                splitString->str[splitString->str.length()-1]) != npos)
        {
            // corner case: empty last substring (str ends with separator)
            //currentSubstr = "<empty-end>";
            currentSubstr = "";
            begin = splitString->str.length() + 1; // do one more iteration
        } else {
            begin = npos; // nothing more to be done; this iter is end()
        }
    } else {
        // regular case during iteration
        oldBegin = begin;

        separatorIdx = splitString->str.find_first_of(
            splitString->separators, begin);
        if (separatorIdx != npos) {
            end = separatorIdx - 1;
            begin = separatorIdx + 1; // for next iteration
        } else {
            end = splitString->str.length();
            begin = splitString->str.length(); // for next iteration
        }

        currentSubstr = 
            splitString->str.substr(oldBegin, end - oldBegin + 1);
    }
}

//----------------------------------------------------------------------------
// MatchString
//----------------------------------------------------------------------------

/**
 * Match the (member) string str against regexp. Optionally returns an
 * array of submatches.
 *
 * @return match string or empty string if no match (ie. perl's $0)
 */
string
MatchString::Match (
    const string & regexp,   ///< (extended) regular expression to match
    MatchArray * matchArray, ///< return array for submatch strings
    int regFlags)            ///< additional regexp flags (see regex.h)
const
{
    regex_t preg;
    int result;
    string matchString;

    result = regcomp( &preg, regexp.c_str(), regFlags | REG_EXTENDED);
    if (result != 0) {
        Error (result, &preg);
    }

    regmatch_t pmatch[MAX_MATCH];
    result = regexec( &preg, str.c_str(), MAX_MATCH, pmatch, 0);

    if (matchArray && result != REG_NOMATCH) {
        for (int i = 0; i < MAX_MATCH; i++) {
            if (pmatch[i].rm_so == -1) {
                // this submatch did not yield a result
                matchArray->push_back ("");
                // if we would just ignore these empty subexpressions, the
                // numbering of following subexpressions would get all messed
                // up
            } else {
                matchArray->push_back (
                    str.substr(pmatch[i].rm_so,
                        pmatch[i].rm_eo - pmatch[i].rm_so));
            }
        }
    }

    if (result == REG_NOMATCH) {
        matchString = "";
    } else {
        matchString = str.substr(pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
    }
    //
    // regex must free internally allocated memory
    regfree (&preg);

    return matchString;
}

/**
 * Do a regular expression match on regexp and replace (sub)string index
 * with subst. The replaced string is returned. The string of this object
 * is NOT changed. Submatch indices are numberd 0..N, where 0 is the
 * complete match, and 1..N are the submatched (ie. 1st..nth parenthesized
 * expression in the regexp). If there is no match, the original string is
 * returned. If there are fewer submatches than index, an error is
 * generated.
 *
 * @return original string with substitution performed
 */
string
MatchString::Substitute (
    const string & regexp,   ///< (extended) regular expression to match
    const int idx,           ///< substring to replace: 0 = complete match,
                             ///< 1..N = submatches
    const string & subst,    ///< replacement string
    int regFlags)            ///< additional regexp flags (see regex.h)
const
{
    regex_t preg;
    int result;
    string matchString;

    result = regcomp( &preg, regexp.c_str(), regFlags | REG_EXTENDED);
    if (result != 0) {
        Error (result, &preg);
    }

    regmatch_t pmatch[MAX_MATCH];
    result = regexec( &preg, str.c_str(), MAX_MATCH, pmatch, 0);

    if (result == REG_NOMATCH) {
        // if there is no match, we return the original string
        // ie. we don't perform any substitution
        matchString = str;
    } else {
        if (idx >= MAX_MATCH) {
            cerr << "MatchString::Substitute: exceeding MAX_MATCH" << endl;
            exit (1);
        }
        if (pmatch[idx].rm_so == -1) {
            cerr << "MatchString::Substitute: "
                 << "insufficient number of substrings" << endl;
            exit (1);
        }

        matchString = str;
        matchString.replace(pmatch[idx].rm_so,
            pmatch[idx].rm_eo - pmatch[idx].rm_so, subst);
    }

    //
    // regex must free internally allocated memory
    regfree (&preg);

    return matchString;
}

/**
 * Report an regexp compilation error.
 */
void
MatchString::Error (
    const int errcode,  ///< error code that was reported
    regex_t * preg)     ///< regex internal datastructure (see regex.h)
const
{
    if (errcode != 0) {
        const int ERRBUF_SIZE = 256;
        char errbuf [ERRBUF_SIZE];
        regerror (errcode, preg, errbuf, ERRBUF_SIZE);
        cerr << "regexp error: " << endl
             << errbuf << endl;
    }
}

//----------------------------------------------------------------------------
// Misc String Functions
//----------------------------------------------------------------------------

/**
 * Substitute all occurances of seachString with replacementString in
 * inputString. The result is returned, inputString is not modified.
 *
 * @return input with replacements done
 */
string
StringSubstituteAll (
    const string & inputString,       ///< replacement done on this string
    const string & searchString,      ///< substring that is replaced
    const string & replacementString) ///< string it is replaced with
{
    string out = inputString;
    string::size_type idx;

    while ((idx = out.find(searchString)) != string::npos) {
        out = out.replace (idx, searchString.length(), replacementString);
    }

    return out;
}

/**
 * Case insensitive compare of two strings.
 *
 * @return -1 if s1 < s2, 0 if s1 == s2, +1 if s1 > s2
 */
int
StringCmpNocase (
    const string & s1, ///< first string to compare
    const string & s2) ///< second string to compare
{
    string::const_iterator it1 = s1.begin();
    string::const_iterator it2 = s2.begin();

    while (it1 != s1.end() && it2 != s2.end()) {
        char c1 = toupper(*it1);
        char c2 = toupper(*it2);
        if (c1 != c2) {
            return (c1 < c2) ? -1 : +1;
        }
        it1++;
        it2++;
    }

    return (s1.size() == s2.size()) ? 0
             : ((s1.size() < s2.size()) ? -1 : +1);
}

/**
 * Translate the input string into a boolean value:
 * false:
 *   empty string, "0", "OFF", "NO", "FALSE"
 * true:
 *   "1", "ON", "YES", "TRUE"
 *
 * @return boolean interpretation of input string
 */
bool
StringToBool (
    const string & input) ///< input string
{
    if (   input.empty()
        || input == "0"
        || StringCmpNocase(input, "OFF") == 0
        || StringCmpNocase(input, "NO") == 0
        || StringCmpNocase(input, "FALSE") == 0)
    {
        return false;
    } else if (
           input == "1"
        || StringCmpNocase(input, "ON") == 0
        || StringCmpNocase(input, "YES") == 0
        || StringCmpNocase(input, "TRUE") == 0)
    {
        return true;
    } else {
        cerr << "StringToBool: can't determine boolean value for "
             << input << endl;
        exit (1);
    }
}

/**
 * Breaks up the input string into muliple lines, each line being at most
 * maxLength characters long. Each internal line wrap is appended by the
 * postfix string, and each internal new line is preceeded by the prefix
 * string. Lines are only wrapped at word boundaries in order to "look
 * nice".
 *
 * @return the line-wrapped string
 */
string
StringLineWrap (
    const string & str,          ///< input string to line-wrap
    string::size_type maxLength, ///< maximal length (characters) of each line
    const string & postfix,      ///< postfix string for internal line breaks
    const string & prefix,       ///< prefix string for internal line breaks
    bool wrapFirstLine)          ///< first line is immediately wrapped
{
    string input = str;
    string wrapped;

    while (input.length() > 0) {
      string line;
        if (input.length() > maxLength) {
            // find a convenient word break
            string::size_type spaceIndex = input.rfind (' ', maxLength - 1);
            if (spaceIndex == string::npos) {
                spaceIndex = input.find (' ');
                if (spaceIndex == string::npos) {
                    spaceIndex = input.length();
                }
            }
            // split string into first part (up to space char)
            // and second part (after space char)
            line = input.substr (0, spaceIndex + 1);
            input = input.replace (0, spaceIndex + 1, "");
        } else {
            line = input;
            input.clear();
        }

        if ( ! wrapFirstLine && wrapped.empty()) {
            // first line
            wrapped = line;
        } else {
            // continuation line
            wrapped += postfix + "\n" + prefix + line;
        }
    }

    return wrapped;
}

/**
 * Trim string, ie. remove trim characters from beginning and end of
 * input string. Any sequence made up of characters found in trimChars
 * will be removed, ie. trimChars behaves like a [] regular expression.
 *
 * @return string after trimming
 */
string
StringTrim (
    const string & in,           ///< input string
    const StringTrimFlags flags, ///< which side of string to trim
    const string & trimChars)    ///< characters that will get trimmed
{
    string::size_type first = 0;
    string::size_type last = in.length() - 1;

    if (flags & TrimLeft) {
        first = in.find_first_not_of(trimChars);
        if (first == string::npos) {
            // no non-trim characters at all
            return "";
        }
    }

    if (flags & TrimRight) {
        last = in.find_last_not_of(trimChars);
        if (last == string::npos) {
            // no non-trim characters at all
            return "";
        }
    }

    return in.substr (first, last - first + 1);
}

/**
 * Replace tabs by appropriate number of spaces.
 *
 * @return de-tabbed string
 */
string
StringDeTab (
    const string & in, ///< input string to convert
    int tabSize)       ///< number of spaces per tab
{
    string out;
    int idx = 0;
    FOREACH_CONST (string, it, in) {
        if (*it == '\t') {
            out += " ";
            idx++;
            for (int i = tabSize - (idx % tabSize); i > 0; i--) {
                out += " ";
                idx++;
            }
        } else {
            out += *it;
            idx++;
        }
    }

    return out;
}

/**
 * Convert the input string to all upper case characters.
 *
 * @return all upper case result string
 */
string
StringToUpper (
    const string & in) ///< input string to convert
{
    string out;
    FOREACH_CONST (string, it, in) {
        out += toupper (*it);
    }

    return out;
}

/**
 * Convert the input string to all lower case characters.
 *
 * @return all lower case result string
 */
string
StringToLower (
    const string & in) ///< input string to convert
{
    string out;
    FOREACH_CONST (string, it, in) {
        out += tolower (*it);
    }

    return out;
}

/**
 * Remove all CR and LF characters from the input string.
 *
 * @return the original string with CR and LF removed.
 */
string
StringRemoveCRLF (
    const string & in) ///< input string to convert
{
    string out;

    out = StringSubstituteAll(in, "\n", "");
    out = StringSubstituteAll(out, "\r", "");

    return out;
}

//----------------------------------------------------------------------------
// Filename Manipulation and File Tests
//----------------------------------------------------------------------------

/**
 * If the input string starts with ~(<username>)?, it is replaced with the
 * (absolute) path to the user's home directory. Otherwise the input
 * string is returned unmodified.
 *
 * @return expanded string
 */
string
ExpandTildeUser (
    const string & original)  ///< file name that needs expansion
{
    string expanded(original);

    if (expanded[0] == '~') {
        glob_t globbuf;

        string::size_type slashIdx = expanded.find('/');
        glob(expanded.substr(0, slashIdx).c_str(), GLOB_TILDE, NULL, &globbuf);
        // we can have 0 or 1 matches on ~<user> glob
        if (globbuf.gl_pathc == 1) {
            expanded.replace(0, slashIdx, globbuf.gl_pathv[0]);
            // home dir found; split and put components onto path stack
        } else {
            // user not found; leave original
        }
        globfree(&globbuf);
    }

    return expanded;
}

/**
 * Canonicalization of the following path components is performed:
 *   - ~<user> ... replaced by user's home directory
 *   - "." ....... dropped
 *   - ".." ...... dropped and removes previous directory; absolute paths
 *                 clamp at root, relative paths must not go back beyond
 *                 beginning of the filename;
 *
 * @return true on success
 */
bool
CanonicalFilename (
    const string & original,  ///< file name that is canonicalized
    string & canonical,       ///< canonicalized file name
    const bool fail)          ///< if true, fail if too many ".." in filename
{
    vector<string *> path;

    //
    // do some preconditioning:
    // if filename starts with ~\S*, expand it as a user home dir
    //
    canonical = ExpandTildeUser(original);

    //
    // Split up the filename into its component directories and
    // canonicalize as we go along. Result will end up as a vector of
    // directories in path.
    //
    SplitString split(canonical, '/');
    bool isUnderflow = false;
    bool isFirst = true;
    FOREACH (SplitString, it, split) {
        string & dir = *it;
        if (dir == ".") {
            // current dir component can just be ignored
        } else if (dir == "..") {
            // parent dir component removes previous directory from path
            if (IsAbsolutePath(canonical)) {
                // absolute paths: always maintain initial "/"
                if (path.size() > 1) {
                    delete path.back();
                    path.pop_back();
                }
            } else {
                if (path.size() == 0) {
                    // relative path:
                    // trying to go back past the beginning of filename
                    // is an illegal operation
                    if (fail) {
                        cerr << "CanonicalFilename:" << endl
                             << "\tCan't .. beyond beginning of path in "
                             << original << endl;
                        exit (1);
                    } else {
                        isUnderflow = true;
                        break;
                    }
                } else {
                    delete path.back();
                    path.pop_back();
                }
            }
        } else {
            // default case just put component directory on path
            path.push_back( new string(dir) );
        }
        isFirst = false;
    }

    //
    // Take the path vector and convert it back to a string
    //
    canonical.clear();
    isFirst = true;
    FOREACH (vector<string*>, it, path) {
        if (! isFirst) {
            canonical += "/";
        }
        if (! isUnderflow) {
            canonical += **it;
        }
        delete *it; // free string

        isFirst = false;
    }

    return (! isUnderflow);
}

/**
 * Join two path names. If the name path is an absolute path, the first
 * name is dropped. "/" is inserted at the join point as needed.
 *
 * @return the joined path
 */
string
FileJoin (
    const string & name1,  ///< first path name
    const string & name2)  ///< second path name
{
    if (IsAbsolutePath(name2) || name1.empty()) {
        return name2;
    }

    if (name1[name1.length() - 1] == '/') {
        return name1 + name2;
    } else {
        return name1 + "/" + name2;
    }
}

/**
 * Check if a file exists
 *
 * @return true if fileName exists
 */
bool
FileExists (
    const string & fileName) ///< the file to check
{
    struct stat statbuf;
    if (stat (fileName.c_str(), &statbuf) == 0) {
        return true;
    } else {
        // some error has occured, so we assume file does not exist
        return false;
    }
}

/**
 * Check if a file is a regular file
 *
 * @return true if fileName is a regular file
 */
bool
FileIsFile (
    const string & fileName) ///< the file to check
{
    struct stat statbuf;
    if (stat (fileName.c_str(), &statbuf) == 0) {
        return (S_ISREG(statbuf.st_mode));
    } else {
        // some error has occured, so we assume file does not exist
        return false;
    }
}

/**
 * Check if a file is a directory file
 *
 * @return true if fileName is a directory
 */
bool
FileIsDirectory (
    const string & fileName) ///< the file to check
{
    struct stat statbuf;
    if (stat (fileName.c_str(), &statbuf) == 0) {
        return (S_ISDIR(statbuf.st_mode));
    } else {
        // some error has occured, so we assume file does not exist
        return false;
    }
}

/**
 * Check if a file is a symbolic link.
 *
 * @return true if fileName is a directory
 */
bool
FileIsSymLink (
    const string & fileName) ///< the file to check
{
    struct stat statbuf;
    if (lstat (fileName.c_str(), &statbuf) == 0) {
        return (S_ISLNK(statbuf.st_mode));
    } else {
        // some error has occured, so we assume file does not exist
        return false;
    }
}

/**
 * Get the head portion of filename. E.g. if file name is
 * /dir1/dir2/.../dirN/file.ext this function returns
 * /dir1/dir2/.../dirN
 *
 * Similar to [t]csh :h
 *
 * @return head portion of filename
 */
string
FileHead (
    const string & fileName) ///< file name to analyze
{
    string::size_type separator = fileName.rfind ('/');
    if (separator == string::npos) {
        // no separator - return full string
        return fileName;
    } else {
        return fileName.substr (0, separator);
    }
}

/**
 * Get the tail portion of filename. E.g. if file name is
 * /dir1/dir2/.../dirN/file.ext this function returns file.ext
 *
 * Similar to [t]csh :t
 *
 * @return tail portion of filename
 */
string
FileTail (
    const string & fileName) ///< file name to analyze
{
    string::size_type separator = fileName.rfind ('/');
    if (separator == string::npos) {
        // no separator - return full string
        return fileName;
    } else {
        if (fileName.length() == separator + 1) {
            return ""; // separator is last char - tail is empty
        } else {
            return fileName.substr (separator + 1);
        }
    }
}

/**
 * Get the extension portion of filename. E.g. if file name is
 * /dir1/dir2/.../dirN/file.ext this function returns .ext
 *
 * Similar to [t]csh :e
 *
 * @return extension portion of filename
 */
string
FileExtension (
    const string & fileName) ///< file name to analyze
{
    string tail = FileTail (fileName);

    string::size_type separator = tail.rfind ('.');
    if (separator == string::npos) {
        // no separator - return nothing
        return "";
    } else {
        return tail.substr (separator);
    }
}

/**
 * Get the root portion of filename. E.g. if file name is
 * /dir1/dir2/.../dirN/file.ext this function returns
 * /dir1/dir2/.../dirN/file
 *
 * Similar to [t]csh :r
 *
 * @return root portion of filename
 */
string
FileRoot (
    const string & fileName) ///< file name to analyze
{
    string extension = FileExtension (fileName);

    return fileName.substr (0, fileName.length() - extension.length());
}

/**
 * Get the directory name portion of filename. E.g. if file name is
 * /dir1/dir2/.../dirN/file.ext this function returns
 * /dir1/dir2/.../dirN
 *
 * @note: FileDirName() is a wrapper around libc dirname().
 *
 * @note: This is slightly different from FileHead() in the corner case
 * when the file name (path) is only a single directory deep. In this
 * case, FileDirName() can return "/" if the file name is absolute or
 * "." or ".." if the file name is relative, wheras FileHead() will return
 * the file name instead. @see man 3 dirname and man tcsh (:h).
 *
 * @return dirname portion of filename
 */
string
FileDirName (
    const string & fileName) ///< file name to analyze
{
    // we need to make a temp copy since dirname will modify its argument!
    char * temp = strdup(fileName.c_str());
    string dirName = dirname (temp);
    free (temp);

    return dirName;
}

/**
 * Get the base name portion of filename. E.g. if file name is
 * /dir1/dir2/.../dirN/file.ext this function returns
 * file.ext
 *
 * @note: FileBaseName() is a wrapper around libc basename().
 *
 * @note: This is slightly different from FileTail() in the corner case
 * when the file name (path) is only a single directory deep. In this
 * case, FileBaseName() can return "/" if the file name is absolute or
 * "." or ".." if the file name is relative, wheras FileTail() will return
 * the file name instead. @see man 3 basename and man tcsh (:t).
 *
 * @return basename portion of filename
 */
string
FileBaseName (
    const string & fileName) ///< file name to analyze
{
    // we need to make a temp copy since basename will modify its argument!
    char * temp = strdup(fileName.c_str());
    string baseName = basename (temp);
    free (temp);

    return baseName;
}

/**
 * Given 2 absolute paths path1 and path2, what is the relative path from
 * path1 to path2. In order for this to be meaningful, path1 must name a
 * directory, NOT a file. Path2 can be either a directory or a file.
 *
 * @return relative path from path1 to path2
 */
string
FileRelativePath (
    const string & path1, ///< path to start at
    const string & path2) ///< path to end up at
{
    // we can only do the computation for two absolute paths
    if (path1.empty() || path1[0] != '/') {
        return path2;
    }
    if (path2.empty() || path2[0] != '/') {
        return path2;
    }

    SplitString splitPath1(path1, "/");
    SplitString splitPath2(path2, "/");
    typedef list<string> StringList;
    StringList path1List;
    StringList path2List;
    string result;

    FOREACH (SplitString, it, splitPath1) {
        if ( ! (*it).empty()) {
            path1List.push_back (*it);
        }
    }
    FOREACH (SplitString, it, splitPath2) {
        if ( ! (*it).empty()) {
            path2List.push_back (*it);
        }
    }

    // stip off common prefix
    while ( ! path1List.empty() && ! path2List.empty() &&
        path1List.front() == path2List.front())
    {
        path1List.pop_front();
        path2List.pop_front();
    }

    // cd .. back to common ancestor directory
    for (int i = path1List.size(); i > 0; i--) {
        result = FileJoin (result, "..");
    }

    // cd into destination directory from comman ancestor
    FOREACH_CONST (StringList, it, path2List) {
        result = FileJoin (result, *it);
    }

    return result;
}


/**
 * Copy a file from source to destination. If destination is a directory,
 * create a file with the same name as the source name in that directory.
 */
void
FileCopy (
    const string & source, ///< source file name
    const string & dest)   ///< destination file or directory name
{
    string destName;
    string line;

    // treat dest directories special
    if (FileIsDirectory(dest)) {
        destName = FileJoin (dest, FileTail (source));
    } else {
        destName = dest;
    }

    // open input file
    ifstream in(source.c_str());
    if ( ! in) {
        cerr << "Error: FileCopy can't open input file " << source << endl;
        exit(1);
    }

    // open output file
    ofstream out(destName.c_str());
    if ( ! out) {
        cerr << "Error: FileCopy can't open output file " << destName << endl;
        exit(1);
    }

    // copy
    while ( ! in.eof()) {
        getline (in, line);
        if (line.empty() && in.eof()) {
            break; // also eof
        }
        out << line << endl;
    }

    // cleanup
    in.close();
    out.close();
}

/**
 * Wrapper around libc chmod() funtionality.
 *
 * @return true on success, false otherwise
 */
bool
FileChmod (
    const string & fileName, ///< file name
    mode_t mode)             ///< new mode
{
    if (chmod (fileName.c_str(), mode)) {
        return false;
    } else {
        return true;
    }
}

/**
 * Make a new directory (and parent directories if necessary)
 * It is OK if the directory already exists.
 */
void
MakeDir (
    const string & dir, ///< directory name to create
    int mode)           ///< permissions of created directory
{
    SplitString splitDir(dir, "/");
    string path;
    if (IsAbsolutePath (dir)) {
        path = "/";
    }
    FOREACH (SplitString, it, splitDir) {
        path = FileJoin (path, *it);
        if (! FileExists(path)) {
            mkdir (path.c_str(), mode);
        } else {
            if (! FileIsDirectory (path)) {
                cerr << "MakeDir: trying to create directory " << dir << endl
                     << "  but found regular file at " << path << endl;
                exit (1);
            }
        }
    }
}


/**
 * Remove everything in a directory. If nukeSelf is true, the directory
 * itself is also removed.
 */
bool ///< true on success, false otherwise
RemoveDir (
    const string & dir,  ///< directory name to nuke
    bool nukeSelf)       ///< remove the directory itself too
{
    glob_t globbuf;  // interface to libc glob
    string pattern;
    typedef vector<string> StringList;
    StringList globs;

    if ( ! FileIsDirectory (dir)) {
        cerr << "Error: can't rmdir " << dir << " - not a directory" << endl;
        return false;
    }

    //
    // get all file names
    //
    // "*" glob for most files
    pattern = FileJoin (dir, "*");
    glob(pattern.c_str(), GLOB_BRACE, NULL, &globbuf);

    for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
        globs.push_back(globbuf.gl_pathv[i]);
    }
    globfree(&globbuf);
    // ".??*" glob of dot files (other than . and ..)
    pattern = FileJoin (dir, ".??*");
    glob(pattern.c_str(), GLOB_BRACE, NULL, &globbuf);

    for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
        globs.push_back(globbuf.gl_pathv[i]);
    }
    globfree(&globbuf);

    //
    // now we're ready to delete them all
    //
    FOREACH_CONST (StringList, it, globs) {
        if (FileIsDirectory (*it) && ! FileIsSymLink(*it)) {
            if ( ! RemoveDir (*it)) {
                return false;
            }
        } else {
            if (unlink ((*it).c_str())) {
                cerr << "Error: could not remove file " << *it << endl;
                return false;
            }
        }
    }

    if ( nukeSelf ) {
        if (rmdir (dir.c_str())) {
            cerr << "Error: could not remove directory " << dir << endl;
            return false;
        }
    }

    return true;
}


/**
 * Get the current working directory.
 *
 * @return current working directory
 */
string
GetCWD (void)
{
    string result;

    char* cwd = getcwd(NULL, 0);
    if (cwd) {
        result = cwd;
        free (cwd);
    } else {
        result = "";
    }

    return result;
}


//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS
void TestSplitString (char * str, char * sep)
{
    SplitString sp(str, sep);
    FOREACH (SplitString, it, sp) {
        cout << "substr: " << *it << endl;
    }
}

void TestMatchString (char * str, char * regexp)
{
    MatchString ms(str);
    MatchString::MatchArray matchArray;

    string match = ms.Match(regexp, matchArray);

    if (match.size() > 0) {
        cout << "match" << endl;
        FOREACH_CONST (MatchString::MatchArray, it, matchArray) {
            cout << "match: " << *it << endl;
        }
    } else {
        cout << "no match" << endl;
    }
}

void TestCanonicalFilename (char * str, bool fail)
{
    string canonical = str;
    bool success = CanonicalFilename (canonical, canonical, fail);
    cout << "canonical: " << canonical
         << (success ? " (OK)" : " (FAIL)") << endl;
}

void TestFile (char * str)
{
    cout << "File: " << str << " "
         << (FileExists(str) ? " EXIST" : "!exist") << ", "
         << (FileIsFile(str) ? " FILE" : "!file") << ", "
         << (FileIsDirectory(str) ? " DIR" : "!dir") << endl;
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
        if (string(argv[1]) == string("--splitstring")) {
            if (argc == 4) {
                TestSplitString (argv[2], argv[3]);
            } else if (argc == 3) {
                TestSplitString (argv[2], "/");
            } else {
                ParseError (argv);
            }
        } else if (string(argv[1]) == string("--matchstring")) {
            if (argc == 4) {
                TestMatchString (argv[2], argv[3]);
            } else {
                ParseError (argv);
            }
        } else if (string(argv[1]) == string("--filerelativepath")) {
            if (argc == 4) {
                cout << FileRelativePath (argv[2], argv[3]) << endl;
            } else {
                ParseError (argv);
            }
        } else if (string(argv[1]) == string("--file")) {
            if (argc == 3) {
                TestFile (argv[2]);
            } else {
                ParseError (argv);
            }
        } else if (string(argv[1]) == string("--canonicalfilename")) {
            if (argc == 4) {
                bool fail = true;
                if (string(argv[3]) == "0") {
                    fail = false;
                }
                TestCanonicalFilename (argv[2], fail);
            } else if (argc == 3) {
                TestCanonicalFilename (argv[2], true);
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
