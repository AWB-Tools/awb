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

bool stripsOn = false;
char stripFile[128] = "strips.stb";

// generic (C++)
#include <string>
#include <sstream>

// ASIM core
#include "asim/stripchart.h"
#include "asim/mesg.h"
#include "asim/ioformat.h"

/**
 * GCD() and GCD2() are two recursive functions to calculate Greatest
 * Common Divisor.  The efficiency of this routine is not crucial
 * because the system only call it when performance model register an
 * stripchart.
 */
UINT64 GCD2(const UINT64 a, const UINT64 b)
{
    if(b==0){
        return(a);
    } else {
        return GCD2(b,a%b);
    }
}

UINT64 GCD(const UINT64 a, const UINT64 b)
{
    if(a<b) {
        return GCD2(b,a);
    } else {
        return GCD2(a,b);
    }
}

//----------------------------------------------------------------------------
// class ASIM_STRIP_NODE
//----------------------------------------------------------------------------

/**
 * Default constructor.
 */
ASIM_STRIP_NODE_CLASS::ASIM_STRIP_NODE_CLASS()
{
    // nothing
}

/**
 * Default destructor.
 */
ASIM_STRIP_NODE_CLASS::~ASIM_STRIP_NODE_CLASS()
{
    // nothing 
}

/**
 * This routine puts the values desired by user. By default we use 4 THREADS
 * and a maximun number of elements of 0 (no maximum defined).
 */
void
ASIM_STRIP_NODE_CLASS::AssignValues(
    const string & description, ///< ? documentation ?
    const UINT64 frequency,     ///< ? documentation ?
    UINT64 * const data,        ///< ? documentation ?
    const UINT64 threads,       ///< ? documentation ?
    const UINT64 max_elems,     ///< ? documentation ?
    const UINT32 cpunum)        ///< ? documentation ?
{
    int i;
    int j;

    if (cpunum == UINT32_MAX) {
        this->description = description;
    } else {
        ostringstream os;
        os << description << " cpu " << fmt("3", cpunum);
        this->description = os.str();
    }

    //
    // In order to simplify the parsing code in stripchart viewer we
    // remove the space character.
    //
    for (string::iterator it = this->description.begin();
         it != this->description.end();
         it++)
    {
        if (*it == ' ') {
            *it = '_';
        }
    }
    ASSERTX(this->description.find(" ") == string::npos);

    this->frequency=frequency;
    this->data=data;
    this->TPU=threads;
    this->max_elems=max_elems;
}

/**
 * This routine returns a pointer to the data struct containing TPU UINT64.
 */
UINT64 * ///< @returns ? documentation ?
ASIM_STRIP_NODE_CLASS::Dump()
const
{
    return(this->data);
}

/**
 * Puts all the elements of array data to value 0.
 */
void
ASIM_STRIP_NODE_CLASS::Reset()
{
    if(stripsOn==false) return;

    for(UINT64 i=0;i<this->TPU;i++) {
        this->data[i]=0;
    }
}

/**
 * Accessor for frequency requested.
 */
UINT64 ///< @returns ? documentation ?
ASIM_STRIP_NODE_CLASS::Frequency()
const
{
    return(this->frequency);
}

/**
 * Accessor for the header composed by the description provided by
 * performance model.
 */
const string & ///< @returns ? documentation ?
ASIM_STRIP_NODE_CLASS::Header()
const
{
    return(this->description);
}

/**
 * Return data array lentht.
 */
UINT64 ///< @returns ? documentation ?
ASIM_STRIP_NODE_CLASS::Threads()
const
{
    return(this->TPU);
}

//----------------------------------------------------------------------------
// class ASIM_STRIP_CHART
//----------------------------------------------------------------------------

/**
 * Default constructor. It opens the strip_chart file and reset
 * the number of active stripcharts.
 */
ASIM_STRIP_CHART_CLASS::ASIM_STRIP_CHART_CLASS()
{
    actives=0;
    next_position=2;
    out.open(stripFile, ios::out | ios::trunc | ios::binary);
    ASSERT (out, "Can not open strip chart output file");
    lines=strips=blocks=markers=bytes_per_line=0;
    version=11;
    WriteCounters();
}

/**
 * Default destructor. It only close the opened file.
 */
