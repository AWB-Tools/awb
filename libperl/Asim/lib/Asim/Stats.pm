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

package Asim::Stats;
use warnings;
use strict;
use Class::Struct;
use File::Basename;
use Asim::Util;

# stats files are XML documents - here is a parser for them
use XML::GDOME;
# useful resources for XML and related standards:
#   XML:     http://www.w3.org/XML/
#   XPath:   http://www.w3.org/TR/xpath    <-- used explicitly here
#   XSL:     http://www.w3.org/Style/XSL/

struct stats_info => {
  basedir      =>        '$',   # Base location of all experiments
  experiments  =>        '@',   # List of experiments
  benchmarks   =>        '@',   # List of benchmarks 
  params       =>        '@',   # List of params
  runtype      =>        '$',   # Batch run or local run stats files
};

#
# These are the set of special functions handled by the program. 
#
our %data_functions = (
    NORMALIZE       =>    {exp => "",
                           bm  => "",
                           par => "",
                           norm_na => "",
                          },
    PDF             =>    {par => "",
                          },
    CDF             =>    {par => "",
                          },
    SUM_ROWS        =>    {par => "",
                          },
    SUM_COLS        =>    {par => "",
                          },
    SUM             =>    {par => "",
                          },
    EXTRACT         =>    {par => "",
			   row => "",
			   col => "",
		          },
    COMBINE         =>    {par => "",
                          },
    REDUCE          =>    {op => "",
                           par =>"",
                          },
    COLLATE_ROWS    =>    {par => "",
                           start => "",
                           end => "",
                          },
    COLLATE_COLS    =>    {par => "",
                           start => "",
                           end => "",
                          },
);

our $debug = 0;

=head1 NAME

Asim::Stats - Library for extracting data from stats files. 

=head1 SYNOPSIS

use Asim::Stats

TBD

=head1 DESCRIPTION

This module provides an object for extracting data from stats files

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $stats = Asim::Stats -E<gt>new()

Create a new stats data object. 

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};
  bless $self, $class;

  $self->_initialize();
  bless	$self, $class;
  return $self;
}

sub _initialize {
  my $self = shift;
  my $info = stats_info->new();

  # Add additional information to self. 
  # The data hash stores the data structure. 
  $self->{'info'} = $info;
  $self->{'data'} = {};
  $self->{'paraminfo'} = {};
  $self->{'functions'} = \%data_functions;
  $self->{'dimensions'} = {};
  $self->{'restricted'} = {};

  # Initialize the base data members
  if (defined $ENV{AWBLOCAL}) {
    my $base = $ENV{AWBLOCAL} . "/build/regtest/fu-runs";
    $info->basedir($base);
  }
  else {
    $info->basedir("./");
  }
  @{$info->experiments} = ("/.*/");
  @{$info->benchmarks} = ("/.*/");
  @{$info->params} = ("/CPU_.*_Overall_IPC");
  $info->runtype("");
  $self->_test_xml_libs();

  return $self;
}

sub _test_xml_libs {
  my $self = shift;

  system("test-xpath-function --regexp -q");
  if ($? == 0) {
    print "*** HAVE xpath regexp()\n"  if ($debug);
    $self->{'xpath_regexp'} = 1;
  } else {
    print "*** MISSING xpath regexp()\n"  if ($debug);
    $self->{'xpath_regexp'} = 0;
  }
}

sub _print_data {
  my $self = shift;
  my $type = shift;  
  my $output = shift;
  my $dim;

  if ($type eq "all") {
    $dim = $self->{dimensions};
  }
  else {
    $dim = $self->{restricted};
  }
  my $pinfo = $self->{paraminfo};
  my $data_param = $self->{data};

  (scalar(keys %{$dim}) != 0) || 
    die "ERROR: No data to print yet. Run 'get data' first.\n";

  my @exps = @{$dim->{experiments}};
  my @bms = @{$dim->{benchmarks}};
  my @params = @{$dim->{params}};
  
  my $exp;
  my $bm;
  my $par;

  # Determine output
  if (! defined ($output)) {
    $output = "-";
  }
  else {
    $output = Asim::Util::expand_tilda($output);
  }
  
  if (! open (OUTPUT, ">$output")) {
    warn "Unable to open output file $output.\n"; 
    return 1;
  }
    
  # Determine the max width of any benchmark. 
  my $maxlen = 0;
  
  foreach $bm (@bms) {
    if ($maxlen < length($bm)) {
      $maxlen = length($bm);
    }
  }
  # Pad the length
  $maxlen = $maxlen + 8;
  
  my $bench_format = "%-${maxlen}s ";
   my $exp_format = "%8s ";
  

  foreach $par (@params) {
    next  if ($par =~ m/^!/); # don't print if we start with ! bang
    if (defined $pinfo->{$par}{type}) {
      if ($pinfo->{$par}{'type'} eq "scalar") {
        
        print OUTPUT "-----------------------------------------------------------------------------\n";
        print OUTPUT "$par: ";
        foreach $exp (@exps) {
          print OUTPUT "$exp\t";
        }
        print OUTPUT "\n";
        print OUTPUT "-----------------------------------------------------------------------------\n";
        foreach $bm (@bms) {
          printf OUTPUT $bench_format, $bm;
          foreach $exp (@exps) {
            if (defined ($$data_param{$exp}{$bm}{$par}{0}{0}) ) {
#              my $data = sprintf $format, $$data_param{$exp}{$bm}{$par}{0}{0};
              printf OUTPUT $exp_format, $self->_pretty_data($$data_param{$exp}{$bm}{$par}{0}{0});
            }
          }        
          print OUTPUT "\n";
        }
      }
      #
      # We have vector or histogram parameter.
      #
      else {
        my $row;
        my $col;
        my $ncols = $pinfo->{$par}{ncols};
        my $nrows = $pinfo->{$par}{nrows};
        my @cnames = @{$pinfo->{$par}{clabels}};
        my @rnames = @{$pinfo->{$par}{rlabels}};
        
        #
        # How we print the param is a function of the number of rows and cols in param.  
        #
        # If #rows = 1 or #cols = 1, then each column represents an element of 
        # the histogram, and each row is a benchmarks. 
        #
        # If #rows != 1 && #cols != 1, then we print one 2-D histogram for each experiment
        # and each benchmark. 
        #
        
        foreach $exp (@exps) {
          #
          # Print header information
          #
          if ($ncols == 1 || $nrows == 1) {
            print OUTPUT "----------------------------------------------------------------------------------\n";
            print OUTPUT "EXPERIMENT: $exp PARAM: $par \n";
            print OUTPUT "----------------------------------------------------------------------------------\n";

	    if ($ncols == 1) {
	      printf OUTPUT $exp_format, "RowName";
	      foreach $bm (@bms) {
	        printf OUTPUT $exp_format, "$bm ";
	      }
	      print OUTPUT "\n";
	    
              foreach $row (@rnames) {
                printf OUTPUT $exp_format, "$row";
		foreach $bm (@bms) {
                  printf OUTPUT $exp_format, $self->_pretty_data($$data_param{$exp}{$bm}{$par}{$row}{$cnames[0]});
                }
                print OUTPUT "\n";
              }
	    }
	    else {
	      printf OUTPUT $exp_format, "ColName";
	      foreach $bm (@bms) {
	        printf OUTPUT $exp_format, "$bm ";
	      }
	      print OUTPUT "\n";
	    
              foreach $col (@cnames) {
                printf OUTPUT $exp_format, "$col";
		foreach $bm (@bms) {
                  printf OUTPUT $exp_format, $self->_pretty_data($$data_param{$exp}{$bm}{$par}{$rnames[0]}{$col});
                }
                print OUTPUT "\n";
              }              
	    }
          }
          else {
            # A 2-D histogram. 
            foreach $bm (@bms) {
              print OUTPUT "----------------------------------------------------------------------------------\n";
              print OUTPUT "EXPERIMENT: $exp BENCHMARK: $bm PARAM:$par \n";
              print OUTPUT "----------------------------------------------------------------------------------\n";
              print OUTPUT "\n";
              
              printf OUTPUT $bench_format, " ";
              foreach $col (@cnames) {
                printf OUTPUT $exp_format, "$col";
              }
              print OUTPUT "\n";
              
              foreach $row (@rnames) {
                printf OUTPUT $bench_format, "$row";
                foreach $col (@cnames) {
                  
#                  my $data = sprintf $format, $$data_param{$exp}{$bm}{$par}{$row}{$col};
                  printf OUTPUT $exp_format, $self->_pretty_data($$data_param{$exp}{$bm}{$par}{$row}{$col});
                  
                }
                print OUTPUT "\n";
              }
            }              
          }
        }
      }
    }
    else {
      print OUTPUT "ERROR: Param $par not found in any stats file.\n";
    }
  }
  print OUTPUT "-----------------------------------------------------------------------------\n";
  close OUTPUT;
  return $self;
}

#
# Creates a nice data string for printing.  Makes sure that we don't have too many 
# siginificant digits. 
#

sub _pretty_data {
  my $self = shift;
  my $data = shift;
  my $new_data = "*NA*";

  if ($data ne "*NA*") {
    $new_data = sprintf "%.0f", $data;
    if ($data != $new_data) {
      # We have a FP value. 
      return (sprintf "%6.3f", $data);
    }
  }
  return $data;
}

#
# Initialize restricted data dimension values to be all values. 
#
sub _init_restricted() {
  my $self = shift;
  my $all = $self->{dimensions};
  my $res = $self->{restricted};
  
  foreach my $key (keys %{$all}) {
    @{$res->{$key}} = @{$all->{$key}}
  }
  return $self;
}

#
# Routine takes an data dimension type and array of lables and stores them 
# in the restricted data dimesion hash.  This is used to print restricted data. 
#
sub _set_restricted() {
  my $self = shift;
  my $type = shift;
  my $restricted = $self->{restricted};
  my @labels = @_;
  my @legal_types = keys %{$self->{dimensions}};

  grep (/$type/, @legal_types) || die "Unable to find $type in list of dimensions.\n";
  
  @{$restricted->{$type}} = @labels;
  return $self;
}

