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
  * @brief
  */

#include "dbtest.h"

//#include <string.h>

// #include <qstring.h>
#include <qstringlist.h>

QString drlFileName;
QString logFileName;
LogMgr* logMgr;
bool dumpItems;
bool dumpTracks;
bool dumpGraph;
bool trackItems;
bool trackCycles;
bool verbose;
bool backProp;
QStringList trackNodeList;
QStringList trackEdgeList;
QStringList trackEnterNodeList;
QStringList trackExitNodeList;
QStringList getItemTagList;
QStringList getTrackTagList;

INT32 cycleTrackId;
DralDB* db;

int main (int argc, char** argv)
{
    initGlobals();
    checkParameters(argc,argv);
    db = DralDB::getInstance();
    openfile();
    loadhdr();
    applyConf();
    readAll();
    applyDumps();
    applyQueries();
    closefile();
    exit(0);
}

bool parseParameters(int argc, char** argv)
{
    int carg = 1;
    if (carg>=argc) return false;

    bool ok = true;
    while (ok && carg<(argc-1))
    {
        ok = parseNextParameter(argc,argv,&carg);
    }
    if (!ok) return false;

    // get file name
    if (carg>=argc) return false;
    drlFileName = QString(argv[carg++]);

    return (carg==argc);
}

bool parseNextParameter(int argc, char** argv, int *idx)
{
    static bool option_l           = false;
    static bool option_dumpgraph   = false;
    static bool option_dumpitems   = false;
    static bool option_dumptracks  = false;
    static bool option_trackitems  = false;
    static bool option_trackcycles = false;
    static bool option_verbose     = false;
    static bool option_bprop       = false;
    int carg = *idx;

    // check -l option
    if (!strcmp(argv[carg],"-l"))
    {
        if (option_l) return false;
        ++carg;
        if (carg>=argc) return false;
        logFileName=argv[carg++];
        *idx = carg;
        option_l = true;
        return true;
    }

    // check -dumpgraph
    if (!strcmp(argv[carg],"-dumpgraph"))
    {
        if (option_dumpgraph) return false;
        dumpGraph = true;
        ++carg;
        *idx = carg;
        option_dumpgraph = true;
        return true;
    }

    if (!strcmp(argv[carg],"-dumpitems"))
    {
        if (option_dumpitems) return false;
        dumpItems = true;
        ++carg;
        *idx = carg;
        option_dumpitems=true;
        return true;
    }

    if (!strcmp(argv[carg],"-dumptracks"))
    {
        if (option_dumptracks) return false;
        dumpTracks = true;
        ++carg;
        *idx = carg;
        option_dumptracks = true;
        return true;
    }

    if (!strcmp(argv[carg],"-verbose"))
    {
        if (option_verbose) return false;
        verbose = true;
        ++carg;
        *idx = carg;
        option_verbose=true;
        return true;
    }

    if (!strcmp(argv[carg],"-trackItemTags"))
    {
        if (option_trackitems) return false;
        ++carg;
        if (carg>=argc) return false;
        char* value =argv[carg++];
        *idx = carg;
        if (!strcasecmp(value,"false"))
        {
            trackItems = false;
        }
        else if (!strcasecmp(value,"true"))
        {
            trackItems = true;
        }
        else
        {
            return false;
        }
        option_trackitems = true;
        return true;
    }

    if (!strcmp(argv[carg],"-trackCycleTags"))
    {
        if (option_trackcycles) return false;
        ++carg;
        if (carg>=argc) return false;
        char* value =argv[carg++];
        *idx = carg;
        if (!strcasecmp(value,"false"))
        {
            trackCycles = false;
        }
        else if (!strcasecmp(value,"true"))
        {
            trackCycles = true;
        }
        else
        {
            return false;
        }
        option_trackcycles = true;
        return true;
    }

    if (!strcmp(argv[carg],"-itemTagBackPropagate"))
    {
        if (option_bprop) return false;
        ++carg;
        if (carg>=argc) return false;
        char* value =argv[carg++];
        *idx = carg;
        if (!strcasecmp(value,"false"))
        {
            backProp = false;
        }
        else if (!strcasecmp(value,"true"))
        {
            backProp = true;
        }
        else
        {
            return false;
        }
        option_bprop = true;
        return true;
    }


    if (!strcmp(argv[carg],"-trackNode"))
    {
        ++carg;
        if (carg>=argc) return false;
        trackNodeList += QString(argv[carg++]);
        *idx = carg;
        return true;
    }

    if (!strcmp(argv[carg],"-trackEnterNode"))
    {
        ++carg;
        if (carg>=argc) return false;
        trackEnterNodeList += QString(argv[carg++]);
        *idx = carg;
        return true;
    }

    if (!strcmp(argv[carg],"-trackExitNode"))
    {
        ++carg;
        if (carg>=argc) return false;
        trackExitNodeList += QString(argv[carg++]);
        *idx = carg;
        return true;
    }

    if (!strcmp(argv[carg],"-trackEdge"))
    {
        ++carg;
        if (carg>=argc) return false;
        trackEdgeList += QString(argv[carg++]);
        *idx = carg;
        return true;
    }

    if (!strcmp(argv[carg],"-getTrackValue"))
    {
        ++carg;
        if (carg>=argc) return false;
        getTrackTagList += QString(argv[carg++]);
        *idx = carg;
        return true;
    }

    if (!strcmp(argv[carg],"-getItemValue"))
    {
        ++carg;
        if (carg>=argc) return false;
        getItemTagList += QString(argv[carg++]);
        *idx = carg;
        return true;
    }

    // nothing known
    return false;
}

