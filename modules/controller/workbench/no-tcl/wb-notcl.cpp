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

#include <ctype.h>
#include <sys/times.h> // for times()
#include <unistd.h> // for sysconf()
#include <stdlib.h> // for atoi()
#include "asim/atoi.h"
#include "wb-notcl.h"
#include "asim/provides/awb_stub.h"

using namespace std;

#define DMSG(x) \
({\
       cerr << __FILE__ << ":" << __LINE__ << ": " <<  x  << endl; \
})


static bool
split_string(const string& args,
             string& arg1,
             string& arg2)
{    
    string::const_iterator p = args.begin();

    while (p != args.end()  && isspace(*p))
    {
        p++;
    }
    if (p == args.end())
    {
        return false;
    }
    while (p != args.end()  && !isspace(*p))
    {
        arg1 += *p;
        p++;
    }
    if (p == args.end())
    {
        return false;
    }
    while (p != args.end()  && isspace(*p))
    {
        p++;
    }
    if (p == args.end())
    {
        return false;
    }
    while (p != args.end()  && !isspace(*p))
    {
        arg2 += *p;
        p++;
    }
    return true;
}

void
FILE_PARSER_CLASS::print_tokens()
{
    list< vector<string> >::iterator iter = input_tokens.begin();
    while(iter != input_tokens.end())
    {
        vector<string>& vs = *iter;
        int nt = vs.size();
        for(int j=0 ; j< nt;  j++)
        {
            cout << '[' << vs[j] << "] ";
        }
        cout << endl;
        iter++;
    }
}

void
FILE_PARSER_CLASS:: read_file(char* fn)
{
    ifstream in(fn);
    if (!in)
    {
        cerr << "Could not open [" << fn << "], running without cmds file" << endl;
        vector<string> vs;
        string s;
        cerr << "Could not open [" << fn << "], running without cmds file" << endl;

        s= "AwbStats";
        vs.push_back(s);
        s= "on";
        vs.push_back(s);
        input_tokens.push_back(vs);
        vs.erase(vs.begin(), vs.end());

        s= "AwbRun";
        vs.push_back(s);
        s= "inst";
        vs.push_back(s);
        s= "-1";
        vs.push_back(s);
        input_tokens.push_back(vs);
        vs.erase(vs.begin(), vs.end());

        s= "AwbExit";
        vs.push_back(s);
        input_tokens.push_back(vs);

    }
    else
    {
        int line = 0;
        int token = 0;
        char ch;
        vector<string> vs;
        string s;
        while(in.get(ch))
        {   
            //cerr << "read [" << ch << "]" << endl;
            if (ch == '{' || ch == '}')
            {
                continue;    // skip the old curly braces
            }
            else if (ch == '\n')
            {
                if (s != "")
                {
                    vs.push_back(s);
                }
                input_tokens.push_back(vs);
                line++;
                token = 0;
                vs.erase(vs.begin(), vs.end());
                s = "";
            }
            else if (isspace(ch))
            {
                if (s != "")
                {
                    vs.push_back(s);
                    s = "";
                    token++;
                }
            }
            else
            {
                s += ch;
            }
        }
    }
}


class DISPATCH_TABLE_CLASS
{
  public:
    char* str;
    AWB_TOKEN_ENUM  token;

#if 0
    DISPATCH_TABLE_CLASS(
        char* arg_str,
        AWB_TOKEN_ENUM  arg_token)
        : str(arg_str),
          token(arg_token)
    {
    }
#endif
};

static DISPATCH_TABLE_CLASS
dispatch_table[] =
{
    { "AwbMarker", AWB_MARKER},
    { "AwbRun",    AWB_RUN},
    { "AwbExit",   AWB_EXIT},
    { "AwbProgress", AWB_PROGRESS},
    { "AwbSkip",AWB_SKIP},
    { "AwbSkipUntil", AWB_SKIPUNTIL},
    { "AwbSample",   AWB_SAMPLE},
    { "AwbStats",   AWB_STATS},
    { "AwbEvents",   AWB_EVENTS},
    { "AwbProtect",   AWB_PROTECT},
    { 0, AWB_INVALID }
};


COMMAND_PARSER_CLASS::COMMAND_PARSER_CLASS(char* awbCmdsFile) // CONS
    : processed_awb_cmds_file(false)
{
    read_file(awbCmdsFile);
    PmScheduleStopCycle(0);
    PmScheduleStart();
}


