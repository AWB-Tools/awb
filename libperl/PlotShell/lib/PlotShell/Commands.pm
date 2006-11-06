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

package PlotShell;
use warnings;
use strict;

use Getopt::Long;
use File::Spec;
use Term::ReadLine;

use Asim::Stats;
use Asim::PlotStats;
use Asim::Util;

#
# Default data values
#
our $asim_stats = Asim::Stats->new();
our $asim_plot = ();
#our $asim_plot = Asim::PlotStats->new();

our @history = ();


################################################################
#
#  Implemetation of individual commands
#
################################################################

sub cd {
  my $dir = shift ||
    $ENV{AWBLOCAL};

  chdir $dir 
    || shell_error("Cd to $dir failed\n") && return ();

  return 1;
}

sub pwd {

  system "pwd";
  return 1;
}

sub ls {
  my $dir = shift || "";

  system "ls $dir";

  return 1;
}

################################################################
#
# Data extraction & manipulation functions
#
################################################################

sub set_basedir {
  push (@history, "set basedir " . join(" ", @_));
  my $path = shift;
  $asim_stats->_set_basedir($path);
  delete_plot();
  return $asim_stats;
}

#****************************************************
# EXPERIMENTS
#****************************************************
sub set_experiments {
  push (@history, "set experiments " . join(" ", @_));
  my @exps = @_;
  $asim_stats->_set_experiments(@exps);
  delete_plot();
  return $asim_stats;
}

sub sort_experiments {
  push (@history, "sort experiments");
  $asim_stats->_sort_dimension("experiments");
  return $asim_stats;
}

sub delete_experiments {
  push (@history, "delete experiments " . join (" ", @_));
  $asim_stats->_delete_entry("experiments", @_);
  delete_plot();
  return $asim_stats;
}
  
#****************************************************
# BENCHMARKS
#****************************************************
sub set_benchmarks {
  push (@history, "set benchmarks " . join(" ", @_));
  my @bms = @_;
  $asim_stats->_set_benchmarks(@bms);
  delete_plot();
  return $asim_stats;
}

sub define_benchmarks {
  push (@history, "define benchmarks " . join(" ", @_));
  $asim_stats->_compute_new_benchmark(@_);
  return $asim_stats;
}

sub sort_benchmarks {
  push (@history, "sort benchmarks");
  $asim_stats->_sort_dimension("benchmarks");
  return $asim_stats;
}

sub delete_benchmarks {
  push (@history, "delete benchmarks " . join (" ", @_));
  $asim_stats->_delete_entry("benchmarks", @_);
  delete_plot();
  return $asim_stats;
}

#****************************************************
# PARAMS
#****************************************************
sub set_params {
  push (@history, "set params " . join(" ", @_));
  my @par = @_;
  $asim_stats->_set_params(@par);
  delete_plot();
  return $asim_stats;
}

sub sort_params {
  push (@history, "sort params ");
  $asim_stats->_sort_dimension("params");
  return $asim_stats;
}

sub delete_params {
  push (@history, "delete params " . join (" ", @_));
  $asim_stats->_delete_entry("params", @_);
  delete_plot();
  return $asim_stats;
}

sub define_params {
  push (@history, "define params " . join(" ", @_));
  $asim_stats->_define_synthetic_param(@_);
  return $asim_stats;
}

sub define_value {
  print "ERROR: define value is a deprecated function.  \n";
  print "       Please use define param or define benchmark to compute new \n";
  print "       param or benchmark, respectively.\n";
  return $asim_stats;
}
  
sub set_runtype {
  print "ERROR: 'set runtype' is a deprecated function.  \n";
  print "       This command is no longer needed.  plot-shell automatically \n";
  print "       figures out the type of run (batch or local).\n";
  return $asim_stats;
}

sub list_params {
  push (@history, "list params");
  $asim_stats->_dump_params("all");
  return $asim_stats;
}

sub list_histograms {
  push (@history, "list histograms");
  $asim_stats->_dump_params("histogram");
  return $asim_stats;
}

sub list_scalars {
  push (@history, "list scalars");
  $asim_stats->_dump_params("scalar");
  return $asim_stats;
}

sub list_experiments {
  push (@history, "list experiments");
  $asim_stats->_dump_info("experiments");
  return $asim_stats;
}

sub list_benchmarks {
  push (@history, "list benchmarks");
  $asim_stats->_dump_info("benchmarks");
  return $asim_stats;
}

