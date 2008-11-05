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
 * @file register.h
 * @author Michael Adler
 * @brief Architecture neutral register properties
 */

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Hey, you!  Yes, you!  This file is included by code in the SoftSDV library.
// The only file that may be included here is asim/syntax.h.  No code for
// ARCH_REGISTER_CLASS can live outside this .h file.
//
// There is a preprocessor variable defined during softsdv stub compilation
// (coincidentally named SOFTSDV_STUB) to avoid problems in SoftSDV.  I know,
// I know, we are studiously trying to avoid preprocessor #defines.  The only
// other option is replicating the file in SoftSDV and risking incorrect
// register value claims.  This seems to be the best option.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _ARCHREGISTER_
#define _ARCHREGISTER_

#include <iostream>
#include <string.h>

// ASIM core
#include "asim/syntax.h"

#ifndef SOFTSDV_STUB
#include "asim/mesg.h"
#endif

typedef class ARCH_REGISTER_CLASS *ARCH_REGISTER;

//
// ARCH_REGISTER_TYPE is an enumeration of all possible register types of
// all machines simulated by Asim.  Keeping all machines in the same
// enumeration allows us to describe registers in the feeder interface.
//
// **** All register numbers are logical on IPF.  The only exception is ****
// **** predicate_vector64, in which the bit positions correspond to    ****
// **** physical register numbering.                                    ****
//
enum ARCH_REGISTER_TYPE
{
    REG_TYPE_INVALID,           // Not a register
    REG_TYPE_INT,               // Integer register
    REG_TYPE_FP64,              // Floating point (64 bits)
    REG_TYPE_FP82,              // Floating point (82 bits - IPF)
    REG_TYPE_PREDICATE,         // Single binary predicate
    REG_TYPE_PREDICATE_VECTOR64,// Vector of 64 predicate bits (IPF)

    REG_TYPE_AR,                // IPF
    REG_TYPE_BR,                // IPF
    REG_TYPE_CR,                // IPF

    REG_TYPE_MSR,               // x86
    REG_TYPE_SEG,               // x86 uop register
    REG_TYPE_CTRL,              // x86 uop register
    REG_TYPE_FLAGS,             // x86 uop register
    REG_TYPE_NULL,              // x86 uop register
    REG_TYPE_INFO,              // x86 uop register
    REG_TYPE_VECTOR,            // x86 uop register

    REG_TYPE_RAW,               // raw register -- used for passing values
                                //   around as an array of bytes
    REG_TYPE_UNKNOWN,           // unknown register type
    REG_TYPE_LAST               // MUST BE LAST!!!
};


//
// ARCH_REGISTER_CLASS supports both scalar and vector registers.  For scalar
// registers it supports 128 bit objects in addition to 64 bit so that
// 80 bit FP and 65 bit integer values can be stored.  Access to FP registers
// is limited to 64 bits.  To describe an 80 bit FP register break it down
// into a pair of 64 bit integer values.
//
class ARCH_REGISTER_CLASS
{
  public:

    /******************************************
     *Constructors
     ******************************************/

    ARCH_REGISTER_CLASS() : regNum(-1), rType(REG_TYPE_INVALID),
                            physicalNumber(0), rawSize(0), knownValue(false)
    {
        u.value[0] = 0;
        u.value[1] = 0;
    };

    ARCH_REGISTER_CLASS(
        ARCH_REGISTER_TYPE rType,
        INT32 regNum) : regNum(regNum), rType(rType),
                        physicalNumber(0), rawSize(0), knownValue(false)
    {
        u.value[0] = 0;
        u.value[1] = 0;
    };

    // Integer register with value -- 64 bits
    ARCH_REGISTER_CLASS(
        ARCH_REGISTER_TYPE rType,
        INT32 regNum,
        UINT64 iValue0) : regNum(regNum), rType(rType),
                          physicalNumber(0), rawSize(sizeof(UINT64)),
                          knownValue(true)
    {
        u.value[0] = iValue0;
        u.value[1] = 0;
    };
            
    // Integer register with value -- 128 bits
    ARCH_REGISTER_CLASS(
        ARCH_REGISTER_TYPE rType,
        INT32 regNum,
        UINT64 iValue0,
        UINT64 iValue1) : regNum(regNum), rType(rType),
                          physicalNumber(0), rawSize(sizeof(UINT64)),
                          knownValue(true)
    {
        u.value[0] = iValue0;
        u.value[1] = iValue1;
    };
            
    // FP register with value
    ARCH_REGISTER_CLASS(
        ARCH_REGISTER_TYPE rType,
        INT32 regNum,
        double fValue) : regNum(regNum), rType(rType),
                         physicalNumber(0), rawSize(sizeof(double)),
                         knownValue(true)
    {
        ASSERTX(rType == REG_TYPE_FP64);
        u.value[1] = 0;
        u.fValue = fValue;
    };
        
