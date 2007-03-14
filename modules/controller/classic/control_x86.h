/*
 *Copyright (C) 2004-2006 Intel Corporation
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
 * @author David Goodwin, Carl Beckmann
 * @brief
 */

#ifndef __CONTROL_X86_H__
#define __CONTROL_X86_H__


// ASIM modules 
#include "asim/provides/controller.h"
#include "pipe.h"
#include "knob.h"


/*CTW I hate putting a global here, but I am not sure how else to do it since I don't want to change the
  the pipetrace stuff, but I need the pipe EXISTS to return true when in quiet mode 
*/
extern bool pipetrace_quiet_mode_on;

// declare knobs we instantiate in .cpp file, so derived classes can use them too
extern Knob<bool>   histo;
extern Knob<int>    perf_max_msgs;
extern Knob<int>    perfchecker_rulemask;
extern Knob<int>    perfchecker_window;
extern Knob<void>   perf_activate_all_mods;
extern Knob<void>   perf_deactivate_all_mods;
extern Knob<void>   force;
extern Knob<void>   explain;
extern Knob<char*>  perfplot_file;
extern Knob<uint32> perfplot_interval;
extern Knob<char*>  perfplot_stats;
extern Knob<char*>  perfplot_firing_stat;
extern Knob<uint32> perfplot_start_from_end;

#if PTV_EVENTS
#define PIPE_RECORD_EVENT_TIME_MACRO4(A,B,C,D)           pipe_record_event_time(A,B,C,D)
#define PIPE_RECORD_EVENT_TIME_MACRO5(A,B,C,D,E)         pipe_record_event_time(A,B,C,D,E)
#define PIPE_RECORD_EVENT_TIME_MACRO6(A,B,C,D,E,F)       pipe_record_event_time(A,B,C,D,E,F)
#define PIPE_RECORD_EVENT_TIME_MACRO7(A,B,C,D,E,F,G)     pipe_record_event_time(A,B,C,D,E,F,G)
#define PIPE_RECORD_EVENT_TIME_MACRO8(A,B,C,D,E,F,G,H)   pipe_record_event_time(A,B,C,D,E,F,G,H)
#define PIPE_RECORD_EVENT_TIME_MACRO9(A,B,C,D,E,F,G,H,I) pipe_record_event_time(A,B,C,D,E,F,G,H,I)
#define PIPE_RECORD_EVENT_TIME_MACRO11(A,B,C,D,E,F,G,H,I,J,K) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K)
#define PIPE_RECORD_EVENT_TIME_MACRO13(A,B,C,D,E,F,G,H,I,J,K,L,M) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M)
#define PIPE_RECORD_EVENT_TIME_MACRO15(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O)

#define PIPE_RECORD_EVENT_TIME_MACRO17(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q)
#define PIPE_RECORD_EVENT_TIME_MACRO19(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S)
#define PIPE_RECORD_EVENT_TIME_MACRO21(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U)\
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U)

#define PIPE_RECORD_EVENT_TIME_MACRO23(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W)

#define PIPE_RECORD_EVENT_TIME_MACRO25(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y)

#define PIPE_RECORD_EVENT_TIME_MACRO27(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA)
#define PIPE_RECORD_EVENT_TIME_MACRO29(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC)
#define PIPE_RECORD_EVENT_TIME_MACRO31(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC,DD,EE) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC,DD,EE)




#define PIPE_RECORD_EXISTS_MACRO(A) (pipe_record_exists(A))
#define PIPE_RECORD_DATA_MACRO(A,B,C) pipe_record_data(A,B,C)
#define PIPE_OPEN_ASIM_RECORD_MACRO(A,B,C,D) pipe_open_asim_record_inst(A,B,C,D)
#define PIPE_CLOSE_RECORD_MACRO(A) pipe_close_record(A)