void
COMMAND_PARSER_CLASS::process_file()
{
    DMSG("Processing (remaining tokens in) awbcmds file");
    while(!parser.input_tokens.empty())
    {
        vector<string>& vs = parser.input_tokens.front();
        AWB_TOKEN_ENUM tok = AWB_INVALID;
        if (vs.size() > 0)
        {
            tok = match_string(vs[0]);
            dispatch(tok, vs);
        }
        parser.input_tokens.pop_front();
        
        if (tok == AWB_RUN || tok == AWB_EXIT)
        {
            return;
        }
    }
}

void
COMMAND_PARSER_CLASS::read_file(char* fn)
{
    parser.read_file(fn);
}

AWB_TOKEN_ENUM
COMMAND_PARSER_CLASS::match_string(const string& s) const
{
    int i = 0;
    while(dispatch_table[i].str)
    {
        string s2(dispatch_table[i].str);
        if ( s == s2 )
        {
            return dispatch_table[i].token;
        }
        i++;
    }
    return AWB_INVALID;
}

void
COMMAND_PARSER_CLASS::print_line(char* s, vector<string>& vs)
{
    cerr << s << ": [";
    for(unsigned int i=0; i<vs.size() ; i++)
    {
        cerr << vs[i] << " ";
    }
    cerr << "]" << endl;
}
        
void
COMMAND_PARSER_CLASS::invalid_line(vector<string>& vs)
{
    print_line("Error: invalid line", vs);
    exit(1);
}

void
COMMAND_PARSER_CLASS:: dispatch(AWB_TOKEN_ENUM token,
                                vector<string>& vs)
{
    switch(token)
    {
      default:
      case AWB_INVALID:
      case AWB_MAX:
        cerr << "Invalid awbcmds command: [" << vs[0] << "]" << endl;
        exit(1);
        break;

      case AWB_MARKER:
        do_marker(vs);
        break;
      case AWB_RUN:
        do_run(vs);
        break;
      case AWB_EXIT:
        if (vs.size() != 1)
        {
            invalid_line(vs);
        }
        do_exit();
        break;
      case AWB_PROGRESS:
        do_progress(vs);
        break;
      case AWB_SKIP:
        do_skip(vs);
        break;
      case AWB_SKIPUNTIL:
        do_skipuntil(vs);
        break;
      case AWB_SAMPLE:
        do_sample(vs);
        break;
      case AWB_STATS:
        do_stats(vs);
        break;
      case AWB_EVENTS:
        do_events(vs);
        break;
      case AWB_PROTECT:
        do_protect(vs);
        break;
    }
}

void
COMMAND_PARSER_CLASS::do_marker(vector<string>& vs) // tid set/clear markerid inst/pc generic-arg/addr
{
    const int sz = vs.size();
    if ( sz != 5  || sz != 6)
    {
        invalid_line(vs);
    }
    else
    {
        int tid = atoi_general(vs[0]);
        THREAD_CLASS* thd = FindThread(tid);

        string& cmd = vs[1];
        INT32 markerID = atoi_general(vs[2]);
        string& subcmd = vs[3];
        string& arg1 = vs[4];
        if (cmd == "set")
        {
            string arg2 = "";
            if (sz == 6)
            {
                arg2 = vs[5];
            }
            //FIXME 10/22/03 Mark Charney: args wrong
            PmMarkerSet(thd->ThdDesc(), markerID, subcmd, arg1, arg2);
        }
        else if (cmd == "clear")
        {
            //FIXME 10/22/03 Mark Charney: args wrong
            PmMarkerClear(thd->ThdDesc(), markerID, subcmd, arg1);
        }
        else
        {
            invalid_line(vs);
        }
    }
    
}
void
COMMAND_PARSER_CLASS::do_run(vector<string>& vs) // cycle/inst/marker amount
{
    print_line("do-run", vs);

    const int sz = vs.size();
    if ( sz != 3 )
    {
        invalid_line(vs);
    }
    else
    {
        INT64 stopTime;
        INT64 amount = atoi_general(vs[2]);
        if (vs[1] == "cycle")
        {
            stopTime = PmCycle() + amount;
            PmScheduleStopCycle(stopTime);
        }
        else if (vs[1] == "inst")
        {
            stopTime = PmGlobalCommitted() + amount;
            DMSG("AwbRun inst until " << stopTime);
            PmScheduleStopInst( stopTime);
        }
        else if (vs[1] == "marker")
        {
            //FIXME: this was copied from Tcl and looks like a bug
            stopTime = PmCommittedMarkers() + 1;
            PmCommittedWatchMarker(amount);
        }
        else
        {
            invalid_line(vs);
        }
        bm_stop_type = vs[1];
        bm_stop_time = stopTime;
        PmScheduleStart();
    }
}

