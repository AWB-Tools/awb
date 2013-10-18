#
# Copyright (C) 2003-2006 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#

#
# ATTENTION MAINTAINERS!
#
# See the header comments in Commands.pm for how to add commands
# to asim-shell.
#


package AsimShell;
use warnings;
use strict;


sub help {
  help_commands();
}

sub help_commands {
  open(HELP, "| $::MORE_CMD");

  print HELP <<"END";

Usage:

  asim-shell [<asim-shell-command>]

Commands:

  set workspace <workspace>              - set default workspace

  new workspace <path>                   - create new workspace
  clone workspace <path>                 - clone the current workspace to <path>
    [--link]                             - create hard links to files in current workspace

  rehash workspace                       - rehash workspace
  edit workspace                         - edit workspace
  show workspace                         - show info about  workspace

  list bundles                           - list all bundles

  checkout bundle <bundle>[/<tag>]       - checkout a bundle of packages from their repositories
    [--user=<user>]                      - do checkout as <user>
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path
    [--golden]                           - check out the recent golden version

  clone bundle <bundle>[/<tag>]          - clone a bundle of packages from their 'distributed' repositories
    [--user=<user>]                      - do checkout as <user>
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path
    [--url]                              - url of the remote repository to clone
 
  pull bundle <bundle>[/<tag>]           - update a bundle of packages in a workspace
    [--nobuild]                          - do not rebuild updated packages
    [--url]                              - url of the remote repository to pull 
  
  use bundle <bundle>                    - copy a bundle of packages into workspace
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path
    [--golden]                           - check out the recent golden version

  update bundle <bundle>[/<tag>]         - update a bundle of packages in a workspace
    [--nobuild]                          - do not rebuild updated packages
 
  show bundle <bundle>                   - show the packages in a bundle
  
  new bundle <bundle>[/<tag>]            - create a new bundle
    [--model=[<apmfile>|.]        |      - from package dependencies of <apmfile> or default model,
     --experiment=<pkg>/<runtype> |      - or models in experiments/<pkg>/<runtype>/<runtype>.models,
     --packages=<packagelist>     |      - or comma-separated package list <packagelist>,
                                  ]      - or from all packages in the current workspace (default).
    [--head]                             - specify HEAD or branch name instead of current revision
    [--type=[release|baseline]]          - bundle type (defaults to "release")
    [--status=[Unknown|Success|Failure]] - status, for baseline bundles (defaults to "Unknown")
    [--file=<bundlefile> |               - write bundle information to file <bundlefile>,
     --install           |               - or to <sysprefix>/etc/asim/bundles/<type>/<bundle>.<type>,
                         ]               - or to default file ~/.asim/bundles/<type>/<bundle>.<type>
  
  awb                                    - invoke awb on current workspace

  new repository                         - create a new repository
  show repository                        - show characteristics of a repository

  rehash repositories                    - rehash the repository list
  list repositories                      - list repositories

  rehash packages                        - rehash the package list
  list packages                          - list checked out packages
  baseline packages                      - show version information of all workspace packages
    [--csn]                              - use CSN information from admin/packages instead of
                                           querying revision control system

  set package <name>                     - set default package
  unset package                          - leave default package undefined

  new package <name>                     - create a new package

  checkout package <repository>[/<tag>]  - checkout a package
    [--user=<user>]                      - do checkout as <user>
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path

  clone package <repository>[/<tag>]     - clone a package
    [--user=<user>]                      - do checkout as <user>
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path
    [--url]                              - url of the remote repository to clone

  use package <repository>[/<tag>]       - copy a package into workspace
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path

  add package <directory>                - prepend package at directory to 
                                           search path
  delete package [all|<package>...]      - delete a checked out package

  pull package [all|<package>...]        - update a package
    [--nobuild]                          - prevent ./configure and build of package
    [--url]                              -  url of the remote repository to pull
  
  show package [all|<package>...]        - show info about package

  regtest package [<package>]            - run regression test on package (obsolete)

  update package [all|<package>...]      - update a package
    [--nobuild]                          - prevent ./configure and build of package
    [--norehash]                         - prevent rehashing of module and model lists.

  upgrade package [all|<package>...]     - upgrade a package to the latest stable release

  commit package [all|<package>...]      - commit a package
    [--nodependent]                      - prevent commit of dependent packages
    [--commitlog=<commit comments file>] - Use in batch mode to supply commit comments
  
  push package [all|<package>...]        - push a package from local to a remote repository
    [--nodependent]                      - prevent commit of dependent packages
    [--url]                              - url of the remote repository to push to

  tag package [<package>] label          - tag current package revision with symbolic label
    [--existing]                         - move an existing tag to the current revision

  release package [<package>] version    - create a new release of a package
  branch package [<package>] branch      - create a branch named "branch"
  merge package [<package>] [CSN]        - merge a branch with the main trunk (HEAD/CSN)
  revert package [<package>]             - revert changes in the working copy of package

  lock package [<package>]               - lock a package
  unlock package [<package>]             - unlock a package

  cd package [<package>]                 - cd to root directory of package
  shell package [<package>]              - start a shell in the package
                                           directory
  cvs package [<package>] <command>      - issue cvs command on a package

  svn package [<package>] <command>      - issue svn command on a package

  build package [all|<package> ...]      - configure and make package(s)

  configure package [all|<package> ...]  - configure package(s)

  make package [all|<package> ...]       - build package(s)

  install package [all|<package> ...]    - install package(s)
    [--source]                           - install sharable source copy of package(s)

  clean package [all|<package> ...]      - clean up object files in package(s)

  status package [all|<package> ...]     - print status of package(s) against repository
    [--verbose]                          - print status of every file


  show configuration [all|<package> ...] - show build tree configuration info for package
    [--verbose]                          - include a more extensive list of output vars

  verify configuration [all|<pkg> ...]   - verify compatibility of package configurations
    [--exhaustive]                       - check the full list of 'configure' output variables


  list regressions                       - list existing regressions

  run regression [default|all|<pkg>...]  - run regression on packages
    [<swtiches>]                           switches to send to regression launcher

  verify regression                      - verify regression results

  clean regression                       - cleanup after regression

  delete regression                      - delete the latest regression

  rehash locks                           - rehash lock list
  list locks                             - list of all locks

  set lock <lockname>                    - set default lock
  unset lock                             - leave default lock undefined

  new lock [<lockname>]                  - create a new lock
  delete lock [<lockname>]               - destroy a lock
  lock lock  [<lockname>]                - acquire a lock
  unlock lock  [<lockname>]              - release a lock

  set model <model>                      - set default model
  unset model                            - leave default model undefined

  new model <model>                      - create a new model
  edit model [<model>]                   - edit a model configuartion

  clean model [<model>]                  - clean a model
    [--builddir <build-directory>]       - directory for model build

  nuke model [<model>]                   - totally cleans a model
    [--builddir <build-directory>]       - directory for model build

  configure model [<model>]              - configure a model
    [--builddir <build-directory>]       - directory for model build
    [--[no]persist]                      - hard links [soft links] from build area to source files

  ncfg model [<model>]                   - nuke & configure a model
    [--builddir <build-directory>]       - directory for model build
    [--[no]persist]                      - hard links [soft links] from build area to source files

  build model [<model>]                  - build a model
    [--builddir <build-directory>]       - directory for model build
    [--buildopt <make-options>]          - make options for build

  show model [<model>]                   - show a model configuration

  cd model [<model>]                     - cd to build directory of model

  update model [<model>...]              - update a model file

  set benchmark <benchmark>              - set default benchmark

  setup benchmark [<benchmark>]          - setup benchmark on a model
    [--model  <model>]                   - model to setup benchmark for
    [--rundir <run-directory>]           - directory for benchmark run

  run benchmark [<benchmark>]            - run benchmark on a model
    [--model  <model>]                   - model to run benchmark
    [--rundir <rundirectory>]            - directory for benchmark run
    [--runopt <run-options>]             - options for benchmark run

  list models                            - list the models
  rehash models                          - rehash the model list

  list modules                           - list the modules
  rehash modules                         - rehash the module list

  set module <module>                    - set default module
  unset module                           - leave default module undefined

  new module <module>                    - create a new module configuration
  edit module [<module>]                 - edit a module configuration
  show module [<module>]                 - show a module configuration

  rehash                                 - rehash all relevant lists
  status                                 - show current state

  ! <shell command>                      - execute a shell command
  cd <dir>                               - cd to directory
  pwd                                    - print current directory     

  source <file>                          - execute awb-shell commands in <file>

  help                                   - display this message
  help abstractions                      - display info on the asim
                                           abstractions
  help code                              - display help on the perl source
                                           code itself
  help examples                          - display an asim-shell cookbook
  help quickstart                        - display info on how to get started

  exit                                   - exit shell
  quit                                   - exit shell


  where,
    <workspace> is a directory containing an asim workspace

    <repository> is a repository name from a .pack file
    <tag> is a version or branch tag

    <package> is the name of a package (run 'list packages' for choices)

    <model> is a model .apm file
        default: \$AWB_MODEL or last model configured/built

    <benchmark> is a benchmark .cfg file
        default: \$AWB_BENCHARK or last benchmark setup/run

    <module> is a module .awb filename

  Note command completion and editing exists at the command prompt 
  and for most of the user input prompts. You may need to install
  the perl module Term::Readline::GNU for this to work...
  
  The <tag> argument for package commands has the following syntax and
  semantics:
  
    HEAD                       - the latest revision on the main trunk
    <branchname>               - the latest revision on the named branch
    CSN-<package>-<number>     - a specific revision on the main trunk
    CSN-<branchname>-<number>  - a specific revision on a branch
    <number>                   - a specific revision on the main trunk (SVN only)
    <branchname>:<number>      - a specific revision on a branch (SVN only)


Environment:

  ASIMSHELLOPT
    Contains additional command line switches, which are effectively
    prepended at the beginning of the command line.

  AWBLOCAL
    If AWBLOCAL is set, it is used to set the default workspace.
    AWBLOCAL must point to the directory that contains the awb.config file
    for the workspace. Using AWBLOCAL explictly is discouranged in favor
    of simply CDing into a workspace.

  AWB_MODEL
    If AWB_MODEL is set, it is used to set the default model.
    AWB_MODEL must point to a model configuration file (*.apm).

  AWB_BENCHMARK
    If AWB_BENCHMARK is set , it is used to set the default benchmark.
    AWB_BENCHMARK must point to a benchmark configuration file (*.cfg).

  ASIMEDITOR or EDITOR
    The ASIMEDITOR or EDITOR environment variable is used when a file needs 
    to be edited, e.g. the changes file during a commit procedure. If neither
    is defined emacs and gvim are tried.

  ASIMTMPDIR or TMPDIR
    Various procedures in asim-shell need to create temporary files.
    these variables point to the directory where these temporary files 
    will be created. If neither is defined /tmp will be used.

  PAGER
    If PAGER is set, its value us used as the name of the program to
    page through text output one screenful at a time. If it is not defined,
    "more" is used for paging.

  SHELL
    The SHELL environment variable is used as the program to start
    for the "shell package" command.

  CVS_RSH
    In order to start cvs commands on packages / repositories, CVS_RSH
    is used to select the correct remote connection method for cvs. If
    this variable is not set, asim-shell set a default of "ssh2" before
    any cvs commands are issued.

Files:
  ~/.asim/asimrc
    User-specific configuration file

  $Asim::sysconfdir/asim/asimrc
    Global configuration file

  ~/.asim/repositories.d/<name>.pack
    User-specific description of repositories (optional)

  ~/.asim/asim.pack
    User-specific description of repositories (deprecated/optional)

  $Asim::sysconfdir/asim/repositories.d/*.pack
    Global descriptions of repositories.

  $Asim::sysconfdir/asim/asim.pack
    More global descriptions of repositories (legacy).


END

  close(HELP);
  return 1;
}



sub help_quickstart {
  open(HELP, "| $::MORE_CMD");

  print HELP <<"END";

  To run asim-shell (and see this information) you need to have 
  asim-shell in your path, and the environment variable \$AWBLOCAL 
  defined. If you have a "standard" asim setup the following
  should work to get you started...

  First time only
  ---------------

  For csh-like shells:

    setenv PATH $Asim::bindir:\$PATH

    asim-shell new workspace <workspace>

    cd <workspace>


  For bourne-like shells:

    export PATH=$Asim::bindir:\$PATH

    asim-shell new workspace <workspace>

    cd <workspace>

  Where:
    <workspace> is a directory to create as your new workspace



  Subsequent uses
  ---------------

  For csh-like shells:

    setenv PATH $Asim::bindir:\$PATH

    cd <workspace>


  For bourne-like shells:

    export PATH=$Asim::bindir:\$PATH

    cd <workspace>

  Where:
    <workspace> is a directory you previously created as your workspace


  Now get more help with
  ----------------------

    asim-shell --help

    asim-shell
    asim> help commands
    asim> help examples


END

  my $sigpipe;
  if (defined $SIG{PIPE}) {
    $sigpipe = $SIG{PIPE};
  }
  $SIG{PIPE} = "IGNORE";

  close(HELP);

  if (defined $sigpipe) {
    $SIG{PIPE} = $sigpipe;
  }
  else {
    $SIG{PIPE} = "DEFAULT";
  }

  return 1;
}

sub help_examples {
  open(HELP, "| $::MORE_CMD");

  print HELP <<"END";

To use a set of locally available packages and start the Asim
Architects Workbench (awb):

   % cd <workspace> [1]
   % asim-shell
   asim> use bundle tukwila [2]
   ...copy and build happens here...
   asim> awb

To check out a set of packages from CVS and start the Asim
Architects Workbench (awb):

   % cd <workspace> [1]
   % asim-shell
   asim> checkout bundle tukwila [2]
   Repository user you want to do the checkout (e.g., your login name or anonymous) [joel]:
   ...checkout and build happens here...
   asim> awb


More examples are needed...

  [1] <workspace> is a directory you previously created as your workspace
  [2] Leave blank and hit <CR> to see a list of choices.

END

  close(HELP);
  return 1;
}


sub help_abstractions {
  help_code("Asim.pm");
}

sub help_code {
  my $item = shift;
  my $file;

  if (!defined($item)) {
    my @choices = ( "Asim.pm",
                    "Asim/Shell.pm",
                    "Asim/Workspace.pm",
                    "Asim/Repository.pm",
                    "Asim/Repository/Svn.pm",
                    "Asim/Repository/Git.pm",
                    "Asim/Repository/DB.pm",
                    "Asim/Package.pm",
                    "Asim/Package/BitKeeper.pm",
                    "Asim/Package/Copy.pm",
                    "Asim/Package/Cvs.pm",
                    "Asim/Package/DB.pm",
                    "Asim/Package/Svn.pm",
                    "Asim/Package/Git.pm",
                    "Asim/Package/Template.pm",
                    "Asim/BuildTree.pm",
                    "Asim/BuildTree/Configuration.pm",
                    "Asim/Lock.pm",
                    "Asim/Model.pm",
                    "Asim/Module/DB.pm",
                    "Asim/Module.pm",
                    "Asim/Benchmark.pm",
                    "Asim/Inifile.pm",
                    "Asim/Signal.pm",
                    "Asim/UnionDir.pm",
                    "Asim/Xaction.pm",
                    "Asim/Rcfile.pm",
                    "Asim/Packfile.pm",
                    "Asim/Base.pm",
                    "Asim/Batch.pm",
                    "Asim/Batch/Base.pm",
                    "Asim/Batch/Local.pm",
                    "Asim/Batch/Netbatch.pm",
                    "Asim/Batch/NetbatchRemote.pm",
                    "Asim/Nightly.pm",
                    "Asim/Regression.pm",
                    "Asim/GenCFG.pm",
                    "Asim/GenCFG/Auto.pm",
                    "Asim/Stats/ToText.pm",
                    "Asim/Stats/ToText/LongNames.pm",
                  );

    $item = Asim::choose_name("Choose perl module", @choices);
  }

  $file = Asim::resolve("libperl/Asim/lib/$item");
  system "perldoc $file";
}


BEGIN {
  #
  # determine which pager program the user prefers; if none is set
  # we fall back to use "more"
  #
  if (defined $ENV{"PAGER"} && $ENV{"PAGER"} ne "") {
    $::MORE_CMD = $ENV{"PAGER"};
  } else {
    $::MORE_CMD = "more";
  }
}

1;
