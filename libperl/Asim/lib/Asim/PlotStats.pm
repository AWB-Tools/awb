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


package Asim::PlotStats;
use warnings;
use strict;
use Class::Struct;
use File::Basename;

struct plot_info => {
  output       =>        '$',   
  output_fl    =>        '$',
  orientation  =>        '$',                     
  legendloc    =>        '$',
  width        =>        '$', 
  height       =>        '$', 
  ymax         =>        '$',
  ymin         =>        '$',
  ylabel       =>        '$', 
  xlabel       =>        '$', 
  title        =>        '$',
  type         =>        '$', 
  bar          =>        '$',           
  cluster      =>        '$', 
  stack        =>        '$', 
  box          =>        '$', 
  dimensions   =>        '$',
};

our %valid_options = (
    output       =>        ["x11", "eps", "gif", "ps", "png", "bwps"],
    lloc         =>        ["top", "bottom", "left", "right"],
    lloc_abbr    =>        {top => "t", bottom => "b", left => "l", right => "r"},
    type         =>        ["bar", "box", "line"],
);

our %pl_scripts = (
    box          => "tools/libploticus/box.script",
    bar          => "tools/libploticus/bar.script",
    line         => "tools/libploticus/line.script",
    basedir      => "",
);

our $ploticus = "pl";
our $TMP_DIR = (defined $ENV{ASIM_TMPDIR} && -d "$ENV{ASIM_TMPDIR}") ? "$ENV{ASIM_TMPDIR}" : 
                                        (defined $ENV{TMPDIR} && -d "$ENV{TMPDIR}") ? "$ENV{TMPDIR}" : 
                                        "/tmp";

#
# We have the different plot types (bar and line so far), and the associated
# information about each plot type.  
#
our %plot_map = (
  bar     =>  { bar     =>  {dim => "", labels => []},
                stack   =>  {dim => "", labels => []},
                cluster =>  {dim => "", labels => []}
              },
  box     =>  { bar     =>  {dim => "", labels => []},
                box     =>  {dim => "", labels => []},
                cluster =>  {dim => "", labels => []},
              },
  line    =>  { xaxis   =>  {dim => "", labels => []},
                line    =>  {dim => "", labels => []},
              },
);

our $debug = 0;

=head1 NAME

Asim::PlotStats - Library for plotting stats data.

=head1 SYNOPSIS

use Asim::PlotStats

TBD

=head1 DESCRIPTION

This module provides an object for plotting extracted stats data

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
  my $info = 
      plot_info->new( output => 'x11',
                      output_fl => "",
                      orientation => "portrait",
                      ymax => "",
                      ymin => "",
		      legendloc => 'top',
		      width => 0, 
		      height => 0, 
		      type => "", 
                      title => "",
                      xlabel => "",
                      ylabel => "",
                      bar => "",
		      cluster => "", 
		      stack => "", 
		      box => "",
                      dimensions => "",
		      );

  # Add additional information to self. 
  # The data hash stores the data structure. 
  $self->{'info'} = $info;
  $self->{'valid_options'} = \%valid_options;
  
  # Mapping of plot dimensions to data dimensions. 
  $self->{'plot_2_data'} = \%plot_map;

  # User generated dimensions used for plotting. 
  $self->{'groups'} = {};

  # User may choose to generate the name of a data dimension value 
  # rather than just specifying one of the data dimensions directly. 
  $self->{'gen_str'} = {};

  # Mapping of data dimensions to plot dimensions. 
  $self->{'data_2_plot'} = {};

  # Resolve path names for all ploticus scripts. 
  $self->{scripts} = \%pl_scripts;
  %pl_scripts->{box} = Asim::Util::resolve(%pl_scripts->{box});
  %pl_scripts->{bar} = Asim::Util::resolve(%pl_scripts->{bar});
  %pl_scripts->{line} = Asim::Util::resolve(%pl_scripts->{line});

  # Determine the base directory for ploticus scripts. We need to set
  # the shell environment PLOTICUS_PREFABS to the base directory so
  # that the ploticus scripts can find some include files that they
  # need.
  $ENV{PLOTICUS_PREFABS} = dirname(%pl_scripts->{box});

  # Make sure ploticus exists and is usable 
  if (system ("which $ploticus > /dev/null 2>&1")) {
    print "ERROR: Unable to find plotting program $ploticus.\n";
    print "       Cannot do any plots at current time. \n";
    return 0;
  }
  $self->{ploticus} = $ploticus;

  # Initialize the base data members
  return $self;
}

sub _check_output {
  my $self = shift;
  my $info = $self->{info};
  my $options = $self->{valid_options};
  
  foreach my $out (@{$options->{output}}) {
    if ($info->output eq $out) {
      if ( $out eq "x11" ) {
        return 1;
      }
      if ($info->output_fl ne "") {
        return 1;
      }
      else {
        print "ERROR: You must specify an output file. \n";
        $info->output("x11");
        $info->output_fl();
        return 1;
      }
    }
  }
  
  print "ERROR: Invalid plot output ", $info->output, "\n";
  $self->_dump_output_options();

  $info->output("x11");
  $info->output_fl();

  return 0;
  
}

sub _check_legendloc {
  my $self = shift;
  my $info = $self->{info};
  my $options = $self->{valid_options};
  
  foreach my $loc (@{$options->{lloc}}) {
    my $abbr = $options->{lloc_abbr}->{$loc};
    if ($info->legendloc eq $loc || $info->legendloc eq $abbr) {
      return 1;
    }
  }
  
  print "ERROR: Unknown legend location ", $info->legendloc, "\n";
  $self->_dump_legenloc_options();
  return 0;
}

sub _check_width {
  my $self = shift;
  my $info = $self->{info};
  
  if ($info->width > 0) {
    return 1;
  }
  print "ERROR: Width must be greater than 0.\n";
  return 0;
}

sub _check_height {
  my $self = shift;
  my $info = $self->{info};

  if ($info->height > 0){
    return 1;
  }
  print "ERROR: Height must be greater than 0.\n";
  return 0;
}

sub _check_type {
  my $self = shift;
  my $info = $self->{info};
  my $options = $self->{valid_options};

  # Does type exist? 
  if ( ! defined ($info->type)) {
    print "ERROR: Plot type not specified.\n";
    $self->_dump_type_options();
    return 0;
  }
  
  # first determine if they've selected appropriate type.
  my $valid_dim = 0;
  foreach my $type (@{$options->{type}}) {
    if ($info->type eq $type) {
      return 1;
    }
    elsif ( defined ($options->{type_abbr}) ) {
      if ($info->type eq $options->{type_abbr}->{$type}) {
        return 1;
      }
    }
  }
  
  print "ERROR: Unknown plot type ", $info->type, "\n";
  $self->_dump_type_options();
  return 0;
}

sub _clear_groups {
  my $self = shift;
  my $groups = $self->{groups};
  my $gen_str = $self->{gen_str};

  foreach my $key (keys %{$groups}) {
    print "WARNING: Deleting group $key.\n";
  }

  undef ($groups);
#  $groups = {};

  # Clear any generated name information. 
  undef ($gen_str);

  return $self;
}

