:
eval 'exec perl "$0" ${1+"$@"}'
       if 0;

#
# Copyright (C) 2002-2006 Intel Corporation
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


################################################################
#
#  The purpose of this program is to take two runs of a 
#  regression test and compare the output results.  We can also 
#  run a test called a sanity test.  The benchmarks for this test
#  labeled S-* in the benchmarks file. The sanity checks to make
#  sure that we have repeatable results for a benchmark. The
#  test starts at a certain point, runs in full trace mode for 
#  fixed number of cycles, repeats the run, and compares the 
#  results.  It then skips a number of cycles and repeats the 
#  procedure until it reaches the end point of the simulation. 
#
################################################################
################################################################
################################################################

#
# Read the level of regression to test.  By default, this 
# test case is run with level A regression unless otherwise
# specified. 
#

$SITE_CONFIG_FILE = "/usr/local/share/asim/etc/regtestrc";
$USER_CONFIG_FILE = expand_tilda( "~/.regtestrc" );

$SANITY_SCRIPT = "tools/sanity\.pl";
$RANDOM_SCRIPT = "tools/random\.pl";
$RANDOM_MT_SCRIPT = "tools/random_mt\.pl";
$REGLEVEL_SCRIPT = "tools/reglevel\.pl";
$RANDOM_GEN_SCRIPT = "tools/random_setup_mt\.pl";
$REG_DIR = regtest;
$LSF_LOG = "lsf\.log";
$REG_LEVEL = "ALL";
$FEEDER = "ALL";
$WATCH_TIMES = "";
$AINT_EXEC = arana_aint_dev;
$MT_AINT_EXEC = arana_mt_aint_dev;
$CMP_AINT_EXEC = tiburon_cmp_aint_dev;
$ATF_EXEC = arana_atf_dev;
$AINT_CFG = "";
$ATF_CFG = "";
$ATOM_EXEC = arana_atom_dev;
$BUILD_DIR = "";
$RESULT_DIR = "";
$CFG_DIR = "config/pm/Arana";
$CMP_CFG_DIR = "config/pm/Tiburon";
$RESOLVER = "awb-resolver";
$GETCONFIG = "getconfig";
$NQS_REJECT = 0;

chomp ($msg = `$RESOLVER .`);
if ($? != 0) {
  die "Can't find $RESOLVER - check your PATH environemt variable\n$msg\n";
}

# This states that the size of files will be no more than 10MB.  This
# is to avoid dumping huge files and filling up the disk. 
$FILE_SIZE_LIMIT = 10000;
 
# Script by default assumes that you're running regression runs rather
# than level BESPEC, for example. 
$BENCHMARK_CFG_DIR = "Regression";

# 
# Name of file which contains the parameters used for the run. 
# This file is copied into the regression run directory for future reference. 
$PARAM_FILE = "exec\.params";

# Default queue. 
$QUEUE="regression";

# Just build executable
$BUILD_ONLY = 0;

# Do not build executable. 
$NO_BUILD = 0;

# Do not run under LSF.  Run on local machine. 
$NO_LSF = 0;

# Run verifier with the regression test. 
$VERIFY = 1;        

# Run oracle with the regression test. 
$ORACLE = 0;        

$TRACE_FLAGS = "";

# Save readme flag. 
$README_FILE = 0;

# Do set up benchmarks
$NO_SETUP = 0;

# Do not print a pulse
$PULSE = 1000;

# Don't bother to check running tests, just launch them
$NO_WATCH = 0;

#
# Parameters for the sanity check. 
# Start at 1 million cycles, warmup for 10000 cycles, run for 200
# cycles and compare, then skip 2,000,000 cycles and repeat until 
# we reach 20,000,000 cycles. 
#

$SANITY_PARAMS = "1000000:10000:200:4000000:10000000";
$BENCHMARK_FL = "";

#
# Setup the random check.
# default number of runs is 5, and the number of cycles run
# in simulator is 100000.
# 
srand;
$RANDOM_NUM_RUNS = 5;
$RANDOM_NUM_CYCLES = 100000;


# default Queueing System
$QUEUE_SYS="lsf";
$nqs_script_num = 0;

#
# The home directory is going to be blasted away, so save a pointer to 
# it, along with the user name.  
#
chomp ($home_dir = `$RESOLVER -config awblocal`);
if ($? != 0) {
 die "ERROR:\n$home_dir\n";
}

$user_name = $ENV{USER};

#
# Define the sampling delays for different types of regression tests. 
# These delays are used to determine if the regression test is complete
# or not, and when a mail is sent to the user indicating that the
# tests are complete. 
#  
%INIT_TIME = ("ALL" => 1000,
	      "A" => 10,
	      "B" => 300,
	      "M" => 300,
	      "C" => 10000,
	      "S" => 100,
	      "R" => 100,
	      "MICRO" => 10);
%MAX_TIME = ("ALL" => 5000,
	      "A" => 500,
	      "B" => 1000,
	      "M" => 1000,
	      "C" => 20000,
	      "S" => 500,
	      "R" => 500,
	      "MICRO" => 500);
	       
#
# we parse arguments by
# - first loading the site-specific config file
# - second loading the user-specific config file
# - and then the command line arguments
#

if (defined $SITE_CONFIG_FILE && -r $SITE_CONFIG_FILE) {
  parse_flags("site config file $SITE_CONFIG_FILE",
              split(' ', `cat $SITE_CONFIG_FILE`));
}

if (defined $USER_CONFIG_FILE && -r $USER_CONFIG_FILE) {
  parse_flags("user config file $USER_CONFIG_FILE",
              split(' ', `cat $USER_CONFIG_FILE`));
}

parse_flags("command line arguments", @ARGV);

sub parse_flags {
  my $source = shift;
  my @flags = @_;
  my $curr_flag;

  while ($curr_flag = shift @flags) {
    if ($curr_flag eq "-level") {
      $REG_LEVEL = shift @flags;
    }
    elsif ($curr_flag eq "-sparams") {
      $SANITY_PARAMS = shift @flags;
    }
    elsif  ($curr_flag eq "-aintexec") {
      $AINT_EXEC = shift @flags;
    }
    elsif  ($curr_flag eq "-mt_aintexec") {
      $MT_AINT_EXEC = shift @flags;
    }
    elsif  ($curr_flag eq "-cmp_aintexec") {
      $CMP_AINT_EXEC = shift @flags;
    }
    elsif ($curr_flag eq "-atfexec") {
      $ATF_EXEC = shift @flags;
    }
    elsif ($curr_flag eq "-atomexec") {
      $ATOM_EXEC = shift @flags;
    }
    elsif ($curr_flag eq "-feeder") {
      $FEEDER = shift @flags;
    }
    elsif ($curr_flag eq "-benchmarkset") {
      $BENCHMARK_CFG_DIR = shift @flags;
    }
    elsif ($curr_flag eq "-benchmarks") {
      $BENCHMARK_FL = shift @flags;
    }
    elsif ($curr_flag eq "-watch") {
      $WATCH_TIMES = shift @flags;
    }
    elsif ($curr_flag eq "-nowatch") {
      $NO_WATCH = 1;
    }
    elsif ($curr_flag eq "-filesize") {
      $FILE_SIZE_LIMIT = shift @flags;
    }
    elsif ($curr_flag eq "-rparams") {
      $RANDOM_NUM_RUNS = shift @flags;
    }
    elsif ($curr_flag eq "-queue") {
      $QUEUE = shift @flags;
    }
    elsif ($curr_flag eq "-queueresource") {
      $QUEUE_RESOURCE = shift @flags;
    }
    elsif ($curr_flag eq "-aintcfg") {
      $AINT_CFG = shift @flags;
    }
    elsif ($curr_flag eq "-mtaintcfg") {
      $MT_AINT_CFG = shift @flags;
    }
    elsif ($curr_flag eq "-cmpaintcfg") {
      $CMP_AINT_CFG = shift @flags;
    }
    elsif ($curr_flag eq "-atfcfg") {
      $ATF_CFG = shift @flags;
    }
    elsif ($curr_flag eq "-buildonly") {
      $BUILD_ONLY = 1;
    }
    elsif ($curr_flag eq "-buildopt") {
      my $opt = shift @flags;
      $BUILD_OPTIONS = "-opt \"$opt\"";
    }
    elsif ($curr_flag eq "-spec") {
      $BENCHMARK_CFG_DIR = "Spec95";
    }
    elsif ($curr_flag eq "-nobuild") {
      $NO_BUILD = 1;
    }
    elsif ($curr_flag eq "-nosetup") {
        $NO_SETUP = 1;
    }
    elsif ($curr_flag eq "-pulse") {
        $PULSE = shift @flags;
    }
    elsif ($curr_flag eq "-nolsf") {
        $NO_LSF = 1;
    }
    elsif ($curr_flag eq "-traceflags") {
        $TRACE_FLAGS = shift @flags;
    }
    elsif ($curr_flag eq "-noverify") {
        $VERIFY=0;
    }
    elsif ($curr_flag eq "-readme") {
        $README_FILE=1;
    }
    elsif ($curr_flag eq "-oracle") {
        $ORACLE=1;
    }
    elsif ($curr_flag eq "-expname") {
        $EXP_NAME = shift @flags;
    }
    elsif ($curr_flag eq "-nqs") {
        $QUEUE_SYS="nqs";
    }
	elsif ($curr_flag =~ "-patchnqs") {
		$NQS_REJECT=1;
	}
    elsif ($curr_flag eq "-help") {
      &usage_and_exit();
    }
    else {
      print STDERR "ERROR: unknown argument $curr_flag in $source\n";
      print STDERR "in line: " . join(" ", @_) . "\n";
      &usage_and_exit();
    }
  }
}