#else
#define PIPE_RECORD_EVENT_TIME_MACRO4(A,B,C,D)           pipe_record_event_time(A,B,C,D)
#define PIPE_RECORD_EVENT_TIME_MACRO5(A,B,C,D,E)         pipe_record_event_time(A,B,C,D,E)
#define PIPE_RECORD_EVENT_TIME_MACRO6(A,B,C,D,E,F)       pipe_record_event_time(A,B,C,D,E,F)
#define PIPE_RECORD_EVENT_TIME_MACRO7(A,B,C,D,E,F,G)     pipe_record_event_time(A,B,C,D,E,F,G)
#define PIPE_RECORD_EVENT_TIME_MACRO8(A,B,C,D,E,F,G,H)   pipe_record_event_time(A,B,C,D,E,F,G,H)
#define PIPE_RECORD_EVENT_TIME_MACRO9(A,B,C,D,E,F,G,H,I) pipe_record_event_time(A,B,C,D,E,F,G,H,I)
#define PIPE_RECORD_EVENT_TIME_MACRO11(A,B,C,D,E,F,G,H,I,J,K) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K)
#define PIPE_RECORD_EVENT_TIME_MACRO13(A,B,C,D,E,F,G,H,I,J,K,L,M) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M)
#define PIPE_RECORD_EVENT_TIME_MACRO15(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O)

#define PIPE_RECORD_EVENT_TIME_MACRO17(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q) pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q)
#define PIPE_RECORD_EVENT_TIME_MACRO19(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S)
#define PIPE_RECORD_EVENT_TIME_MACRO21(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U)\
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U)

#define PIPE_RECORD_EVENT_TIME_MACRO23(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W)

#define PIPE_RECORD_EVENT_TIME_MACRO25(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y)

#define PIPE_RECORD_EVENT_TIME_MACRO27(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA)
#define PIPE_RECORD_EVENT_TIME_MACRO29(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC)
#define PIPE_RECORD_EVENT_TIME_MACRO31(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC,DD,EE) \
        pipe_record_event_time(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,AA,BB,CC,DD,EE)




#define PIPE_RECORD_EXISTS_MACRO(A) (pipetrace_quiet_mode_on||pipe_record_exists(A))
//#define PIPE_RECORD_EXISTS_MACRO(A) (pipe_record_exists(A))
#define PIPE_RECORD_DATA_MACRO(A,B,C) pipe_record_data(A,B,C)
#define PIPE_OPEN_ASIM_RECORD_MACRO(A,B,C,D) pipe_open_asim_record_inst(A,B,C,D)
#define PIPE_CLOSE_RECORD_MACRO(A) pipe_close_record(A)
#endif


#define CFG_ARGS_MAX 64

/********************************************************************
 *
 * Interface between the controller the performance model
 */


/*
 * Exception handling in
 * Get next token from feeder cfg file.
 */
struct getword_error_dummy {};  


/*
 * Save a checkpoint file on the trigger
 */