sub _clear_mappings {
  my $self = shift;
  my $info = $self->{info};
  my $map_base = $self->{plot_2_data};
  my $inv_map = $self->{data_2_plot};

  print "WARNING: Clearing all previous plot dimension to data dimension mapping.\n";
  my $map;
  
  foreach my $map_key (keys %{$map_base}) {
    foreach my $key ( keys %{$map_base->{$map_key}} ) {
      undef ($map_base->{$map_key}{$key}{dim});
      undef ($map_base->{$map_key}{$key}{labels});
    }
  }
  
  foreach my $key (keys %{$inv_map}) {
    undef ($self->{data_2_plot});
    $self->{data_2_plot} = {};
  }
  
  $info->type("");

  return $self;
}

########################################################################
# Purpose: creates a synthetic data dimension. 
#
# Restrictions: Group must contain more than one value.
########################################################################


sub _group_dim {
  my $self = shift;
  my $name = shift;
  my $dimensions = shift;
  my $type = shift;
  my @values = @_;
  my @elements;
  my @all_elems;

  (defined $dimensions->{$type}) || die "Non-existent data dimension $type.\n";
  @elements = Asim::Util::expand_regexp($dimensions->{$type}, @values);

  #
  # Now take this through the _set_group command. 
  $self->_set_group($name, @elements);
}      


sub _set_group {
  my $self = shift;
  my $name = shift;
  my @values = @_;
  my $groups = $self->{groups};
  
  if ( ! defined $name ) {
    print "ERROR: No group name specified.\n";
    return 1;
  }
  
  if ( (scalar(@values) < 1) ) {
    print "ERROR: You must specify at least 2 or more values for group.\n";
    return 1;
  }
  
  if ( ! defined($groups->{$name}) ) {
    print "Setting group $name.\n";
  }
  else {
    print "WARNING: Group $name already exists.\n";
    print "WARNING: Overwrite old values.\n";
    if ( defined ($groups->{$name}{data_dim}) ) {
      print "WARNING: Group $name is used to define $groups->{$name}{data_dim}.\n";
      print "WARNING: You must rerun 'set plot type' before plotting data.\n";
      undef $groups->{$name}{data_dim};
    }
  }

  @{$groups->{$name}{labels}} = @values;
  return 1;
}

# 
# Inputs:
# - Name of data dimension which we're going to undefine. 
# - Legal dimension names
#
sub _undefine_synthetic_name {
  my $self = shift;
  my $d_dim = shift;
  my $avail_dim = shift;
  my $gen_str = $self->{gen_str};
  my $groups = $self->{groups};
  my @avail_data_dim = keys %$avail_dim;

  #  Make sure the data dimension specified is a legal one. 
  my $check = grep (/^$d_dim$/, @avail_data_dim);
  
  if ($check != 1) {
    print "ERROR: Specified dimension $d_dim is not a legal data dimension.\n";
    return 1;
  }
  
  if ( ! defined ($gen_str->{$d_dim}) ) {
    print "ERROR: Data dimension $d_dim is not defined, so nothing to undefine. \n";
    return 1;
  }
  
  # Undo the mapping from group to data dimension. 
  my @inp = $gen_str->{$d_dim};
  
  foreach my $element (@inp) {
    if ($element =~ /group\[\s*(\S+)\s*\]/) {
      #
      # Erase mapping of group to data dimension. 
      #
      undef ($groups->{$1}{"data_dim"});
    }
  }

  delete ($gen_str->{$d_dim});

  return $self;
}


#####################################################################################
#
# Inputs: 
# - Data dimension we're synthesizing a name for 
# - Information on how to generate name:
#   Name generation syntax: A space separated list where each element in list is a 
#                           group specified as "group[<name>] or a string.
#                           group A = "foo1, foo2"
#                           group B = "bar1, bar2"
#                           EX: group[A] "_" group[B]
#                           This produces names: foo1_bar1, foo1_bar2, foo2_bar1
#                                                foo2_bar2
#
#####################################################################################
sub _define_synthetic_name {
  my $self = shift;
  my $d_dim = shift;
  my $avail_dim = shift;
  my @inp = @_;
  my $gen_str = $self->{gen_str};
  my $groups = $self->{groups};
  my @avail_data_dim = keys %$avail_dim;

  # Make sure the data dimension specified is a legal one. 
  my $check = grep (/^$d_dim$/, @avail_data_dim);
  
  if ($check != 1) {
    print "ERROR: Specified dimension $d_dim is not a legal data dimension.\n";
    return 1;
  }
  
  # Check the user inputs. 
  # Make sure any group specified is a valid group.
  my $search_str="";
  my $num_group_elems = 1;

  foreach my $element (@inp) {
    if ($element =~ /group\[\s*(\S+)\s*\]/) {
      #
      # We've found a group.  Check whether this is a valid group.
      #
      (defined ($groups->{$1})) || die "ERROR: Group $1 is not a valid group. \n";

      
      # Make sure that the group has not already been assigned a data dimension.
      if ( defined ($groups->{$1}{"data_dim"}) ) {
        print "WARNING: Group $1 previously assigned to data dimension $groups->{$1}{data_dim}\n";
        print "WARNING: Overwriting previous assignment. \n";
      }

      # Indicate that this group maps data dimension $d_dim. 
      $groups->{$1}{"data_dim"} = $d_dim;

      # Compute number of elements in group. 
      $num_group_elems *= scalar(@{$groups->{$1}{"labels"}});
      
      # Create search string out of the values in the group.
      my $str = join ("|", @{$groups->{$1}{"labels"}});
      $search_str = "$search_str" . "($str)";
    }
    else {
      $search_str = "$search_str" . "$element";
    }
  }

  # Indicate the string to be used to generate the name
  @{$gen_str->{$d_dim}} = @inp;

  # Get all elements  in the synthetic dimension.
  my @synth_elems = grep (/^$search_str$/, @{$avail_dim->{$d_dim}});

  #
  # Make sure that the number of synthetic elements = Product of number of 
  # entries in each group assigned to synthetic element.  If there are no
  # groups, then the value must be 1. 
  if (scalar(@synth_elems) != $num_group_elems) {
    print "ERROR: Number of entries in synthetic dimension (";
    print scalar(@synth_elems);
    print ")\n";
    print "       must equal the product of all elements in group ($num_group_elems)\n";
    die "Not forming legal names for dimension $d_dim.\n";
  }
  
  return @synth_elems;
}
###########################################################################

#= item $plotstats-E<gt>_check_plot()
#
#Makes sure that the plotting routine can plot correctly given the number 
#of entries in each plotting dimension.  There is no HARD limit on these
#things.  However, it doesn't make for nice charts and the data is unreadable.
#
#=cut

###########################################################################

