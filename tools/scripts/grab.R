:
eval 'exec perl -w "$0" ${1+"$@"}'
       if 0;

##
## pick out a number of parameters from a collection of result files
##
## $Id: grab.R 1490 2004-04-02 13:17:02Z jsemer $
##
## (c) 1998, 1999
## Dirk Grunwald <grunwald@cs.colorado.edu> and
## Artur Klauser <klauser@cs.colorado.edu>, 
## University of Colorado at Boulder
##

use Getopt::Long;

$Getopt::Long::order = $REQUIRE_ORDER;

$MAX_TPUS = 4;

## setup some defaults
my %OPT = (
#  "benchmarks"     => "/share/users/klauser/etc/benchmarks.levelA",
  "benchmarks"     => "/usr/users/etune/",
  "experiments"    => ".",
  "directory"      => ".",
  "parameters"     => "IPC",
  "threads"        => 0, 
  "regression"     => 0,
  "help"           => 0
);

## parse command line parameters
if (!GetOptions(\%OPT, 
                "benchmarks=s",
                "experiments=s",
                "directory=s",
                "parameters=s",
								"threads=s",
                "regression!",
                "help!") ||
    $OPT{help}) {

    usage_and_exit();
}

## print usage message and exit
sub usage_and_exit {
  printf "Parameters and current settings:\n";
  foreach $par (sort keys %OPT) {
    printf "  %-20s %s\n", $par ." " . "." x (20-length($par)), $OPT{$par};
  }
  printf "\n";
  printf "Definitions of parameters:\n";
  printf <<"END";
-benchmarks: File containing list of benchmarks from which to collect data

 Example:
  If the file has the following benchmarks:

  C-AINT-applu95
  C-AINT-ijpeg95

  then the script will scan for info for these two benchmarks. 

-experiments: directory containing either the STATS directory 
  (in case you are looking at regression runs), or location of 
  benchmark runs. If you are specifying regression runs, then you
  must use the -regression flag. 

    For example: 
    # If using -regression flag
    -experiments /work/users/vssadsrc/.awb/run/regtest/030100_4268
      
    # or
    -experiments /work/users/vssadsrc/build/arana_aint_dev/bm

-directory: This is the path which is prepended to the value specified
  by the -experiments flag.  For example, if you have multiple regression
  tests which you want checked under /work/users/vssadsrc/.awb/run/regtest,
  then you can specify:

    -experiments "030100_4268 030200_5826"  \
    -directory "/work/users/vssadsrc/.awb/run/regtest"

-param: These are the stats which you want retrieved from the
  stats files.  These can be either regular statistics or histogram
  statistics.  The script will figure out which they are.  

    -param "L1LoadMisses LdqUtilization"

  In the example, above, L1LoadMisses is a normal stat, and LdqUtilization
  is a histogram stat.  Parameters are separated by spaces. 

-define: The user can calculate new values based on the value of specific
  parameters.  The define option allows the user to specify the operations
  which are to be performed. 

-threads: This flag allows the user to specify which threads they want
  to look at.  If no thread is specified, then it defaults to 0.  The 
  thread numbers are seperated by spaces.  
  
 Example: 
  To look at data for threads 0 and 2, 
  -threads "0 2"

 Example: 
  To look at data for all threads and the sum, 
  -threads "0 1 2 3" 

-regression: This flag is used in conjunction with the -experiments flag
  to determine where the script should look for the stats files.  If the 
  -regression flag is specified, the script expects to see a STATS directory
  under the \$directory/\$experiments directory. 

-one-minux-x: Specify this flag if you want the data to be reversed. 

-gmean, -amean: These flags are used by the script to print out geometric 
  or arithmetic mean values for the general stats.  Note that this has 
  no effect on histogram stats.  Histogram statistics only generate amean
  values.  

EXAMPLE: 

     grab.pl -regression -param "StqResource LdqResource" \
       -benchmarks "C-fp" \
	 -experiments "~vssadsrc/.awb/run/regtest/030500_16899/" 
         -threads "0 1"

     The invocation above will look in directory ~vssadsrc/.awb/run/regtest
     /030500_16899 for benchmarks specified in file C-fp.  The parameters
     extracted from the file are StqResource and LdqResource, and the 
     directory specified is for a regression test.  Furthermore, produce
     results only for thread 0 and thread 1.  
END
    
  exit 1;
}