    // Any register with raw value.  Useful for opaque transfer of values
    // between code that understands how to interpret the data.
    ARCH_REGISTER_CLASS(
        ARCH_REGISTER_TYPE rType,
        INT32 regNum,
        void *rawValue,
        UINT32 nBytes) : regNum(regNum), rType(rType),
                         physicalNumber(0), rawSize(nBytes), knownValue(true)
    {
        ASSERTX(rType == REG_TYPE_RAW);
        ASSERTX(nBytes <= sizeof(u.rawValue));
        memcpy(u.rawValue, rawValue, nBytes);
    };

    ~ARCH_REGISTER_CLASS() {};


    /*********************************************
     *Get Operations
     *********************************************/
   
    ARCH_REGISTER_TYPE GetType(void) const { return rType; };

    UINT64 GetPhysicalNumber(void) const { return physicalNumber; };

    INT32 GetNum(void) const { return regNum; };

    UINT64 GetIValue(void) const
    {
        ASSERTX(HasKnownValue());
        return u.value[0];
    };

    UINT64 GetHighIValue(void) const
    {
        ASSERTX(HasKnownValue() && (rType == REG_TYPE_FP82));
        return u.value[1];
    };

    double GetFValue(void) const
    {
        ASSERTX(HasKnownValue() && (rType == REG_TYPE_FP64));
        return u.fValue;
    };

    const void *GetRawValue(void) const
    {
        ASSERTX(HasKnownValue());
        return u.rawValue;
    };

    UINT32 GetRawSize(void) const
    {
        ASSERTX(HasKnownValue());
        return rawSize;
    };

    //
    // FP82 components
    //
    UINT32 GetFP82Sign(void) const
    {
        ASSERTX(HasKnownValue() && (rType == REG_TYPE_FP82));
        return (u.value[1] >> 17) & 1;
    };

    UINT32 GetFP82Exponent(void) const
    {
        ASSERTX(HasKnownValue() && (rType == REG_TYPE_FP82));
        return (u.value[1] & 0x1ffff);
    };

    UINT64 GetFP82Significand(void) const
    {
        ASSERTX(HasKnownValue() && (rType == REG_TYPE_FP82));
        return u.value[0];
    };

    /*********************************************
     *Register Tests
     *********************************************/

    bool IsValid(void) const
    {
        return (rType != REG_TYPE_INVALID) && (regNum != -1);
    };

    bool HasKnownValue(void) const
    {
        return knownValue;
    };

    bool SameRegister(const ARCH_REGISTER_CLASS &rhs) const
    {
        return ((rType == rhs.rType) && (regNum == rhs.regNum));
    };

    //
    // Test to see if the registers map to the same physical register
    // must have the same type and register name.
    //
    bool SamePhysicalRegister(const ARCH_REGISTER_CLASS &rhs) const
    {
        return ((physicalNumber == rhs.physicalNumber) &&
                (rType == rhs.rType));
    };

    /*********************************************
     *Register Field assigments
     *********************************************/

    //
    // Note:  setting most fields individually in the register is difficult
    // on purpose.  It is assumed that the details of the register are known
    // when the constructor is used.  By making it difficult to set invidual
    // fields it is easier to keep values names and numbers consistent.
    //

    void SetValue(UINT64 iValue0, UINT64 iValue1 = 0)
    {
        u.value[0] = iValue0;
        u.value[1] = iValue1;
        knownValue = true;
    };

    void SetPhysicalNumber(UINT64 physicalNumber)
    {
        this->physicalNumber = physicalNumber;
    };

/**************************************************
 *Register Data Members
 **************************************************/

  private:
    INT32 regNum;                   // Register number
    ARCH_REGISTER_TYPE rType;

        // IPF Specfic details of physical register number:
        // The physical number of the register is 64 bits because one of
        // the ways to flatten the rotating register stack is to use the
        // BSP which essentially maps the register stack into a memory space. 
        // For predicate and floating point registers that are rotated 
        // this value will be the physical register after the rotation. 
    UINT64 physicalNumber;          // The physical register holding
                                    // architectural register

    //
    // Multiple representations of a register's value
    //
    union
    {
        UINT64 value[2];
        double fValue;
        unsigned char rawValue[16];
    } u;

    UINT32 rawSize;
    bool knownValue;                // Value is valid
};


#ifndef SOFTSDV_STUB

//
// Format an ARCH_REGISTER_CLASS for printing.
//
ostream&
operator<< (
    ostream& out,
    const ARCH_REGISTER_CLASS &reg);

#endif /* SOFTSDV_STUB */


#endif /* _ARCHREGISTER_ */
