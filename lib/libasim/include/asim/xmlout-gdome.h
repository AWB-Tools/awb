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
 * @brief Implementation of XML Output Object
 * @see @ref XMLOut_Documentation "XML Output Documentation"
 * @ingroup XMLOut
 */

#ifndef _XML_OUT_
#define _XML_OUT_ 1

// generic
#include <string>
#include <iostream>

// XML GMetaDom (C++ wrapper for GDome / libxml2 DOM 2.0 implementation)
#include <GdomeSmartDOM.hh>

namespace DOM = GdomeSmartDOM;
using namespace std;


/**
 * @brief XML Output Object
 *
 * This class provides a simplified interface for XML file creation in
 * a linearized way (ie. the order that the XML contents will
 * eventually be represented in the output file, which is starting
 * at the root element, depth-first). XMLOut does not expose the whole XML
 * DOM (2.0) interface, but rather only very few methods sufficient to
 * incrementally build up an XML tree in depth-first order and be able
 * to write it out to a file. 
 *
 * @see @ref XMLOut_Documentation "XML Output Documentation"
 *
 * @ingroup XMLOut
 */
class XMLOut {
  private:
    // data members
    /// output file name
    string filename;

    /// if true, sync output after every operation.
    bool debug;

    /// XML DOM implementation (DOM document factory and
    /// implementation specific operations).
    DOM::DOMImplementation domImpl;

    /// XML DOM document node (the root of the DOM document hierarchy)
    DOM::Document domDoc;

    /// The current DOM (element) node. All output operations are
    /// associated with the current node and extend it or, after all
    /// operations are completed on a node, move the notion of the
    /// current node back to its parent node (ie. depth-first behavior).
    DOM::Element currentNode;

  public:
    // constructors / destructors / initializers
    /// Create a new XML Output object
    XMLOut (const char* filename, const char* root,
        const char* dtdPublicId = NULL, const char* dtdSystemId = NULL,
        const bool debug = false);

    /// Write out XML Output object to file and destroy object
    ~XMLOut ();

    // accessors
    /// Get current debug mode setting
    bool GetDebug (void) { return debug; }
    /// Get output filename
    string GetFilename (void) { return filename; }

    // modifiers
    /// Set debug mode (defaults to true)
    void SetDebug (bool debug = true) { this->debug = debug; }

    // output methods
    /// Add an element node to the XML tree and make it the current node
    void
    AddElement (const char* name);

    /// Close the current node and make its parent the new current node
    void
    CloseElement(void);

    /// Add an attribute node to the current node
    void
    AddAttribute (const char* name, const char* value);

    /// Add a text node to the current node
    void
    AddText (const char* text);

    /// Add a CDATA node to the current node
    void
    AddCDATA (const char* text);

    /// Add a comment node to the current node
    void
    AddComment (const char* text);

    /// Add a processing instruction
    void
    AddProcessingInstruction (const char* target, const char* data);

    /// Text output binding to operator<< on XMLOut objects
    XMLOut& operator<< (const char* text) { AddText(text); return *this; }

  private:
    // private methods
    /// Dump current XML tree to output file
    void DumpToFile (void);

    /// Dump current XML tree to ostream
    void DumpToStream (ostream& out);
    /// external operator<< needs access to XMLOut private dump method
    friend ostream& operator<< (ostream& out, XMLOut& xml);

    /// Write current XML tree to output file
    void ConditionalDebug (void) { if (debug) { DumpToFile(); } }
};

/**
 * XMLOut output binding to operator<< on ostream. This is mainly
 * provided for debugging XMLOut and should not be used for normal
 * operation.
 */
inline ostream&    ///< @returns ostream object for operation chaining
operator<< (
    ostream& out,  ///< output stream to write to
    XMLOut& xml)   ///< XMLOut object to write
{
     xml.DumpToStream(out);

     return out;
}

#endif // _XML_OUT_
