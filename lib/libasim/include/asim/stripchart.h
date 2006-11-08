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
 * @file
 * @author Isaac Hernandez
 * @brief ASIM Strip Charts
 */

#ifndef _STRIP_CHART_
#define _STRIP_CHART_

// generic (C++)
#include <string>
#include <iostream>
#include <fstream>

// generic (C)
#include <stdio.h>

// ASIM core
#include "asim/syntax.h"

using namespace std;

/*
 * At this level CPU_THREADS isn't available. We define the number of
 * threads with an alternative define.
 *
 * Current stripchart code stores statically the different stripcharts
 * and I define a maximun of 100. If you need extra stripcharts you only
 * must change MAX_STRIP_CHART define.
 *
 */

#define MAX_STRIP_CHART 4000
#define THREADS 32

/**
 * Class ASIM_STRIP_TABLE_CLASS
 * This class contains the data related to one stripchart with 4 threads.
 * This class isn't accesible out there.
 */
class ASIM_STRIP_NODE_CLASS
{
  private:
    string description; ///< description header of the stripchart
    UINT64 frequency;   ///< sample frequency 
    UINT64 *data;       ///< pointer to data generated by the performance model
    UINT64 TPU;         ///< number of TPUs used
    UINT64 max_elems;   ///< ? documentation ?

  protected:

  public:
    // constructors / destructors
    ASIM_STRIP_NODE_CLASS();
    ~ASIM_STRIP_NODE_CLASS();

    // accessors / modifiers
    UINT64 *Dump(void) const;
    UINT64 Frequency(void) const;
    const string & Header(void) const;
    UINT64 Threads(void) const;

    void Reset(void);
    void AssignValues(const string & description, const UINT64 frequency,
        UINT64 * const data, const UINT64 threads, const UINT64 max_elems,
        const UINT32 cpunum);
};

typedef class ASIM_STRIP_NODE_CLASS * ASIM_STRIP_NODE;

/**
 * Class ASIM_STRIP_CHART_CLASS
 * This class contains all the stripcharts and general status of stripchart
 * registered on performance model.
 */
class ASIM_STRIP_CHART_CLASS
{
  private:
    ASIM_STRIP_NODE_CLASS table[MAX_STRIP_CHART];   ///< ? documentation ?
    UINT64 actives;           ///< Number of stripchart registered
    UINT64 general_frequency; ///< greatest common divisor of all frequencies
    ofstream out;             ///< output file
    UINT64 next_position;     ///< ? documentation ?

    UINT64 version;
    UINT64 lines;
    UINT64 strips;
    UINT64 blocks;
    UINT64 markers;
    UINT64 bytes_per_line;

  protected:
    
  public:
    // constructors / destructors
    ASIM_STRIP_CHART_CLASS();
    ~ASIM_STRIP_CHART_CLASS();

    void RegisterStripChart(const string & description, const UINT64 frequency,
        UINT64 * const data, const UINT64 threads=1,
        const UINT64 max_elems=0, const UINT32 cpunum=UINT32_MAX);
    void HeadDump(void);
    void Dump(const UINT64 cycle);
    void DumpRAWString(const string & str);
    void WriteCounters();
};

typedef class ASIM_STRIP_CHART_CLASS * ASIM_STRIP_CHART;

#endif // _STRIP_CHART_