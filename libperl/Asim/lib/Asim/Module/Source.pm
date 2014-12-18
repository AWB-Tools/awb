#
# Copyright (C) 2003-2014 Intel Corporation
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

package Asim::Module::Source;
use warnings;
use strict;

use Asim::Base;

our @ISA = qw(Asim::Base);

our %a = ( attributes   => [ "attributes", 
                             "HASH" ], 
           filelist     => [ "filelist",
                             "ARRAY" ]
       );

=head1 NAME

Asim::Module::Source - Library represeting AWB sources and their metadata

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $inifile = Asim::Module::SourcesE<gt>new($file, $attributes)

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
                  attributes => {},
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
    return qw( setAttribute
               getAttribute
               getAllAttributes
               files
             );
}


################################################################

=item $model-E<gt>setAttribute($attributeKey, $attributeValue)

Set source attribute to provided value.

=cut

################################################################

sub setAttribute {
    my $self = shift;
    my $attributeKey = shift;
    my $attributeValue = shift;

    if (defined($attributeKey) && defined($attributeValue)) {
        $self->{attributes}->{$attributeKey} = $attributeValue;
    }
}


################################################################

=item $model-E<gt>getAttribute($attributeKey)

Returns value of attribute key, or undefined if no attribute exists.

=cut

################################################################

sub getAttribute {
    my $self = shift;
    my $attributeKey = shift;

    if (defined($attributeKey)) {
        if (exists $self->{attributes}->{$attributeKey}) {
            return $self->{attributes}->{$attributeKey};
        } else {
            return undef;
        }
    }
} 

################################################################

=item $model-E<gt>getAllAttributes()

Returns a hash containing all attributes of the given module. 

=cut

################################################################

sub getAllAttributes {
    my $self = shift;
    return $self->{attributes};
} 

################################################################

=item $model-E<gt>files()

Return file list.

=cut

################################################################

sub files {
    my $self = shift;
    my @files = @_; 

    if (scalar(@files)) {
       push(@{$self->{filelist}}, @files);  
    }

    return @{$self->{filelist}};
}

=back

=head1 BUGS


=head1 AUTHORS

Kermin Fleming

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2014

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