# Determine if queue is legal. 
if ($QUEUE =~ m/^(normal|debug|regression|exerciser)$/) {
    if ($QUEUE eq "regression") {
	print ("WARNING....WARNING....WARNING....WARNING....\n");
	print ("USING QUEUE $QUEUE\n");
	$answer = getinput("DO YOU REALLY WANT TO RUN ON THE REGRESSION QUEUE (y,n)?", "n");
	if ($answer eq "n" || $answer eq "N") {
	    print ("CHANGING QUEUE TO normal\n");
	    $QUEUE="normal";
	}
    }
}
else {
    print ("ERROR: Queue $QUEUE is not a legal queue name.\n");
    exit (1);
}

# 
# Parse the params for the sanity check. 
#
if ($REG_LEVEL eq "S" || $REG_LEVEL eq "ALL") {
  @sparams = split (":", $SANITY_PARAMS);
  $param_count = @sparams + 0;
  ($param_count == 5) || 
    die "ERROR: Sanity parameters $SANITY_PARAMS are not specified correctly: $!";
  
  # Make sure that the starting point is less than 
  # the ending point. 
  # Syntax is start:duration:skip:end
  ($sparams[0] < $sparams[4]) || 
    die "ERROR: Starting point $sparams[0] must be less than ending point $sparams[4]: $!";
  ($sparams[1] >= 1000) || 
    die "ERROR: Warmup must be greater than 1000 cycles: $!";
  ($sparams[1] < 100000) || 
    die "ERROR: Warmup must be less than 100000 cycles: $!";
  ($sparams[2] > 10) || 
    die "ERROR: Duration must be greater than 10 instructions: $!";
  ($sparams[2] <= 1000) || 
    die "ERROR: Duration must be <= 1000 instructions: $!";
  ($sparams[3] > ($sparams[1] + $sparams[2])) || 
    die "ERROR: Skip distance must be greater than duration + warmup: $!";
}

#
# Parse the params for the watch times. 
#
if ($WATCH_TIMES ne "") {
  @watch_times = split (":", $WATCH_TIMES);
  $watch_count = @watch_times + 0;
  ($watch_count == 2) || 
    die "ERROR: Run time parameters are not specified correctly: $!";
  ($watch_times[0] > 0) || 
    die "ERROR: Initialization time before checking for completion must be > 0: $!";
  ($watch_times[1] > 10) || 
    die "ERROR: Wait time must be greater than 10 seconds: $!";

  $INIT_TIME[$REG_LEVEL] = $watch_times[0];
  $MAX_TIME[$REG_LEVEL] = $watch_times[1];
}

#
# Setup location of benchmark models 
#
$AINT_BM = "config/bm/Aint";
$MT_AINT_BM = "config/bm/MT_Aint";
$CMP_AINT_BM = "config/bm/Aint";
$ATF_BM = "config/bm/Atf";

chomp ($AWB = `$RESOLVER awb/asim`);
if ($? != 0) {
 die "ERROR:\n$AWB\n";
}

# 
# Read the names of the regression test cases from file.
#

if ($BENCHMARK_FL eq "") {
  chomp ($benchmarks_file = `$RESOLVER $REG_DIR/benchmarks`);
  if ($? != 0) {
   die "ERROR:\n$benchmarks_file\n";
  }
}
elsif ($BENCHMARK_FL =~ /^\/\S+/) {
  $benchmarks_file = $BENCHMARK_FL;
}
else {
  $benchmarks_file = $ENV{PWD} . "/$BENCHMARK_FL";
}

# 
# Determine the location of the build directory. 
#
chomp ($BUILD_DIR = `$RESOLVER -config builddir`);
if ($? != 0) {
 die "ERROR:\n$BUILD_DIR\n";
}
print ("ASIM BUILD DIRECTORY: $BUILD_DIR\n\n");

#
# Build executable. 
#
if (! $AINT_CFG &&
    ($FEEDER eq "ALL" || $FEEDER eq "AINT")) {
  chomp ($AINT_CFG = `$RESOLVER $CFG_DIR/$AINT_EXEC.config`);
  if ($? != 0) {
   die "ERROR:\n$AINT_CFG\n";
  }
}
if (! $MT_AINT_CFG &&
    ($FEEDER eq "ALL" || $FEEDER eq "MT_AINT")) {
  chomp ($MT_AINT_CFG = `$RESOLVER $CFG_DIR/$MT_AINT_EXEC.config`);
  if ($? != 0) {
   die "ERROR:\n$MT_AINT_CFG\n";
  }
}
if (! $CMP_AINT_CFG &&
    ($FEEDER eq "ALL" || $FEEDER eq "CMP_AINT")) {
  chomp ($CMP_AINT_CFG = `$RESOLVER $CMP_CFG_DIR/$CMP_AINT_EXEC.config`);
  if ($? != 0) {
   die "ERROR:\n$CMP_AINT_CFG\n";
  }
}
if (! $ATF_CFG &&
    ($FEEDER eq "ALL" || $FEEDER eq "ATF")) {
  chomp ($ATF_CFG = `$RESOLVER $CFG_DIR/$ATF_EXEC.config`);
  if ($? != 0) {
   die "ERROR:\n$ATF_CFG\n";
  }
}

#
# If requested, build models and dump the packages file for 
# each model into the regtest results directory. 
#
local @config_files;
if ($FEEDER eq "ALL" || $FEEDER eq "AINT") {
  if ($NO_BUILD == 0) {
    if (system ("$AWB -pm $AINT_CFG $BUILD_OPTIONS clean build")) {
      print "ERROR: UNABLE TO BUILD AINT EXECUTABLE WITH CONFIGURATION FILE $AINT_CFG.\n";
      exit (1);
    }
  }
  push @config_files, $AINT_CFG;
}

if ($FEEDER eq "ALL" || $FEEDER eq "MT_AINT") {
  if ($NO_BUILD == 0) {
    if (system ("$AWB -pm $MT_AINT_CFG $BUILD_OPTIONS clean build")) {
      print "ERROR: UNABLE TO BUILD MT_AINT EXECUTABLE WITH CONFIGURATION FILE $MT_AINT_CFG.\n";
      exit (1);
    }
  }
  push @config_files, $MT_AINT_CFG;
}

if ($FEEDER eq "ALL" || $FEEDER eq "CMP_AINT") {
  if ($NO_BUILD == 0) {
    if (system ("$AWB -pm $CMP_AINT_CFG $BUILD_OPTIONS clean build")) {
	  print "ERROR: UNABLE TO BUILD CMP_AINT EXECUTABLE WITH CONFIGURATION FILE $CMP_AINT_CFG.\n";
	  exit (1);
    }
  }
  push @config_files, $CMP_AINT_CFG;
}

