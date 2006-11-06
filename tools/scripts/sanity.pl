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


#############################################################################
#
#  PURPOSE: Run the same test twice.  If the tests complete and the results
#  are the same, then touch file DONE.$1.  Otherwise, touch file ERROR.$1
#
#############################################################################

# Different types of errors
# 1: No run file
# 2: Cannot execute run file or it does not complete
# 3: Diff did not match. 

if ($ARGV[0]) {
    $benchmark_dir = $ARGV[0];
    shift;
    $start_cycle = $ARGV[0];
    shift;
    $regr_results_dir = $ARGV[0];
    shift;
}
else {
    print "ERROR: Incorrect input.\n";
    exit (1);
}

print "Benchmark_dir = $benchmark_dir, start_cycle = $start_cycle, result_dir = $regr_results_dir\n";

# The name of the benchmark is the name of the directory.  Since the
# directory is provided as an absolute path, chop the name from the
# directory path.
($benchmark = $benchmark_dir) =~ s#.*/##s;

$RM_FILES="$benchmark_dir/log1.$start_cycle $benchmark_dir/log2.$start_cycle $benchmark_dir/error.$start_cycle $benchmark_dir/run.$start_cycle $benchmark_dir/awbcmds.$start_cycle $benchmark_dir/core";

# Set specific environment parameters. 
&SetupEnv;

$error_file = $benchmark_dir . "/error";

#
# Setup a flag which indicates by default that this was NOT a clean run. 
# This needs to be cleared for those cases where the run completed
# correctly. 
#

$clean_run = 0;
if (! open (ERROR_FILE, ">$error_file")) {
  print "ERROR: Unable to run regression test on $benchmark_dir.\n";
  print "ERROR: Unable to open ERROR file $error_file.\n";
  system ("touch $result_dir/ERROR/$benchmark");
}
elsif (! chdir $benchmark_dir) {
  print ERROR_FILE "ERROR: Unable to change directory to ${benchmark_dir}\n";
  close ERROR_FILE;
  system ("cp $error_file $result_dir/ERROR/$benchmark");
}
else {
    # Setup core file to dump to /dev/null
    system ("ln -s /dev/null core");
    
    if ( ! system ("$ENV{PROGRAM1}")) {
	if (! system ("$ENV{PROGRAM2}")) {
	    if (chomp ($num_lines = `diff log1.${start_cycle} log2.${start_cycle} | egrep -v \"Started:|Finished:|Elapsed|CPU|Space|Processors\" | wc -l`) le 4) {
		# Clean run.  Clean up. 
		system ("touch ${regr_results_dir}/DONE/${benchmark}.${start_cycle}");
		system ("rm $RM_FILES");
		$clean_run = 1;
	    }
	    else {
		&SetupErrorScript;
		&PrintErrorCommands;
		print ERROR_FILE "ERROR: Differences in execution between two runs\n";
		print ERROR_FILE "ERROR: awbcmds file located in $benchmark_dir/awbcmds.$start_cycle\n";
		print ERROR_FILE "ERROR: Run file located in $benchmark_dir/run.$start_cycle\n";
		print ERROR_FILE "ERROR: Log files located in log1.$start_cycle and log2.$start_cycle.\n";
	    }
	}
	else {
	    &SetupErrorScript;
	    &PrintErrorCommands;
	    print ERROR_FILE "ERROR: run.${start_cycle} did not run to completion on second execution\n";
	    print ERROR_FILE "ERROR: Sanity check for $benchmark_dir died during second execution.\n";
	    print ERROR_FILE "ERROR: awbcmds file located in $benchmark_dir/awbcmds.$start_cycle.\n";
	    print ERROR_FILE "ERROR: Run file located in $benchmark_dir/run.$start_cycle.\n";
	    print ERROR_FILE "ERROR: Error file located in $benchmark_dir/error.$start_cycle.\n";
	    print ERROR_FILE "ERROR: Log file located in $benchmark_dir/log2.$start_cycle.\n";
	    print ERROR_FILE "**********************************************************************\n";
	    print ERROR_FILE "RESULTS FROM ERROR FILE ${benchmark_dir}/error.${start_cycle}\n";
	    print ERROR_FILE "**********************************************************************\n";
	    $log_file = $benchmark_dir . "/" . "log";
	    if (open (LOG_FILE, $log_file)) {
		print ERROR_FILE  "**********************************************************************\n";
		print ERROR_FILE "RESULTS FROM ERROR FILE ${benchmark_dir}/error.${start_cycle}\n";
		print ERROR_FILE "**********************************************************************\n";
		while (<LOG_FILE>) {
		    print ERROR_FILE "$_";
		}
		close LOG_FILE;
	    }
	    else {
		print ERROR_FILE "ERROR: Results are located in $benchmark_dir/log.\n";
	    }
	}
    }
    else {
	&SetupErrorScript;
	&PrintErrorCommands;
	print ERROR_FILE "ERROR: run.${start_cycle} did not run to completion on first execution\n";
	print ERROR_FILE "ERROR: Sanity check for $benchmark_dir died during first execution.\n";
	print ERROR_FILE "ERROR: awbcmds file located in $benchmark_dir/awbcmds.$start_cycle.\n";
	print ERROR_FILE "ERROR: Run file located in $benchmark_dir/run.$start_cycle.\n";
	print ERROR_FILE "ERROR: Error file located in $benchmark_dir/error.$start_cycle.\n";
	print ERROR_FILE "ERROR: Log file located in $benchmark_dir/log1.$start_cycle.\n";
	print ERROR_FILE "**********************************************************************\n";
	print ERROR_FILE "RESULTS FROM ERROR FILE ${benchmark_dir}/error.${start_cycle}\n";
	print ERROR_FILE "**********************************************************************\n";
	$log_file = $benchmark_dir . "/" . "log";
	if (open (LOG_FILE, $log_file)) {
	    print ERROR_FILE  "**********************************************************************\n";
	    print ERROR_FILE "RESULTS FROM ERROR FILE ${benchmark_dir}/error.${start_cycle}\n";
	    print ERROR_FILE "**********************************************************************\n";
	    while (<LOG_FILE>) {
		print ERROR_FILE "$_";
	    }
	    close LOG_FILE;
	}
	else {
	    print ERROR_FILE "ERROR: Results are located in $benchmark_dir/log.\n";
	}
    }
    close ERROR_FILE;
    if (! $clean_run) {
        system ("cp $error_file ${regr_results_dir}/ERROR/${benchmark}.${start_cycle}");
    }
}

