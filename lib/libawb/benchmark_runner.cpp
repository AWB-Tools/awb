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
 * @brief ASIM benchmark handler: support for running benchmarks on models
 */

// generic (C)
#include <unistd.h>
#include <stdlib.h>

// generic (C++)
#include <iostream>
#include <fstream>
#include <iterator>

// local
#include "benchmark_runner.h"
#include "util.h"

/**
 * Create a new benchmark runner for the given model and benchmark.
 */
BenchmarkRunner::BenchmarkRunner(
    const Workspace & theWorkspace, ///< workspace to use
    const Benchmark & theBenchmark, ///< benchmark to run
    const string & theBenchmarkDir, ///< directory where benchmark should run
    const string & theModelExe)     ///< model executable to run benchmark on
  : workspace(theWorkspace),
    benchmark(theBenchmark),
    benchmarkDir(theBenchmarkDir),
    modelExecutable(theModelExe)
{
    // nada
}

/**
 * Destroy this benchmark runner.
 */
BenchmarkRunner::~BenchmarkRunner()
{
    // nada
}

/**
 * Setup the benchmark directory with all files necessary to run the
 * benchmark on the given model executable (ie. compiled simulator).
 *
 * @return true on success, false otherwise
 */
bool
BenchmarkRunner::SetupBenchmarkDir (void)
{
    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {0}")

    const char * const awbcmds = "awbcmds"; // awbcmds file
    const char * const run     = "run";     // run script (file)
    bool success = true;

    //
    // make sure benchmarkDir exists
    //
    if ( ! FileExists (benchmarkDir)) {
        MakeDir (benchmarkDir);
    }

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {1}")

    //
    // create awbcmds file from info in benchmark
    //
    string awbcmdsFileName = FileJoin (benchmarkDir, awbcmds);
    ofstream awbcmdsFile(awbcmdsFileName.c_str());
    if ( ! awbcmdsFile) {
        cerr << "Error: Can't open awbcmds file " << awbcmdsFileName << endl;
        return false;
    }
    const Benchmark::StringList & awbcommands = benchmark.GetCommands();
    copy (awbcommands.begin(), awbcommands.end(),
          ostream_iterator<string>(awbcmdsFile, "\n"));
    awbcmdsFile.close();

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {2}")

    //
    // run the benchmark's setup script
    //
    string cwd = GetCWD();

    string setupFile = benchmark.SubstituteVariables (
        benchmark.GetSetupFile());
    setupFile = workspace.GetSourceTree().FullName(setupFile);

    string setupDir = FileDirName (setupFile);
    if (chdir (setupDir.c_str())) {
        cerr << "Error: Can't cd to benchmark setup directory "
             << setupDir << endl;
        return false;
    }

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {3}")

    //
    // Set some environment variables for use in all scripts.  One could argue
    // that these should be passed as arguments.  That is an option, assuming
    // we can find a way to automate the argument passing.  Having to define
    // these common values in every workload .cfg file seems tedious.
    //
    if (setenv("AWB_BENCHMARKS_ROOT",
               workspace.GetDirectory(Workspace::BenchmarkDir).c_str(), 1)) {
        cerr << "Error: No space for new environment variables" << endl;
        return false;
    }
    if (setenv("ASIM_CONFIG_MODEL", modelExecutable.c_str(), 1)) {
        cerr << "Error: No space for new environment variables" << endl;
        return false;
    }

    string cmd = setupFile
                 + " " + benchmark.GetSetupArgs()
                 + " " + setupDir
                 + " " + benchmarkDir;
    
    // DEBUG cerr << "CMD=" << cmd << endl;
    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {4}: CMD=" << cmd)
    
    if (system (cmd.c_str()) != 0) {
        cerr << "Error: Setting up benchmark " << benchmark.GetName() << endl;
        success = false;
    }

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {5}")

    unsetenv("AWB_BENCHMARKS_ROOT");
    unsetenv("ASIM_CONFIG_MODEL");

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {6}")

    if (chdir (cwd.c_str()) && success) {
        cerr << "Error: Can't cd back to previous working directory "
             << cwd << endl;
        return false;
    }
    if (! success) {
        return false;
    }

    //
    // now rewrite the run script (file)
    //
    string generalFlags = string("-cf ") + awbcmds + " " +
        benchmark.SubstituteVariables (benchmark.GetGeneralFlags());
    string systemFlags = benchmark.SubstituteVariables (
        benchmark.GetSystemFlags());
    string feederFlags = benchmark.SubstituteVariables (
        benchmark.GetFeederFlags());

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {7}")

    // read in original run file
    string runFileName = FileJoin (benchmarkDir, run);
    ifstream runFileIn(runFileName.c_str());
    if ( ! runFileIn) {
        cerr << "Error: Can't open run file for read" << runFileName << endl;
        return false;
    }
    StringList runFileLines;
    string line;
    while ( ! runFileIn.eof()) {
        getline (runFileIn, line);
        if (line.empty() && runFileIn.eof()) {
            break; // also eof
        }
        runFileLines.push_back (line);
    }
    runFileIn.close();

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {8}")

    // replace all $genFlags with ${1+"$@"}
    const string searchStr =  "$genFlags";
    const string replaceStr = "$genFlags ${1+\"$@\"}";
    FOREACH (StringList, it, runFileLines) {
        string::size_type idx = 0;
        while (idx < it->size() &&
               (idx = it->find(searchStr, idx)) != string::npos)
        {
            it->replace (idx, searchStr.length(), replaceStr);
            idx += 2;
        }
    }

    string extraFeederFlags = workspace.GetConfVar ("extraFeederFlags");
    if (extraFeederFlags == "__extraFeederFlags_undefined__") {
        extraFeederFlags = "";
    }
    
    // write out new run file
    ofstream runFileOut(runFileName.c_str(), ios_base::out | ios_base::trunc);
    if ( ! runFileOut) {
        cerr << "Error: Can't open run file for write" << runFileName << endl;
        return false;
    }

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {9}")

    StringList::iterator runFileIt = runFileLines.begin();

    // first line of run script is copied through directly (shell invocation)
    runFileOut << *runFileIt++ << endl;
    // add some variable assignments for script
    runFileOut << "model=" << modelExecutable << endl;
    runFileOut << "genFlags=\"" << generalFlags << "\"" << endl;
    runFileOut << "sysFlags=\"" << systemFlags << "\"" << endl;
    runFileOut << "feedFlags=\"" << feederFlags << " " << extraFeederFlags << "\"" << endl;

    // copy remaining lines of original run script
    copy (runFileIt, runFileLines.end(),
          ostream_iterator<string>(runFileOut, "\n"));
    runFileOut.close();

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {10}")

    FileChmod (runFileName, 0755);

    SPEED_DEBUG("BenchmarkRunner::SetupBenchmarkDir() {11}")

    return true;
}

