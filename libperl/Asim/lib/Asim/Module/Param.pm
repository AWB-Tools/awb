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


package Asim::Module::Param;
use warnings;
use strict;

use Asim::Base;

our @ISA = qw(Asim::Base);

our %a = ( name         => [ "name", 
                             "SCALAR" ], 
           description  => [ "description",
                             "SCALAR" ],
           type         => [ "type",
                             "SCALAR" ],
           dynamic      => [ "dynamic",
                             "SCALAR" ],
           default      => [ "default",
                             "SCALAR" ],
           value        => [ "value",
                             "SCALAR" ]
       );

=head1 NAME

Asim::Module::Param - Library for manipulating ASIM module parameters

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $inifile = Asim::Module::Param-E<gt>new($file)

Create a new ASIM parameter

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;

  my $self = _initialize(@_);

  bless	$self, $class;

  return $self;
}

sub _initialize {
  my $self = {  accessors => \%a,
		name => "",
		description => "",
		type => "",
                dynamic => 0,
		global => 0,
		value => undef,
		default => undef,
                isInt => 0,
                isString => 0,
		@_
	     };

  #
  # Preserve invariant that a parameter that has the 
  # default value has its value left undefined.
  #
  if (defined($self->{value})
      && $self->{value} eq $self->{default}) {
      $self->{value} = undef;
  }

  return $self;
}

################################################################

=item $module-E<gt>dup()

Return a copy of the object

=cut

################################################################

sub dup {
  my $self = shift;

  my $param = Asim::Module::Param->new( type        => $self->{type},
					dynamic     => $self->{dynamic},
					global      => $self->{global},
					name        => $self->{name},
					default     => $self->{default},
					value       => $self->{value},
                                        isInt       => $self->{isInt},
                                        isString    => $self->{isString},
					description => $self->{description});


  return $param;
}


################################################################

=item $module-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
  return qw( name
             description
             type
             dynamic
             default
             value
             isInt
             isString
	   );
}


################################################################

=item $model-E<gt>name([$value])

Set parameter name to $value if supplied. 
Always return current (updated) parameter name.

=cut

################################################################

# Implemented in Asim::Base

################################################################

=item $model-E<gt>description([$value])

Set parameter's description to $value if supplied. 
Always return current (updated) parameter description.

=cut

################################################################

# Implemented in Asim::Base

################################################################

=item $model-E<gt>type([$value])

Set parameter type to $value if supplied. 
Always return current (updated) paramter type.

=cut

################################################################

sub type {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{type} = $value;
  }
  return $self->{type};
}

################################################################

=item $model-E<gt>dynamic([$value])

Set parameter dynamic to $value if supplied. 
Always return current (updated) paramter dynamic.

=cut

################################################################

sub dynamic {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{dynamic} = $value;
  }
  return $self->{dynamic};
}

################################################################

=item $model-E<gt>global([$value])

Set parameter global to $value if supplied. 
Always return current (updated) paramter global.

=cut

################################################################

sub global {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{global} = $value;
  }
  return $self->{global};
}

################################################################

=item $model-E<gt>isexport([$value])

Return true if this is an export parameter

=cut

################################################################

sub isexport {
  my $self = shift;

  return ($self->{type} eq "export");
}

################################################################

=item $model-E<gt>isString([$value])

Return true if this is a string value parameter

=cut

################################################################

sub isString {
  my $self = shift;

  return $self->{isString};
}

################################################################

=item $model-E<gt>isInt([$value])

Return true if this is an integer value parameter

=cut

################################################################

sub isInt {
  my $self = shift;

  return $self->{isInt};
}

################################################################

=item $model-E<gt>default([$value])

Set object default value to $value if supplied. 
Always return current (updated) default value.

Note: A semantic choice was made with respect to
setting a new default that if the current 'value'
of a paramter  is the 'default' value then after
setting a new default the 'value' will still be
the 'default' value rather than the old 'default'
value.

=cut

################################################################

sub default {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{default} = $value;
      #
      # Preserve invariant that default values
      # are left undefined.
      #
      if (defined($self->{value}) && ($self->{value} eq $self->{default})) {
	  $self->{value} = undef;
      }  
  }
  return $self->{default};
}


################################################################

=item $model-E<gt>value([$value])

Set parameter's value to $value if supplied. 
Always return current (update) parameter value.

WARNING: if this parameter has been duplicated to appear
in the list of global parameters of a model then changing
a value here will not be reflected at the model level. Such
parameters must be updated from the top down...

=cut

################################################################

sub value {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{value} = $value;
      #
      # Preserve invariant that default values
      # are left undefined.
      #
      if ($self->{value} eq $self->{default}) {
	  $self->{value} = undef;
      }
  }

  if ( ! defined($self->{value}) ) {
      return $self->{default};
  }

  return $self->{value};
}




################################################################

=item $inifile-E<gt>dump()

Textually dump the contents of the object

=cut

################################################################


# Implemeted in Asim::Base

=back

=head1 NOTES

Although nominally owned by an Asim::Module, parameters can also
be owned by an Asim::Model when they are the 'global' parameters
of a submodel.

=head1 BUGS

Should restrict parameter types to "param" and "export". 
More error checking needed.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
