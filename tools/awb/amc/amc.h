//
// Copyright (C) 2002-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// 

/**
 * @file
 * @author Artur Klauser
 * @brief AWB Model Commander (AMC) - AWB command line tool
 */

#ifndef _AMC_
#define _AMC_ 1

// local
#include "libawb/workspace.h"
#include "libawb/model.h"
#include "libawb/model_builder.h"
#include "libawb/benchmark.h"
#include "libawb/benchmark_runner.h"

using namespace std;

class AMC {
  private:
    Workspace * workspace;
    Model * model;
    ModelBuilder * builder;
    Benchmark * benchmark;
    BenchmarkRunner * runner;

    string modelFileName;
    string benchmarkFileName;
    string buildDir;
    string buildOptions;
    string modelExecutable;
    string runDir;
    string runOptions;
    int    persist_configureOption;

  public:
    // constructors / destructors
    AMC();
    ~AMC();

    void ProcessCommandLine (int argc, char ** argv);

  private:
    void SetupModel (const poptContext & optContext);
    void SetupModelBuilder (const poptContext & optContext);
    void SetupBenchmark (const poptContext & optContext);
    void SetupBenchmarkRunner (const poptContext & optContext);
    void PrintHelp (const poptContext & optContext);

    std::string TranslateBuildDir()
        { return buildDir.empty() 
              ? FileJoin ( FileJoin (workspace->GetDirectory(Workspace::BuildDir),
                                     model->TranslateFileName(modelFileName)),
                           "pm")
              : buildDir; };
};

#endif // _AMC_