#
# Clear a restricted dimension. 
sub _clear_restricted() {
  my $self = shift;
  my $type = shift;
  my $restricted = $self->{restricted};
  my $dimensions = $self->{dimensions};
  my @legal_types = keys %{$dimensions};

  grep (/$type/, @legal_types) || die "Unable to find $type in list of dimensions.\n";

  @{$restricted->{$type}} = @{$dimensions->{$type}};
  return $self;
}
#
# Takes param definition and creates a string that is used to search for params. 
# All regular expressions are legal (or at least the reg exp that are supported by POSIX)
#
sub _gen_param_search_string() {
  my $self = shift;
  my $pars = shift;

  my $par_str = "";

  # 
  # If regular expressions are supported in the XML library (which can only happen if 
  # a patch is applied to the XML code), then parse the expressions in the following way
  if ($self->{'xpath_regexp'} == 1) {
    my $case;
    foreach my $par (@{$pars}) {
      $case = "";
      if ($par =~ /^\/(.*)\/[i]*$/) {
        
        # We have a regular expression. 
        $par =~ s/^\/(.*)$/$1/;
        
        #
        # Search for the 'case insensitive' option. <str>/i
        if ( $par =~ /^.*\/i$/) {
          $case = 'i';
        }
        $par =~ s/^(.*)\/.*/$1/;
      }
      else {
        #
        # Translate * glob features to regular expressions.  The * glob features
        # are used if you don't have regular expression support in XML. 
        #
        if ( $par =~ /^\*(\w+)\*/ || $par =~ /\|\*(\w+)\*/ ||
             $par =~ /(\w+)\*$/ || $par =~ /\|(\w+)\*$/ ) {
          
          #
          # These are old glob wildcards that I'll translate for now) (i.e., * => .*)
          # However, the glob description will be deprecated. 
          
          $par =~ s/^\*(\w+)/\.\*$1/;
          $par =~ s/\|\*(\w+)/\|\.\*$1/g;
          $par =~ s/(\w+)\*\|/$1\.\*\|/g;
          $par =~ s/(\w+)\*$/$1\.\*/;
          print "WARNING: The <str>* and *<str>* construct has been deprecated. \n";
          print "         I'll fix the syntax for now, but in the future, please replace: \n";
          print "         '<str>*' => '/<str>.*/' \n";
          print "         '*<str>*' => '/.*<str>.*/' \n";
          #      die();
        }
        
        # 
        # Make string explicit (FOO becomes /^FOO$/)
        $par =~ s/^(.*)$/\^$1\$/;
      }
      
      # Make it a xml friendly string. 
      if ($par_str ne "") {
        $par_str .= " or ";
      }
      $par_str .= "regexp(name, '(" . $par . ")','$case')";
      
      #    print "par_str = $par_str\n";
    }
    return $par_str;
  }
  # 
  # Regular expression syntax is not supported in the XML code.  Therefore, use 
  # the glob * syntax to define wildcards for params. 
  #
  else {
    my @search_str;
    my $search_str;
    
    my @abs_str = ();
    my @contains_str = ();
    my @start_str = ();
    
    foreach my $par ( @{$pars} )
    {
      if ( $par =~ /^\*\S*[^*]$/ || $par =~ /^\s*$/ || $par =~ /^\*$/ ||
           $par =~ /^\/(.*)\/[i]*$/ ) {
        print "ERROR: Parameter values must be of the following types: \n";
        print "       <param_name> => Extract parameters named <param_name>.\n";
        print "       *<param_substring>* => Extract parameters which contain substring <param
_substring>\n";
        print "       <param_startstr>* => Extract parameters that start with string <param_st
artstr>\n";
        print "       ILLEGAL VALUES: *<param_substring> && * && /<regular express>/\n";
        die "Undecipherable params $par.\n";
      }
      
      #
      # The syntax of the param specification has been checked in the _set_params code. 
      # Therefore, no additional checks needed here. 
      #
      if ( $par =~ /^\*(\S+)\*$/ ) {
        #
        # All params containing string.
        #
        push (@contains_str, $1);
      }
      elsif ( $par =~ /^(\S+)\*$/ ) {
        # All params starting with string.
        push (@start_str, $1);
      }
      elsif ( $par =~ /^(\S+)\*$/ ) {
        # All params starting with string.
        push (@start_str, $1);
      }
      else {
        push (@abs_str, $par);
      }
    }
    
    # Create the search str.
    if ( scalar(@abs_str) != 0 ) {
      push (@search_str, "name='" . join("' or name='", @abs_str) . "'");
    }
    if ( scalar(@start_str) != 0 ) {
      push (@search_str, "starts-with(name,'" . join("') or starts-with(name,'", @start_str) .
            "')");
    }
    if ( scalar(@contains_str) != 0 ) {
      push (@search_str, "contains(name,'" . join("') or contains(name,'", @contains_str) . "'
)");
    }
    $search_str = join (" or ", @search_str);
    
    return $search_str;
  }
}

#
# This is the crux of the data extraction.  It is basically grabbed from
# grab.pl.  The data generated is stored in a multi-dimensional datastructure
# that is accessed for all data manipulation and plotting.  
#
# NOTE: Initially, all param names have the full path name attached to them.
# This is to make sure that multiple instances of the same parameter have 
# unique names.  Once the extraction is complete, the param names are simplified.
# If there is only one instance of the param, then the path information is 
# removed.  If there are multiple instances of the param, then the smallest
# unique name for each instance is computed.  
#
sub _extract_data {
  my $self = shift;
  my $info = $self->{info};
  my $dir = $info->basedir;
  my @param;
  my @exp;
  my @bench;
  my $run_flag;
  my $num_params = 0;
  my $num_exps = 0;
  my $num_bms = 0;
  my($hist_flag) = 0;
  my($hist_data_flag) = 0;
  my($hist_name) = 0;
  my($rownum) = 0;

  # Clear any existing extracted data. 
  $self->_clear_data;
  my $data_param = ($self->{data}); 
  my $pinfo = $self->{paraminfo};
  my $dimensions = $self->{dimensions};
  
  # Make sure all our options are kosher.  This method also sets the runtype
  # (i.e., batch or local).  
  if ( ! $self->_check_and_expand_options() ) {
    return 0;
  }

  # Set the run flag 
  $run_flag = $info->runtype;

  # 
  # Set inputs.  We HAVE to run _check_and_expand_options first because the
  # routine expands the names of benchmarks and experiments specified by user. 
  # The expanded exp and bm names are put in the 'dimensions' hash. 
  #
  @param = @{$info->params};
  @exp = @{$dimensions->{experiments}};
  @bench = @{$dimensions->{benchmarks}};
  
  my $exp_exp;
  my $search_xpath;

  #
  # Variable needed to patch up paths for parameters.
  # 
  my %ppaths;  # Param paths.

  # build a search command searching for all params at once
  $search_xpath = $self->_gen_param_search_string(\@param);
#  $search_xpath = "name='" . join("' or name='",@param) . "'";
  foreach my $exp (@exp) {
    $exp_exp = Asim::Util::expand_tilda($exp);
    ##
    ## Look for each file in turn
    ##
    foreach my $bench (@bench) {
      ##
      ## Check for compressed file
      ##
      # Remove any spaces around the benchmark
      $bench =~ s/\s*(\S*)\s*/$1/;

      my($filename);
      if ($exp_exp =~ m|^/|) {
        # is absolute path
        $dir = "";
      }

      if ($run_flag eq "batch") {
        $filename = "$dir/$exp_exp/$bench.stats";
      } else {
        $filename = "$dir/$exp_exp/bm/$bench/$bench.stats";
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
        foreach my $elem ($root->findnodes("//scalar[(type='uint' or type='double') and ($search_xpath)]")) {
          my $name = get_nodevalue($elem, "name/text()");
          my $path;
          $path = $self->_add_scalar( $elem, $exp, $bench );
          if ( ! defined $pinfo->{$path} ) {
            #
            # First time we're seeing this param, so include info about it.  
            #
            $pinfo->{$path}{'type'} = "scalar";
            $pinfo->{$path}{nrows} = 1;
            $pinfo->{$path}{ncols} = 1;
            @{$pinfo->{$path}{rlabels}} = ("0");
            @{$pinfo->{$path}{clabels}} = ("0");
            push (@{%ppaths->{$name}}, $path);
            push @{$dimensions->{params}}, $path;
          }
        }

        # process all vector descendents of root
        foreach my $elem ($root->findnodes("//vector[(type='uint' or type='double') and ($search_xpath)]")) {
          my $name = get_nodevalue($elem, "name/text()");
          my $path;
          my $clabels;
          ($path, $clabels) = $self->_add_vector( $elem, $exp, $bench );
          if ( ! defined $pinfo->{$path} ) {
            #
            # First time for seeing vector, so include info. 
            #
            $pinfo->{$path}{'type'} = "histogram";
            $pinfo->{$path}{nrows} = 1;
            $pinfo->{$path}{ncols} = scalar(@{$clabels});
            @{$pinfo->{$path}{rlabels}} = ("0");
            @{$pinfo->{$path}{clabels}} = @{$clabels};
            push (@{%ppaths->{$name}}, $path);
            push @{$dimensions->{params}}, $path;
          }
          else {
            if ( scalar(@{$clabels}) != $pinfo->{$path}{ncols} ) {
              print "ERROR: Vector columns don't match for $path. \n";
              print "       Existing vector has $pinfo->{$path}{ncols}, \n";
              print "       while newly extracted data has scalar(@{$clabels}).\n";
              $self->_clear_data();
              return 0;
            }
          }
        }

        # process all histogram descendents of root
        foreach my $elem ($root->
                          findnodes("//compound[(type='resource' or type='histogram') and ($search_xpath)]"))
        {
          my $name = get_nodevalue($elem, "name/text()");
          my $path = $self->_add_xpath_param ($elem);  # Full path to name

          #
          # get column names.  Do this first because histogram could be 
          # empty. If it is empty, then it is not included in list of params
          # unless it gets in there through another benchmark & experiment. 
          #
          my @data = $elem->findnodes("vector[name='column names']");
          if ($#data != 0) {
            warn "Empty histogram for EXP: $exp, BM: $bench, so patching up.\n"
          }
          else {
            if (! defined $pinfo->{$path}) {
              push (@{%ppaths->{$name}}, $path);
              $pinfo->{$path}{'type'} = "histogram";
              push @{$dimensions->{params}}, $path;
            }
            my $desc = get_nodevalue($elem, "desc/text()");
            
            # get number of Rows
            my $rows = get_nodevalue($elem, "scalar[name='rows']/text()");
            if (defined $pinfo->{$path}{nrows}) {
              ($pinfo->{$path}{nrows} == $rows) || 
              die "Histogram size varies across benchmarks for $path\n";
            } else {
              $pinfo->{$path}{nrows} = $rows;
            }
            
            # get number of Columns
            my $cols = get_nodevalue($elem, "scalar[name='cols']/text()");
            if (defined $pinfo->{$path}{ncols}) {
              if ($pinfo->{$path}{ncols} != $cols) {
                print "ERROR: Histogram size varies across benchmarks for $path\n";
                $self->_clear_data;
                return 0;
              }
            } else {
              $pinfo->{$path}{ncols} = $cols;
            }
            
            my $vec = $data[0];
            my $colnum = 0;
            my @col_labels;
            my @row_labels;
            
            foreach my $data ($vec->findnodes("value/text()")) {
              my $colname = $data->getNodeValue();
              
              if ( defined $pinfo->{$path}{clabels} ) {
                if ($colname ne $pinfo->{$path}{clabels}[$colnum]) {
                  print "ERROR: Col names for $path do not match across benchmarks and experiments\n";
                  $self->_clear_data;
                  return 0;
                }
              }
              else {
                push (@col_labels, $colname);
              }
              $colnum++;
            }
            if ($pinfo->{$path}{ncols} != $colnum) {
              print "ERROR: Mismatch in num of cols and num of col labels for $path\n";
              $self->_clear_data;
              return 0;
            }
            
            #
            # If it's the first time we're seeing this histogram, than copy the col
            # labels to the param info structure. 
            #
            if ( ! defined $pinfo->{$path}{clabels} ) {
              @{$pinfo->{$path}{clabels}} = @col_labels;
            }
            
            #
            # get data
            #
            $rownum = 0;
            foreach my $vec ($elem->findnodes("compound[name='data']/vector")) {
              my $rowname = get_nodevalue($vec, "name/text()");
              
              if ( defined $pinfo->{$path}{rlabels} ) {
                if ($rowname ne $pinfo->{$path}{rlabels}[$rownum]) {
                  print "ERROR: Row names for $path do not match across benchmarks and experiments\n";
                  $self->_clear_data;
                  return 0;
                }
              }
              else {
                push (@row_labels, $rowname);
              }
              
              $colnum = 0;
              foreach my $data ($vec->findnodes("value/text()")) {
                my $value = $data->getNodeValue();
                my $colname = $pinfo->{$path}{clabels}[$colnum];
                #              print "exp: $exp, bm: $bench, par: $path, row: $rowname, col: $colname, value: $value\n";
                $$data_param{"$exp"}{"$bench"}{"$path"}{"$rowname"}{"$colname"} = $value;
                #              print "Hash Value: $$data_param{$exp}{$bench}{$path}{$rowname}{$colname}.\n";
                $colnum++;
              }
              $rownum++;
            }
            #
            # If it's the first time we're seeing this histogram, than copy the row
            # labels to the param info structure. 
            #
            if ( ! defined $pinfo->{$path}{rlabels} ) {
              @{$pinfo->{$path}{rlabels}} = @row_labels;
            }
          }
        }
      }
    }
  }

  # 
  # Come up with simplified path names.  
  # 
  my $path_map = $self->_simplify_param_names(\%ppaths);
  
  #
  # Replace full paths with abbreviated paths. 
  # This modifies both $dimensions->{params} and paraminfo. 
  #
  $self->_replace_path_names($path_map);

  # 
  # Set restricted params to be the same as all params. 
  #
  $self->_init_restricted();

  # 
  # Dump out information about the parameters extracted. 
  #
  $self->_dump_params("all");
  return $self;
}

