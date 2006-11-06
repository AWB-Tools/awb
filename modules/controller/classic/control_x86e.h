/*
 *Copyright (C) 2006 Intel Corporation
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
 * @brief
 */

#ifndef _AWBCMD_
#define _AWBCMD_


// ASIM core
#include "asim/syntax.h"
#include "asim/stateout.h"

// ASIM modules 
#include "asim/provides/basesystem.h" // non-hierarchical module include - BAD
#include "asim/provides/awb_stub.h"
#include "pipe.h"


/*CTW I hate putting a global here, but I am not sure how else to do it since I don't want to change the
  the pipetrace stuff, but I need the pipe EXISTS to return true when in quiet mode 
*/
extern bool pipetrace_quiet_mode_on;


#if PTV_EVENTS

#define PIPE_RECORD_EVENT_TIME_MACRO(...)                pipe_record_event_time(__VA_ARGS__)

#define PIPE_RECORD_EXISTS_MACRO(A) (pipe_record_exists(A))
#define PIPE_RECORD_DATA_MACRO(A,B,C) pipe_record_data(A,B,C)
#define PIPE_OPEN_ASIM_RECORD_MACRO(A,B,C,D) pipe_open_asim_record_inst(A,B,C,D)
#define PIPE_CLOSE_RECORD_MACRO(A) pipe_close_record(A)

#else

#define PIPE_RECORD_EVENT_TIME_MACRO(...)                pipe_record_event_time(__VA_ARGS__)

#define PIPE_RECORD_EXISTS_MACRO(A) (pipetrace_quiet_mode_on||pipe_record_exists(A))
//#define PIPE_RECORD_EXISTS_MACRO(A) (pipe_record_exists(A))
#define PIPE_RECORD_DATA_MACRO(A,B,C) pipe_record_data(A,B,C)
#define PIPE_OPEN_ASIM_RECORD_MACRO(A,B,C,D) pipe_open_asim_record_inst(A,B,C,D)
#define PIPE_CLOSE_RECORD_MACRO(A) pipe_close_record(A)

#endif


enum EXIT_CONDITION {THREAD_END};

extern void CheckExitConditions(EXIT_CONDITION ec);

#define CFG_ARGS_MAX 64
/*******************************************************************/

/*
 * Times when actions can occur.
 *
 * ACTION_NANOSECOND_ONCE   : Action occurs once at the specified nanosecond
 * ACTION_NANOSECOND_PERIOD : Action occurs every 'n' nanoseconds
 * ACTION_CYCLE_ONCE   : Action occurs once at the specified cycle
 * ACTION_CYCLE_PERIOD : Action occurs every 'n' cycles
 * ACTION_INST_ONCE    : Action occurs once in cycle after specified
 *                       number of instructions are committed
 * ACTION_INST_PERIOD  : Action occurs every 'n' committed instrs
 * ACTION_MACROINST_ONCE    : Action occurs once in cycle after specified
 *                       number of macro instructions are committed
 * ACTION_MACROINST_PERIOD  : Action occurs every 'n' committed macro instrs

 * ACTION_NOW          : Action occurs in the next cycle
 * ACTION_NEVER        : Action never occurs
 *
 */
enum CMD_ACTIONTRIGGER {ACTION_NANOSECOND_ONCE, ACTION_NANOSECOND_PERIOD,
                        ACTION_CYCLE_ONCE, ACTION_CYCLE_PERIOD,
                        ACTION_INST_ONCE, ACTION_INST_PERIOD,
                        ACTION_MACROINST_ONCE, ACTION_MACROINST_PERIOD,
                        ACTION_NOW, ACTION_NEVER, ACTION_PACKET_ONCE};

/********************************************************************
 *
 * Interface between the controller the performance model
 */


/*
 * Dump PTV/STATS
 */
extern void DumpStats (int err_occurred);

extern void ArchlibSupport_SetThreads(uint32 np, uint32 nc, uint32 nt);


/*
 * Get next token from feeder cfg file.
 */
string getword(FILE *fp, int *linenum);
struct getword_error_dummy {};  

/*
 * Controller scheduler loop. Maintains a list of the next "action" that
 * needs to be performed by the performance model or by the controller
 * itself and advances through the list by running the performance model
 * forward.
 */
extern void CMD_SchedulerLoop (void);

/*
 * Initialize the performance model. Performance model
 * returns false if anything goes wrong.
 */
