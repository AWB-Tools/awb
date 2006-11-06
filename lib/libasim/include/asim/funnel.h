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

#ifndef _FUNNEL_
#define _FUNNEL_

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"


/*
 * Class ASIM_FUNNEL
 *
 * Base class for funnels. Specific instances of funnels
 * should be created using ASIM_FUNNELTEMPLATE template.
 */
typedef class ASIM_FUNNEL_CLASS *ASIM_FUNNEL;
class ASIM_FUNNEL_CLASS {
    private:
        /*
         * User assigned name for the funnel.
         */
        const char * const name;


    public:
        ASIM_FUNNEL_CLASS (const char * const n) : name(n) { }
        
        /*
         * Accessors...
         */
        const char *Name (void) const { return(name); }
                 
};


/*
 * Template ASIM_FUNNELTEMPLATE
 *
 * Template to create funnels. Currently we just use a list that
 * uses O(n) time to insert and constant time to remove.
 *
 * B - (class) type of object held by the funnel
 */
template<class B> class ASIM_FUNNELTEMPLATE :
                  public ASIM_FUNNEL_CLASS {
    private:
        struct funnel_link {
            B data;
            UINT64 ready;
            funnel_link *next;
            funnel_link (B d, UINT64 r, funnel_link *l) : data(d), ready(r), next(l) { }
        };

        /*
         * List of 'B's in funnel, sorted in order of ready cycle.
         */
        funnel_link *funnel;

    public:
        ASIM_FUNNELTEMPLATE (const char * const n) : ASIM_FUNNEL_CLASS(n), funnel(NULL) { }

        /*
         * Attempt to read this funnel. If successful, return true
         * and the value read. Remove the read value from the funnel.
         * A read fails if the funnel is empty or if the head
         * entry is not ready to be read.
         */
        bool Read (B *v, UINT64 cycle) {
            if ((funnel == NULL) || (funnel->ready > cycle))
                return(false);

            funnel_link *head = funnel;
            funnel = funnel->next;
            *v = head->data;
            delete head;
            return(true);
        }

        /*
         * Write to the funnel. Insert 'v' in 'cycle' order. Items inserted
         * with the same 'cycle' must be placed in the list in the order in
         * which they were inserted, i.e. for a given 'cycle' it is FIFO.
         */
        bool Write (B v, UINT64 cycle) {
            if ((funnel == NULL) || (funnel->ready > cycle)) {
                funnel = new funnel_link(v, cycle, funnel);
            }
            else {
                funnel_link *scan = funnel;
                while ((scan->next != NULL) && (scan->next->ready <= cycle)) {
                    scan = scan->next;
                }
            
                scan->next = new funnel_link(v, cycle, scan->next);
            }

            return(true);
        }
};


#endif /* _FUNNEL_ */