#
# Routine updates internal data structures with the abbreviated path 
# names for params.  Input is a mapping from the param name to the 
# full and abbreviated paths associated with the parameter. 
# 
sub _replace_path_names ()
{
  my $self = shift;
  my $path_map = shift;
  my $pinfo = $self->{paraminfo};
  my $data = $self->{data};
  my $dimensions = $self->{dimensions};
  my @new_params;

#  foreach my $path (keys %{$pinfo}) {
  foreach my $path (@{$dimensions->{params}}) {
    if (! defined $path_map->{$path}) {
      print "WARNING: No shortened name found for $path.\n";
      push @new_params, $path
    }
    else {
      my $abbr_path = $path_map->{$path};
      %{$pinfo->{$abbr_path}} = %{$pinfo->{$path}};
      delete ($pinfo->{$path});
      push @new_params, $abbr_path
    }
  }

  @{$dimensions->{params}} = @new_params;

  # 
  # Fix names for all data parameters
  #
  foreach my $exp (@{$dimensions->{experiments}}) {
    foreach my $bm (@{$dimensions->{benchmarks}}) {
      foreach my $full_path (keys %{$path_map}) {
        my $abbr_path = $path_map->{$full_path};
	if ( defined $data->{$exp}{$bm}{$full_path} ) {
	  %{$data->{$exp}{$bm}{$abbr_path}} = %{$data->{$exp}{$bm}{$full_path}};
          delete ($data->{$exp}{$bm}{$full_path});
	}
	else {
	  # Data is missing, so fill in *NA*.  Have to walk through all rows and
	  # columns. 
	  foreach my $row (@{$pinfo->{$abbr_path}{rlabels}}) {
	    foreach my $col (@{$pinfo->{$abbr_path}{clabels}}) {
	      $data->{$exp}{$bm}{$abbr_path}{$row}{$col} = "*NA*";
	    }
	  }
        }
      }
    }
  }
}

#
# Routine takes a hash indexed by the param.  Each hash entry contains 
# an array.  The array consists of all instances of the given param. 
# This routine returns a hash with the shortened names for each param. 
# If there is only 1 instance of a param, then the param name is used. 
# 
sub _simplify_param_names ()
{
  my $self = shift;
  my $paths = shift;
  my $path_map = {};
  
  foreach my $param (keys %{$paths}) {
    if (scalar @{$paths->{$param}} == 0) {
      print "WARNING: There are no params named $param.\n";
    }
    elsif (scalar @{$paths->{$param}} == 1) {
      # We only have one instance of param, so use just param name.
      $path_map->{$paths->{$param}[0]} = $param;
    }
    else {
      #
      # Take the first path and determine the hierarchy names. 
      #
      my @hierarchy = split /::/, ($paths->{$param}[0]);
      my $hiercount = scalar(@hierarchy);
      my $match_str;
      
      #
      # Loop through path until we get to the param name itself.  
      # Note that since the param name is the last element in path, 
      # we only loop until count is less than one from the end.
      #
      my $count = 0;
      while ( $count < ($hiercount - 1) ) {
        my $name;
        my $match = 1;
        foreach my $path (@{$paths->{$param}}) {
          my @hiernames = split /::/, $path;
          if (defined $name) {
            if ($hiernames[$count] ne $name) {
              $match = 0;
            }
          }
          else {
            $name = $hiernames[$count];
          }
        }
        if ($match == 1) {
          # 
          # We can remove this hierarchical name from string. 
          if (defined $match_str) {
            $match_str = $match_str . "|" . $name . "::";
          }
          else {
            $match_str = $name . "::";
          }
        }
        $count++;
      }
      if (defined $match_str) {
        #
        # We have common names we have to rip out. 
        #
        foreach my $path (@{$paths->{$param}}) {
          ($path_map->{$path} = $path) =~ s/$match_str//g;
        }
      }
    }
  }
  return $path_map;
}

#
# Computes new param which is a function or existing params or constants. 
# The new param can be either a histogram or a scalar, depending on the 
# input params and the type of operation. 
#
sub _compute_new_param
{
  my $self = shift;
  my @eq_str = @_;
  my $dimensions = $self->{dimensions};
  my $data = $self->{data};
  my $pinfo = $self->{paraminfo};
  my $right;
  my $left;
  
  # Make sure we only have one variable on the left side of equation. 
  my $eq_str = join ("", @eq_str);
  my ($new_param, @vars) = $self->_check_equation_syntax("param", @eq_str);

  if (! defined $new_param) {
    return 1;
  }
  
  #
  # Make sure new param doesn't already exist. 
  #
  if ( defined $pinfo->{$new_param} ) {
    print "ERROR: Param $new_param already exists. \n";
    return 1;
  }

  #
  # Are all the parameters specified on right side legal? 
  #
  my $entry = 0;
  my $prev_hist;
  my $nrows = 1;
  my $ncols = 1;
  my @col_names = ("0");
  my @row_names = ("0");
  foreach my $var (@vars) {
    if ( ! defined $pinfo->{$var} ) {
      print "ERROR: Unable to find $var in list of available parameters.\n";
      print "       Have you run 'get data' yet? \n";
      return 1;
    }
    else {
      #
      # Do we have histograms in equation?  If so, make sure either we only have one
      # histogram on right side of '=' or that all the histograms on right side are
      # the same size.  Also, the left side of equation must be a histogram if there 
      # are any histograms on right side.  The new parameter will have the same 
      # row and column names as the first histogram on the right side of equation. 
      #
      if ($pinfo->{$var}{type} ne "scalar") {
        if (!defined($prev_hist)) {
          $prev_hist = $var;
          $nrows = $pinfo->{$var}{nrows};
          $ncols = $pinfo->{$var}{ncols};
          @col_names = @{$pinfo->{$var}{clabels}};
          @row_names = @{$pinfo->{$var}{rlabels}};
        }
        else {
          ($self->_hist_dimensions_match($prev_hist, $var)) || 
            die "ERROR: Can't deal with different sized histograms in equation.\n";
        }
          
        #
        # Change the variable symbol in equation from $ to @ so that we know this is a 
        # histogram.
        #
        $eq_str =~ s/\$$var([\s\W]+|$)/\@$var$1/;
      }
    }
  }
  
  #
  # If we had histograms in right side of equation, then the left side must also 
  # be a histogram.  Therefore, change symbol from $ to @
  #
  if ($nrows > 1 || $ncols > 1) {
    $eq_str =~ s/\$([a-zA-Z0-9][\w.:_]*\s*=)/\@${1}/;
  }
  
  # Evaluate expression. 
  foreach my $exp (@{$dimensions->{experiments}}) {
    foreach my $bm (@{$dimensions->{benchmarks}}) {
      for (my $row = 0; $row < $nrows; $row++) {
        for (my $col = 0; $col < $ncols; $col++) {
          my $str = $eq_str;
          #
          # Substitute for Scalar variables, if any.  
          #
          $str =~ s/\$([a-zA-Z0-9][\w.:_]*)/\$\$data\{"$exp"\}\{"$bm"\}\{"${1}"\}{0}{0}/g;
          
          #
          # Substitute for Histogram variables, if any.  
          #
          $str =~ s/\@([a-zA-Z0-9][\w.:_]*)/\$\$data\{"$exp"\}\{"$bm"\}\{"${1}"\}\{"$row_names[$row]"\}\{"$col_names[$col]"\}/g;
          
          $left = $str;
          $left =~ s/=[\S\s]*//;
        
          eval "$str";

          # If there's an error, then set value to be *NA*
          if ($@) {
            $str = $left . "= \"*NA*\"";
            eval "$str";
          }
        }
      }
    }
  }
  $self->_add_new_param($new_param, $nrows, $ncols, \@row_names, \@col_names);

  return $self;
}

