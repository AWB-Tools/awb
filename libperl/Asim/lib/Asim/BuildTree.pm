#
# *****************************************************************************
# *
# * @brief BuildTree.pm  
# *
# * @author Oscar Rosell
# *
# Copyright (C) 2005-2006 Intel Corporation
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

package Asim::BuildTree;
use warnings;
use strict;

=head1 NAME

Asim::BuildTree - Class to manipulate a build tree: configure, build and install

=head1 SYNOPSIS

use Asim::BuildTree;

our @ISA = qw(Asim::BuildTree ...);


=head1 DESCRIPTION

Probably this is most useful as base class from which other classes
can be derived in order to give them the capability to be operated on
as a checked out CVS repository.

Note that the parent class must implement a "location()" method.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $build_tree-E<gt>configure()

Configure this package

=cut

################################################################

sub configure {
  my $self = shift;
  my $location = $self->location();
  my $configflags = $Asim::default_workspace->awb_package_configflags();
  my $command = "cd $location; ./configure";

  if (defined($configflags)) {
      $command = $command . " $configflags"
  }
  
  return 1 if (! -x "$location/configure");

  print "% $command\n";
  eval {print `($command)`};

  if ($@) {
      warn $@;
      return undef;
  }

  # Sleep to avoid clock skew problems if
  # the build is done immediately afterwards...
  sleep(2);

  return 1;
}

################################################################

=item $build_tree-E<gt>build()

Build this package

=cut

################################################################

sub build {
  my $self = shift;
  my $location = $self->location();
  my $makeflags = $Asim::default_workspace->awb_package_makeflags();
  my $maketarget = $self->maketarget();
  my $command;
  if (defined $maketarget)
  {
      $command = "cd $location; make $maketarget";
  }
  else
  {
      $command = "cd $location; make";
  }

  if (defined($makeflags)) {
      $command = $command . " $makeflags"      
  }

  return 1 if (! -e "$location/Makefile");

  print "% $command\n";
  if (system($command)) {
      warn("Make failed\n");
      return undef;
  }

  return 1;
}

################################################################

=item $build_tree-E<gt>install()

Install this package

=cut

################################################################

sub install {
  my $self = shift;
  my $location = $self->location();
  my $command = "cd $location; make install";

  return 1 if (! -e "$location/Makefile");

  print "% $command\n";
  if (system($command)) {
      warn("Install failed\n");
      return undef;
  }

  return 1;
}

################################################################

=item $build_tree-E<gt>clean()

Clean up this package's build

=cut

################################################################

sub clean {
  my $self = shift;
  my $location = $self->location();
  my $command = "cd $location; make clean";

  return 1 if (! -e "$location/Makefile");

  print "% $command\n";
  if (system($command)) {
      warn("Clean failed\n");
      return undef;
  }

  return 1;
}

=back

=head1 BUGS

=head1 AUTHORS

Oscar Rosell created the file by moving some methods from Cvs.pm by
    Joel Emer, Roger Espasa, Artur Klauser and  Pritpal Ahuja

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2005

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;

