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
 * @author David Goodwin, Roger Gramunt
 * @brief Base class for ASIM module abstraction.
 */

#ifndef _MODULE_
#define _MODULE_

// generic
#include <errno.h>
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/except.h"
#include "asim/registry.h"
#include "asim/stateout.h"
#include "asim/event.h"
#include "asim/item.h"
#include "asim/trace.h"
#include "asim/clockable.h"

using namespace std;

typedef class ASIM_MODULELINK_CLASS *ASIM_MODULELINK;
typedef class ASIM_MODULE_CLASS *ASIM_MODULE;
typedef class ASIM_SYSTEM_CLASS *ASIM_SYSTEM;

/*
 * MODULE_MAX_PATH
 *
 * The maximum heirarchical name of any module (e.g. /uni/cpu/ibox/...)
 */
#define MODULE_MAX_PATH        256


/*
 * Class ASIM_MODULELINK
 *
 * Simple class implementing a link in a list
 * of modules.
 */
typedef class ASIM_MODULELINK_CLASS *ASIM_MODULELINK;
class ASIM_MODULELINK_CLASS
{
    public:
        ASIM_MODULE module;
        ASIM_MODULELINK next;
    
        ASIM_MODULELINK_CLASS (ASIM_MODULE m, ASIM_MODULELINK l) :
	  module(m), next(l) { }

};

class ASIM_DRAL_NODE_CLASS
{
  public:
    explicit ASIM_DRAL_NODE_CLASS(ASIM_MODULE parent, const char* const name);
    virtual ~ASIM_DRAL_NODE_CLASS(){};

	UINT16      GetUniqueId(void) const {return (uniqueId); }

   /* 
    * Some events that an asim mudule may generate.
    * All of them, hide the use of event identifiers (like module 
    * uniqueId ).
    */

    //inline UINT16 NewNode(
    //   const char* const name, ASIM_MODULE parent)
    //{
    //    UINT16 uniqueId = 0;
    //    if (runWithEventsOn)
    //    {
    //        EVENT(uniqueId = DRALEVENT(NewNode(name, (parent ? parent->GetUniqueId() : 0), 0, true)));
    //    }
    //    return uniqueId;
    //}