void dumpSyntax(char* binname)
{
    printf("DBTEST is a regression test tool for the DralDB Library.\n");
    printf("Version 0.5, june 2003. Federico Ardanaz, Intel Labs Barcelona.\n");
    printf("Expected Syntax:\n");
    printf("%s options dralfile\n\n",binname);
    printf("Available options:\n\n");
    printf("-trackItemTags true|false:\t\tEnable/disable item tags tracking. False by default.\n");
    printf("-itemTagBackPropagate true|false:\tEnable/disable item tags value back propagation. False by default.\n");
    printf("-trackCycleTags true|false:\t\tEnable/disable cycle-tag tracking. False by default.\n");
    printf("-trackNode \"node[inst];d1,d2,...dN\":\tRequest tracking of such a node slot.\n");
    printf("-trackEnterNode \"node[i];d1...\":\tRequest tracking of enter nodes on such a node slot.\n");
    printf("-trackExitNode \"node[i];d1...\":\t\tRequest tracking of exit nodes on such a node slot.\n");
    printf("-trackEdge \"edge;from;to;position\":\tRequest tracking of such an edge slot.\n");
    printf("\n");
    printf("-getTrackValue \"track_id tag cycle\":\tQuery the tag's value on a given cycle and track.\n");
    printf("-getItemValue \"item_id tag cycle\":\tQuery the tag's value on a given cycle and item.\n");
    printf("\n");
    printf("-dumpgraph:\t\t\t\tDumps out the contents of the encoded dral graph\n");
    printf("-dumpitems:\t\t\t\tDumps out all the stored items with all their tags\n");
    printf("-dumptracks:\t\t\t\tDumps out all the registered tracks\n");
    printf("\n");
    printf("-l logfile:\t\t\t\tSpecifies the log file name. dbtest.log by default.\n");
    printf("-verbose:\t\t\t\tDumps out some auxiliar information and track identifiers.\n");
    printf("\n");
    printf("Notes:\n");
    printf("1) Multiple -trackNode, -trackEnterNode, -trackExitNode, -trackEdge, \n");
    printf("   -getTrackValue and -getItemValue instances supported.\n");
    printf("\n");
    printf("2) If you are unsure about the dral-graph structure of the examined drl file, run DBTEST\n");
    printf("   first only with the -dumpgraph option to get header information.\n");
    printf("\n");
    printf("3) If you want to check tag values of items or tracks, run DBTEST first with the appropiated\n");
    printf("   -track options and -verbose to get the computed track numbers and -dumpitems to get the\n");
    printf("   whole list of available items on the drl file.\n");
    printf("\n");
    printf("4) Warning, if the drl file being loades has a version smaller than 2 (DRAL 1.xx), the\n");
    printf("   -itemTagBackPropagate flag will be ignored. Tag values are backward propagated\n");
    printf("   for items in any case.\n");
    printf("\n");
}

