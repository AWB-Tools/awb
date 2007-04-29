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

  private:
    // XXX This method is privated to avoid its usage in the ASIM models code.
    //     We made it privated becase it was causing a memory leak in the
    //     DRAL server live items data base.
    //     Try to inherit from ASIM_MM_CLASS and use MM pointers.
    //     If it does not work, please contact paux.s.cabre@intel.com
    //     and we will try to find a solution
    ASIM_ITEM_CLASS & operator = (const ASIM_ITEM_CLASS & aic)
    {
        ASSERTX(false);
        return *this;
    }

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

    inline UINT32 GetRecId() { return recId; };



    inline void
    OpenEventRec (DRAL_ITEM_DESC_CLASS *rec, UINT32 thread_id, ASIM_ITEM_CLASS *parent=NULL)
    {
        if (runWithEventsOn && eventsEnabled)
        {
            EVENT(
                  UINT32 parent_id = parent ? parent->GetRecId() : 0;
                  
                  recId = DRALEVENT(OpenEventRec(rec, thread_id, parent_id));
                  );
        }
    }

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
