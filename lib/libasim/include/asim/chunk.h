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
 * @author Nathan Binkert
 * @brief
 */

#ifndef _CHUNK_
#define _CHUNK_

template<class T, int N>
class Chunk
{
private:
  struct {
    T Data;
    bool Valid;
  } Array[N];
  
  const T Dummy;

  int Used;

public:
  Chunk();
  Chunk(const Chunk<T,N>& chunk);
  ~Chunk();

  int GetUsed();
  int GetFree();
  int GetMaxSize();
  bool IsFull();
  bool IsEmpty();

  int Add(const T& data);
  bool Remove(const T& data);
  bool Remove(int index);

  T& operator[](int index);
  const Chunk<T,N>& operator=(const Chunk<T,N>& chunk);
};

template<class T, int N>
inline
Chunk<T,N>::Chunk() : Dummy(T())
{
    Used = 0;
    for (int count = 0; count < GetMaxSize(); count++)
	Array[count].Valid = false;
}

template<class T, int N>
inline
Chunk<T,N>::Chunk(const Chunk<T,N>& chunk) : Dummy(T())
{ operator=(chunk); }

template<class T, int N>
inline
Chunk<T,N>::~Chunk()
{}

template<class T, int N>
inline int
Chunk<T,N>::GetUsed()
{ return Used; }

template<class T, int N>
inline int
Chunk<T,N>::GetMaxSize()
{ return N; }

template<class T, int N>
inline int
Chunk<T,N>::GetFree()
{ return GetMaxSize() - GetUsed(); }

template<class T, int N>
inline bool
Chunk<T,N>::IsFull()
{ return GetUsed() == GetMaxSize(); }

template<class T, int N>
inline bool
Chunk<T,N>::IsEmpty()
{ return GetUsed() == 0; }

template<class T, int N>
inline int
Chunk<T,N>::Add(const T& data)
{
  if (GetFree() == 0)
    return -1;

  int count = 0;
  while (Array[count].Valid)
    ++count;

  Array[count].Data = data;
  Array[count].Valid = true;
  ++Used;

  return count;
}

template<class T, int N>
inline bool
Chunk<T,N>::Remove(const T& data)
{
  if (GetFree() == 0)
    return false;

  int count = 0;
  while (!Array[count].Valid || Array[count].Data != data)
    if (++count >= GetMaxSize())
      return false;

  Array[count].Valid = false;
  Array[count].Data = Dummy;
  --Used;

  return true;
}

template<class T, int N>
inline bool
Chunk<T,N>::Remove(int index)
{
  if (index < 0 || index >= GetMaxSize())
    return false;

  ASSERTX(Array[index].Valid);
  Array[index].Valid = false;
  Array[index].Data = Dummy;
  --Used;

  return true;
}

template<class T, int N>
inline T&
Chunk<T,N>::operator[](int index)
{
    ASSERTX(index >= 0 && index < GetMaxSize());
    return Array[index].Data;
}

template<class T, int N>
inline const Chunk<T,N>&
Chunk<T,N>::operator=(const Chunk<T,N>& chunk)
{
  for (int count = 0; count < GetMaxSize; count++) {
    Array[count].Valid = chunk.Array[count].Valid;
    if (Array[count].Valid)
      Array[count].Data = chunk.Array[count].Data;
  }

  Used = chunk.Used;
}
#endif //_CHUNK_