extern bool CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv, char **knobsArgv, char **allEnvp);

/*
 * Print command-line usage information for feeder and system.
 */
extern void CMD_Usage (FILE *file);

/*
 * Return a list of all the exposed performance model state. This cannot be
 * called until the model has been initialized, or an error will occur.
 */
extern ASIM_STATELINK CMD_StateList (void);

/*
 * Start and stop the performance model running.
 */
extern void CMD_Start (void);
extern void CMD_Stop (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Show performance model progress
 */
extern void CMD_Progress (AWB_PROGRESSTYPE type, char *args, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Emit statistics file every <n> cycles
 */
extern void CMD_EmitStats (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Reset statistics file every <n> cycles
 */
extern void CMD_ResetStats (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Exit performance model.
 */
extern void CMD_Exit (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Turn debugging on or off.
 */
extern void CMD_Debug (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Turn tracing on or off.
 */
extern void CMD_Trace (TRACEABLE_DELAYED_ACTION act, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Save a checkpoint file on the trigger
 */
extern void CMD_SaveCheckpoint (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Load a checkpoint file on the trigger
 */
extern void CMD_LoadCheckpoint (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);


/*
 * Turn stats on or off.
 */
extern void CMD_Stats (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Turn ptv on or off.
 */
extern void CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);


/*
 * Turn events on or off.
 */
extern void CMD_Events (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Turn stripcharts on or off.
 */
extern void CMD_Stripchart (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Turn profiling on or off.
 */
extern void CMD_Profile (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Called by feeder to inform the controller that a new thread is
 * available to be scheduled on the performance model, or that a thread
 * has ended, and can no longer be fetched.
 */
extern void CMD_ThreadBegin (ASIM_THREAD thread);
extern void CMD_ThreadEnd (ASIM_THREAD thread);

/*
 * Called by feeder to inform the controller that a thread needs to be
 * blocked in the performance model, or that a thread
 * needs to be un-blocked.
 */
extern void CMD_ThreadBlock (ASIM_THREAD thread, ASIM_INST inst);
extern void CMD_ThreadUnblock (ASIM_THREAD thread);

/*
 * Skip 'n' instructions in 'thread'. If 'thread' is NULL then all threads
 * are skipped 'n' instructions each.
 */
extern void CMD_SkipThread (ASIM_THREAD thread, UINT64 insts, INT32 markerID,
                            CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

/*
 * Schedule a thread on the performance model, or remove a thread from the
 * performance model.
 */
extern void CMD_ScheduleThread (ASIM_THREAD thread, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
extern void CMD_UnscheduleThread (ASIM_THREAD thread, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);

// ptv active / not
extern bool pipetrace_closed;

/********************************************************************
 *
 * Global system object.
 *
 ********************************************************************/

extern ASIM_SYSTEM asimSystem;




/********************************************************************
 *
 * The following are the classes for the objects that are queued on
 * the work queues and for the object used to acknowledge back to 
 * the queuee that the work is done.
 *
 * These classes have action effects both on at the controller level
 * and at the performance model level....
 *
 * Note: These are probably really private, so don't belong here.
 *
 ********************************************************************/


typedef class CMD_WORKLIST_CLASS *CMD_WORKLIST;
typedef class CMD_WORKITEM_CLASS *CMD_WORKITEM;
typedef class CMD_ACK_CLASS *CMD_ACK;

// ASIM local module
#include "schedule.h"


/********************************************************************/


/********************************************************************
 * CMD_ACK
 *
 * Acknowledgement from the performance model to the controller.
 *
 *******************************************************************/




class CMD_ACK_CLASS
{
    private:
        /*
         * Work item this ack is for, and the outcome of that item.
         */
        CMD_WORKITEM wItem;
        bool success;

    public:
        CMD_ACK_CLASS (CMD_WORKITEM i, bool s) : wItem(i), success(s) { }

        /*
         * Accessors...
         */
        CMD_WORKITEM WorkItem (void) const { return(wItem); }
        bool Success (void) const { return(success); }

};




/********************************************************************
 * CMD_WORKITEM
 *
 * Base class for types of work the controller and performance model 
 * can perform.
 *
 *********************************************************************/


class CMD_WORKITEM_CLASS
{
        /*
         * Worklist is a friend so that it can manipulate the
         * next field.
         */
        friend class CMD_WORKLIST_CLASS;

    private:
        /*
         * Name for this item...
         */
        char *name;
        
        /*
         * Next workitem in a list...
         */
        CMD_WORKITEM next;

    protected:
        /*
         * Is this action trigger by cycles or instructions.
         */
        CMD_ACTIONTRIGGER trigger;

        /*
         * Cycle or committed instruction when this action should happen.
         */
        UINT64 actionTime;

        /*
         * For periodic actions, the number of cycles or instructions
         * between occurrences.
         */
        UINT64 period;
        
    public:
        CMD_WORKITEM_CLASS (char *n, CMD_ACTIONTRIGGER t =ACTION_NEVER, UINT64 c =0) :
            name(n), next(NULL), trigger(t), actionTime(c), period(c) {
            VERIFYX((trigger != ACTION_NOW) || (actionTime == 0));
            ASSERTX(period || !(trigger == ACTION_NANOSECOND_PERIOD
                                || trigger == ACTION_CYCLE_PERIOD
                                || trigger == ACTION_INST_PERIOD
                                || trigger == ACTION_MACROINST_PERIOD));
        }
        virtual ~CMD_WORKITEM_CLASS () { }

        /*
         * Accessors...
         */
        char *Name (void) { return(name); }
        CMD_ACTIONTRIGGER Trigger (void) const { return(trigger); }
        UINT64& ReadyNanosecond (void) {
            ASSERTX((trigger == ACTION_NANOSECOND_ONCE) || (trigger == ACTION_NANOSECOND_PERIOD));
            return(actionTime);
        }
        UINT64& ReadyCycle (void) {
            ASSERTX((trigger == ACTION_CYCLE_ONCE) || (trigger == ACTION_CYCLE_PERIOD));
            return(actionTime);
        }
        UINT64& ReadyInst (void) {
            ASSERTX((trigger == ACTION_INST_ONCE) || (trigger == ACTION_INST_PERIOD));
            return(actionTime);
        }

        UINT64& ReadyMacroInst (void) {
            ASSERTX((trigger == ACTION_MACROINST_ONCE) || (trigger == ACTION_MACROINST_PERIOD));
            return(actionTime);
        }

        UINT64& ReadyPacket (void) {
            ASSERTX((trigger == ACTION_PACKET_ONCE));
            return(actionTime);
        }
        UINT64 Period (void) const {
            // CMP_FIX (remove this comment before checkin)
            // ASSERTX((trigger == ACTION_INST_PERIOD) || (trigger == ACTION_CYCLE_PERIOD));
            return(period);
        }

        UINT64 ActionTime(void) const {
            return (actionTime);
        }
        
        /*
         * Called by the performance model to take the action
         * appropriate for this item. The default action is to do nothing.
         */
        virtual CMD_ACK PmAction (void) { return(new CMD_ACK_CLASS(this, true)); }

        /*
         * Called by the controller to take the action appropriate
         * for this item. The default action is to do nothing.
         */
        virtual void CmdAction (void) { }

        /*
         * Schedule this work item in 'schedule' based on 'actionTime',
         * 'trigger', and 'period'.
         */
        virtual void Schedule (CMD_SCHEDULE schedule, UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst,UINT64 currentMacroInst, UINT64 currentPacket) {
            schedule->Schedule(this, currentNanosecond, currentCycle, currentInst, currentMacroInst, currentPacket);
        }

};


/********************************************************************
 * Classes derived from CMD_WORKITEM
 *
 * All the classes for the specific work that the controller and
 * performance model can perform.
 *
 *********************************************************************/

/*
 * CMD_INIT
 *
 * Initialize the performance model.
 */
typedef class CMD_INIT_CLASS *CMD_INIT;
class CMD_INIT_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        /*
         * Arguments for the feeder and system.
         */
        UINT32 fdArgc, pmArgc;
        char **fdArgv, **pmArgv, **Envp;

    public:
        CMD_INIT_CLASS (UINT32 fc, char **fv, UINT32 pc, char **pv, char **ep):
            CMD_WORKITEM_CLASS("INIT"),
            fdArgc(fc), pmArgc(pc), fdArgv(fv), pmArgv(pv), Envp(ep) { }

        CMD_ACK PmAction (void);

        /*
         * Schedule should not be called for this item.
         */
        void Schedule (CMD_SCHEDULE schedule, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket) {
            VERIFYX(false);
        }

};


/*
 * CMD_START
 *
 * Start the performance model.
 */
typedef class CMD_START_CLASS *CMD_START;
class CMD_START_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_START_CLASS (void) : CMD_WORKITEM_CLASS("START", ACTION_NOW) { }
        void CmdAction (void);
        
};


/*
 * CMD_STOP
 *
 * Stop the performance model.
 */
typedef class CMD_STOP_CLASS *CMD_STOP;
class CMD_STOP_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_STOP_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) : CMD_WORKITEM_CLASS("STOP", t, c) { }
        void CmdAction (void);

};


/*
 * CMD_PROGRESS
 *
 * Show performance model progress
 */
typedef class CMD_PROGRESS_CLASS *CMD_PROGRESS;
class CMD_PROGRESS_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        AWB_PROGRESSTYPE type;
        char *args;
        
    public:
        CMD_PROGRESS_CLASS (AWB_PROGRESSTYPE p, char *a, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("PROGRESS", t, c), type(p), args(a) { }

        void Schedule (CMD_SCHEDULE schedule, UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket);
        void CmdAction (void);

};


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
 * CMD_EMITSTATS
 *
 * Emit performance model stats
 */
typedef class CMD_EMITSTATS_CLASS *CMD_EMITSTATS;
class CMD_EMITSTATS_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_EMITSTATS_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("EMITSTATS", t, c) { }

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
 * CMD_RESETSTATS
 *
 * Reset performance model stats
 */
typedef class CMD_RESETSTATS_CLASS *CMD_RESETSTATS;
class CMD_RESETSTATS_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_RESETSTATS_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("RESETSTATS", t, c) { }

        void CmdAction (void);
};


/*
 * CMD_DEBUG
 *
 * Turn debugging on or off.
 */
typedef class CMD_DEBUG_CLASS *CMD_DEBUG;
class CMD_DEBUG_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        bool on;
        
    public:
        CMD_DEBUG_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("DEBUG", t, c), on(o) { }

        CMD_ACK PmAction (void);

};

/*
 * CMD_TRACE
 *
 * Turn Tracing on or off.
 */
typedef class CMD_TRACE_CLASS *CMD_TRACE;
class CMD_TRACE_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        TRACEABLE_DELAYED_ACTION regex;

    public:
        CMD_TRACE_CLASS (TRACEABLE_DELAYED_ACTION _regex, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("DEBUG", t, c), regex(_regex) { }
        ~CMD_TRACE_CLASS()
        {
            if(regex) {
                delete(regex);
            }
        }

        CMD_ACK PmAction (void);
};

/*
 * CMD_STATS
 *
 * Turn stats on or off.
 */
typedef class CMD_STATS_CLASS *CMD_STATS;
class CMD_STATS_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        bool on;
        
    public:
        CMD_STATS_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("STATS", t, c), on(o) { }

        CMD_ACK PmAction (void);

};


/*
 * CMD_PTV
 *
 * Turn ptv on or off.
 */
typedef class CMD_PTV_CLASS *CMD_PTV;
class CMD_PTV_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        bool on;
        
    public:
        CMD_PTV_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("PTV", t, c), on(o) { }

        CMD_ACK PmAction (void);

};

/*
 * CMD_EVENTS
 *
 * Turn events on or off.
 */
typedef class CMD_EVENTS_CLASS *CMD_EVENTS;
class CMD_EVENTS_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        bool on;
        
    public:
        CMD_EVENTS_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("EVENTS", t, c), on(o) { }

        CMD_ACK PmAction (void);

};


/*
 * CMD_STRIPCHART
 *
 * Turn stripcharts on or off.
 */
typedef class CMD_STRIPCHART_CLASS *CMD_STRIPCHART;
class CMD_STRIPCHART_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        bool on;
        
    public:
        CMD_STRIPCHART_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("STRIPCHART", t, c), on(o) { }

        CMD_ACK PmAction (void);

};


/*
 * CMD_PROFILE
 *
 * Turn profiling on or off.
 */
typedef class CMD_PROFILE_CLASS *CMD_PROFILE;
class CMD_PROFILE_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        bool on;
        
    public:
        CMD_PROFILE_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("PROFILE", t, c), on(o) { }

        CMD_ACK PmAction (void);

};


/*
 * CMD_EXIT
 *
 * Exit the performance model.
 */
typedef class CMD_EXIT_CLASS *CMD_EXIT;
class CMD_EXIT_CLASS : public CMD_WORKITEM_CLASS
{
    public:
        CMD_EXIT_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) : CMD_WORKITEM_CLASS("EXIT", t, c) { }
        void CmdAction (void);

};


/*
 * CMD_EXECUTE
 *
 * Start performance model execution.
 */
typedef class CMD_EXECUTE_CLASS *CMD_EXECUTE;
class CMD_EXECUTE_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        UINT64 nanoseconds;
        UINT64 cycles;
        UINT64 insts;
        UINT64 macroinsts;
        UINT64 packets;
        
    public:
        CMD_EXECUTE_CLASS (UINT64 n, UINT64 c, UINT64 i, UINT64 m,UINT64 p) :
            CMD_WORKITEM_CLASS("EXECUTE"), nanoseconds(n), cycles(c), insts(i), macroinsts(m), packets(p) { }
        
        CMD_ACK PmAction (void);

        /*
         * Schedule should not be called for this item.
         */
        void Schedule (CMD_SCHEDULE schedule, UINT64 currentNanosecond, UINT64 currentCycle, UINT64 currentInst, UINT64 currentMacroInst, UINT64 currentPacket) {
            VERIFYX(false);
        }

};


