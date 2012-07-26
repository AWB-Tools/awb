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

// generic (C)
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>

// generic (c++)
#include <iostream>

// local
#include "amc.h"
#include "libawb/util.h"


/**
 * Create a new asim command line tool.
 */
AMC::AMC(void)
{
    workspace = Workspace::Setup();
    if ( ! workspace) {
        cerr << "Error: Workspace creation failed!" << endl;
        exit (1);
    }

    model = NULL;
    builder = NULL;
    benchmark = NULL;
    runner = NULL;
    persist_configureOption = 0;
}

/**
 * Destroy this asim command line tool.
 */
AMC::~AMC()
{
    // make sure we delete objects in reverse creation order, since later
    // allocated objects can depend on earlier ones;
    if (runner) {
        delete runner;
    }

    if (benchmark) {
        delete benchmark;
    }

    if (builder) {
        delete builder;
    }

    if (model) {
        delete model;
    }

    if (workspace) {
        delete workspace;
    }
}

/**
 * Process an ASIM command line
 */
void
AMC::ProcessCommandLine (
    int argc,
    char ** argv)
{
    SPEED_DEBUGN(1, "AMC::ProcessCommandLine() starting");
    poptContext optContext;   // context for parsing command-line options
    char c;
    char * c_modelFileName = NULL;
    char * c_benchmarkFileName = NULL;
    char * c_buildDir = NULL;
    char * c_buildOptions = NULL;
    char * c_modelExecutable = NULL;
    char * c_runDir = NULL;
    char * c_runOptions = NULL;
    int  * c_persist_configureOption = NULL;
    const char ** commands = NULL;

    struct poptOption helpOptionsTable[] = {
        { "help", 'h', POPT_ARG_NONE, NULL, 'h',
          "show this help message", NULL },
        { "usage", '\0', POPT_ARG_NONE, NULL, 'u',
          "display brief usage message", NULL },
        POPT_TABLEEND
    };

    struct poptOption optionsTable[] = {
        { "model", '\0', POPT_ARG_STRING,
          &c_modelFileName, 0,
          "performance model configuration file", "<file>" },
        { "benchmark", '\0', POPT_ARG_STRING,
          &c_benchmarkFileName, 0,
          "benchmark configuration file", "<file>" },
        { "builddir", '\0', POPT_ARG_STRING,
          &c_buildDir, 0,
          "root of build directory tree", "<dir>" },
        { "buildopt", '\0', POPT_ARG_STRING,
          &c_buildOptions, 0,
          "options passed on to make", "<options>" },
        { "modelexe", '\0', POPT_ARG_STRING,
          &c_modelExecutable, 0,
          "model executable file", "<file>" },
        { "rundir", '\0', POPT_ARG_STRING,
          &c_runDir, 0,
          "benchmark run directory", "<dir>" },
        { "runopt", '\0', POPT_ARG_STRING,
          &c_runOptions, 0,
          "options passed on to benchmark run", "<options>" },
        { "persist", '\0', POPT_ARG_NONE,
          &c_persist_configureOption, 0,
          "Create hard links to build sources during model configure", "<options>" },
//        { "nodynamicparams", '\0', POPT_ARG_NONE,
//          &noDynamicParams, 0,
//          "configure all parameters as static", NULL },
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
          "Help options:", NULL },
        POPT_TABLEEND
    };

    optContext = poptGetContext ("amc", argc,
        const_cast<const char**>(argv), optionsTable, 0);
    poptSetOtherOptionHelp (optContext, "[OPTION]* <command> ...");
    poptReadDefaultConfig(optContext, 0);

    if (argc < 2) {
        cerr << "Error: no command given" << endl;
        cerr << endl;
        poptPrintUsage(optContext, stderr, 0);
        exit(1);
    }

    //
    // now process options
    //
    while ((c = poptGetNextOpt(optContext)) >= 0) {
        switch (c) {
            case 'h': 
                PrintHelp (optContext);
                exit (0);
                break;
            case 'u':
                poptPrintUsage(optContext, stderr, 0);
                exit (0);
                break;
        }
        // all other options are handled automatically by popt
    }
    if (c < -1) {
        // an error occurred during option processing
        cerr << poptBadOption(optContext, POPT_BADOPTION_NOALIAS) << " : "
             << poptStrerror(c) << endl;
        cerr << endl;
        poptPrintUsage(optContext, stderr, 0);
        exit (1);
    }
    // remaining command line args are commands - processed later
    commands = poptGetArgs(optContext);

    //
    // copy all strings parsed on command line to C++ strings and sanitize
    // some file and directory names
    //
    if (c_modelFileName) {
        modelFileName = c_modelFileName;
    }
    if (c_benchmarkFileName) {
        benchmarkFileName = c_benchmarkFileName;
    }
    if (c_buildOptions) {
        buildOptions = c_buildOptions;
    }
    if (c_buildDir) {
        buildDir = c_buildDir;
        // rest of code assumes buildDir is absolute directory!
        if (IsRelativePath (buildDir)) {
            buildDir = FileJoin (GetCWD(), buildDir);
        }
    }
    if (c_modelExecutable) {
        modelExecutable = c_modelExecutable;
        // make absolute path - just in case
        if (IsRelativePath (modelExecutable)) {
            modelExecutable = FileJoin (GetCWD(), modelExecutable);
        }
    }
    if (c_runOptions) {
        runOptions = c_runOptions;
    }
    if (c_persist_configureOption) {
	persist_configureOption = 1;
    }
    if (c_runDir) {
        runDir = c_runDir;
        // make absolute path - just in case
        if (IsRelativePath (runDir)) {
            runDir = FileJoin (GetCWD(), runDir);
        }
    }

    //
    // process commands now
    //
    for (int i = 0; commands && commands[i]; i++) {
        string command = commands[i];

        if (command == "nuke") {
            cout << "Nuking build tree" << endl;
            SetupModelBuilder(optContext);
            bool success = builder->NukeBuildTree();
            if ( ! success) {
                cerr << "Error nuking build tree " << buildDir << endl;
            }
        } else if (command == "configure") {
            cout << "Configuring build tree" << endl;
            SetupModelBuilder(optContext);
            bool success = builder->CreateBuildTree(persist_configureOption);
            if ( ! success) {
                cerr << "Error creating build tree for model "
                     << modelFileName << endl;
            }
        } else if (command == "build") {
            cout << "Building model" << endl;
            SetupModelBuilder(optContext);
            bool success = builder->RunMake(buildOptions);
            if ( ! success) {
                cerr << "Error building model " << modelFileName << endl;
                exit (1);
            }
            // if a model executable name was given, we copy the
            // built target to the executable name and strip it;
            string builtExecutable = FileJoin (buildDir, model->GetFileName());
            if (builtExecutable != modelExecutable) {
                string cmd = string("strip -o ") + modelExecutable +
                    " " + builtExecutable;
                if (system (cmd.c_str()) != 0) {
                    cerr << "Error stripping model " << model->GetName()
                         << endl;
                    exit (1);
                }
            }
        } else if (command == "setup") {
            cout << "Setting up benchmark" << endl;
            SPEED_DEBUGN(1, "AMC::ProcessCommandLine()->SetupBenchmarkRunner()");
            SetupBenchmarkRunner(optContext);
            SPEED_DEBUGN(-1, "AMC::ProcessCommandLine()->SetupBenchmarkRunner() done");
            SPEED_DEBUGN(1, "AMC::ProcessCommandLine()->runner->SetupBenchmarkDir() about to begin");
            bool success = runner->SetupBenchmarkDir();
            SPEED_DEBUGN(-1, "AMC::ProcessCommandLine()->runner->SetupBenchmarkDir() done");
            if ( ! success) {
                cerr << "Error setting up benchmark " << benchmarkFileName
                     << endl;
            }
        } else if (command == "run") {
            cout << "Running benchmark" << endl;
            SetupBenchmarkRunner(optContext);
            bool success = runner->Run(runOptions);
            if ( ! success) {
                cerr << "Error running benchmark " << benchmarkFileName
                     << endl;
            }
        } else {
            cerr << "Error: unknown command '" << command << "'" << endl;
            cerr << endl;
            PrintHelp (optContext);
            exit (1);
        }
    }

    poptFreeContext(optContext);

    SPEED_DEBUGN(-1, "AMC::ProcessCommandLine() ending");
}

