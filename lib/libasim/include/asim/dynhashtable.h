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

/**
 * @file
 * @author Shubu Mukherjee
 * @brief A bucket-based hash table that can be grown dynamically. 
 */


#ifndef _DYNHASHTABLE_
#define _DYNHASHTABLE_

// generic
#include <memory.h>
#include <math.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"

template <class KeyType, class T, unsigned Table_Size>
class DynHashTable 
{
  public:
    class HashItem 
    {
      public:
	HashItem(KeyType k, HashItem *n = NULL, T *i = NULL)
	    : _next(n), _item(i)
	{

	    // do ___NOT___ move this into the parameter list, because
	    // LongUint somehow does not work for it.  It probably uses
	    // an operator different from "="

	    _key = k;	
	}

	~HashItem() {}
	KeyType  _key;
	HashItem *_next;
	T	 *_item;
    };


    class Iter 
    {
      public:
	T *item() 		{ return _hashItem->_item; }
	KeyType key()		{ return _hashItem->_key; }
	operator bool()		{ return (_hashItem != NULL); }

	void operator ++()
	{
	    _hashItem = _hashItem->_next;

	    if (_hashItem == NULL)
	    {
		advanceToNextBucket();
	    }
	}

	Iter(DynHashTable *t)
	    : _tblObj(t), _index(0), _hashItem(t->tbl[0])
	{
	    if (_hashItem == NULL)
	    {
		advanceToNextBucket();
	    }
	}

      private:

	void advanceToNextBucket();

	DynHashTable	*_tblObj;
	unsigned	 _index;
	HashItem	*_hashItem;
    };

    T *&Lookup(KeyType key);
    T *Remove(KeyType key);

    DynHashTable()
	: _numEntries(0)
    {
	memset(tbl, 0, sizeof(tbl));
    }

    UINT64 NumEntries()
    {
	return _numEntries; 
    }

    HashItem *tbl[Table_Size];

  private:
    INT64 _numEntries; 
};

// Lookup an entry.  If entry missing, then create new entry and return
// pointer to entry. 
template <class KeyType, class T, unsigned Table_Size>
T *&DynHashTable<KeyType, T, Table_Size>::Lookup(KeyType key)
{
    HashItem **bucketPtr = tbl + (key % Table_Size);
    HashItem *head = *bucketPtr;

    if (head == NULL)
    {
	++_numEntries;
	return (*bucketPtr = new HashItem(key))->_item;
    }
    else if (head->_key == key)
    {
	return head->_item;
    }
    else
    {
	HashItem **prevPtrPtr = bucketPtr;
	HashItem **ptrPtr = &(head->_next);
	HashItem *ptr = *ptrPtr;

	while (ptr != NULL && ptr->_key != key)
	{
	    prevPtrPtr = ptrPtr;
	    ptrPtr = &(ptr->_next);
	    ptr = *ptrPtr;
	}

	if (ptr == NULL)
	{
	    // not found... prepend to chain
	    ++_numEntries;
	    return (*bucketPtr = new HashItem(key, head))->_item;
	}
	else
	{
	    // found... move to head
	    *ptrPtr = ptr->_next;
	    ptr->_next = head;
	    return (*bucketPtr = ptr)->_item;
	}
    }
}

// Remove entry from hashtable
template <class KeyType, class T, unsigned Table_Size>
T *DynHashTable<KeyType, T, Table_Size>::Remove(KeyType key)
{
    HashItem **bucketPtr = tbl + (key % Table_Size);
    HashItem *head = *bucketPtr;

    if (head == NULL)
    {
	return (T *)NULL;
    }
    else if (head->_key == key)
    {
	T *p;
	*bucketPtr = head->_next;
	p = head->_item;
	--_numEntries;
	ASSERTX(_numEntries >= 0); 
	delete head;
	return p;
    }
    else
    {
	HashItem **prevPtrPtr = bucketPtr;
	HashItem **ptrPtr = &(head->_next);
	HashItem *ptr = *ptrPtr;

	while (ptr != NULL && ptr->_key != key)
	{
	    prevPtrPtr = ptrPtr;
	    ptrPtr = &(ptr->_next);
	    ptr = *ptrPtr;
	}

	if (ptr == NULL)
	{
	    return (T *)NULL;
	}
	else
	{
	    T *p;

	    // found... move to head
	    p = ptr->_item;
	    *ptrPtr = ptr->_next;

	    --_numEntries;
	    ASSERTX(_numEntries >= 0); 

	    delete ptr;
	    return p;
	}
    }
}

template <class KeyType, class T, unsigned Table_Size>
void DynHashTable<KeyType, T, Table_Size>::Iter::advanceToNextBucket()
{
    ASSERTX(_hashItem == NULL);

    while (++_index < Table_Size)
    {
	if (_tblObj->tbl[_index] != NULL)
	{
	    _hashItem = _tblObj->tbl[_index];
	    return;
	}
    }
}


#endif 

