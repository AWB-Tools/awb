# Copyright (C) 2004-2008 Intel Corporation
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

package Asim::Benchmark;
use warnings;
use strict;

use File::Basename;

use Asim::Base;

our @ISA = qw(Asim::Base);

our %a =  ( filename =>             [ "filename",
                                      "SCALAR" ],
            name =>                 [ "name",
                                      "SCALAR" ],
            description =>          [ "description",
                                      "SCALAR" ],
            feeder =>               [ "feeder",
                                      "SCALAR" ],
            region =>               [ "region",
                                      "SCALAR" ],
            setup_command =>        [ "setup_command",
                                      "SCALAR" ],
            setup_args =>           [ "setup_args",
                                      "SCALAR" ],
            general_flags =>        [ "general_flags",
                                      "SCALAR" ],
            feeder_flags =>         [ "feeder_flags",
                                      "SCALAR" ],
            system_flags =>         [ "system_flags",
                                      "SCALAR" ],
      );


our $debug = 0;


=head1 NAME

Asim::Benchmark - An Asim benchmark

=head1 SYNOPSIS

use Asim::Benchmark;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an ASIM benchmark

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $model = Asim::Benchmark::DB-E<gt>new($name, [$variable, $value]...)

Create a new benchmark from $benchmarkDB with name $name and using variable substitution
from each $variable, $value pair.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  $self->_initialize();

  #
  # Open specific benchmark if given
  #
  if (@_) {
    my $filename = shift;

    $self->open($filename) || return undef;

    # There must be a better way to do this...

    while (@_) {
      my $a = shift;
      my $b = shift;

      $self->{vars}->{$a} = $b;
    }
  }

  return $self;
}

sub _initialize {
  my $self = shift;

  $self->{accessors} = \%a;
  $self->{filename}  = undef;

  $self->{name} = "";
  $self->{description} = "";
  $self->{feeder} = "";

  $self->{region} = undef;
  $self->{setup_command} = "";
  $self->{setup_args} = "";
  $self->{general_flags} = "";
  $self->{feeder_flags} = "";
  $self->{system_flags} = "";

  $self->{vars} = {};

  $self->modified(0);
  return $self;
}

################################################################

=item $model-E<gt>open($file)

Parse a performance model file $file and populate the attributes
of the model object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;
  my $status;

  my $orig_file = $file;

  #
  # Strip region specifier
  #
  $self->{region} = '';
  my $region_tag = '';
  my $is_region = $file =~ m/_r([0-9]+)$/;
  if ($is_region) {
      $self->{region} = $1;
      $file =~ s/_r[0-9]+$//;
      $region_tag = '_r' . $self->{region};
  }

  #
  # Handle path with .cfx in them
  # 
  my $is_cfx = $file =~ m/(.*\.cfx)\/(.*)/;
  my $cfg_base = $1;
  my $cfx_suffix = $2;
  my $cfx_file;
  my $open_file;

  if ($is_cfx) {

    #Use the CFX to generate the CFG

    my $cfx_file = Asim::resolve($cfg_base) 
      ||  return undef;

    $open_file = "asimstarter -- $cfx_file --emit $cfx_suffix |";
  } else {

    my $cfg_file = Asim::resolve($file)
      || return undef;

    $open_file = "< $cfg_file";
  }

  #
  # Parse cfg file (version 1)
  #
  my @cfg = ();

  CORE::open(CFG, $open_file)
    || return undef;

  while (($#cfg < 8) && (my $line = <CFG>)) {
    chomp($line);

    # Drop leading whitespace and TCL {} string encapsulation
    $line =~ s/^[ \t{]*//;
    $line =~ s/[ \t}]*$//;

    # Substitute region tag
    $line =~ s/%R/$region_tag/g;

    # Keep all but the leading BmAdd { token
    if (! ($line =~ /^BmAdd[ \t{]*$/)) {
        push(@cfg, $line);
    }
  }

  ##
  ## This is where we would parse TCL controller commands.
  ##

  CORE::close(CFG);

  if ($#cfg != 8) {
      ierror("Error parsing benchmark file ${orig_file}\n");
      ierror("Wrong number of items found: got $#cfg but expected 8.\n");
      return 0;
  }

  $self->{name} = $cfg[0];
  $self->{filename} = $cfg[1];
  $self->{description} = $cfg[2];
  $self->{feeder} = $cfg[3];

  #
  # Setup command and arguments comes from line 4...
  #
  my $setup = $cfg[4];

  $setup =~ m/ *([^ ]*) (.*)/;
  my $setup_command = $1;
  my $setup_args = $2;
  if ($is_region) {
      $setup_args .= " --region " . $self->{region};
  }

  $self->{setup_command} = Asim::resolve($setup_command);
  $self->{setup_args} = $setup_args;

  #
  # Hack alert.  General flags can include flags to this code,
  # e.g. the --regions tag used by awb to show individual regions.
  # The same is true of --queryregions. 
  # Strip them.
  #
  my $genfl = $cfg[5];
  $genfl =~ s/--regions[ \t]*[0-9]*[ \t]*//g;
  $genfl =~ s/--queryregions[ \t]*//g;
  $self->{general_flags} = $genfl;

  $self->{feeder_flags} = $cfg[6];
  $self->{system_flags} = $cfg[7];

  $self->modified(0);

  return 1;
}


################################################################

=item $model-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    return qw(filename name description feeder region setup_command setup_args general_flags feeder_flags system_flags);
}