    inline void SetNodeLayout(
       UINT16 dimensions, UINT32 capacity [], bool persistent=true)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeLayout(
                          uniqueId, dimensions, capacity,persistent));
        }
    }

    inline void SetNodeLayout(UINT32 capacity, bool persistent=true)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeLayout(uniqueId, capacity,persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT64 value, bool persistent=false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, 0, NULL, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], INT64 value, bool persistent=false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, 0, NULL, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT32 value, bool persistent=false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, 0, NULL, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], INT32 value, bool persistent=false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, 0, NULL, persistent));
        }

    }

    inline void SetNodeTag(
        const char tagName [], const char str [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, str, 0, NULL, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], char character, bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, character, 0, NULL, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT16 n, UINT64 set [],
        bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, n, set, 0, NULL, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT64 value,
        UINT32 position, bool persistent = false)
    {
        if (runWithEventsOn)
        {
            UINT32 lst[1] = { position };

            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, 1, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT64 value,
        UINT32 position1, UINT32 position2, bool persistent = false)
    {
        if (runWithEventsOn)
        {
            UINT32 lst[2] = { position1, position2 };

            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, 2, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT64 value,
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, level, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], INT64 value,
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, level, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT32 value,
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, level, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], INT32 value,
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, value, level, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], const char str [],
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, str, level, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], char character,
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, character, level, lst, persistent));
        }
    }

    inline void SetNodeTag(
        const char tagName [], UINT16 n, UINT64 set [],
        UINT16 level, UINT32 lst [], bool persistent = false)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeTag(
                          uniqueId, tagName, n, set, level, lst, persistent));
        }
    }

    inline void EnterNode (
       ASIM_ITEM item, UINT32 position, 
       bool persistent=false)
    {
        if (runWithEventsOn && active && item->GetEventsEnabled())
        {
           UINT32 positions[1] = {position};
           DRALEVENT(EnterNode(
                          uniqueId, item->GetItemId(),1,positions,persistent));
        }
    }

    inline void EnterNode (
       ASIM_ITEM_CLASS item, UINT32 position,
       bool persistent=false)
    {
        if (runWithEventsOn && active && item.GetEventsEnabled())
        {
           UINT32 positions[1] = {position};
           DRALEVENT(EnterNode(
                          uniqueId, item.GetItemId(),1,positions,persistent));
        }
    }

    inline void EnterNode (
       ASIM_ITEM item, UINT32 position1, UINT32 position2,
       bool persistent=false)
    {
        if (runWithEventsOn && active && item->GetEventsEnabled())
        {
           UINT32 positions[2] = { position1, position2 };
           DRALEVENT(EnterNode(
                          uniqueId, item->GetItemId(), 2, positions, persistent));
        }
    }

    inline void EnterNode (
       ASIM_ITEM_CLASS item, UINT32 position1, UINT32 position2,
       bool persistent=false)
    {
        if (runWithEventsOn && active && item.GetEventsEnabled())
        {
           UINT32 positions[2] = {position1, position2};
           DRALEVENT(EnterNode(
                          uniqueId, item.GetItemId(),2,positions,persistent));
        }
    }

    inline void EnterNode (
       ASIM_ITEM item, UINT16 dimensions, UINT32 position[],
       bool persistent=false)
    {
        if (runWithEventsOn && active && item->GetEventsEnabled())
        {
            DRALEVENT(EnterNode(
                          uniqueId, item->GetItemId(),dimensions,position,persistent));
        }
    }

    inline void EnterNode (
       ASIM_ITEM_CLASS item, UINT16 dimensions, UINT32 position[],
       bool persistent=false)
    {
        if (runWithEventsOn && active && item.GetEventsEnabled())
        {
            DRALEVENT(EnterNode(
                          uniqueId, item.GetItemId(),dimensions,position,persistent));
        }
    }

    inline void ExitNode (
       ASIM_ITEM item, UINT32 position, 
       bool persistent=false)
    {
        if (runWithEventsOn && active && item->GetEventsEnabled())
        {
           UINT32 positions[1] = {position};
           DRALEVENT(ExitNode(
                          uniqueId, item->GetItemId(),1,positions,persistent));
        }
    }

    inline void ExitNode (
       ASIM_ITEM_CLASS item, UINT32 position,
       bool persistent=false)
    {
        if (runWithEventsOn && active && item.GetEventsEnabled())
        {
           UINT32 positions[1] = {position};
           DRALEVENT(ExitNode(
                          uniqueId, item.GetItemId(),1,positions,persistent));
        }
    }

    inline void ExitNode (
       ASIM_ITEM item, UINT32 position1, UINT32 position2, 
       bool persistent=false)
    {
        if (runWithEventsOn && active && item->GetEventsEnabled())
        {
           UINT32 positions[2] = { position1, position2 };
           DRALEVENT(ExitNode(
                          uniqueId, item->GetItemId(), 2, positions, persistent));
        }
    }

    inline void ExitNode (
       ASIM_ITEM_CLASS item, UINT32 position1, UINT32 position2, 
       bool persistent=false)
    {
        if (runWithEventsOn && active && item.GetEventsEnabled())
        {
           UINT32 positions[2] = {position1, position2};
           DRALEVENT(ExitNode(
                          uniqueId, item.GetItemId(),2,positions,persistent));
        }
    }

    inline void ExitNode (
       ASIM_ITEM item, UINT16 dimensions, UINT32 position[],
       bool persistent=false)
    {
        if (runWithEventsOn && active && item->GetEventsEnabled())
        {
            DRALEVENT(ExitNode(
                          uniqueId, item->GetItemId(),dimensions,position,persistent));
        }
    }

    inline void ExitNode (
       ASIM_ITEM_CLASS item, UINT16 dimensions, UINT32 position[],
       bool persistent=false)
    {
        if (runWithEventsOn && active && item.GetEventsEnabled())
        {
            DRALEVENT(ExitNode(
                          uniqueId, item.GetItemId(),dimensions,position,persistent));
        }
    }

    inline void SetNodeInputBandwidth (UINT32 bandwidth, bool persistent=true)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeInputBandwidth(uniqueId, bandwidth, persistent));
        }
    }

    inline void SetNodeOutputBandwidth (UINT32 bandwidth, bool persistent=true)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetNodeOutputBandwidth(uniqueId, bandwidth, persistent));
        }
    }

    inline void SetTagDescription (
        const char tag [], const char desc [], bool persistent=true)
    {
        if (runWithEventsOn)
        {
            DRALEVENT(SetTagDescription(tag,desc,persistent));
        }
    }

    void
    ActivateEvents()
    {
        active = true;
    }

    void
    DeactivateEvents()
    {
        active = false;
    }

    bool
    AreEventsActivated()
    {
        return active;
    }
         
  private:
    UINT16 uniqueId;
    bool active;
};