/*
 * CMD_THDBEGIN
 *
 * A new thread is beginning.
 */
typedef class CMD_THDBEGIN_CLASS *CMD_THDBEGIN;
class CMD_THDBEGIN_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDBEGIN_CLASS (ASIM_THREAD t) : CMD_WORKITEM_CLASS("THDBEGIN", ACTION_NOW), thread(t) { }
        
        void CmdAction (void);

};


/*
 * CMD_THDEND
 *
 * A thread is ending.
 */
typedef class CMD_THDEND_CLASS *CMD_THDEND;
class CMD_THDEND_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDEND_CLASS (ASIM_THREAD t) : CMD_WORKITEM_CLASS("THDEND", ACTION_NOW), thread(t) { }
        
        void CmdAction (void);

};


/*
 * CMD_THDUNBLOCK
 *
 * A new thread is un-blocking.
 */
typedef class CMD_THDUNBLOCK_CLASS *CMD_THDUNBLOCK;
class CMD_THDUNBLOCK_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDUNBLOCK_CLASS (ASIM_THREAD t) : CMD_WORKITEM_CLASS("THDUNBLOCK", ACTION_NOW), thread(t){ }
        
        void CmdAction (void);
        CMD_ACK PmAction (void);

};


/*
 * CMD_THDBLOCK
 *
 * A thread is blocking.
 */
