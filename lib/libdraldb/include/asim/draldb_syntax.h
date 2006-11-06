/*
 * Copyright (C) 2003-2006 Intel Corporation
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author David Goodwin, Artur Klauser, Pritpal Ahuja
 * @brief OS and architecture specific definitions.
 */

#ifndef _SYNTAX_
#define _SYNTAX_

/*
 * OS specific definitions
 */

/*
 * x86 Linux
 */
#if defined(HOST_LINUX_X86)

// stdint.h is generic, move this out of ifdef eventually!
#include <stdint.h>
#include <limits.h>

typedef  int8_t              INT8;
typedef uint8_t             UINT8;
typedef  int16_t             INT16;
typedef uint16_t            UINT16;
typedef  int32_t             INT32;
typedef uint32_t            UINT32;
typedef  int64_t             INT64;
typedef uint64_t            UINT64;
typedef  intptr_t  PTR_SIZED_INT;
typedef uintptr_t PTR_SIZED_UINT;

// already defined in stdint.h:
// #define UINT32_MAX          4294967295UL
// #define UINT64_MAX          18446744073709551615ULL
#define UINT64_CONST(c)     UINT64_C(c)

// DEPRECATED! architecture dependent print format support
#define FMT32D              "%d"
#define FMT16D              "%hd"
#define FMT32U              "%u"
#define FMT16U              "%hu"
#define FMT32X              "%x"
#define FMT16X              "%hx"
// stdint.h doesn't help us with the formatting strings.
// On 32-bit systems, 64-bit integers are "long long",
// and on 64-bit systems they are just plain "long".
#if __WORDSIZE < 64
# define FMT64D             "%lld"
# define FMT64U             "%llu"
# define FMT64X             "%llx"
# define FMT64O             "%llo"
# define FMTP64O          "%#0llo"
# define FMTP64X          "%#0llx"
# define FMTP64U           "%0llu"
#else
# define FMT64D             "%ld"
# define FMT64U             "%lu"
# define FMT64X             "%lx"
# define FMT64O             "%lo"
# define FMTP64O          "%#0lo"
# define FMTP64X          "%#0lx"
# define FMTP64U           "%0lu"
#endif


/*
 * Digital Unix, Alpha Linux, IA64 Linux (64-bit *nixs)
 */
#elif defined(HOST_DUNIX) || defined(HOST_LINUX) || defined(HOST_LINUX_IA64)
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef long                INT64;
typedef unsigned long       UINT64;
typedef INT64               PTR_SIZED_INT;
typedef UINT64              PTR_SIZED_UINT;
// generic
#include <limits.h>
#define UINT32_MAX          UINT_MAX
#define UINT64_MAX          ULONG_MAX
#define UINT64_CONST(c)     (c##UL)

// DEPRECATED! architecture dependent print format support
#define FMT64D              "%ld"
#define FMT32D              "%d"
#define FMT16D              "%hd"
#define FMT64U              "%lu"
#define FMT32U              "%u"
#define FMT16U              "%hu"
#define FMT64X              "%lx"
#define FMT32X              "%x"
#define FMT16X              "%hx"
#define FMT64O              "%lo"
#define FMTP64O             "%#0lo"
#define FMTP64X             "%#0lx"
#define FMTP64U             "%0lu"

/*
 * FreeBSD
 */
#elif defined(HOST_FREEBSD) || defined(HOST_FREEBSD_X86)
typedef char                INT8;
typedef u_int8_t            UINT8;
typedef int16_t             INT16;
typedef u_int16_t           UINT16;
typedef int32_t             INT32;
typedef u_int32_t           UINT32;
typedef int64_t             INT64;
typedef u_int64_t           UINT64;
typedef INT32               PTR_SIZED_INT;
typedef UINT32              PTR_SIZED_UINT;
// generic
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#define UINT32_MAX          UINT_MAX
#define UINT64_MAX          UQUAD_MAX
#define UINT64_CONST(c)     (c##ULL)

