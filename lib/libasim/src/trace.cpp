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

#include <asim/trace.h>

// Include support for the old trace format.
#include <src/trace_legacy.cpp>

#define LOCK_MUTEX(mtx) if(TRACEABLE_CLASS::enableThreadProtection) { pthread_mutex_lock(&mtx); }
#define UNLOCK_MUTEX(mtx) if(TRACEABLE_CLASS::enableThreadProtection) { pthread_mutex_unlock(&mtx); }

using namespace std;

bool printTraceNames = false;

list<TRACEABLE> *TRACEABLE_CLASS::traceables = 0;
pthread_mutex_t TRACEABLE_CLASS::traceablesMutex = PTHREAD_MUTEX_INITIALIZER;

UNCONDITIONAL_TRACEABLE_CLASS _unconditionalTraceable;
UNCONDITIONAL_TRACEABLE unconditionalTraceable = &_unconditionalTraceable;

TRACEABLE_REGEX_LIST *TRACEABLE_CLASS::regexes = 0;
pthread_mutex_t TRACEABLE_CLASS::regexesMutex = PTHREAD_MUTEX_INITIALIZER;

bool TRACEABLE_CLASS::enableThreadProtection = true;

TRACEABLE_CLASS::TRACEABLE_CLASS() 
{
    if(!traceables) 
    {
        LOCK_MUTEX(traceablesMutex);
        if(!traceables) 
        {
            traceables = new list<TRACEABLE>();
        }
        UNLOCK_MUTEX(traceablesMutex);
    }
    // Append this to the traceables list
    LOCK_MUTEX(traceablesMutex);
    traceables->push_back(this);

    // Store an iterator pointing to me on the list to make removal fast.
    myTraceablesEntry = traceables->end();
    --myTraceablesEntry;
    UNLOCK_MUTEX(traceablesMutex);

    SetTraceOn(false);
    SetTraceLevel(1);
}

TRACEABLE_CLASS::TRACEABLE_CLASS(const TRACEABLE_CLASS& t) 
{
    assert(traceables);
    LOCK_MUTEX(traceablesMutex);
    // Append this to the traceables list
    traceables->push_back(this);

    // Store an interator pointing to me on the list to make removal fast.
    myTraceablesEntry = traceables->end();
    --myTraceablesEntry;
    UNLOCK_MUTEX(traceablesMutex);

    SetTraceOn(t.myTraceOn);
    SetTraceLevel(t.traceLevel);
    SetObjectName(t.objectName);
}

TRACEABLE_CLASS::~TRACEABLE_CLASS() 
{
    assert(traceables);
    assert(*myTraceablesEntry == this);
    LOCK_MUTEX(traceablesMutex);
    traceables->erase(myTraceablesEntry);
    if(traceables->empty()) 
    {
        delete(traceables);
        traceables = NULL;
    }
    UNLOCK_MUTEX(traceablesMutex);
}

void TRACEABLE_CLASS::SetTraceableName(string name) 
{
    SetTraceOn(false);
    SetTraceLevel(1);
    objectName = name;
    if(name == "") 
    {
        cerr << "WARNING: Traceable object has no traceable name." << endl;
    }
    if(regexes) 
    {
        LOCK_MUTEX(regexesMutex);
        // replay any regexes that have already been applied
        for(list<TRACEABLE_REGEX>::iterator iter = regexes->begin(); iter != regexes->end(); iter++) 
        {
            TRACEABLE_REGEX r = *iter;
            EnableByRegex(r->regex, r->level);
        }
        UNLOCK_MUTEX(regexesMutex);
    }
}

void TRACEABLE_CLASS::EnableByRegex(Regex *regex, int level) 
{
    // If this object matches the regular expression, set it's trace level.
    if(regex->match(objectName)) 
    {
        SetTraceOn(true);
        SetTraceLevel(level);
    }
}

