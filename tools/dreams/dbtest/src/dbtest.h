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

/**
  * @file
  * @brief
  */

#ifndef _DRALDBTEST_H
#define _DRALDBTEST_H

// QT
//#include <qstring.h>

// STD C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// DreamsDB Library
#include <asim/DralDB.h>

int main (int argc, char** argv);
void dumpSyntax(char* binname);
void checkParameters(int argc, char** argv);
bool parseParameters(int argc, char** argv);
bool parseNextParameter(int argc, char** argv, int *idx);
void initGlobals();
void applyConf();
void applyQueries();
void openfile();
void loadhdr();
void readAll();
void applyDumps();
void closefile();
bool decodeQuery(QString query,INT32* itemid,QString* tag,UINT32* cycle);

#endif

