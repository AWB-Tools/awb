/*
 * Copyright (C) 2003-2006 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @author David Goodwin
 * @brief
 */

#ifndef _STATE_
#define _STATE_

// generic
#include <iostream>
#include <sstream>
#include <string>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/module.h"
#include "asim/resource_stats.h"
#include "asim/ioformat.h"
#include "asim/stateout.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

extern bool statsOn; 
extern bool warningsOn; 

typedef class ASIM_STATE_CLASS *ASIM_STATE;
typedef class ASIM_STATELINK_CLASS *ASIM_STATELINK;
typedef class STATE_ITERATOR_CLASS *STATE_ITERATOR;


/*
 * ASIM_STATETYPE
 *
 * Type of state values.
 */
enum ASIM_STATETYPE
{
    STATE_UINT,
    STATE_FP, 
    STATE_STRING, 
    STATE_HISTOGRAM,
    STATE_RESOURCE, 
    STATE_THREE_DIM_HISTOGRAM
}; 


/*
 * Class ASIM_STATE
 *
 * Class representing a performance model state that is being exposed.
 *
 */
typedef class ASIM_STATE_CLASS *ASIM_STATE;
class ASIM_STATE_CLASS
{
    private:
        /*
         * Name and description of the state.
         */
        const char * const name;
        const char * const desc;

        /*
         * Path to the state.
         */
        const char * const path;
        
        /*
         * True if state is suspendable. I.e. true if we can restore the
         * state variable to a value that was grabbed earlier.
         */
        const bool suspendable;
        
        /*
         * Number of elements in the state. Will be 1 for a scalar and
         * the size of the array for a vector.
         */
        const UINT32 size;

        /*
         * Type of the state variable.
         */
        const ASIM_STATETYPE type;

        /*
         * Pointer to the state variable.
         */
        union
        {
            UINT64 *iPtr;
            double *fPtr;
            string *sPtr;
            HISTOGRAM_TEMPLATE<true> * hPtr;
            THREE_DIM_HISTOGRAM_TEMPLATE<true> * tdhPtr;
            RESOURCE_TEMPLATE<true> * rPtr;
        } u;

        /*
         * True if the state variable is currently suspended.
         */
        bool suspended;

        /*
         * Save area to hold the state variable's value at the time
         * when it is suspended. When the variable is un-suspended
         * we set it's value to the saved value.
         */
        UINT32 saveSz;
        void *save;
        void *initial_values_save;
        
        /*
         * Utility routines to add an array of ints or floats.
         */
        UINT64 SumIntArray (UINT64 *vec, UINT32 sz) const
        {
            ASSERTX(sz > 0);
            UINT64 sum = 0;
            for (; sz > 0; sz--)
                sum += vec[sz-1];
            return(sum);
        }

        double SumFpArray (double *vec, UINT32 sz) const
        {
            ASSERTX(sz > 0);
            double sum = 0;
            for (; sz > 0; sz--)
                sum += vec[sz-1];
            return(sum);
        }

        /*
         * Save the state variables current value in 'save'.
         */
        void SaveValue (void)
        {
            if (type == STATE_UINT) 
                memcpy(save, u.iPtr, saveSz);
            else if (type == STATE_FP)
                memcpy(save, u.fPtr, saveSz);
            else if (type == STATE_STRING)
                *((string *)save) = *(u.sPtr);
            else if (type == STATE_HISTOGRAM)
            {
                *((HISTOGRAM_TEMPLATE<true> *)save) = *(u.hPtr);
            }
            else if (type == STATE_THREE_DIM_HISTOGRAM)
            {
                *((THREE_DIM_HISTOGRAM_TEMPLATE<true> *)save) = *(u.tdhPtr);
            }
            else if (type == STATE_RESOURCE)
            {
                *((RESOURCE_TEMPLATE<true> *)save) = *(u.rPtr);
            }
            else
                ASSERTX(false);
        }
        
