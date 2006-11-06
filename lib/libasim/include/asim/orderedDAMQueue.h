/*
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

//
// Author: Federico Silla
//



#ifndef _ORDERED_DAMQUEUE_H
#define _ORDERED_DAMQUEUE_H

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"

template <class T> 
class OrderedDAMQueue
{
  public:
    OrderedDAMQueue<T>()
        :  Head(0), Tail(0), Middle(0), Size(0), SearchPtr(0)
    {}

    void   Init(UINT16 n);
    bool   Exists(void); 
    bool   IsEmpty(void);
    bool   IsFull(void); 
    void   Store(T t, UINT64 key); 
    T      GetEntry(UINT16 n); 
    void   DelEntry(UINT16 n); 
    UINT16 GetOccupancy(void);
    UINT16 GetCapacity(void);
    UINT16 GetFreeSpace(void);
    INT16  GetNextEntryNum(void);
    T      GetNextEntry(void);
    INT16  GetCurrentEntryNum(void);
    T      GetCurrentEntry(void);
    void   ResetSearchPtr(void);



  protected:
    struct ITEM 
      {T      Data;
       UINT64 Key;
       INT32  Next;
       INT32  Prev;
      };

    ITEM   *Pool;
    UINT16 Head;
    UINT16 Tail;
    UINT16 Middle;
    UINT16 Size; 
    UINT16 SearchPtr;
    UINT16 FreePtr;
    UINT16 MaxSize; 
};



template <class T> 
void OrderedDAMQueue<T>::Init(UINT16 n)
{
    if (n != 0)
       {MaxSize = n; 
	Pool = new ITEM[MaxSize];
        for (UINT16 i=0; i<MaxSize; i++)
          Pool[i].Data = NULL;
       }
    else
       {MaxSize = 0;
	Pool = NULL; 
       }
}



template <class T>
inline
bool OrderedDAMQueue<T>::Exists(void)
{
 return MaxSize > 0;
}



template <class T>
inline
bool OrderedDAMQueue<T>::IsFull(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n"); 
    return (Size == MaxSize); 
}



template <class T>
inline
bool OrderedDAMQueue<T>::IsEmpty(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    return (!Size); 
}



template <class T>
inline
UINT16 OrderedDAMQueue<T>::GetOccupancy(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    return (Size); 
}




template <class T>
inline
UINT16 OrderedDAMQueue<T>::GetCapacity(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    return (MaxSize); 
}




template <class T>
inline
UINT16 OrderedDAMQueue<T>::GetFreeSpace(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    return (MaxSize - Size); 
}





template <class T>
INT16 OrderedDAMQueue<T>::GetNextEntryNum(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    if (!IsEmpty())
      {if (Pool[SearchPtr].Next != -1)
        {SearchPtr = Pool[SearchPtr].Next;
         return SearchPtr;
        }
       else
         return -1;
      }
    else
       return -1;
}



template <class T>
T OrderedDAMQueue<T>::GetNextEntry(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    if (!IsEmpty())
      {if (Pool[SearchPtr].Next == -1)
         return NULL;
       else
        {SearchPtr = Pool[SearchPtr].Next;
         return Pool[SearchPtr].Data;
        }
      }
    else
       return NULL;
}



template <class T>
void OrderedDAMQueue<T>::Store(T t, UINT64 key)
{
    VERIFY(!IsFull(), "OrderedDAMQueue is full\n");
    if (IsEmpty())
      {Pool[0].Data = t;
       Pool[0].Key = key;
       Pool[0].Next = -1;
       Pool[0].Prev = -1;
       Size++;
       Head = 0;
       Tail = 0;
       Middle = 0;
       FreePtr = 1;
       return;
      }
    else
      {if (Pool[FreePtr].Data != NULL)
         {for (UINT16 i = 0; i < MaxSize; i++)
            {FreePtr = (Tail + i) % MaxSize;
             if (Pool[FreePtr].Data == NULL)
                break;
            }
         } 
       Pool[FreePtr].Data = t;
       Pool[FreePtr].Key = key;
       Size++;
       // The new element is stored. Now we have to set the Next and Prev links to
       // point to the proper elements
       UINT16 Ptr = 0;
       bool Upward = false;
       // First, decide the starting point for the search
       if (key <= Pool[Head].Key)
         {Ptr = Head;
          Upward = false;
         }
       else if ((key > Pool[Head].Key) && (key <= Pool[Middle].Key))
         {if ((key - Pool[Head].Key) < (Pool[Middle].Key - key))
            {Ptr = Head;
             Upward = true;
            }
          else
            {Ptr = Middle;
             Upward = false;
            }
         }
       else if ((key > Pool[Middle].Key) && (key < Pool[Tail].Key))
         {if ((key - Pool[Middle].Key) < (Pool[Tail].Key - key))
            {Ptr = Middle;
             Upward = true;
            }
          else
            {Ptr = Tail;
             Upward = false;
            }
         }
       else if (key >= Pool[Tail].Key)
         {Ptr = Tail;
          Upward = true;
         }
       else
          ASIMERROR("Oops!, we have a problem with the order in OrderedDAMQueue!!\n"); 

       // Now, find the correct location for the new item
       if (Upward)
         {while ((Pool[Ptr].Key <= key) && (Pool[Ptr].Next != -1))
            Ptr = Pool[Ptr].Next;
            if (Pool[Ptr].Key <= key)
              {Pool[FreePtr].Next = -1;
               Pool[FreePtr].Prev = Ptr;
               Pool[Ptr].Next = FreePtr;
               Tail = FreePtr;
              }
            else
              {Pool[FreePtr].Next = Ptr;
               Pool[FreePtr].Prev = Pool[Ptr].Prev;
               Pool[Pool[Ptr].Prev].Next = FreePtr;
               Pool[Ptr].Prev = FreePtr;
              }
         }
       else
         {while ((Pool[Ptr].Key >= key) && (Pool[Ptr].Prev != -1))
            Ptr = Pool[Ptr].Prev;
          if (Pool[Ptr].Key >= key)
            {Pool[FreePtr].Next = Ptr;
             Pool[FreePtr].Prev = -1;
             Pool[Ptr].Prev = FreePtr;
             Head = FreePtr;
            }
          else
            {Pool[FreePtr].Next = Pool[Ptr].Next;
             Pool[FreePtr].Prev = Ptr;
             Pool[Pool[Ptr].Next].Prev = FreePtr;
             Pool[Ptr].Next = FreePtr;
            }
         }
       // Adjust the Middle pointer
       Middle = Head;
       for (UINT16 j = 0; j < Size/2; j++)
         Middle = Pool[Middle].Next;
       return;
      }
    ASIMERROR("OrderedDAMQueue<T>::Store() could not find a free entry\n");
}




template <class T>
void OrderedDAMQueue<T>::DelEntry(UINT16 n)
{
    VERIFY(n < MaxSize, "Index exceeds OrderedDAMQueue limits\n"); 
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    VERIFY(Pool[n].Data, "Entry does not exists\n"); 
    
    Pool[n].Data = NULL; //this should be compatible with Smart Pointers
    Size--; 
    FreePtr = n;
    if (Size)
      {if (n == Head)
         {Head = Pool[n].Next;
          Pool[Pool[n].Next].Prev = -1;      
         }
       else if (n == Tail)
         {Tail = Pool[n].Prev;
          Pool[Pool[n].Prev].Next = -1;      
         }
       else
         {Pool[Pool[n].Next].Prev = Pool[n].Prev;
          Pool[Pool[n].Prev].Next = Pool[n].Next;
         }
       if (n == Middle)
         {if (Pool[n].Next != -1)
            Middle =  Pool[n].Next;
          else
            Middle =  Tail;
         }
      }
}



template <class T>
inline
T OrderedDAMQueue<T>::GetEntry(UINT16 n)
{
    VERIFY(n < MaxSize, "Index exceeds OrderedDAMQueue limits\n"); 
    VERIFY(Pool[n].Data, "Entry 'n' is empty\n"); 
    return Pool[n].Data; 
}



template <class T>
inline
void OrderedDAMQueue<T>::ResetSearchPtr(void)
{
    VERIFY(Exists(), "OrderedDAMQueue is not initialized\n");
    SearchPtr = Head;
}



template <class T>
inline
INT16 OrderedDAMQueue<T>::GetCurrentEntryNum(void)
{
    if (!IsEmpty())
	return SearchPtr; 
    else
	return -1; 
}




template <class T>
inline
T OrderedDAMQueue<T>::GetCurrentEntry(void)
{
    if (!IsEmpty())
      {VERIFY(Pool[SearchPtr].Data, "Current Entry is empty\n"); 
       return Pool[SearchPtr].Data;
      }
    else
       return NULL; 
}


#endif //_ORDERED_DAMQUEUE_H

