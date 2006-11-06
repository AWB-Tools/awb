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


package Asim::Benchmark;
use warnings;
use strict;

use File::Basename;

use Asim::Base;

our @ISA = qw(Asim::Base);

our %a =  ( name =>                 [ "name",
                                      "SCALAR" ],
            description =>          [ "description",
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


=item $model = Asim::Benchmark::DB-E<gt>new($benchmarkDB, $name, [$variable, $value]...)

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
    my $benchmarkDB = shift;
    my $benchmarkname = shift;

    $self->open($benchmarkDB, $benchmarkname) || return undef;

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
  my $benchmarkDB = shift;
  my $name = shift;

  #
  # Rememeber benchmarkDB and benchmark name
  #
  $self->{benchmarkDB} = $benchmarkDB;
  $self->{name} = $name;

  $self->modified(0);
  return 1;
}




################################################################

=item $model-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    return qw(name description);
}


################################################################

=item $model-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}


################################################################

=item $model-E<gt>version([$value])

Set model "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return "1.0" }



################################################################

=item $model-E<gt>name([$value])

Set model "name" to $value if supplied. 
Always return model "name".

=cut

################################################################

sub name        { return $_[0]->{name} }

################################################################

=item $model-E<gt>description([$value])

Set model "description" to $value if supplied. 
Always return current model "description".

=cut

################################################################

sub description { return $_[0]->{description} }

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