/*
 * Class ASIM_ADF_NODE
 *
 * Class encapsulation all automathic ADF genarion function for modules.
 * ADF files are used by dreams visulaization toolkit in order to know how to visualize
 * DRAl events. The aim of this class is to auctomatically generate some default portions of this
 * file.
 * Right now, this class only perfomes a subset of tasks related to watchwindow adf 
 * sections.
 *
 */
typedef class ASIM_ADF_NODE_CLASS *ASIM_ADF_NODE;
class ASIM_ADF_NODE_CLASS 
{
  public:

    static set<string> stagList; // Holds a list of all tag names that will be used in the for rules in the ADF.

    // All ADF modules grouped toguether in a list
    ASIM_ADF_NODE descendants;
    ASIM_ADF_NODE next;

    // holds the ADF rules related to the wathcwindow section
    ostringstream watchwindow_adf;


    /// Funcions
    ASIM_ADF_NODE_CLASS(ASIM_ADF_NODE parent, const char * const name);
    
    virtual ~ASIM_ADF_NODE_CLASS();
    
    // Get all general ADF definitions for all nodes
    virtual string GetWatchWindowADF();  
    
    // Get all Watch window ADF tag desfriptor definitions for all nodes
    string GetWatchWindowADFTagDescriptors();
};


/*
 * Class ASIM_MODULE
 *
 * Base class representing common module functionality.
 *
 */
typedef class ASIM_MODULE_CLASS *ASIM_MODULE;
class ASIM_MODULE_CLASS : public ASIM_REGISTRY_CLASS, public ASIM_DRAL_NODE_CLASS, public ASIM_CLOCKABLE_CLASS, public ASIM_ADF_NODE_CLASS
{

        /*
         * The state and event iterators must read the contained modules
         * list and the contained states/events.
         */
         friend class STATE_ITERATOR_CLASS;

   private:
        /*
         * User specified name for this module, and the full pathname
         * for this module.
         */
        const char * const name;
        char *path;
        
        /*
         * Module that contains this one, or NULL if this
         * is the top-level module.
         */
        ASIM_MODULE parent;

        /*
         * Exception module to send exceptions raised in this module.
         */
        ASIM_EXCEPT eMod;

        /*
         * List of modules contained within this module.
         */
        ASIM_MODULELINK contained;

        /*
         * System, derived from walking back through the module hierarchy.
         */
        ASIM_SYSTEM system;

        /*
         * Module ID (set by the user). This is used currently to
         * specify the PROCESSING ELEMENT ID of each cpu in a
         * multiprocessor system.  */
        UINT16		module_id;

    protected:
        /*
	 * a thread object, if this module wants to run in parallel.
	 * This is left protected, so that derived classes can allocate
	 * threads in non-standard ways if they choose.
	 */
	ASIM_SMP_THREAD_HANDLE thread;

    public:
        ASIM_MODULE_CLASS (ASIM_MODULE p, const char * const n,
			   ASIM_EXCEPT e = NULL, bool create_thread = false);

        virtual ~ASIM_MODULE_CLASS ();

        /*
         * Accessors...
         */
        const char *Name (void) const { return(name); }
        const char *ProfileId(void) const { return(path); } 
        const char *Path (void) const { return(path); }
        ASIM_EXCEPT Except (void) { ASSERTX(eMod != NULL);  return(eMod); }
        ASIM_MODULE GetParent(void) const { return(parent); }
        UINT16      GetId (void) const { return(module_id); }

        /*
         * Modifiers
         */
        void	   SetId(UINT16 id)	{ module_id = id; }
    
