#
# *****************************************************************************
# *
# * @brief Copy.pm : utility methods to handle "copy" repositories,
# *                  i.e. those checked out by the "use package" command.
# *                  This is a subclass of class "Package.pm".
# *
# * @author Carl Beckmann
# *
# Copyright (C) 2006 Intel Corporation
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

package Asim::Package::Copy;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Package);



=head1 NAME

Asim::Package::Copy - Class to maniputate a "use package" repository.


=head1 SYNOPSIS

use Asim::Package::Copy;

Asim::Package::Copy::init();

my $repo = Asim::Package->new("~/repodir");

Asim::Package::Copy->set_type( $repo );

$repo->update();


=head1 DESCRIPTION

This is a class to allow a directory to be treated as 
a "use package" repository, i.e. on copied from the shared area. 

This is a subclass of Asim::Package.  After creating an instance of Asim::Package,
you can call the set_type() method here to check whether the package is in a "copy"
repository and set it to this subclass if so.  Set_type will return 1 if it is a
Copy package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $copy = Asim::Cvs::set_type( $package )

If $package is a Copy repository, set the object's subclass to Asim::Package::Copy and return 1.
Otherwise return 0.

=cut

################################################################

sub set_type {
  my $self = shift;
  my $location = $self->location();
  # if there is a file named "COPY", then this was checked out using "use".
  # Note we should test this first, because CVS, SVN and BK metafiles and directories
  # may have been copied inadvertently so those tests might pass, too.
  if (-e "$location/COPY") {
    bless $self;
    return 1;
  }
  return 0;
}

################################################################

=item $package-E<gt>type()

Return type of package - "copy" in this case.

=cut

################################################################

sub type {
  return 'copy';
}

################################################################

=item Asim::Package::Copy::init()

Global init of Copy module.

=cut

################################################################

sub init {
  1;
}

################################################################

=item $copy-E<gt>update()

Update this package from the shared area

=cut

################################################################

sub update {
  my $self = shift;

  my $destination = $self->location();
  my $source = `cat $destination/COPY`;
  chomp $source;

  if ($source eq "") {
    Asim::Package::ierror("File COPY missing from package\n");
    return undef;
  }

  print "Updating a COPY package\n";
  my $status  = system("rsync -av --exclude CVS --delete $source $destination");
  $status    |= system("echo $source >$destination/COPY");

  if ($status) {
    Asim::Package::ierror("Rsync failed\n");
    return undef;
  }

  return 1;
}

################################################################
#
# Function: status
#
# Check on the status of each file in the current package
#
# Return a array with one element for each file.  This looks like
# CVS status, but there are really only 2 possible states:
# "up-to-date" and "needs update".
#
# Each array element contains:
#
#  ($directory, $filename, $status, $reprev, $date)
#
################################################################

sub status {
  #
  # FIX FIX !
  # This should do a "diff" with the shared area,
  # and report any files that are out-of-date, added, or removed.
  #
  Asim::Package::iwarn("Status not implemented for COPY repositories\n");
  return ();
}

################################################################

=item $cvs-E<gt>commit_check()

    This always returns 0, since it's impossible to commit changes
    from a COPY repository.
=cut

################################################################

sub commit_check {
  Asim::Package::ierror("Cannot commit a COPY repository\n");
  return 0;
}

=back


=head1 BUGS

Need to implement the status() method.


=head1 AUTHORS

Carl Beckmann

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2006

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