        void SaveInitialValue (void)
        {
            if (type == STATE_UINT) 
                memcpy(initial_values_save, u.iPtr, saveSz);
            else if (type == STATE_FP)
                memcpy(initial_values_save, u.fPtr, saveSz);
            else if (type == STATE_STRING)
                *((string *)initial_values_save) = *(u.sPtr);
            else if (type == STATE_HISTOGRAM)
            {
                // Not saved
            }
            else if (type == STATE_THREE_DIM_HISTOGRAM)
            {
                // Not saved
            }
            else if (type == STATE_RESOURCE)
            {
                // Not saved
            }
            else
                ASSERTX(false);
        }        

        /*
         * Restore the state variables current value from 'save'.
         */
        void RestoreValue (void)
        {
            if (type == STATE_UINT) 
                memcpy(u.iPtr, save, saveSz);
            else if (type == STATE_FP)
                memcpy(u.fPtr, save, saveSz);
            else if (type == STATE_STRING)
                *(u.sPtr) = *((string *)save);
            else if (type == STATE_HISTOGRAM)
            {
                *(u.hPtr) = *((HISTOGRAM_TEMPLATE<true> *)save);
            }
            else if (type == STATE_THREE_DIM_HISTOGRAM)
            {
                *(u.tdhPtr) = *((THREE_DIM_HISTOGRAM_TEMPLATE<true> *)save);
            }
            else if (type == STATE_RESOURCE)
            {
                *(u.rPtr) = *((RESOURCE_TEMPLATE<true> *)save);
            }
            else
                ASSERTX(false);
        }

    public:
        
        ASIM_STATE_CLASS (UINT64 *s, const char * const n,
                          const char * const d, const char * const p, 
                          const bool sus) :
            name(strdup(n)), desc(strdup(d)), path(strdup(p)), 
            suspendable(sus), size(1), type(STATE_UINT), suspended(false)
        {
            u.iPtr = s;
            saveSz = sizeof(UINT64)*size;
            save = new char[saveSz];
            initial_values_save = new char[saveSz];
            Suspend();
            SaveInitialValue();
        }

        ASIM_STATE_CLASS (UINT64 *s, const UINT32 sz, const char * const n,
                          const char * const d, 
                          const char * const p, const bool sus) :
            name(strdup(n)), desc(strdup(d)), path(strdup(p)), 
            suspendable(sus), size(sz), type(STATE_UINT), suspended(false)
        {
            u.iPtr = s;
            saveSz = sizeof(UINT64)*size;
            save = new char[saveSz];
            initial_values_save = new char[saveSz];
            Suspend();
            SaveInitialValue();
        }

        ASIM_STATE_CLASS (double *s, const char * const n,
                          const char * const d, const char * const p, const bool sus) :
            name(strdup(n)), desc(strdup(d)), path(strdup(p)), suspendable(sus), size(1), type(STATE_FP), suspended(false)
        {
            u.fPtr = s;
            saveSz = sizeof(double)*size;
            save = new char[saveSz];
            initial_values_save = new char[saveSz];
            Suspend();
            SaveInitialValue();
        }

        ASIM_STATE_CLASS (double *s, const UINT32 sz, const char * const n,
                          const char * const d, const char * const p, const bool sus) :
            name(strdup(n)), desc(strdup(d)), path(strdup(p)), suspendable(sus), size(sz), type(STATE_FP), suspended(false)
        {
            u.fPtr = s;
            saveSz = sizeof(double)*size;
            save = new char[saveSz];
            initial_values_save = new char[saveSz];
            Suspend();
            SaveInitialValue();
        }

        ASIM_STATE_CLASS (string * s, const char * const n,
                          const char * const d, const char * const p, const bool sus) :
        name(strdup(n)), desc(strdup(d)), path(strdup(p)), suspendable(sus), size(1), type(STATE_STRING), suspended(false)
        {
            u.sPtr = s;
            saveSz = sizeof(string)*size;
            save = new string;
            initial_values_save = new string;
            Suspend();
            SaveInitialValue();
        }