sub list_stats_options {
  push (@history, "list stats options");
  $asim_stats->_dump_options();
  return $asim_stats;
}

sub get_data {
  push (@history, "get data " . join(" ", @_));
  delete_plot();
  if ($asim_stats->_extract_data()) {
    # Create a new plot object.
    if ( ! ($asim_plot = Asim::PlotStats->new()) ) {
      undef ($asim_plot);
    }
    else {
      $asim_plot->_set_avail_dimensions($asim_stats->_get_avail_dimensions);
    }
  }
}

################################################################
#
# Loop construct
#
################################################################
sub foreach {
  push (@history, "#LOOP COMMAND: foreach " . join (" ", @_));
  my @inputs = @_;
  my $var = shift @inputs;
  my $val_str = "";
  my @values;
  my $str = join (" ", @inputs);
  $str =~ s/\[/\[ /;
  $str =~ s/\]/ \]/;
  $str =~ s/,/, /g;
  @inputs = split (/ +/, $str);
  if ($inputs[0] ne "[") {
    return (&error_in_loop());
  }
  shift @inputs;
  while ( defined($inputs[0]) && $inputs[0] ne "]" ) {
    $val_str .= shift @inputs;
#    push @values, shift @inputs;
  }

  # Make sure we've reached a "]"
  if (!defined($inputs[0])) {
    return (&error_in_loop());
  }
  else {
    # Shift out the "]"
    shift @inputs;
  }

  # Make inputs into array. 
  push @values, (split /[, ]+/, $val_str);
  
  # Execute the loop requested.  I end up calling 
  # the run_command function with the substitued 
  # equation
  foreach my $val (@values) {
    my $command = join (" ", @inputs);
    $command =~ s/\[$var\]/$val/g;
    foreach my $com (split /\s*;\s*/, $command) {
      my @command_str = split / /, $com;
      print "EXECUTING COMMAND: $com\n";
      run_command(@command_str);
    }
  }
  return 1;
}

################################################################
#
# Plot Functions
#
################################################################

sub delete_plot {
  if (defined ($asim_plot)) {
    print "WARNING: Deleting all plotting options.\n";
    undef ($asim_plot);
  }
  return 1;
}

sub check_plot {
  if ( ! defined($asim_plot) ) {
    print "ERROR: Can't set any plot options until you extract data.\n";
    print "       Run 'get data' first.\n";
    return 0;
  }
  return 1;
}
    
sub set_plot_legendloc {
  if (check_plot()) {
    push (@history, "set plot legendloc " . join(" ", @_));
    my $loc = shift;
    my $info = $asim_plot->{info};
    
    $loc =~ tr/A-Z/a-z/;
    $info->legendloc($loc);
    $asim_plot->_check_legendloc();
    return $asim_plot;
  }
  return 1;
}

sub set_plot_width {
  if (check_plot()) {
    push (@history, "set plot width " . join(" ", @_));
    my $w = shift;
    my $info = $asim_plot->{info};
    
    $info->width($w);
    $asim_plot->_check_width();
    return $asim_plot;
  }
  return 1;
}

sub set_plot_height {
  if (check_plot()) {
    push (@history, "set plot height " . join(" ", @_));
    my $h = shift;
    my $info = $asim_plot->{info};
    $info->height($h);
    $asim_plot->_check_height();
    return $asim_plot;
  }
  
  return 1;
}

#
# Note: We don't check that ymin < ymax here because ymax value might 
# be unknown. If ymax is not set, it is computed during plot data
#
sub set_plot_ymin {
  if (check_plot()) {
    push (@history, "set plot ymin " . join(" ", @_));
    my $y = shift;
    my $info = $asim_plot->{info};
    $info->ymin($y);
    return $asim_plot;
  }
  
  return 1;
}

sub set_plot_ymax {
  if (check_plot()) {
    push (@history, "set plot ymax " . join(" ", @_));
    my $y = shift;
    my $info = $asim_plot->{info};
    $info->ymax($y);
    return $asim_plot;
  }
  
  return 1;
}

sub set_plot_output {
  if (check_plot()) {
    push (@history, "set plot output " . join(" ", @_));
    my $out = shift;
    my $filename = shift;

    my $info = $asim_plot->{info};
    
    $out =~ tr/A-Z/a-z/;
    $info->output($out);
    if (defined $filename) {
      $info->output_fl(Asim::Util::expand_shell($filename));
    }
    $asim_plot->_check_output();
    return $asim_plot;
  }
  return 1;
}