## commonly used routines in definition files
## safe divide
sub div
{
  my ($a) = shift;
  my ($b) = shift;
  my ($ret);

  return (0) if (!defined($a) || !defined($b));

  if ($b != 0) {
    $ret = $a / $b;
  } else {
    $ret = 0;
  }
  return ($ret);
}

## safe percent
sub percent
{
  my ($a,$b) = @_;
  return (0) if (!defined($a) || !defined($b));
  return div(100*$a, $b);
}

## prettier and shorter names for benchmarks
%benchmark_rename = (
  "s95-gcc"            => "gcc",
  "s95-compress-small" => "compress",
  "s95-go-2stone9"     => "go",
  "s95-perl-small"     => "perl",
  "s95-perl-small-jumble"=>"perl-j",
  "s95-m88ksim-test"   => "m88ksim",
  "s95-ijpeg-small"    => "jpeg",
  "s95-vortex-small"   => "vortex",
  "s95-li-train"       => "lisp"
);

## read experiments
@exp = parse_file_or_params( $OPT{"experiments"} );
foreach $exp (@exp) {
		-d $exp ||die ("Unable to read directory $exp\n")
}

## read benchmarks
@bench = parse_file_or_params( $OPT{"benchmarks"} );
@bench_hist = parse_file_or_params( $OPT{"benchmarks"} );

## read threads
@threads = parse_file_or_params( $OPT{"threads"} );

## read params and histogram
@param = parse_file_or_params( $OPT{"parameters"} );

## create the necessary arrays 
%DATA_PARAM = ();
%DATA_HIST = ();
%HIST_NUM_ROWS = ();
%HIST_NUM_COLS = ();
%HIST_ROW_LABELS = ();
%HIST_COL_LABELS = ();
@stats = ();
@hist_stats = ();
&extract_data(@param);


# make means if requestd
if ($OPT{"gmean"} || $OPT{"amean"}) {
  generate_mean($OPT{"gmean"}, $OPT{"amean"}, $OPT{"meanformat"});
}

##
## Iterate over the items in the data array.
##
$bench_format = "%-30s";
$exp_format = "%30s";

$bench_format = "%s\t";
$exp_format = "%s\t";


##
## Print information about basic stats. 
##
foreach $exp (@exp) {
  foreach $param (@stats) {
    next  if ($param =~ m/^!/); # don't print if we start with ! bang
    open (FILE, ">$param.R") ||
    die "Cannot open file $param.\n";
    printf "$param: ";
    
    # We need to print out the number of rows in each benchmark
    $num_rows = @threads + 1;
    printf FILE "$num_rows\n";
    
    foreach $bench (@bench) {
      if (defined $benchmark_rename{$bench}) {
        $bench_rename = $benchmark_rename{$bench};
      } else {
        $bench_rename = $bench;
        $bench_rename =~ s/^s95-//;
      }
      if (! defined $DATA_INCOMPLETE{$exp}{$bench}{$param}) {
        printf FILE "$bench_rename\n";
        printf FILE "$param\n";
        local($data) = "-??-";
        foreach $thread (@threads) {
          # Only print out thread sum information if requested. 
          $data = $DATA_PARAM{$exp}{$bench}{$param}{$thread};
          printf FILE "TPU$thread ";
          printf FILE $exp_format, "$data\n";
        }
      }
    }
  }
  close(FILE)
}

