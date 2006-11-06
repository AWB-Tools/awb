/*
 * ****************************************************************
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
 * @brief Implementation of I/O Formatting Objects
 * @see @ref IoFormat_Documentation "I/O Format Documentation"
 * @ingroup IoFormat
 */

#ifndef _IOFORMAT_
#define _IOFORMAT_ 1

// generic
#include <iostream>
#include <sstream>
#include <iosfwd>
#include <iomanip>
#include <string>

/**
 * @brief Namespace for I/O Formatting Objects
 *
 * @see @ref IoFormat_Documentation "I/O Format Documentation"
 *
 * @ingroup IoFormat
 */
namespace IoFormat {
  /// global format for "%x" printing
  extern const class Format fmt_x;
  /// global format for "%p" printing
  extern const class Format fmt_p;
  /// global format for printing bool as true/false
  extern const class Format fmt_b;

  // import a few names from namespace std we use repeatedly
  using std::ios_base;
  using std::ostream;
  using std::string;

  template <typename Type>
    class BoundFormat;  // Format plus value of type Type

  typedef ios_base::fmtflags fmtflags;

  /**
   * @brief Unbound Format - the fundamental formatting object
   *
   * This class keeps all formating state for Format objects similar
   * to ostream objects and defines the standard ostream modifiers on
   * this state.
   * @see @ref IoFormat_Documentation "I/O Format Documentation"
   *
   * @ingroup IoFormat
   */
  class Format {
    // data members
    const char* f_prefix;  ///< prefix string
    const char* f_postfix; ///< postfix string
    fmtflags f_flags;      ///< (std::ios_base) formating flags
    int f_width;           ///< (std::ios_base) minimum field width
    int f_precision;       ///< (std::ios_base) floating point precision
    char f_fill;           ///< (std::basic_ios) fill character

  public:
    /**
     * @brief Error class for exceptions
     *
     * This class implements the object that is thrown when an
     * unresolvable error occurs in the IoFormat package.
     *
     */
     class Error {
     public:
       /// Format string @fmtstr could not be parsed
       class Fmtstr {
       public:
         string msg;
         string fmtstr;

         Fmtstr(string m, string f = NULL) : msg(m), fmtstr(f) {};
       };
     };

    /**
     * @name Constructors
     * @par Syntax:
     * Format(["fmtstr"]);\n
     * Format("prefix","fmtstr"[,"postfix"]);
     * @{
     */
    /// default constructor
    Format() { init(); }
    /**
     * @brief Printf-like format string constructor
     * @par Syntax:
     * Format("fmtstr");
     */
    Format (
      const char* const fmtstr) ///< printf-like format string
    {
      format(fmtstr);
    }
    /**
     * @brief Printf-like format string constructor w/ prefix/postfix
     * @par Syntax:
     * Format("prefix","fmtstr"[,"postfix"]);
     */
    Format (
      const char* const pre,         ///< prefix string
      /// printf-like format string
      const char* const fmtstr,
      const char* const post = NULL) ///< optional postfix string
    {
      format(fmtstr);
      if (pre) {
        prefix(pre);
      }
      if (post) {
        postfix(post);
      }
    }
    ///@}

    /// Initialize Format object to default state
    inline void init(void)
    {
      // reset all formating options to their defaults
      f_prefix = f_postfix = NULL; // no pre/post-fix strings
      f_flags = fmtflags(0);       // default clears all flags
      f_width = 0;      // no minimum field width
      f_precision = 6;  // standard says default precision is 6
      f_fill = ' ';     // fill with spaces
    }

    /**
     * @name Format Binders
     * Bind Format & value to create BoundFormat object
     * @par Syntax:
     * Format f1; f1(["prefix",]value[,"postfix"]);
     * @{
     */
    /**
     * @brief Make BoundFormat from *this format, value and optional postfix
     * @par Syntax:
     * Format f1; f1(value,["postfix"]);
     */
    template <typename Type>
      BoundFormat<Type>
      operator() (Type val, const char* const post = NULL) const; 

    /**
     * @brief Make BoundFormat from prefix, *this format, value, and optional postfix
     * @par Syntax:
     * Format f1; f1("prefix",value,["postfix"]);
     */
    template <typename Type>
      BoundFormat<Type>
      operator() (
        const char* const pre, Type val, const char* const post = NULL) const; 
    ///@}

