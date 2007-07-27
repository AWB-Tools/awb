/*****************************************************************************
 *
 * @brief Base class for ASIM modules, ports and messages.
 *
 * @author Roger Gramunt 
 * 
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


#ifndef _ITEM_
#define _ITEM_

// ASIM core
#include "asim/syntax.h"
#include "asim/event.h"
#include "asim/mesg.h"

// for var args
#include <stdarg.h>

using namespace std;

// forward declarations
class DRAL_DATA_DESC_CLASS;
class DRAL_EVENT_DESC_CLASS;
class DRAL_ITEM_DESC_CLASS;

/*
 * Class ASIM_ITEM
 *
 * Base class representing common functionality.
 *
 */
typedef class ASIM_ITEM_CLASS *ASIM_ITEM;
class ASIM_ITEM_CLASS 
{
  public:
    // The "explicit" modifier is important in that constructor, since if it is
    // not present, then the basic types (bool, UINT16, etc.) are casted to
    // ASIM_ITEM_CLASS when traveling trough ports (and then automatic events
    // would be generated).
    explicit ASIM_ITEM_CLASS(bool generateEvents = true) :
      recId(0),
      eventsEnabled(generateEvents),
      idGenerated(false)
    {     
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(itemId = DRALEVENT(NewItem()));
            idGenerated = true;
        }
    }

    // This second contructor is not the default one.
    // It does not produce a NewItemEvent.
    explicit ASIM_ITEM_CLASS(UINT32 i, bool generateEvents = true) :
      recId(0),
      eventsEnabled(generateEvents)
    {
        itemId = i;
        idGenerated = true;
    }

    ~ASIM_ITEM_CLASS() 
    { 
        if (runWithEventsOn && idGenerated)
        {
            DRALEVENT_GUARDED(DeleteItem(itemId));
        }
    }

    // we have some AXP structures with entries that we would like to associate
    // dral/ptv events.  however, they currently require come copying when
    // new entries are added.  so, rather than explicitly disallow copying of
    // ASIM_ITEMs, i am going to do the following on a copy:
    //   1) lvalue must delete and/or close appropriate records
    //   2) rvalue passes on dral/ptv references to the lvalue
    //   3) rvalue disassociates himself from ids
    //
    // here, we essentially pass on the dral/ptv info from one object to
    // another, and the originating item loses its handles.
    ASIM_ITEM_CLASS & operator = (/*const*/ ASIM_ITEM_CLASS & aic)
    {
        if (runWithEventsOn && idGenerated)
        {
            // kill the current item because we are going to 
            // inherit aic's dral/ptv id
            DRALEVENT_GUARDED(DeleteItem(itemId));
            EVENT(DRALEVENT(CloseEventRec(recId)));

            // inherit from aic
            itemId = aic.itemId;
            idGenerated = aic.idGenerated;
            recId = aic.recId;

            // dissaociate aic from the id's.
            aic.itemId = 0;
            aic.idGenerated = false;
            aic.recId = 0;
        }
	
        return *this;
    }