/**
 * Print help message
 */
void
AMC::PrintHelp (
    const poptContext & optContext) ///< command-line parsing context
{
    poptPrintHelp(optContext, stderr, 0);
    cerr << endl;
    cerr << "Commands:" << endl;
    cerr << "  nuke                    remove everything in build tree"
         << endl;
    cerr << "  configure               configure build tree" << endl;
    cerr << "  build                   build the model" << endl;
    cerr << "  setup                   setup the benchmark directory contents"
         << endl;
    cerr << "  run                     run the benchmark" << endl;
}

/**
 * Set up any objects required for the model
 */
void
AMC::SetupModel (
    const poptContext & optContext) ///< command-line parsing context
{
    // setup the model if we have not already done so
    if ( ! model) {
        if ( ! modelFileName.empty()) {
            model = new Model (*workspace);
            SPEED_DEBUGN(1, "AMC::SetupModel()->Parse() about to begin");
            if ( ! model->Parse (modelFileName)) {
                cerr << "Error: could not parse model " << modelFileName
                     << endl;
                exit (1);
            }
            SPEED_DEBUGN(-1, "AMC::SetupModel()->Parse() done");

            // synthesize a backward compatible buildDir
            buildDir = TranslateBuildDir();

            if (modelExecutable.empty()) {
                // synthesize a backward compatible modelExecutable
                modelExecutable = FileJoin (buildDir, model->GetFileName());
            }
        } else {
            cerr << "Error: missing model" << endl;
            poptPrintUsage(optContext, stderr, 0);
            exit (1);
        }
    }
}

