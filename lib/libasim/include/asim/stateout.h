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
 * @brief Implementation of stats output
 *
 * This file implements an output class for stats file that allows us
 * to tightly control the allowed formatting of stats files. This
 * enables automatic post-processing of those stats files without
 * breaking parsing tools by adding new stats, as is frequently seen
 * if the output format is just a random sequence of text strings. We
 * use XML formatting of the output and here is where we make sure
 * that the output conforms to the DTD we have set up for stats files.
 */

#ifndef _STATE_OUT_
#define _STATE_OUT_

// generic
#include <iostream>
#include <sstream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/xmlout.h"


// forward declaration
typedef class STATE_OUT_CLASS *STATE_OUT;

/**
 * @brief Output class for (well formed, ie. parsable) stats files
 *
 * This class implements the interface for output operations of state
 * objects. It is meant to be the <b>only</b> way to print state into
 * a stats file in order to guarantee meeting strict formatting
 * conventions on stats files to ease automatic post-processing with a
 * variety of tools.
 *
 * State output is in a tree structured XML document and follows a
 * simple depth-first build order.
 *
 * @par Stats Output Structure
 * The stats output is an XML document with a very simple structure.
 * The main elements are <scalar>, <vector>, and <compound> elemtents and
 * the deprecated <text> elemtent. The former three element types have
 * <type>, <name>, and <desc> elements associated with them, whereas
 * the latter only contains plain text. <scalar> and <vector> (and
 * possibly <text>) represent actual state contents, whereas
 * <compound> is a grouping construct for building hierarchies, e.g.
 * modules and sub-modules.  Please refer to the
 * <a href="http://asim.intel.com/home/asim/xml/dtd/asim-stats-0.2.dtd">
 * XML DTD for asim-stats</a> for the ultimate definition of the
 * output format.
 *
 * @par <scalar>
 * Description: One scalar value, e.g. an integer.<br>
 * Contents:
 * <dl>
 * <dt> <type>
 *   <dd>The type of this state, e.g. uint, double.</dd>
 * <dt> <name>
 *   <dd>The name of this state.</dd>
 * <dt> <desc> ?
 *   <dd>[optional] A description for this state.</dd>
 * <dt> text
 *   <dd>The scalar value of the state.</dd>
 * </dl>
 *
 * @par <vector>
 * Description: One vector, e.g. an array of integers.<br>
 * Contents:
 * <dl>
 * <dt> <type>
 *   <dd>The type of this state, e.g. uint, double.</dd>
 * <dt> <name>
 *   <dd>The name of this state.</dd>
 * <dt> <desc> ?
 *   <dd>[optional] A description for this state.</dd>
 * <dt> <value> *
 *   <dd>Each value within the vector is represented as the text
 *       within a <value> element. There are as many <value> elements
 *       as there are elements in the vector.
 *   </dd>    
 * </dl>
 *
 * @par <compound>
 * Description: A compound is a container for arbitrary collections.
 * It can contain any number of the other elements <scalar>, <vector>,
 * <compound> and <text> in any order. It is similar to a C
 * struct.<br>
 * Contents:
 * <dl>
 * <dt> <type>
 *   <dd>The type of this compound, e.g. module, histogram.</dd>
 * <dt> <name>
 *   <dd>The name of this compound.</dd>
 * <dt> <desc> ?
 *   <dd> [optional] A description for this compound.</dd>
 * <dt> ( <scalar> | <vector> | <compound> | <text> )*
 *   <dd>The compound can contain an arbitrary collection of
 *       the listed subelements.
 *   </dd>
 * </dl>
 */
class STATE_OUT_CLASS
{
  private:
    /// @name Constants for XML element (tag) names
    /// @{
    static const char * const elementScalar;
    static const char * const elementVector;
    static const char * const elementValue;
    static const char * const elementCompound;
    static const char * const elementText;
    static const char * const elementType;
    static const char * const elementName;
    static const char * const elementDesc;
    /// @}

    // variables
    XMLOut * xmlStats;  ///< the XML output object for the stats

    // methods
    /// Add the common elements type, name, and desc
    void
    AddCommonInfo (const char* type, const char* name, const char* desc);

  public:
    // constructors / destructors / initializers
    /// Create a new STATE_OUT object
    STATE_OUT_CLASS (const char* filename);

    /// Sync output to disk and destroy object
    ~STATE_OUT_CLASS ();

    // accessors

    // modifiers

    // output methods
    /// Add compound element to the output and make it the current element
    void
    AddCompound (const char* type, const char* name, const char* desc = NULL);
    
    /// Close the current compound and make its parent current
    void
    CloseCompound (void);

    /// Add a scalar state of type <Type> to the output
    template <typename Type>
    void
    AddScalar (const char* type, const char* name, const char* desc,
               const Type& value);

    /// Template specialization for adding scalar variable of string type
    // Note: bizar, member template specialization does not seem to
    // work here, but declaring a non-template member of the right
    // type seems to do ...
    void
    AddScalar (const char* type, const char* name, const char* desc,
               const char* value);

    /// Add a vector state to the output
    template <class InputIterator>
    void
    AddVector (const char* type, const char* name, const char* desc,
               InputIterator first, InputIterator last);

    /// Add arbitrary text to the output - use is <b>deprecated!</b>
    void
    AddText (const char* text);
};


/**
 * Add a scalar element to the output. This method takes care of
 * implementing the <scalar> syntax of the XML asim-stats DTD.
 * Adding a scalar of any type to the output works by converting the
 * scalar of type <Type> to a string and then adding the string as a
 * scalar value.
 * @note Requirements:
 * <ul>
 * <li> Type needs an operator<< (ostream, Type).
 * </ul>
 */
template <typename Type>
void
STATE_OUT_CLASS::AddScalar (
    const char* type,   ///< type of the scalar element
    const char* name,   ///< name of the scalar element
    const char* desc,   ///< description of the scalar element
    const Type& value)  ///< value to be printed
{
  ostringstream os;

  // convert value to a string and pass on
  os << value;
  AddScalar (type, name, desc, os.str().c_str());
}


/**
 * Add a vector element to the output. This method takes care of
 * implementing the <vector> syntax of the XML asim-stats DTD.
 *
 * The elements of the vector are passed in via a pair of input
 * iterators, which must be defined for the container of the values.
 *
 * @note Requirements:
 * <ul>
 * <li> Iterators need operator++(int), operator!=(), operator*().
 * <li> Type needs an operator<< (ostream, Type).
 * </ul>
 */
template <class InputIterator>
void
STATE_OUT_CLASS::AddVector (
    const char* type,    ///< type of the scalar element
    const char* name,    ///< name of the scalar element
    const char* desc,    ///< description of the scalar element
    InputIterator first, ///< iterator for first element
    InputIterator last)  ///< iterator past last element
{
    xmlStats->AddElement(elementVector);
    AddCommonInfo(type, name, desc);

    // add all values
    for ( ; first != last; first++) {
        xmlStats->AddElement(elementValue);
        ostringstream os;
        os << *first;
        xmlStats->AddText(os.str().c_str());
        xmlStats->CloseElement();
    }
    xmlStats->CloseElement();
}


#endif /* _STATE_OUT_ */