        /* 
         * Do an initialization of the module once the connections
         * between modules have been established. The idea is that in
         * order to create a box you will follow these steps
         *  1- new all your subboxes
         *  2- CombineBuffers and ConnectBuffers
         *  3- When your InitModule() routine is called, then you invoke
         *     InitModule() for each of your sub-boxes
         *  4- If any of your sub-boxes returns 'false' print a
         *  message and also return 'false' to your invoker so that he
         *  will know that you did not succeed in init-ing your box.
         */
        virtual bool InitModule (void)
        {
            cout << "Init Module " << Name()
                  << ": (default empty method): OK" << endl;
            return true;
        }


        /*
         * Handle exceptions sent to this module.
         */
        virtual void HandleException (ASIM_EXCEPTDATA data, UINT64 cycle)
        {
            VERIFY(false, "Module needs exception handler if it registers "
		   "for exceptions.\n");
        }

        /*
         * Return the ASIM_STATE object for the state specified
         * by 'stateStr'. Return NULL is no state exists.
         */
        ASIM_STATE GetState (const char *stateStr);

        /*
         * Print this module statistics and then recursively the stats
         * of all contained modules.
         */
        virtual void PrintModuleStats (STATE_OUT stateOut);
        virtual void PrintModuleStats (char *filename);

        virtual void DumpFunctionalState(ostream& out)
        {
           //for now we will assert if we ever call this on a module with
           //the function defined. This might be a NOP in the future since
           //we may not want to have this defined on each module
           ASSERT(false,"This module does not have a dump function");
        }
        virtual void LoadFunctionalState(istream& in)
        {
           //for now we will assert if we ever call this on a module with
           //the function defined. This might be a NOP in the future since
           //we may not want to have this defined on each module
           ASSERT(false,"This module does not have a load function");
        }
        /*
         * Clear this module statistics and then recursively the stats
         * of all contained modules.
         */
        virtual void ClearModuleStats ();
        

        /*                                                                                                               
         * GetSystem() allows any child of the system module to find the system                                          
         * object.  The system module will redeclare its own copy of GetSystem()                                         
         * that returns the handle.                                                                                      
         *                                                                                                               
         * This makes it possible to pass system state down through the tree                                             
         * without having to change all the interfaces each time a new piece                                             
         * of state is added.                                                                                            
         */
        virtual ASIM_SYSTEM GetSystem(void)
        {
            //                                                                                                           
            // It would have been nice to use a dynamic_cast<> here but the                                              
            // target of a dynamic_cast has to be fully defined, which isn't                                             
            // the case for ASIM_SYSTEM here.  It is just an opaque pointer.                                             
            //                                                                                                           

            if (system != NULL)
            {
                return system;
            }

            ASSERT(parent != NULL, "System did not handle module GetSystem() call");
            system = parent->GetSystem();
            return system;
        };


        /*
         * Find the nearest parent module of a specific type.  E.g. to
         * find the system call GetParentOfType<ASIM_SYSTEM>().
         */
        template <class PT> inline PT GetParentOfType(void);
        

        /*                                                                                                               
         * GetHostThread() returns the thread handle, if any, for the module tree
         * we are enclosed in.  This thread handle is typically passed to RegisterClock()
         * to get clock server callbacks in a parallel thread.
         *
         * If this module or one of its ancestors wants to run in a separate thread,
         * it will typically pass create_thread=true to the constructor, and then
	 * all its children will find it using GetHostThread().
         *
         * But derived classes can override this function in order to allocate threads
         * using whatever algorithm they want.  In that case, this function should simply
         * return a handle to the thread this module should run in.
         */
        virtual ASIM_SMP_THREAD_HANDLE GetHostThread(void)
        {
            // if this module allocated a thread itself, return it:
	    if (thread)
            {
                return thread;
            }
	    // otherwise, get the thread from the parent, if any:
            ASIM_MODULE p = GetParent();
            if (p)
            {
                return p->GetHostThread();
            }
            return NULL;
        };
};

template <class PT>
inline
PT ASIM_MODULE_CLASS::GetParentOfType(void)
{
    ASIM_MODULE p = this;
    while (p != NULL)
    {
        //
        // dynamic_cast<> returns NULL if the object is not the right type
        // and non-NULL for a type match...
        //
        PT pt = dynamic_cast<PT>(p);
        if (pt != NULL)
        {
            // Found the requested type
            return pt;
        }

        p = p->GetParent();
    }

    // Didn't find type
    return NULL;    
};


#endif /* _MODULE_ */
