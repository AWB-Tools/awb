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
 * @brief ASIM benchmark information
 */

#ifndef _BENCHMARK_
#define _BENCHMARK_ 1

// generic (C++)
#include <vector>
#include <map>

// local
#include "workspace.h"

using namespace std;

/**
 * @brief ASIM benchmark information.
 *
 * This class encapsulates all information that is known about a concrete
 * ASIM benchmark.
 * @note: Benchmarks do not have abstract objects (unlike modules).
 * There is only one benchmark abstraction, which is built-in.
 */
class Benchmark {
  public:
    // types
    typedef vector<string> StringList;

  private:
    // members
    const Workspace & workspace;  ///< workspace to use

    string configFile;      ///< file name where this config is stored
    string configArgs;      ///< arguments to the config file (.cfx script)
    string name;            ///< benchmark name
    string fileName;        ///< redundant!?
    string desc;            ///< benchmark description
    string feederType;      ///< type of feeder to be used by this benchmark
    string setupFile;       ///< file name of setup file
    string setupArgs;       ///< arguments passed to setup script
    string generalFlags;    ///< flags for simulator run: general section
    string feederFlags;     ///< flags for simulator run: feeder section
    string systemFlags;     ///< flags for simulator run: system section
    StringList commandList; ///< commands list for simulator to execute
    int regionNumber;       ///< region number (pinpoint) to select

  public:
    // constructors / destructors
    Benchmark (const Workspace & theWorkspace);
    ~Benchmark ();

    /// Parse benchmarkFileName into benchmark object.
    bool Parse (const string & benchmarkFileName);

    // accessors / modifiers
    string GetConfigFile (void) const { return configFile; }
    void SetConfigFile (const string & theConfigFile)
        { configFile = theConfigFile; }
    //
    string GetConfigArgs (void) const { return configArgs; }
    void SetConfigArgs (const string & theConfigArgs)
        { configArgs = theConfigArgs; }
    //
    string GetName (void) const { return name; }
    void SetName (const string & theName)
        { name = theName; }
    //
    string GetFileName (void) const { return fileName; }
    void SetFileName (const string & theFileName)
        { fileName = theFileName; }
    //
    string GetDesc (void) const { return desc; }
    void SetDesc (const string & theDesc)
        { desc = theDesc; }
    //
    string GetFeederType (void) const { return feederType; }
    void SetFeederType (const string & theFeederType)
        { feederType = theFeederType; }
    //
    string GetSetupFile (void) const { return setupFile; }
    void SetSetupFile (const string & theSetupFile)
        { setupFile = theSetupFile; }
    //
    string GetSetupArgs (void) const { return setupArgs; }
    void SetSetupArgs (const string & theSetupArgs)
        { setupArgs = theSetupArgs; }
    //
    string GetGeneralFlags (void) const { return generalFlags; }
    void SetGeneralFlags (const string & theGeneralFlags)
        { generalFlags = theGeneralFlags; }
    //
    string GetFeederFlags (void) const { return feederFlags; }
    void SetFeederFlags (const string & theFeederFlags)
        { feederFlags = theFeederFlags; }
    //
    string GetSystemFlags (void) const { return systemFlags; }
    void SetSystemFlags (const string & theSystemFlags)
        { systemFlags = theSystemFlags; }
    //
    int GetRegionNumber (void) const { return regionNumber; }
    void SetRegionNumber (int theRegionNumber)
        { regionNumber = theRegionNumber; }
    //
    //
    const StringList & GetCommands (void) const { return commandList; }
    void AddCommand (const string & command)
        { commandList.push_back (command); }

    ///< Substitute benchmark variables values.
    string SubstituteVariables (const string & in) const;

    /// Dump internal data structures to ostream
    ostream & Dump(ostream & out, const string & prefix = "") const;
};

/**
 * @brief ASIM benchmark database.
 *
 * This class is a (very, very, very simple) database of benchmarks. It
 * keeps track of a set of benchmarks by their (real) file name.
 */
class BenchmarkDB {
  private:
    // types
    typedef map<string, Benchmark *> BenchmarkMap;

    // members
    const Workspace & workspace;  ///< workspace

    BenchmarkMap benchmarkMap;

  public:
    // constructors / destructors
    BenchmarkDB (const Workspace & theWorkspace);
    ~BenchmarkDB ();

    void RefreshAll (const string & configFiles = "");
    void ReadBenchmarks (const string & fileName);
    void Add (Benchmark * benchmark);

    /// Dump internal data structures to ostream
    ostream & Dump(ostream & out, const string & prefix = "") const;
};

#endif // _BENCHMARK_ 
