#
# Copyright (C) 2003-2007 Intel Corporation
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
#
# *****************************************************************************
# *
# * @brief SourceList.pm  
# *
# * @author Angshuman Parashar
# *
# *****************************************************************************
#

package Asim::Module::SourceList;
use warnings;
use strict;

use Asim::Base;
use Asim::Module::Source;


our @ISA = qw(Asim::Base);

our %a = ( type         => [ "type", 
                             "SCALAR" ], 
           visibility   => [ "visibility",
                             "SCALAR" ],
           files        => [ "files",
                             "ARRAY" ]
       );

=head1 NAME

Asim::Module::SourceList - Library for manipulating ASIM module sources

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $inifile = Asim::Module::Sources-E<gt>new($file)

Create a new ASIM source file list with a common type and visibility

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
                type => "",
                visibility => "",
                filelist => [],
                @_
             };

  return $self;
}

################################################################

=item $module-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
  return qw( type
             visibility
             files
	   );
}


################################################################

=item $model-E<gt>type([$value])

Set sourcelist type to $value if supplied.
Always return current (updated) sourcelist type.

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

=item $model-E<gt>visibility([$value])

Set sourcelist visibility to $value if supplied.
Always return current (updated) sourcelist visibility.

=cut

################################################################

sub visibility {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{visibility} = $value;
  }

  return $self->{visibility};
}

################################################################

=item $model-E<gt>files()

Return file list.

=cut

################################################################

sub files {
    my $self = shift;
    return @{$self->{filelist}};
}

################################################################

=item $model-E<gt>addfiles([$value])

Add files to file list

=cut

################################################################

sub addfiles {
    my $self = shift;
    my @flist = @_;

    push(@{$self->{filelist}}, @flist);
}

=back

=head1 BUGS


=head1 AUTHORS

Angshuman Parashar

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2007

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
