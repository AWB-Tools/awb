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

  rehash workspace                       - rehash workspace
  edit workspace                         - edit workspace
  show workspace                         - show info about  workspace

  list bundles                           - list all bundles

  checkout bundle <bundle>               - CVS checkout a bundle of packages
    [--user=<user>]                      - do checkout as <user>
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path
    [--golden]                           - check out the recent golden version

  use bundle <bundle>                    - copy a bundle of packages into workspace
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path
    [--golden]                           - check out the recent golden version

  show bundle <bundle>                   - show the packages in a bundle
  
  awb                                    - invoke awb on current workspace

  rehash repositories                    - rehash the repository list
  list repositories                      - list repositories

  rehash packages                        - rehash the package list
  list packages                          - list checked out packages

  set package <name>                     - set default package
  unset package                          - leave default package undefined

  new package <name>                     - create a new package

  checkout package <repository>[/<tag>]  - CVS checkout a package
    [--user=<user>]                      - do checkout as <user>
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path

  use package <repository>[/<tag>]       - copy a package into workspace
    [--nobuild]                          - do not build package
    [--noaddpath]                        - do not add package to search path

  delete package <name>                  - delete a checked out package
  show package [<package>]               - show info about package

  add package <directory>                - prepend package at directory to 
                                           search path

  regtest package [<package>]            - run regression test on package

  update package [<package>]             - CVS update a package
    [--nobuild]                          - prevent ./configure and build of package

  update package * | all                 - CVS update all checked out packages
    [--nobuild]                          - prevent ./configure and build of package

  commit package [<package>]             - CVS commit a package
    [--nodependent]                      - prevent commit of dependent packages

  release package [<package>] version    - create a new release of a package
  branch package [<package>] branch      - create CVS branch named "branch"
  merge package [<package>] [CSN]        - CVS merge a branch with the
                                           main trunk (HEAD/CSN)

  lock package [<package>]               - lock a package
  unlock package [<package>]             - unlock a package

  cd package [<package>]                 - cd to root directory of package
  shell package [<package>]              - start a shell in the package
                                           directory
  cvs package [<package>] <command>      - issue cvs command on a package

  svn package [<package>] <command>      - issue svn command on a package

  build package [<package> ...]          - configure and make package(s)
  configure package [<package> ...]      - configure package(s)
  make package [<package> ...]           - build package(s)
  install package [<package> ...]        - install package(s)
  clean package [<package> ...]          - clean up object files in package(s)

  status package [<package> ...]         - print status of package(s) against repository
    [--verbose]                          - print status of every file

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

  ncfg model [<model>]                   - nuke & configure a model
    [--builddir <build-directory>]       - directory for model build

  build model [<model>]                  - build a model
    [--builddir <build-directory>]       - directory for model build
    [--buildopt <make-options>]          - make options for build

  setup model [<model>] <benchmark>      - setup benchmark on a model
    [--rundir <run-directory>]           - directory for benchmark run

  run model [<model>] <benchmark>        - run benchmark on a model
    [--rundir <rundirectory>]            - directory for benchmark run
    [--runopt <run-options>]             - options for benchmark run

  show model [<model>]                   - show a model configuration

  cd model [<model>]                     - cd to build directory of model

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
    <tag> is a CVS version or branch tag
    <package> is the name of a package (run 'list packages' for choices)
    <model> is a model .apm filename
    <module> is a module .awb filename

  Note command completion and editing exists at the command prompt 
  and for most of the user input prompts. You may need to install
  the perl module Term::Readline::GNU for this to work...


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

  PAGER
    If PAGER is set, its value us used as the name of the program to
    page through text output one screenful at a time. If it is not defined,
    "more" is used for paging.

  EDITOR
    The EDITOR environment variable is used when a file needs to be
    edited, e.g. the changes file during a commit procedure.

  SHELL
    The SHELL environment variable is used as the program to start
    for the "shell package" command.

  TMPDIR
    Various procedures in asim-shell need to create temporary files.
    TMPDIR points to the directory where these temporary files will be
    created in.

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

  ~/.asim/asim.pack
    User-specific description of repositories (optional)

  $Asim::sysconfdir/asim/asim.pack
    Global description of repositories, which is used 
    if ~/.asim/asim.pack does not exist.

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
                    "Asim/Repository/DB.pm",
                    "Asim/Package/DB.pm",
                    "Asim/Package.pm",
                    "Asim/Lock.pm",
                    "Asim/Cvs.pm",
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
