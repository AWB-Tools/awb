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

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/arch_register.h"

ostream&
operator<< (
    ostream& out,
    const ARCH_REGISTER_CLASS &reg)
{
    if (! reg.IsValid())
    {
        out << "<Invalid>";
        return out;
    }

    char t;
    switch (reg.GetType())
    {
      case REG_TYPE_INT:
        t = 'I';
        break;
      case REG_TYPE_FP64:
      case REG_TYPE_FP82:
        t = 'F';
        break;
      case REG_TYPE_PREDICATE:
      case REG_TYPE_PREDICATE_VECTOR64:
        t = 'P';
        break;
      case REG_TYPE_AR:
        t = 'A';
        break;
      case REG_TYPE_BR:
        t = 'B';
        break;
      case REG_TYPE_CR:
        t = 'C';
        break;
      case REG_TYPE_MSR:
        t = 'M';
        break;
      case REG_TYPE_SEG:
        t = 'S';
        break;
      case REG_TYPE_RAW:
        t = 'R';
        break;
      default:
        out << "<Unknown reg type>";
        return out;
    }

    out << t;
    if (reg.GetType() == REG_TYPE_PREDICATE_VECTOR64)
    {
        out << 'V';
    }
    else
    {
        out << reg.GetNum();
    }

    out << " (";

    if (! reg.HasKnownValue())
    {
        out << "<Unknown value>";
    }
    else
    {
        if (reg.GetType() == REG_TYPE_RAW)
        {
            out << "0x";
            UINT32 i = reg.GetRawSize();
            const UINT8 *value = (const UINT8 *)reg.GetRawValue();
            while (i)
            {
                i -= 1;
                out << fmt("02x", UINT32(value[i]));
            }
        }
        else if (reg.GetType() == REG_TYPE_FP82)
        {
            out << reg.GetFP82Sign()
                << " 0x" << fmt_x(reg.GetFP82Exponent())
                << " 0x" << fmt_x(reg.GetFP82Significand())
                << " [bignum]";
        }
        else
        {
            out << "0x" << fmt_x(reg.GetIValue()) << dec;

            if (reg.GetType() == REG_TYPE_FP64)
            {
                out << " [" << reg.GetFValue() << "]";
            }
        }
    }

    out << ")";
    return out;
}