sub _check_plot {
  my $self = shift;
  my $info = $self->{info};
  my $map_base = $self->{plot_2_data};
  my $map = $map_base->{$info->type};

  #
  # Checking bar plots.
  #
  if ($info->type eq "bar") {
    my $num_bar = scalar @{$map->{bar}{labels}};
    my $num_cluster = scalar @{$map->{cluster}{labels}};
    my $num_stack = scalar @{$map->{stack}{labels}};
    
    # We can't do more than 25 bars in a non-clustered bar chart.
    if ($num_cluster == 1) {
      if ($num_bar > 40) {
        print "WARNING: Number of bars ($num_bar) exceeds recommended limit of 40. \n";
        return 0;
      }
    }
    #
    # In a clustered bar chart, the number of clusters * bars can't exceed 100.
    #
    else {
      if ( ($num_bar * $num_cluster) > 100 ) {
        print "WARNING: Number of clusters * number of bars ($num_cluster*$num_bar) exceeds 100.\n";
        return 0;
      }
    }
    
    # 
    # We really can't see anything once we exceed about 15 stack entries.  
    # Currently setting a higher limit of 25.
    #
    if ($num_stack > 50) {
      print "WARNING: Number of stacked entries ($num_stack) exceeds 50.\n";
      return 0;
    }
  }

  #
  # Checking box plots.
  #
  if ($info->type eq "box") {
    my $num_bar = scalar @{$map->{bar}{labels}};
    my $num_cluster = scalar @{$map->{cluster}{labels}};
    my $num_box = scalar @{$map->{box}{labels}};

    # We can't do more than 16 bars in a non-clustered bar chart.
    if ($num_cluster == 1) {
      if ($num_bar > 16) {
        print "WARNING: Number of bars ($num_cluster) exceeds limit of 30. \n";
        return 0;
      }
    }
    #
    # In a clustered box chart, the number of clusters * bars can't exceed 30
    #
    else {
      if ( ($num_bar * $num_cluster) > 60 ) {
        print "WARNING: Number of clusters * number of bars ($num_cluster*$num_bar) exceeds 60.\n";
        return 0;
      }
    }
  }  

  #
  # Checking line plots.
  #
  if ($info->type eq "line") {
    my $num_xaxis = scalar @{$map->{xaxis}{labels}};
    my $num_line = scalar @{$map->{line}{labels}};

    if ($num_line > 25) {
      print "WARNING: Number of lines ($num_line) exceeds limit of 25.\n";
      return 0;
    }
  }
  return 1;
}
  