typedef class CMD_THDBLOCK_CLASS *CMD_THDBLOCK;
class CMD_THDBLOCK_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        ASIM_INST inst;
        
    public:
        CMD_THDBLOCK_CLASS (ASIM_THREAD t, ASIM_INST i) : CMD_WORKITEM_CLASS("THDBLOCK", ACTION_NOW), thread(t) , inst(i){ }
        
        void CmdAction (void);
        CMD_ACK PmAction (void);

};

/*
 * CMD_THDSKIP
 *
 * Skip the feeder ahead the specified number of instructions.
 */
typedef class CMD_THDSKIP_CLASS *CMD_THDSKIP;
class CMD_THDSKIP_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        UINT64 insts;
        INT32 markerID;
        
    public:
        CMD_THDSKIP_CLASS (ASIM_THREAD thd, UINT64 i, INT32 id, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("THDSKIP", t, c), thread(thd), insts(i),
            markerID(id) { }
        
        CMD_ACK PmAction (void);

};


/*
 * CMD_THDSCHED
 *
 * Schedule thread on the performance model.
 */
typedef class CMD_THDSCHED_CLASS *CMD_THDSCHED;
class CMD_THDSCHED_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDSCHED_CLASS (ASIM_THREAD thd, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("THDSCHED", t, c), thread(thd) { }
        
        CMD_ACK PmAction (void);

};