// OLD   private:
// OLD     // XXX This method is privated to avoid its usage in the ASIM models code.
// OLD     //     We made it privated becase it was causing a memory leak in the
// OLD     //     DRAL server live items data base.
// OLD     //     Try to inherit from ASIM_MM_CLASS and use MM pointers.
// OLD     //     If it does not work, please contact paux.s.cabre@intel.com
// OLD     //     and we will try to find a solution
// OLD     ASIM_ITEM_CLASS & operator = (const ASIM_ITEM_CLASS & aic)
// OLD     {
// OLD         ASSERTX(false);
// OLD         return *this;
// OLD     }

  public:

    void SetEnableEvents(bool generateEvents)
    {
        eventsEnabled = generateEvents && runWithEventsOn;

        if (eventsEnabled && !idGenerated)
        {
            EVENT(itemId = DRALEVENT(NewItem()));
            idGenerated = true;
        }
    }

    UINT64 GetItemId() const { return(itemId);}

    bool GetEventsEnabled() const { return eventsEnabled; };

    //
    // Here we go with the hundreds of ways that a tag can be attached to an
    // item.
    // All of them ecapsulate the use of itemId.
    //
    inline void 
    SetItemTag (char tag_name[], UINT64 value, bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, value,persistent));
        }
    }

    inline void
    SetItemTag (char tag_name[], INT64 value, bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, value,persistent));
        }
    }

    inline void
    SetItemTag (char tag_name[], UINT32 value, bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, value, persistent));
        }
    }

    inline void
    SetItemTag (char tag_name[], INT32 value, bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, value, persistent));
        }
    }

    inline void 
    SetItemTag (
        char tag_name[], const char str[], bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, str, persistent));
        }
    }

    inline void 
    SetItemTag (char tag_name[], char character, bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, character, persistent));
        }
    }

    inline void 
    SetItemTag (
        char tag_name[], UINT32 nval, UINT64 value[], bool persistent=false)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            DRALEVENT(SetItemTag(itemId, tag_name, nval, value, persistent));
        }
    }

    // accessors for the event record id.  it is not terribly wise to play with
    // these, but they are nevertheless provided for portability and power users.
    // dont use these unless you know what you are doing.  there are likely 
    // better ways to do what you are trying to do.  this is the analog for 
    // the ptlib id called Pipe_RecId
    inline void SetRecId(UINT32 x) { recId = x; };
    inline UINT32 GetRecId() { return recId; };

    // open the event record based on the item descriptor, the thread id, and 
    // the optional parent item from which this item was derived.  this is the 
    // analog for the ptlib function pipe_open_record_inst().
    inline UINT32
    OpenEventRec (DRAL_ITEM_DESC_CLASS *rec, UINT32 thread_id, ASIM_ITEM_CLASS *parent=NULL)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  UINT32 parent_id = parent ? parent->GetRecId() : 0;
                  
                  recId = DRALEVENT(OpenEventRec(rec, thread_id, parent_id));
                  );
        }

        return recId;
    }

    // reference an existing event record from another item.  this is used to
    // inherit (or share) event records among different simulation objects.
    // this is the preferred method for sharing when compared to the next
    // method which passes the record id directly.  this is the analog for 
    // the ptlib function pipe_reference_record_inst().    
    inline UINT32
    RefEventRec (ASIM_ITEM_CLASS *item)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  UINT32 rec_id = item ? item->GetRecId() : 0; 
                  recId = DRALEVENT(RefEventRec(rec_id));
                  );
        }

        return recId;
    }

    // reference an existing event record from another rec id.  this is used to
    // inherit (or share) event records among different simulation objects.  the
    // rec_id must be a valid record id from another open event record.  this is 
    // the analog for the ptlib function pipe_reference_record_inst().
    inline UINT32
    RefEventRec (UINT32 rec_id)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  recId = DRALEVENT(RefEventRec(rec_id));
                  );
        }

        return recId;
    }

    // closes an open event record.  if the recId is zero, then nothing happens.
    // also, the closure of the event record depends on the absense of all 
    // references to this record.  for example, if we open an event record and
    // create two references via RefEventRec(), then we need to CloseEventRec()
    // on all three items.  this is the analog for the ptlib function 
    // pipe_close_record_inst().
    inline void
    CloseEventRec ()
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  DRALEVENT(CloseEventRec(recId));
                  );
        }
    }

    // decorate the event record with the PTV-style syntax.  The var args are as
    // follows:
    //
    //  SetItemTag(desc0, value0, 
    //             ..., 
    //             descN, valueN, 
    //             NULL);
    //
    // degenerate case is:
    //
    //  SetItemTag(NULL);
    //     
    // this is the analog for the ptlib function pipe_record_data_va().
    inline void
    SetItemTag (DRAL_DATA_DESC_CLASS *data, ...)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  va_list ap;
                  va_start(ap, data);
                  DRALEVENT(SetItemTag(itemId, recId, data, ap));
                  va_end(ap);
                  );
        }
    }

    // trigger an event with PTV-style syntax.  The var args are as follows:
    //
    //  SetEvent(desc, cycle, dur, desc0, value0, 
    //           ..., 
    //           descN, valueN, 
    //           NULL);
    //
    // degenerate case with no associated data is is:
    //
    //  SetEvent(desc, cycle, duration, NULL);
    //
    // this is the analog for the ptlib function pipe_record_event_time().
    inline void
    SetEvent (DRAL_EVENT_DESC_CLASS *desc, UINT64 cycle, UINT64 duration, DRAL_DATA_DESC_CLASS *data, ...)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  va_list ap;
                  va_start(ap, data);
                  DRALEVENT(SetEvent(itemId, recId, desc, cycle, duration, data, ap));
                  va_end(ap);
                  );
        }
    }

    // trigger an event with PTV-style syntax.  same as above except the 
    // descriptor is found in the descriptor database which is hashed by
    // the action name.  this is the analog for the ptlib function 
    // pipe_record_event_time().
    inline void
    SetEvent (const char *action_name, UINT64 cycle, UINT64 duration, DRAL_DATA_DESC_CLASS *data, ...)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  DRAL_EVENT_DESC_CLASS *desc = DRALEVENT(FindEventDesc(action_name));
                  ASSERTX(desc);
                  
                  va_list ap;
                  va_start(ap, data);
                  DRALEVENT(SetEvent(itemId, recId, desc, cycle, duration, data, ap));
                  va_end(ap);
                  );
        }
    }
    
  protected:

    UINT64 itemId;

    UINT32 recId;

  private:

    bool eventsEnabled;

    bool idGenerated;
};

/*
 * Class ASIM_SILENT_ITEM
 *
 * Same as ASIM_ITEM but it doesn-t produce any
 * kind of event. This class is used to avoid th
 * g++ warning "non-POD type blah, blah".
 * We should use this class when send an object throught
 * a port, but we don't want any event.
 *
 */
typedef class ASIM_SILENT_ITEM_CLASS *ASIM_SILENT_ITEM;
class ASIM_SILENT_ITEM_CLASS 
{
};

#endif /* _ITEM_ */
