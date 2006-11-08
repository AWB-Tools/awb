/**************************************************************************
 * INTEL CONFIDENTIAL Copyright (c) 2002 Intel Corp. 
 *
 * Recipient is granted a non-sublicensable copyright license under
 * Intel copyrights to copy and distribute this code internally only.
 * This code is provided "AS IS" with no support and with no
 * warranties of any kind, including warranties of MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY
 * INFRINGEMENT. By making any use of this code, Recipient agrees that
 * no other licenses to any Intel patents, trade secrets, copyrights
 * or other intellectual property rights are granted herein, and no
 * other licenses shall arise by estoppel, implication or by operation
 * of law. Recipient accepts all risks of use.
 **************************************************************************/

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
#include <fstream>
#include <vector>

using namespace std;


/**
 * @brief XML Output Object
 *
 * This class provides a simplified interface for XML file creation in
 * a linearized way (ie. the order that the XML contents will
 * eventually be represented in the output file, which is starting
 * at the root element, depth-first). This XMLOut uses a built-in
 * simlictic XML writer and does not depend on external XML libraries.
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

    /**
     * @brief Simple XML writer
     *
     * This class is a butcher job of replacing the XML DOM library stack
     * to write out XML files front-to-back. It is self contained and
     * does not depend on any XML libraries. On the downside, it does
     * only minimalistic error checking, can easily generate XML
     * output that is not-well-formed (ie. violates the most
     * fundamental constraints of what defines an XML document) by
     * calling functions in the wrong order, and in general is only
     * supposed to be a XML writer replacement for this very simple
     * front-to-back writing job that XMLOut requires.
     *
     * The class always writes immediately and directly into the
     * output file. Only in certain cases the writing of a few
     * trailing characters might be delayed (e.g. final '>' of opening
     * tags) if it is not yet clear if newer output has to be written
     * out before that (e.g. attributes).
     */
    class Writer {
      private:
        FILE* out;              ///< output file
        string rootTag;            ///< delayed root tag name
        string delayedWrite;       ///< string pending write
        vector<string> activeTags; ///< opened and not yet closed tag stack

        /// Check if root tag writing was delayed and has to be done now
        void DelayedRootTag (void);
        /// Write output whose writing had been delayed
        void DelayedWrite (void);

      public:
        // constructors / destructors / initializers
        Writer (const char* filename);
        ~Writer ();

        /// Create the XML file header
        void CreateHeader (const char* root, 
            const char* dtdPublicId, const char* dtdSystemId,
            const char* creator);
        /// Write out all pending end tags
        void Finish (void);
        /// Create a new element (write its opening tag)
        void AddElement (const char* name);
        /// Close the topmost element (write its end tag)
        void CloseElement (void);
        /// Add an attribute to the top-most element
        void AddAttribute (const char* name, const char* value);
        /// Add text to the top-most element
        void AddText (const char* text);
        /// Add a CDATA section to the top-most element
        void AddCDATA (const char* text);
        /// Add a comment
        void AddComment (const char* text);
        /// Add a processing instruction
        void AddProcessingInstruction (const char* target, const char* data);
    };

    /// the writer for this XMLOut object
    Writer writer;

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
    /// external operator<< needs access to XMLOut private dump method
    //friend ostream& operator<< (ostream& out, XMLOut& xml);
};

/**
 * XMLOut output binding to operator<< on ostream. This is mainly
 * provided for debugging XMLOut and should not be used for normal
 * operation.
 */
//inline ostream&    ///< @returns ostream object for operation chaining
//operator<< (
//    ostream& out,  ///< output stream to write to
//    XMLOut& xml)   ///< XMLOut object to write
//{
//    ASIMERROR ("XMLOut::DumpToStream: unimplemented method!" << endl);
//
//    return out;
//}

#endif // _XML_OUT_