if ($FEEDER eq "ALL" || $FEEDER eq "ATF") {
  if ($NO_BUILD == 0) {
    if (system ("$AWB -pm $ATF_CFG $BUILD_OPTIONS clean build")) {
      print "ERROR: UNABLE TO BUILD ATF EXECUTABLE WITH CONFIGURATION FILE $ATF_EXEC.\n";
      exit (1);
    }
  }
  push @config_files, $ATF_CFG;
}

#
# Setup the regression test directories. 
#
if ( $BUILD_ONLY == 0 ) {
  if (&SetupRegression) {
    print "ERROR: Problems setting up regression results directory\n";
    print "EXITING REGRESSION SUITE\n";
    exit (1);
  }
  #
  # Copy the packages files referenced by the configuration files in 
  # array config_files and place them in the ADMIN directory of the 
  # regression results. 
  #
  if (&CopyPackages(\@config_files, "$RESULT_DIR" . "/ADMIN")) {
    print "ERROR: Unable to copy packages file into $RESULT_DIR/ADMIN\n";
    print "EXITING REGRESSION SUITE\n";
    exit (1);
  }
}
else {
  # We've only been asked to build the executable and not run any benchmarks. 
  print "SUCCESSFULLY BUILT EXECUTABLE FOR ";
  if ($FEEDER eq "ALL") {
    print "AINT AND MT_AINT AND ATF.  \n";
  }
  else {
    print "$FEEDER.\n";
  }
  exit (0);
}

#
# Check to see if the user wants to record some information about this run. 
#
if ($README_FILE == 1) {
  $EDITOR = $ENV{'EDITOR'};
  if ( ! $EDITOR ) {
    die "Please, set your \$EDITOR variable properly\n";
  }
  else {
    print "\n\n";
    print "I will now launch your favorite editor on a README file\n";
    print "so that you can type in your comments";
    print "."; sleep 2;
    print "."; sleep 2;
    print ".\n\n\n"; sleep 2;
    system "$EDITOR $RESULT_DIR/README";
    print "Comments are recorded in file $RESULT_DIR/README.\n\n"
  }
}
 