##
## Print out only raw data
foreach $exp (@exp) {
  foreach $param (@hist_stats) {
    next  if ($param =~ m/^!/); # don't print if we start with ! bang
    # Now we print all histogram data for each benchmark. 
    #
    # For each param, we first print out column information.
    #
    open(FILE, ">$param.R") || 
    die "Cannot open file $param.R\n";
    
    # Print the number of rows per benchmark
    $num_rows = $HIST_NUM_ROWS{$param} + 1;
    printf FILE "$num_rows\n";
    foreach $bench (@bench_hist) {
      if (defined ($DATA_HIST{$exp}{$bench}{$param})
          && $DATA_HIST{$exp}{$bench}{$param}{"entries"} != 0) {
        printf FILE "$bench\n";
        for ($i = 0; $i < $HIST_NUM_COLS{$param}; $i++) {
          printf FILE "$HIST_COL_LABELS{$param}{$i} ";
        }
        
        printf FILE "\n";
        
        local($data) = "-??-";
        local($num_rows) = $HIST_NUM_ROWS{$param};
        local($num_cols) = $HIST_NUM_COLS{$param};
        ##
        ## Print some initial info.
        ##
        for ($row=0; $row < $num_rows; $row++) {
          printf FILE "$HIST_ROW_LABELS{$param}{$row} ";
          for ($col = 0; $col < $num_cols; $col++) {
            # If this parameter is defined and if the number
            # of entries is !0
            if (defined $DATA_HIST{$exp}{$bench}{$param}{$row}{$col}{"RAW"}) {
              $data = $DATA_HIST{$exp}{$bench}{$param}{$row}{$col}{"RAW"};
              printf FILE "$data ";
            }
            else {
              printf "Unable to find certain rows or columns\n";
              exit 1;
            }
          }
          printf FILE "\n";
        }
      }
    }
    close(FILE);
  }
}

