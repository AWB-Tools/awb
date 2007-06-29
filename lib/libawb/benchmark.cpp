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

// generic (C++)
#include <iostream>
#include <fstream>
#include <stdio.h>

// local
#include "benchmark.h"
#include "util.h"

//----------------------------------------------------------------------------
// class Benchmark
//----------------------------------------------------------------------------

/**
 * Create a new benchmark object.
 */
Benchmark::Benchmark (
    const Workspace & theWorkspace)
  : workspace(theWorkspace),
    regionNumber(0)
{
    // nada
}

/**
 * Destroy this benchmark object.
 */
Benchmark::~Benchmark()
{
    // nada
}

/**
 * Parse the file benchmarkFileName into the internal data structures of
 * this benchmark.
 *
 * @fixme: The initial error checking has a high degree of overlap with
 * the module parser. Maybe this should be factored out?
 *
 * @return true on success, false otherwise
 */
bool
Benchmark::Parse (
    const string & benchmarkFileName)  ///< path to benchmark file to parse
{
    SPEED_DEBUG("Benchmark::Parse() {0}");

    // do some error checking first
    if (benchmarkFileName.empty()) {
        cerr << "Benchmark::Parse: Empty benchmark file name" << endl;
        return false;
    }

    //
    // Region number (e.g. pinpoint) may be encoded in the file name as
    // a suffix: _r[number].  Remove the suffix to fine the .cfg file.
    //
    string realBenchmarkFileName = benchmarkFileName;
    MatchString regionMatch(benchmarkFileName);
    MatchString::MatchArray regionMatchArr;
    string regionSuffix;
    if (! regionMatch.Match(string("_r[0-9]*$"), regionMatchArr).empty()) {
        //
        // There can only be one match entry since the regexp must be at
        // the end of the line.
        //
        regionSuffix = regionMatchArr[0];

        realBenchmarkFileName =
            realBenchmarkFileName.substr(0,
                                         realBenchmarkFileName.length() -
                                         regionSuffix.length());
        string regionNumStr = regionSuffix.substr(2, regionSuffix.length() - 2);
        int regionNum = atol(regionNumStr.c_str());

        cout << "Selecting region #" << regionNum << endl;
        SetRegionNumber(regionNum);
    }

    SPEED_DEBUG("Benchmark::Parse() {1}");

    //
    // Is there a script in the middle of the path that generates the CFG
    // file?  Look for a "directory" with a .cfx suffix.  Yes, this is a hack,
    // but it is more efficient than looking for every level in the hierarchy.
    //
    string::size_type cfxPos = realBenchmarkFileName.find(".cfx/");
    if (cfxPos != string::npos) {
        // Arguments are everything beyond the .cfx file
        string cfxArgs = realBenchmarkFileName.substr(
            cfxPos + 5,
            realBenchmarkFileName.length() - cfxPos - 5);

        SetConfigArgs(string("--emit ") + cfxArgs);

        // The benchmark file is the .cfx script
        realBenchmarkFileName.resize(cfxPos + 4);
    }

    SPEED_DEBUG("Benchmark::Parse() {2}");

    UnionDir & sourceTree = workspace.GetSourceTree();
    if (sourceTree.IsDirectory (realBenchmarkFileName)) {
        cerr << "Benchmark::Parse: benchmark points to directory "
             << "instead of file - " << realBenchmarkFileName << endl;
        return false;
    }

    string dirName = FileDirName (realBenchmarkFileName);
    string fullName = sourceTree.FullName (realBenchmarkFileName);
    SetConfigFile (fullName);

    SPEED_DEBUG("Benchmark::Parse() {3}");

    if ( ! sourceTree.IsFile (realBenchmarkFileName)) {
        // if we can't find the config file, figure out what is the first
        // directory in its path that can't be found and supply a pointed
        // error message for that
        string missingDir;
        if ( ! sourceTree.Exists (dirName)) {
            while ( ! sourceTree.Exists (dirName)) {
                missingDir = dirName;
                dirName = FileDirName (dirName);
                if (dirName == missingDir) {
                    cerr << "Benchmark::Parse: internal error" << endl;
                    exit(1);
                }
            }
            
            missingDir = string("  non-existent directory ")
                + missingDir + '\n';
        }
        cerr << "Benchmark::Parse:" << endl << missingDir
             << "  Benchmark file " << realBenchmarkFileName << " not found"
             << endl;
        return false;
    }

    //
    // now we are ready to parse the benchmark file
    //
    string line;

    // open benchmark file
    char cfgBuf[4096];
    FILE *benchmarkFile;
    bool isPipe = false;

    SPEED_DEBUG("Benchmark::Parse() {4}");

    if (GetConfigArgs().empty()) {
        // Config file is just a normal file.
        benchmarkFile = fopen(fullName.c_str(), "r");
    }
    else {
        // Config file is a script.
        isPipe = true;
        string cfgCmd = fullName + string(" ") + GetConfigArgs();
        benchmarkFile = popen(cfgCmd.c_str(), "r");
    }

    if (!benchmarkFile) {
        cerr << "Error: Can't open " << fullName << " for read" << endl;
        return false;
    }
 
    SPEED_DEBUG("Benchmark::Parse() {5}");

    // parse benchmark file
    int itemNumber = 0;
    // parse all but
    while ((itemNumber < 8) && fgets(cfgBuf, sizeof(cfgBuf), benchmarkFile)) {
        SPEED_DEBUG("Benchmark::Parse() {5.5}");
        line = StringRemoveCRLF(cfgBuf);
        
        // substutite region number
        line = StringSubstituteAll (line, "%R", regionSuffix);
        // remove space and TCL {} string encapsulation
        line = StringTrim (line, TrimLeft, " \t{");
        line = StringTrim (line, TrimRight, " \t}");
        if (StringTrim (StringTrim (line), TrimRight, " \t{") == "BmAdd") {
            continue; // skip header line
        }
        // yuk! positional indexing - but thats TCL inheritance
        // this matches the way the original TCL code BmFormat writes
        // out those benchmark files; even though the TCL reader would
        // understand a larger range of input formats, for simplicity we
        // restrict ourselves here to exactly what the TCL writer
        // produces.
        switch (itemNumber) {
            case 0: SetName (line); break;
            case 1: SetFileName (line); break;
            case 2: SetDesc (line); break;
            case 3: SetFeederType (line); break;
            case 4:
            {
                
              string::size_type wPos = line.find_first_of(" \t");
              string args;
              if (wPos == string::npos)
              {
                  // No arguments
                  SetSetupFile(line);
              }
              else
              {
                  // Arguments follow path of setup script
                  SetSetupFile(line.substr(0, wPos));
                  args = StringTrim(line.substr(wPos, line.length()-wPos), TrimLeft, " \t");
              }
              if (! regionSuffix.empty())
              {
                  char buf[128];
                  sprintf(buf, "%d", GetRegionNumber());
                  args = args + " --region " + buf;
              }
              args = SubstituteVariables(args);
              SetSetupArgs(args);
              break;
            }
            case 5:
            {
                // Hack alert.  General flags can include flags to this code,
                // e.g. the --regions tag used by awb to show individual regions.
                // The same is true of --queryregions. 
                // Strip them.
                MatchString regions(line);
                line = regions.Substitute(string("--regions *[0-9]* *"), 0, string());
                MatchString queryregions(line);
                line = queryregions.Substitute(string("--queryregions *"), 0, string());

                SetGeneralFlags (line);
                break;
            }
            case 6: SetFeederFlags (line); break;
            case 7: SetSystemFlags (line); break;
        }
        itemNumber++;
    }

    SPEED_DEBUG("Benchmark::Parse() {6}");

    // parse commands section
    string::size_type cmdsIndent = 0;
    string::size_type indent = 0;
    bool indentSet = false;
    while (fgets(cfgBuf, sizeof(cfgBuf), benchmarkFile)) {
        line = StringRemoveCRLF(cfgBuf);

        // substutite region number
        line = StringSubstituteAll (line, "%R", regionSuffix);

        string origLine = StringDeTab (line);
        line = StringTrim (origLine); // remove indent
        if (line == "{}") {
            break; // empty commands section - done parsing
        } else if ( ! indentSet && line == "{") {
            // un-indentation is heuristic (but safe)
            cmdsIndent = origLine.find ('{');
            if (cmdsIndent == string::npos) {
                cmdsIndent = 0;
                indent = 0;
            } else {
                indent = cmdsIndent + 2; // following lines are indented 2 more
            }
            indentSet = true;
            continue; // begin of commands section
        } else {
            // heuristically un-indent
            string myLine;
            if ( ! origLine.empty() &&
                StringTrim (origLine.substr (0, cmdsIndent)) == "")
            {
                // un-indent correctly
                myLine = origLine.substr (cmdsIndent);
            } else {
                // un-indent would lose non-space characters - trim instead
                myLine = StringTrim (origLine, TrimLeft);
            }
            myLine = StringTrim (myLine, TrimRight);
            if (myLine == "}") {
                break; // end of commands section
            }
        }

        // remove space and TCL {} string encapsulation
        if ( ! origLine.empty() &&
            StringTrim (origLine.substr (0, indent)) == "")
        {
            // un-indent correctly
            line = origLine.substr (indent);
        } else {
            // un-indent would lose non-space characters - trim instead
            line = StringTrim (line, TrimLeft);
        }
        line = StringTrim (line, TrimRight);
        AddCommand (line);
    }

    // error checking
    if (itemNumber != 8) {
        cerr << "Benchmark::Parse: Error parsing benchmark file "
             << fullName << endl;
        cerr << "  Wrong number of items found: got " << itemNumber
             << " but expected 8." << endl;
        return false;
    }
    fgets(cfgBuf, sizeof(cfgBuf), benchmarkFile);
    line = StringRemoveCRLF(cfgBuf);
    if (StringTrim (line) != "}") {
        cerr << "Benchmark::Parse: Error parsing benchmark file "
             << fullName << endl;
        cerr << "  File end not recognized" << endl;
        return false;
    }

    SPEED_DEBUG("Benchmark::Parse() {7}");

    if (isPipe) {
        pclose(benchmarkFile);
    }
    else {
        fclose(benchmarkFile);
    }

    SPEED_DEBUG("Benchmark::Parse() {8}");

    return true;
}