void initGlobals()
{
    drlFileName = QString::null;
    logFileName = QString("dbtest.log");
    logMgr = NULL;
    dumpItems = false;
    dumpTracks = false;
    dumpGraph = false;
    trackItems = false;
    trackCycles = false;
    verbose = false;
    backProp = false;
    cycleTrackId = -1;
    trackNodeList.clear();
    trackEdgeList.clear();
    trackEnterNodeList.clear();
    trackExitNodeList.clear();
    getItemTagList.clear();
    getTrackTagList.clear();

    db=NULL;
}

void applyConf()
{
    if (dumpGraph) db->dumpGraphDescription();

    // back prop?
    db->setTagBackPropagate(backProp);

    // apply item conf
    db->trackItemTags(trackItems);

    // apply cycle conf
    if (trackCycles)
    {
        cycleTrackId = db->trackCycleTags();
        if (verbose) printf("Cycle Tag track id =%d\n",cycleTrackId);
    }

    // register node tracking
    if (trackNodeList.count()>0)
    {
        QStringList::Iterator it = trackNodeList.begin();
        while (it != trackNodeList.end())
        {
            INT32 trackId = db->trackNodeTags(*it);
            if (trackId<0)
            {
                printf("Warning, node slot %s is invalid for this drl, skipping...\n",(*it).latin1());
            }
            else
            {
                if (verbose) printf("Node slot %s got track id = %d\n",(*it).latin1(),trackId);
            }
            ++it;
        }
    }

    // register edge tracking
    /*
    if (trackEdgeList.count()>0)
    {
        QStringList::Iterator it = trackEdgeList.begin();
        while (it != trackEdgeList.end())
        {
            INT32 trackId = db->trackMoveItem(*it);
            if (trackId<0)
            {
                printf("Warning, edge slot %s is invalid for this drl, skipping...\n",(*it).latin1());
            }
            else
            {
                if (verbose) printf("Edge slot %s got track id = %d\n",(*it).latin1(),trackId);
            }
            ++it;
        }
    }
    */
    // register enter node tracking
    if (trackEnterNodeList.count()>0)
    {
        QStringList::Iterator it = trackEnterNodeList.begin();
        while (it != trackEnterNodeList.end())
        {
            INT32 trackId = db->trackEnterNode(*it);
            if (trackId<0)
            {
                printf("Warning, node slot %s is invalid for this drl, skipping...\n",(*it).latin1());
            }
            else
            {
                if (verbose) printf("EnterNode slot %s got track id = %d\n",(*it).latin1(),trackId);
            }
            ++it;
        }
    }

    // register exit node tracking
    if (trackExitNodeList.count()>0)
    {
        QStringList::Iterator it = trackExitNodeList.begin();
        while (it != trackExitNodeList.end())
        {
            INT32 trackId = db->trackExitNode(*it);
            if (trackId<0)
            {
                printf("Warning, node slot %s is invalid for this drl, skipping...\n",(*it).latin1());
            }
            else
            {
                if (verbose) printf("ExitNode slot %s got track id = %d\n",(*it).latin1(),trackId);
            }
            ++it;
        }
    }
}