sub extract_data() {
  my(@param) = @_;
  my($hist_flag) = 0;
  my($hist_data_flag) = 0;
  my($hist_name) = 0;
  my ($thread) = 0;
  my ($row_number) = 0;
  
  # remove no-display (!) annotation from params
  foreach $i (0..$#param) {
    $param[$i] =~ s/^!//;
  }
  # build a search command searching for all params at once
  $search_param = "(" . join("|",@param) . ")";
  
  # Remove all spaces in search string
  $search_param =~ s/ //g;
  foreach $exp (@exp) {
    $exp_exp = expand_tilda($exp);
    ##
    ## Look for each file in turn
    ##
    foreach $bench (@bench) {
      ##
      ## Check for compressed file
      ##
      #my($filename) = "$OPT{directory}/$exp_exp/Out/$bench";
      my($filename);
      my($dir) = $OPT{directory};
      (-d $dir) ||
      die ("Unable to find directory $dir\n");
      if ($exp_exp =~ m|^/|) {
        # is absolute path
        $dir = "";
      }
      if ($OPT{regression}) {
        $filename = "$dir/$exp_exp/STATS/$bench.stats";
      } else {
        $filename = "$dir/$exp_exp/bm/$bench/$bench.stats";
      }
      my($status) = "";
      
      if ( -r "$filename.gz" ) {
        if ( -z "$filename.gz" ) {
          $status = "*IP*";
        } else {
          open(FILE, "gunzip -c $filename.gz |") || 
          die "Can not uncompress and open $filename.gz \n";
        }
      } elsif ( -r "$filename.bz2" ) {
        if ( -z "$filename.bz2" ) {
          $status = "*IP*";
        } else {
          open(FILE, "bunzip2 -c $filename.bz2 |") || 
          die "Can not uncompress and open $filename.bz2 \n";
        }
      } elsif ( -r "$filename" ) {
        if ( -z "$filename" ) {
          $status = "*IP*";
        } else {
          open(FILE, "<$filename") || die "Can not open $filename\n";
        }
      } else {
        print "Unable to find file $filename\n";
        $status = "*NA*";
      }
      
      my ($found_stat) = 0;
      if ( $status eq "" ) {
        foreach (<FILE>) {
          $found_stat = 0;
          if (/^$search_param\s*:\s*([\S\s]*)/) {
            if ($2 !~ m/^<error/) {
              $found_stat = 1;
              grep (/^$1$/, @stats) ||
              push (@stats, $1);
              @tpu = split (":", $2);
              $tpu_num = @tpu - 1;
              
              # Walk through every tpu and stuff appropriate values
              # into the data arrays. If there is no value for this
              # tpu, then we stuff a "NA". 
              for ($i = 0; $i < $MAX_TPUS; $i++) {
                if ($i <= $tpu_num) {
                  # We have data for this tpu. 
                  chomp $tpu[$i];
                  $DATA_PARAM{$exp}{$bench}{$1}{$i} = $tpu[$i];
                }
                else {
                  $DATA_PARAM{$exp}{$bench}{$1}{$i} = "*NA*";
                }
              }
              # Generate the sum for the threads requested. 
              my ($param_local) = $1;
              $DATA_PARAM{$exp}{$bench}{$param_local}{"sum"} = 0;
              foreach $thread (@threads) {
                if ( defined $DATA_PARAM{$exp}{$bench}{$param_local}{$thread} &&
                     $DATA_PARAM{$exp}{$bench}{$param_local}{$thread} =~ m/\d+(\.\d*)?/) {
                  $DATA_PARAM{$exp}{$bench}{$param_local}{"sum"} += 
                  $DATA_PARAM{$exp}{$bench}{$param_local}{$thread};
                }
              }
            }
          }
          if (/^BEGIN\s*(RESOURCE|HISTOGRAM)\s*:\s*$search_param\s*:\s*([\w\s]*)/) {
            ($found_stat == 0) || 
            die "ILLEGAL STATS FILE: Parameter $2 is registered as a normal stat and as a histogram.\n";
            # We found a histogram or resource
            $hist_flag = 1;
            chomp($2);
            $hist_name = $2;
            grep (/^$hist_name$/, @hist_stats) ||
            push (@hist_stats, $hist_name);
            
            $DATA_HIST{$exp}{$bench}{$hist_name}{"desc"}=$3;
          }
          elsif ($hist_flag == 1) {
            chomp;
            #
            # Check to see if we've reached the end of the histogram. 
            # Note that sometimes there is no data in histogram.  If 
            # the total number of entries is 0, asim doesn't print out
            # any data.  In this case, we never get hist_data_flag == 1. 
            #
            if (/^END\s*(RESOURCE|HISTOGRAM)\s*:\s*([\s\S]*)/) {
              chomp ($1);
              chomp ($2);
              ($hist_flag == 1) || die "Incorrect parsing of histogram information.\n";
              ($hist_name eq $2) || die "Incorrect Parsing of histogram $hist_name, bench=$bench, exp=$exp\n";
              $hist_flag = 0;
              $hist_data_flag = 0;
              $row_number = 0;
            }
            #
            # Read data from histogram
            #
            elsif ($hist_data_flag == 1) {
              @row_data = split (":", $_);
              if (defined $HIST_ROW_NAMES{$hist_name}{$row_number}) {
                ($row_data[0] == $HIST_ROW_NAMES{$hist_name}{$row_number}) ||
                die "Row names for $hist_name do not match across experiments\n";
              }
              else {
                $HIST_ROW_LABELS{$hist_name}{$row_number} = $row_data[0];
                for ($col = 0; $col < $HIST_NUM_COLS{$hist_name}; $col++) {
                  #
                  # 5-dim array: experiment, benchmark, histogram name, row, col
                  # 
                  $DATA_HIST{$exp}{$bench}{$hist_name}{$row_number}{$col}{"RAW"} = 
                  $row_data[$col+1];
                  $DATA_HIST{$exp}{$bench}{$hist_name}{$col}{"column_total"} += 
                  $row_data[$col+1];
                }
              }
              $row_number++;
            }
            #
            # We've reached the data segment of histogram. 
            #
            elsif (/^BEGIN\s+DATA/) {
              $hist_data_flag = 1;
            }
            #
            # Read column names
            #
            elsif (/^COLUMN\s+NAMES:\s*([\S\s]+)/) {
              chomp ($1);
              @col_names = split (":", $1);
              $num_cols = @col_names;
              #
              # Check histogram to make sure col labels match number of cols
              #
              ($HIST_NUM_COLS{$hist_name} == $num_cols) ||
              die "Mismatch in num of cols and num of col labels for $hist_name\n";
              for ($i = 0; $i < $num_cols; $i++) {
                $HIST_COL_LABELS{$hist_name}{$i} = $col_names[$i];
              }
            }
            #
            # Read number of Rows
            #
            elsif (/^Number\s+of\s+Rows:\s*(\d*)/) {
              if (defined $HIST_NUM_ROWS{$hist_name}) {
                ($HIST_NUM_ROWS{$hist_name} == $1) || 
                die "Histogram size varies across benchmarks for $hist_name\n";
              }
              else {
                $HIST_NUM_ROWS{$hist_name} = $1;
              }
            }
            #
            # Read number of Columns
            #
            elsif (/^Number\s+of\s+Cols:\s*(\d*)/) {
              if (defined $HIST_NUM_COLS{$hist_name}) {
                ($HIST_NUM_COLS{$hist_name} == $1) || 
                die "Histogram size varies across benchmarks for $hist_name\n";
              }
              else {
                $HIST_NUM_COLS{$hist_name} = $1;
              }
            }
            #
            # Read Total number of entries in histogram
            #
            elsif (/^Total\s+Entries in Histogram:\s*(\d*)/) {
              $DATA_HIST{$exp}{$bench}{$hist_name}{"entries"}=$1;
            }
          }
        }
        
        foreach $define (@define) {
          $defexe = $define;
          # substitute local variables
          $defexe =~ s/\$([a-zA-Z0-9][\w.:_]*)/\$DATA_PARAM\{"$exp"\}\{"$bench"\}\{"${1}"\}/g;
          # substitute global variables
          $defexe =~ s/\$\(([a-zA-Z0-9][\w.:_]*)\)/\$${1}/g;
          $defexe = 0  if $defexe eq "";
          ## eval destroys some variable contents - why?
          ## evaluate definitions
          eval "$defexe, 1" 
          or die "define error in $defexe\n".$@;

        }
        close(FILE);
      }
      else {
        foreach $param (@param) {
          $DATA_INCOMPLETE{$exp}{$bench}{$param} = $status;
        }
      }
    }
  }
}


##
## try to get parameters from a string in either of two ways:
##  (1) the string can be a file name; in this case read the
##      parameters from this file
##  (2) the string is a space separated list of parameters; in that
##      case just stuff them into an array
##
sub parse_file_or_params {
  my($string) = shift;
  my(@param);
  
  $string = expand_tilda($string);
  if (! -d $string && open RDPARAM, "<$string") {
    while (<RDPARAM>) {
      if (!m/^#/) {
	  /\s*([\s\S]+)\s*\n/;
	  push @param, $1;
	}
    }
    close RDPARAM;
  } else {
    @param = split ' ', $string;
  }
  
  return @param;
}

##
## expand ~/ and ~user/ filenames to absolute paths
##
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

##
## get home directory for a user
##
sub get_homedir {
  my($user) = shift;
  my($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$dir,$shell,$expire);
  
  ($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$dir,$shell,$expire) =
    getpwnam( $user )  or die("Can't find user $user\n") ;
  
  if (!defined $name) {
    die("Can't find user $user\n");
  }
  return $dir;
}