sub set_plot_landscape {
  if (check_plot()) {
    push (@history, "set plot landscape");
    my $info = $asim_plot->{info};
    $info->orientation("landscape");
    
    if ("$info->output()" ne "ps") {
      print "WARNING: Landscape orientation will only work on ps output files.\n";
      print "         Use 'set plot output ps <filename>' to change output file type to ps.\n";
    }
  }
}

sub set_plot_portrait {
  if (check_plot()) {
    push (@history, "set plot portrait");
    my $info = $asim_plot->{info};
    $info->orientation("portrait");
    
    if ("$info->output()" ne "ps") {
      print "WARNING: Portrait orientation is only relevant to ps output files.\n";
    }
  }
}

sub set_plot_type {
  if (check_plot()) {
    push (@history, "set plot type " . join(" ", @_));
    my $type = shift;
    my @dim = @_;
    # asim_plot receives the data dimension structure from asim_stats so that 
    # it can populate the labels field correctly from the list of params, 
    # experiments, benchmarks, etc. 
    $asim_plot->_set_plot_type($asim_stats->_get_avail_dimensions, 
                               $asim_stats->_get_param_info,
                               $type, @dim);
    return $asim_plot;
  }
  return 1;
}

sub restrict {
  if (check_plot()) {
    push (@history, "restrict " . join(" ", @_));
    my $dim = shift;
    my @str = @_;
    my @res_elems;
    @res_elems = $asim_plot->_define_synthetic_name($dim, 
						    $asim_stats->_get_avail_dimensions, 
						    @str);
    $asim_stats->_set_restricted($dim, @res_elems);
    return $asim_plot;
  }
  return 1;
}

sub replace {
  print "WARNING: The 'replace' command is being deprecated. \n";
  print "         Please use 'restrict' in the future.\n";
  restrict(@_);
}

sub unrestrict {
  if (check_plot()) {
    my $dim = shift;
    push (@history, "unrestrict $dim " . join(" ", @_));
    $asim_plot->_undefine_synthetic_name($dim, $asim_stats->_get_avail_dimensions);
    $asim_stats->_clear_restricted($dim);
    return $asim_plot;
  }
  return 1;
}


sub reset {
  print "WARNING: The 'reset' command is being deprecated. \n";
  print "         Please use 'unrestrict' in the future.\n";
  unrestrict(@_);
}
    
sub group {
  if (check_plot()) {
    my $dim = shift; 
    push (@history, "group $dim " . join(" ", @_));
    my $name = shift;
    my @inp = @_;
    $asim_plot->_group_dim($name, $asim_stats->_get_avail_dimensions, $dim, @inp);
  }
  return 1;
}

sub set_group {
  if (check_plot()) {
    push (@history, "set group " . join(" ", @_));
    my $name = shift;
    my @inp = @_;
    $asim_plot->_set_group($name, @inp);
  }
  return 1;
}

sub set_plot_group {
  print "WARNING: The 'set plot group' command is being deprecated. \n";
  print "         Please use 'set group' in the future.\n";
  set_group(@_);
  return 1;
}
  
sub set_plot_title {
  if (check_plot()) {
    push (@history, "set plot title \"" . join(" ", @_) . "\"");
    my $title = "\"\"" . join(" ", @_) . "\"\"";
    print "TITLE: $title\n";
    my $info = $asim_plot->{info};
    $info->title($title);
    return $asim_plot;
  }
  return 1;
}    

sub set_plot_ylabel {
  if (check_plot()) {
    push (@history, "set plot ylabel \"" . join(" ", @_) . "\"");
    my $label = shift;
    my $info = $asim_plot->{info};
    $info->ylabel($label);
    return $asim_plot;
  }
  return 1;
}

sub set_plot_xlabel {
  if (check_plot()) {
    push (@history, "set plot xlabel \"" . join(" ", @_) . "\"");
    my $label = shift;
    my $info = $asim_plot->{info};
    $info->xlabel($label);
    return $asim_plot;
  }
  return 1;
}

sub list_plot_dimensions {
  if (check_plot()) {
    push (@history, "list plot dimensions");
    $asim_plot->_dump_dimensions();
    return $asim_plot;
  }
  return 1;
}

sub list_plot_options {
  if (check_plot()) {
    push (@history, "list plot options");
    $asim_plot->_dump_options();
    return $asim_plot;
  }
  return 1;
}

