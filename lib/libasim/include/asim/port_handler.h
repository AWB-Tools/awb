/*****************************************************************************
 *
 *
 * @author Oscar Rosell
 *
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

#ifndef PORT_HANDLER_H
#define PORT_HANDLER_H

#include "asim/syntax.h"

/**********************************************************
*
*   Port Handlers can be used when we want to use ports
*   polymorphically. Normal ports don't allow polymorphism
*   for performance reasons but sometimes code can be much
*   simpler this way.
*
**********************************************************/

template<class T>
class WRITE_PORT_HANDLER_BASE
{
  public:
    WRITE_PORT_HANDLER_BASE(){}; 
    virtual ~WRITE_PORT_HANDLER_BASE(){};
    virtual bool Write(T& msg, UINT64 cycle) = 0;
};

template <class WRITE_PORT, class Type>
class WRITE_PORT_HANDLER : public WRITE_PORT_HANDLER_BASE<Type>
{
  public:
    WRITE_PORT_HANDLER(WRITE_PORT& port)
      : port_(port)
    { }
    virtual bool Write(Type& msg, UINT64 cycle)
    {
        return port_.Write(msg, cycle);
    }
  private:
    WRITE_PORT& port_;
};

template<class T>
class READ_PORT_HANDLER_BASE
{
  public:
    READ_PORT_HANDLER_BASE(){};
    virtual ~READ_PORT_HANDLER_BASE(){};
    virtual bool Read(T& msg, UINT64 cycle) = 0;
};

template <class READ_PORT, class Type>
class READ_PORT_HANDLER : public READ_PORT_HANDLER_BASE<Type>
{
  public:
    READ_PORT_HANDLER(READ_PORT& port)
      : port_(port)
    { }
    virtual bool Read(Type& msg, UINT64 cycle)
    {
        return port_.Read(msg, cycle);
    }
  private:
    READ_PORT& port_;
};

#endif // PORT_HANDLER_H