    /**
     * @name Primary Formating State Modifiers
     * Low-level methods to change the state of a Format object.
     * All operations return the Format object for operation chaining.
     * @{
     */
    /// Set minimum field width
    inline Format&
    width (int w) { f_width = w; return *this; }
    /// Set floating point precision
    inline Format&
    precision (int p) { f_precision = p; return *this; }
    /// Set fill character
    inline Format&
    fill (char f) { f_fill = f; return *this; }
    /// Set (|) formatting flags
    inline Format&
    setf (fmtflags f) { f_flags |= f; return *this; }
    /// Change formatting flags under mask
    inline Format&
    setf (fmtflags flags, fmtflags mask)
    {
      f_flags = (f_flags & ~mask) | (flags & mask);
      return *this;
    }
    /// Clear formatting flags under mask
    inline Format&
    unsetf (fmtflags mask) { f_flags = f_flags & ~mask; return *this; }
    /// Set prefix string
    inline Format&
    prefix (const char* const p) { f_prefix = p; return *this; }
    /// Set postfix string
    inline Format&
    postfix (const char* const p) { f_postfix = p; return *this; }
    /// Parse printf-like format string and set formatting state from that
    Format& format (const char* const fmtstr);
    ///@}

    /**
     * @name Simplified General Modifiers
     * A set of fmtflags modifiers with names equivalent to
     * standard manipulator names that change basic flags.
     * Note that this is an extension to the regular ostream
     * functionality, i.e. these modifiers are not available on
     * ostreams.
     * @{
     */
    // 27.4.5.1 fmtflags manipulator equivalents
    /// Symbolic representation (false/true) for bool values
    inline Format& 
    boolalpha(void) { return setf(ios_base::boolalpha); }
    /// Numeric representation (0/1) for bool values
    inline Format& 
    noboolalpha(void) { return unsetf(ios_base::boolalpha); }
    /// Prefix octal by 0, hex by 0x or 0X
    inline Format& 
    showbase(void) { return setf(ios_base::showbase); }
    /// Don't prefix integers by number base indicator
    inline Format& 
    noshowbase(void) { return unsetf(ios_base::showbase); }
    /// Always show dot and trailing zeros for floating point numbers
    inline Format& 
    showpoint(void) { return setf(ios_base::showpoint); }
    /// Show dot and trailing zeros for floating point numbers
    /// only when needed (ie. if fraction != 0)
    inline Format& 
    noshowpoint(void) { return unsetf(ios_base::showpoint); }
    /// Explicit + sign for positive integers
    inline Format& 
    showpos(void) { return setf(ios_base::showpos); }
    /// Don't show + sign for positive integers
    inline Format& 
    noshowpos(void) { return unsetf(ios_base::showpos); }
    /// Skip white space in input
    /// @note This is unused for output formats
    inline Format& 
    skipws(void) { return setf(ios_base::skipws); }
    /// Don't skip white space in input
    /// @note This is unused for output formats
    inline Format& 
    noskipws(void) { return unsetf(ios_base::skipws); }
    /// Show capital X for hex and E for power of 10 indicators
    inline Format& 
    uppercase(void) { return setf(ios_base::uppercase); }
    /// Show lowercase x for hex and e for power of 10 indicators
    inline Format& 
    nouppercase(void) { return unsetf(ios_base::uppercase); }
    /// Flush output after each output operation
    /// @note This is unused
    inline Format& 
    unitbuf(void) { return setf(ios_base::unitbuf);      }
    /// Don't flush output after operation
    /// @note This is unused
    inline Format& 
    nounitbuf(void) { return unsetf(ios_base::unitbuf); }
    ///@}

    /**
     * @name Simplified Adjustfield Modifiers
     * A set of fmtflags modifiers with names equivalent to
     * standard manipulator names that change adjustfield flags.
     * Note that this is an extension to the regular ostream
     * functionality, i.e. these modifiers are not available on
     * ostreams.
     * @{
     */
    // 27.4.5.2 adjustfield manipulator equivalents
    /// Fill character inserted between sign and value
    inline Format& 
    internal(void) {
      return setf(ios_base::internal, ios_base::adjustfield);
    }
    /// Flush left (fill character inserted right)
    inline Format& 
    left(void)
    {
      return setf(ios_base::left, ios_base::adjustfield);
    }
    /// Flush right (fill character inserted left)
    inline Format& 
    right(void)
    {
      return setf(ios_base::right, ios_base::adjustfield);
    }
    ///@}