sub list_groups {
  if (check_plot()) {
    push (@history, "list groups");
    $asim_plot->_dump_groups();
    return $asim_plot;
  }
  return 1;
}

sub list_replacements {
  print "WARNING: The 'list replacements' command is being deprecated. \n";
  print "         Please use 'list restrictions' in the future.\n";
  list_restrictions();
}

sub list_restrictions {
  if (check_plot()) {
    push (@history, "list restrictions");
    $asim_plot->_dump_replacements();
    return $asim_plot;
  }
  return 1;
}

sub list_mappings {
  if (check_plot()) {
    push (@history, "list mappings");
    $asim_plot->_dump_mappings();
    return $asim_plot;
  }
  return 1;
}

#
# Deprecated call.  In the future, please use plot scurve
#
sub plot_sorted_data {
  print "WARNING: The 'plot sorted data' command is being deprecated. \n";
  print "         Please use 'plot scurve' in the future.\n";
  plot_scurve(@_);
  return 1;
}

#
# There are two different type of curve plots: scurve and ucurve.
# A '1' means scurve, and a '2' means ucurve (also known as vcurve
sub plot_scurve {
  if (check_plot()) {
    push (@history, "plot scurve @_");
    _plot_curve(1, @_);
  }
  return 1;
}

# Same as ucurve
sub plot_vcurve {
  return plot_ucurve(@_);
}
  

sub plot_ucurve {
  if (check_plot()) {
    push (@history, "plot ucurve @_");
    _plot_curve(2, @_);
  }
  return 1;
}
  

#
# This routine should never be called directly. 
#
sub _plot_curve {
  my $sort_type = shift;
  my $sort_name = shift;
  if (check_plot()) {
    my $key;
    my $name = "";
    if (defined $sort_name) {
      ($key, $name) = split (/=/, $sort_name);
      if ($key ne "line" && $key ne "lines" && $key ne "l") {
        print "ERROR: $sort_name is not legal.\n";
        print "ERROR: Legal specification is 'line=<name of line>'\n";
        return 0;
      }
    }
    
    # Dump data in the required format for plotting.
    $asim_plot->_plot_data($asim_stats->_get_avail_dimensions, 
                           $asim_stats->_get_data, 
                           $asim_stats->_get_param_info, 
                           $sort_type,   # Sort data flag
                           $name);       # Sort field name
    return $asim_plot;
  }
  return 1;
}

sub plot_data {
  # Make sure we've extracted data before trying to plot it. 
  if (check_plot()) {
    push (@history, "plot data");
    # Dump data in the required format for plotting.
    $asim_plot->_plot_data($asim_stats->_get_avail_dimensions, 
                           $asim_stats->_get_data, 
                           $asim_stats->_get_param_info, 
                           0,            # Sort data flag
                           "");          # Sort field name
    return $asim_plot;
  }
  return 1;
}


################################################################
#
# Print Functions
#
################################################################

sub print_data {
  push (@history, "print data @_");
  my $output = shift;
  if (defined $output) {
    $output = Asim::Util::expand_shell($output);
  }
  $asim_stats->_print_data("all", $output);
  return $asim_stats;
}

sub print_restricted_data {
  push (@history, "print restricted data @_");
  my $output = shift;
  if (defined $output) {
    $output = Asim::Util::expand_shell($output);
  }
  $asim_stats->_print_data("restricted", $output);
  return $asim_stats;
}

################################################################
#
# Miscellaneous functions
#
################################################################

sub dump_history {
  my $file = shift;
  if (defined $file) {
    $file = Asim::Util::expand_shell($file);
    if (! open (HIST_FL, ">$file")) {
      print "ERROR: Unable to open file $file for dumping command history.\n";
      return 0;
    }
    else {
      foreach my $com (@history) {
        printf HIST_FL "$com\n";
      }
      close HIST_FL;
    }
  }
  else {
    foreach my $com (@history) {
      print "$com\n";
    }
  }
}

sub error_in_loop {
  print "ERROR: There's an error in the definition of the loop.\n";
  print "Correct Syntax: \n";
  print "\t\tforeach <name> (<val1>, <val2>, <val3>...<valN>) <plot-shell command> \n\n";
  print "\t\tEXAMPLE: \n\n";
  print "\t\t\tforeach var [0, 1, 2, 3, 4] define param \$MR_[var] = \$SLICE_[var]::DcacheMisses / \$SLICE_[var]::DcacheHits\n\n";
  return 0;
}

1;


