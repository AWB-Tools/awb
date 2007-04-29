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

#ifndef MOD_NUMBERS_H
#define MOD_NUMBERS_H

#define MAP_TO_INTERNAL(x) (int)((x) - min_value)
#define MAP_TO_EXTERNAL(x) (int)((x) + min_value)
class ModNumberDyn
{
  private:
    int offset;
    
    int min_value;
    int max_value;
    unsigned int max_offset;

    //  Check and correct wrapping
    void correctWrap()
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

  public:
    // default sets up a toggler
    ModNumberDyn(int max=1, int min=0, int value=0)
    {
        SetMaxMin(max, min, value);
    }

    ModNumberDyn(const ModNumberDyn & x, int value)
    {
        *this = x;
        Set(value);
    }

    ModNumberDyn Set(int value)
    {
        offset = MAP_TO_INTERNAL(value);
        correctWrap();
        
        return *this;
    }

    ModNumberDyn Set(ModNumberDyn num)
    {
        offset = num.offset;
        min_value = num.min_value;
        max_value = num.max_value;
        
        return *this;
    }
    
    int Get()
    {
        return MAP_TO_EXTERNAL(offset);
    }

    int GetMax() 
    { 
        return max_value; 
    }
    
    int GetMin() 
    { 
        return min_value; 
    }

    void SetMaxMin(int max=1, int min=0, int initial_value=0)
    {
        // asim bug #0000741.  do not allow for max to be less than the min.  
        // this can cause very bad things to happen (see bug report).
        // --slechta
        if (max < min)
        {
            max = min;
        }

        min_value = min;
        max_value = max;
        
        max_offset = max_value - min_value;
        
        offset = MAP_TO_INTERNAL(initial_value);
        correctWrap();
    }

    ModNumberDyn Next()
    {
        ++offset;
        correctWrap();
        
        return *this;
    }

    ModNumberDyn Prev()
    {
        --offset;
        correctWrap();
        
        return *this;
    }

#if OPERATORS
    // a + b == a.operator+(b)
    int operator+(int delta) {}
    int operator-(int delta) {}
#endif

    //  ++a  == a.operator++()     [prefix]
    ModNumberDyn operator++()
    {
        ++offset;
        correctWrap();

        return *this;
    }

    ModNumberDyn operator--()
    {
        --offset;
        correctWrap();

        return *this;
    }

    //  a++  == a.operator++(int)  [postfix]
    ModNumberDyn operator++(int dummy)
    {
        ModNumberDyn rv(*this);

        ++offset;
        correctWrap();

        return rv;
    }

    ModNumberDyn operator--(int dummy)
    {
        ModNumberDyn rv(*this);

        --offset;
        correctWrap();

        return rv;
    }

    //  Booleans
    bool operator==(int value)
    {
        return (offset == (int)(MAP_TO_INTERNAL(value) % (max_offset + 1)));
    }

    bool operator==(ModNumberDyn num)
    {
        return (offset == num.offset);
    }

    bool operator!=(int value)
    {
        return (offset != (int)(MAP_TO_INTERNAL(value) % (max_offset + 1)));
    }

    bool operator!=(ModNumberDyn num)
    {
        return (offset != num.offset);
    }
};



#undef MAP_TO_INTERNAL
#undef MAP_TO_EXTERNAL

#endif