    /**
     * @name Simplified Basefield Modifiers
     * A set of fmtflags modifiers with names equivalent to
     * standard manipulator names that change basefield flags.
     * Note that this is an extension to the regular ostream
     * functionality, i.e. these modifiers are not available on
     * ostreams.
     * @{
     */
    // 27.4.5.3 basefield manipulator equivalents
    /// Decimal integer representation
    inline Format& 
    dec(void)
    {
      return setf(ios_base::dec, ios_base::basefield);
    }
    /// Hexadecimal integer representation
    inline Format& 
    hex(void)
    {
      return setf(ios_base::hex, ios_base::basefield);
    }
    /// Octal integer representation
    inline Format& 
    oct(void)
    {
      return setf(ios_base::oct, ios_base::basefield);
    }
    ///@}

    /**
     * @name Simplified Floatfield Modifiers
     * A set of fmtflags modifiers with names equivalent to
     * standard manipulator names that change floatfield flags.
     * Note that this is an extension to the regular ostream
     * functionality, i.e. these modifiers are not available on
     * ostreams.
     * @{
     */
    // 27.4.5.4 floatfield manipulator equivalents
    /// Fixed floating point representation
    inline Format& 
    fixed(void)
    {
      return setf(ios_base::fixed, ios_base::floatfield);
    }
    /// Scientific floating point representation
    inline Format& 
    scientific(void)
    {
      return setf(ios_base::scientific, ios_base::floatfield);
    }
    ///@}

    /**
     * @name Accessors
     * Accessors to state of Format object.
     * @{
     */
    /// Get prefix string
    const char* const prefix (void) const { return f_prefix; }
    /// Get postfix string
    const char* const postfix (void) const { return f_postfix; }
    ///@}

    /// Copy local formating state to an ostream object
    void copy (ostream& os) const
    {
      os.flags(f_flags);         // set flags
      os.width(f_width);         // set width
      os.precision(f_precision); // set precision
      os.fill(f_fill);           // set fill character
    }
  };

  /**
   * @brief Bound Format & value
   *
   * This class is used to bind a Format to a specific value. Its
   * purpose is as a transitional object that can be used by the
   * operator<< on an ostream to print a value with a predefined
   * format, i.e.
   * @code
   *   Form f1;
   *   cout << f1(1234);
   * @endcode
   * will print 1234 with the format set up in f1 (w/o permanently
   * changing the formatting options on cout).
   * This class is the one that binds f1 to the value 1234 in
   * order to pass both together with cout to operator<<
   * The class is templatized to allow binding to <b>any</b> type
   * of argument.
   *
   * The class also allows binding another prefix and postfix string
   * to the Format and value. This is mostly syntactic sugar for
   * notational convenience rather than necessity. In case both the
   * Format and the BoundFormat end up with a prefix or postfix
   * string, the BoundFormat pre/postfix string is used instead of the
   * Format pre/postfix string.
   *
   * @see @ref IoFormat_Documentation "I/O Format Documentation"
   *
   * @ingroup IoFormat
   */

  // Using gcc 3.4 we have to forward declare the following:
  template<typename Type> class BoundFormat;
  template<typename Type> ostream& operator<<(ostream& out, const BoundFormat<Type>& bf);

  template <typename Type>
    class BoundFormat {
      const Format& form;  ///< Format object to use
      Type value;          ///< value to be printed
      const char* prefix;  ///< prefix string to be printed before value
      const char* postfix; ///< postfix string to be printed after value
      /// Deallocation indicator for temporary Format
      /**
       * This flag is set if the method that creates the BoundFormat
       * also has to create a temporary Format. In order to keep
       * the temporary Format available for the lifetime of the
       * BoundFormat, the Format needs to be created on the heap.
       * The BoundFormat will delete such temporary Format objects
       * from the heap when the BoundFormat object is destructed.
       */
      bool dealloc;

    friend ostream&
    operator<< <Type> (ostream& out, const BoundFormat<Type>& bf);