################################################################

=item $benchmark-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}


################################################################

=item $model-E<gt>version()

Return configuration file "version".

=cut

################################################################

sub version     { return "1.0" }


################################################################

=item $model-E<gt>name()

Return model "name".

=cut

################################################################

sub name        { return $_[0]->{name} }

################################################################

=item $model-E<gt>description()

Return current model "description".

=cut

################################################################

sub description { return $_[0]->{description} }

################################################################

=item $model-E<gt>feeder()

Return current model "feeder".

=cut

################################################################

sub feeder { return $_[0]->{feeder} }

################################################################

=item $model-E<gt>region()

Return the region of the workload.

=cut

################################################################

sub region { return $_[0]->{region} }

################################################################

=item $benchmark-E<gt>setup_command()

Filename of setup script

=cut

################################################################

sub setup_command { return $_[0]->{setup_command} }


################################################################

=item $benchmark-E<gt>setup_args()

Arguemnts to the setup script

=cut

################################################################

sub setup_args { return $_[0]->{setup_args} }


################################################################

=item $benchmark-E<gt>general_flags()

General flags to in the run script

=cut

################################################################

sub general_flags { return $_[0]->{general_flags} }


################################################################

=item $benchmark-E<gt>feeder_flags()

Feeder flags to in the run script

=cut

################################################################

sub feeder_flags { return $_[0]->{feeder_flags} }


################################################################

=item $benchmark-E<gt>system_flags()

System flags to in the run script

=cut

################################################################

sub system_flags { return $_[0]->{system_flags} }


################################################################

=item $model-E<gt>get($name)

Get value of field $name.

=cut

################################################################

sub get {
  my $self = shift;
  my $field = shift;

  my $benchmarkDB = $self->{benchmarkDB};
  my $benchmarkname = $self->{name};

  my $value = $benchmarkDB->get_item($benchmarkname, $field);

  if (defined($value)) {
    return $self->_substitute($value);
  } else {
    return undef;
  }
}


sub _substitute {
  my $self = shift;
  my $string = shift;

  # Parse for string that looks like ${name} or ${name:default}

  while ($string =~ /\${([a-zA-Z0-9]*)(:(.*))?}/) {
    my $name = $1;
    my $junk = $2;
    my $default = $3;
    my $value = $self->{vars}->{$name};

    # Try substituting a local variable

    if (defined($value)) {
      $string =~ s/\${${name}[^}]*}/${value}/;
      next;
    }

    # Try substituting a field from benchmarkDB

    $value = $self->get($name);

    if (defined($value)) {
      $string =~ s/\${${name}[^}]*}/${value}/;
      next;
    }


    # Try default value

    if (defined($default)) {
      $string =~ s/\${${name}.*}/${default}/;
      next;
    }

    # Could not find any substitution...

    $string =~ s/\${${name}[^}]*}/***UNDEFINED***/;
  }

  return $string;
}

################################################################

=item $model-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base

################################################################

################################################################

=item $model-E<gt>dump()

Dump the model in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;

  $self->Asim::Base::dump();

  print ("Variables:\n");

  for my $v (keys %{$self->{vars}}) {
    print $v . " = " . $self->{vars}->{$v} . "\n";
  }
}


################################################################
#
# Internal error utility function
#
################################################################

sub iwarn {
  my $message = shift;

  print "Asim::Benchmark:: Warning: - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Asim::Benchmark:: Error - $message";

  return 1;
}

###########################################################################

=back

=head1 BUGS

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