       /* 
         * Added a new state class to handle HISTOGRAM data. 
         */
        ASIM_STATE_CLASS (HISTOGRAM_TEMPLATE<true> *s, const char * const n,
                          const char * const d, const char * const p, const bool sus) :
	    name(strdup(n)), desc(strdup(d)), path(strdup(p)), 
	    suspendable(sus), size(1), type(STATE_HISTOGRAM), 
	    suspended(false)
        {
            s->SetName(strdup(n));
            u.hPtr = s;
            saveSz = sizeof(HISTOGRAM_TEMPLATE<true>)*size;
            save =  new HISTOGRAM_TEMPLATE<true> ();
            initial_values_save = NULL;
            Suspend();
        }

       /* 
         * Added a new state class to handle THREE_DIM HISTOGRAM data. 
         */
        ASIM_STATE_CLASS (THREE_DIM_HISTOGRAM_TEMPLATE<true> *s, 
			  const char * const n,
                          const char * const d, 
			  const char * const p, 
			  const bool sus) :
        name(strdup(n)), desc(strdup(d)), path(strdup(p)), 
        suspendable(sus), size(1), type(STATE_THREE_DIM_HISTOGRAM), 
        suspended(false)
        {
            s->SetName(strdup(n));
            u.tdhPtr = s;
            saveSz = sizeof(THREE_DIM_HISTOGRAM_TEMPLATE<true>)*size;
            save =  new THREE_DIM_HISTOGRAM_TEMPLATE<true> ();
            initial_values_save = NULL;
            Suspend();
        }

       /*
         * Added a new state class to handle RESOURCE_STATS. 
         */
        ASIM_STATE_CLASS (RESOURCE_TEMPLATE<true> *s, const char * const n,
                          const char * const d, const char * const p, const bool sus) :
	    name(strdup(n)), desc(strdup(d)), path(strdup(p)), 
	    suspendable(sus), size(1), type(STATE_RESOURCE), 
	    suspended(false)
        {
            u.rPtr = s;
            saveSz = sizeof(RESOURCE_TEMPLATE<true>)*size;
            save =  new RESOURCE_TEMPLATE<true> ();
            initial_values_save = NULL;
            Suspend();
        }

        // free what we have allocated
        ~ASIM_STATE_CLASS ()
        {
            if (name)
            {
                free (const_cast<char*> (name));
            }
            if (desc)
            {
                free (const_cast<char*> (desc));
            }
            if (path)
            {
                free (const_cast<char*> (path));
            }
            if (save)
            {
                // yikes - we need to differentiate freeing by type
                switch (type)
                {
                  case STATE_UINT:
                    delete [] (char*) save;
                    break;
                  case STATE_FP:
                    delete [] (char*) save;
                    break;
                  case STATE_STRING:
                    delete (string*) save;
                    break;
                  case STATE_HISTOGRAM:
                  {
                    HISTOGRAM_TEMPLATE<true> * ptr =
                        (HISTOGRAM_TEMPLATE<true> *) save;
                    // note: we need to warn this 'save' object about its
                    // impeding delete (pointer freeing issues)
                    ptr->PrepareSaveDelete();
                    delete ptr;
                    break;
                  }
                  case STATE_THREE_DIM_HISTOGRAM:
                  {
                    THREE_DIM_HISTOGRAM_TEMPLATE<true> * ptr =
                        (THREE_DIM_HISTOGRAM_TEMPLATE<true> *) save;
                    // note: we need to warn this 'save' object about its
                    // impeding delete (pointer freeing issues)
                    ptr->PrepareSaveDelete();
                    delete ptr;
                    break;
                  }
                  case STATE_RESOURCE:
                  {
                    RESOURCE_TEMPLATE<true> * ptr =
                        (RESOURCE_TEMPLATE<true> *) save;
                    // note: we need to warn this 'save' object about its
                    // impeding delete (pointer freeing issues)
                    ptr->PrepareSaveDelete();
                    delete ptr;
                    break;
                  }
                }
            }
            if (initial_values_save)
            {
                switch (type)
                {
                  case STATE_UINT:
                  case STATE_FP:
                    delete [] (char*) initial_values_save;
                    break;
                  case STATE_STRING:
                    delete (string*) initial_values_save;
                    break;
                  default: ;
                }
            }
        }