void
COMMAND_PARSER_CLASS::do_exit() // no args
{
    DMSG("AwbExit...");
    PmScheduleExitNow();
    PmScheduleStart();
}
void
COMMAND_PARSER_CLASS::do_progress(vector<string>& vs) // type period
{
    DMSG("do-progress type: " << vs[1] << " period: " << vs[2]);
    const int sz = vs.size();
    if (sz != 3)
    {
        invalid_line(vs);
    }
    string& type = vs[1];
    string& period = vs[2];
    PmScheduleProgress(type, period);
}
void
COMMAND_PARSER_CLASS::do_skip(vector<string>& vs) // uid inst marker
{
    const int sz = vs.size();
    if (sz == 3 || sz == 4)
    {
        INT32 marker = -1;
        if (sz == 4)
        {
            marker = atoi_general(vs[3]);
        }
        string str_uid = vs[1]; // may be numeric or "all"
        int uid;
        if (str_uid == "all")
        {
            uid = -1;
        }
        else
        {
            uid = atoi_general(str_uid);
        }
        UINT64 inst = atoi_general_unsigned(vs[2]);
        UINT64 currTime = PmCycle();
        string cycle_once("cycle_once");
        UnscheduleThread( uid, cycle_once, currTime);
        SkipThread( uid, inst, marker, cycle_once, currTime + 1);
        ScheduleThread(uid, cycle_once, currTime + 2);
    }
    else
    {
        invalid_line(vs);
    }
}
void
COMMAND_PARSER_CLASS::do_skipuntil(vector<string>& vs) // uid when -- deprecated
{
    cerr << "AwbSkipUntil is deprecated" << endl;
    exit(1);
}
void
COMMAND_PARSER_CLASS::do_sample(vector<string>& vs) // warmup run skip iter/forever
{
    UINT64 warmup = atoi_general_unsigned(vs[0]);
    UINT64 run = atoi_general_unsigned(vs[1]);
    UINT64  skip = atoi_general_unsigned(vs[2]); //FIXME 10/22/03 Mark Charney:  could be one or two numbers!
    string& iter = vs[3];
    INT64 iterval = atoi_general(vs[3]);
    bool forever = (iter == "forever");


    if ( (warmup > 0 || run > 0 || skip > 0) &&
         (forever || iterval > 0)  )
    {
        if (! forever )
        {
            iterval = iterval - 1;
        }

        //FIXME insert AwbSample warmup run skip iter

        if (skip > 0)
        {
            // insert AwbStats off
            // insert AwbBeginDrain
            // insert AwbRun cycle 1000
            // insert AwbNewSkip all $skip
            // insert AwbEndDrain
        }
        if (run > 0)
        {
            // insert AwbStats on
            // insert AwbRun inst $run
            // insert AwbStats off
        }
        
        if (warmup > 0)
        {
            // insert AwbStats off
            // insert AwbRun inst $warmup
        }
    }

}

void
COMMAND_PARSER_CLASS::do_stats(vector<string>& vs) // onoffdump file
{
    print_line("do-stats", vs);

    string stats("stats.out");
    const int sz = vs.size();
    if (vs[1] == "on" && sz == 2)
    {
        string all("all");
        PmStateUnsuspend(all);
    }
    else if (vs[1] == "off" && sz == 2)
    {
        string all("all");
        PmStateSuspend(all);
    }
    else  if (vs[1] == "dump")
    {
        switch(sz)
        {
          case 2:
            PmStateDump(stats);
            break;
          case 3:
            PmStateDump(vs[2]);
            break;
          default:
            invalid_line(vs);
        }
    }
    else if (vs[1] == "dumponexit")
    {
        switch(sz)
        {
          case 2:
            dumponexitfile = "stats.out";
            break;
          case 3:
            dumponexitfile = vs[2];
            break;
          default:
            invalid_line(vs);
        }
    }
    else
    {
        invalid_line(vs);
    }
}
void
COMMAND_PARSER_CLASS::do_events(vector<string>& vs) // onofffile file
{
    const int sz = vs.size();
    if (vs[1] == "on" && sz == 2)
    {
        PmEventOn();
    }
    else if (vs[1] == "off" && sz == 2)
    {
        PmEventOff();
    }
    else if (vs[1] == "filename" )
    {
        string s;
        if (sz == 3)
        {
            s = vs[2];
        }
        else if (sz == 2)
        {
            s = "Events.drl.gz";
        }
        else
        {
            invalid_line(vs);
        }
        PmEventFilename(s);
    }
    else
    {
        invalid_line(vs);
    }
}
void
COMMAND_PARSER_CLASS::do_protect(vector<string>& vs) // script
{
    //FIXME
    cerr << "do_protect() / AwbProtect not implemented yet" << endl;
    exit(1);
    
}