# Clean up file from pending directory. 
system ("rm ${regr_results_dir}/PENDING/${benchmark}.${start_cycle}");
	
sub SetupErrorScript {
  $run_error = $benchmark_dir . "/run\.error" . "\.${start_cycle}";
  open(ERROR_SCRIPT, ">$run_error") || 
    die "ERROR: Unable to open file $run_error.\n\n";
  print( "Dumping environment\n" );
  print ERROR_SCRIPT "#!/bin/sh\n";
  foreach $key (keys %ENV) {
    print ERROR_SCRIPT "$key=\"$ENV{$key}\"\n";
    print ERROR_SCRIPT "export $key\n";
  }
  print ERROR_SCRIPT "$ENV{PROGRAM1}\n";
  close ERROR_SCRIPT;
  system ("chmod 777 $run_error");
}

sub PrintErrorCommands {
    print ERROR_FILE "TO REPEAT ERROR, DO THE FOLLOWING:\n";
    print ERROR_FILE "1. Rlogin to machine $ENV{LSB_HOSTS}. \n";
    print ERROR_FILE "2. Change directory to $benchmark_dir.\n";
    print ERROR_FILE "3. Type env -i $run_error\n\n";
}

sub SetupEnv {
    PutEnv("SHELL", "/bin/sh");
    PutEnv("PROGRAM1", "./run.$start_cycle > log1.$start_cycle 2>&1");
    PutEnv("PROGRAM2", "./run.$start_cycle > log2.$start_cycle 2>&1");
}
    
sub PutEnv {
  local($name, $value) = @_;
  $ENV{$name} = $value;
#  print( "$name = $ENV{$name} \n" );
}

    
    