// DEPRECATED! architecture dependent print format support
#define FMT64D              "%qd"
#define FMT32D              "%d"
#define FMT16D              "%hd"
#define FMT64U              "%qu"
#define FMT32U              "%u"
#define FMT16U              "%hu"
#define FMT64X              "%qx"
#define FMT32X              "%x"
#define FMT16X              "%hx"

#else
#error Specify host type (e.g. HOST_DUNIX, HOST_FREEBSD, HOST_LINUX)
#endif


/*
 * Common defintions
 */
#define EXTERNC             extern "C"

typedef unsigned int UINT;
typedef signed int   INT;


/*
 * Common defintions
 */
#ifndef NULL
#define NULL                0
#endif

// -------------------------------------------------------------------------
// No matter whether we inherited QT/ASIM or not, BYTE and UBYTE types
// are not defined
// -------------------------------------------------------------------------
typedef INT8   BYTE;
typedef UINT8 UBYTE;

/*
 * NAME_CAT - Infamous preprocessor hack.  This macro
 * expands to the concatenation of name1 and name2.  Thus
 * NAME_CAT(xx,yy) ==> xxyy.
 *
 * WARNING:  You must write NAME_CAT(xx,yy) and not NAME_CAT( xx, yy )
 *
 * NOTE:  The lack of spaces below is deliberate. Don't change it.
 */
#define NAME_GLUE(name1,name2) name1##name2
#define NAME_CAT(name1,name2) NAME_GLUE(name1,name2)


/*
 * MAX
 * MIN
 */
#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#define MIN(a,b) (((a) > (b)) ? (b) : (a))

/*
 * Pretty printing of a bool value
 */
#define BOOL2STR(a) (((a) == true) ? "true" : "false")

/* circular queue inc/dec macros; avoid slow modulo */
#define UP_1_WRAP(VAR,SIZE)        ((VAR)+1 < (SIZE) ? (VAR)+1 : 0)
#define DOWN_1_WRAP(VAR,SIZE)      ((VAR) == 0 ? (SIZE)-1 : (VAR)-1)
#define UP_N_WRAP(VAR,SIZE,N)        ((VAR)+N < (SIZE) ? (VAR)+N : ((VAR)+N)-SIZE)

/* special versions for size being a power of 2 */
#define UP_1_WRAP_POW2(VAR,SIZE)   (((VAR)+1) & ((SIZE)-1))
#define DOWN_1_WRAP_POW2(VAR,SIZE) (((VAR)-1) & ((SIZE)-1))

/* make sure something is really a power of 2 */
#define IS_POW2(VAR) ((VAR & (VAR-1)) == 0)

// -------------------------------------------------------------------------
//
// Macros for declaring a variable that has simple Get() and Set() accessors.
// Direct access to the storage location is prevented.  These macros make it
// easy to declare controlled storage so that you later have the option to
// make the Get/Set accessor functions more complicated by replacing the
// storage declaration with other code.  In addition to providing the Get/Set
// functions, the storage is automatically initialized to 0 or a value
// you specify.  Because of limitations in C++ templates the type of the
// storage must support initialization with an integer value.  This works
// for scalars, boolean, FP...
//
// There are 12 macros with variations for the Set() function being public,
// protected and private and supporting the ability to set the initial value
// of the storage or letting it default to 0.  In addition you may specify
// an assertion to test on the Set() function to validate the incoming value.
//
// Macros that set the initial value to 0 are:
//     PRIVATE_OBJ
//     PROTECTED_OBJ
//     PUBLIC_OBJ
//
// Macros that initialize the storage with a user-specified value are:
//     PRIVATE_OBJ_INIT
//     PROTECTED_OBJ_INIT
//     PUBLIC_OBJ_INIT
//
// Macros with assertions are the same as above with _ASSERT at the end.
//
// The type of the object is always the first argument follwed by the name
// and, optionally, the initial value.  Access to the storage is through
// Get<name>() and Set<name>().  For example:
//
// class Foo {
//     PRIVATE_OBJ(UINT64, Bar);
// }
//
// adds a UINT64 to class Foo and initializes it to 0.  It defines a public
// GetBar() function for reading the object and a SetBar() function private
// to Foo for setting it.
//
// Assertions specify the condition to test and are passed to ASSERTX.
// The name of the new value in an assertion is "_NewVal".  For example:
//
// class Foo {
//     PRIVATE_OBJ_INIT_ASSERT(UINT8, Bar, 1, _NewVal < 128);
// }
//
// declares a one byte integer with initial value 1 and an assertion that
// triggers on attempts to set the value greater than 127.
//
// NOTE: the macros always end with private scope.  There is no way in the
// preprocessor to know what the current scope is and restore it at the end
// of the macro.
//
// -------------------------------------------------------------------------