        /*
         * Accessors...
         */
        const char * const Name (void) const { return(name); }
        const char * const Description (void) const { return(desc); }
        const char * const Path (void) const { return(path); }
        bool Suspended (void) const { return(suspended); }
        UINT32 Size (void) const { return(size); }
        ASIM_STATETYPE Type (void) const { return(type); }

        /*
         * Update the variable's value to be its unsuspended value. If the
         * variable is currently unsuspended, then its value is correct,
         * and we don't need to do anything. If the variable is currently
         * suspended, then the variable's correct value is in 'save'. The
         * variable's value could be incorrect because the performance
         * model may keep modifying it even though the variable is
         * suspended (recall that we copied the value into 'save' when the
         * variable was suspended so that the performance model *could*
         * keep modifying the variable and not have to check everytime to
         * see if it was suspended or not). Thus, if the variable is
         * suspended, we need to copy 'save' into the variable.
         */
        void Update (void)
        {
            if (suspended)
                RestoreValue();
        }
         
        /*
         * Suspend the state variable. To do this we simply save the current
         * value of the variable. Then when the variable is unsuspended, we
         * restore it using the saved value.
         */
        void Suspend (void)
        {
            if (!suspended && suspendable) {
                suspended = true;
                SaveValue();
            }
        }

        /*
         * Unsuspend the state variable by restoring the previously saved value.
         */
        void Unsuspend (void)
        {
            if (suspended) {
                suspended = false;
                RestoreValue();
            }
        }

        /* 
         * Dump information about a particuluar state variable.
         * The dump function  is dependent on the type of variable. 
         */
        void DumpValue(STATE_OUT stateOut)
        {
            if (type == STATE_UINT)
            {
                if (Size() == 1)
                {
                    stateOut->AddScalar("uint", Name(), Description(),
                        u.iPtr[0]);
                }
                else
                {
                    stateOut->AddVector<UINT64*>("uint", Name(), Description(),
                        & u.iPtr[0], & u.iPtr[Size()]);
                }
            }
            else if (type == STATE_FP)
            {
                // fmt(".2f")
                if (Size() == 1)
                {
                    stateOut->AddScalar("double", Name(), Description(),
                        u.fPtr[0]);
                }
                else
                {
                    stateOut->AddVector<double*>("double", Name(), Description(),
                        & u.fPtr[0], & u.fPtr[Size()]);
                }
            }
            else if (type == STATE_STRING)
            {
                    stateOut->AddScalar("string", Name(), Description(), *u.sPtr);
            }
            else if (type == STATE_HISTOGRAM)
            {
                stateOut->AddCompound("histogram", Name(), Description());
                u.hPtr->Dump(stateOut);
                stateOut->CloseCompound();
            }
            else if (type == STATE_THREE_DIM_HISTOGRAM)
            {
                stateOut->AddCompound("3D histogram", Name(), Description());
                u.tdhPtr->Dump(stateOut);
                stateOut->CloseCompound();
            }
            else if (type == STATE_RESOURCE)
            {
                stateOut->AddCompound("resource", Name(), Description());
                u.rPtr->Dump(stateOut);
                stateOut->CloseCompound();
            }
            else
            {
                ASSERTX(false);
            }
        }
        
        /* 
         * Clear values of state variables.
         * WARNING: The initial value is assumed to be 0!
         */
        void ClearStats()
        {
            if (type == STATE_UINT)
            {
                memcpy(u.iPtr, initial_values_save, saveSz);
            }
            else if (type == STATE_FP)
            {
                memcpy(u.fPtr, initial_values_save, saveSz);
            }
            else if (type == STATE_STRING)
            {
                *(u.sPtr) = *((string *)initial_values_save);
            }
            else if (type == STATE_HISTOGRAM)
            {
                u.hPtr->ClearValues();
            }
            else if (type == STATE_THREE_DIM_HISTOGRAM)
            {
                u.tdhPtr->ClearValues();
            }
            else if (type == STATE_RESOURCE)
            {
                u.rPtr->ClearValues();
            }
            else
            {
                ASSERTX(false);
            }
        }