sub _check_equation_syntax 
{
  my $self = shift;
  my $type = shift;
  my @eq_str = @_;

  # Make sure we only have one variable on the left side of equation. 
  my $eq_str = join ("", @eq_str);
  my @left_vars = $eq_str =~ m/[\$\@]([a-zA-Z0-9][\w.:_]*)\s*=/g;
  if (scalar (@left_vars) != 1) {
    print "ERROR: Equation improperly specified.  Can't parse variable \n";
    print "       name on left side of '='.\n";
    return;
  }

  #
  # Make sure we have at least one variable on the right side of equation.
  #
  my $right = $eq_str;
  $right =~ s/\$[a-zA-Z0-9][\w.:_]*\s*=//;

  my @right_vars = $right =~ m/\$([a-zA-Z0-9][\w.:_]*)/g;

  if (scalar (@right_vars) == 0) {
    print "ERROR: You must specify at least 1 $type name on the right\n";
    print "       side of equation.\n";
    return;
  }

  return ($left_vars[0], @right_vars);
}

sub _compute_new_benchmark
{
  my $self = shift;
  my @eq_str = @_;
  my $dimensions = $self->{dimensions};
  my $data = $self->{data};
  my $pinfo = $self->{paraminfo};
  my $right;
  my $left;

  #
  # Check equation syntax.  Routine returns the left and right side variable
  # names. 
  #
  my ($new_bm, @vars) = $self->_check_equation_syntax("benchmark", @eq_str);

  if ( ! defined $new_bm ) {
    return 1;
  }
  
  my $eq_str = join ("", @eq_str);

  #
  # Make sure new benchmark doesn't already exist. 
  #
  if ( grep /^$new_bm$/, @{$dimensions->{benchmarks}} ) {
    print "ERROR: Benchmark $new_bm already exists. \n";
    return 1;
  }

  #
  # Are all the benchmarks specified on the right side legal? 
  #
  my $entry = 0;
  foreach my $var (@vars) {
    $entry = grep (/^$var$/, @{$dimensions->{benchmarks}});
    if ($entry == 0) {
      print "ERROR: Unable to find $var in list of available benchmarks.\n";
      print "       Have you run 'get data' yet? \n";
      return 1;
    }
  }
  
  # Evaluate expression. 
  foreach my $exp (@{$dimensions->{experiments}}) {
    foreach my $param (@{$dimensions->{params}}) {
      for (my $row = 0; $row < $pinfo->{$param}{nrows}; $row++) {
        for (my $col = 0; $col < $pinfo->{$param}{ncols}; $col++) {
          my $str = $eq_str;
          #
          # Substitute for benchmarks.
          #
          $str =~ s/\$([a-zA-Z0-9][\w.:_]*)/\$\$data\{"$exp"\}\{"${1}"\}\{"$param"\}\{"$pinfo->{$param}{rlabels}[$row]"\}\{"$pinfo->{$param}{clabels}[$col]"\}/g;
          
          $left = $str;
          $left =~ s/=[\S\s]*//;
        
          eval "$str";

          # If there's an error, then set value to be *NA*
          if ($@) {
            $str = $left . "= \"*NA*\"";
            eval "$str";
          }
        }
      }
    }
  }    
    
  # 
  # Insert new benchmark in list of benchmarks.
  #
  push @{$dimensions->{benchmarks}}, $new_bm;
  return $self;
}

#
# Subroutine synthesizes a new value based on user inputs. 
#
sub _define_synthetic_param
{
  my $self = shift;
  my @eq_str = @_;
  my $dimensions = $self->{dimensions};
  my $data = $self->{data};
  my $pinfo = $self->{paraminfo};
  my $functions = $self->{functions};
  my $right;
  my $left;
  my @vars;
  my $new_param;

  my $eq_str = join ("", @eq_str);
  
  my $func_names = join ("|", keys %{$functions});

  # 
  # The new param is either defined as a function (NORMALIZE, SUM, etc) 
  # or as an algebraic equation.  If defined as a function, call the function
  # with the inputs.  Otherwise, call subroutine _compute_new_param
  #
  if ( $eq_str =~ m/\s*\$(\S*)\s*=\s*($func_names)\s*\(([\s\S]*)\)/ ) {
    # We have a function call.
    my $func_name = "_func_" . $2;

    # Call the function specified with the name of the new parameter to 
    # generate and the information passed to the function, i.e., the stuff
    # within the (). 
    my @inputs = split /,/, $3;
    $self->$func_name($1, \@inputs);
  }
  else {
    $self->_compute_new_param(@eq_str);
  }
  return $self;
}

#
# XML syntax parsed here:
# <scalar>
#   <name>foo</name>
#   123.456
# </scalar>
#


sub get_nodevalue # lifted from grab.pl / Stats.pm
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


#
# Path is added to each parameter so that we can come up with unique
# param names for multiple instances of same param in file. 
#
sub _add_xpath_param
{
  my $self = shift;
  my $elem = shift;

  my $parent = $elem;
  my $path = "";
  # iterate over all ancestors elements (*) that have a 'name' child
  # and build the path to this param (incl. the param name)
  foreach my $data ($elem->findnodes("ancestor-or-self::*/name/text()")) {
    my $value = $data->getNodeValue();
    $path .= "::" . $value;
  }
  $path =~ s|^::||; # strip leading /

  return $path;
}

#
# XML syntax parsed here:
# <scalar>
#   <name>foo</name>
#   123.456
# </scalar>
#
sub _add_scalar
{
  my $self = shift;
  my $elem = shift;
  my $exp = shift;
  my $bench = shift;
  my $data_param = ($self->{data});
  my $path;

  my $name = get_nodevalue($elem, "name/text()");
  my $value = get_nodevalue($elem, "text()");
  return  if ($value =~ m/^<error/);
  
  chomp $value;

  #
  # Add the full path to name. 
  $path = $self->_add_xpath_param( $elem );
  
  # Currently adding dummy row and column number of 0 since scalar has 
  # only one value.  
  $$data_param{$exp}{$bench}{$path}{0}{0} = $value;

  #
  # Return the param with the computed path. 
  return $path;
}


# XML syntax parsed here:
# <vector>
#   <name>foo</name>
#   <value>123.456</value>
#   <value>234.678</value>
#   ...
# </vector>
#
sub _add_vector
{
  my $self = shift;
  my $elem = shift;
  my $exp = shift;
  my $bench = shift;
  my $data_param = ($self->{data});
  my $path;
  
  # Determine full path
  $path = _add_xpath_param( $elem );

  # get all values
  my $count = 0;
  my @clabels;
  foreach my $data ($elem->findnodes("value/text()")) {
    my $value = $data->getNodeValue();
    return  if ($value =~ m/^<error/);
    $$data_param{$exp}{$bench}{$path}{0}{$count} = $value;
    push @clabels, $count;
    $count++;
  }
  
  return $path, \@clabels;
}

#
# Function checks that param exists and that it is a histogram.
# 
sub _check_hist_par {
  my $self = shift;
  my $par = shift;
  
  my $pinfo = $self->{paraminfo};

  ( defined ($pinfo->{$par}) ) || die "ERROR: Unknown parameter $par.\n";

  ( $pinfo->{$par}{type} eq "histogram" ) || 
  die "ERROR: Parameter $par is not a histogram.\n";
}

#
# Function returns the sum of each histogram.  It is assumed that the 
# checks to determine whether the parameter is correct and it is a 
# histogram or not have already been done.  
#
sub _sum_hist {
  my $self = shift;
  my $par = shift;
  my $dimensions = $self->{dimensions};
  my $pinfo = $self->{paraminfo};
  my $data = $self->{data};
  my $sum = {};
  
  foreach my $exp ( @{$dimensions->{experiments}} ) {
    foreach my $bm ( @{$dimensions->{benchmarks}} ) {
      my $sum_hist = 0;
      foreach my $row ( @{$pinfo->{$par}{rlabels}} ) {
        foreach my $col ( @{$pinfo->{$par}{clabels}} ) {
	  $sum_hist += $$data{$exp}{$bm}{$par}{$row}{$col};
        }
      }
      $sum->{$exp}{$bm} = $sum_hist;
    }
  }
  
  return $sum;
}