bool TRACEABLE_CLASS::EnableTraceByRegex(string regexStr, int level, bool saveRegex) 
{
    // build a regular expression for regex
    Regex *regex = new Regex();
    if(!regex->assign(regexStr.c_str(), false)) 
    {
        delete(regex);
        return(false);
    }
    return(EnableTraceByRegex(regex, level, saveRegex));
}

bool TRACEABLE_CLASS::EnableTraceByRegex(Regex *regex, int level, bool saveRegex) 
{
    assert(traceables);

    // record the regex
    if(saveRegex) 
    {
        if(!regexes) 
        {
            LOCK_MUTEX(regexesMutex);
            if(!regexes) 
            {
                regexes = new TRACEABLE_REGEX_LIST();
            }
            UNLOCK_MUTEX(regexesMutex);
        }
        LOCK_MUTEX(regexesMutex);
        regexes->push_back(new TRACEABLE_REGEX_CLASS(regex, level));
        UNLOCK_MUTEX(regexesMutex);
    }
    
    // Walk through the list of traceable objects.
    LOCK_MUTEX(traceablesMutex);
    for(list<TRACEABLE>::iterator iter = TRACEABLE_CLASS::traceables->begin(); 
        iter != TRACEABLE_CLASS::traceables->end(); iter++) 
    {
        TRACEABLE t = *iter;
        t->EnableByRegex(regex, level);
    }
    UNLOCK_MUTEX(traceablesMutex);
    return(true);
}

void TRACEABLE_CLASS::PrintNames() 
{
    assert(traceables);
    list<string> names;
    // Walk through the list of traceable objects.
    LOCK_MUTEX(traceablesMutex);
    for(list<TRACEABLE>::iterator iter = TRACEABLE_CLASS::traceables->begin();
        iter != TRACEABLE_CLASS::traceables->end(); iter++) 
    {
        TRACEABLE t = *iter;
        names.push_back(t->objectName);
    }
    UNLOCK_MUTEX(traceablesMutex);
    names.sort();
    names.unique();
    for(list<string>::iterator iter = names.begin(); iter != names.end(); iter++) 
    {
        string name = *iter;
        cout << name << endl;
    }
}

UNCONDITIONAL_TRACEABLE_CLASS::UNCONDITIONAL_TRACEABLE_CLASS() 
{
    SetTraceableName("_UNCONDITIONAL_");
    SetTraceOn(true);
    SetTraceLevel(1);
}

TRACEABLE_DELAYED_ACTION_CLASS::TRACEABLE_DELAYED_ACTION_CLASS() 
{
    LOCK_MUTEX(TRACEABLE_CLASS::regexesMutex);
    savedRegexes = TRACEABLE_CLASS::regexes;
    TRACEABLE_CLASS::regexes = 0;
    UNLOCK_MUTEX(TRACEABLE_CLASS::regexesMutex);
    
    // reapply the saved regexes with trace level 0 but
    // don't save the regex
    if(savedRegexes) 
    {
        for(list<TRACEABLE_REGEX>::iterator iter = savedRegexes->begin(); iter != savedRegexes->end(); iter++) 
        {
            TRACEABLE_REGEX r = *iter;
            TRACEABLE_CLASS::EnableTraceByRegex(r->regex, 0, false);
        }
    }
}

TRACEABLE_DELAYED_ACTION_CLASS::TRACEABLE_DELAYED_ACTION_CLASS(string regexStr, int level) 
{
    // build a regular expression for regex
    Regex *regex = new Regex();
    if(regex->assign(regexStr.c_str(), false)) 
    {
        savedRegexes = new TRACEABLE_REGEX_LIST();
        savedRegexes->push_back(new TRACEABLE_REGEX_CLASS(regex, level));
    }
}

void TRACEABLE_DELAYED_ACTION_CLASS::go() 
{
    if(savedRegexes) 
    {
        for(list<TRACEABLE_REGEX>::iterator iter = savedRegexes->begin(); iter != savedRegexes->end(); iter++) 
        {
            TRACEABLE_REGEX r = *iter;
            TRACEABLE_CLASS::EnableTraceByRegex(r);
        }
    }
}
