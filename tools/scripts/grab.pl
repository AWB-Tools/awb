:
eval 'exec perl -w "$0" ${1+"$@"}'
       if 0;

##
## pick out a number of parameters from a collection of result files
##
## $Id: grab.pl 1490 2004-04-02 13:17:02Z jsemer $
##
## (c) 1998, 1999
## Dirk Grunwald <grunwald@cs.colorado.edu> and
## Artur Klauser <klauser@cs.colorado.edu>, 
## University of Colorado at Boulder
##

use Getopt::Long;

# stats files are XML documents - here is a parser for them
use XML::GDOME;
# useful resources for XML and related standards:
#   XML:     http://www.w3.org/XML/
#   XPath:   http://www.w3.org/TR/xpath    <-- used explicitly here
#   XSL:     http://www.w3.org/Style/XSL/

$Getopt::Long::order = $REQUIRE_ORDER;

$MAX_TPUS = 8;

## setup some defaults
my %OPT = (
  "benchmarks"     => "/share/users/klauser/etc/benchmarks.levelA",
  "experiments"    => "",
  "directory"      => ".",
  "filename"       => "",
  "parameters"     => "IPC",
  "xpath"          => "",
  "define"         => "",
  "columnwidth"    => 8,
  "column1width"   => 24,
  "legend"         => 0,
  "threads"        => 0, 
  "sumthreads"     => 0,
  "regression"     => 0,
  "one-minus-x"    => 0,
  "gmean"          => 0,
  "amean"          => 0,
  "meanformat"     => "%.2f",
  "sc"             => 0,
  "verbose"        => 0,
  "help"           => 0
);