void applyQueries()
{
    // 1) check for item queries
    if (getItemTagList.count()>0)
    {
        QStringList::Iterator it = getItemTagList.begin();
        while (it != getItemTagList.end())
        {
            INT32 itemId;
            QString tagName;
            UINT32 cycle;
            bool dok = decodeQuery(*it,&itemId,&tagName,&cycle);
            if (!dok)
            {
                printf("Warning, the query '%s' is invalid for this drl, skipping...\n",(*it).latin1());
            }
            else
            {
                ItemHandler ih;
		db->lookForItemId(&ih, itemId);
                if (! ih.isValidItemHandler())
                {
                    printf("Warning, the query '%s' is invalid. Item not found.\n",(*it).latin1());
                }
                else
                {
                    // ok, I found the item, now look for the tag:
                    TagValueType tgvtype;
                    UINT64 tgvalue;
                    UINT64 tgwhen;
                    INT16  tgbase;
                    INT32 pos = ih.getTagByName(tagName,&tgvalue, &tgvtype,&tgbase,&tgwhen,cycle);
                    if (pos<0)
                    {
                        printf("Warning, the query '%s' failed. Tag not defined in the given item and cycle.\n",(*it).latin1());
                    }
                    else
                    {
                        QString formated= ih.getFormatedTagValue();
                        printf("Item query '%s': tag found at cycle=%d, value=%s\n",
                               (*it).latin1(),(int)tgwhen,formated.latin1());
                    }
                }
            }
            ++it;
        }
    }

    // 2) check for track queries
    if (getTrackTagList.count()>0)
    {
        QStringList::Iterator it = getTrackTagList.begin();
        while (it != getTrackTagList.end())
        {
            INT32 trackId;
            QString tagName;
            UINT32 cycle;
            bool dok = decodeQuery(*it,&trackId,&tagName,&cycle);
            if (!dok)
            {
                printf("Warning, the query %s is invalid for this drl, skipping...\n",(*it).latin1());
            }
            else
            {
                QString result = QString::null;
                bool qok = db->getFormatedTrackTagValue(trackId, tagName,cycle,&result);
                if (!qok)
                {
                    printf("Warning, the query %s failed. Tag not defined in the given track and cycle.\n",(*it).latin1());
                }
                else
                {
                    printf("Track query '%s': tag found (at cycle...), value=%s\n",
                               (*it).latin1(),result.latin1());
                }
            }
            ++it;
        }
    }
}

bool decodeQuery(QString query,INT32* itemid,QString* tag,UINT32* cycle)
{
    bool pok;
    QRegExp rx( "^(\\S+)\\s(\\S+)\\s(\\S+)");
    int pos = rx.search(query);
    QStringList list = rx.capturedTexts();

    if (pos<0) return false;
    *itemid = list[1].toInt(&pok);
    if (!pok) return false;

    *tag= list[2];

    *cycle = list[3].toUInt(&pok);

    return pok;
}


void applyDumps()
{
    if (dumpItems)
    {
        // dump out the stored items...
        printf("* Saved Items:\n");
        ItemHandler ith;
	db->getFirstItem(&ith);
        while (ith.isValidItemHandler())
        {
            ith.dumpItem();
            ith.skipToNextItem();
        }
    }

    if (dumpTracks)
    {
        // dump out the stored items...
        printf("* Tracks:\n");
        // dbg API
        db->dumpTrackHeap();
    }
}

void openfile()
{
    if (verbose) { printf ("opening file...\n");fflush(stdout); }
    bool openok = db->openDRLFile(drlFileName);
    if (!openok)
    {
        printf("drl open failed for file %s\n",drlFileName.latin1());
        exit(-1);
    }
}

void loadhdr()
{
    if (verbose) {printf ("loading header...\n");fflush(stdout);}
    bool hok = db->loadDRLHeader();
    if (!hok)
    {
        printf("drl header processing failed!?\n");
        exit(-1);
    }
}

void checkParameters(int argc, char** argv)
{
    bool pok = parseParameters(argc,argv);
    if (!pok)
    {
        dumpSyntax(argv[0]);
        exit(-1);
    }

    if (logFileName!=QString::null)
    {
        LogMgr* logMgr = LogMgr::getInstance(logFileName);
    }
}

void readAll()
{
    // read it all
    if (verbose) {printf ("reading trace...\n");fflush(stdout);}
    bool sok = db->processAllEvents();
    if (!sok)
    {
        printf("drl simulation trace processing failed, see log for details on %s\n",
               logFileName.latin1());
        //exit(-1);
    }
}

void closefile()
{
    bool closeok = db->closeDRLFile();
    if (!closeok)
    {
        printf("drl closing failed!?\n");
        exit(-1);
    }

    db->destroy();
}

