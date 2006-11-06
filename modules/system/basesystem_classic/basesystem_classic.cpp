/**************************************************************************
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
 * @author Artur Klauser
 * @brief Base class for ASIM system top-level object.
 */

// generic
#include <iostream>
#include <string>
#include <sstream>

// ASIM core
#include "asim/event.h"
#include "asim/state.h"
#include "asim/config.h"

// ASIM public modules
#include "asim/provides/default_adf.h"

// ASIM local module
#include "basesystem_classic.h"


ASIM_SYSTEM_CLASS::ASIM_SYSTEM_CLASS(
    const char *n,
    UINT16 ncpu,
    ASIM_EXCEPT e,
    UINT32 feederThreads)
  : ASIM_MODULE_CLASS(NULL, n, e),
    cycles(0),
    base_cycles(0),
    num_feeder_threads(feederThreads)
{
  if (ncpu == 0)
  {
      // Models that set the number of CPUs to 0 size themselves to the
      // number of threads requested by the feeder.
      ncpu = num_feeder_threads;
      if (ncpu > MAX_TOTAL_NUM_CPUS)
      {
          ncpu = MAX_TOTAL_NUM_CPUS;
      }
  }

  num_cpus = ncpu;
  nonDrainCycles = 0;
  pipeDraining = false;          
  committed = new UINT64[ncpu];
  macroCommitted = new UINT64[ncpu];
  cpu2module = new ASIM_MODULE[ncpu];
  receivedPkt=0; // for the network simulator
  memset(committed, 0, sizeof(UINT64) * ncpu); 
  memset(macroCommitted, 0, sizeof(UINT64) * ncpu);

  committedMarkers = 0;
  commitWatchMarker = -1; // clear marker
  hasMicroOps = false;
}

bool
ASIM_SYSTEM_CLASS::InitModule (void)
{
    // init Events
    InitEvents();
    return true;
}

void
ASIM_SYSTEM_CLASS::InitEvents (void)
{
    char * defaultADF;
    string adf;
    // get some ADF (built-in or from file)
    if (ADF_DEFAULT == "$built-in$") {
        // use built-in default
        defaultADF = DefaultADF();
        if (defaultADF == NULL) {
            return;
        }
        adf = defaultADF;
        delete [] defaultADF;
    } else {
        // no ADF file required
        if (ADF_DEFAULT.empty()) {
            return;
        }

        // read in file specified in ADF_DEFAULT and use that
        ifstream adfFile(ADF_DEFAULT.c_str());
        if (!adfFile) {
            ASIMWARNING("Can't open default ADF file " << ADF_DEFAULT
                << " for read." << endl
                << "  Not including any default ADF file into DRAL file"
                << endl);
            return;
        }

        string line;
        // read adfFile into a string
        while (! adfFile.eof()) {
            getline (adfFile, line);
            if (line.empty() && adfFile.eof()) {
                break; // also eof
            }
            adf += line;
            adf += "\n";
        }
        adfFile.close();
    }
    // replace magic string with PM config information
    const char * const replacePerlConfig = "$REPLACE$PERL_CONFIG";
    string::size_type idx = adf.find(replacePerlConfig);
    if (idx != string::npos) {
        // its a perl script that wants simulator configuration included
        ostringstream osParam;
        ostringstream osModule;
        STATE_ITERATOR_CLASS iter(this, true);
        ASIM_STATE state;

        osParam << "undef %param;" << endl;
        osModule << "undef %module;" << endl;
        while ((state = iter.Next()) != NULL) {
            // find all parameters and modules and generate a config that
            // is included into the ADF perl script;
            // Note: this is a bit hacky here, since its depending on the
            // name prefixes that are synthesized by AMC for registering
            // the configuration in the as state of the config module;
            string name = state->Name();
            const char * const paramPrefix = "Param_";
            const char * const modulePrefix = "Module_";
            if (name.find(paramPrefix) == 0) {
                // handle params
                name = name.substr(strlen(paramPrefix));
                if (state->Type() == STATE_UINT) {
                    osParam << "$param{'" << name << "'} = "
                        << state->IntValue() << ";" << endl;
                } else {
                    osParam << "# param " << name
                            << " has unsupported type" << endl;
                }
            } else if (name.find(modulePrefix) == 0) {
                // handle modules
                name = name.substr(strlen(modulePrefix));
                string desc = state->Description();
                string junk("provides " + name + " (");
                if (desc.find(junk) == 0) {
                    // starts with junk - remove junk
                    desc = desc.substr(junk.length());
                    if (desc.rfind(")") == desc.length() - 1) {
                        // ends in junk - remove junk
                        desc = desc.substr(0, desc.length() - 1);
                    }
                }
                osModule << "$module{'" << name << "'} = '"
                    << desc << "';" << endl;
            }
        }
        string config = string("\n") + osParam.str() + osModule.str();
        adf.replace(idx, strlen(replacePerlConfig), config);
    }
    // put ADF into DRAL file
    DRALEVENT(Comment(ADF_MAGICNUM,adf.c_str(),true));
} 
