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
# This script runs one regression test for a particular level.
# It is called from LSF.
#
################################################################
################################################################
################################################################
$benchmark_dir = $ARGV[0];
shift;
$gold_dir = $ARGV[0];
shift;
$result_dir = $ARGV[0];
shift;

print "Benchmark_dir = $benchmark_dir, gold_dir = $gold_dir, result_dir = $result_dir\n";

($benchmark = $benchmark_dir) =~ s#.*/##s;
($test_pid = $result_dir) =~ s#.*_##s;

print "$ARGV[0]\n";
if ($ARGV[0]) {
  # There is an error. 
  print "ERROR: Unknown argument $ARGV[0].\n";
  exit (1);
}


# Note the location of the LSF host machine 
# This environment variable gets deleted when setting up the environment. 
$lsf_host = $ENV{LSB_HOSTS};

# Execute the benchmark. 
&NukeEnv;

$error_file = $benchmark_dir . "/error";
if (! open (ERROR_FILE, ">$error_file")) {
  print "ERROR: Unable to run regression test on $benchmark_dir.\n";
  print "ERROR: Unable to open ERROR file $error_file.\n";
  system ("touch $result_dir/ERROR/$benchmark");
}
# cd into benchmark directory. 
elsif (! chdir ($benchmark_dir)) {
  print ERROR_FILE "ERROR: Unable to change directory to ${benchmark_dir}\n";
  close ERROR_FILE;
  system ("mv $error_file $result_dir/ERROR/$benchmark");
}
else {
    # Setup core file to dump to /dev/null
    system ("ln -s /dev/null core");
    if (system ("$ENV{PROGRAM}")) {
	print ERROR_FILE "ERROR: Unable to run regression test on $benchmark_dir.\n\n";
	&SetupErrorScript;
	&PrintErrorCommands;

	$log_file = $benchmark_dir . "/" . "log";
	if (open (LOG_FILE, $log_file)) {
	    print ERROR_FILE "**************************************************************\n";
	    print ERROR_FILE "* RESULTS FROM LOG FILE WHICH IS LOCATED IN $benchmark_dir/log.\n";
	    print ERROR_FILE "**************************************************************\n";
	    while (<LOG_FILE>) {
		print ERROR_FILE "$_";
	    }
	    close LOG_FILE;
	}
	else {
	    print ERROR_FILE "ERROR: Results are located in $benchmark_dir/log.\n";
	}

	close ERROR_FILE;
	system ("mv $error_file $result_dir/ERROR/$benchmark");
    }
    else {
	# Everything ran cleanly so far. 
	# Check that we match the gold standard. 
	# Open the regression test. 
	$new_file = $benchmark_dir . "/" . $benchmark . "\.stats";
	if (! open (NEW_FILE, $new_file)) {
	    print ERROR_FILE "ERROR: cannot open file $new_file\n"; 
	    print ERROR_FILE "NOT CHECKING BENCHMARK: $benchmark\n\n";
	    close ERROR_FILE;
	    system ("mv $error_file $result_dir/ERROR/$benchmark");
	}
	else {
	    # We were able to complete the run.  Therefore, copy the stats file
	    # into the STATS directory before continuing any further. 
	    if (system ("cp $new_file $result_dir/STATS/.")) {
		# error in copying stats file over to the STATS directory. 
		print ERROR_FILE "ERROR: Unable to copy stats file $new_file to $result_dir/STATS directory.\n";
		close ERROR_FILE;
		system ("mv $error_file $result_dir/ERROR/$benchmark");
	    }
	    else {
		#
		# gzip output file.
		#
		if (system ("gzip $result_dir/STATS/$benchmark.stats")) {
		  print ERROR_FILE "ERROR: Unable to ZIP file $result_dir/STATS/$benchmark.stats\n";
		}
		$reg_file_gz = $gold_dir . "/$benchmark.stats.gz";
		# If file doesn't exist or is not readable...
		if (!(-e $reg_file_gz && -r _)) {
		    print ERROR_FILE "ERROR: cannot open file $reg_file_gz\n";
		    print ERROR_FILE "NOT CHECKING AGAINST GOLD STANDARD: $benchmark\n\n";
		    
		    # Cat the new stats file into the error file. 
		    print ERROR_FILE "DUMP OF CURRENT RUN STATISTICS ($new_file).\n\n";
		    print ERROR_FILE "******************************************************************\n";
		    print ERROR_FILE "******************************************************************\n";
		    print ERROR_FILE "* Results from New Execution. NO COMPARISON DONE.\n";
		    print ERROR_FILE "******************************************************************\n";
		    print ERROR_FILE "******************************************************************\n";
                    # (r2r) this creates yet another copy of the stats file,
                    # - uncompressed; takes lots of disk space - bad idea
		    #while (<NEW_FILE>) {
                    #   print ERROR_FILE "$_";
		    #}
		    close ERROR_FILE;
		    system ("mv $error_file $result_dir/ERROR/$benchmark");
		}
		# Open the new execution file. 
		elsif (&CompareResults) {
		    print ERROR_FILE "ERROR: Differences in execution statistics between current run and gold standard.\n";
		    print ERROR_FILE "ERROR: gold statistics file is located in $reg_file_gz.\n";
		    print ERROR_FILE "ERROR: Current run statistics file is located in $new_file.\n\n";
		    print ERROR_FILE "******************************************************************\n";
		    print ERROR_FILE "******************************************************************\n";
		    print ERROR_FILE "* Results from Comparison. \n";
		    print ERROR_FILE "******************************************************************\n";
		    print ERROR_FILE "******************************************************************\n";
		    # Cat the results from the comparison to the error file. 
                    while (<DIFF_FILE>) {
                        print ERROR_FILE "$_";
                    }
                    close DIFF_FILE;
		    
		    close ERROR_FILE;
		    system ("mv $error_file $result_dir/ERROR/$benchmark");
		}
		else {
		    # No errors, so place in DONE directory. 
		    system ("touch $result_dir/DONE/$benchmark");
		}
	    }
	}
	close NEW_FILE;
    }
}

