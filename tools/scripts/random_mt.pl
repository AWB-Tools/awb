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
#  PURPOSE: Run a test and makes sure it completes. 
#
#############################################################################

# Different types of errors
# 1: No run file
# 2: Cannot execute run file or it does not complete

if ($ARGV[0]) {
    $benchmark_dir = $ARGV[0];
    shift;
    $run_number = $ARGV[0];
    shift;
    $regr_results_dir = $ARGV[0];
    shift;
}
else {
    print "ERROR: Incorrect input.\n";
    exit (1);
}

print "Benchmark_dir = $benchmark_dir, result_dir = $regr_results_dir\n";

# The name of the benchmark is the name of the directory.  Since the
# directory is provided as an absolute path, chop the name from the
# directory path.
($benchmark = $benchmark_dir) =~ s#.*/##s;

$working_dir = $benchmark_dir . "/random_$run_number";

if ($ARGV[0]) {
  # There is an error. 
  print "ERROR: Unknown argument $ARGV[0].\n";
  exit (1);
}

$RM_FILES="${working_dir}/log ${working_dir}/error ${working_dir}/core";

# Set specific environment parameters. 
&SetupEnv;

$error_file = $working_dir . "/error";
if (! open (ERROR_FILE, ">$error_file")) {
  print "ERROR: Unable to run regression test on $working_dir.\n";
  print "ERROR: Unable to open ERROR file $error_file.\n";
  system ("touch $result_dir/ERROR/$benchmark");
}
elsif (! chdir $working_dir) {
  print ERROR_FILE "ERROR: Unable to change directory to ${working_dir}\n";
  close ERROR_FILE;
  system ("mv $error_file $result_dir/ERROR/$benchmark");
}
else {
    # Setup core file to dump to /dev/null
    system ("ln -s /dev/null core");
    if (system ("$ENV{PROGRAM}")) {
	# We have an error. 
	print ERROR_FILE "ERROR: Unable to run regression test on $working_dir.\n";
	print ERROR_FILE "ERROR: did not run to completion.\n"; 
	print ERROR_FILE "ERROR: Random SMT check for $working_dir died during execution.\n";
	print ERROR_FILE "ERROR: awbcnmds file located in $working_dir/awbcmds.\n";
	print ERROR_FILE "ERROR: Run file located in $working_dir/run\n";
	print ERROR_FILE "ERROR: Error file located in $working_dir/error.\n";
	print ERROR_FILE "ERROR: Log file located in $working_dir/log.\n\n";
	&SetupErrorScript;
	&PrintErrorCommands;
	$log_file = $working_dir . "/" . "log";
	if (open (LOG_FILE, $log_file)) {
	    print ERROR_FILE  "**********************************************************************\n";
	    print ERROR_FILE "RESULTS FROM ERROR FILE ${working_dir}/error\n";
	    print ERROR_FILE "**********************************************************************\n";
	    while (<LOG_FILE>) {
		print ERROR_FILE "$_";
	    }
	    close LOG_FILE;
	}
	else {
	    print ERROR_FILE "ERROR: Results are located in $working_dir/log.\n";
	}
	close ERROR_FILE;
	system ("mv $error_file ${regr_results_dir}/ERROR/${benchmark}.${run_number}");
    }
    else {
	# Clean run.  Clean up. 
	# Dump awbcmds and run script into the DONE file so that we can figure out
	# how we ran this MT benchmark. 
	$awbcmds_file = $working_dir . "/awbcmds";
	$run_file = $working_dir . "/run";
	$done_file = $regr_results_dir . "/DONE/$benchmark\.$run_number";
	
	# Copy awbcmds and run files over. 
	&DumpFile($awbcmds_file, $done_file);
	&DumpFile($run_file, $done_file);
	
	system ("rm $RM_FILES");
    }
}
# Clean up file from pending directory. 
system ("rm ${regr_results_dir}/PENDING/${benchmark}.${run_number}");

sub DumpFile {
    local ($from_file, $to_file) = @_;
    open (FROM_FILE, $from_file) || 
	  die "ERROR: Cannot open $from_file file. $!";
    open (TO_FILE, ">> $to_file") || 
	  die "ERROR: Cannot open $to_file file. $!";
    print TO_FILE "**********************************************************************\n";
    print TO_FILE  "DUMP OF $from_file\n";
    print TO_FILE "**********************************************************************\n";
    while (<FROM_FILE>) {
	print TO_FILE "$_";
    }
    close FROM_FILE;
    close TO_FILE;
}
    

sub SetupErrorScript {
    $run_error = $working_dir . "/run\.error";
    open(ERROR_SCRIPT, ">$run_error") || 
	die "ERROR: Unable to open file $run_error.\n\n";
    print( "Dumping environment\n" );
    print ERROR_SCRIPT "#!/bin/sh\n";
    foreach $key (keys %ENV) {
	print ERROR_SCRIPT "$key=\"$ENV{$key}\"\n";
	print ERROR_SCRIPT "export $key\n";
    }
    print ERROR_SCRIPT "$ENV{PROGRAM}\n";
    close ERROR_SCRIPT;
    system ("chmod 777 $run_error");
}

sub PrintErrorCommands {
    print ERROR_FILE "TO REPEAT ERROR, DO THE FOLLOWING:\n";
    print ERROR_FILE "1. Rlogin to machine $ENV{LSB_HOSTS}. \n";
    print ERROR_FILE "2. Change directory to $working_dir.\n";
    print ERROR_FILE "3. Type env -i $run_error\n\n";
}
	
sub SetupEnv {
    PutEnv("SHELL", "/bin/sh");
    PutEnv("PROGRAM", "./run > log 2>&1");
}
    
sub PutEnv {
  local($name, $value) = @_;
  $ENV{$name} = $value;
#  print( "$name = $ENV{$name} \n" );
}
   