//
// This class and macro support the main macros.  Don't use them outside of
// this file as they may change.
//

#ifdef __cplusplus

template <class T, int initialValue = 0>
class PROTECTED_STORAGE_CLASS
{
private:
    T _val;

public:
	PROTECTED_STORAGE_CLASS(const T& t = T(initialValue))
        : _val(t)
    {}

	const T& operator()() const { return _val; }	// Get
    void operator()(const T& t) { _val = t; }		// Set
};

//
// CPP won't concatenate obj ## (.  Hence the use of obj ## _(.
//
#define OBJ_INSTANCE(type, obj, init, scope) \
	scope: \
	    const void Set ## obj(const type & t) { obj ## _(t); }; \
	public: \
	    const type & Get ## obj() const { return obj ## _(); }; \
	private: \
	    PROTECTED_STORAGE_CLASS<type, init> obj ## _


#define OBJ_INSTANCE_ASSERT(type, obj, init, scope, assertion) \
	scope: \
	    const void Set ## obj(const type & _NewVal) { \
	        ASSERTX(assertion); \
            obj ## _(_NewVal); \
        }; \
	public: \
	    const type & Get ## obj() const { return obj ## _(); }; \
	private: \
	    PROTECTED_STORAGE_CLASS<type, init> obj ## _


//
// Macros for use outside this file...
//
#define PRIVATE_OBJ_INIT(type, obj, init) OBJ_INSTANCE(type, obj, init, private)
#define PRIVATE_OBJ(type, obj) PRIVATE_OBJ_INIT(type, obj, 0)

#define PROTECTED_OBJ_INIT(type, obj, init) OBJ_INSTANCE(type, obj, init, protected)
#define PROTECTED_OBJ(type, obj) PROTECTED_OBJ_INIT(type, obj, 0)

#define PUBLIC_OBJ_INIT(type, obj, init) OBJ_INSTANCE(type, obj, init, public)
#define PUBLIC_OBJ(type, obj) PUBLIC_OBJ_INIT(type, obj, 0)

#define PRIVATE_OBJ_INIT_ASSERT(type, obj, init, assertion) OBJ_INSTANCE_ASSERT(type, obj, init, private, assertion)
#define PRIVATE_OBJ_ASSERT(type, obj, assertion) PRIVATE_OBJ_INIT_ASSERT(type, obj, 0, assertion)

#define PROTECTED_OBJ_INIT_ASSERT(type, obj, init, assertion) OBJ_INSTANCE_ASSERT(type, obj, init, protected, assertion)
#define PROTECTED_OBJ_ASSERT(type, obj, assertion) PROTECTED_OBJ_INIT_ASSERT(type, obj, 0, assertion)

#define PUBLIC_OBJ_INIT_ASSERT(type, obj, init, assertion) OBJ_INSTANCE_ASSERT(type, obj, init, public, assertion)
#define PUBLIC_OBJ_ASSERT(type, obj, assertion) PUBLIC_OBJ_INIT_ASSERT(type, obj, 0, assertion)

#endif /* __cplusplus */

#endif /* _SYNTAX_ */
