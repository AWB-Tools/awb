/*****************************************************************************
 *
 * @brief Chip
 *
 * @author Sailashri Parthasarathy (based on Mark Charney's original implementation)
 *
 * Copyright (c) 2003 Intel Corporation, all rights reserved.
 * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
 * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
 *
 *****************************************************************************/

// generic
#include <signal.h>
#include <unistd.h>
#include <sstream>

// ASIM core
#include "asim/trace.h"
#include "asim/trackmem.h"
#include "asim/cmd.h"
//#include "asim/ioformat.h"

// ASIM public modules
#include "asim/provides/instfeeder_interface.h"
#include "asim/provides/chip.h"


ASIM_CHIP_CLASS::ASIM_CHIP_CLASS(
    ASIM_MODULE parent,             // CONS
    const char* const name)
    : ASIM_MODULE_CLASS(parent, name)
{
    NewClockDomain("CORE_CLOCK_DOMAIN", (float) 4);

    myCpu = new ASIM_CPU_CLASS*[TOTAL_NUM_CPUS];
    for(int i = 0; i < TOTAL_NUM_CPUS; i++)
    {
        myCpu[i] = new ASIM_CPU_CLASS(this, "CPU",i);
    }
}

ASIM_CHIP_CLASS::~ASIM_CHIP_CLASS()
{
    for(int i = 0; i < TOTAL_NUM_CPUS; i++)
    {
        delete myCpu[i];
    }
    delete[] myCpu;
}


void 
ASIM_CHIP_CLASS::Clock(
    const UINT64 cycle)
{
    for(int i = 0; i < TOTAL_NUM_CPUS; i++)
    {
        myCpu[i]->Clock(cycle);
    }
}