sub _set_plot_type {
  my $self = shift;
  my $data_dim = shift;
  my $pinfo = shift;
  my $plot_type = shift;
  my @inp = @_;
  my $info;
  my $map_base;
  my $map;
  my $inv_map;
  my $groups;
  my $gen_str;
  my $key;
  my $value;
  my @avail_data_dim;
  
  if (! defined ($plot_type)) {
    print "ERROR: No Plot type specifed.\n";
    return 1;
  }

  # Clear any existing mappings of plot dimensions to data dimensions
  # for this plot type.
  $self->_clear_mappings();

  #
  # Set and check type of plot
  #
  $info = $self->{info};
  $info->type($plot_type);
  if (! $self->_check_type) {
    print "ERROR: Must set plot type.\n";
    return 1;
  }
 
  #
  # Initialize mapping information. 
  #
  $map_base = $self->{plot_2_data};
  $inv_map = $self->{data_2_plot};
  $groups = $self->{groups};
  $gen_str = $self->{gen_str};
  @avail_data_dim = keys %$data_dim;
  push (@avail_data_dim, "rows");
  push (@avail_data_dim, "cols");

  # Get the mapping data structure
  $map = $map_base->{$info->type};

  # Determine the dimensions available
  my @avail_plot_dim = keys %$map;
 
  #
  # Remove any spaces around the '=' sign in user inputs
  #
  my $inp = join (" ", @inp);
  $inp =~ s/\s*(=)\s*/$1/g;
  @inp = split (" ", $inp);

  #
  # Map the given dimensions. 
  #
  foreach my $inp_pair (@inp) {
    ($key, $value) = split /=/, $inp_pair;

    if ( defined ($map->{$key}) ) {
      if ( defined ($map->{$key}{dim}) ) {
        print "ERROR: Plotting dimension $key has already been mapped.\n";
        $self->_clear_mappings;
        return 1;
      }
      $map->{$key}{dim} = $value;
      @avail_plot_dim = grep (!/^$key$/, @avail_plot_dim);

      if ( defined ($data_dim->{$value}) ) {
        my $avail = grep (/^$value$/, @avail_data_dim);
        if ($avail == 1) {

          #
          # Make sure this data dimension is not mapped to a 
          # synthetic dimension. If it is, then you HAVE to use 
          # the synthetic dimension.
          #

          if ( defined($gen_str->{$value}) ) {
            print "ERROR: $value is defined to be ", join (" ", $gen_str->{$value}), "\n";
            print "       Therefore, it cannot be mapped directly to a plotting dimension. \n";
            print "       You can only map the groups that define the data dimension.\n";
            $self->_clear_mappings;
            return 1;
          }
          else {
            # if this is params, experiments or benchmarks....
            @{$map->{$key}{labels}} = @{$data_dim->{$value}};
            $inv_map->{$value} = $key;
            @avail_data_dim = grep (!/^$value$/, @avail_data_dim);
          }
        }
        else {
          print "ERROR: Data dimension $value has already been mapped.\n";
          print "ERROR: The mapping could have happened either directly,\n";
          print "       or through a group which used the data dimension.\n";
          $self->_clear_mappings;
          return 1;
        }
      }
      elsif ( defined ($groups->{$value}) ) {
        #
	# We have a case where the plot dimension is being mapped
	# to an artificial data dimension also known as a group.
        #
	@{$map->{$key}{labels}} = @{$groups->{$value}{labels}};
        $inv_map->{$value} = $key;

	#
	# Remove the data dimension associated with this group
        # from list of available dimensions. Note that multiple
	# groups can be created from the same data dimension.  
	# Therefore, the data dimension might not be in the list
	# of available dimensions. This is legal. 
	#
	if ( defined($groups->{$value}{data_dim}) ) {
          my $d_dim = $groups->{$value}{data_dim};
          @avail_data_dim = grep (!/^$d_dim$/, @avail_data_dim);
	}
	else {
          # The mapping from group to data dimension has not 
          # been specified by user.  Therefore, exit. 
          print "ERROR: Don't know which data dimension group $value belongs to.\n";
          print "ERROR: Specify data to group mapping using 'restrict <data dim> <group[<..>]|string>\n'";
          print "       EXAMPLE: restrict experiments group[A] \"_\" group[B]\n";
          print "                In this case, A & B are groups previously specified, and\n";
          print "                experiment name is an element in group A followed by an '_'\n";
          print "                followed by an element in group B for all elements in groups A and B.\n";
          $self->_clear_mappings;
          return 1;
	}
      }
      else {
        #
        # We could have a case where we have the rows or cols or 
        # a parameter. The syntax for this is <param name>!rows 
        # or <param name>!cols
        my ($param, $pdim);
        ($param, $pdim) = split (/!/, $value);
        if (defined $pinfo->{$param}) {
          if ( $pdim eq "rows" ) {
            @{$map->{$key}{labels}} = @{$pinfo->{$param}{rlabels}};
          }
          elsif ( $pdim eq "cols" ) {
            @{$map->{$key}{labels}} = @{$pinfo->{$param}{clabels}};
          }
          else {
            print "ERROR: Must specify either 'rows' or 'cols' for parameter dimension.\n";
            $self->_clear_mappings;
            return 1;
          }
          $inv_map->{$pdim} = $key;
          @avail_data_dim = grep (!/^$pdim$/, @avail_data_dim);
        }
        else {
          print "ERROR: Unknown data dimension $value.\n";
          $self->_clear_mappings;
          return 1;
        }
      }
    }
    else {
      print "ERROR: No plot dimension named $key available.\n";
      $self->_clear_mappings;
      return 1;
    }
  }

  #
  # Make sure every data dimension is mapped.  If it isn't, then
  # it must only have one value.  This loop only checks params, exps, 
  # and benchmarks.  It doesn't check rows and cols.  This is because 
  # we have to first determine name of param before we can determine 
  # the number of rows and cols for params.  The param, exp, and benchmark
  # name, size of params, and number of rows and cols are all checked in 
  # routine _check_plot_inputs.
  #
  foreach my $dim (@avail_data_dim) {
    if (! defined ($inv_map->{$dim})) {
      # This particular data dimensions is not mapped, so make sure there 
      # is only one element in this dimension. 
      if (defined $data_dim->{$dim}) {
        my $num_entries = scalar (@{$data_dim->{$dim}});
        
        if ($num_entries > 1) {
          #
          # We have more than one entry for this data dimension.  See if there is 
          # a mapping for this dimension.  
          # Ex: params is set to FOO, and BAR.  To limit params to only FOO, 
          # restrict params FOO
          # Now params is limited to one value... FOO
          #
          if ( defined($gen_str->{$dim}) ) {
            # 
            # The data dimension name is synthesized. 
            #
            my $syn_name = "";
            foreach my $element (@{$gen_str->{$dim}}) {
              if ($element =~ /group\[\s*(\S+)\s*\]/) {
                #
                # By defintion, if we map to a group, then the group has more than 
                # one element in it. 
                #
                print "ERROR: Data dimension $dim is defined to consist of group \n";
                print "       $1.  Group $1 contains more than one value but it \n";
                print "       doesn't map to any plotting dimension. \n";
                print "\n";
                $self->_clear_mappings();
                return 1;
              }
              $syn_name = $syn_name . "$element";
            }
            # 
            # Check that the generated name is a legal name for this data dimension. 
            #
            my $check = grep (/^$syn_name$/, @{$data_dim->{$dim}});
            if ($check != 1) {
              print "ERROR: Synthesized name $syn_name for data dimension $dim is not\n";
              print "       a legal name.\n";
              print "       Legal values are: ", join (", ", @{$data_dim->{$dim}}), "\n";
              $self->_clear_mappings();
              return 1;
            }
          }
          else {
            print "ERROR: Need a mapping for data dimension $dim.\n";
            $self->_clear_mappings();
            return 1;
          }
        }
        elsif ($num_entries == 0) {
          print "ERROR: No entries for data dimension $dim.\n";
          $self->_clear_mappings;
          return 1;
        }
        else {
          # There is only one entry for this dimension, so put the entry in 
          # the title. TO BE DONE.  
          # Also, set the value of dimension to be a constant.
        }
      }
    } 
  }
  
  # Set all plotting dimensions that are not mapped to constant value.
  foreach my $dim (@avail_plot_dim) {
    # Certain plot dimensions MUST BE MAPPED.  
    if ($info->type eq "bar" && $dim eq "bar") {
      print "ERROR: You must map plotting dimension 'bar'.\n";
      $self->_clear_mappings;
      return 1;
    }
    elsif ($info->type eq "box" && $dim eq "box") {
      print "ERROR: You must map plotting dimension 'box'.\n";
      $self->_clear_mappings;
      return 1;
    }
    elsif ($info->type eq "box" && $dim eq "bar") {
      print "ERROR: You must map plotting dimension 'bar'.\n";
      $self->_clear_mappings;
      return 1;
    }
    elsif ($info->type eq "line" && $dim eq "xaxis") {
      print "ERROR: You must map plotting dimension 'xaxis'.\n";
      $self->_clear_mappings;
      return 1;
    }
    @{$map->{$dim}{labels}} = ("dummy");
  }

  # This routine checks that the params, exps, and bms specified are all legal,
  # and it also checks that the number of rows and cols is correct.
  if (! $self->_check_plot_inputs($data_dim, $pinfo)) {
    $self->_clear_mappings;
    return 1;
  }

  #
  # Make sure we have a reasonable number of entries for each type of plot. 
  # For now, I don't exit, but just print a warning. 
  #
  if (! $self->_check_plot()) {
    print "**************************************************************\n";
    print "*****YOU MAY GET UNREADEABLE GRAPHS OR PLOTICUS MAY CRASH*****\n";
    print "**************************************************************\n";
#    $self->_clear_mappings;
#    return 1;
  }

  return $self;
}

#
# Inputs are:
# 0. Data dimension for which we're trying to produce a names
# 1. Hash with keys being plot objects (cluster, bar, stack, box, x-axis, 
#    line, etc), and values being the current object being mapped to key.
# 2. Names of all objects in this data dimension. 
#
# Output:
#    Name of experiment. 
#
# Algorithm: The name could be produced in one of three ways. 
# 1.) The data dimension may never have been mapped to a plotting dimension. 
#     Therefore, it's a constant, and the routine return one value. 
# 2.) The plotting dimension is mapped directly to a data dimension. Therefore, 
#     the routine returns the value associated with the plotting dimension 
#     to which the data dimension is mapped. 
# 3.) The data dimension in question is not mapped directly to any one plotting
#     dimension, and there are more than one value in the data dimension. 
#     Therefore, we need to generate the name of the data dimension using a 
#     syntax provided in the generation string.  
# 