#
# Function that takes a number of params and operates on them with the 
# given operation.  Actually, this routine just creates an equation and
# then sends the equation recursively through the define_synthetic_param 
# subroutine to be evaluated.
#
# Syntax: REDUCE(<op>, <param reg exp>)
# 
# First input: The operation to perform (+, /, -, *, etc)
# Second input: A regular expression representing the type of param(s) you 
#               want to operate on. 
#
sub _func_REDUCE {
  my $self = shift;
  my $new_param = shift;
  my $input = shift;
  my $pinfo = $self->{paraminfo};
  my $dimensions = $self->{dimensions};
  my @pars;
  my $op = shift @{$input};
  
  if (scalar(@{$input}) == 0) {
    print "ERROR: Need at least one input for params.\n";
    print "       Inputs are: \n";
    print "                    1.) Operation to perform.\n";
    print "                    2.) Regular Expression for params on which to perform operation.\n";
    print "                    EX: REDUCE(+, /FOO.*/)\n";
    print "\n";
    return 1;
  }
  
  if (!defined $dimensions->{params}) {
    print "ERROR: No parameter extracted.  Run 'get data' first.\n";
    return 1;
  }
  
  @pars = Asim::Util::expand_regexp($dimensions->{params}, @{$input});
  
  if (scalar(@pars) == 0) {
    print "ERROR: No params matching search string @{$input} found.\n";
    print "       Type: 'list params' to determine list of available params.\n";
    return 1;
  }
  
  if (scalar(@pars) == 1) {
    print "ERROR: Only one param found ($pars[0]) to match search string @{$input}.\n";
    print "       Type: 'list params' to determine list of available params.\n";
    return 1;
  }

  print "Resulting equation contains the following params: \n";
  foreach my $par (@pars) {
    print "\t$par\n";
  }

  #
  # Create equation.
  my $eq = join ("$op\$", @pars);
  $eq = "\$" . "$new_param" . "=\$" . $eq;

  print "Final Equation:\n";
  print "\t$eq\n";
    
  #
  # Send equation through the _define_synthetic_param subroutine (this ends up being a recursive call). 
  $self->_define_synthetic_param($eq);
}

#
# Function that determines if the dimensions of the two histograms given match.
# Matching means: Same number of rows, same number of cols, same row labels, and 
#                 same column labels
#
sub _hist_dimensions_match()
{
  my $self = shift;
  my $p1 = shift;
  my $p2 = shift;
  my $pinfo = $self->{paraminfo};
  
  if ($pinfo->{$p1}{nrows} != $pinfo->{$p2}{nrows}) {
    print "ERROR: Number of rows across histograms do not match.\n";
    return 0;
  }

  if ($pinfo->{$p1}{ncols} != $pinfo->{$p2}{ncols}) {
    print "ERROR: Number of cols across histograms do not match.\n";
    return 0;
  }
  
  my $label1 = join (" ", @{$pinfo->{$p1}{rlabels}});
  my $label2 = join (" ", @{$pinfo->{$p2}{rlabels}});

  if ("$label1" ne "$label2") {
    print "ERROR: Row names do not match across histogarms.\n";
    return 0;
  }

  $label1 = join (" ", @{$pinfo->{$p1}{clabels}});
  $label2 = join (" ", @{$pinfo->{$p2}{clabels}});

  if ("$label1" ne "$label2") {
    print "ERROR: Col names do not match across histogarms.\n";
    return 0;
  }

  return 1;
}


#
# Function that takes a number of scalar or histogram parameters and create a 
# new histogram.  If the inputs are scalar values, then the result will be a histogram
# with 1 row and N columns where N is the number of inputs.  If the inputs are 
# histograms, then following conditions must hold: 
# 1.) All the histogram inputs must either have 1 row or 1 col.
# 2.) All the histograms must be the same size. 
# 3.) In the case of multiple rows, they must all have the same row names, and in 
#     the case of multiple columns, they must all have the same col names. 
#  
# The resulting histogram will retain the row (or col) names of the original data, 
# depending on which one is the non-unit field, and the new col (or row) names will
# be the names of the histograms from where the data came. 
#
# Syntax: COMBINE(<par1|par2|par3>)
#
#         Any perl syntax is legal. 
#         EX: COMBINE(/FOO*|BAR*|FunkyParam|FUNKYParam[13]/)
#
sub _func_COMBINE {
  my $self = shift;
  my $new_param = shift;
  my $input = shift;
  my $pinfo = $self->{paraminfo};
  my $dimensions = $self->{dimensions};
  my $data = $self->{data};
  my @pscalar;
  my @comb;

  if (!defined $dimensions->{params}) {
    print "ERROR: No parameter extracted.  Run 'get data' first.\n";
    return 1;
  }
  
  #
  # Find the requested values
  @comb = Asim::Util::expand_regexp($dimensions->{params}, @{$input});

  #
  # Check some stuff.
  if (scalar(@comb) == 0) {
    print "ERROR: Unable to find any params that match search string ", join(" ", @{$input}), "\n";
    print "       Unable to create new param $new_param.\n";
    return 1;
  }
  
  #
  # Make sure we have more than one param.  Otherwise, it's a mute point.
  if (scalar(@comb) == 1) {
    print "ERROR: Only scalar that matched search string ", join(" ", @{$input}), " is $comb[0].\n";
    print "       It's a mute point to make a vector out of one element.\n";
    return 1;
  }
  
  #
  # Make sure all grepped params are the same size, same labels, etc
  my $rows = 0;
  my $cols = 0;
  my @col_labels;
  my @row_labels;
  my $prev;

  foreach my $param (@comb) {
    if (! defined $prev) {
      $prev = $param;
      $rows = $pinfo->{$param}{nrows};
      $cols = $pinfo->{$param}{ncols};
      @col_labels = @{$pinfo->{$param}{clabels}};
      @row_labels = @{$pinfo->{$param}{rlabels}};
      if ($pinfo->{$prev}{nrows} > 1 && $pinfo->{$prev}{ncols} > 1) {
        print "ERROR: Trying to combine histogram with >1 rows and >1 cols.\n";
        goto COMBINE_ERROR; 
      }
    }
    else {
      # Make sure the prev and current param match. 
      if (! $self->_hist_dimensions_match($param, $prev)) {
	goto COMBINE_ERROR;
      }
    }
  }
  
  #
  # Create new vector (or full 2-d histogram)
  if ($rows == 1 && $cols == 1) {
    # We're dealing with only scalar objects.
    my $nrows = 1;
    my $ncols = scalar(@comb);
    
    foreach my $exp (@{$dimensions->{experiments}}) {
      foreach my $bm (@{$dimensions->{benchmarks}}) {
        foreach my $col (@comb) {
          $$data{$exp}{$bm}{$new_param}{0}{$col} =
          $$data{$exp}{$bm}{$col}{0}{0};
        }
      }
    }
    #
    # Update parameter information.
    # param name, # rows, # cols, row labels, col labels
    $self->_add_new_param($new_param, 1, $ncols, ["0"], \@comb);
  }
  else {
    if ($rows > 1) {
      my $nrows = $rows;
      my $ncols = scalar(@comb);
      # 1 column, multiple rows per param
      foreach my $exp (@{$dimensions->{experiments}}) {
        foreach my $bm (@{$dimensions->{benchmarks}}) {
          foreach my $row (@row_labels) {
            foreach my $col (@comb) {
              $$data{$exp}{$bm}{$new_param}{$row}{$col} =
              $$data{$exp}{$bm}{$col}{$row}{$col_labels[0]};
            }
          }
        }
      }
      # Update parameter information.
      # param name, # rows, # cols, row labels, col labels
      $self->_add_new_param($new_param, $nrows, $ncols, \@row_labels, 
                            \@comb);
    }
    else {
      # 1 row, multiple columns per param
      my $ncols = $cols;
      my $nrows = scalar(@comb);
      foreach my $exp (@{$dimensions->{experiments}}) {
        foreach my $bm (@{$dimensions->{benchmarks}}) {
          foreach my $row (@comb) {
            foreach my $col (@col_labels) {
              $$data{$exp}{$bm}{$new_param}{$row}{$col} =
              $$data{$exp}{$bm}{$col}{$row_labels[0]}{$col};
            }
          }
        }
      }
      # Update parameter information.
      # param name, # rows, # cols, row labels, col labels
      $self->_add_new_param($new_param, $nrows, $ncols, \@comb, 
                            \@col_labels);
    }
  }
  return $self;

 COMBINE_ERROR:
  # Print error
  print "List of params extracted.\n";
  foreach my $par (@comb) {
    print "\t$par,  rows = $pinfo->{$par}{nrows}, cols=$pinfo->{$par}{ncols}\n";
  }
  return 0;
}
  
#
# Function that extracts a given row(s) and/or col(s) from histogram
# If the result of the extract is a single value, then the new 
# parameter becomes a scalar.  Otherwise, it is a histogram. 
#
# Syntax: EXTRACT(<row|rows>=<....>, <col|cols>=<....>, 
#                 <par|param|params>=<param name>)
#
#         Syntax for specifying rows (or cols) is to use the row 
#              label or column label as seen in the histogram: 
#    
#         row=row1:row3:row3:row4..row6:row8
#
#         row1:row3 => Extract rows labeled 'row1' and 'row3'
# 
#         rowN..rowM => Extract rows from label rowN to label rowM, inclusive
#
#         row1:row4:row5..row8 => Extract rows labeled row1, row4, row5, 
#                                 row6, row7, and row8
#     
#
#         REQUIRED: Input parameter
#         OPTIONAL: row or col specification.
#                   If only row is specified, then all columns associated
#                   with the row are extracted, and vice-versa.  If neither
#                   are specified, then the new parameter = input histogram
#
#
sub _func_EXTRACT {
  my $self = shift;
  my $new_param = shift;
  my $inputs = shift;
  my $dimensions = $self->{dimensions};
  my $data = $self->{data};
  my $param;
  my $row_string = "";
  my $col_string = "";
  my @row_names;
  my @col_names;
  
  #
  # Parse input string.
  #
  @{$inputs} = grep { s/\s*(=)\s*/$1/g } @{$inputs};
  my ($key, $value) = 0;
  foreach my $inp_pair (@{$inputs}) {
    ($key, $value) = split/=/, $inp_pair;
    
    if ($key =~ m/param|params|par|pars/) {
      $param = $value;
      $self->_check_hist_par($param);
    }
    elsif ($key =~ m/row|rows/) {
      $row_string = $value;
    }
    elsif ($key =~ m/col|cols/) {
      $col_string = $value;
    }
    else {
      print "ERROR: Unknown input $key to function EXTRACT.\n";
      return 1;
    }
  }
  
  #
  # Determine the rows or cols from the string specified. This function
  # also makes sure that the rows and columns specified are legal labels
  # for the input histogram.
  #
  @row_names = $self->_parse_labels("rlabels", $row_string, $param);
  @col_names = $self->_parse_labels("clabels", $col_string, $param);
  
  # 
  # Create new parameter based on the rows and cols specified.
  #
  my $sval = 0;
  if (scalar(@row_names) == 1 && scalar(@col_names) == 1) {
    $sval = 1;
  }
  
  
  foreach my $exp (@{$dimensions->{experiments}}) {
    foreach my $bm (@{$dimensions->{benchmarks}}) {
      foreach my $row (@row_names) {
        foreach my $col (@col_names) {
          if ($sval == 1) {
            # We have a scalar result
            $$data{$exp}{$bm}{$new_param}{0}{0} = 
            $$data{$exp}{$bm}{$param}{$row}{$col};
          }
          else {
            $$data{$exp}{$bm}{$new_param}{$row}{$col} =
            $$data{$exp}{$bm}{$param}{$row}{$col};
          }
        }
      }
    }
  }

  # Add new parameter to parameter database
  if ($sval == 1) {
    $self->_add_new_param($new_param, 1, 1, ["0"], ["0"]);
  }
  else {
    $self->_add_new_param($new_param, scalar(@row_names), scalar(@col_names), 
                          \@row_names, \@col_names);
  }
  return $self;
}

