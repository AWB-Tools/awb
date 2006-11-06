#
# Copyright (C) 2003-2006 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#

# makefile written for gnu make
CXX		= g++
SRC		= ./src
CPPFLAGS	= -I$(SRC)
DEBUG		= -g
OPTIMIZE	= -O2
GCCWARN		= -Wall -Wstrict-prototypes
CXXFLAGS	= $(DEBUG) $(GCCWARN) $(OPTIMIZE) $(INCLUDES)

LIB		= ./libXmlRpc.a

# Add your system-dependent network libs here. These are
# only used to build the tests (your application will need them too).
# Linux: none
# Solaris: -lsocket -lnsl
#SYSTEMLIBS	= -lsocket -lnsl
SYSTEMLIBS	=
LDLIBS		= $(LIB) $(SYSTEMLIBS)

OBJ		= $(SRC)/XmlRpcClient.o $(SRC)/XmlRpcDispatch.o \
		$(SRC)/XmlRpcServer.o $(SRC)/XmlRpcServerConnection.o \
		$(SRC)/XmlRpcServerMethod.o $(SRC)/XmlRpcSocket.o $(SRC)/XmlRpcSource.o \
		$(SRC)/XmlRpcUtil.o $(SRC)/XmlRpcValue.o

all:		$(LIB) tests

$(LIB):		$(OBJ)
		$(AR) $(ARFLAGS) $(LIB) $(OBJ)


tests:		$(LIB)
		cd test && $(MAKE) CXX=$(CXX) CXXFLAGS="$(CXXFLAGS)" SYSTEMLIBS="$(SYSTEMLIBS)"

doc doxygen:
		cd src && doxygen Doxyfile

clean:
		rm -f $(SRC)/*.o
		rm -f $(SRC)/*~
		rm -f $(LIB)
		cd test && $(MAKE) clean

