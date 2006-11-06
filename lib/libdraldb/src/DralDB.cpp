// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file  DralDB.cpp
  */

#include "asim/DralDB.h"

#define DRALDB_PROCESS_STEP 4096

/**
 * The instance is NULL at the beginning.
 */
DralDB* DralDB::_myInstance=NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
DralDB*
DralDB::getInstance()
{
    if (_myInstance==NULL)
    {
        _myInstance = new DralDB();
    }
    return _myInstance;
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
DralDB::destroy()
{
    if (_myInstance!=NULL)
    {
        delete _myInstance;
    }
}

/**
 * Creator of this class. Gets all the instances and sets the other
 * fields to the default values.
 *
 * @return new object.
 */
DralDB::DralDB()
{
    ItemHandler::initialize();
    dblistener    = DBListener::getInstance();
    trackHeap     = TrackHeap::getInstance();
    itemTagHeap   = ItemTagHeap::getInstance();
    tagDescVector = TagDescVector::getInstance();
    dbConfig      = DBConfig::getInstance();
    dbGraph       = DBGraph::getInstance();
    logMgr        = LogMgr::getInstance();
    strtable      = StrTable::getInstance();
    eventFile     = NULL;
    dralClient    = NULL;
    converter     = NULL;
    numTrackedEdges = 0;
}

/**
 * Destructor of this class. Just close the DRL file.
 *
 * @return destroys the object.
 */
DralDB::~DralDB()
{
    closeDRLFile();
    if(converter != NULL)
    {
        delete converter;
    }
}

// -------------------------------------------------------------------
// -- Initialization Methods
// -------------------------------------------------------------------
/**
 * Opens filename to read a dral trace.
 *
 * @return if the file could be opened.
 */
bool
DralDB::openDRLFile(QString filename)
{
    // If another file was open closes it first.
    if (eventFile!=NULL)
    {
        bool ok = closeDRLFile();
        if (!ok)
        {
            return false;
        }
    }
    // Opens the file.
    eventFile = new QFile(filename);
    Q_ASSERT(eventFile!=NULL);
    // Checks for errors.
    if ( !eventFile->open( IO_ReadOnly ) )
    {
        QString err = "IO Error, unable to read "+filename;
        if (logMgr!=NULL)
        {
            logMgr->addLog(err);
        }
        return false;
    }
    if (logMgr!=NULL)
    {
        logMgr->addLog("Starting to read event file: "+filename+"...");
    }

    // Resets the database.
    reset();

    int fd = eventFile->handle();
    if (dralClient!=NULL)
    {
        delete dralClient;
        dralClient=NULL;
    }
    if(converter != NULL)
    {
        delete converter;
    }

    converter = new DRAL_LISTENER_CONVERTER_CLASS(dblistener);
    dralClient = new DRAL_CLIENT_CLASS(fd, converter, 1024 * 64);
    return (dralClient!=NULL);
}

/**
 * Closes the actual dral trace file.
 *
 * @return if the file could be closed.
 */
bool
DralDB::closeDRLFile()
{
    if (eventFile==NULL)
    {
        return false;
    }
    delete eventFile;
    eventFile=NULL;
    
    // flush any pending itemids...
    dblistener->flush();
    return true;
}

/**
 * Resets the content of the database.
 *
 * @return void.
 */
void
DralDB::reset()
{
    dblistener->reset();
    trackHeap->reset();
    itemTagHeap->reset();
	tagDescVector->reset();
	dbGraph->reset();
	strtable->reset();
	
    numTrackedEdges = 0;
}

// -------------------------------------------------------------------
// -- DRAL Processing Methods
// -------------------------------------------------------------------
/**
 * Process the header of the dral trace file.
 *
 * @return true if the header was loaded correctly.
 */
bool
DralDB::loadDRLHeader()
{
    INT32 result = 1;
    while (
           dblistener->isProcessingDralHeader() &&
           dblistener->getLastProcessedEventOk() &&
           (result == 1)
           )
    {
        result = dralClient->ProcessNextEvent(true,1);
    }
    return (result==1);
}

/**
 * Process the events of the dral trace file.
 *
 * @return true if the events were loaded correctly.
 */
bool
DralDB::processAllEvents()
{
    // prepare listener
    dblistener->setTrackedEdges(numTrackedEdges);
    dblistener->propagateFirstCycle();
    INT32 cmds=1;
    bool ok = true;
    while ((cmds>0) && ok )
    {
        cmds = dralClient->ProcessNextEvent(true,DRALDB_PROCESS_STEP);
        ok = dblistener->getLastProcessedEventOk();
    }
    return (ok && (cmds==0));
}

// -------------------------------------------------------------------
// -- Misc
// -------------------------------------------------------------------
/**
 * Dumps the graph description. Forwards the call to dbgraph.
 *
 * @return void.
 */
void
DralDB::dumpGraphDescription()
{
	// just for debugging
	QString str = dbGraph->getGraphDescription();
	printf("%s\n",str.latin1());
}

/**
 * Gets the graph description. Forwards the call to dbgraph.
 *
 * @return the graph description.
 */
QString
DralDB::getGraphDescription()
{
    return dbGraph->getGraphDescription();
}

/**
 * Dumps the track heap. Forwards the call to track heap.
 *
 * @return void.
 */
void
DralDB::dumpTrackHeap()
{
    trackHeap->dumpTrackHeap();
}
