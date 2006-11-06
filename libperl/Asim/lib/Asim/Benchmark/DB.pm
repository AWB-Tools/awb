#
# Copyright (C) 2004-2006 Intel Corporation
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


package Asim::Benchmark::DB;
use warnings;
use strict;

use File::Basename;

use Asim::Base;
use Asim::Inifile;

our @ISA = qw(Asim::Base Asim::Inifile);

our %a =  ( version =>              [ "version",
                                      "SCALAR" ],
            name =>                 [ "name",
                                      "SCALAR" ],
            description =>          [ "description",
                                      "SCALAR" ],
      );


our $debug = 0;


=head1 NAME

Asim::Benchmark::DB - Library for manipulating ASIM benchmarks lists

=head1 SYNOPSIS

use Asim::Benchmark::DB;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an ASIM benchmark

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $benchmarkDB = Asim::Benchmark::DB-E<gt>new([$file])

Create a new performance model, optionally reading configuration 
file $file to populate the object.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  $self->_initialize();

  #
  # Parse file if given
  #
  if (@_) {
      $self->open($_[0]) || return undef;
  }

  return $self;
}

sub _initialize {
  my $self = shift;

  $self->{accessors} = \%a;
  $self->{filename}  = undef;
  $self->{inifile}   = {};

  $self->{inifile} = Asim::Inifile->new();

  $self->name("New Asim Benchmark::DB");
  $self->description("");

  $self->modified(0);
  return $self;
}

################################################################

=item $benchmarkDB-E<gt>open($file)

Parse a performance model file $file and populate the attributes
of the model object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;
  my $inifile;

  #
  # Cannonicalize the filename and parse the configuration file
  #
  $file = $Asim::default_workspace->resolve($file) || return undef;
  $inifile = Asim::Inifile->new($file) || return undef;

  $self->{filename} = $file;
  $self->{inifile} = $inifile;

  #
  # Get the well known fields
  #
  my $version = $inifile->get("Global", "Version");

  if ("$version" != "1.0") {
    ierror("Illegal benchmarkDB version - $version\n");
    return undef;
  }

  $self->modified(0);
  return 1;
}




################################################################

=item $benchmarkDB-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    return qw(version name description);
}


################################################################

=item $benchmarkDB-E<gt>filename()

Filename of source of benchmark information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}


################################################################

=item $benchmarkDB-E<gt>version([$value])

Set model "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","Version",$_[1]) || "2.0"; }



################################################################

=item $benchmarkDB-E<gt>name([$value])

Set model "name" to $value if supplied. 
Always return model "name".

=cut

################################################################

sub name        { return $_[0]->_accessor("Global","Name",$_[1]) || ""; }

################################################################

=item $benchmarkDB-E<gt>description([$value])

Set model "description" to $value if supplied. 
Always return current model "description".

=cut

################################################################

sub description { return $_[0]->_accessor("Global","Description",$_[1]) || ""; }


################################################################

=item $benchmarkDB-E<gt>directory()

List individual benchmarks in the benchmarkDB.

=cut

################################################################

sub directory {
  my $self = shift;
  my $inifile = $self->{inifile};

  my @list = $inifile->get_grouplist();

  # Groups 'Global' and '_*' are not benchmarks

  @list = grep(!/^(Global$|_)/, @list);

  return (sort(@list));
}

################################################################

=item $benchmarkDB-E<gt>get_benchmark($benchmarkname)

Return a benchmark object with the given name.

=cut

################################################################

sub get_benchmark {
  my $self = shift;
  my $name = shift;
  my @variables = @_;

  my $benchmark = Asim::Benchmark->new($self, $name, @variables);

  return $benchmark;
}

################################################################

=item $benchmarkDB-E<gt>get_item($benchmark, $name)

Get a field value for $name

=cut

################################################################

sub get_item {
  my $self = shift;
  my $benchmark = shift;
  my $name = shift;

  my $inifile = $self->{inifile};
  my $value = $inifile->get($benchmark, $name);

  if (defined($value)) {
    return $value;
  }

  # Follow includes if it exists

  my $include = $inifile->get($benchmark, "include");
  if (defined($include)) {
    return $self->get_item($include, $name);
  }

  return undef;
}

################################################################

=item $benchmarkDB-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base

################################################################

################################################################

=item $benchmarkDB-E<gt>dump()

Dump the model in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;

  $self->Asim::Base::dump();

  #$self->{workbench}->dump();
  $self->{system}->dump();

}


################################################################
#
# Internal error utility function
#
################################################################

sub iwarn {
  my $message = shift;

  print "Asim::Benchmark::DB:: Warning: - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Asim::Benchmark::DB:: Error - $message";

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