# Remove file from PENDING directory. 
if (system ("rm $result_dir/PENDING/$benchmark")) {
  print "ERROR: Unable to delete file $result_dir/PENDING/$benchmark.\n";
}

system ("rm core");


#############################################################################
#SUBROUTINES
#############################################################################

sub CompareResults {
  my($dok);

  $diff_file = "/tmp/diff_" . "$$";
  print ("COMPARING BENCHMARK: $benchmark\n");
  print ("Output located in $diff_file\n\n");
  $dok = system ("gzip -c -d $reg_file_gz | diff -  $new_file > $diff_file");

  #
  # Let's make sure the difference is not just in the date fields
  # of the two files
  #
  # roger.
  #
  if ( $dok ) {
    open(DIFF_FILE,$diff_file) || die "ERROR: Unable to open file containing comparison results.\n\n";
    
    #
    # Filter out the date difference. The diff file should look as follows:
    # 2c2
    # < Wed Aug 11 16:39:31 1999
    # ---
    # > Fri Aug  6 16:56:48 1999
    #
    # Thus, we read 4 lines checking that they match this pattern. Then, if the diff file is
    # empty, this is not really an ERROR.
    
    # First line. If match fails, immediately report error
    $_ = <DIFF_FILE>;
    return 1 if ( ! /^\d+c\d+/ );
    
    # Second line: a date format
    $_ = <DIFF_FILE>;
    return 2 if ( ! /^<\s+\w+\s+\w+\s+\d+\s+\d+:\d+:\d+\s+\d+\s*$/ );
    
    # Third line: three dashes
    $_ = <DIFF_FILE>;
    return 3 if ( ! /^---\s*$/ );
    
    # Fourth line: another date format
    $_ = <DIFF_FILE>;
    return 4 if ( ! /^>\s+\w+\s+\w+\s+\d+\s+\d+:\d+:\d+\s+\d+\s*$/ );
    
    # At this point the file should be empty
    while ( <DIFF_FILE> ) {
      return 5; # if we get here, the file is not empty --> error!
    }
    
    seek (DIFF_FILE, 0, 0); # reposition to beginning of file
    # remove the directory entry - we'll keep the open file handle around
    # so we can still read the file with it
    unlink $diff_file;
    return 0;
  }
  else {
    print ERROR_FILE "ERROR: Unable to unzip and compare $reg_file_gz";
    return 1;
  }
    

  return $dok;
#  &CompareGlobalStats;
#  &CompareCboxStats;
#  &CompareMboxStats;
#  &CompareEboxStats;
#  &CompareQboxStats; 
#  &ComparePboxStats;
#  &CompareIboxStats;
}

sub NukeEnv {
  print( "Nuking environment\n" );
  foreach $key (keys %ENV) {
#    print( "key = $key\n" );
#    if ($key ne "USER") {
      delete $ENV{$key};
#    }
  }

  PutEnv("PATH", ".:/usr/local/bin:/usr/bin:/bin:".
	 "/usr/local/b.alpha/gcc");
  PutEnv("USER", $ENV{"USER"} eq "" ? "vssadsrc" : $ENV{"USER"});
  PutEnv("SHELL", "/bin/sh");
  PutEnv("HOME", "/tmp");
  PutEnv("LOGDIR", "/tmp");
  PutEnv("TTY", "/dev/tty");
  PutEnv("TERM", "vt100");
  PutEnv("PWD", ".");
  PutEnv("PROGRAM", "./run.regtest > log 2>&1");
}

sub PrintErrorCommands {
    print ERROR_FILE "TO REPEAT ERROR, DO THE FOLLOWING:\n";
    print ERROR_FILE "1. Rlogin to machine $lsf_host. \n";
    print ERROR_FILE "2. Change directory to $benchmark_dir.\n";
    print ERROR_FILE "3. Type env -i $run_error\n\n";
}

sub SetupErrorScript {
  $run_error = $benchmark_dir . "/run\.error\.$test_pid";
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

sub PutEnv {
  local($name, $value) = @_;
  $ENV{$name} = $value;
#  print( "$name = $ENV{$name} \n" );
}


