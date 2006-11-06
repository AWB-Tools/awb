/*
 *Copyright (C) 2004-2006 Intel Corporation
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

/////////////////////////////////////////////////////////////////////////////////
//
// $Log$
// Revision 1.2  2005/05/20 03:35:30  mcadler
// CSN-core-1256
//
// Revision 1.1  2004/11/03 18:32:12  snpeffer
// CSN-core-1063
//
//
// regular expression class header
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef _MERLIN_REGEX_H
#define _MERLIN_REGEX_H

#include <string>
#include <regex.h>

using namespace std;

class Regex 
{
public:
    Regex();

    //  initialize with a regular expression pattern
    Regex(const char *, bool caseSensitive = true);

    // create a compiled regex pattern - return status
    int create(const char *, bool caseSensitive = true);
    
    // match the pattern with the string; true on success, false otherwise 
    bool match(const char *);
    bool match(string &);

    // match the pattern with the string, storing substrings; true on success, false otherwise 
    bool matchSub(const char *);
    bool matchSub(string &);

    // Get any error strings for the last match attempt.
    string getMatchError();

    // Check an index to see if it's a valid substring from the last match.
    bool isValidIndex(int index);

    // Get a substring from the last match call.
    string getSubstring(int index);

    // assign the regular expression; returns true on successful syntax 
    // check, false otherwise.
    bool assign(const char *, bool caseSensitive = true);

    // print error 
    void error(int errorCode);

    // clean up
    void dispose();

    // return whether the regex is ready for matching 
    bool ok();

    ~Regex() {
	dispose();
    }

protected:
    bool initialized;
    regex_t regexPattern;

    // For pattern substitution, keep an array for reuse.
    unsigned subArraySize;
    regmatch_t *subArray;
    // record the number of subexpressions matched
    unsigned lastNumSubsMatched;
    // record the last string matched against for extracting substrings
    // and producing error reports
    string lastMatch;
    // the last status code from a match call
    int lastStatus;

protected:
    bool _match(const char *);
    bool _matchSub(const char *);
};


// Provide the capability of substituting sub strings from a regular
// expression match (see Regex above) into another string using markers
// to indicate where to insert the substrings. \0 matches the whole 
// string, \1 matches the first substring, etc.
class RegexSubstitution {
    // The string to substitute substrings from Regex into.
    string substring;
    // used by substring substitution to walk through the char array
    const char *subPtr;
    // used by substring substitution to store the substituted string
    string substitutedString;
    
public:
    RegexSubstitution() {}

    RegexSubstitution(const char *str) {
        substring = str;
    }

    // Change the subsitution string.
    bool assign(const char *str) {
        substring = str;
        return(true);
    }

    // Does the string have substitution parameters (they look like \1, \2, ...)
    bool hasSubstitution();

    // Substitute the substrings from re into the substitution string.
    // Returns true of there were no errors.
    bool substitute(Regex &re);

    // Get the result of the substitution.
    string &getSubstitutedString() {
        return(substitutedString);
    }

private:
    // Extract a substitution index. str should point to the backslash
    // of a valid substitution marker. 
    int getSubstitution(const char *str, char **endptr);
};

#define REGEX_ERRORBUF_SIZE 400    

#endif

/////////////////////////////////////////////////////////////////////////////////