    public:
      // constructors
      /// Make BoundFormat from Format object, value, and optional
      /// pre/postfix strings
      BoundFormat (
        const Format& fmt,             ///< Format object
        Type val,                      ///< value to be printed
        const char* const pre = NULL,  ///< optional prefix string
        const char* const post = NULL, ///< optional postfix string
        /// optional memory management indicator for Format
        bool dea = false)
      : form(fmt),
        value(val),
        prefix(pre),
        postfix(post),
        dealloc(dea)
      {}

      // destructors
      /// Delete temporary Format object if needed
      ~BoundFormat ()
      {
        // if this BoundFormat manages a temporary Format object,
        // we need to delete the Format when the BoundFormat is deleted;
        if (dealloc) {
          delete &form;
        }
      }
    };

  /**
   * @par Syntax:
   * Format f1; f1(value,["postfix"]);
   */
  template <typename Type>
    inline BoundFormat<Type>
    Format::operator() (
      Type val,               ///< value to be bound
      const char* const post) ///< optional postfix string to be bound
    const
    {
      return BoundFormat<Type> (*this, val, /*pre*/NULL, post);
    }

  /**
   * @par Syntax:
   * Format f1; f1("prefix",value,["postfix"]);
   */
  template <typename Type>
    inline BoundFormat<Type>
    Format::operator() (
      const char* const pre,  ///< prefix string to be bound
      Type val,               ///< value to be bound
      const char* const post) ///< optional postfix string to be bound
    const
    {
      return BoundFormat<Type> (*this, val, pre, post);
    }

  /**
   * @brief BoundFormat binding to operator<< on ostream
   * @par Syntax:
   * Format f1;\n
   * cout << f1(value);
   */
  template <typename Type>
    ostream&
    operator<< (
      ostream& out,                ///< ostream where this output goes
      const BoundFormat<Type>& bf) ///< BoundForm that is printed out
    {
      //
      // print prefix if available
      if (bf.prefix) {
        // BoundForm prefix has highest priority
        out << bf.prefix;
      } else if (bf.form.prefix()) {
        // unbound Form prefix has lower priority
        out << bf.form.prefix();
      }
      //
      // format and print value
      std::ostringstream os;
      bf.form.copy(os);  // set up output format
      os << bf.value;    // print formated value into string
      out << os.str();   // output string on stream
      //
      // print postfix if available
      if (bf.postfix) {
        // BoundForm postfix has highest priority
        out << bf.postfix;
      } else if (bf.form.postfix()) {
        // unbound Form postfix has lower priority
        out << bf.form.postfix();
      }
      return out;  // for << chaining return stream
    }

  /**
   * @name Implicit Format Use Functions
   * Functions adding support for a variety of formatting
   * options expressible inside an operator<< use of a format.
   * @par Syntax:
   *   cout << fmt(["prefix",]{"fmtstr"|<Format>},value[,"postfix"]);
   * @{
   */
  /**
   * @brief Make BoundFormat from @ref fmtstr, value, and optional
   * postfix string
   * @par Syntax:
   * cout << fmt("fmtstr",value[,"postfix"]);
   */
  template <typename Type>
    inline BoundFormat<Type>
    fmt (
      const char* const fmtstr,      ///< printf-like @ref fmtstr
      Type val,                      ///< value to print
      const char* const post = NULL) ///< optional postfix string
    {
      Format* formp = new Format(fmtstr);
      return BoundFormat<Type> (*formp, val, /*pre*/NULL, post, true);
    }

  /**
   * @brief Make BoundFormat from prefix string, @ref fmtstr, value,
   * and optional postfix string
   * @par Syntax:
   * cout << fmt("prefix","fmtstr",value[,"postfix"]);
   */
  template <typename Type>
    inline BoundFormat<Type>
    fmt (
      const char* const pre,         ///< prefix string
      const char* const fmtstr,      ///< printf-like @reg fmtstr
      Type val,                      ///< value to print
      const char* const post = NULL) ///< optional postfix string
    {
      Format* formp = new Format(fmtstr);
      return BoundFormat<Type> (*formp, val, pre, post, true);
    }

  /**
   * @brief Make BoundFormat from Format, value, and optional
   * postfix string
   * @par Syntax:
   *   Format f1;\n
   *   cout << fmt(f1,value[,"postfix"]);\n
   * This is a redundant way of expressing\n
   *   cout << f1(value[,"postfix"]);
   */
  template <typename Type>
    inline BoundFormat<Type>
    fmt (
      const Format& form,            ///< Format to use for printing
      Type val,                      ///< value to print
      const char* const post = NULL) ///< optional postfix string
    {
      return BoundFormat<Type> (form, val, /*pre*/NULL, post);
    }