#
# Function that parses row and col label string provided by user into 
# an array.
#
sub _parse_labels {
  my $self = shift;
  my $label_type = shift;
  my $str = shift;
  my $par = shift;
  my $pinfo = $self->{paraminfo};
  
  my @partial_labels;
  my @param_labels;
  my @parsed_labels;

  (defined $pinfo->{$par}{$label_type}) || 
      die "Unknown label type $label_type for parameter $par.\n";

  @param_labels = @{$pinfo->{$par}{$label_type}};

  # The user didn't specify any string to match, so return all lablels.
  if ($str eq "") {
    return @param_labels;
  }
  
  # remove all spaces
  $str =~ s/\s*//g;

  # Break down string partially using ":" as separator 
  @partial_labels = split /:/, $str;

  # Now determine the labels where the specification is label1..label2
  foreach my $elem ( @partial_labels ) {
    if ( $elem =~ m/(\S+)\.\.(\S+)/ ) {
      #
      # Case where a start and end label are given, and we have to fill
      # in rest of labels.
      #
      my $start_label = $1;
      my $end_label = $2;
      my $start = 0;
      my $end = 0;

      ( $start_label ne $end_label ) || 
	  die ("Start label $start_label and end label $end_label are the same.\n");

      foreach my $label ( @param_labels ) {
        if ( $label eq $start_label ) {
          $start = 1;
	  push @parsed_labels, $label;
        }
	elsif ( $label eq $end_label ) {
	  $end = 1;
	  push @parsed_labels, $label;
          last;
        }
        elsif ( $start == 1 ) {
          push @parsed_labels, $label;	  
        }
      }

      #
      # Make sure we found both the starting and ending label.
      ( $start == 1 ) || die ("Unable to find starting label $start_label.\n");
      ( $end == 1 )   || die ("Unable to find ending label $end_label.\n");
    }
    else {
      # We just have a single label so make sure it's a legal name. 
      ( grep (/^$elem$/, @param_labels) ) ||
        die ("Unable to find label $elem in param $par.\n");
      push @parsed_labels, $elem;
    }
  }

  return @parsed_labels;
}

#
# Function that performs the SUM of a histogram
#
sub _func_SUM {
  my $self = shift;
  my $new_param = shift;
  my $inputs = shift;
  my $dimensions = $self->{dimensions};
  my $pinfo = $self->{paraminfo};
  my $data = $self->{data};

  if (scalar (@{$inputs}) > 1) {
    print "ERROR: Can't sum more than one histogram parameter at a time.\n";
    print "       SYNTAX:  define param $<name> = SUM(<one histogram param>) \n";
    return 0;
  }


  my $par = ${$inputs}[0];
  $self->_check_hist_par($par);

  my $sum = $self->_sum_hist($par);

  foreach my $exp ( @{$dimensions->{experiments}} ) {
    foreach my $bm ( @{$dimensions->{benchmarks}} ) {
      $$data{$exp}{$bm}{$new_param}{0}{0} = $sum->{$exp}{$bm};
    }
  }

  # Add new parameter to list of params. 
  $self->_add_new_param($new_param, 1, 1, ["0"], ["0"]);
  return $self;
}

#
# Function that calculates the PDF of a histogram. 
#
# Inputs: Name of histogram parameter
#         Name of new, synthesized parameter 
# 
sub _func_PDF {
  my $self = shift;
  my $new_param = shift;
  my $inputs = shift;
  my $data = $self->{data};
  my $dimensions = $self->{dimensions};
  my @exps = @{$dimensions->{experiments}};
  my @bms = @{$dimensions->{benchmarks}};
  my $param = ${$inputs}[0];
  
  my $pinfo = $self->{paraminfo};
  
  # Make sure the input is a histogram. 
  $self->_check_hist_par($param);
  
  # Calculate the sum of the histogram.  Function returns a hash with a different
  # sum value for each exp and bm. 
  my $sum = $self->_sum_hist( $param );
  
  #
  # Compute PDF
  #
  my @row_names = @{$pinfo->{$param}{rlabels}};
  my @col_names = @{$pinfo->{$param}{clabels}};
  my $nrows = $pinfo->{$param}{nrows};
  my $ncols = $pinfo->{$param}{ncols};
  foreach my $exp (@exps) {
    foreach my $bm (@bms) {
      my $total;
      if ( ! defined $sum->{$exp}{$bm} ) {
	print "WARNING: No histogram for exp $exp, bm $bm; PDF will be 0.\n";
	$total = 0;
      }
      elsif ( $sum->{$exp}{$bm} == 0 ) {
	print "WARNING: Sum of histogram is 0; PDF of each entry will also be 0. \n";
	$total = 0;
      }
      else {
        $total = $sum->{$exp}{$bm};
      }

      foreach my $row (@row_names) {
	foreach my $col (@col_names) {
	  if ($total != 0) {
	    $$data{$exp}{$bm}{$new_param}{$row}{$col} = 
		($$data{$exp}{$bm}{$param}{$row}{$col} / $total) * 100;
	  }
	  else {
	    $$data{$exp}{$bm}{$new_param}{$row}{$col} = 0;
	  }
	}
      }
    }
  }

  # Add new parameter to list of params. 
  $self->_add_new_param($new_param, $nrows, $ncols, \@row_names, \@col_names);
  return $self;
}
			
#
# Function that normalize data relative to a given bm and/or exp. 
#
# Inputs: Name of new parameter
#         Exp name
#         Bm name
#         Param to normalize
#
# If exp is specified with no bm, then data is normalized per benchmarks relative
# to the experiment given.  
# EXAMPLE: benchmarks FOO, and BAR, and experiment base, and 16KCache.
#   _func_NORMALIZE(new_ipc, exp=base, param=IPC) produces numbers for 
#   every benchmark in 16KCache relative to every benchmark in base.  
#
# If bm is specified with no experiment, then the data
# is normalized to the bm provided within each experiments. 
# EXAMPLE: benchmarks FOO, and BAR, and experiment base, and 16KCache.
#   _func_NORMALIZE(new_IPC, bm=FOO, param=IPC) procues IPC numbers for
#   16KCAche/BAR relative to 16KCache/FOO and base/BAR relative to base/FOO. 
#
# If bm and experiment is given, then all experiments over all benchmarks are
# shown relative the the experiment and benchmark given. 
#
sub _func_NORMALIZE {
  my $self = shift;
  my $new_param = shift;
  my $inputs = shift;
  my $rel_exp;
  my $rel_bm;
  my $rel_par;
  my $rel_norm_na;
  my $dimensions = $self->{dimensions};
  my $pinfo = $self->{paraminfo};
  my $data = $self->{data};
  
  @{$inputs} = grep { s/\s*(=)\s*/$1/g } @{$inputs};
  my ($key, $value) = 0;
  foreach my $inp_pair (@{$inputs}) {
    ($key, $value) = split /=/, $inp_pair;
    
    if ($key =~ m/experiments|exps|exp|experiment/) {
      $rel_exp = $value;
      if ( ! grep (/^$value$/, @{$dimensions->{experiments}}) ) {
        print "ERROR: $value is not a legal experiment. \n";
        return 1;
      }
    }
    elsif ($key =~ m/benchmarks|benchmark|bms|bm/) {
      $rel_bm = $value;
      if ( ! grep (/^$value$/, @{$dimensions->{benchmarks}}) ) {
        print "ERROR: $value is not a legal benchmark. \n";
        return 1;
      }
    }
    elsif ($key =~ m/params|param|par|pars/) {
      $rel_par = $value;
      if ( ! grep (/^$value$/, @{$dimensions->{params}}) ) {
        print "ERROR: $value is not a legal param. \n";
        return 1;
      }
    }
    elsif ($key =~ m/norm_na/) {
      $rel_norm_na = $value;
    }
    else {
      print "ERROR: Unknown dimension $key.\n";
      return 1;
    }
  }

  #
  # ERror checks.
  #
  if (! defined $rel_par) {
    print "ERROR: No parameter specified to use for computation.\n";
    return 1;
  }
  elsif ($pinfo->{$rel_par}{type} ne "scalar") {
    print "ERROR: Can only NORMALIZE scalar values.\n";
    print "       $rel_par is of type $pinfo->{$rel_par}{type}.\n";
    return 1;
  }
  elsif (! defined ($rel_bm) && ! defined ($rel_exp)) {
    print "ERROR: Must specify either an experiment or a benchmark for which to normalize to.\n";
    return 1;
  }
  if (! defined $rel_norm_na || $rel_norm_na eq "") {
    $rel_norm_na = 0;
  }
  
  #
  # Compute new parameter
  #
  my $exp_name;
  my $bm_name;
  foreach my $exp (@{$dimensions->{experiments}}) {
    ( ! defined $rel_exp ) ? ($exp_name = $exp) : ($exp_name = $rel_exp);
    foreach my $bm (@{$dimensions->{benchmarks}}) {
      ( ! defined $rel_bm ) ? $bm_name = ($bm) : ($bm_name = $rel_bm);

      my $norm_value;
      my $param_value;
      $norm_value = $$data{"$exp_name"}{"$bm_name"}{"$rel_par"}{0}{0};

      if (! defined $norm_value) {
        print "ERROR: Something is wrong.  We don't have a value.\n";
        print "ERROR: This will result in incorrect data being computed . \n";
        print "       Exp: $exp_name, BM: $bm_name, PAR: $rel_par, ROW: 0, COL: 0.\n";
	return 1;
      }

      $param_value = $$data{"$exp"}{"$bm"}{"$rel_par"}{0}{0};
      if (! defined $param_value) {
        print "ERROR: Something is wrong.  We don't have a value.\n";
        print "ERROR: This will result in incorrect data being computed . \n";
        print "       Exp: $exp, BM: $bm, PAR: $rel_par, ROW: 0, COL: 0.\n";
	return 1;
      }

      if ($norm_value eq "*NA*" && $param_value ne "*NA*") {
        print "WARNING: No value for EXP: $exp_name, BM: $bm_name, PARAM: $rel_par.\n";
        print "         Result will be $rel_norm_na.\n";
        $$data{"$exp"}{"$bm"}{"$new_param"}{0}{0} = $rel_norm_na;
      }
      elsif ($norm_value eq "*NA*") {
        print "WARNING: No value for EXP: $exp_name, BM: $bm_name, PARAM: $rel_par.\n";
        print "         Result will be 0.\n";
        $$data{"$exp"}{"$bm"}{"$new_param"}{0}{0} = 0;
      }
      elsif ($norm_value == 0) {
        print "WARNING: NORMALIZING to value of 0.\n";
        print "         Result will be 0.\n";
	$$data{"$exp"}{"$bm"}{"$new_param"}{0}{0} = 0;
      }
      else {
        $$data{"$exp"}{"$bm"}{"$new_param"}{0}{0} = $param_value / $norm_value;
      }
    }
  }

  #
  # Update param list with new parameter. 
  #
  $self->_add_new_param($new_param, 1, 1, ["0"], ["0"]);

  return $self;
}