/**
 * Substitute occurances of variable references in benchmark information.
 *
 * @return input with variables substituted
 */
string
Benchmark::SubstituteVariables (
    const string & in) ///< input string for substitution
const
{
    const string & benchmarkDir = 
        workspace.GetDirectory (Workspace::BenchmarkDir);
    UnionDir & sourceTree = workspace.GetSourceTree();
    const string configDir = FileDirName (
        sourceTree.FullName (GetConfigFile()));

    string out = StringSubstituteAll (in, "BENCHMARKDIR", benchmarkDir);
    out = StringSubstituteAll (out, "CONFIGDIR", configDir);

    return out;
}


/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
Benchmark::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "Benchmark::" << endl;
    out << prefix << "  Name: " << GetName() << endl;
    out << prefix << "  FileName: " << GetFileName() << endl;
    out << prefix << "  Desc: " << GetDesc() << endl;
    out << prefix << "  ConfigFile: " << GetConfigFile() << endl;
    out << prefix << "  ConfigArgs: " << GetConfigArgs() << endl;
    out << prefix << "  FeederType: " << GetFeederType() << endl;
    out << prefix << "  SetupFile: " << GetSetupFile() << endl;
    out << prefix << "  GeneralFlags: " << GetGeneralFlags() << endl;
    out << prefix << "  FeederFlags: " << GetFeederFlags() << endl;
    out << prefix << "  SystemFlags: " << GetSystemFlags() << endl;
    out << prefix << "  Commands:" << endl;
    FOREACH_CONST (StringList, it, commandList) {
        out << prefix << "    " << *it << endl;
    }

    return out;
}


