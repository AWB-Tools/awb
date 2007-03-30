/*
 *Copyright (C) 2005-2006 Intel Corporation
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


#ifndef MOD_NUMBERS_H
#define MOD_NUMBERS_H

#define MAP_TO_INTERNAL(x) (int)((x) - min_value)
#define MAP_TO_EXTERNAL(x) (int)((x) + min_value)

template<int MAXIMUM, int MINIMUM=0>
class ModNumber
{
    //
    //  NOTE:  Internal value representation
    //  
    //  In order to simplify internal calculations, the value will be
    //  represented by 2 numbers: the minimum value and an offset from
    //  the minimum value.
    //
    //  The offset will always be in the range [0..(max_value - min_value)]
    //  Preprocessor macros are used to handle the internal/external and
    //  external/internal mapping to alloc the compiler to optimize for the
    //  common case (minimum value is zero), while avoiding partial template
    //  specialization
    //
  private:
    int offset;
    
    int min_value;
    int max_value;
    unsigned int max_offset;

    //  Check and correct wrapping
    void correctWrap();

  public:
    ModNumber(int initial_value=MINIMUM);

    ModNumber Set(int value);
    ModNumber Set(ModNumber num);

    int Get();

    int GetMax() { return MAXIMUM; }
    int GetMin() { return MINIMUM; }

    ModNumber<MAXIMUM, MINIMUM> Next();
    ModNumber<MAXIMUM, MINIMUM> Prev();

#if OPERATORS
    // a + b == a.operator+(b)
    int operator+(int delta);   
    int operator-(int delta);   
#endif

    //  ++a  == a.operator++()     [prefix]
    ModNumber<MAXIMUM, MINIMUM>  operator++();
    ModNumber<MAXIMUM, MINIMUM>  operator--();

    //  a++  == a.operator++(int)  [postfix]
    ModNumber<MAXIMUM, MINIMUM>  operator++(int dummy);
    ModNumber<MAXIMUM, MINIMUM>  operator--(int dummy);

    //  Booleans
    bool operator==(int value);
    bool operator==(ModNumber num);
    bool operator!=(int value);
    bool operator!=(ModNumber num);
};

//
//  Constructors
//
template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::ModNumber(int initial_value)
{
    min_value = MINIMUM;
    max_value = MAXIMUM;

    max_offset = max_value - min_value;

    offset = MAP_TO_INTERNAL(initial_value);
    correctWrap();
}

template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::Set(int value)
{
    offset = MAP_TO_INTERNAL(value);
    correctWrap();

    return *this;
}

template<int MAXIMUM, int MINIMUM>
inline ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::Set(ModNumber num)
{
    offset = num.offset;
    min_value = num.min_value;
    max_value = num.max_value;

    return *this;
}

template<int MAXIMUM, int MINIMUM>
inline int
ModNumber<MAXIMUM, MINIMUM>::Get()
{
    return MAP_TO_EXTERNAL(offset);
}

template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::Next()
{
    ++offset;
    correctWrap();

    return *this;
}

//  Prefix ++
template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::operator++()
{
    ++offset;
    correctWrap();

    return *this;
}

//  Postfix ++
template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::operator++(int dummy)
{
    ModNumber<MAXIMUM, MINIMUM> rv = *this;

    ++offset;
    correctWrap();

    return rv;
}

//--------------------------------------------

template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::Prev()
{
    --offset;
    correctWrap();

    return *this;
}

//  Prefix --
template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::operator--()
{
    --offset;
    correctWrap();

    return *this;
}


//  Postfix --
template<int MAXIMUM, int MINIMUM>
ModNumber<MAXIMUM, MINIMUM>
ModNumber<MAXIMUM, MINIMUM>::operator--(int dummy)
{
    ModNumber<MAXIMUM, MINIMUM> rv = *this;

    --offset;
    correctWrap();

    return rv;
}


#if OPERATORS
template<int MAXIMUM, int MINIMUM>
inline int
ModNumber<MAXIMUM, MINIMUM>::operator+(int delta)
{
}

template<int MAXIMUM, int MINIMUM>
inline int
ModNumber<MAXIMUM, MINIMUM>::operator-(int delta)
{
}
#endif


template<int MAXIMUM, int MINIMUM>
inline bool
ModNumber<MAXIMUM, MINIMUM>::operator==(ModNumber<MAXIMUM, MINIMUM> num)
{
    return (offset == num.offset);
}

template<int MAXIMUM, int MINIMUM>
inline bool
ModNumber<MAXIMUM, MINIMUM>::operator==(int value)
{
    return (offset == MAP_TO_INTERNAL(value));
}

template<int MAXIMUM, int MINIMUM>
inline bool
ModNumber<MAXIMUM, MINIMUM>::operator!=(ModNumber<MAXIMUM, MINIMUM> num)
{
    return (offset != num.offset);
}

template<int MAXIMUM, int MINIMUM>
inline bool
ModNumber<MAXIMUM, MINIMUM>::operator!=(int value)
{
    return (offset != MAP_TO_INTERNAL(value));
}

//========================================

template<int MAXIMUM, int MINIMUM>
inline void
ModNumber<MAXIMUM, MINIMUM>::correctWrap()
{
    //  Since the correct 'offset' value is always in the
    //  range [0..(max_value - min_value)], we can simply
    //  use the modulo operator to correct wrapping problems

    //  correct the 'past the high end' wrap
    offset = offset % (max_offset + 1);

    //  correct for the 'lower than the low end' wrap
    if (offset < 0) {
	offset = offset + (max_offset + 1);
    }
}

//========================================

#if 0
template<int MAXIMUM, int MINIMUM>
void 
operator=(int &lvalue, ModNumber<MAX, MIN> &rvalue)
{
    lvalue = rvalue.Get();
}
#endif


#undef MAP_TO_INTERNAL
#undef MAP_TO_EXTERNAL

#endif