/**
 * Set up any objects required for the model builder
 */
void
AMC::SetupModelBuilder (
    const poptContext & optContext) ///< command-line parsing context
{
    SetupModel (optContext);

    // setup the model builder, if we have not already done so
    if ( ! builder) {
        builder = new ModelBuilder(*workspace, *model, buildDir);
        if ( ! builder) {
            cerr << "Error: could not create model builder for model "
                 << modelFileName << endl
                 << "       with build dir " << buildDir << endl;
            exit (1);
        }
    }
}

/**
 * Set up any objects required for the benchmark
 */
void
AMC::SetupBenchmark (
    const poptContext & optContext) ///< command-line parsing context
{
    SPEED_DEBUGN(1, "AMC::SetupBenchmark() begin");
    // setup the benchmark if we have not already done so
    if ( ! benchmark) {
        if ( ! benchmarkFileName.empty()) {
            benchmark = new Benchmark (*workspace);
            SPEED_DEBUGN(1, "AMC::SetupBenchmark()->benchmark->Parse() about to begin");
            if ( ! benchmark->Parse (benchmarkFileName)) {
                cerr << "Error: could not parse benchmark " << benchmarkFileName
                     << endl;
                exit (1);
            }
            SPEED_DEBUGN(-1, "AMC::SetupBenchmark()->benchmark->Parse() done");
        } else {
            cerr << "Error: missing benchmark" << endl;
            poptPrintUsage(optContext, stderr, 0);
            exit (1);
        }
    }
    SPEED_DEBUGN(-1, "AMC::SetupBenchmark() done");
}


/**
 * Set up any objects required for the benchmark runner
 */
void
AMC::SetupBenchmarkRunner (
    const poptContext & optContext) ///< command-line parsing context
{
    SPEED_DEBUGN(1, "AMC::SetupBenchmarkRunner() about to begin");
    SetupBenchmark (optContext);

    // setup the benchmark runner, if we have not already done so
    if ( ! runner) {
        if (runDir.empty()) {

            // synthesize a backward compatible runDir
            runDir = FileJoin ( FileJoin ( FileJoin (
                workspace->GetDirectory(Workspace::BuildDir),
                model->TranslateFileName(modelFileName)),
                "bm"),
                benchmark->GetFileName());
        }
        if (modelExecutable.empty()) {

            // synthesize a backward compatible modelExecutable
            modelExecutable = FileJoin (TranslateBuildDir(), model->TranslateFileName(modelFileName));
        }
        SPEED_DEBUGN(1, "AMC::SetupBenchmarkRunner()->newBenchmarkRunner() about to begin");
        runner = new BenchmarkRunner (*workspace, *benchmark, runDir,
                                      modelExecutable);
        SPEED_DEBUGN(-1, "AMC::SetupBenchmarkRunner()->newBenchmarkRunner() done");
        if ( ! runner) {
            cerr << "Error: could not create benchmark runner for "
                 << benchmarkFileName << endl
                 << "       with run dir " << runDir << endl;
            exit (1);
        }
    }
    SPEED_DEBUGN(-1, "AMC::SetupBenchmarkRunner() end");
}

//----------------------------------------------------------------------------

int main (int argc, char ** argv)
{
    AMC amc;

    amc.ProcessCommandLine (argc, argv);
}
