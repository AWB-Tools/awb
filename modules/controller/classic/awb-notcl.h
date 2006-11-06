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
 * @author David Goodwin
 * @brief Driver for "Architects Workbench", awb
 */

#ifndef _AWB_
#define _AWB_

// ASIM core
#include "asim/syntax.h"
#include "asim/stateout.h"

extern char *awbCmdFile;
extern char *overrideWorkbench; /* NOT USED */

/*
 * Types of progress reportable to AWB_Progress
 */

enum AWB_PROGRESSTYPE { AWBPROG_CLEARCYCLE, AWBPROG_CLEARINST, AWBPROG_CLEARMACROINST,
                        AWBPROG_CLEARPACKET, AWBPROG_CLEARNANOSECOND,
                        AWBPROG_EXIT, AWBPROG_START, AWBPROG_STOP,  AWBPROG_MACROINST,
                        AWBPROG_CYCLE, AWBPROG_INST, AWBPROG_PACKET, AWBPROG_NANOSECOND,
                        AWBPROG_THREADBEGIN, AWBPROG_THREADEND,
                        AWBPROG_THREADUNBLOCK, AWBPROG_THREADBLOCK,
                        AWBPROG_EVENTOVERFLOW };

#define AWB_PROGRESSSTRS { "clearcycle", "clearinst","clearmacroinst", "clearpacket", "clearnanosecond", \
                           "exit", "start", "stop", "macroinst",\
                           "cycle", "inst", "packet", "nanosecond", \
                           "threadbegin", "threadend", \
                           "threadunblock", "threadblock", "eventoverflow" }

/*
 * Interface to allow controller thread to control awb.
 */
extern void AWB_Initialize();
extern void AWB_Activate();

extern void AWB_Progress (AWB_PROGRESSTYPE type,
                          string args = "");
extern void AWB_InformProgress (void);

extern void AWB_Exit (void);

void PmScheduleStart();
void PmScheduleStopNow();
void PmScheduleStopNanosecond(UINT64 stopTime);
void PmScheduleStopCycle(UINT64 stopTime);
void PmScheduleStopInst(UINT64 stopCount);
void PmScheduleStopMacroInst(UINT64 stopCount);

void PmScheduleExitNow();

void
PmScheduleProgress(string& type,
                   string& period);

void
PmScheduleThread(string& desc,
                 string& trigger,
                 UINT64 time);
void
PmUnscheduleThread(string& desc,
                   string& trigger,
                   UINT64 time);
void
PmScheduleSkipThread(string& arg_thread,
                     UINT64 insts,
                     INT32  markerID,
                     string& arg_action,
                     UINT64 time);

UINT64 PmCycle();
UINT64 PmCycle(unsigned int cpunum);
UINT64 PmCommitted(unsigned int cpunum);
UINT64 PmGlobalCommitted();
unsigned int PmNumCpus();
UINT64 PmCommittedMarkers();
void PmCommittedWatchMarker(INT32 markerID);
void PmBeginDrain();
void PmEndDrain();
UINT64 PmReceivedPackets();

void PmStateDump(string& filename);
void PmStateUnsuspend(string& thread_desc);
void PmStateSuspend(string& thread_desc);

void PmEventOn();
void PmEventOff();
void PmEventFilename(string& filename);

void
PmMarkerClear( string& desc,
               INT32 markerID,
               string& subcmd,
               string& args);
void
PmMarkerSet( string& desc,
             INT32 markerID,
             string& subcmd,
             string& arg1,
             string& arg2);

#endif /* _AWB_ */
//Local Variables:
//pref: "awb.cpp"
//End:
