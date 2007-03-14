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
 * @author David Goodwin, Alexey Klimkin, Carl Beckmann
 * @brief
 */

#ifndef __CONTROL_X86E_H__
#define __CONTROL_X86E_H__


// ASIM modules 
#include "asim/provides/controller.h"
#include "pipe.h"

// local module
#include "control_x86.h"


#define PIPE_RECORD_EVENT_TIME_MACRO(...)                pipe_record_event_time(__VA_ARGS__)



/********************************************************************
 *
 * The following is a singleton object class that represents the controller.
 * It inherits most of its behavior from the "x86" controller base class,
 * and just adds x86 controller specializations.
 *
 ********************************************************************/

class CONTROLLER_X86E_CLASS: public CONTROLLER_X86_CLASS {
  public:
    CONTROLLER_X86E_CLASS():
        CONTROLLER_X86_CLASS() {};

    // primary entry point for the code
    int main(INT32 argc, char *argv[], char *envp[]);

    // these methods are changed from x86 controller
    bool CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv, char **knobsArgv, char **allEnvp);
    void CMD_EmitStats (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_ResetStats (CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    void CMD_Ptv (bool on, CMD_ACTIONTRIGGER trigger =ACTION_NOW, UINT64 n =0);
    
    // not normally used, but we need this to keep the parent class' main() happy:
    bool CMD_Init (UINT32 fdArgc, char **fdArgv, UINT32 pmArgc, char **pmArgv,                   char **allEnvp)
    {    return
	 CMD_Init (       fdArgc,        fdArgv,        pmArgc,        pmArgv, /*knobsArgv*/NULL,       allEnvp);
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
 * CMD_INIT_X86E
 *
 * Initialize the performance model.
 */
typedef class CMD_INIT_X86E_CLASS *CMD_INIT_X86E;
class CMD_INIT_X86E_CLASS : public CMD_INIT_X86_CLASS
{
    public:
        CMD_INIT_X86E_CLASS (UINT32 fc, char **fv, UINT32 pc, char **pv, char **ep):
            CMD_INIT_X86_CLASS (fc, fv, pc, pv, ep) { }

        CMD_ACK PmAction (void);
};

/*
 * CMD_EMITSTATS_X86E
 *
 * Emit performance model stats
 */
typedef class CMD_EMITSTATS_X86E_CLASS *CMD_EMITSTATS_X86E;
class CMD_EMITSTATS_X86E_CLASS : public CMD_EMITSTATS_X86_CLASS
{
    public:
        CMD_EMITSTATS_X86E_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_EMITSTATS_X86_CLASS (t, c) { }

        void CmdAction (void);
};


/*
 * CMD_RESETSTATS
 *
 * Reset performance model stats
 */
typedef class CMD_RESETSTATS_X86E_CLASS *CMD_RESETSTATS_X86E;
class CMD_RESETSTATS_X86E_CLASS : public CMD_RESETSTATS_X86_CLASS
{
    public:
        CMD_RESETSTATS_X86E_CLASS (CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_RESETSTATS_X86_CLASS (t, c) { }

        void CmdAction (void);
};


/*
 * CMD_PTV
 *
 * Turn ptv on or off.
 */
typedef class CMD_PTV_X86E_CLASS *CMD_PTV_X86E;
class CMD_PTV_X86E_CLASS : public CMD_PTV_CLASS
{
    public:
        CMD_PTV_X86E_CLASS (bool o, CMD_ACTIONTRIGGER t, UINT64 c) :
            CMD_PTV_CLASS (o, t, c) { }

        CMD_ACK PmAction (void);
};


#endif /*__CONTROL_X86E_H__ */