ASIM_STRIP_CHART_CLASS::~ASIM_STRIP_CHART_CLASS()
{
    out.close();
}

void
ASIM_STRIP_CHART_CLASS::WriteCounters()
{
    bytes_per_line=strips*sizeof(UINT64);

    out.seekp(0,ios::beg);
    out.write((char *)&version,sizeof(version));
    out.write((char *)&lines,sizeof(lines));
    out.write((char *)&strips,sizeof(strips));
    out.write((char *)&blocks,sizeof(blocks));
    out.write((char *)&markers,sizeof(markers));
    out.write((char *)&bytes_per_line,sizeof(bytes_per_line));
    out.seekp(0,ios::end);

}

/**
 * This routine is called to register an stripchart. User
 * must provide a descripcion, a frequency, number of threads,
 * maximum number of elements and data array with the value. 
 * By default if number of threads or maximum number of elements
 * every cycle is not provided we assume no maximum (0 value) and
 * 4 threads. The data array must have length equal to 4 if no alternative
 * threads value is provided.
 */
void
ASIM_STRIP_CHART_CLASS::RegisterStripChart(
    const string & description, ///< ? documentation ?
    const UINT64 frequency,     ///< ? documentation ?
    UINT64 * const data,        ///< ? documentation ?
    const UINT64 threads,       ///< ? documentation ?
    const UINT64 max_elems,     ///< ? documentation ?
    const UINT32 cpunum)        ///< ? documentation ?
{
    //
    // If code breaks at this assert you are adding
    // extra stripcharts and you must increase MAX_STRIP_CHART
    // to allow the new value.
    //
    ASSERTX(actives<MAX_STRIP_CHART);

    //
    // Takes an stripchart an assign the values.
    //
    table[actives].AssignValues(description,frequency,data,threads,max_elems,
        cpunum);
    general_frequency=actives==0? frequency : GCD(general_frequency,frequency);
    actives++;

    //
    // Prints the file header with stripchart information
    // to help the graphical viewer to decode the stripcharts.
    //
    out.write((char *)&next_position,sizeof(next_position));
    out.write((char *)(table[actives-1].Header().c_str()),table[actives-1].Header().length());
    for(UINT32 i=0;i<100-table[actives-1].Header().length();i++) out.write(" ",1);
    out.write((char *)&threads,sizeof(threads));
    out.write((char *)&frequency,sizeof(frequency));
    out.write((char *)&general_frequency,sizeof(general_frequency));
    out.write((char *)&max_elems,sizeof(max_elems));
    next_position+=threads;

    blocks++;
    strips+=threads;
    WriteCounters();
}

/**
 * Puts into output file the sample global frequency.
 */
void
ASIM_STRIP_CHART_CLASS::HeadDump()
{
    out.flush();
}

/**
 * This routine takes the data assigned to all the stripcharts and
 * puts this data into output file.
 */
void
ASIM_STRIP_CHART_CLASS::Dump(
    const UINT64 cycle) ///< ? documentation ?
{
    UINT64 *tmp_data;
    UINT64 head_id=0;

    if(stripsOn==false) return;

    if(actives==0) {
        return;
    }

    if((cycle % general_frequency)==0) {
        out.write((char *)&head_id,sizeof(head_id));
        out.write((char *)&cycle,sizeof(cycle));
        for(UINT32 i=0; i<actives;i++) {
            tmp_data=table[i].Dump();
            for(UINT64 j=0; j<table[i].Threads();j++){
        	out.write((char *)&(tmp_data[j]),sizeof(tmp_data[j]));
            }
            table[i].Reset();
        }
        lines++;
        WriteCounters();
        out.flush();
    }
}


void
ASIM_STRIP_CHART_CLASS::DumpRAWString(
    const string & str) ///< ? documentation ?
{
    UINT64 bytes=8;
    UINT64 head_id=1;

    if(stripsOn==false) return;

    for(UINT32 i=0; i<actives;i++) {
        bytes=bytes+(table[i].Threads()*8);
    }
    
    
    out.write((char *)&head_id,sizeof(head_id));
    out.write(str.c_str(),str.length());
    for(UINT32 i=0;i<bytes-str.length();i++) out.write(" ",1);
    lines++;
    markers++;
    WriteCounters();
    out.flush();
}