/*
 * CMD_THDUNSCHED
 *
 * Schedule thread on the performance model.
 */
typedef class CMD_THDUNSCHED_CLASS *CMD_THDUNSCHED;
class CMD_THDUNSCHED_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDUNSCHED_CLASS (ASIM_THREAD thd, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("THDUNSCHED", t, c), thread(thd) { }
        
        CMD_ACK PmAction (void);

};


/*
 * CMD_THDHOOK
 *
 * Hook thread to the performance model.
 */
typedef class CMD_THDHOOK_CLASS *CMD_THDHOOK;
class CMD_THDHOOK_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDHOOK_CLASS (ASIM_THREAD thd, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("THDHOOK", t, c), thread(thd) { }
        
        CMD_ACK PmAction (void);

};


/*
 * CMD_THDUNHOOK
 *
 * Unhook thread from the performance model.
 */
typedef class CMD_THDUNHOOK_CLASS *CMD_THDUNHOOK;
class CMD_THDUNHOOK_CLASS : public CMD_WORKITEM_CLASS
{
    private:
        ASIM_THREAD thread;
        
    public:
        CMD_THDUNHOOK_CLASS (ASIM_THREAD thd, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_WORKITEM_CLASS("THDUNHOOK", t, c), thread(thd) { }
        
        CMD_ACK PmAction (void);

};


/*******************************************************************/


/*******************************************************************
 * CMD_WORKLIST
 *
 * A list of work items
 ******************************************************************/


typedef class CMD_WORKLIST_CLASS *CMD_WORKLIST;

class CMD_WORKLIST_CLASS
{
    private:
        CMD_WORKITEM head, tail;