        /*
          * Return the state's value as either an integer or a float.
          * For an array state, we add all the entries and return that value.
          */
        UINT64 IntValue (void) const
        {
            return((type == STATE_UINT) ? SumIntArray(u.iPtr, size) :
                                          (UINT64)SumFpArray(u.fPtr, size));
        }
        double FpValue (void) const
        {
            return((type == STATE_UINT) ? (double)SumIntArray(u.iPtr, size) :
                                          SumFpArray(u.fPtr, size));
        }

        /*
         * For vector array's return the value of the specified element.
         * (actually this can be called for scalar states too, as long as
         * 'el' is zero.
         */
        UINT64 IntValue (UINT32 el) const
        {
            ASSERTX(el < size);
            return((type == STATE_UINT) ? u.iPtr[el] : (UINT64)(u.fPtr[el]));
        }
        double FpValue (UINT32 el) const
        {
            ASSERTX(el < size);
            return((type == STATE_UINT) ? (double)(u.iPtr[el]) : (u.fPtr[el]));
        }
        
};


/*
 * Class ASIM_STATELINK
 *
 * Simple class implementing a link in a list of state objects.
 */
typedef class ASIM_STATELINK_CLASS *ASIM_STATELINK;
class ASIM_STATELINK_CLASS
{
    public:
        
        ASIM_STATE state;
        ASIM_STATELINK next;
        bool ownState;       ///< we 'own' the state object
    
        ASIM_STATELINK_CLASS ( ASIM_STATE s, ASIM_STATELINK l, bool owner)
          : state(s), next(l), ownState(owner) { }
          
        ~ASIM_STATELINK_CLASS ()
        {
            // if the link object 'owns' the associated state, it must
            // also delete the state when it is destructed
            if (ownState && state)
            {
                delete state;
            }
        }
};


/*
 * Class STATE_ITERATOR_CLASS
 *
 * An iterator over state objects and modules.
 */
typedef class STATE_ITERATOR_CLASS *STATE_ITERATOR;
class STATE_ITERATOR_CLASS
{
    private:
        /*
         * "Top" module we are iterating from. We will iterate through
         * the state in this module and optionally through all the state of
         * modules contained within this one.
         */
        ASIM_MODULE module;

        /*
         * Pointer into the list of modules contained within 'module'
         * indicating the next contained module we need to visit.
         */
        ASIM_MODULELINK nextContained;

        /*
         * Pointer to the state item in this module that we are currently
         * visiting.
         */
        ASIM_STATELINK nextState;

        /*
         * If we are iterating through the contained modules, 'cIter' is
         * the iterator that is currently recursing a contained module.
         */
        STATE_ITERATOR cIter;

    public:
        /*
         * Initialize the iterator to start in module 'm'. If 'r' is true
         * then we also iterate through state of all contained modules.
         */
        STATE_ITERATOR_CLASS (ASIM_MODULE m, bool r) : module(m)
        {
            nextContained = module->contained;
            nextState = module->states;
            if (r && (nextContained != NULL))
            {
                cIter = new STATE_ITERATOR_CLASS(nextContained->module, r);
                nextContained = nextContained->next;
            }
            else
                cIter = NULL;
        }
        ~STATE_ITERATOR_CLASS ()
        {
            delete cIter;
        }

        /*
         * Return the next state. We first return the state in 'module',
         * and then visit each contained module (if 'recurse' is
         * true). Return NULL when done.
         */
        ASIM_STATE Next (void )
        {
            //
            // Visit 'nextState' until it is NULL.

            if (nextState != NULL)
            {
                ASIM_STATE state = nextState->state;
                nextState = nextState->next;
                return(state);
            }

            //
            // If 'cIter' is non-NULL then we have a contained module that
            // can potentially provide our next state object.

            while (cIter != NULL)
            {
                //
                // If 'cIter' has no more state, then we move to the next
                // contained module.
                
                ASIM_STATE state = cIter->Next();
                if (state != NULL)
                    return(state);

                delete cIter;
                cIter = NULL;
                
                //
                // If we don't have any more contained modules, then we are
                // done iterating.

                if (nextContained != NULL)
                {
                    cIter = new STATE_ITERATOR_CLASS(nextContained->module, true);
                    nextContained = nextContained->next;
                }
            }

            return(NULL);
        }
        
};

#endif /* _STATE_ */