#
# Defined function CDF.  Computes the CDF for a histogram. 
#
sub _func_CDF {
  my $self = shift;
  my $new_param = shift;
  my $inputs = shift;
  my $data = $self->{data};
  my $dimensions = $self->{dimensions};
  my @exps = @{$dimensions->{experiments}};
  my @bms = @{$dimensions->{benchmarks}};
  my $param = ${$inputs}[0];
  
  my $pinfo = $self->{paraminfo};
  
  # Make sure the input is a histogram. 
  $self->_check_hist_par($param);
  
  # Calculate the sum of the histogram.  Function returns a hash with a different
  # sum value for each exp and bm. 
  my $sum = $self->_sum_hist( $param );
  
  #
  # Compute PDF
  #
  my @row_names = @{$pinfo->{$param}{rlabels}};
  my @col_names = @{$pinfo->{$param}{clabels}};
  my $nrows = $pinfo->{$param}{nrows};
  my $ncols = $pinfo->{$param}{ncols};
  foreach my $exp (@exps) {
    foreach my $bm (@bms) {
      my $total;
      if ( ! defined $sum->{$exp}{$bm} ) {
	print "WARNING: No histogram for exp $exp, bm $bm; CDF will be 0.\n";
	$total = 0;
      }
      elsif ( $sum->{$exp}{$bm} == 0 ) {
	print "WARNING: Sum of histogram is 0; CDF of each entry will also be 0. \n";
	$total = 0;
      }
      else {
        $total = $sum->{$exp}{$bm};
      }

      my $running_total = 0;
      foreach my $row (@row_names) {
	foreach my $col (@col_names) {
	  if ($total != 0) {
	    $$data{$exp}{$bm}{$new_param}{$row}{$col} = $running_total +
		($$data{$exp}{$bm}{$param}{$row}{$col} / $total) * 100;
            $running_total = $$data{$exp}{$bm}{$new_param}{$row}{$col};
	  }
	  else {
	    $$data{$exp}{$bm}{$new_param}{$row}{$col} = 0;
	  }
	}
      }
    }
  }

  # Add new parameter to list of params. 
  $self->_add_new_param($new_param, $nrows, $ncols, \@row_names, \@col_names);
  return $self;
}

#
# Defined function SUM_ROWS.  Sums the rows for a histograms. 
# The result is a histogram with 1 row and N columns, where 
# N is the number of columns in the original histogram. 
#
sub _func_SUM_ROWS {
  die "Function SUM_ROWS is not yet implemented.\n";
}

#
# Defined function SUM_COLS.  Sums the cols for a histograms. 
# The result is a histogram with 1 col and N ROWS, where 
# N is the number of rows in the original histogram. 
#
sub _func_SUM_COLS {
  die "Function SUM_COLS is not yet implemented.\n";
}

#
# Defined function SUM_ALL.  Sums all entries in a histogram. 
# The result is a scalar value. 
#
sub _func_SUM_ALL {
  die "Function SUM_ALL is not yet implemented.\n";
}

#
# Defined function COLLATE_ROWS.  Given a row name and N histograms, 
# this function collects the row specified from each histogram and 
# creates a new histogram.  The histograms specified must have the 
# same number of columns (M), and must have a row with the given name.  
# The resulting histogram has N rows and M columns.  The name of each 
# row is the name of the histogram.  The column names remain the same
# as those of the original histograms.
#
sub _func_COLLATE_ROWS {
  die "Function COLLATE_ROWS is not yet implemented.\n";
}

#
# Defined function COLLATE_COLS.  Given a column name and M histograms, 
# this function collects the column specified from each histogram and 
# creates a new histogram.  The histograms specified must have the 
# same number of rows (N), and must have a row with the given name.  
# The resulting histogram has N rows and M columns.  The name of each 
# col is the name of the histogram from which the column was pulled.  
# The row names remain the same as those of the original histograms.
#
sub _func_COLLATE_COLS {
  die "Function COLLATE_COLS is not yet implemented.\n";
}

sub _delete_entry {
  my $self = shift;
  my $type = shift;
  my @items = @_;
  my $search_str = join ("|", @items);
  my $pinfo = $self->{paraminfo};
  my @all_entries = keys %{$pinfo->{$type}};
  my @del_entries;

  foreach my $str (@items) {
    my @matches;
    @matches = grep (/^$str$/, @all_entries);
    push @del_entries, @matches;
  }
  
  (scalar(@del_entries) != 0) || die "ERROR: No $type match given inputs $search_str.\n";

  print "ERROR: Unimplemented feature (for now).\n";
  return 1;
}
  
sub _add_new_param {
  my $self = shift;
  my $param = shift;
  my $nrows = shift;
  my $ncols = shift;
  my $rlabels = shift;
  my $clabels = shift;
  my $pinfo = $self->{paraminfo};
  my $dimensions = $self->{dimensions};

  (! defined $pinfo->{"$param"}) || 
      die "ERROR: Parameter $param already exists in data base. \n";

  # Add new parameter onto list of params
  push (@{$dimensions->{params}}, $param);

  # Add new param to paraminfo list. 
  $pinfo->{$param}{ncols} = $ncols;
  $pinfo->{$param}{nrows} = $nrows;
  @{$pinfo->{$param}{rlabels}} = @{$rlabels};
  @{$pinfo->{$param}{clabels}} = @{$clabels};
  if ($nrows == 1 && $ncols == 1) {
    $pinfo->{$param}{'type'} = "scalar";
  }
  else {
    $pinfo->{$param}{'type'} = "histogram";
  }
}

sub _get_avail_dimensions {
  my $self = shift;
  return $self->{dimensions};
}

#
# Returns a multi-dimensional hash representing all extracted data. 
#
sub _get_data {
  my $self = shift;
  return $self->{data};
}

#
# Returns list of parameters.
#
sub _get_params {
  my $self = shift;
  my $info = $self->{info};
  return $info->params;
}

#
# Returns information about each param. 
#
sub _get_param_info {
  my $self = shift;
  my $pinfo = $self->{paraminfo};
  return $pinfo;
}

sub _sort_dimension {
  my $self = shift;
  my $type = shift;
  my $dimensions = $self->{dimensions};
  
  if (scalar (keys %{$dimensions}) == 0) {
    print "Can't sort $type until you have extracted data. \n";
    print "Run command: 'get data' \n";
  }
  else {
    my @dim = @{$dimensions->{$type}};
    @{$dimensions->{$type}} = sort @dim;
  }
  return $self;
}

sub _clear_data {
  my $self = shift;
  $self->{'dimensions'} = {};
  $self->{restricted} = {};
  $self->{data} = {};
  $self->{paraminfo} = {};
}

#
# Set the params, benchmarks and experiments that we want.  
# Setting these values will clear all existing data from the
# databased.  This means that we no longer have the old data 
# array or the previous param informatino. 
# 
# Note: All this does is setup the data base with the appropriate.  
# information.  It does not determine if the information is correct.
# That can only be determined when the routine _extract_data is run. 
# 

sub _set_benchmarks {
  my $self = shift;
  my @inp = @_;
  my $info = $self->{info};
  
  my $bm = join (" ", @inp);

  #
  # Expand any shell variables in name. This also takes care of ~.
  #
  $bm = Asim::Util::expand_shell($bm);
  
  my @bm = Asim::Util::parse_file_or_params($bm);

  @{$info->benchmarks} = @bm;
  $self->_clear_data();
  return $self;
}