sub _get_dim_name {
  my $self = shift;
  my $data_dim = shift;
  my $plot_label = shift;
  my $data_names = shift;
  
  my $info = $self->{info};
  my $map_base = $self->{plot_2_data};
  my $map = $map_base->{$info->type};
  my $inv_map = $self->{data_2_plot};
  my $gen_str = $self->{gen_str};
  
  # Do we have a direct mapping from plotting dimension to data dimension? 
  if ( ! defined ($inv_map->{$data_dim}) ) {
    # Else, do we have to generate the data dimension label? 
    if ( ! defined ($gen_str->{$data_dim}) ) {
      # This data dimension does not map onto any plotting dimension.
      # Therefore, there must be only one value for this data dimension. 
      if ( scalar(@{$data_names}) != 1 ) {
        print "ERROR: $data_dim has more than one value.\n";
        return "";
      }
      return $$data_names[0];
    }
    else {
      # We have to generate the name of the data dimension label.
      my $label = "";
      foreach my $element ( @{$gen_str->{$data_dim}} ) {
        if ( "$element" =~ /group\[(\S+)\]/ ) {
          # We have a group.  Determine what that group maps onto. 
          if ( defined $inv_map->{$1} ) {
            $label = $label . "$plot_label->{$inv_map->{$1}}";
          }
          else {
            #
            # There is no mapping for this group.  That means that the value in group
            # is a constant. 
            #
            print "ERROR: Group $1 is contained in string used to generate $data_dim names.\n";
            print "ERROR: However, Group $1 doesn't map onto any plotting dimension.\n";
            return "";
          }
        }
        # If the syntax is not group[...], then it's a string. 
        else {
          $label = $label . "$element";
        }
      }
      # 
      # Now that we've generated a label, check whether it is one of the legal
      # names associated with the data dimension. 
      my $exists = grep (/^$label$/, @{$data_names});
      if ($exists == 1) {
        return $label;
      }
      elsif ($exists == 0) {
        print "ERROR: Unable to find generated name $label in data dimension $data_dim.\n";
        return "";
      }
      else {
        print "ERROR: Found more than one entry named $label in data dimension $data_dim.\n";
      }
    }
  }
  # We have a direct mapping from plot dimension to data dimension. 
  else {
    return ( $plot_label->{$inv_map->{$data_dim}} );
  }
}

# Input to method is a reference to an array which contains the valid dimensions.
# If we're setting the available data dimensions, then we need to clear all 
# plotting options we've set in the past. 
sub _set_avail_dimensions {
  my $self = shift;
  my $dim_ptr = shift;
  my $info = $self->{info};
  $info->dimensions($dim_ptr);
}

#
# This subroutine basically duplicates much of the stuff in plot_data.  It is used
# to generate names of exps, params, and benchmarks, and checks that the names are 
# valid.  It is also used to make sure that all parameters given have the same number
# of rows and cols.  
#
sub _check_plot_inputs {
  my $self = shift;
  my $data_dim = shift;
  my $pinfo = shift;
  my $info = $self->{info};
  my $map_base = $self->{plot_2_data};
  my $inv_map = $self->{data_2_plot};
  my $data_fl = "$TMP_DIR/data$$";
  my $scripts = $self->{scripts};
  my @params_plotted;

  my $map = $map_base->{$info->type};
  if ($info->type eq "bar") {
    my $stack_elem;   
    my $first_elem;
    my $second_elem;

    #
    # If clusters is specifed, then we need to do cluster, then bars, then stack.
    # Else, we do bars, then cluster, then stack
    #
    my $first_dim = "cluster";
    my $second_dim = "bar";
    if (@{$map->{cluster}{labels}}[0] eq "dummy") {
      $first_dim = "bar";
      $second_dim = "cluster"
    }
    
    my $stackval = 0;

    foreach $first_elem ( @{$map->{$first_dim}{labels}} ) {
      foreach $second_elem ( @{$map->{$second_dim}{labels}} ) {
        foreach $stack_elem ( @{$map->{"stack"}{labels}} ) {
          
          # Determine the name of exp, bm, and param based on the mapping
          # Do the mapping
          my %plot_names = (
                            stack => $stack_elem,
                            $first_dim => $first_elem,
                            $second_dim => $second_elem,
                           );

          my $exp_name = $self->_get_dim_name("experiments", \%plot_names, 
                                              $data_dim->{"experiments"});
          my $bm_name = $self->_get_dim_name("benchmarks", \%plot_names, 
                                             $data_dim->{"benchmarks"});
          my $param_name = $self->_get_dim_name("params", \%plot_names, 
                                                $data_dim->{"params"});

	  if ($exp_name eq "" || $bm_name eq "" || $param_name eq "") {
            print "ERROR: Can't determine data dimension name.\n";
            return 0;
	  }
          my $entry = grep (/^$param_name$/, @params_plotted);
          if ($entry == 0) {
            push (@params_plotted, $param_name);
          }
        }
      }
    }
  }
  elsif ($info->type eq "box") {
    my $bar_elem;
    my $cluster_elem;
    my $box_elem;   # Stack or box elem.

    #
    # Doing a box plot
    #
    foreach $box_elem ( @{$map->{"box"}{labels}} ) {
      foreach $cluster_elem ( @{$map->{"cluster"}{labels}} ) {
        foreach $bar_elem ( @{$map->{"bar"}{labels}} ) {

	  # Determine the name of exp, bm, and param based on the mapping
	  # Do the mapping
          my %plot_names = (
                            box => $box_elem,
                            bar => $bar_elem,
                            cluster => $cluster_elem,
                           );
	  my $exp_name = $self->_get_dim_name("experiments", \%plot_names, 
                                       $data_dim->{"experiments"});
	  my $bm_name = $self->_get_dim_name("benchmarks", \%plot_names, 
                                       $data_dim->{"benchmarks"});
	  my $param_name = $self->_get_dim_name("params", \%plot_names, 
                                       $data_dim->{"params"});

	  if ($exp_name eq "" || $bm_name eq "" || $param_name eq "") {
            print "ERROR: Can't determine data dimension name.\n";
            return 0;
	  }

          my $entry = grep (/^$param_name$/, @params_plotted);
          if ($entry == 0) {
            push (@params_plotted, $param_name);
          }
        }
      }
    }
  }
  elsif ($info->type eq "line") {
    #
    # Doing a line plot
    #

    my $line_elem;
    my $x_elem;
    foreach $x_elem ( @{$map->{"xaxis"}{"labels"}} ) {
      foreach $line_elem ( @{$map->{"line"}{"labels"}} ) {
        # Determine the name of exp, bm, and param based on the mapping
        # Do the mapping
        my %plot_names = (
                          line => $line_elem,
                          xaxis => $x_elem,
                         );
        my $exp_name = $self->_get_dim_name("experiments", \%plot_names, 
                                            $data_dim->{"experiments"});
        my $bm_name = $self->_get_dim_name("benchmarks", \%plot_names, 
                                           $data_dim->{"benchmarks"});
        my $param_name = $self->_get_dim_name("params", \%plot_names, 
                                              $data_dim->{"params"});
        
        
        if ($exp_name eq "" || $bm_name eq "" || $param_name eq "") {
          print "ERROR: Can't determine data dimension name.\n";
          return 0;
        }
        
        my $entry = grep (/^$param_name$/, @params_plotted);
        if ($entry == 0) {
          push (@params_plotted, $param_name);
        }
      }
    }
  }

  # Now that we have the list of params to plot, make sure that they all have
  # the same number of rows and cols. 
  my $num_params = scalar(@params_plotted);

  #
  # If there's only one param, than it doesn't matter what size it is. 
  if ($num_params == 1) {
    return 1;
  }
    
  #
  # There are more than one param.  Therefore, all params must be the same size, and the
  # labels must be the same.
  my $nrows = 0;
  my $ncols = 0;
  my $rlabels;
  my $clabels;

  #
  # If rows and cols are already mapped to plotting dimensions, then set the starting
  # value to the numbers and labels already specified. Otherwise, set the starting
  # value to the first param in list of params. 
  #
  if (defined $inv_map->{"rows"}) {
    $nrows = scalar(@{$map->{$inv_map->{"rows"}}{labels}});
    $rlabels = join (",", @{$map->{$inv_map->{"rows"}}{labels}});
  }
  else {
    # The number of rows and row labels is the same as the first param.
    $nrows = $pinfo->{$params_plotted[0]}{nrows};
    $rlabels = join(",", @{$pinfo->{$params_plotted[0]}{rlabels}});
  }
  if (defined $inv_map->{"cols"}) {
    $ncols = scalar(@{$map->{$inv_map->{"cols"}}{labels}});
    $clabels = join (",", @{$map->{$inv_map->{"cols"}}{labels}});
  }
  else {
    # The number of rows and row labels is the same as the first param.
    $ncols = $pinfo->{$params_plotted[0]}{ncols};
    $clabels = join(",", @{$pinfo->{$params_plotted[0]}{clabels}});
  }
  
  #
  # Walk through each parameter and make sure the number and name of each label 
  # matches each other and matches any values mapped to plotting dimensions. 
  foreach my $param (@params_plotted) {
    $nrows == $pinfo->{$param}{nrows} || die "Number of rows don't match between params.";
    $ncols == $pinfo->{$param}{ncols} || die "Number of cols don't match between params.";
    my $clab = join(",", @{$pinfo->{$param}{clabels}});
    my $rlab = join(",", @{$pinfo->{$param}{rlabels}});
    ("$rlabels" eq "$rlab") || die "Row labels do not match between parameters.";
    ("$clabels" eq "$clab") || die "Col labels do not match between parameters.";
  }
  #
  # If the number of rows and/or cols is greater than one, then rows and cols must be 
  # mapped to a plotting dimension. 
  if ($nrows > 1 && !defined($inv_map->{"rows"})) {
    print "ERROR: Number of rows > 1 for given parameter(s), but rows not mapped to any plotting dimension. \n";
    return 0;
  }
  
  if ($ncols > 1 && !defined($inv_map->{"cols"})) {
    print "ERROR: Number of cols > 1 for given parameter(s), but cols not mapped to any plotting dimension. \n";
    return 0;
  }
  return 1;
}

  