//----------------------------------------------------------------------------
// class BenchmarkDB
//----------------------------------------------------------------------------

/**
 *
 */
BenchmarkDB::BenchmarkDB (
    const Workspace & theWorkspace)
  : workspace(theWorkspace)
{
    // nada
}

/**
 *
 */
BenchmarkDB::~BenchmarkDB ()
{
    FOREACH (BenchmarkMap, it, benchmarkMap) {
        delete it->second; // delete the benchmark
    }
} 

/**
 *
 */
void
BenchmarkDB::RefreshAll (
    const string & configFiles)
{
    configFiles.empty(); // keep compiler happy (unused param)
}

/**
 *
 */
void
BenchmarkDB::ReadBenchmarks (
    const string & fileName)
{
    fileName.empty(); // keep compiler happy (unused param)
}

/**
 *
 */
void
BenchmarkDB::Add (
    Benchmark * benchmark)
{
    if (benchmark) return; // keep compiler happy (unused param)
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
BenchmarkDB::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "BenchmarkDB::" << endl;

    return out;
}


//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

void TestBenchmark (int argc, char ** argv)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
        exit (1);
    } else {
        Benchmark benchmark(*workspace);

        if ( ! benchmark.Parse (argv[1])) {
            cerr << "Benchmark parsing error!" << endl;
        }
        benchmark.Dump(cout);
    }

    delete workspace;
}

void TestBenchmarkDB (int argc, char ** argv)
{
    Workspace * workspace = NULL;

    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Workspace::Setup:: Workspace creation failed!" << endl;
        exit (1);
    } else {
        BenchmarkDB benchmarkDB(*workspace);

        //benchmarkDB.CollectAllModules();
        benchmarkDB.Dump(cout);
    }

    delete workspace;
}

int main (int argc, char ** argv)
{
#if 0
    if (argc >= 2) {
        TestBenchmark (argc, argv);
    }
#endif

#if 0
    if (argc >= 1) {
        TestBenchmarkDB (argc, argv);
    }
#endif
}

#endif // TESTS 