  /**
   * @brief Make BoundFormat from prefix string, Format, value, and
   * optional postfix string
   * @par Syntax:
   *   Format f1;\n
   *   cout << fmt("prefix",f1,value[,"postfix"]);\n
   * This is a redundant way of expressing\n
   *   cout << f1("prefix",value[,"postfix"]);
   */
  template <typename Type>
    inline BoundFormat<Type>
    fmt (
      const char* const pre,         ///< prefix string
      const Format& form,            ///< Format to use for printing
      Type val,                      ///< value to print
      const char* const post = NULL) ///< optional postfix string
    {
      return BoundFormat<Type> (form, val, pre, post);
    }
  ///@}

  //-------------------------------------------------------------------

  /**
   * @name Standard Manipulators
   * Manipulators allow us to change Format object state with the
   * customary operator<< notation.
   * @par Syntax:
   *   Format f1;\n
   *   f1 << left << hex;\n
   * equivalent to\n
   *   Format f1;\n
   *   f1.left().hex();
   * @note
   * Standard manipulators w/o argument also do <b>not</b> have
   * a pair of empty parentheses () after their name, i.e.\n
   *   f1 << left() << hex();\n
   * would be illegal.
   * @{
   */

  /**
   * @brief Standard manipulator binding to operator<< on ostream
   *
   * @par Syntax:
   *   Format f1;\n
   *   f1 << hex << left;
   */
  inline Format&
  operator<< (
    Format& form,           ///< Form that is modified
    Format& (*f) (Format&)) ///< function pointer to a manipulator function
  {
    f(form);
    return form;
  }
  // the real manipulators are really below, but we can't nest
  // doxygen groups;
  ///@}

  /**
   * @name General Fmtflags Manipulators
   * A set of fmtflags manipulators that change basic flags.
   * @{
   * @param form Form that is modified
   * @return input Form for manipulator chaining
   */
  // 27.4.5.1 fmtflags manipulators:
  /// Manipulator for modifier Format::boolalpha()
  inline Format&
  boolalpha(Format& form) { return form.boolalpha(); }
  /// Manipulator for modifier Format::noboolalpha()
  inline Format&
  noboolalpha(Format& form) { return form.noboolalpha(); }
  /// Manipulator for modifier Format::showbase()
  inline Format&
  showbase(Format& form) { return form.showbase(); }
  /// Manipulator for modifier Format::noshowbase()
  inline Format&
  noshowbase(Format& form) { return form.noshowbase(); }
  /// Manipulator for modifier Format::showpoint()
  inline Format&
  showpoint(Format& form) { return form.showpoint(); }
  /// Manipulator for modifier Format::noshowpoint()
  inline Format&
  noshowpoint(Format& form) { return form.noshowpoint(); }
  /// Manipulator for modifier Format::showpos()
  inline Format&
  showpos(Format& form) { return form.showpos(); }
  /// Manipulator for modifier Format::noshowpos()
  inline Format&
  noshowpos(Format& form) { return form.noshowpos(); }
  /// Manipulator for modifier Format::skipws()
  inline Format&
  skipws(Format& form) { return form.skipws(); }
  /// Manipulator for modifier Format::noskipws()
  inline Format&
  noskipws(Format& form) { return form.noskipws(); }
  /// Manipulator for modifier Format::uppercase()
  inline Format&
  uppercase(Format& form) { return form.uppercase(); }
  /// Manipulator for modifier Format::nouppercase()
  inline Format&
  nouppercase(Format& form) { return form.nouppercase(); }
  /// Manipulator for modifier Format::unitbuf()
  inline Format&
  unitbuf(Format& form) { return form.unitbuf(); }
  /// Manipulator for modifier Format::nounitbuf()
  inline Format&
  nounitbuf(Format& form) { return form.nounitbuf(); }
  ///@}

  /**
   * @name Adjustfield Fmtflags Manipulators
   * A set of fmtflags manipulators that change adjustfield flags.
   * @{
   * @param form Form that is modified
   * @return input Form for manipulator chaining
   */
  // 27.4.5.2 adjustfield manipulators:
  /// Manipulator for modifier Format::internal()
  inline Format& 
  internal(Format& form) { return form.internal(); }
  /// Manipulator for modifier Format::left()
  inline Format& 
  left(Format& form) { return form.left(); }
  /// Manipulator for modifier Format::right()
  inline Format& 
  right(Format& form) { return form.right(); }
  ///@}

