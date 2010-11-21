#
# Copyright (C) 2010 Intel Corporation
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

package Asim::Module::Attribute;
use warnings;
use strict;

use File::Basename;
use Getopt::Long;

our %a =  ( fullname =>             [ "fullname",
                                      "SCALAR" ],
            name =>                 [ "name",
                                      "SCALAR" ],
            is_manditory =>         [ "is_manditory",
                                      "SCALAR" ],
            is_conflicting =>       [ "is_conflicting",
                                      "SCALAR" ],
          );

our $debug = 0;

=head1 NAME

Asim::Attribute - Library for manipulating ASIM model/module attributes

=head1 SYNOPSIS

use Asim::Attribute;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $attribute = Asim::Attribute-E<gt>new($name)

Create a new attribute object. Currently the attribute
is specified as a string. An attribute that begins with
a + is a manditory attribute and an attribute that begins
with a - is a conflicting attribute. An attribute's name
is the string without the leading markers, i.e., +/-

=cut

################################################################

sub new {
  my $this = shift;
  my $fullname = shift;

  my $class = ref($this) || $this;
  my $self = _initialize();

  if (! defined($fullname)) {
    print "Attribute name not defined\n";
    return undef;
  }

  $self->{fullname} = $fullname;

  if ($fullname =~ /^\+(.*)/) {
    $self->{is_manditory} = 1;
    $self->{name} = $1;
  }

  if ($fullname =~ /^\-(.*)/) {
    $self->{is_conflicting} = 1;
    $self->{name} = $1;
  }

  bless	$self, $class;

  return $self;
}


sub _initialize {
  my $fullname = shift;

  my $self = { accessors => \%a,
               fullname => "",
               name => "",
               is_manditory => 0,
               is_conflicting => 0
             };


  return $self;
}


################################################################

=item $attribute-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
  my $self = shift;

  return qw(name
	    is_manditory
	    is_conflicting
	  );
}


################################################################

=item $attribute-E<gt>fullname()

Return the fullname for an attribute, i.e., with the qualifiers

=cut

################################################################

sub fullname {
  my $self = shift;

  return $self->{fullname};
}

################################################################

=item $attribute-E<gt>name()

Return the name for an attribute, i.e., with the qualifiers removed

=cut

################################################################

sub name {
  my $self = shift;

  return $self->{name};
}

################################################################

=item $attribute-E<gt>is_manditory()

Is the attribute manditory

=cut

################################################################

sub is_manditory {
  my $self = shift;

  return $self->{is_manditory};
}

################################################################


=item $attribute-E<gt>is_conflicting()

Is the attribute conflicting

=cut

################################################################

sub is_conflicting {
  my $self = shift;

  return $self->{is_conflicting};
}

################################################################

=item $module-E<gt>dump()

Textually dump module to STDOUT

=cut

################################################################

sub dump {
  my $self = shift;

  my $level = 0;
  if (defined($_[0]) && $_[0] =~ /^\d+$/) {
    $level = shift;
  }

  return;
}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
    my $self = shift;
    my $message = shift;

    if (exists($self->{filename})) {
        print "Asim::Module::Attribute ($self->{filename}) Error - $message";
    }
    else {
        print "Asim::Module::Attribute Error - $message";
    }

    return 1;
}

=back

=head1 BUGS

This should be Asim::Module::Attribute and Asm::Model  can use it...


=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel, 2010

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;