/////////////////////////////////////////////////////
// Sequencing routines...
/////////////////////////////////////////////////////

void
COMMAND_PARSER_CLASS::ProcessBmCmds()
{
    DMSG("\t\tProcessBmCmds");
    while(!parser.input_tokens.empty())
    {
        vector<string>& vs = parser.input_tokens.front();
        int nt = vs.size();
        string& s = vs[0];

        parser.input_tokens.pop_front(); 

        AWB_TOKEN_ENUM tok = match_string(s);
        dispatch(tok, vs);

        if (tok == AWB_RUN || tok == AWB_EXIT)
        {
            return;
        }
    }

    if (bm_stop_type == "")
    {
        DMSG("\t\t\tStart - Stop");
        PmScheduleStart();
        PmScheduleStopNow();
    }
}

void
COMMAND_PARSER_CLASS::CheckBmCmds()
{
    DMSG("\tCheckBmCmds");
    check_awb_cmds_file(); 

    if (bm_stop_type == "cycle")
    {
        if (PmCycle() == bm_stop_time)
        {
            ProcessBmCmds();
        }
    }
    else if (bm_stop_type == "inst")
    {
        if (PmGlobalCommitted() >= bm_stop_time)
        {
            ProcessBmCmds();
        }
    }
    else if (bm_stop_type == "marker")
    {
        if (PmCommittedMarkers() >= bm_stop_time)
        {
            ProcessBmCmds();
        }
    }
    else
    {
        ProcessBmCmds();
    }
}
void
COMMAND_PARSER_CLASS::check_awb_cmds_file()
{
//    if (!processed_awb_cmds_file)
//    {
        process_file();
//        processed_awb_cmds_file = true;
//    }
}


void
COMMAND_PARSER_CLASS::BatchProgress(list<string> args)
{

    while(!args.empty())
    {
        string s = args.front();
        args.pop_front();

        DMSG("BatchProgress [" << s << "]");


        if (s == "cycle" || s == "inst")
        {
            ShowProgress();
        }
        else if (s == "start")
        {
            // nada
        }
        else if (s == "stop")
        {
            CheckBmCmds();
        }
        else if (s == "threadbegin")
        {
        
            string two_args = args.front();
            args.pop_front(); // one string, two args
            string arg1, arg2;
            bool okay = split_string(two_args,arg1,arg2);
            if (!okay)
            {
                cerr << "Invalid BatchProgress threadbegin token: "
                     << two_args << endl;
                exit(1);
            }


            DMSG("BatchProgress threadbegin "
                 << "[" << arg1 << "] "
                 << "[" << arg2 << "]");

            ThreadBegin(atoi_general(arg1), arg2);
        }
        else if (s == "threadend")
        {
            string two_args = args.front();
            args.pop_front(); // one string, two args
            string arg1, arg2;
            bool okay = split_string(two_args,arg1,arg2);
            if (!okay)
            {
                cerr << "Invalid BatchProgress threadbegin token: "
                     << two_args << endl;
                exit(1);
            }

            ThreadEnd(atoi_general(arg1), arg2);
        }
        else if (s == "threadblock")
        {
            // nada
        }
        else if (s == "threadunblock")
        {
            // nada
        }
        else if (s == "exit")
        {
            if (dumponexitfile != "")
            {
                PmStateDump(dumponexitfile);
                cerr << "Exiting ... " << endl;
                exit(0); //FIXME -- should we exit?
            }
        }
        else
        {
            cerr << "Invalid BatchProgress token: " << s << endl;
            exit(1);
        }
            
    }
}
    


/////////////////////////////////////////////////////
// Support routines...
/////////////////////////////////////////////////////

UINT64
COMMAND_PARSER_CLASS::get_time() 
{
    struct tms time;
    times(&time);
    UINT64 t = (time.tms_utime + time.tms_stime);
    UINT64 ticks_per_second = sysconf(_SC_CLK_TCK);
    return t * ticks_per_second;
}