#
# Run the benchmark, and compare the results.  The benchmark is run
# in a "clean" environment so that there are not variations across
# different users.  For now, I'm just doing a diff on the files. 
# In the future, I would like to get this printed out in a nice format. 
#
$num_exec = 0;
open (BENCHMARKS_FILE, $benchmarks_file) || die "ERROR: cannot open benchmarks file $benchmarks_file: $!";
while ($benchmark = <BENCHMARKS_FILE>) {
  chomp $benchmark;
  next if ($benchmark =~ m/^\s*#/); # skip comment lines
  $benchmark =~ s/\s+$//;
  @program = split (/-/, $benchmark);
  $benchmark_name = $program[2];
  
  # Find the level of benchmark to run. 
  $program[0] =~ tr/a-z/A-Z/;
  if ($program[0] =~ /[A-CFMSR]/) {

    # Determine if this level matches the level chosen by the user. 
    # If REG_LEVEL is not set, then we run all levels. Otherwise, 
    # we only run the requested level. 
    if (($REG_LEVEL eq "ALL" || $program[0] eq $REG_LEVEL) &&
	($FEEDER eq "ALL" || $program[1] eq $FEEDER)) {

      # Find the executable (ATF | AINT | MT_AINT | ATOM)
      $program[1] =~ tr/a-z/A-Z/;

      # Create benchmark configuration file name
      if ($program[1] eq "ATF") {
        $bm = $ATF_BM;
      } elsif ($program[1] eq "AINT") {
        $bm = $AINT_BM;
      } elsif ($program[1] eq "MT_AINT") {
	$bm = $MT_AINT_BM;
      } elsif ($program[1] eq "CMP_AINT") {
	$bm = $CMP_AINT_BM;
      }

      $benchmark_cfg = "$BENCHMARK_CFG_DIR/Level-$program[0]/$benchmark.cfg";

      # if we can't find this file, drop the Level- prefix;
      # note that only "Regression" is likely to use the Level- prefix anyway;
      chomp ($bm_cfg_file = `$RESOLVER $bm/$benchmark_cfg`);
      if (! -e "$bm_cfg_file") {
        $benchmark_cfg = "$BENCHMARK_CFG_DIR/$program[0]/$benchmark.cfg";
        chomp ($bm_cfg_file = `$RESOLVER $bm/$benchmark_cfg`);
        if ($? != 0) {
          # too bad, we can't find the file - error msg is in bm_cfg_file
          die "ERROR:\n$bm_cfg_file\n";
        }
      }

      if ($program[1] eq "ATF") {
	#
	# Setup the benchmarks directory unless the $NO_SETUP flag is set.
	if ($NO_SETUP == 0) {			
	  system ("$AWB -pm $ATF_CFG -bm $bm_cfg_file setup");
	}

	$benchmark_dir = $BUILD_DIR . "/$ATF_EXEC/bm" . "/$benchmark";
	if (! chdir ($benchmark_dir)) {
	  print "ERROR: cannot cd to $benchmark_dir\n";
	  print "SKIPPING benchmark $benchmark\n\n";
	  next;
	}
      }
      elsif ($program[1] eq "AINT") {
	#
	# Setup the benchmarks directory unless the $NO_SETUP flag is set.
	if ($NO_SETUP == 0) {			
	  system ("$AWB -pm $AINT_CFG -bm $bm_cfg_file setup");
	}

	$benchmark_dir = $BUILD_DIR . "/$AINT_EXEC/bm" . "/$benchmark";
	if (! chdir ($benchmark_dir)) {
	  print "ERROR: cannot cd to $benchmark_dir\n";
	  print "SKIPPING benchmark $benchmark\n\n";
	  next;
	}
      }
      elsif ($program[1] eq "MT_AINT") {
	#
	# Setup the benchmarks directory unless the $NO_SETUP flag is set.
	if ($NO_SETUP == 0) {			
	  system ("$AWB -pm $MT_AINT_CFG -bm $bm_cfg_file setup");
	}

	$benchmark_dir = $BUILD_DIR . "/$MT_AINT_EXEC/bm" . "/$benchmark";
	if (! chdir ($benchmark_dir)) {
	  print "ERROR: cannot cd to $benchmark_dir\n";
	  print "SKIPPING benchmark $benchmark\n\n";
	  next;
	}
      }
      elsif ($program[1] eq "CMP_AINT") {
	#
	# Setup the benchmarks directory unless the $NO_SETUP flag is set.
	if ($NO_SETUP == 0) {			
	  system ("$AWB -pm $CMP_AINT_CFG -bm $bm_cfg_file setup");
	}

	$benchmark_dir = $BUILD_DIR . "/$CMP_AINT_EXEC/bm" . "/$benchmark";
	if (! chdir ($benchmark_dir)) {
	  print "ERROR: cannot cd to $benchmark_dir\n";
	  print "SKIPPING benchmark $benchmark\n\n";
	  next;
	}
      }
      elsif ($program[1] eq "ATOM") {
	#
	# CURRENTLY NOT SETTING UP ATOM BENCHMARKS DIRECTORY. 
	#
	$benchmark_dir = $BUILD_DIR . "/$ATOM_EXEC/bm" . "/$benchmark";
	if (! chdir ($benchmark_dir)) {
	  print "ERROR cannot cd to $benchmark_dir\n";
	  print "SKIPPING benchmark $benchmark\n\n";
	  next;
	}
      }
      else {
	print "Undecipherable benchmark: $benchmark\n";
	print "Proper naming convention is: \n";
	print "\t\t<A|B|C|F|M|R|S|BESPEC|RM|MICRO|>-<ATF | AINT | MT_AINT | ATOM>-<benchmark name>\n\n";
	next;
      }

      if ($program[0] eq "S") {
	# Here we're running the sanity check part of the program. 
	# In this case, we don't have to clean up the environment 
	# since we're running the same program twice and checking
	# the results. 
	# Fede patching
	if ( ($NQS_REJECT == 0) || (&ReSubmitBenchmark($benchmark))) {
	if (&RunSanity) {
	  # Something went wrong with the sanity check. 
	  print "ERROR: Sanity check failed for $benchmark\n";
	  print "ERROR: Check in $benchmark_dir for results\n";
	  next;
        }
	else {
	  $num_exec++;
	}
    }
	}
      elsif ($program[0] eq "R" || $program[0] eq "RM") {
        # We're running the random benchmarks. 
	    if ( ($NQS_REJECT == 0) || (&ReSubmitBenchmark($benchmark))) {
        if (&RunRandom($program[0])) {
	  # Something went wrong with the random check. 
	  print "ERROR: Random check failed for $benchmark\n";
          print "ERROR: Unable to run $benchmark\n";
	  next;
        }
		}
	else {
	  $num_exec++;
	}
      }
      else {
	# Run the level A|B|C|F|M|BESPEC regression test part of the program. 
	print ("RUNNING BENCHMARK: $benchmark\n");
    if ( ($NQS_REJECT == 0) || (&ReSubmitBenchmark($benchmark)))  {
	if ( &SubmitBenchmark($benchmark) ) {
	  # Remove existing stats file. 
	  system ("rm $benchmark.stats >& /dev/null");
	  # Modify the run script so that it dumps IPC information every few cycles. 
	  $new_run_file = $benchmark_dir . "/run.regtest";
	  
	  open (NEW_RUN_FILE, "> $new_run_file") ||
	    die "ERROR: Cannot open file ./run.regtest in $benchmark_dir. $!";
	  
	  open (RUN_FILE, "./run") ||
	    die "ERROR: Cannot open ./run file in $benchmark_dir. $!";
	  
	  while (<RUN_FILE>) {
	    if ($_ =~ /genFlags\=.*/) {
	      if ( $PULSE == 0 ) {
		print NEW_RUN_FILE "genFlags=\"-cf awbcmds -awb $TRACE_FLAGS\"\n";
	      }
	      else {
		print NEW_RUN_FILE "genFlags=\"-cf awbcmds -awb -pc $PULSE $TRACE_FLAGS\"\n";
	      }
	    }
	    elsif ( ($_ =~ /\$model/) && ($VERIFY == 1) && ($program[1] eq "AINT" && $program[0] ne "M") ) {
	      # Parse string.
	      # check to make sure we're not already running a multi-threaded run.
	      if ($_ =~ /[\s\S]+-p[\s\S]+-p/) {
		# Already a multithreaded run, so can't run verify.  
		print ("WARNING: Can't run $program[2] with verifier because it is already MT.\n");
		print NEW_RUN_FILE $_;
	      }
	      else {
		/(\$model\s+\$genFlags\s+--feeder\s+feeder)\s+(-p\s+\"[\s\S]+\")\s+([\s\S]+)/;
		print NEW_RUN_FILE "$1 -V $2 $2 $3\n";
	      }
	    }
	    elsif ( ($_ =~ /\$model/) && ($ORACLE == 1) && ($program[1] eq "AINT" && $program[0] ne "M") ) {
	      # Parse string.
	      /(\$model\s+\$genFlags\s+--feeder\s+feeder)\s+(-p\s+\"[\s\S]+\")\s+([\s\S]+)/;
	      print NEW_RUN_FILE "$1 -O $2 $2 $3\n";
	    }
	    else {		
	      print NEW_RUN_FILE $_;
	    }
	  }
	  
	  #
	  # Make it executable.
	  if (system ("chmod +x $new_run_file")) {
	    print "ERROR: Unable to make run.regtest an executable.\n";
	    return (1);
	  }
	  close NEW_RUN_FILE;
	  close RUN_FILE;
	  
	  #
	  # If we don't want to run under LSF (and want to run on machine, then submit program"
	  chomp ($gold_dir = `$RESOLVER $REG_DIR/gold`);
	  if ($? != 0) {
	    die "ERROR:\n$gold_dir\n";
	  }
	  chomp ($script = `$RESOLVER $REGLEVEL_SCRIPT`);
	  if ($? != 0) {
	    die "ERROR:\n$script\n";
	  }
	  if ($NO_LSF == 1) {
	    &TouchSetupFiles("$benchmark");;
	    $bsub_res=`$script $benchmark_dir $gold_dir $RESULT_DIR`;
	  }
	  else {
            $bsub_res=QueueSubmit($QUEUE, "$RESULT_DIR/ADMIN/$LSF_LOG", $FILE_SIZE_LIMIT, "$benchmark ","$script $benchmark_dir $gold_dir $RESULT_DIR");
	    #	    $bsub_res=`bsub -q $QUEUE -o $RESULT_DIR/ADMIN/$LSF_LOG -F $FILE_SIZE_LIMIT -J $benchmark $script $benchmark_dir $gold_dir $RESULT_DIR`;
	  }
	  if ($? ne 0) {
	    print "ERROR: Unable to submit $benchmark. \n";
	    next;
	  }
	  else {
	    $num_exec++;
	  }
	  &TouchSetupFiles("$benchmark");;
	}
      }
    }
	}
    else {
      print ("\nSKIPPING BENCHMARK: $benchmark\n");
      print ("Not requested level or feeder.\n\n");
    }
  }
  else {
    print "ERROR: Undecipherable benchmark: $benchmark\n";
    print "Proper naming convention is: <A|B|C|M|R|S>-<ATF | AINT>-<benchmark name>\n\n";
  }
}
#
# Check runs and send mail when we are done. 
#
if ( $NO_LSF == 0 ) {
    if ( $num_exec > 0 ) {
	if ( $NO_WATCH == 0 ) {
	    &CheckRuns;
	}
    }
    else {
	print "**************************************\n";
	print "ERROR: NO REGRESSION TESTS WERE RUN\n";
	print "**************************************\n";
	exit (1);
    }
}

############################################################################
############################################################################

######

sub QueueSubmit {
 my $queue = $_[0];
 my $outputfile = $_[1];
 my $filesizelimit = $_[2];
 my $jobname = $_[3];
 my $script = $_[4];

 if ($QUEUE_SYS eq "lsf") {
    if (defined $QUEUE_RESOURCE && $QUEUE_RESOURCE ne "") {
        $res_flag = "-R $QUEUE_RESOURCE";
    } else {
        $res_flag = "";
    }
    $bsub_res=`bsub -q $queue $res_flag -o $outputfile -F $filesizelimit -J $jobname $script`;
 }
 elsif ($QUEUE_SYS eq "nqs") {

    if (!open (SCRIPT_FILE, ">./regnqs$nqs_script_num")) {
      print "ERROR: Unable to open file ./regnqs$nqs_script_num\n";
      $error = 1;
    }
    print SCRIPT_FILE "#!/bin/sh \n";
    print SCRIPT_FILE "$script\n";
    close SCRIPT_FILE;
    $bsub_res=`sleep 5; qsub -d -eo -q $queue -o $outputfile -lF $filesizelimit -r $jobname ./regnqs$nqs_script_num`;
    $nqs_script_num++;
 }
 else {
  die "Unknown queueing system"
 }
 return $bsub_res
}

sub TouchSetupFiles {
  
  my $file_name = $_[0];
  my $error = 0;
  
  if (! open (PENDING_FILE, ">$RESULT_DIR/PENDING/$file_name")) {
    print "ERROR: Unable to open file $RESULT_DIR/PENDING/$file_name\n";
    print "ERROR: This might produce incorrect results for mailing program.\n";
    $error = 1;
  }
  else {
    printf PENDING_FILE "$bsub_res";
  }
  
  if (! open (SUBMITTED_FILE, ">$RESULT_DIR/SUBMITTED/$file_name")) {
    print "ERROR: Unable to open file $RESULT_DIR/SUBMIT$file_name\n";
    print "ERROR: This might produce incorrect results for mailing program.\n";
    $error = 1;
  }
  else {
    printf SUBMITTED_FILE "$bsub_res";
  }
  close PENDING_FILE;
  close SUBMITTED_FILE;

  return ($error);
}

#
# PURPOSE: To setup the results directory for LSF result dumps. 
#
# Each benchmark in the regression suite runs under LSF, and there is no
# way for the perl script to keep track of which jobs have completed
# and which have not.  Therefore, each LSF job records its status
# into a directory which is checked for completion or error.  
#
# Directory structure: All results are located in ASIM_ROOT/regtest/results
# Name of directory: <date>_<pid>  For example, 19990527_4576
# Subdirectories: SUBMITTED, PENDING, ERROR, DONE
#                 SUBMITTED contains files which have been submitted.
#                 PENDING contains the files which are still outstanding
#                 ERROR contains results for tests which failed. 
#                 DONE contains results for tests which completed and passed.
# Each subdirectory contains files whose name is a concatenation of the 
# type of regression run and the name of the benchmark. Furthermore, there
# might be additional data referring to specifics about a test.  
# EXAMPLE: Level A regression test for AINT using compress95 completed 
#          cleanly.  In this case, there will be a file called 
#          A-AINT-compress95 in SUBMITTED, and DONE directories. 
# EXAMPLE: Level S regression test for AINT using compress95 completed
#          cleanly when starting at 10000 instructions, and failed 
#          when starting at 100000 instructions.  In this case, there 
#          will be files S-AINT-compress95.10000 and 
#          S-AINT-compress95.100000 in SUBMITTED, S-AINT-compress95.10000
#          in DONE, and S-AINT-compress95.100000 in ERROR.  Furthermore,
#          the file in S-AINT-compress95.100000 will contain information
#          on the type of error.  

sub SetupRegression {
  if (! defined $EXP_NAME || $EXP_NAME eq "") {
    $today=`date +%Y%m%d`;
    chomp($today);
    $EXP_NAME = "${today}_$$";
  }
  $RESULT_DIR = "$BUILD_DIR" . "/regtest/$EXP_NAME";

  if (! chdir ($BUILD_DIR)) {
    print "ERROR: Unable to change to build directory $BUILD_DIR.\n";
    return (1);
  }
  if (! -e "regtest") {
    if (system ("mkdir regtest")) {
      print "ERROR: Unable to create directory regtest in $BUILD_DIR.\n";
      return (1);
    }
  }
    
  #
  # The user can specify the same results directory as a previous run. 
  # The program now checks the various directories for runs before rerunning
  # jobs if the directory exists.   
  # 
  if ( ! -d $RESULT_DIR ) {
    if (system ("mkdir $RESULT_DIR")) {
      print "ERROR: Unable to create directory $result_dir in $BUILD_DIR/regtest.\n";
      return (1);
    }
  }
  else {
      print "WARNING: Directory $BUILD_DIR/regtest/$RESULT_DIR already exists.\n\n";
  }  

  if ( ! -d "$RESULT_DIR/SUBMITTED" ) {
    if (system ("mkdir $RESULT_DIR/SUBMITTED")) {
      print "ERROR: Unable to create directory $result_dir/SUBMITTED.\n";
      return (1);
    }
  }

  if ( ! -d "$RESULT_DIR/PENDING" ) {
    if (system ("mkdir $RESULT_DIR/PENDING")) {
      print "ERROR: Unable to create directory $result_dir/PENDING.\n";
      return (1);
    }
  }

  if ( ! -d "$RESULT_DIR/DONE" ) {
    if (system ("mkdir $RESULT_DIR/DONE")) {
      print "ERROR: Unable to create directory $result_dir/DONE.\n";
      return (1);
    }
  }

  if ( ! -d "$RESULT_DIR/ERROR" ) {
    if (system ("mkdir $RESULT_DIR/ERROR")) {
      print "ERROR: Unable to create directory $result_dir/ERROR.\n";
      return (1);
    }
  }

  if ( ! -d "$RESULT_DIR/STATS" ) {
    if (system ("mkdir $RESULT_DIR/STATS")) {
      print "ERROR: Unable to create directory $result_dir/STATS.\n";
      return (1);
    }
  }

  if ( ! -d "$RESULT_DIR/ADMIN" ) {
    if (system ("mkdir $RESULT_DIR/ADMIN")) {
      print "ERROR: Unable to create directory $result_dir/ADMIN.\n";
      return (1);
    }
  }
  return (0);
}


# --------------------------------------------------------------------
# Federico Ardanaz notes: 
# In BSSAD we have a problem: sometimes NQS get lost and reject
# jobs so we want to be able to rerun regtest under the same
# directory and rerun only the "Pending" jobs
# --------------------------------------------------------------------
sub ReSubmitBenchmark {
  my $benchmark = $_[0];
  
  # well, if this is a sanity run then the "Pending" file is something like
  # $benchmark.somethingelse => we check any of them:
  
  if ($benchmark =~ /^S.+/)
  {
	opendir(DIR,"$RESULT_DIR/PENDING/");
	@pendingFiles = readdir(DIR);
	closedir(DIR);

	$FND = 0;

	for (@pendingFiles)
	{
		if ( eval "/^$benchmark/") 
		{
			$FND = 1;
		}
	}

	if ( $FND  )
	{
    	print ("WARNING: I'll rerun SANITY benchmark $benchmark 'cause some of the runs didn't finish\n");
		return (1);
	}
  }

  # Similar stuff for random guys (we dosn't repeat random guys)
  
  if ($benchmark =~ /^R.+/)
  {	return (0);  }

  
  if ( -e "$RESULT_DIR/PENDING/$benchmark") {
    print ("WARNING: I'll rerun benchmark $benchmark because it shows up in $RESULT_DIR/PENDING.\n");
    return (1);
  }
  else
  {
    print ("WARNING: benchmark $benchmark is been skipped...\n");
	return (0);
  }
}



#
# This function copies all packages files referenced by the perf.
# model configuration files into the regression results directory.  
# This is so that other scripts (such as asimcommit) can check to 
# make sure that the packages being committed match the packages
# specified in the *.pack file for the performance model.  The 
# packages are checked when the regression directory is specified 
# to asimcommit. 
#
sub CopyPackages {
  my ($config_array_ptr, $result_dir) = @_;
  my $entry = "";
  my $filename = "";

  foreach $entry (@$config_array_ptr) {
    #
    # Step 1: Determine name of file only, since all 
    # packages files will be called <pm config name>.pack
    #
    ($filename = $entry) =~ s/\S*\/(\S*)\.\S*/$1/;

    $filename = $result_dir . "/$filename.pack";

    #
    # Step 2: Determine name of packages file from 
    # configuration files. 
    #
    my ($system, $pkg_name, $pkg_fl);
    
    chomp ($system=`$GETCONFIG $entry Model system`);
    if ($? != 0) {
      print "ERROR: Unable to find system in Model in file $entry.\n";
      return (1);
    }
    
    chomp ($pkg_name=`$GETCONFIG $entry "$system/Requires" packages`);
    if ($? != 0) {
      print "ERROR: Unable to find packages in $system/Requires in file $entry.\n";
      return (1);
    }

    chomp ($pkg_fl=`$GETCONFIG $entry "$pkg_name" File`);
    if ($? != 0) {
      print "ERROR: Unable to find File in $pkg_name in file $entry.\n";
      return (1);
    }

    chomp ($pkg_fl = `$RESOLVER $pkg_fl`);

    if (system ("cp $pkg_fl $filename")) {
      print "ERROR: Unable to copy configuration file ${entry} to ${filename}.\n";
      return (1);
    }
  }
  return 0;
}

#
# Result directory might exist.  Therefore, run checks on PENDING 
# and DONE to determine which benchmarks need to be rerun.  The alg 
# is as follows:
# For given benchmark....
#   if (benchmark is in DONE directory), 
#     Inform user that benchmark is already done.
#     Verify that benchmark is in STATS directory. 
#     Continue with next benchmark. 
#   else if (benchmark is in PENDING directory), 
#     Inform user to remove from PENDING directory 
#     Continue with next benchmark.  
#   else if (benchmark is anywhere else (SUBMITTED, ERROR or no dirs))
#     Inform user that you're running benchmark. 
#     Remove from ERROR directory if it is there. 
#     Verify that benchmark is NOT in STATS directory. 
#     Remove benchmark from SUBMITTED directory, if there. 
#     Submit benchmark. 
#     Continue with next benchmark
#
sub SubmitBenchmark {
  my $benchmark = $_[0];
  my $level = "";

  #
  # Has benchmark already been run? 
  #
  if ( -e "$RESULT_DIR/DONE/$benchmark") {
    print ("WARNING: Not running $benchmark because it already exists in directory $RESULT_DIR/DONE.\n");
    ($level = $benchmark) =~ s/-.*//;
    if ($level !~ m/^(S|R|RM)/) {
      #
      # Verify we have a stats file
      #
      if (! -e "$RESULT_DIR/STATS/$benchmark.stats.gz") {
	print ("ERROR (POTENTIALLY): DONE/$benchmark exists, but STATS/$benchmark.stats.gz does not.\n");
      }
    }
    return (0);
  }
  elsif (( -e "$RESULT_DIR/PENDING/$benchmark") && ($NQS_REJECT == 0) ) {
    print ("ERROR: Cannot run benchmark $benchmark because it shows up in $RESULT_DIR/PENDING.\n");
    print ("FIX: Verify benchmark is still not running and remove file from PENDING directory.\n");
    return (0);
  }
  else {
    if ( -e "$RESULT_DIR/ERROR/$benchmark" ) {
      if (system ("rm $RESULT_DIR/ERROR/$benchmark")) {
	print ("ERROR: Can't remove file $RESULT_DIR/ERROR/$benchmark.\n");
	print ("WARNING: Not running $benchmark.\n");
	return (0);
      }
    }
    if ( -e "$RESULT_DIR/STATS/$benchmark.stats.gz" || -e "$RESULT_DIR/STATS/$benchmark.stats" ) {
      print ("ERROR: Found stats file $benchmark.stats.gz, but benchmark is not in DONE directory.\n");
      print ("WARNING: Not running $benchmark.\n");
      return (0);
    }
    if ( -e "$RESULT_DIR/SUBMITTED/$benchmark" ) {
      if (system ("rm $RESULT_DIR/SUBMITTED/$benchmark")) {
	print ("ERROR: Unable to remove file $RESULT_DIR/SUBMITTED/$benchmark.\n");
	print ("WARNING: Not running $benchmark. \n");
	return (0);
      }
    }
  }
  return (1);
}
  
#
# The sanity check code runs the same exact program twice and makes
# sure that it returns the same results.  Each pair of runs is run
# under bsub.  If there is a problem, it will return an error, and the
# check program will indicate the location of the error to the user. 
# Otherwise, it will return a clean signal and the log files will be 
# cleaned up. 
#
sub RunSanity {
  # Set initialization point. 
  my $start_point = $sparams[0];
  my $end_point = $sparams[4];
  my $warmup = $sparams[1];
  my $duration = $sparams[2];
  my $skip = $sparams[3];
  my $benchmark_name = "";

  print ("**********************************************************************\n");
  print ("SANITY CHECK: benchmark $benchmark_dir.\n");
  print ("**********************************************************************\n");

  ($benchmark_name = $benchmark_dir) =~ s#.*/##s;
  if ( &SubmitBenchmark ("$benchmark_name.$start_point") ) {
    while ($start_point < $end_point) {
      print ("SANITY CHECK: Start: $start_point, Trace Duration: $duration\n");
      # Name of new run script is a function of the starting point. 
      $new_run_script = $benchmark_dir . "/run\." . $start_point;
      
      # Modify the run script to trace for the requested
      # duration of instructions. 
      open (RUN_FILE, "./run") || 
	die "ERROR: Cannot open ./run file in $benchmark_dir. $!";
      open (NEW_RUN_FILE, "> $new_run_script") || 
	die "ERROR: Cannot create $new_run_script file for execution. $!";
      
      # Determine ending cycle. 
      my $end_cycle = $warmup + $duration; 
      
      while (<RUN_FILE>) {
	if ($_ =~ /genFlags\=.*/) {
	  if ( $PULSE == 0 ) {
	    print NEW_RUN_FILE "genFlags=\"-cf awbcmds.$start_point -awb -tsc $warmup -tec $end_cycle\"\n";
	  }
	  else {
	    print NEW_RUN_FILE "genFlags=\"-cf awbcmds.$start_point -awb -pc $PULSE -tsc $warmup -tec $end_cycle\"\n";
	  }
	}
	else {
	  print NEW_RUN_FILE $_;
	}
      }
      close NEW_RUN_FILE;
      
      # create awbcmds file to reflect correct number of skipped instructions. 
      $new_awbcmds = $benchmark_dir . "/awbcmds\." . $start_point;
      open (NEW_AWBCMDS_FILE, ">$new_awbcmds") || 
	die "ERROR: Cannot create awbcmds.$start_point file for execution. $!";
      print NEW_AWBCMDS_FILE <<"END";
{AwbSkip 0 $start_point}
{AwbRun cycle $warmup}
{AwbStats on}
{AwbRun inst $duration}
{AwbStats off}
{AwbStats dump $benchmark.stats}
AwbExit
END
      
      close NEW_AWBCMDS_FILE;

      # Run the benchmark twice. 
      if (system ("chmod +x $new_run_script")) {
        print "ERROR: Unable to make run.$start_point an executable.\n";
	return (1);
      }

      # run a script under bsub which executes the file twice 
      # and makes sure that the results match.  If they do, 
      # then a file called DONE.$start_point is created.  
      # Otherwise, a file called ERROR.$start_point is created. 

      chomp ($script = `$RESOLVER $SANITY_SCRIPT`);
      if ($? != 0) {
        die "ERROR:\n$script\n";
      }
      if ($NO_LSF == 1) {
	# Mark pending file in directory. 
	if (&TouchSetupFiles("$benchmark.$start_point")) {
	  return (1);
	}
	$bsub_res = `$script $benchmark_dir $start_point $RESULT_DIR`;
	if ($? ne 0) {
	  print "ERROR: Unable to submit sanity check. \n";
	  return (1);
	}
      }
      else {
#	$bsub_res=`bsub -q $QUEUE -o $RESULT_DIR/ADMIN/$LSF_LOG -F $FILE_SIZE_LIMIT -J $benchmark\.$start_point $script $benchmark_dir $start_point $RESULT_DIR`;
        $bsub_res=QueueSubmit($QUEUE, "$RESULT_DIR/ADMIN/$LSF_LOG", $FILE_SIZE_LIMIT, "$benchmark\.$start_point ","$script $benchmark_dir $start_point $RESULT_DIR");
	if ($? ne 0) {
	  print "ERROR: Unable to submit sanity check. \n";
	  return (1);
	}
	# Mark pending file in directory. 
	if (&TouchSetupFiles("$benchmark.$start_point")) {
	  return (1);
	}
      }

      # Update counters and try again. 
      $start_point = $start_point + $skip;
    }
    close RUN_FILE;
  }
  return (0);
}

#
# The random check code runs a program using random inputs for the
# number of instructions skipped, the number of cycles warmed up,
# and the number of cycles over which stats are collected. 
# This just checks for functional completion, and does not verify
# against a gold standard. 
#
# The number of random runs defaults to 5. 
#

sub RunRandom {
  local ($random_type) = @_;

  # Set initialization point. 
  my $skip = 0;
  my $submit = 0;

  for ($runs=0; $runs < $RANDOM_NUM_RUNS; $runs++) {
      if ($random_type eq "R") {
	# Choose a number between 0 and 10000000 for skipping. 
	$skip = int(rand(100000000));
	
	print ("**********************************************************************\n");
	print ("RANDOM CHECK: benchmark $benchmark_dir.\n");
	print ("RANDOM CHECK: Skip: $skip, Duration: $RANDOM_NUM_CYCLES\n");
	print ("**********************************************************************\n");
	
	# Check if this benchmark has already run. 
	if ( &SubmitBenchmark ("$benchmark_name.$start_point") ) {
	  # Set flag here so that we know whether we should submit 
	  # this benchmark or not. 
	  $submit = 1;
	  
	  # Name of new run script is a function of the starting point. 
	  $new_run_script = $benchmark_dir . "/run.$skip";
	  
	  open (RUN_FILE, "./run") || 
	    die "ERROR: Cannot open ./run file in $benchmark_dir. $!";
	    
	  # Modify the run script to trace for the requested
	  # duration of instructions. 
	  open (NEW_RUN_FILE, "> $new_run_script") || 
	    die "ERROR: Cannot create $new_run_script file for execution. $!";
	  
	  while (<RUN_FILE>) {
	    if ($_ =~ /genFlags\=.*/) {
	      if ($PULSE == 0) {
		print NEW_RUN_FILE "genFlags=\"-cf awbcmds.$skip -awb $TRACE_FLAGS \"\n";
	      }
	      else {
		print NEW_RUN_FILE "genFlags=\"-cf awbcmds.$skip -awb $TRACE_FLAGS -pc $PULSE\"\n";
	      }
	    }
	    elsif ( ($_ =~ /\$model/) && ($VERIFY == 1) && ($program[1] eq "AINT") ) {
	      # Parse string.
	      if ($_ =~ /[\s\S]+-p[\s\S]+-p/) {
		# Already a multithreaded run, so can't run verify.  
		print ("WARNING: Can't run $program[2] with verifier because it is already MT.\n");
		print NEW_RUN_FILE $_;
	      }
	      else {
		/(\$model\s+\$genFlags\s+--feeder\s+feeder)\s+(-p\s+\"[\s\S]+\")\s+([\s\S]+)/;
		print NEW_RUN_FILE "$1 -V $2 $2 $3\n";
	      }
	    }
	    elsif ( ($_ =~ /\$model/) && ($ORACLE == 1) && ($program[1] eq "AINT") ) {
	      # Parse string.
	      /(\$model\s+\$genFlags\s+--feeder\s+feeder)\s+(-p\s+\"[\s\S]+\")\s+([\s\S]+)/;
	      print NEW_RUN_FILE "$1 -O $2 $2 $3\n";
	    }
	    else {
	      print NEW_RUN_FILE $_;
	    } 
	  }
	  close NEW_RUN_FILE;
	  close RUN_FILE;
	  
	  # create awbcmds file to reflect correct number of skipped instructions. 
	  $new_awbcmds = $benchmark_dir . "/awbcmds.$skip";
	  open (NEW_AWBCMDS_FILE, ">$new_awbcmds") || 
	    die "ERROR: Cannot create $new_awbcmds file for execution. $!";
	  print NEW_AWBCMDS_FILE <<"END";
{AwbSkip 0 $skip}
{AwbRun inst 10000}
{AwbStats on}
{AwbRun cycle $RANDOM_NUM_CYCLES}
{AwbStats off}
{AwbStats dump $benchmark.stats} 
AwbExit
END
	  
	  close NEW_AWBCMDS_FILE;
          if (system ("chmod +x $new_run_script")) {
	    print "ERROR: Unable to make run.$skip an executable.\n";
	    return (1);
	  }
        }   
      }
      elsif ($random_type eq "RM") {
        # Run multi-threaded random tests. All random
        # tests are started in the subdirectory under the current
        # directory.  The nameing convention for the directory is
        # random_<run number>
        $skip = $skip + 1;

	if ( &SubmitBenchmark ("$benchmark_name.$start_point") ) {
	  $submit = 1;
	  $new_dir = $benchmark_dir . "/random_$skip";
	  system ("rm -rf $new_dir > /dev/null 2>&1");
	  if (system ("mkdir $new_dir")) {
	    print "ERORR: Unable to create random MT directory $new_dir.\n";
	    return (1);
	  }
	  # Run script (written by Artur) to mix and match a number of files 
	  # for the random test. 
	  chomp ($script = `$RESOLVER $RANDOM_GEN_SCRIPT`);
	  if ($? != 0) {
	    die "ERROR:\n$script\n";
	  }
	  $random_res=`$script $benchmark_dir $new_dir`;
	  if ($random_res != 0) {
	    print "ERROR: Unable to generate random MT run in directory $new_dir.\n";
	    return (1);
	  }
	}
    }
      
    # run a script under bsub which executes the benchmark. 
    # If it runs correctly, then a file called DONE.$start_point is 
    # created.  Otherwise, a file called ERROR/$benchmark.$skip is 
    # created. 
    if ($submit == 1) {
      if ( $NO_LSF == 1 ) {
	# Mark pending file in directory. 
	if (&TouchSetupFiles("$benchmark.$skip")) {
	  return (1);
	}
        if ($random_type eq "R") {
	  chomp ($script = `$RESOLVER $RANDOM_SCRIPT`);
	  if ($? != 0) {
	    die "ERROR:\n$script\n";
	  }
	  $bsub_res=`$script $benchmark_dir $skip $RESULT_DIR`;
	  if ($? ne 0) {
	    print "ERROR: Unable to submit random regression tests to LSF. \n";
	    return (1);
	  }
	}
        elsif ($random_type eq "RM") {
	  chomp ($script = `$RESOLVER $RANDOM_MT_SCRIPT`);
	  if ($? != 0) {
	    die "ERROR:\n$script\n";
	  }
	  $bsub_res=`$script $benchmark_dir $skip $RESULT_DIR`;
	  if ($? ne 0) {
	    print "ERROR: Unable to submit random regression tests to LSF. \n";
	    return (1);
	  }
        }
      }
      else {
        if ($random_type eq "R") {
	  chomp ($script = `$RESOLVER $RANDOM_SCRIPT`);
	  if ($? != 0) {
	    die "ERROR:\n$script\n";
	  }
	  $bsub_res=QueueSubmit($QUEUE, "$RESULT_DIR/ADMIN/$LSF_LOG", $FILE_SIZE_LIMIT, "$benchmark\.$skip ","$script $benchmark_dir $skip $RESULT_DIR");
	  #	    $bsub_res=`bsub -q $QUEUE -o $RESULT_DIR/ADMIN/$LSF_LOG -F $FILE_SIZE_LIMIT -J $benchmark\.$skip $script $benchmark_dir $skip $RESULT_DIR`;
	  if ($? ne 0) {
	    print "ERROR: Unable to submit random regression tests to LSF. \n";
	    return (1);
	  }
	  # Mark pending file in directory. 
	  if (&TouchSetupFiles("$benchmark.$skip")) {
	    return (1);
	  }
	}
        elsif ($random_type eq "RM") {
	  chomp ($script = `$RESOLVER $RANDOM_MT_SCRIPT`);
	  if ($? != 0) {
	    die "ERROR:\n$script\n";
	  }
	  $bsub_res=QueueSubmit($QUEUE, "$RESULT_DIR/ADMIN/$LSF_LOG", $FILE_SIZE_LIMIT, "$benchmark\.$skip ","$script $benchmark_dir $skip $RESULT_DIR");
	  #	    $bsub_res=`bsub -q $QUEUE -o $RESULT_DIR/ADMIN/$LSF_LOG -F $FILE_SIZE_LIMIT -J $benchmark\.$skip $script $benchmark_dir $skip $RESULT_DIR`;
	  if ($? ne 0) {
	    print "ERROR: Unable to submit random regression tests to LSF. \n";
	    return (1);
	  }
	  # Mark pending file in directory. 
	  if (&TouchSetupFiles("$benchmark.$skip")) {
	    return (1);
	  }
	}
      }
    }
  }
  return (0);
}

#
# This subroutine checks the result directory to see whether all the
# jobs have completed or not.  It does this by checking $RESULT_DIR/PENDING
# directory.  If there are any files left in the directory, then the 
# results have not yet completed.  If the jobs have completed, then a 
# mail is sent to the user indicating that regression test is done.  The
# mail also notes whether all jobs completed cleanly or not.  If the 
# jobs have not completed, then the following happens.
#
# Every N seconds, the directory is checked for completed jobs.  N is 
# set depending on the the level of jobs run.  A mail is sent if all 
# jobs have completed. 
#
# After M seconds, the test times out even if all the jobs are not finished.
# the max timeout can either be set by the user, or a default value is
# set based on the level of the regression test. 

sub CheckRuns {

  my $init_time = 0;
  my $run_time = 0;
  my $max_time = 0;
  my $sample_time = 0;
  my $exit_value = 0;

  # BOBBIE: Use should really use a hash here. 
  #
  $init_time = $INIT_TIME{"$REG_LEVEL"};
  $max_time = $MAX_TIME{"$REG_LEVEL"};

  $sample_time = ($max_time - $init_time)/100;
  if ($sample_time < 60) {
    $sample_time = 60;
  }
  if ($sample_time > 1000) {
    $sample_time = 1000;
  }

  open (MAIL_FILE, ">/tmp/$user_name.mail") ||
    die "Cannot open file /tmp/$user_name.mail for mailing user results of regression. $!\n";

  print "WAITING FOR $init_time seconds before sampling for completion of regression test.\n\n";
  sleep $init_time;

  opendir(PENDING_DIR, "$RESULT_DIR/PENDING") ||
    die "Cannot open directory $RESULT_DIR/PENDING. $!\n";
  while ($num_files = grep !/^\.\.?$/, readdir(PENDING_DIR)) {
    print "JOBS STILL RUNNING:";
    print `date`;
    # there are still runs pending. 
    if ($run_time < $max_time) {
      # We haven't reached the limit yet. 
      $run_time = $run_time + $sample_time;
      sleep $sample_time;
      closedir (PENDING_DIR);
      opendir(PENDING_DIR, "$RESULT_DIR/PENDING") ||
	die "Cannot open directory $RESULT_DIR/PENDING. $!\n";
    }
    else {
      # We've timed out, so send a message stating so. 
      $exit_value = 0;
      print "WARNING: We have time out before regression is completed.\n\n";
      printf MAIL_FILE "******************WARNING: SYSTEM TIME OUT*****************************\n";
      printf MAIL_FILE "Regression check timed out before all tests completed.\n\n";
      printf MAIL_FILE "Directory $RESULT_DIR/PENDING contains tests still pending completion.\n";
      printf MAIL_FILE "Check each file for LSF process id of submitted job. \n\n";
      printf MAIL_FILE "Directory $RESULT_DIR/ERROR contains tests which have completed with errors.\n";
      printf MAIL_FILE "Check each file for type of error. \n\n";
      printf MAIL_FILE "Directory $RESULT_DIR/DONE contains tests which have completed cleanly.\n\n";
      printf MAIL_FILE "Directory $RESULT_DIR/SUBMITTED contains all tests submitted.\n\n";
      printf MAIL_FILE "File $RESULT_DIR/ADMIN/$LSF_LOG contains any outputs from LSF executions. \n\n";
      printf MAIL_FILE "NOTE: Just because the system for checking regressions timed out does not \n";
      printf MAIL_FILE "mean that the regression tests have failed.  If the pending jobs are \n";
      printf MAIL_FILE "running but have not completed, then they might yet complete.  If the \n";
      printf MAIL_FILE "jobs are no longer queued, but they have not completed, then there is \n";
      printf MAIL_FILE "an error. \n";
      printf MAIL_FILE "***********************************************************************\n";
      close MAIL_FILE;
      system ("mail $user_name < /tmp/$user_name.mail");
      system ("rm /tmp/$user_name.mail");
      exit ($exit_value);
    }
  }
  # If we're here, then we have completed within the alloted time
  printf MAIL_FILE "*******************REGRESSION FINISHED******************\n";
  printf MAIL_FILE "Regression test completed. \n";
  printf MAIL_FILE "Directory $RESULT_DIR/ERROR contains tests which have completed with errors.\n";
  printf MAIL_FILE "Check each file for type of error.\n\n ";
  printf MAIL_FILE "Directory $RESULT_DIR/DONE contains tests which have completed cleanly.\n ";
  printf MAIL_FILE "Directory $RESULT_DIR/SUBMITTED contains all tests submitted.\n\n";
  printf MAIL_FILE "File $RESULT_DIR/ADMIN/$LSF_LOG contains any outputs from LSF executions.\n\n ";
  printf MAIL_FILE "***********************************************************************\n";

  # Check to see if there are any files in the ERROR directory. 
  opendir(ERROR_DIR, "$RESULT_DIR/ERROR") ||
    die "Cannot open directory $RESULT_DIR/ERROR. $!\n";
  if ($num_files = grep !/^\.\.?$/, readdir(ERROR_DIR)) {
    printf MAIL_FILE "###########ERROR FILES FOUND#############.\n";
    $exit_value = 0;
  }
  
  close MAIL_FILE;

  system ("mail $user_name < /tmp/$user_name.mail");
  system ("rm /tmp/$user_name.mail");
  exit ($exit_value);
}


sub usage_and_exit {
  print STDERR <<"END";

Program Execution:

regtest.pl [ -level A|B|C|F|M|R|S|RM|BESPEC|MICRO ] [ -aintexec <EXECUTABLE NAME> ] 
   [ -mt_aintexec <EXECUTABLE NAME>  ] 
   [ -cmp_aintexec <EXECUTABLE NAME> ] [ -atfexec <EXECUTABLE NAME> ] 
   [ -benchmarks <Benchmarks file> ]
   [ -feeder <AINT|ATF|ATOM> ]
   [ -sparams <Start Point>:<Warmup>:<Trace Duration>: <Increment>:<End Point> ]
   [ -rparams <Number of runs> ]
   [ -queue <normal|regression> ]
   [ -queueresource <queue resource description> ]
   [ -aintcfg ]
   [ -mt_aintcfg]
   [ -cmp_aintcfg]
   [ -atfcfg ]
   [ -buildonly ]
   [ -nobuild ]
   [ -nosetup ]
   [ -pulse <Number of cycles between printing comments> ]
   [ -spec ]
   [ -nqs ]
   [ -patchnqs ]
   [ -benchmarkset ]
   [ -nolsf ]
   [ -noverify ]
   [ -readme ]
   [ -oracle ]
   [ -watch <Init Time>:<Max Time> ]
   [ -filesize <max_file_size in KBytes> ]
Definitions of level:
    A: Simple tests which complete within less than 5 minutes.  
    B: Tests which complete within 1/2 hour. 
    C: Tests which complete overnight. 
    F: Full run of program. 
    M: SMT runs using multiple threads.
    R: Random tests which skip a random number of instructions and run for
       a fixed number of instructions. 
    RM: Random multi-threaded tests 
    S: Sanity check.  This runs the same program twice and makes sure that 
       the results match exactly. 
    BE: Bruce Edwards test.  This is the equivalent to the runs executed
        to evaluate Arana. 

Default parameters are:
    level = all levels A, B, and C, F, M, R, RM, S and BE which are in benchmarks file
    aintexec = arana_aint_dev 
    atfexec = arana_atf_dev 
    feeder = all feeders (AINT|ATOM|ATF)
    benchmarks = <ASIM_ROOT>/regtest/benchmarks
    sparams = 1000000:10000:200:1000000:20000000
    rparams = 10
    queue = regression (Defaults to regression queue)
    aintcfg = configuration file in source directory
    atfcfg = configuration file in source directory
    buildonly = FALSE 
    nobuild = FALSE (Build and run benchmarks)
    nosetup = FALSE (Run awb setup on benchmarks)
    pulse = 1000 (print a comment every 1000 cycles)
    spec = FALSE (Defaults to running benchmarks under regression directory)
    benchmarkset = <dir>  benchmark config base directory (e.g. Spec95_ev6)
    nolsf = FALSE (runs under LSF instead of locally)
    noverify = 0  (runs under verification unless you specify the -noverify flag)
    readme = 0  (If -readme used, then asks user for README file.  
                 By default, it is not set. )
    oracle = FALSE (does not run instruction oracle)
    traceflags = "" (provide flags for tracing level A, B, C, F, M, or SPECBE programs)
    filesize = 10000 (Size of max file size in KBytes)
    watch = "ALL" => 10000, 20000
	    "A" => 10, 500
	    "B" => 300, 1000
	    "C" => 10000, 20000
	    "M" => 300, 1000
	    "S" => 100, 500
	    "R" => 100, 500
             Watch parameters are a function of the level 
             of the run.  When running all benchmarks, for
             example, the watch parameters are wait for 
             10,000 seconds before beginning sampling, and
             check up to a maximum of 20,000 cycles before
             signalling an error. 
END

  exit(1);
}

sub getinput
{
 my($msg) = $_[0];
 my($val) = $_[1];

 print "$msg [$val] : ";
 $in = <STDIN>;

 if ( $in ne "\n" ) {
  chop $in;
  return $in;
 }

 return $val;
}

sub expand_tilda {
  my($file) = shift;
  my($user, $home);
  
  $home = "";
  if ($file =~ m|^~/|) {
    if (defined $ENV{HOME}) {
      $home = $ENV{HOME};
    } elsif (defined $ENV{USER}) {
      $user = $ENV{USER};
    } elsif (defined $ENV{LOGNAME}) {
      $user = $ENV{LOGNAME};
    } else {
      die( "Who the hell are you?\n");
    }
    if ($home eq "") {
      $home = get_homedir( $user );
    }
    $file =~ s|^~|$home|;
  } elsif ($file =~ m|^~([^/]+)/|) {
    $user = $1;
    $home = get_homedir( $user );
    $file =~ s|^~([^/]+)|$home|;
  }
  return $file;
}