## parse command line parameters
if (!GetOptions(\%OPT, 
                "benchmarks=s",
                "experiments=s",
                "directory=s",
                "filename=s",
                "parameters=s",
                "xpath=s",
                "define=s",
                "columnwidth|cw=s",
                "column1width|cw1=s",
                "legend!",
		"threads=s",
		"sumthreads!",
                "regression!",
                "one-minus-x!",
                "gmean!",
                "amean!",
                "meanformat=s",
                "sc!",
                "verbose!",
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

-experiments: directory containing either the *.stats files
  (in case you are looking at regression runs), or location of 
  benchmark runs. If you are specifying regression runs, then you
  must use the -regression flag. 

    For example: 
    # If using -regression flag
    -experiments /work/users/vssadsrc/.awb/run/regtest/<regression dir>
      
    # or
    -experiments /work/users/vssadsrc/build/arana_aint_dev/bm

-directory: This is the path which is prepended to the value specified
  by the -experiments flag.  For example, if you have multiple regression
  tests which you want checked under /work/users/vssadsrc/.awb/run/regtest,
  then you can specify:

    -experiments "030100_4268 030200_5826"  \
    -directory "/work/users/vssadsrc/.awb/run/regtest"

-filename: This overrides benchmarks, experiments, and directory. You
  can specify a stats file name directly and this will be the only
  file that is used to look for the specified parameters.

-parameters: These are the stats which you want retrieved from the
  stats files.  These can be either regular statistics or histogram
  statistics.  The script will figure out which they are.  

    -parameters "L1LoadMisses LdqUtilization"

  In the example, above, L1LoadMisses is a normal stat, and LdqUtilization
  is a histogram stat.  Parameters are separated by spaces. 

-xpath: (experimental) XPath support for XML stats files. See
    http://www.w3.org/TR/xpath
  for a full description of the XPath language.
  XPath allows more control over the specification of which parameters
  to retrieve. Eg.

    -xpath "//scalar[contains(name,'IPC')]"

  would pick out all scalar states with a name that contains the
  string 'IPC'. And

    -xpath "//scalar"

  would retrieve ALL scalar states in the file.

-define: The user can calculate new values based on the value of specific
  parameters.  The define option allows the user to specify the operations
  which are to be performed. 

-threads: This flag allows the user to specify which threads they want
  to look at.  If no thread is specified, then it defaults to 0.  The 
  thread numbers are seperated by spaces.  
  
 Example: 
  To look at data for threads 0 and 2, 
  -threads "0 2"

-sumthreads: This flag tells the program to create a column which adds
  the values for all the threads specified together.

 Example: 
  To look at data for all threads and the sum, 
  -threads "0 1 2 3" -sumthreads

-regression: This flag is used in conjunction with the -experiments flag
  to determine where the script should look for the stats files.  If the 
  -regression flag is specified, the script expects to see *.stats files
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

QUICK EXAMPLE:

     grab.pl -param CPU_0_Overall_IPC -filename foo.stats

     To look for one stat (CPU_0_Overall_IPC) in one file (foo.stats).

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

## read benchmarks
@bench = parse_file_or_params( $OPT{"benchmarks"} );
@bench_hist = parse_file_or_params( $OPT{"benchmarks"} );

## read defines
@define = parse_file_or_params( $OPT{"define"} );

## read threads
@threads = parse_file_or_params( $OPT{"threads"} );

## read params and histogram
@param = parse_file_or_params( $OPT{"parameters"} );

## filename overrides experiments and benchmarks
if ($OPT{"filename"} ne "") {
  @exp = ("unknown_experiment");
  @bench = ("unknown_ benchmark");
}

## create the necessary arrays 
%DATA_PARAM = ();
%DATA_HIST = ();
%HIST_NUM_ROWS = ();
%HIST_NUM_COLS = ();
%HIST_ROW_LABELS = ();
%HIST_COL_LABELS = ();
@HIST_PRINT_TYPES = ();
%stats = (); # basic stats found
@stats = (); # basic stats to print
@hist_stats = ();
&extract_data(@param);
# all basic stats that were found are recoreded in %stats
# all params that are requested are recorded in @param
# we'll now figure out which ones to print and put in @stats
foreach $param (@param) {
  next  if ($param =~ m/^!/); # don't print if we start with ! bang
  if (defined $stats{$param}) {
    push @stats, $param;
  }
}


## make means if requestd
if ($OPT{"gmean"} || $OPT{"amean"}) {
  generate_mean($OPT{"gmean"}, $OPT{"amean"}, $OPT{"meanformat"});
}

if ($#DATA_HIST  != 0) {
  # We have some histogram info
  generate_hist_stats($OPT{"meanformat"});
}

if ( $OPT{"verbose"} ) {
  print "Benchmarks = ", join(',', @bench), "\n";
}

##
## Iterate over the items in the data array.
##
$bench_format = "%-30s";
$exp_format = "%30s";

$bench_format = "%s\t";
$exp_format = "%s\t";

$bench_format = "%-$OPT{column1width}s ";
$exp_format = "%$OPT{columnwidth}s ";

if ($OPT{legend}) {
  # print a numbered legend on top with centered numbers as column headers
  # this allows printing of long and descriptive column names withouth
  # excessive line overflows
  foreach $nexp (0..$#exp) {
    printf "%4s  %s\n", "($nexp)", $exp[$nexp];
  }
  print "\n";
  printf $bench_format, "";
  $gap = ($OPT{columnwidth} + 1 - 4);
  $center = $gap / 2; # this centers column headers

  foreach $nexp (0..$#exp) {
    if ($nexp == 0) {
      printf " " x $center;
    } else {
      printf " " x $gap;
    }
    printf "%4s", "($nexp)";
  }
} else {
  printf $bench_format, "";

  foreach $exp (@exp) {
    printf $exp_format, $exp;
  }
}

print "\n";

##
## Print information about basic stats. 
##
foreach $param (@stats) {
  next  if ($param =~ m/^!/); # don't print if we start with ! bang
  print "-----------------------------------------------------------------------------\n";
  print "$param: ";
  $print_label = "";
  foreach $thread (@threads) {
    if ($thread !~ /sum/) {
      if ($print_label eq "") {
	$print_label = "TPU$thread";
      }
      else {
	$print_label = $print_label . " / TPU$thread";
      }
    }
    else {
      if ($OPT{sumthreads}) {
	$print_label = $print_label . " / SUM";
      }
    }
  }
  print "$print_label\n";
  print "-----------------------------------------------------------------------------\n";
  
  foreach $bench (@bench) {
    if (defined $benchmark_rename{$bench}) {
      $bench_rename = $benchmark_rename{$bench};
    } else {
      $bench_rename = $bench;
      $bench_rename =~ s/^s95-//;
    }
    printf $bench_format, $bench_rename;
    foreach $exp (@exp) {
      local($data) = "-??-";
      $count = 0;
      foreach $thread (@threads) {
	# Only print out thread sum information if requested. 
	if ($thread !~ /sum/ || ($OPT{sumthreads})) {
	  if (! defined $DATA_INCOMPLETE{$exp}{$bench}{$param}) {
	    $data = $DATA_PARAM{$exp}{$bench}{$param}{$thread};
	  }
	  else {
	    $data = $DATA_INCOMPLETE{$exp}{$bench}{$param};
	  }
	  if ($OPT{"one-minus-x"} && $data =~ m/\d+(\.\d*)?/) {
	    $data = sprintf "%6.4f", 1 - $data;
	  }
	  if ($count != 0) {
	    print "/";
	  }
	  printf $exp_format, $data;
	  $count++;
	}
      }
    }
    print "\n";
  }
}

print "-----------------------------------------------------------------------------\n";

##
## Iterate over the items in the data array.
##
if ( $OPT{"verbose"} ){
  foreach $exp (keys %DATA_PARAM) {
    foreach $bench (keys %{ $DATA_PARAM{$exp} }) {
      foreach $param (keys %{ $DATA_PARAM{$exp}{$bench} }) {
	printf "%-10s, %-10s, %-10s = %s\n",
	$exp, $bench, $param, $DATA_PARAM{$exp}{$bench}{$param};
      }
    }
  }
}

##
## Print information about histogram

push (@HIST_PRINT_TYPES, "RAW");
push (@HIST_PRINT_TYPES, "NORM");

foreach $param (@hist_stats) {
    next  if ($param =~ m/^!/); # don't print if we start with ! bang
    # Now we print all histogram data for each benchmark. 
    foreach $exp (@exp) {
      foreach $print_type (@HIST_PRINT_TYPES) {
	print "------------------------------------------------------------------------------------------------------\n";
	print "$exp: $param: $print_type VALUES\n";
	print "------------------------------------------------------------------------------------------------------\n";
	
        #
	# For each param, we first print out benchmark name followed by the column 
	# number if there are multiple columns per benchmark.
	#
	printf "Bin ";
	foreach $bench (@bench_hist) {
	    #
	    # We don't print out values for std_dev and mean. 
	    #
	    if ($bench !~ /AMEAN/ && $bench !~ /STD-DEV/) {
		for ($i = 0; $i < $HIST_NUM_COLS{$param}; $i++) {
		    printf "${bench}:$HIST_COL_LABELS{$param}{$i} ";
		}
	    }
	}
	printf "\n";
	local($data) = "-??-";
	local($num_rows) = $HIST_NUM_ROWS{$param};
	local($num_cols) = $HIST_NUM_COLS{$param};
	##
	## Print some initial info.
	##
	##printf "Total Entries: $DATA_HIST{$exp}{$bench}{$param}{\"entries\"}\n";
	for ($row=0; $row < $num_rows; $row++) {
	    printf "$HIST_ROW_LABELS{$param}{$row} ";
	    foreach $bench (@bench_hist) {
		if ($bench !~ /AMEAN/ && $bench !~ /STD-DEV/) {
		    for ($col = 0; $col < $num_cols; $col++) {
			if ( defined $DATA_HIST{$exp}{$bench}{$param}{$row}{$col}{"$print_type"} ) {
			    $data = $DATA_HIST{$exp}{$bench}{$param}{$row}{$col}{"$print_type"};
			    $data =~ s/(\d*\.\d\d)\d*/$1/;		
			}
			else {
			    $data = "*NA*";
			}
			if ($OPT{"one-minus-x"} && $data =~ m/\d+(\.\d*)?/) {
			    $data = sprintf "%6.4f", 1 - $data;
			}
			printf "$data ";
		    }
		}
	    }
	    print "\n";
	}
      }
      # 
      # Now we print out the mean and std-dev across all benchmarks.  This looks like
      # a 2-dim histograms, but the data is for all benchmarks.  
      #
      local(@stats_type) = ('AMEAN', 'STD-DEV');
      foreach $stat (@stats_type) {
	  print "------------------------------------------------------------------------------------------------------\n";
	  print "$exp: $param: $stat VALUES\n";
	  print "------------------------------------------------------------------------------------------------------\n";
	  printf "Bin ";
	  #
	  # Print column names
	  #
	  for ($col = 0; $col < $HIST_NUM_COLS{$param}; $col++) {
	      printf "$HIST_COL_LABELS{$param}{$col} ";
	  }
	  printf "\n";
	  local($data) = "-??-";
	  local($num_rows) = $HIST_NUM_ROWS{$param};
	  local($num_cols) = $HIST_NUM_COLS{$param};
	  ##
	  ## Print some initial info.
	  ##
	  for ($row=0; $row < $num_rows; $row++) {
	      printf "$HIST_ROW_LABELS{$param}{$row} ";
	      for ($col = 0; $col < $num_cols; $col++) {
		  if ( defined $DATA_HIST{$exp}{"$stat"}{$param}{$row}{$col}{"NORM"} ) {
		      $data = $DATA_HIST{$exp}{"$stat"}{$param}{$row}{$col}{"NORM"};
		      $data =~ s/(\d*\.\d\d)\d*/$1/;		
		  }
		  else {
		      $data = "*NA*";
		  }
		  if ($OPT{"one-minus-x"} && $data =~ m/\d+(\.\d*)?/) {
		      $data = sprintf "%6.4f", 1 - $data;
		  }
		  printf "$data ";
	      }
	      print "\n";
	  }
      }
    }
}

#	print "------------------------------------------------------------------------------------------------------\n";
#	print "$exp: $param: NORMALIZED VALUES\n";
#	print "------------------------------------------------------------------------------------------------------\n";
#	
#	# For each param, we first print out benchmark name 
#	printf "Bin ";
#	foreach $bench (@bench_hist) {
#	    # We don't print out normalized values for std_dev and mean. 
#	    printf "${bench} ";
#	}
#	printf "\n";
#	local($data) = "-??-";
#	local($bin) = $HIST_SIZE{$param};
#	for ($i=0; $i <= $bin; $i++) {
#	    printf "$i ";
#	    foreach $bench (@bench_hist) {
#		if ( defined $DATA_HIST{$exp}{$bench}{$param}{$i}{"NORM"} ) {
#		    $norm = $DATA_HIST{$exp}{$bench}{$param}{$i}{"NORM"};
#		    $norm =~ s/(\d*\.\d\d)\d*/$1/;
#		}
#		else {
#		    $norm = "*NA*";
#		}
#		if ($OPT{"one-minus-x"} && $norm =~ m/\d+(\.\d*)?/) {
#		    $norm = sprintf "%6.4f", 1 - $norm;
#		}
#		printf "$norm ";
#	    }
#	    print "\n";
#	}

print "-----------------------------------------------------------------------------\n";

##
## Iterate over the items in the data array.
##
if ( $OPT{"verbose"} ){
  foreach $exp (keys %DATA_HIST) {
    foreach $bench (keys %{ $DATA_HIST{$exp}}) {
      foreach $param (keys %{ $DATA_HIST{$exp}{$bench} }) {
	printf "%-10s, %-10s, %-10s = %s\n",
	$exp, $bench, $param, $DATA_HIST{$exp}{$bench}{$param};
      }
    }
  }
}


sub get_nodevalue
{
    my $elem = shift;
    my $xpath = shift;

    my @data = $elem->findnodes($xpath);
    if ($#data == 0)
    {
	return $data[0]->getNodeValue();
    }
    else
    {
	# printf STDERR "processing elem= %s\n", $elem;
	# printf STDERR "           xpath= %s\n", $xpath;
	# printf STDERR "       num data = %s\n", $#data;
	my $i;
	for( $i = 0 ; $i <= $#data ; $i++)
	{
	    my $s = $data[$i]->getNodeValue();  
	    # printf STDERR " data[%d] = [%s]\n", $i, $s;
	    # if ($s eq "\n")
	    # {
	    #    printf STDERR " ---Newline\n";
	    # }
	    if ($s ne "" && $s ne "\n")
	    {
		return $s;
	    }
	}
	die "error\n";     
    }
}


sub add_param_data
{
  my($name) = shift;
  my(@value) = @_;

  # Walk through every tpu and stuff appropriate values
  # into the data arrays. If there is no value for this
  # tpu, then we stuff a "NA". 
  my $tpu_num = @value - 1;
  for (my $i = 0; $i < $MAX_TPUS; $i++) {
    if ($i <= $tpu_num) {
      # We have data for this tpu. 
      chomp $value[$i];
      $DATA_PARAM{$exp}{$bench}{$name}{$i} = $value[$i];
    }
    else {
      $DATA_PARAM{$exp}{$bench}{$name}{$i} = "*NA*";
    }
  }

  # Generate the sum for the threads requested. 
  $DATA_PARAM{$exp}{$bench}{$name}{"sum"} = 0;
  foreach $thread (@threads) {
    if ( defined $DATA_PARAM{$exp}{$bench}{$name}{$thread} &&
         $DATA_PARAM{$exp}{$bench}{$name}{$thread} =~ m/\d+(\.\d*)?/) {
      $DATA_PARAM{$exp}{$bench}{$name}{"sum"} += 
          $DATA_PARAM{$exp}{$bench}{$name}{$thread};
    }
  }
}


# FIXME: this is a hack, but it does for now for experimental XPath
# support
sub add_xpath_param
{
  my $elem = shift;

  my $parent = $elem;
  my $path = "";
  # iterate over all ancestors elements (*) that have a 'name' child
  # and build the path to this param (incl. the param name)
  foreach my $data ($elem->findnodes("ancestor-or-self::*/name/text()")) {
    my $value = $data->getNodeValue();
    $path .= "::" . $value;
  }
  $path =~ s|^::||; # strip leading ::

  #print "*** add to \@param $path\n";
  push @param, $path;

  return $path;
}

#
# XML syntax parsed here:
# <scalar>
#   <name>foo</name>
#   123.456
# </scalar>
#
sub add_scalar
{
  my $elem = shift;
  my $isXpath = shift;
  my $path;
   
  my $name = get_nodevalue($elem, "name/text()");
  my $value = get_nodevalue($elem, "text()");
  return  if ($value =~ m/^<error/);
  
  if ($isXpath) {
    $path = add_xpath_param( $elem );
  } else {
    $path = $name;
  }

  add_param_data( $path, $value );
}


#
# XML syntax parsed here:
# <vector>
#   <name>foo</name>
#   <value>123.456</value>
#   <value>234.678</value>
#   ...
# </vector>
#
sub add_vector
{
  my $elem = shift;
  my $isXpath = shift;
  my $path;
  my @value;

  my $name = get_nodevalue($elem, "name/text()");

  # get all values
  foreach my $data ($elem->findnodes("value/text()")) {
    my $value = $data->getNodeValue();
    return  if ($value =~ m/^<error/);
    push @value, $value;
  }

  if ($isXpath) {
    $path = add_xpath_param( $elem );
  } else {
    $path = $name;
  }

  add_param_data( $path, @value );
}


sub extract_data
{
  my(@param) = @_;
  my($hist_flag) = 0;
  my($hist_data_flag) = 0;
  my($hist_name) = 0;
  my($thread) = 0;
  my($row_number) = 0;
  
  # remove no-display (!) annotation from params
  foreach $i (0..$#param) {
    $param[$i] =~ s/^!//;
  }
  # build a search command searching for all params at once
  #$search_param = "(" . join("|",@param) . ")";
  $search_xpath = "name='" . join("' or name='",@param) . "'";
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
      # Remove any spaces around the benchmark
      $bench =~ s/\s*(\S*)\s*/$1/;
      my($filename);
      my($dir) = $OPT{directory};
      if ($exp_exp =~ m|^/|) {
        # is absolute path
        $dir = "";
      }
      if ($OPT{regression}) {
        $filename = "$dir/$exp_exp/$bench.stats";
      } else {
        $filename = "$dir/$exp_exp/bm/$bench/$bench.stats";
      }

      ## filename overrides it all
      if ($OPT{filename}) {
        $filename = $OPT{filename};
      }

      my($status) = "";

      my($doc);  # XML document
      my($root); # XML document root element
      my($xml_data); # XML file in a string (for manual decompression)
      my($decompress_cmd) = ""; # manual decompression command

      if ( -r "$filename.bz2" ) {
        # use bzip2-ed file if available
        $filename .= ".bz2";
        $decompress_cmd = "bzip2 -dc";
      } elsif ( -r "$filename.gz" ) {
        # use gzip-ed file if available
        $filename .= ".gz";
      }

      if ( -r "$filename" ) {
        if ( -z "$filename" ) {
          $status = "*IP*";
        } else {
          if ($decompress_cmd eq "") {
            # gdome handles the file natively
            $doc = XML::GDOME->createDocFromURI("$filename");
            $root = $doc->getDocumentElement();
          } else {
            # we have to manually read the file and point gdome to a
            # memory buffer with the file contents
            $xml_data = `$decompress_cmd $filename`;
            $doc = XML::GDOME->createDocFromString($xml_data);
            $root = $doc->getDocumentElement();
          }
        }
      } else {
        $status = "*NA*";
      }

      my ($found_stat) = 0;
      if ( $status eq "" ) {
        # Note: for some reason, getElementsByName() is painfully slow
        # when we got several thousand nodes that match, so we use the
        # iterator approach instead, which is almost an order of
        # magnitude faster.

        # process all scalar descendents of root
        foreach my $elem ($root->findnodes("//scalar[$search_xpath]")) {
          add_scalar( $elem );
        }

        # process all vector descendents of root
        foreach my $elem ($root->findnodes("//vector[$search_xpath]")) {
          add_vector( $elem );
        }

        # process elements explicitly searched with xpath
        if ($OPT{'xpath'} ne "") {
          foreach my $elem ($root->findnodes($OPT{'xpath'})) {
            my $tagname = $elem->tagName();
            if ($tagname eq "scalar") {
              add_scalar( $elem, 1);
            } elsif ($tagname eq "vector") {
              add_vector( $elem, 1);
            } else {
              die "ERROR: found xpath node for $OPT{xpath}\n".
                  "       and tag '$tagname'\n".
                  "       but don't know what to do with this tag.\n";
            }
          }
        }

        # process all histogram descendents of root
        foreach my $elem ($root->findnodes("//compound[type='histogram' and ($search_xpath)]"))
        {
          my $name = get_nodevalue($elem, "name/text()");
          my $desc = get_nodevalue($elem, "desc/text()");

          grep (/$name/, @hist_stats) || push (@hist_stats, $name);
          $DATA_HIST{$exp}{$bench}{$name}{"desc"}=$desc;

          # get number of Rows
          my $rows = get_nodevalue($elem, "scalar[name='rows']/text()");
          if (defined $HIST_NUM_ROWS{$name}) {
            ($HIST_NUM_ROWS{$name} == $rows) || 
              die "Histogram size varies across benchmarks for $name\n";
          } else {
            $HIST_NUM_ROWS{$name} = $rows;
          }

          # get number of Columns
          my $cols = get_nodevalue($elem, "scalar[name='cols']/text()");
          if (defined $HIST_NUM_COLS{$name}) {
            ($HIST_NUM_COLS{$name} == $cols) || 
              die "Histogram size varies across benchmarks for $name\n";
          } else {
            $HIST_NUM_COLS{$name} = $cols;
          }

          # get total number of entries
          my $entries = get_nodevalue($elem, "scalar[name='entries']/text()");
          $DATA_HIST{$exp}{$bench}{$name}{"entries"} = $entries;

          #
          # get column names
          #
          my @data = $elem->findnodes("vector[name='column names']");
          die "internal parsing error"  if ($#data != 0);
          my $vec = $data[0];
          my @col_names;
          foreach my $data ($vec->findnodes("value/text()")) {
            my $value = $data->getNodeValue();
            push @col_names, $value;
          }
          my $num_cols = @col_names;
          ($HIST_NUM_COLS{$name} == $num_cols) ||
              die "Mismatch in num of cols and num of col labels for $name\n";
          for (my $i = 0; $i < $num_cols; $i++) {
            $HIST_COL_LABELS{$name}{$i} = $col_names[$i];
          }

          #
          # get data
          #
          foreach my $vec ($elem->findnodes("compound[name='data']/vector")) {
            my $rowname = get_nodevalue($vec, "name/text()");
            if (defined $HIST_ROW_LABELS{$name}{$row_number}) {
              ($rowname == $HIST_ROW_LABELS{$name}{$row_number}) ||
                die "Row names for $name do not match across experiments\n";
            } else {
              $HIST_ROW_LABELS{$name}{$row_number} = $rowname;
            }
            my @row_data;
            foreach my $data ($vec->findnodes("value/text()")) {
              my $value = $data->getNodeValue();
              push @row_data, $value;
            }
            for ($col = 0; $col < $HIST_NUM_COLS{$name}; $col++) {
              #
              # 5-dim array: experiment, benchmark, histogram name, row, col
              # 
              $DATA_HIST{$exp}{$bench}{$name}{$row_number}{$col}{"RAW"} = 
                  $row_data[$col];
              $DATA_HIST{$exp}{$bench}{$name}{$col}{"column_total"} += 
                  $row_data[$col];
            }
            $row_number++;
          }
        }

        foreach $define (@define) {
          $defexe = $define;
          # substitute local variables
          $defexe =~ s/\$([a-zA-Z0-9][\w.:_]*)/\$DATA_PARAM\{"$exp"\}\{"$bench"\}\{"${1}"\}\{0\}/g;
          # substitute global variables
          $defexe =~ s/\$\(([a-zA-Z0-9][\w.:_]*)\)/\$${1}/g;
	  $defexe = 0  if $defexe eq "";
          ## eval destroys some variable contents - why?
          ## evaluate definitions
	  eval "$defexe, 1" 
	      or die "define error in $defexe\n".$@;
$,=" ";
        }
      } else {
        foreach $param (@param) {
	  $DATA_INCOMPLETE{$exp}{$bench}{$param} = $status;

          if ($OPT{filename}) {
            add_param_data( $param, $status );
          }
        }
      }
    }
  }
  # figure out which basic stats we have got
  foreach $exp (@exp) {
    foreach $bench (@bench) {
      foreach $param (keys %{$DATA_PARAM{$exp}{$bench}}) {
        $stats{$param} = 1;
      }
    }
  }

  # Indicate that we're also looking at the sum of all threads. 
  push @threads, "sum";
}


sub generate_mean {
  my($gm) = shift;
  my($am) = shift;
  my($format) = shift;
  my($data,$num);
  my($prod,$gmean);
  my($sum,$amean);

  foreach $param (@stats) {
    foreach $exp (@exp) {
      foreach $thread (@threads) {
        $prod = 1;
        $sum = 0;
        $num = 0;
        foreach $bench (@bench) {
	  if (! defined $DATA_INCOMPLETE{$exp}{$bench}{$param}) {
	    $data = $DATA_PARAM{$exp}{$bench}{$param}{$thread};
	    if ($data !~ m/\*..\*/) {
	      $prod *= $data;
	      $sum += $data;
	      $num++;
	    }
	  }
	  if ($num != 0) {
	    $gmean = $prod ** (1/$num);
	    $amean = $sum / $num;
          } else {
            $gmean = 0;
	    $amean = 0;
          }
	  if ($gm) {
	    $DATA_PARAM{$exp}{"gmean"}{$param}{$thread} = sprintf($format, $gmean);
          }
          if ($am) {
	    $DATA_PARAM{$exp}{"amean"}{$param}{$thread} = sprintf($format, $amean);
	  }
        }
      }
    }
  }
  # add this pseudo benchmark
  if ($gm) {
    push @bench, "gmean";
  }
  if ($am) {
    push @bench, "amean";
  }
}


##
## The following information will be printed out for 
## all histograms.  
## 1.) The number of histogram bins.
## 2.) The total number of entries for all bins. 
## 3.) The number of entries per bin, and the bin occupancy as a 
##     percentage of the total occupancy.
## 4.) The average number of entries per bin. 
## 5.) The variance in number of entries per bin. 
##
sub generate_hist_stats {
  my ($sum_sqs) = 0;
  my ($sum) = 0;
  my ($format) = shift;
  my ($amean) = 0;
  my ($num_rows) = 0;
  my ($var) = 0;
  my ($std_dev) = 0;
  my ($num) = 0;
  foreach $exp (@exp) {
    foreach $param (@hist_stats) {
      $num_rows = $HIST_NUM_ROWS{$param};
      $num_cols = $HIST_NUM_COLS{$param};
      ($num_rows != 0) || die "There are no rows defined for param $param.\n";
      ($num_cols != 0) || die "There are no cols defined for param $param.\n";
      #
      # Walk across each row and calculate averages for each benchmark. 
      # Since there can be multiple columns for a histogram for each benchmark, 
      # we calculate mean and std-dev per each col of histogram. 
      #
      for ($col = 0; $col < $num_cols; $col++) {
	$DATA_HIST{$exp}{"AMEAN"}{$param}{$col}{"entries"} = 100;
	$DATA_HIST{$exp}{"STD-DEV"}{$param}{$col}{"entries"} = 100;
	for ($row = 0; $row < $num_rows; $row++) {
	  $sum_sqs = 0;
	  $sum = 0;
	  $num = 0;
	  #
	  # Walking across a row means that we visit each benchmark. 
	  #
	  foreach $bench (@bench_hist) {
	    #
	    # Check to make sure we found this histogram in this benchmark,
	    # and that it had some entries in it. 
	    #
	    if (! defined $DATA_INCOMPLETE{$exp}{$bench}{$param} &&
		$DATA_HIST{$exp}{$bench}{$param}{"entries"} != 0 ) {
	      #
	      # Now check that we have some entries for this column of
	      # histogram. Each column specifically might have no entries.
	      #
	      if ($DATA_HIST{$exp}{$bench}{$param}{$col}{"column_total"} != 0) {
		$data = $DATA_HIST{$exp}{$bench}{$param}{$row}{$col}{"RAW"}/
		  $DATA_HIST{$exp}{$bench}{$param}{$col}{"column_total"};
		( defined $data ) || 
		  die "No data defined for $exp, $bench, $param, row $row.\n";
		( $data <= 1 ) || 
		  die "Value cannot be greater than 100%\n";
		$DATA_HIST{$exp}{$bench}{$param}{$row}{$col}{"NORM"} = $data*100;
		$sum_sqs += ($data ** 2);
		$sum += $data;
	      }
	      $num++;
	    }
	  }
	  if ($num == 0) {
	    $num = 1;
	  }
	  $amean = $sum / $num;
	  $var = ($sum_sqs - (2 * $amean) * $sum + $num * ($amean ** 2)) / $num;
	  $std_dev = $var ** 0.5;
	  ## print "amean: $amean, var: $var, std_dev: $std_dev\n";
	  $DATA_HIST{$exp}{"AMEAN"}{$param}{$row}{$col}{"NORM"} = 
	    sprintf($format, $amean*100);
	  $DATA_HIST{$exp}{"STD-DEV"}{$param}{$row}{$col}{"NORM"} = 
	    sprintf($format, $std_dev*100);
	}
      }
    }
  }
  # add more pseudo benchmarks
  push @bench_hist, "AMEAN";
  push @bench_hist, "STD-DEV";
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
        chop;
        push @param, $_;
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