void
COMMAND_PARSER_CLASS::ShowProgress()
{
    int ncpus = PmNumCpus();
    int precision = cout.precision();
    cout.setf(ios::fixed);
    for(int i=0 ; i < ncpus ; i++)
    {
        cout << "CPU: " << std::setw(2) << i
             << "\tGlobal_Cycle: " << PmCycle()
             << "\tLocal_Cycle: " << PmCycle(i)
             << "\tIPC: " << std::setprecision(2) << ipc(i) 
             << "\tCPS: " << std::setprecision(3) <<  cps()
             << std::setprecision(precision) 
             << endl;
    }
}

double
COMMAND_PARSER_CLASS::cps()
{
    UINT64 cycles = PmCycle();
    UINT64 elapsed_time =  get_time();
    return 1.0 * cycles / elapsed_time;
}

double
COMMAND_PARSER_CLASS::ipc(int cpunum)
{
    UINT64 cycle = PmCycle(cpunum);
    UINT64 ret = PmCommitted(cpunum);
    if (cycle == 0)
    {
        return 0.0;
    }
    return (1.0* ret / cycle);
}



THREAD_CLASS*
COMMAND_PARSER_CLASS::FindThread(int uid)
{
    if (uid >= 0 && (unsigned int) uid < threads.size())
    {
        return threads[uid];
    }
    return 0;
}
    
void
COMMAND_PARSER_CLASS::ThreadBegin(int uid,
                                  string tdesc)
{
    DMSG("ThreadBegin uid: " << uid << "   desc: " << tdesc);
    //FIXME: check for duplicate threads
    THREAD_CLASS* tnew = new THREAD_CLASS(uid, tdesc, THREAD_IDLE);
    threads.push_back( tnew );
    active_threads++;
    ScheduleThread(uid, "now", 0);
}

void
COMMAND_PARSER_CLASS::ThreadEnd(int uid,
                                string tdesc)
{
    //FIXME: make sure it exists and is alive
    THREAD_CLASS* thd = FindThread(uid);
    thd->SetThdState(THREAD_TERMINATED);
    active_threads--;

    UnscheduleThread(uid, "now", 0);

    // if no threads left, exit
    if (active_threads == 0)
    {
        do_exit();
    }
}
    

void
COMMAND_PARSER_CLASS::ScheduleThread(int uid,
                                     string trigger,
                                     UINT64 time)
{
    if (uid == -1) // all threads
    {
        vector<THREAD_CLASS*>::iterator iter = threads.begin();
        for( ; iter != threads.end(); iter++) 
        {
            THREAD_CLASS*& i = *iter;
            if (i->ThdState() == THREAD_IDLE)
            {
                PmScheduleThread(i->ThdDesc(), trigger, time);
                i->SetThdState(THREAD_RUNNING);
            }
        }
    }
    else
    {
        THREAD_CLASS* thd = FindThread(uid);
        PmScheduleThread(thd->ThdDesc() , trigger, time);
        thd->SetThdState(THREAD_RUNNING);
    }
}

void
COMMAND_PARSER_CLASS::UnscheduleThread(int uid,
                                       string trigger,
                                       UINT64 time)
{
    if (uid == -1) // all threads
    {
        vector<THREAD_CLASS*>::iterator iter = threads.begin();
        for( ; iter != threads.end(); iter++) 
        {
            THREAD_CLASS*& i = *iter;
            if (i->ThdState() == THREAD_RUNNING)
            {
                PmUnscheduleThread(i->ThdDesc(), trigger, time);
                i->SetThdState(THREAD_IDLE);
            }
        }
    }
    else
    {
        THREAD_CLASS* thd = FindThread(uid);
        PmUnscheduleThread( thd->ThdDesc(), trigger, time);
        thd->SetThdState(THREAD_IDLE);
    }
}

void
COMMAND_PARSER_CLASS::SkipThread(int uid,
                                 UINT64 inst,
                                 INT64 markerID,
                                 string trigger,
                                 UINT64 time)
{
    //FIXME
    if ( uid == -1 ) // all threads
    {
        // count the idle threads
        int idlecnt = 1;
        //FIXME

        string all("all");
        PmScheduleSkipThread( all , inst / idlecnt,  markerID, trigger, time);
    }
    else
    {
        THREAD_CLASS* thd = FindThread(uid);
        PmScheduleSkipThread( thd->ThdDesc(), inst, markerID, trigger, time);
    }
}

UINT64
COMMAND_PARSER_CLASS::Random(UINT64 min, UINT64 max)
{
    return (max+min)/2; //FIXME
}


//Local Variables:
//pref: "nextwb.h"
//End:
