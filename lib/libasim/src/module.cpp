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
 * @brief Base class for ASIM module abstraction.
 */

// ASIM core
#include "asim/syntax.h"
#include "asim/module.h"
#include "asim/state.h"
#include "asim/stateout.h"

// ASIM public modules -- BAD! in asim-core
//#include "asim/provides/controller.h"

ASIM_DRAL_NODE_CLASS::ASIM_DRAL_NODE_CLASS(ASIM_MODULE parent, const char* const name):
    active(true)
{
    // Assign event id for this node
    // Notify that a new dral node has been created. Such notification is required 
    // for the events to work properly.
    if (runWithEventsOn)
    {
        EVENT(uniqueId = DRALEVENT(NewNode(name, (parent ? parent->GetUniqueId() : 0), 0, true)));
    }
}

ASIM_MODULE_CLASS::ASIM_MODULE_CLASS(ASIM_MODULE p, const char *const n,
				     ASIM_EXCEPT e) :
    ASIM_REGISTRY_CLASS(),
    ASIM_DRAL_NODE_CLASS(p,n),
    ASIM_CLOCKABLE_CLASS((ASIM_CLOCKABLE)p),
    ASIM_ADF_NODE_CLASS(p,n),
    name(strdup(n)), parent(p), eMod(e), contained(NULL)
/*
 * Initialize...
 */
{
    char *ppath = "";
    
    if (parent != NULL)
    {
        parent->contained = new ASIM_MODULELINK_CLASS(this, parent->contained);
        ppath = (char *)parent->Path();
    }

    path = new char[strlen(ppath)+strlen(name)+4];
    strcpy(path, ppath);
    strcat(path, "/");
    strcat(path, name);
    SetRegPath(path);

    string traceName = path;
    for(int i = 0; i < traceName[i] != 0; i++) {
        if(traceName[i] == '/') {
            traceName[i] = '.';
        }
    }
    if(traceName[0] == '.') {
        traceName.erase(0,1);
    }
    SetTraceableName(traceName);
}

ASIM_MODULE_CLASS::~ASIM_MODULE_CLASS ()
{
    if (name) {
      free (const_cast<char*> (name));
    }
    if (path)
    {
        delete [] path;
    }
    while (contained)
    {
        ASIM_MODULELINK_CLASS * tmp = contained;
        contained = contained->next;
        delete tmp;
    }
}


ASIM_STATE
ASIM_MODULE_CLASS::GetState (const char *stateStr)
/*
 * Return the ASIM_STATE object for the state specified by
 * 'stateStr'. Return NULL is no state exists.
 */
{
    //
    // Check the first "entry" in 'stateStr' to see if it matches this
    // module's name. If it does, then pass it on to each contained module.
    //
    // Strip leading '/' if there is one.

    if (*stateStr == '/')
        stateStr++;

    if (strncasecmp(Name(), stateStr, strlen(Name())) != 0)
        return(NULL);

    //
    // 'stateStr' contains this module's name as the first entry. Strip the
    // name and pass 'stateStr' to each contained module.

    stateStr += strlen(Name());
    ASIM_MODULELINK scan = contained;
    while (scan != NULL)
    {
        ASIM_STATE state = scan->module->GetState(stateStr);
        if (state != NULL)
            return(state);

        scan = scan->next;
    }

    //
    // 'stateStr' was not found by any contained module, so check to see if
    // it is an actual state object in this module.  Returns NULL if none
    // is found. 
    //
    return (SearchStates(stateStr));
}

void
ASIM_MODULE_CLASS::PrintModuleStats (STATE_OUT stateOut)
/*
 * Print this modules statistics and then recursively the stats of all
 * contained modules.
 */
{
    stateOut->AddCompound("module", Name());

    DumpStats(stateOut);

    ASIM_MODULELINK scan = contained;
    while (scan != NULL)
    {
        scan->module->PrintModuleStats(stateOut);
        scan = scan->next;
    }

    stateOut->CloseCompound(); // module
}

void
ASIM_MODULE_CLASS::PrintModuleStats(char *filename)
{
    // create a STATE_OUT object for the stats file
    STATE_OUT stateOut = new STATE_OUT_CLASS(filename);

    if (! stateOut) 
    {
        ASIMERROR("Unable to create stats output file \"" << filename
            << "\", " << strerror(errno));
    }
    
    PrintModuleStats(stateOut);	

    // dump stats to file and delete object
    delete stateOut;
}

void
ASIM_MODULE_CLASS::ClearModuleStats ()
/*
 * Clear this module statistics and then recursively the stats of all
 * contained modules.
 */
{

    ClearStats();

    ASIM_MODULELINK scan = contained;
    while (scan != NULL)
    {
        scan->module->ClearModuleStats();
        scan = scan->next;
    }
}

//
// To understand this function, see the color coding explanation in function
// FillUpdate in the tarantula workbench (in $ASIMDIR/config/wb/tarantula.cfg)
//
//
UINT32
ColorCoding(UINT32 nentries, UINT32 hwm, UINT32 max)
{
    if ( nentries == 0 ) return 0;
    if ( nentries == max ) return 4;
    if ( nentries >= hwm ) return 3;
    
    //
    // Two shade colors for how far we are from the HWM
    hwm /= 2;
    if ( nentries >= hwm ) return 2;
    return 1;
}



// ADF node constructor. it creates a linked list among all ADF_NODES.
ASIM_ADF_NODE_CLASS::ASIM_ADF_NODE_CLASS(ASIM_ADF_NODE parent, const char * const name)
{
    descendants = NULL;
    next = NULL;
    if (parent != NULL) {
        next = parent->descendants;
        parent->descendants = this;
    }
}

// ADF node destructor
ASIM_ADF_NODE_CLASS::~ASIM_ADF_NODE_CLASS()
{
}


// Get all general ADF definitions for all nodes
string 
ASIM_ADF_NODE_CLASS::GetWatchWindowADF()
{
    ASIM_ADF_NODE list = descendants;
    
    while (list) {
        watchwindow_adf << list->GetWatchWindowADF();
        list = list->next;
    }    

    return watchwindow_adf.str();
}

// Global list of tags used in the ADF file
set<string> ASIM_ADF_NODE_CLASS::stagList;

// Get all Watch window ADF tag desfriptor definitions for all nodes
string 
ASIM_ADF_NODE_CLASS::GetWatchWindowADFTagDescriptors()
{
    ostringstream adf;

    for(set<string>::iterator i = stagList.begin(); i != stagList.end(); i++) {
        adf << "    <tagdescriptor tagname=\"" << (*i) << "\" type=\"integer\"/>" << endl;
    }

    return adf.str();
}