/**
 * Run the benchmark now.
 */
bool
BenchmarkRunner::Run (
    const string & runOptions)
{
    bool success = true;

    //
    // run the benchmark, ie. execute the run script
    //
    string cwd = GetCWD();

    if (chdir (benchmarkDir.c_str())) {
        cerr << "Error: Can't cd to benchmark run directory "
             << benchmarkDir << endl;
        return false;
    }
    string cmd = string("./run ") + runOptions;
    if (system (cmd.c_str()) != 0) {
        cerr << "Error running benchmark " << benchmark.GetName() << endl;
        success = false;
    }
    if (chdir (cwd.c_str()) && success) {
        cerr << "Error: Can't cd back to previous working directory "
             << cwd << endl;
        return false;
    }

    return success;
}

/**
 * Dump internal data structures to ostream.
 *
 * @return ostream for operation chaining
 */
ostream &
BenchmarkRunner::Dump(
    ostream & out,         ///< ostream to dump to
    const string & prefix) ///< prefix string to print on each line
const
{
    out << prefix << "BenchmarkRunner::" << endl;
    out << prefix << "  Benchmark = " << benchmark.GetName() << endl;
    out << prefix << "  BenchmarkConfigFile = " << benchmark.GetConfigFile()
        << endl;
    out << prefix << "  BenchmarkDir = " << benchmarkDir << endl;
    out << prefix << "  ModelExecutable: " << modelExecutable << endl;

    return out;
}


//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef TESTS

#include <list>

void TestRun (int argc, char ** argv)
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

        BenchmarkRunner runner(*workspace,
            benchmark, "/home/klauser/work/awb/bm",
            "/home/klauser/work/awb/pm/foo.exe");

        runner.Dump(cout);
        /*
        bool success = 
            runner.SetupBenchmarkDir();
        if ( ! success) {
            cerr << "Error setting up benchmark dir" << endl;
        }
        */
    }

    delete workspace;
}

int main (int argc, char ** argv)
{
#if 0
    if (argc >= 2) {
        TestRun (argc, argv);
    }
#endif
}

#endif // TESTS 
