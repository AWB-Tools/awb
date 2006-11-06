#
# *****************************************************************
# *                                                               *
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


package Asim::Packfile;
use warnings;
use strict;

use Asim::Base;
use Asim::Inifile;

our @ISA = qw(Asim::Base Asim::Inifile);

our %a =  ( version =>              [ "version",
                                      "SCALAR" ],
            name =>                 [ "name",
                                      "SCALAR" ], 
            description =>          [ "description",
                                      "SCALAR" ],
            dependencies =>         [ "dependencies",
                                      "ARRAY" ],
      );

=head1 NAME

Asim::Packfile - Library for manipulating ASIM packfiles

=head1 SYNOPSIS

use Asim::Packfile;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an ASIM packfile

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $packfile = Asim::Packfile-E<gt>new([$file])

Create a new packfile, optionally reading configuration 
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
      $self->open($_[0]) || return ();
  }

  return $self;
}

sub _initialize {
  my $self = shift;

  $self->{accessors} = \%a;
  $self->{filename}  = "";
  $self->{inifile}   = {};
  $self->{dependencies} = ();

  return $self;
}

################################################################

=item $packfile-E<gt>open($file)

Parse a performance packfile file $file and populate the attributes
of the packfile object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;
  my $inifile;

  #
  # Cannonicalize the filename and parse the configuration file
  #
  $file = $Asim::default_workspace->resolve($file) || return ();

  $inifile = Asim::Inifile->new($file);

  $self->{filename} = $file;
  $self->{inifile} = $inifile;

  #
  # Get the dependencies
  #
  @{$self->{dependencies}} = ($self->_accessor_array("Global","Packages"," "));

  return 1;
}


################################################################

=item $module-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    return qw(version name description dependencies);
}


################################################################

=item $module-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $packfile-E<gt>version([$value])

Set packfile "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","Version",$_[1]); }



################################################################

=item $packfile-E<gt>name([$value])

Set packfile "name" to $value if supplied. 
Always return packfile "name".

=cut

################################################################

sub name        { return $_[0]->_accessor("Global","Name",$_[1]); }

################################################################

=item $packfile-E<gt>description([$value])

Set packfile "description" to $value if supplied. 
Always return current packfile "description".

=cut

################################################################

sub description { return $_[0]->_accessor("Global","Description",$_[1]); }


################################################################

=item $module-E<gt>dependencies()

Return list of packages on which this packfile is dependent

=cut

################################################################

sub dependencies {
  my $self = shift;

  return (@{$self->{dependencies}});
}


################################################################

=item $packfile-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base

################################################################

=item $packfile-E<gt>edit()

Edit the packfile

=cut

################################################################

sub edit {
  my $self = shift;

  $self->Asim::Base::edit();
}

################################################################

=item $packfile-E<gt>dump()

Dump the packfile in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;

  $self->Asim::Base::dump();
}

###########################################################################

=back

=head1 BUGS

Indeterminant

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2002

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