        /*
         * Remove an item from the list, it could be at the head, middle,
         * or tail.
         */
        CMD_WORKITEM RemoveItem (CMD_WORKITEM item) {
            if (item == NULL)
                return(NULL);

            if (item == head) {
                if (head == tail)
                    head = tail = NULL;
                else
                    head = head->next;
            }
            else {
                CMD_WORKITEM scan = head;
                while (scan->next != item) {
                    VERIFYX(scan != NULL);
                    scan = scan->next;
                }

                scan->next = item->next;
                if (item == tail)
                    tail = scan;
            }
            
            item->next = NULL;
            return(item);
        }
        
    public:
        // constructors / destructors
        CMD_WORKLIST_CLASS () : head(NULL), tail(NULL) { }

        ~CMD_WORKLIST_CLASS () {
            while (head) {
                CMD_WORKITEM item = RemoveItem(head);
                delete item;
            }
        }

        /*
         * Return the head item in the list, or NULL if the list is empty.
         */
        CMD_WORKITEM Head (void) { return(head); }

        /*
         * Add 'wi' to the list so that it is ordered by 'actionTime'.
         * Items with the same 'actionTime' are ordered in the manner in
         * which they are inserted, first items inserted are first
         * in the list.
         */
        void InsertOrdered (CMD_WORKITEM wi) {
            ASSERTX((wi->trigger != ACTION_NOW) || (wi->actionTime == 0));
            ASSERTX(wi->next == NULL);
            if (head == NULL) {
                ASSERTX(tail == NULL);
                head = tail = wi;
            }
            else {
                ASSERTX(tail != NULL);

                if (head->actionTime > wi->actionTime) {
                    wi->next = head;
                    head = wi;
                }
                else {
                    CMD_WORKITEM scan = head;
                    while ((scan != tail) && (scan->next->actionTime <= wi->actionTime))
                        scan = scan->next;

                    wi->next = scan->next;
                    scan->next = wi;
                    if (scan == tail)
                        tail = wi;
                }
            }
        }
        
        /*
         * Add a work item to the tail of the list.
         */
        void Add (CMD_WORKITEM wi) {
            ASSERTX(wi->next == NULL);
            if (head == NULL) {
                ASSERTX(tail == NULL);
                head = tail = wi;
            }
            else {
                ASSERTX(tail != NULL);
                tail->next = wi;
                tail = wi;
            }
        }

        /*
         * Remove the head item from the list. Error if there
         * is nothing in the list.
         */
        CMD_WORKITEM Remove (void) {
            if (head == NULL) {
                ASIMERROR("CMD_WORKLIST_CLASS: Attempt to remove from empty list\n");
            }
            
            CMD_WORKITEM item = RemoveItem(head);
            return(item);
        }

        /*
         * Clear all CMD_PROGRESS work items from the list.
         */
        void ClearProgress (void) {

            CMD_WORKITEM scan = head;
            while (scan != NULL) {
                CMD_WORKITEM next = scan->next;
                if (strcmp(scan->Name(), "PROGRESS") == 0) {
                    delete RemoveItem(scan);
                }
                scan = next;
            }
        }
};


#endif /* _CMD_ */