  /**
   * @name Basefield Fmtflags Manipulators
   * A set of fmtflags manipulators that change basefield flags.
   * @{
   * @param form Form that is modified
   * @return input Form for manipulator chaining
   */
  // 27.4.5.3 basefield manipulators:
  /// Manipulator for modifier Format::dec()
  inline Format& 
  dec(Format& form) { return form.dec(); }
  /// Manipulator for modifier Format::hex()
  inline Format& 
  hex(Format& form) { return form.hex(); }
  /// Manipulator for modifier Format::oct()
  inline Format& 
  oct(Format& form) { return form.oct(); }
  ///@}

  /**
   * @name Floatfield Fmtflags Manipulators
   * A set of fmtflags manipulators that change floatfield flags.
   * @{
   * @param form Form that is modified
   * @return input Form for manipulator chaining
   */
  // 27.4.5.4 floatfield manipulators:
  /// Manipulator for modifier Format::fixed()
  inline Format& 
  fixed(Format& form) { return form.fixed(); }
  /// Manipulator for modifier Format::scientific()
  inline Format& 
  scientific(Format& form) { return form.scientific(); }
  ///@}

  //-------------------------------------------------------------------

  /**
   * @name Resetiosflags Manipulator
   * @{
   */
  /// Helper for resetiosflags()
  struct Format_Resetiosflags { fmtflags mask; };
  /// Helper for resetiosflags manipulator
  inline Format_Resetiosflags 
  resetiosflags (
    fmtflags mask) ///< fmtflags that are reset
  { 
    Format_Resetiosflags x; 
    x.mask = mask; 
    return x; 
  }
  /// Alternate name for resetiosflags manipulator
  inline Format_Resetiosflags
  resetflags (
    fmtflags mask) ///< fmtflags that are reset
  { return IoFormat::resetiosflags(mask); }
  /// Resetiosflags manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,           ///< Form that is modified
    Format_Resetiosflags x) ///< contains fmtflags that are reset
  { return form.setf(fmtflags(0), x.mask); }
  ///@}

  /**
   * @name Setiosflags Manipulator
   * @{
   */
  /// Helper for setiosflags()
  struct Format_Setiosflags { fmtflags flags; };
  /// Helper for setiosflags manipulator
  inline Format_Setiosflags 
  setiosflags (
    fmtflags flags) ///< fmtflags that are set
  { 
    Format_Setiosflags x; 
    x.flags = flags; 
    return x; 
  }
  /// Alternate name for setiosflags manipulator
  inline Format_Setiosflags
  setflags (
    fmtflags flags) ///< fmtflags that are set
  { return IoFormat::setiosflags(flags); }
  /// Setiosflags manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,         ///< Form that is modified
    Format_Setiosflags x) ///< contains fmtflags that are set
  { return form.setf(x.flags); }
  ///@}

  /**
   * @name Setbase Manipulator
   * @{
   */
  /// Helper for setbase()
  struct Format_Setbase { int base; };
  /// Helper for setbase manipulator
  inline Format_Setbase 
  setbase (
    int base) ///< base that is set (allowed is only 8, 10, or 16)
  { 
    Format_Setbase x; 
    x.base = base; 
    return x; 
  }
  /// Alternate name for setbase manipulator
  inline Format_Setbase
  base (
    int base) ///< base that is set (allowed is only 8, 10, or 16)
  { return IoFormat::setbase(base); }
  /// Setbase manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,     ///< Form that is modified
    Format_Setbase x) ///< contains base that is set
  {
    return form.setf (
      x.base ==  8 ? ios_base::oct : 
      x.base == 10 ? ios_base::dec : 
      x.base == 16 ? ios_base::hex : 
      fmtflags(0), ios_base::basefield);
  }
  ///@}

  /**
   * @name Setfill Manipulator
   * @{
   */
  /// Helper for setfill()
  struct Format_Setfill { char fill; };
  /// Helper for setfill manipulator
  inline Format_Setfill
  setfill (
    char fill) ///< fill character that is set
  { 
    Format_Setfill x; 
    x.fill = fill; 
    return x; 
  }
  /// Alternate name for setfill manipulator
  inline Format_Setfill
  fill (
    char fill) ///< fill character that is set
  { return IoFormat::setfill(fill); }
  /// Setfill manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,     ///< Form that is modified
    Format_Setfill x) ///< contains fill character that is set
  { return form.fill(x.fill); }
  ///@}

  /**
   * @name Setprecision Manipulator
   * @{
   */
  /// Helper for setprecision()
  struct Format_Setprecision { int precision; };
  /// Helper for setprecision manipulator
  inline Format_Setprecision 
  setprecision (
    int precision) ///< precision that is set
  { 
    Format_Setprecision x; 
    x.precision = precision; 
    return x; 
  }
  /// Alternate name for setprecision manipulator
  inline Format_Setprecision
  precision ( ///< precision that is set
    int precision)
  { return IoFormat::setprecision(precision); }
  /// Setprecision manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,          ///< Form that is modified
    Format_Setprecision x) ///< contains precision that is set
  { return form.precision(x.precision); }
  ///@}

  /**
   * @name Setwidth Manipulator
   * @{
   */
  /// Helper for setw()
  struct Format_Setw { int width; };
  /// Helper for setw manipulator
  inline Format_Setw 
  setw (
    int width) ///< minimum field width that is set
  { 
    Format_Setw x; 
    x.width = width; 
    return x; 
  }
  /// Alternate name for setw manipulator
  inline Format_Setw
  width (
    int width) ///< minimum field width that is set
  { return IoFormat::setw(width); }
  /// Setw manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,  ///< Form that is modified
    Format_Setw x) ///< contains minimum field width that is set
  { return form.width(x.width); }
  ///@}

  //-------------------------------------------------------------------
  // non-standard manipulators
  //-------------------------------------------------------------------

  /**
   * @name Setprefix Manipulator
   * @{
   */
  /// Helper for setprefix()
  struct Format_Setprefix { const char* prefix; };
  /// Helper for setprefix manipulator
  inline Format_Setprefix
  setprefix (
    const char* const prefix) ///< prefix string that is set
  { 
    Format_Setprefix x; 
    x.prefix = prefix; 
    return x; 
  }
  /// Alternate name for setprefix manipulator
  inline Format_Setprefix
  prefix ( ///< prefix string that is set
    const char* const prefix)
  {
    return IoFormat::setprefix(prefix);
  }
  /// Setprefix manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,       ///< Form that is modified
    Format_Setprefix x) ///< contains prefix string that is set
  { return form.prefix(x.prefix); }
  ///@}

  /**
   * @name Setpostfix Manipulator
   * @{
   */
  /// Helper for setpostfix()
  struct Format_Setpostfix { const char* postfix; };
  /// Helper for setpostfix manipulator
  inline Format_Setpostfix 
  setpostfix (
    const char* const postfix) ///< postfix string that is set
  { 
    Format_Setpostfix x; 
    x.postfix = postfix; 
    return x; 
  }
  /// Alternate name for setpostfix manipulator
  inline Format_Setpostfix
  postfix (
    const char* const postfix) ///< postfix string that is set
  { return IoFormat::setpostfix(postfix); }
  /// Setpostfix manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,        ///< Form that is modified
    Format_Setpostfix x) ///< contains postfix string that is set
  { return form.postfix(x.postfix); }
  ///@}

  /**
   * @name Setformat Manipulator
   * @{
   */
  /// Helper for setformat()
  struct Format_Setfmt { const char* fmtstr; };
  /// Helper for setformat manipulator
  inline Format_Setfmt 
  setformat (
    const char* const fmtstr) ///< @ref fmtstr that is set
  { 
    Format_Setfmt x; 
    x.fmtstr = fmtstr; 
    return x; 
  }
  /// Alternate name for setformat manipulator
  inline Format_Setfmt
  format (
    const char* const fmtstr) ///< @ref fmtstr that is set
  { return IoFormat::setformat(fmtstr); }
  /// Setformat manipulator binding to operator<< on ostream
  inline Format&
  operator<< (
    Format& form,    ///< Form that is modified
    Format_Setfmt x) ///< contains @ref fmtstr that is set
  { return form.format(x.fmtstr); }
  ///@}
}

#endif /* _IOFORMAT_ */