#
# PLOT THE DATA
#
sub _plot_data {
  my $self = shift;
  my $data_dim = shift;
  my $data_array = shift;
  my $pinfo = shift;
  my $sort_type = shift;
  my $sort_name = shift;
  my $info = $self->{info};
  my $map_base = $self->{plot_2_data};
  my $inv_map = $self->{data_2_plot};
  my $data_fl = "$TMP_DIR/data$$";
  my $scripts = $self->{scripts};

  if ($info->type eq "") {
    print "ERROR: No dimensions have been mapped.  Run set plot type first.\n";
    return 1;
  }

  my $map = $map_base->{$info->type};

  # Open data file
  open (DATA_FL, ">$data_fl") || die "Unable to open file $data_fl for plotting data.\n";
  
  
  #
  # Walk through each plot dimension in the order required.  
  # The order required is a function of the type of graph required. 
  #
  # If we're doing a non-box bar graph (stacked and/or clustered), 
  # then we do cluster, cluster, then stack
  #
  # If we're doing a box plot, then we do box, cluster, then bar
  #
  
  my $maxval = 0;
  my $minval = 1e35;
  if ($info->type eq "bar") {
    my $stack_elem;   
    my $cluster_elem;
    my $bar_elem;

    my $stackval = 0;

    foreach $cluster_elem ( @{$map->{"cluster"}{labels}} ) {
      printf DATA_FL "$cluster_elem ";
      foreach $bar_elem ( @{$map->{"bar"}{labels}} ) {
        foreach $stack_elem ( @{$map->{"stack"}{labels}} ) {
          
          # Determine the name of exp, bm, and param based on the mapping
          # Do the mapping
          my %plot_names = (
                            stack => $stack_elem,
                            bar => $bar_elem,
                            cluster => $cluster_elem,
                           );

          my $exp_name = $self->_get_dim_name("experiments", \%plot_names, 
                                              $data_dim->{"experiments"});
          my $bm_name = $self->_get_dim_name("benchmarks", \%plot_names, 
                                             $data_dim->{"benchmarks"});
          my $param_name = $self->_get_dim_name("params", \%plot_names, 
                                                $data_dim->{"params"});
          my $row_name = $self->_get_dim_name("rows", \%plot_names, 
                                              $pinfo->{$param_name}{rlabels});
          my $col_name = $self->_get_dim_name("cols", \%plot_names, 
                                              $pinfo->{$param_name}{clabels});

	  if ($exp_name eq "" || $bm_name eq "" || $param_name eq "" ||
              $row_name eq "" || $col_name eq "") {
	      close DATA_FL;
	      return 1;
	  }
          
          my $val = $$data_array{$exp_name}{$bm_name}{$param_name}{$row_name}{$col_name};
          if (! defined ($val)) {
            print "ERROR: Something is wrong.  We don't have a value.\n";
            print "ERROR: This will result in incorrect data being plotted. \n";
            print "       Exp: $exp_name, BM: $bm_name, PAR: $param_name, ROW: $row_name, COL: $col_name.\n";
            close DATA_FL;
            return 1;
          }
            
          $stackval = $stackval + $val;
          printf DATA_FL "$val ";
          
        }
        if ($maxval < $stackval) {
          $maxval = $stackval;
        }
        if ($minval > $stackval) {
          $minval = $stackval;
        }
        $stackval = 0;
      }
      printf DATA_FL "\n";
    }
  }
  elsif ($info->type eq "box") {
    my $bar_elem;
    my $cluster_elem;
    my $box_elem;   # Stack or box elem.

    #
    # Doing a box plot
    #
    foreach $box_elem ( @{$map->{"box"}{labels}} ) {
      printf DATA_FL "$box_elem ";
      foreach $cluster_elem ( @{$map->{"cluster"}{labels}} ) {
        foreach $bar_elem ( @{$map->{"bar"}{labels}} ) {

	  # Determine the name of exp, bm, and param based on the mapping
	  # Do the mapping
          my %plot_names = (
                            box => $box_elem,
                            bar => $bar_elem,
                            cluster => $cluster_elem,
                           );
	  my $exp_name = $self->_get_dim_name("experiments", \%plot_names, 
                                       $data_dim->{"experiments"});
	  my $bm_name = $self->_get_dim_name("benchmarks", \%plot_names, 
                                       $data_dim->{"benchmarks"});
	  my $param_name = $self->_get_dim_name("params", \%plot_names, 
                                       $data_dim->{"params"});
          my $row_name = $self->_get_dim_name("rows", \%plot_names, 
                                              $pinfo->{$param_name}{rlabels});
          my $col_name = $self->_get_dim_name("cols", \%plot_names, 
                                              $pinfo->{$param_name}{clabels});

	  if ($exp_name eq "" || $bm_name eq "" || $param_name eq "" ||
              $row_name eq "" || $col_name eq "") {
	      close DATA_FL;
	      return 1;
	  }
          
          my $val = $$data_array{$exp_name}{$bm_name}{$param_name}{$row_name}{$col_name};
          if (! defined ($val)) {
            print "ERROR: Something is wrong.  We don't have a value.\n";
            print "ERROR: This will result in incorrect data being plotted. \n";
            print "       Exp: $exp_name, BM: $bm_name, PAR: $param_name, ROW: $row_name, COL: $col_name.\n";
            close DATA_FL;
            return 1;
          }
          
          printf DATA_FL "$val ";
          if ($maxval < ($val+0)) {
            $maxval = $val;
          }
          if ($minval > ($val+0)) {
            $minval = $val;
          }
        }
      }
      printf DATA_FL "\n";
    }
  }
  elsif ($info->type eq "line") {
    #
    # Doing a line plot
    #

    my $line_elem;
    my $x_elem;
    foreach $x_elem ( @{$map->{"xaxis"}{"labels"}} ) {
      printf DATA_FL "\"$x_elem\" ";
      foreach $line_elem ( @{$map->{"line"}{"labels"}} ) {
        # Determine the name of exp, bm, and param based on the mapping
        # Do the mapping
        my %plot_names = (
                          line => $line_elem,
                          xaxis => $x_elem,
                         );
        my $exp_name = $self->_get_dim_name("experiments", \%plot_names, 
                                            $data_dim->{"experiments"});
        my $bm_name = $self->_get_dim_name("benchmarks", \%plot_names, 
                                           $data_dim->{"benchmarks"});
        my $param_name = $self->_get_dim_name("params", \%plot_names, 
                                              $data_dim->{"params"});
        my $row_name = $self->_get_dim_name("rows", \%plot_names, 
                                            $pinfo->{$param_name}{rlabels});
        my $col_name = $self->_get_dim_name("cols", \%plot_names, 
                                            $pinfo->{$param_name}{clabels});
        
        
        if ($exp_name eq "" || $bm_name eq "" || $param_name eq "" ||
            $row_name eq "" || $col_name eq "") {
          close DATA_FL;
          return 1;
        }
          
        my $val = $$data_array{$exp_name}{$bm_name}{$param_name}{$row_name}{$col_name};
        if (! defined ($val)) {
          print "ERROR: Something is wrong.  We don't have a value.\n";
          print "ERROR: This will result in incorrect data being plotted. \n";
          print "       Exp: $exp_name, BM: $bm_name, PAR: $param_name, ROW: $row_name, COL: $col_name.\n";
          close DATA_FL;
          return 1;
        }
        printf DATA_FL "$val ";
        if ($maxval < ($val+0)) {
          $maxval = $val;
        }
        if ($minval > ($val+0)) {
          $minval = $val;
        }
      }
      printf DATA_FL "\n";
    }
  }
  
  close DATA_FL;

  #
  # Generate inputs for ploticus script
  #
  my $legendloc=$info->legendloc;
  my $width = $info->width;
  my $height = $info->height;  
  my $title = $info->title;
  my $xlabel = $info->xlabel;
  my $ylabel = $info->ylabel;
  my $output = $info->output;
  my $output_fl = $info->output_fl;
  my $orientation = $info->orientation;
  #my $minval = 0;
  my $max_computed = 0;
  my $min_computed = 0;
  
  # If ymax is undefined by user, then use the calculated value.  Else, use the 
  # user specified value.
  if (defined $info->ymax && $info->ymax ne "") {
    $maxval = $info->ymax;
  }
  else {
    $max_computed = 1;
  }
  
  # Make sure ymax > ymin.  Otherwise, set ymin to be ymax - 1 or 0, 
  # whichever is larger. 
  if (defined $info->ymin && $info->ymin ne "") {
    $minval = $info->ymin;
  }
  else {
    $min_computed = 1;
  }

  if ($minval >= $maxval) {
    print "WARNING: ymin of $minval > ymax of $maxval.\n";
#    if (($maxval - 1) > 0) {
#      $minval = $maxval-1;
#    } else {
    $minval = 0;
#   }
    print "WARNING: Resetting ymin to be $minval.\n";
  }

  if ($info->type eq "bar") {
    my $num_bar;
    my $bar_fl = "$TMP_DIR/bar$$";

    if ($sort_type != 0) {
      print "WARNING: You can only sort data for line plots.  Ignoring sort command.\n";
    }

    open (BAR_FL, ">$bar_fl") || die "Unable to open file $bar_fl for plotting data.\n";

    $num_bar = scalar ( @{$map->{"bar"}{labels}} );
    for my $bar_label (@{$map->{"bar"}{labels}}) {
      print BAR_FL "$bar_label\n";
    }
    close BAR_FL;
    
    my $num_stack = scalar ( @{$map->{"stack"}{labels}} );
    my $stack_fl = "$TMP_DIR/stack$$";
    open (STACK_FL, ">$stack_fl") || die "Unable to open file $stack_fl for plotting data.\n";

    for my $stack_label (@{$map->{"stack"}{labels}}) {
      print STACK_FL "$stack_label\n";
    }
    close STACK_FL;

    system ("$self->{ploticus} $scripts->{bar} legloc=$legendloc data=$data_fl ymaxval=$maxval yminval=$minval num_bar=$num_bar num_stack=$num_stack bar_name=$bar_fl stack_name=$stack_fl width=$width height=$height xlabel=\"$xlabel\" ylabel=\"$ylabel\" title=\"$title\" -$output -o $output_fl -$orientation");

    # Cleanup
    system ("rm $bar_fl $stack_fl > /dev/null 2>&1");
  }

  elsif ($info->type eq "box") {
    my $num_cluster;
    my $num_bar;
    
    my $cluster_fl = "$TMP_DIR/cluster$$";
    my $bar_fl = "$TMP_DIR/bar$$";

    if ($sort_type != 0) {
      print "WARNING: You can only sort data for line plots.  Ignoring sort command.\n";
    }
    
    open (CLUSTER_FL, ">$cluster_fl") || die "Unable to open file $cluster_fl for plotting data.\n";
    open (BAR_FL, ">$bar_fl") || die "Unable to open file $bar_fl for plotting data.\n";

    $num_cluster = scalar ( @{$map->{"cluster"}{labels}} );
    $num_bar = scalar ( @{$map->{"bar"}{labels}} );

    #
    # The file with cluster names must contain one cluster name separated by blank lines equal 
    # to the number of bars per cluster. 
    # The file with bar names must N copies of all bar names, where N is the number of clusters.
    #
    for my $cluster_name ( @{$map->{"cluster"}{labels}} ) {
      print CLUSTER_FL "$cluster_name\n";
      for my $bar_name ( @{$map->{"bar"}{labels}} ) {
        print CLUSTER_FL "\n";
        print BAR_FL "$bar_name\n";
      }
      print BAR_FL "\n";
    }
    close CLUSTER_FL;
    close BAR_FL;
    
    system ("$self->{ploticus} $scripts->{box} data=$data_fl ymaxval=$maxval yminval=$minval num_cluster=$num_cluster cluster_name=$cluster_fl num_bar=$num_bar bar_name=$bar_fl width=$width height=$height xlabel=\"$xlabel\" ylabel=\"$ylabel\" title=\"$title\" -$output -o $output_fl -$orientation");

    system ("rm $cluster_fl $bar_fl > /dev/null 2>&1");
  }

  elsif ($info->type eq "line"){
    my $num_lines = scalar ( @{$map->{"line"}{labels}} );
    my $line_fl = "$TMP_DIR/line$$";
    my $sort_field = 1;
    
    open (LINE_FL, ">$line_fl") || die "Unable to open file $line_fl for plotting data.\n";
    my $count = 1;
    for my $line_name (@{$map->{"line"}{labels}}) {
      print LINE_FL "$line_name\n";
      if (defined $sort_name) {
        if ($sort_name eq $line_name) {
          $sort_field = $count;
        }
      }
      $count++;
    }
    close LINE_FL;
    
    # Sort data for s-curves, and u-curve
    if ($sort_type != 0) {
      my $sorted_data_fl = "$TMP_DIR/sorted_data$$";
      if (system ("sort -n +$sort_field < $data_fl > $sorted_data_fl")) {
        print "ERROR: Unable to sort data.\n";
      }
      else {
        if ($sort_type == 2) {
          # Invert all numbers below 1.0 for u-curve
          open (SORT_FL, "<$sorted_data_fl") || 
          die "Unable to open sorted data file $sorted_data_fl.\n";
          
          my $inv_data_fl = "$TMP_DIR/inv_data$$";
          open (INV_FL, ">$inv_data_fl");
          
          my $min_inv_val = 1e30;
          my $max_inv_val = -1;
          
          while (<SORT_FL>) {
            # First field is the xaxis value. 
            my $inv_line;

            foreach my $value (split(" ", $_)) {
              if (! defined $inv_line)
              {
                # First element in line is the string associated with 
                # the xaxis tick mark.
                $inv_line = $value;
              }
              elsif ( $value < 1.0 && $value != 0 ) {
                # invert value. 
                my $v = 1/$value;
                $inv_line = $inv_line . " " . $v;
                if ($max_inv_val < $v) {
                  $max_inv_val = $v;
                }
                if ($min_inv_val > $v) {
                  $min_inv_val = $v;
                }
              }
              else {
                $inv_line = $inv_line . " " . $value;
                if ($max_inv_val < $value) {
                  $max_inv_val = $value;
                }
                if ($min_inv_val > $value) {
                  $min_inv_val = $value;
                }
              }
            }
            
            print INV_FL "$inv_line\n";
          }

          close (SORT_FL);
          close (INV_FL);

          # Reset maxval and minval. 
          if ($max_computed == 1) {
            $maxval = $max_inv_val;
          }
          if ($min_computed == 1) {
            $minval = $min_inv_val;
          }
          
          print "....Plotting U-CURVE....\n";
          system ("cp $inv_data_fl $data_fl");
          system ("rm $inv_data_fl > /dev/null 2>&1");
        }
        else {
          print "....Plotting S-CURVE....\n";
          system ("cp $sorted_data_fl $data_fl");
        }
        system ("rm $sorted_data_fl > /dev/null 2>&1");
      }
    }
      
    system ("$self->{ploticus} $scripts->{line} data=$data_fl ymaxval=$maxval yminval=$minval num_lines=$num_lines line_name=$line_fl width=$width height=$height xlabel=\"$xlabel\" ylabel=\"$ylabel\" title=\"$title\" -$output -o $output_fl -$orientation");

    system ("rm $line_fl > /dev/null 2>&1");
  }
  
  # Cleanup
  system ("rm $data_fl > /dev/null 2>&1");
  
  return $self;
}