#
# The syntax for a simple parameter extraction is to name 
# the parameter value.  If the user wants all parameters that
# match a string value, then place the param string within '[]'
# Example: [FOO] will retrieve all parameters that have the string
# FOO within their name. 
#
sub _set_params {
  my $self = shift;
  my @inp = @_;
  my $info = $self->{info};

  my $par = join (" ", @inp);
  $par = Asim::Util::expand_shell($par);

  my @par = Asim::Util::parse_file_or_params($par);

  # 
  # Check the user input.  Make sure we're specifying params correctly based on 
  # whether we have the regular expression capabilities in the code or not. 
  # 
  my $error = 0;
  foreach my $par ( @par ) {
    if ($self->{'xpath_regexp'} == 0) {
      # XML library doesn't have regular expression patch. 
      if ( $par =~ /^\*\S*[^*]$/ || $par =~ /^\s*$/ || $par =~ /^\*$/ ||
           $par =~ /^\/(.*)\/[i]*$/ ) {
        print "ERROR: Illegal param: $par.\n";
        $error = 1;
      }
    }
    else {
      # We have regular experssions
      if ( $par =~ /^\*\S*[^*]$/ || $par =~ /^\s*$/ || $par =~ /^\*$/ ) {
        print "ERROR: Illegal param: $par.\n";
        $error = 1;
      }
    }
  }
  
  if ($error == 1) {
    print "ERROR: Illegal syntax for parameters. \n";
    if ($self->{'xpath_regexp'} == 0) {
      print "ERROR: No XML regular expression patch found. Params specification must use glob syntax. \n";
      print "       <param_name>        => Extract parameters named <param_name>.\n";
      print "       *<param_substring>* => Extract parameters which contain substring <param_substring>\n";
      print "       <param_startstr>*   => Extract parameters that start with string <param_startstr>\n";
      print "       ILLEGAL VALUES: *<param_substring> && * && /<reg exp>/[i]\n";
      die "Undecipherable param $par.\n";
    }
    else {
      print "ERROR: Params specification uses either glob syntax or regular expression syntax. \n";
      print "       <param_name>        => Extract parameters named <param_name>.\n";
      print "       *<param_substring>* => Extract parameters which contain substring <param_substring>\n";
      print "       <param_startstr>*   => Extract parameters that start with string <param_startstr>\n";
      print "       /<reg exp>/[i]      => Extract params that match given regular expression.\n";
      print "       ILLEGAL VALUES: *<param_substring> && *\n";
      die "Undecipherable param $par.\n";
    }
  }
  
  @{$info->params} = @par;
  $self->_clear_data();
  return $self;
}

sub _set_experiments {
  my $self = shift;
  my @inp = @_;
  my $info = $self->{info};

  my $exp = join (" ", @inp);

  # 
  # Expand any shell variables.
  #
  $exp = Asim::Util::expand_shell($exp);
  my @exp = Asim::Util::parse_file_or_params($exp);

  @{$info->experiments} = @exp;
  $self->_clear_data();
  return $self;
}

sub _set_basedir {
  my $self = shift;
  my $dir = shift;
  my $info = $self->{info};

  $dir = Asim::Util::expand_shell($dir);

  if (-d $dir) {
    $info->basedir($dir);
    $self->_clear_data;
    return $self;
  }
  else {
    print "ERROR: Base directory ", $dir," is not a valid directory.\n";
    $info->basedir("");
    return 1;
  }
}

sub _dump_params {
  my $self = shift;
  my $type = shift;
  my $info = $self->{info};
  my $pinfo = $self->{paraminfo};
  my $dimensions = $self->{dimensions};

  if (scalar(keys %{$dimensions}) == 0) {
    print "Data is not yet extracted. \n";
    print "\tUser Defined Value: \n\t\t", join("\n\t\t", @{$info->params}), "\n";
  }
  else {
    my $format = "%-30s%-12s%-8s%-8s\n"; 
    printf $format, "NAME", "TYPE", "ROWS", "COLS";
    foreach my $par (@{$dimensions->{params}}) {
      if ($type eq "all" || $type eq $pinfo->{$par}{type}) {
        if (length($par) > 29) {
          print "$par\n";
          printf $format, "", $pinfo->{$par}{type}, $pinfo->{$par}{nrows}, 
          $pinfo->{$par}{ncols};
        }
        else {
          printf $format, $par, $pinfo->{$par}{type}, $pinfo->{$par}{nrows}, 
          $pinfo->{$par}{ncols};
        }
      }
    }
  }
}

sub _dump_options {
  my $self = shift;
  my $info = $self->{info};

  print "STATS OPTIONS: \n";
  print "Basedir: \n";
  print "\t", $info->basedir,"\n";
 
  $self->_dump_info("experiments");
  $self->_dump_info("benchmarks");	
  $self->_dump_params("all");
}

sub _dump_info {
  my $self = shift;
  my $dim = shift;
  my $info = $self->{info};
  my $dimensions = $self->{dimensions};

  print $dim, ": \n";

  if (scalar (keys %{$dimensions}) == 0) {
    print "Data is not yet extracted. \n";
    print "\tUser Defined Value: \n\t\t", join("\n\t\t", @{$info->$dim}), "\n";
  }
  else {
    for my $elem (@{$dimensions->{$dim}}) {
      print "\t$elem\n";
    }
  }
}

#
# Set experiments to all directories under basedir.
#
sub _expand_experiments {
    my $self = shift;
    my $info = $self->{info};
    my $basedir = $info->basedir;

    if (! opendir(BASEDIR, "$basedir")) {
	print "ERROR: Unable to open directory $basedir.\n";
	return 0;
    }
    
    # Find all subdirectories of BASEDIR
    my @exps = readdir BASEDIR;
    @exps = grep {$_ ne '.' and $_ ne '..'} @exps;
    @exps = grep {-d "$basedir/$_"} @exps;

    close (BASEDIR);

    #
    # Get the experiments requeted by user. 
    my @all=Asim::Util::expand_regexp(\@exps, @{$info->experiments});
    (scalar (@all) != 0) || die "ERROR: No directories found in $basedir.\n";
    
    return (@all);
}

#
# Set benchmarks to the superset of benchmarks in all experiments directories. 
#
sub _expand_benchmarks {
    my $self = shift;
    my @exps = @_;
    my $info = $self->{info};
    my $basedir = $info->basedir;

    my @bms;
    my $runtype;
    foreach my $exp (@exps) {
      #
      # Determine whether we have a directory structure created by
      # asim-run ($basedir/$exp) or one created by local runs of asim
      # ($basedir/bm/$exp)
      #
      if (!defined $runtype) {
        if (opendir (EXPDIR, "$basedir/$exp/bm")) {
          $runtype = "local";
        }
        elsif (opendir (EXPDIR, "$basedir/$exp")) {
          $runtype = "batch";
        }
        else {
          print "WARNING: Unable to open directory $basedir/$exp/bm or $basedir/$exp.\n";
        }
      }
      if (defined $runtype && $runtype eq "local") {
        if (! opendir(EXPDIR, "$basedir/$exp/bm")) {
          print "WARNING: Unable to open directory $exp/bm.\n";
        }
        my @alldirs = readdir EXPDIR;
        @alldirs = grep {$_ ne '.' and $_ ne '..'} @alldirs;
        @alldirs = grep {-d "$basedir/$exp/bm/$_"} @alldirs;
        
        #
        # Make sure there's a stats file in the benchmark dir. 
        # If not, then the directory is probably not a benchmark dir. 
        #
        foreach my $dir (@alldirs) {
          if (-f "$basedir/$exp/bm/$dir/$dir.stats" ||
              -f "$basedir/$exp/bm/$dir/$dir.stats.gz" ||
              -f "$basedir/$exp/bm/$dir/$dir.stats.bz2")
          {
            push @bms, $dir;
          }
        }
        closedir EXPDIR;
      }
      elsif (defined $runtype && $runtype eq "batch") {
        if (! opendir(EXPDIR, "$basedir/$exp")) {
          print "WARNING: Unable to open directory $exp.\n";
        }
        else {
          my @files = grep (/\S+.stats$/ || /\S+.stats.gz/ || /\S+.stats.bz2/, readdir EXPDIR);
          @files = grep { s/.stats\S*$// } @files;
          push (@bms, @files);
          closedir EXPDIR;
        }
      }
    }

    my @all = Asim::Util::expand_regexp(\@bms, @{$info->benchmarks});
    (scalar (@all) != 0) || die "ERROR: Unable to find any benchmark files. \n";
    #
    # Set the runtype that we found. 
    #
    $info->runtype("$runtype");
    return @all;
}
  

sub _check_and_expand_options {
  my $self = shift;
  my $info = $self->{info};
  my $dimensions = $self->{dimensions};
  my $basedir = $info->basedir;
  my $bm_fl;

  my $num_exps = 0;
  my $num_bms = 0;
  my @expanded_exps;
  my @expanded_bms;
  
  # Check that basedir exists
  if ( ! -d $basedir ) {
    print "ERROR: Directory $basedir does not exist.\n";
    return 0;
  }

  #
  # Expand "all" requests.  If the user specifies "all", then 
  # experiments = all directories in basedir, and benchmarks = 
  # the superset of all benchmarks found in all experiments directories. 
  #

  if (scalar @{$info->experiments} == 1 && @{$info->experiments}[0] eq "all") {
    print "WARNING: The 'set experiments all' command has been deprecated.\n";
    print "         In the future, please use 'set experiments /.*/\n";
    @{$info->experiments} = ("/.*/");
  }
  
  @expanded_exps = $self->_expand_experiments();
#  print "exps = ", join (", ", @expanded_exps), "\n";
  
  if (scalar @{$info->benchmarks} == 1 && @{$info->benchmarks}[0] eq "all") {
    print "WARNING: The 'set benchmarks all' command has been deprecated.\n";
    print "         In the future, please use 'set benchmarks /.*/\n";
    @{$info->benchmarks} = ("/.*/");
  }
  
  @expanded_bms = $self->_expand_benchmarks(@expanded_exps);
#  print "bms = ", join (", ", @expanded_bms), "\n";
  
  # Check for experiments 
  foreach my $exp (@expanded_exps) {
    if ( ! -d "$basedir/$exp" ) {
      print "WARNING: Unable to find $exp under $basedir.\n";
    }
    else {
      $num_exps++;
      foreach my $bm (@expanded_bms) {
        if ( $info->runtype eq "batch" ) {
          $bm_fl = "$basedir/$exp/$bm.stats";
        }
        else {
          $bm_fl = "$basedir/$exp/bm/$bm/$bm.stats";
        }
        if ( ! -f "$bm_fl" && ! -f "$bm_fl.gz" && ! -f "$bm_fl.bz2") {
          print "WARNING: Unable to find $bm_fl(.gz|.bz2)?\n";
        }
        else {
          $num_bms++;
        }
      }
    }
  }
  
  if ( $num_exps == 0 ) {
    print "ERROR: Unable to find any valid experiments.\n";
    return 0;
  }
  if ( $num_bms == 0 ) {
    print "ERROR: Unable to find any valid benchmarks.\n";
    return 0;
  }

  # 
  # Set the extracted dimensions to be the expanded bms and exps.
  @{$dimensions->{experiments}} = @expanded_exps;
  @{$dimensions->{benchmarks}} = @expanded_bms;
  
  return 1;
}

###########################################################################

=back

=head1 AUTHORS

Srilatha Manne

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
