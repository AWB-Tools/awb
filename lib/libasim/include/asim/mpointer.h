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

#ifndef _MPOINTER_H
#define _MPOINTER_H

#include <string>
#include "asim/mesg.h"
#include "asim/event.h"

//
// A MPointer is a software abstraction used to represent typical
// structure read/write head/tail pointers. It provides convenient
// operators for moving the pointer around and implicitly wraps around the
// "capacity" of the pointer (structure). You have a bunch of arithmetic
// operators defined on the pointer so you can do safe arithmetic with it.
//
class MPointer
{
 public:

     MPointer();

     //
     // Constructor with capacity
     //
     MPointer(UINT32 capacity);

     //
     // Consructor to be used if you whish this class to automatically
     // generate SetNodeTags with the pointer value for you. Each time
     // the pointer value changes, a SetNodeTag command will be issued
     // with the new pointer value
     //
     MPointer(UINT32 capacity, char *name, UINT32 nodeid);

     //
     // Operators
     //
     const MPointer&  operator++();
     const MPointer&  operator--();
     const MPointer   operator++(int);
     const MPointer   operator--(int);
     const MPointer&  operator+=(int off);
     const MPointer&  operator-=(int off);
     const MPointer   operator+(int off);
     const MPointer   operator-(int off);
     const MPointer&  operator=(const MPointer& p);
     const MPointer&  operator=(UINT32 idx);
     UINT32           operator*();
     UINT32           Get();
                      operator UINT32() const;
     void           Enable();
     void           Disable();
     bool           IsEnabled();

 private:
     string name;
     UINT32 capacity;
     UINT32 nodeid;
     UINT32 curr;
     bool enabled;
};

inline 
MPointer::MPointer() :
   name("ANONYMOUS"),
   capacity(0), 
   nodeid(0)
{
 curr     = 0;
 enabled  = true;
}

inline 
MPointer::MPointer(UINT32 capacity) :
   name("ANONYMOUS"),
   capacity(capacity), 
   nodeid(0)
{
 curr     = 0;
 enabled  = true;
}
inline 
MPointer::MPointer(UINT32 capacity, char *name, UINT32 nodeid) :
   capacity(capacity), 
   nodeid(nodeid)
{
 this->name = strdup(name);
 curr     = 0;
 enabled  = true;
}

inline void
MPointer::Enable()
{
 enabled  = true;
 DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));
}

inline void 
MPointer::Disable()
{
 enabled  = false;
 DRALEVENT(SetNodeTag(nodeid, name.c_str(), capacity, 0, NULL, false));
}

inline bool 
MPointer::IsEnabled()
{
 return enabled;
}

inline const MPointer&
MPointer::operator++()
{
 curr++;
 if ( curr >= capacity ) curr = 0;

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return *this;
}

inline const MPointer
MPointer::operator++(int)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 MPointer p = *this;
 operator++();

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return p;
}

inline const MPointer&
MPointer::operator--()
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 curr--;
 // Note curr is UINT32, hence when curr drops below zero it will become a
 // very large positive number. Hence, the use of ">=".
 if ( curr >= capacity ) curr = (capacity - 1);

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return *this;
}

inline const MPointer
MPointer::operator--(int)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 MPointer p = *this;
 operator--();

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return p;
}

inline const MPointer&
MPointer::operator+=(int off)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 //
 // Avoid negative case by flipping sign and calling the decrement
 // operator
 //
 if ( off < 0 ) return operator-=(-off);

 curr += off;
 if ( curr >= capacity ) curr -= capacity;

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return *this;
}

inline const MPointer&
MPointer::operator-=(int off)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 //
 // Avoid negative case by flipping sign and calling the increment
 // operator
 //
 if ( off < 0 ) return operator+=(-off);

 if ( ((UINT32)off) > curr ) {
  curr = capacity - off + curr;
 }
 else {
  curr -= off;
 }
 ASSERTX(curr < capacity);

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return *this;
}

inline const MPointer
MPointer::operator+(int off)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 MPointer p = *this;
 p += off;

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return p;
}

inline const MPointer
MPointer::operator-(int off)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 MPointer p = *this;
 p -= off;

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return p;
}

inline const MPointer&
MPointer::operator=(const MPointer& p)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 curr = p.curr;
 capacity = p.capacity;
 name = p.name;
 nodeid = p.nodeid;

 return *this;
}

inline const MPointer&
MPointer::operator=(UINT32 idx)
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 //
 // The % operation is so expensive that we try to avoid it with a
 // previous "if" to check whether it is really necessary
 //
 if ( idx < capacity ) {
  curr = idx;
 }
 else { 
  curr = idx % capacity;
 }

  DRALEVENT(SetNodeTag(nodeid, name.c_str(), curr, 0, NULL, false));

 return *this;
}

inline
MPointer::operator UINT32 () const
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 return curr;
}

inline UINT32
MPointer::operator*()
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 return curr;
}

inline UINT32
MPointer::Get()
{
 ASSERT(enabled, name.c_str() << " mpointer accessed while disabled");
 return curr;
}

#endif // _MPOINTER_H
