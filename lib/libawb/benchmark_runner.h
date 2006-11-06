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

#ifndef _BENCHMARK_RUNNER_
#define _BENCHMARK_RUNNER_ 1

// generic (C++)
#include <string>
#include <vector>

// local
#include "workspace.h"
#include "benchmark.h"

using namespace std;

/**
 * @brief ASIM benchmark handler: support for running benchmarks on models
 *
 * This class holds state and methods needed for setup and running of
 * a benchmark on a model.
 */
class BenchmarkRunner {
  private:
    // types
    typedef vector<string> StringList;

    // members
    const Workspace & workspace;
    const Benchmark & benchmark;
    const string benchmarkDir;
    const string modelExecutable;
    
  public:
    // constructors / destructors
    BenchmarkRunner(const Workspace & theWorkspace,
        const Benchmark & theBenchmark, const string & theBenchmarkDir,
        const string & theModelExe);
    ~BenchmarkRunner();

    // top level driver methods
    /// Setup file in the benchmark directory (aka. 'setup').
    bool SetupBenchmarkDir (void);
    /// Run the benchmark.
    bool Run (const string & runOptions = "");

    // debug
    /// Dump state of internal data structures
    ostream & Dump (ostream & out, const string & prefix = "") const;
};

#endif // _BENCHMARK_RUNNER_ 