sub _dump_type_options {
  my $self = shift;
  my $options = $self->{valid_options};
  
  print "\tValid plot types: \n\t\t", join(", ", @{$options->{type}}), "\n";
}

sub _dump_output_options {
    my $self = shift;
    my $options = $self->{valid_options};
    
    print "\tValid plot outputs: \n";
    print "\t\t", join(", ", @{$options->{output}}), "\n";
}

sub _dump_legendloc_options {
    my $self = shift;
    my $options = $self->{valid_options};
    
    print "\tValid plot legend locations: \n";
    print "\t\t";
    foreach my $loc (@{$options->{lloc}}) {
	my $abbr = $options->{lloc_abbr}->{$loc};
	print "[$loc | $abbr], "
    }
    print "\n";
}

sub _dump_dimensions {
  my $self = shift;
  my $info = $self->{info};
  my @dims = keys %{$info->dimensions};

  print "\tImplicit Data Dimensions:\n";
  
  if ( scalar (@dims) == 0 ) {
    print "\tWARNING: No hard data dimensions available for plotting.\n";
    print "\tYou need to run 'get data' first.\n";
  }
  else {
    print "\t\t", join(", ", @dims), "\n";
  }
  return $self;
}


sub _dump_groups {
  my $self = shift;
  my $info = $self->{info};
  my $groups = $self->{groups};
  my @dims = keys %{$groups};
    
  print "\tSynthetic Data Dimensions or groups.\n";

  if ( scalar (@dims) == 0 ) {
    print "\t\tNo synthetic data dimensions available for plotting.\n";
  }
  else {
    foreach my $name (keys %{$groups}) {
      print "\t\t$name: ", join(", ", @{$groups->{$name}{labels}}), "\n";
    }
  }
  return $self;
}

