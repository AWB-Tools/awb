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


package Asim::Base;

use warnings;
use strict;

use Asim::Inifile;

our $debug = 0;

=head1 NAME

Asim::Base - Implemetation of base class for various Asim objects.

=head1 SYNOPSIS

package Asim::Base;

=head1 DESCRIPTION

This module provides a set of generic methods common to a number
of Asim objects. Not useful as a class by itself, but as a base
class for other classes, such as Models and Modules.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $object-E<gt>accessors()

Return a list of accessor functions for this object
Must be overridden in the derived class.

=cut

################################################################

sub accessors {
  die("Internal program error: A method named 'accessor' must be provided\n".
      "by the class that has been derived from Asim::Base");
}

################################################################

=item $object-E<gt>name([$value])

Optionally update name of object to $value.
Return the current (updated) name of object.

=cut

################################################################


sub name {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"name"} = $value;
      $self->modified(1);
  }

  return $self->{"name"};
}


################################################################

=item $object-E<gt>description([$value])

Optionally update name of object to $value.
Return the current (updated) description of object.

=cut

################################################################


sub description {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"description"} = $value;
      $self->modified(1);
  }

  return $self->{"description"};
}



################################################################

=item $object-E<gt>location([$value])

Optionally update location of object to $value.
Return the current (updated) location of object.

=cut

################################################################


sub location {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"location"} = $value;
      $self->modified(1);
  }

  return $self->{"location"};
}




################################################################
#
# Generic function to access attributes that came from an inifile
#             (see usage examples in Model.pm)
################################################################

sub _accessor {
  my $self = shift;
  my $group = shift;
  my $item = shift;

  my $value = shift;

  my $newval;

  if (defined $value) {
    #print "Setting [$group] $item=$value\n";
    $self->{inifile}->put($group,$item,$value);
    $self->modified(1);
  }

  $newval = $self->{inifile}->get($group,$item);

  if (!defined($newval)) {
    return undef;
  }

  return $newval;
}



sub _accessor_array {
  my $self = shift;
  my $group = shift;
  my $item = shift;
  my $delim = shift;

  my $items;

  $items = $self->{inifile}->get($group, $item)
    || return ();

  return (split(/$delim/, $items));
}

sub _accessor_array_set {
  my $self = shift;
  my $group = shift;
  my $item = shift;
  my $delim = shift;

  my @list = (@_);

  $self->{inifile}->put($group, $item, join($delim, @list));
  $self->modified(1);

  return ($self->_accessor_array($group, $item, $delim));
}

################################################################

=item $object-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################


sub modified {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"modified"} = $value;
      print $self->name() . ": Modified = $value\n" if ($debug);
  }

  return $self->{"modified"};
}


################################################################

=item $object-E<gt>issame($object2)

Return TRUE if $object2 is the same as the base object. This is not
the LISP eq operation, since comparison is done just on object name.

=cut

################################################################

sub issame {
  my $self = shift;
  my $obj2 = shift;

  return ($self->name() eq $obj2->name());
}

################################################################

=item $object-E<gt>edit([@fields])

Interactively iterate through the fields of an object and
allow the user to update them. If @fields is provided it
specifies the names of the fields to change.

=cut

################################################################


sub edit {
  my $self = shift;
  my @accessors;

  if (@_) {
    @accessors = (@_);
  } else {
    @accessors = $self->accessors();
  }

  foreach my $i (@accessors) {
    $self->_edit_field($i);
  }

  $self->modified(1);

  return $self;
}



################################################################
#
#  Utility functions for entering and editing fields of an object
#
################################################################

sub _edit_field {
  my $self = shift;
  my $fname = shift;

  my $field;
  my $ftype;
  my $otype;

  ($field, $ftype, $otype) = (@{$self->{accessors}->{$fname}});

  if ($ftype eq "ARRAY") {
    _edit_array($self, $field, $otype);
  } elsif ( $ftype eq "SCALAR") {
    _edit_scalar($self, $field, $otype);
  } else {
    print "Can't handle $field\n;"
  }
}

sub _edit_array {
  my $self = shift;
  my $field = shift;
  my $otype = shift;

  my $value;
  my $prompt;
  my $input;

  print "Processing $field:\n";
  my @value = $self->$field();
  my @newvalue = ();

  foreach my $v (@value) {
    if (ref($v)) {
      if (Asim::choose_yes_or_no("Edit $field " . $v->name())) {
        $v->edit();
      } else {
        push(@newvalue, $v);
      }	
    } else {
      $prompt = "  Enter $field";
      $input = Asim::choose($prompt,$v);

      if (defined ($input) && $input) {
        if ($input ne ".") {
          push(@newvalue, $input);
        }
      }
    }
  }

  if ( defined($otype)) {
    while (1) {
      if (Asim::choose_yes_or_no("Add $field ")) {
	my $newobj = $otype->new();
	$newobj->edit();
	push(@newvalue, $newobj);
      } else {
	last;
      }
    }
  } else {
    while (1) {
      $prompt = "  Additional ${field}";
      $input = Asim::choose($prompt,".");

      if ($input && $input ne ".") {
	push(@newvalue, $input);
      } else {
	last;
      }
    }
  }

  $self->$field(@newvalue);
}


sub _edit_scalar {
  my $self = shift;
  my $field = shift;

  my $value;
  my $prompt;
  my $input;

  $value = $self->$field() || "";

  $prompt = "Enter $field";
  $input = Asim::choose($prompt,$value);

  if (defined($input) && $input) {
    $self->$field($input);
  }
}


################################################################

=item $object-E<gt>dump([$indent], [@fields])

Generic function to iterate through the fields of an object and dump
them to STDOUT. If $indent is provided dump will be indented by
specifed amount.  If @fields is provided it specifies the names of the
fields to dump.

=cut

################################################################


sub dump {
  my $self = shift;

  my $level = 0;

  if (defined($_[0]) && $_[0] =~ /^\d+$/) {
    $level = shift;
  }

  my @accessors;
  if (@_) {
    @accessors = (@_);
  } else {
    @accessors = $self->accessors();
  }

  foreach my $i (@accessors) {
    $self->_dump_field($level, $i);
  }

  return $self;
}



################################################################
#
#  Utility functions for dumping fields of an object
#
################################################################

sub _dump_field {
  my $self = shift;
  my $level = shift;
  my $fname = shift;

  my $field;
  my $ftype;
  my $otype;

  ($field, $ftype, $otype) = (@{$self->{accessors}->{$fname}});

  my $indent = "  " x $level;

  if ($ftype eq "ARRAY") {
    print "$indent$fname: \n";
    foreach my $i ($self->$field()) {
      if (ref($i)) {
	$i->dump($level+1);
        print "\n";
      } else {
	print $indent . "  " . $i . "\n";
      }
    }
    print "\n";

  } elsif ( $ftype eq "SCALAR") {
    my $i = $self->$field();
    if ($field eq "parent" && defined($i)) {	
	my $name = "";
	if (defined($i)) { $name = $i->name(); }
      print $indent . "$fname: " . $name . "\n";
    } elsif (ref($i)) {
      $i->dump($level+1);
    } elsif (defined($i)) {
      print $indent . "$fname: " . $i . "\n";
    }

  } else {
    print "Can't handle $field\n;"
  }
}

###########################################################################


=back

=head1 BUGS

  Dump does not indent properly...

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