extern void CMD_SaveCheckpoint (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Load a checkpoint file on the trigger
 */
extern void CMD_LoadCheckpoint (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Turn ptv on or off.
 */
extern void CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);


// ptv active / not
extern bool pipetrace_closed;



/********************************************************************
 *
 * The following is a singleton object class that represents the controller.
 * It inherits most of its behavior from the "classic" controller base class,
 * and just adds x86 controller specializations.
 *
 ********************************************************************/

class CONTROLLER_X86_CLASS: public CONTROLLER_CLASS {
  public:
    CONTROLLER_X86_CLASS();
    ~CONTROLLER_X86_CLASS();

    // primary entry point for the code
    int main(INT32 argc, char *argv[], char *envp[]);

    // these methods are changed from classic controller
    void CMD_SchedulerLoop (void);
    bool CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv, char **allEnvp);
    void CMD_Progress (AWB_PROGRESSTYPE type, char *args, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_EmitStats (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_ResetStats (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    
    // these methods are new since classic controller
    void DumpStats (int err_occurred);
    string getword(FILE *fp, int *linenum);
    void CMD_SaveCheckpoint (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_LoadCheckpoint (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_Perfplot (CMD_ACTIONTRIGGER trigger, UINT64 n);
    void Perfplot_Init();

    // these args handling routines have some changes in them
    void PartitionArgs   ( INT32 argc,   char *argv[]           );
    void PartitionOneArg ( INT32 argc,   char *argv[], INT32 &i );
    void Usage           ( char *exec,   FILE *file             );
    bool ParseOneEvent   ( INT32 argc,   char *argv[], INT32 &i );
    int  ParseVariables  ( char *argv[], UINT32 argc            );

    // public data:

    char *CheckPointFileName;

    // knobs control these:
    int perf_force;
    int perf_explain;

  protected:
    // command line arguments have an additional --knobs section
    UINT32 knobsArgc;
    char **knobsArgv;
    bool   knobsArgs;
};


/********************************************************************
 *
 * helper classes for command/response queues
 *
 ********************************************************************/

/**** changed classes ****/

/*
 * CMD_INIT_X86
 *
 * Initialize the performance model.
 */
typedef class CMD_INIT_X86_CLASS *CMD_INIT_X86;
class CMD_INIT_X86_CLASS : public CMD_INIT_CLASS
{
    public:
        CMD_INIT_X86_CLASS (UINT32 fc, char **fv, UINT32 pc, char **pv, char **ep):
            CMD_INIT_CLASS (fc, fv, pc, pv, ep) { }

        CMD_ACK PmAction (void);
};

/*
 * CMD_PROGRESS_X86
 *
 * Show performance model progress
 */
typedef class CMD_PROGRESS_X86_CLASS *CMD_PROGRESS_X86;
class CMD_PROGRESS_X86_CLASS : public CMD_PROGRESS_CLASS
{
    public:
        CMD_PROGRESS_X86_CLASS (AWB_PROGRESSTYPE p, char *a, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_PROGRESS_CLASS (p, a, t, c) { }

        void CmdAction (void);
};

/*
 * CMD_EMITSTATS_X86
 *
 * Emit performance model stats
 */
typedef class CMD_EMITSTATS_X86_CLASS *CMD_EMITSTATS_X86;
class CMD_EMITSTATS_X86_CLASS : public CMD_EMITSTATS_CLASS
{
    public:
        CMD_EMITSTATS_X86_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_EMITSTATS_CLASS (t, c) { }

        void CmdAction (void);
};

/*
 * CMD_RESETSTATS_X86
 *
 * Reset performance model stats
 */
typedef class CMD_RESETSTATS_X86_CLASS *CMD_RESETSTATS_X86;
class CMD_RESETSTATS_X86_CLASS : public CMD_RESETSTATS_CLASS
{
    public:
        CMD_RESETSTATS_X86_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_RESETSTATS_CLASS (t, c) { }

        void CmdAction (void);
};

/*
 * CMD_EXECUTE_X86
 *
 * Start performance model execution.
 */
typedef class CMD_EXECUTE_X86_CLASS *CMD_EXECUTE_X86;
class CMD_EXECUTE_X86_CLASS : public CMD_EXECUTE_CLASS
{
    public:
        CMD_EXECUTE_X86_CLASS (UINT64 n, UINT64 c, UINT64 i, UINT64 m, UINT64 p) :
            CMD_EXECUTE_CLASS (n, c, i, m, p) { }
        
        CMD_ACK PmAction (void);
};

/**** new classes ****/

/*
 * CMD_PERFPLOT
 *
 * Turn stats on or off.
 */
typedef class CMD_PERFPLOT_CLASS *CMD_PERFPLOT;
class CMD_PERFPLOT_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_PERFPLOT_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :CMD_WORKITEM_CLASS("PERFPLOT", t, c) { }

		void CmdAction (void);
};


/*
 * CMD_DUMPCHECKPOINT
 *
 * Dump a checkpoint of the machine
 */
typedef class CMD_DUMPCHECKPOINT_CLASS *CMD_DUMPCHECKPOINT;
class CMD_DUMPCHECKPOINT_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_DUMPCHECKPOINT_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("DUMPCHECKPOINT", t, c) { }

        void CmdAction (void);
};


/*
 * CMD_LOADCHECKPOINT
 *
 * Dump a checkpoint of the machine
 */
typedef class CMD_LOADCHECKPOINT_CLASS *CMD_LOADCHECKPOINT;
class CMD_LOADCHECKPOINT_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_LOADCHECKPOINT_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("LOADCHECKPOINT", t, c) { }

        void CmdAction (void);
};


/*
 * CMD_PTV
 *
 * Turn ptv on or off.
 */
typedef class CMD_PTV_CLASS *CMD_PTV;
class CMD_PTV_CLASS : public CMD_WORKITEM_CLASS
{
    protected:
        bool on;
        
    public:
        CMD_PTV_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("PTV", t, c), on(o) { }

        CMD_ACK PmAction (void);
};


#endif /* __CONTROL_X86_H__ */