sub _dump_replacements {
  my $self = shift;
  my $gen_str = $self->{gen_str};

  foreach my $dim (keys %{$gen_str}) {
    print "$dim => ";
    print join(" ", @{$gen_str->{$dim}}), "\n";
  }
}

sub _dump_mappings() {
  my $self = shift;
  my $info = $self->{info};
  my $map_base = $self->{plot_2_data};

  if ( defined ($map_base->{$info->type}) ) {
    my $map = $map_base->{$info->type};
    
    foreach my $plot_dim (keys %{$map}) {
      print "\t\t$plot_dim => ";
      if (defined $map->{$plot_dim}{dim}) {
	print "$map->{$plot_dim}{dim}";
      }
      print "\n";
    }
  }
  else {
    print "\tNo mappings specified. Run command 'set plot type'.\n";
  }
}

sub _dump_options {
  my $self = shift;
  my $info = $self->{info};
  my $option;

  print "PLOT OPTIONS: \n";
  print "Output: \n";
  $self->_dump_output_options();
  print "\tCurrent Value: ", $info->output, "\n";
  print "\tOutput File: ", $info->output_fl, "\n";

  print "Plot Legend Location: \n";
  $self->_dump_legendloc_options();
  print "\tCurrentValue: ", $info->legendloc, "\n";

  print "Plot Width: \n";
  print "\tCurrentValue: ", $info->width, "\n";

  print "Plot Height: \n";
  print "\tCurrentValue: ", $info->height, "\n";

  print "Plot Type: \n";
  $self->_dump_type_options();
  print "\tCurrentValue: ", $info->type, "\n";

  print "Available Data Dimensions for Plotting: \n";
  $self->_dump_dimensions();
  $self->_dump_groups();

  print "Current Plot to Data Mappings.\n";
  $self->_dump_mappings();
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





