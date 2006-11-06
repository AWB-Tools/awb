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

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include <sys/times.h> // for times()
#include <unistd.h> // for sysconf()
#include <stdlib.h> // for atoi()
#include "asim/syntax.h"

using namespace std;


class FILE_PARSER_CLASS
{
    // Parses up a txt file based on white space.  It builds a vector of
    // lines, input_tokens, where each line is a vector of strings.

    // This uses vectors and strings so it has no limits on lines or line lengths.

  public:
    FILE_PARSER_CLASS()
    {
    }

    list< vector<string> > input_tokens;

    void
    print_tokens();


    void
    read_file(char* fn);
};


typedef enum {
    AWB_INVALID, 
    AWB_MARKER,
    AWB_RUN,
    AWB_EXIT,
    AWB_PROGRESS,
    AWB_SKIP,
    AWB_SKIPUNTIL,
    AWB_SAMPLE,
    AWB_STATS,
    AWB_EVENTS,
    AWB_PROTECT,
    AWB_MAX
} AWB_TOKEN_ENUM;

typedef enum
{
    THREAD_RUNNING,
    THREAD_IDLE,
    THREAD_TERMINATED
}
THREAD_STATE_ENUM;

class THREAD_CLASS
{
    int uid;
    string desc;
    THREAD_STATE_ENUM state;
  public:
    THREAD_CLASS(int arg_uid,
                 string arg_desc,
                 THREAD_STATE_ENUM arg_state)
        : uid(arg_uid),
          desc(arg_desc),
          state(arg_state)
    {
    }
    int ThdUid() { return uid; }
    string& ThdDesc() { return desc; }
    THREAD_STATE_ENUM ThdState() { return state; }
    
    void SetThdState(THREAD_STATE_ENUM arg_state) 
    {
        state = arg_state;
    }
};

class COMMAND_PARSER_CLASS
{
    FILE_PARSER_CLASS parser;
    bool processed_awb_cmds_file;

    string dumponexitfile;

    string bm_stop_type;
    UINT64 bm_stop_time;

    void check_awb_cmds_file();

  public:

    COMMAND_PARSER_CLASS(char* awbCmdsFile); // CONS

    void
    AwbInit(); // win mode cmds cmdsData/Null

    void
    process_file();

    void
    read_file(char* fn);

    AWB_TOKEN_ENUM
    match_string(const string& s) const;

    void
    invalid_line(vector<string>& vs);

    void
    print_line(char* s,
               vector<string>& vs);

    void
    dispatch(AWB_TOKEN_ENUM token,
             vector<string>& vs);

    void do_marker(vector<string>& vs); // tid set/clear markerid inst/pc generic-arg/addr

    void do_run(vector<string>& vs); // cycle/inst/marker amount
    void do_exit(); // no args
    void do_progress(vector<string>& vs); // type period
    void do_skip(vector<string>& vs); // uid inst marker
    void do_skipuntil(vector<string>& vs); // uid when -- deprecated
    void do_sample(vector<string>& vs); // warmup run skip iter/forever
    void do_stats(vector<string>& vs); // onoffdump file
    void do_events(vector<string>& vs); // onofffile file
    void do_protect(vector<string>& vs); // script

    /////////////////////////////////////////////////////
    // Sequencing routines...
    /////////////////////////////////////////////////////

    void ProcessBmCmds();

    void CheckBmCmds();

    void BatchProgress(list<string> args);
    


    /////////////////////////////////////////////////////
    // Support routines...
    /////////////////////////////////////////////////////

    UINT64 get_time() ;

    void ShowProgress();

    double cps();
    double ipc(int cpunum);



    vector<THREAD_CLASS*> threads;
    unsigned int active_threads;

    THREAD_CLASS*
    FindThread(int uid);
    
    void ThreadBegin(int uid,
                     string tdesc);

    void ThreadEnd(int uid,
                   string tdesc);

    void
    ScheduleThread(int uid,
                   string trigger,
                   UINT64 time);
    void
    UnscheduleThread(int uid,
                     string trigger,
                     UINT64 time);

    void
    SkipThread(int uid,
               UINT64 inst,
               INT64 markerID,
               string trigger,
               UINT64 time);

    UINT64
    Random(UINT64 min, UINT64 max);
};

//Local Variables:
//pref: "wb-notcl.cpp"
//End:
