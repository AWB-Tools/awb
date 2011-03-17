#
# *****************************************************************************
# *
# * @brief Copy.pm : utility methods to handle copy repositories
# *
# * @author Mohit Gambhir
# *
# * Copyright (c) 2009 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::Repository::Copy;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::Copy - Class to access and manipulate an Copy repository.

=head1 SYNOPSIS

use Asim::Repository::Copy;

Asim::Repository::Copy::checkout();

my $copy = Asim::Repository::Copy->new("~/copydir");

$copy->update();


=head1 DESCRIPTION

This is a class to allow access to an copy repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here to check whether the package type in asim.pack is Copy and set it to this subclass if so.  Set_type will return 
1 if it is a Copy package.  If it returns 0, you should keep checking other repository types.

=cut

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $copy = Asim::Repository::Copy::set_type( $package )

If $package is an Copy repository, set the object's subclass to Asim::Repository::Copy and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;
  # If method is copy, then this is Subversion:
  if ( $self->{method} eq "copy" ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of package - "copy" in this case.

=cut

#################################################################################

sub type {
  return 'copy';
}

#################################################################################

=item $copy = Asim::Repository::Copy:init()

Global init of Copy module.

=cut

#################################################################################

sub init {
  # Deal with any Copy environment variables
  1;
}

#################################################################################

=item $dir = $repository-E<gt>create()

Create a repository from a package.

=cut

################################################################################

sub create {
  my $self = shift;
  my $type = $self->type();

  print "Repository creation for type $type unavailable\n";
  return undef;
}

#################################################################################

=item $dir = $repository-E<gt>checkout([$user])

Check out a copy repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || "anonymous";

  my $access = $archive->{access} || return undef;
  
  my $package_dir = $self->checkoutdir();

  my $status;

  # Call Asim::Repository::checkout to perform pre checkout validation
  if ( ! $self->SUPER::checkout() ) {
    return undef;
  }
  # Do a 'checkout' via a copy of a publically available version

  # Force / at end of access
  $access =~ s-([^/])$-$1/-;
  if ( ! -d $access) {
    ierror("No public package to copy available at $access\n");
    return undef;
  }

  # Copy the package
  #   Remember to exclude all repository administration files (CVS/, .svn/, SCCS/)
  #   This must match the method Asim::Package::Copy::update()

  print("Rsync from $access/ to $package_dir\n");

  $status =  system("rsync -av --exclude=CVS/ --exclude=.svn/ --exclude=.git/ --exclude=SCCS/ $access/ $package_dir");
  $status |= system("echo $access >$package_dir/COPY");

  if ($status) {
    ierror("Package rsync failed\n");
    return undef;
  }
  print("Package checked out at $package_dir\n") if ($DEBUG);

  #
  # Change the permissions on the directory just checked out,
  # to keep it private to the current user:
  #
  system("chmod 0700 $package_dir");

  #
  # Rehash the workspace since we have a new package in it
  #
  $Asim::default_workspace->rehash();

  #
  # We return the base directory so that the caller can 
  # see if it is in the path...since we do not add it.
  #
  return $package_dir;
}

################################################################################
#
# Internal error utility function
#
################################################################################

sub ierror {
  my $message = shift;

  print "Repository::Copy: Error - $message";

  return 1;
}


=back

=head1 BUGS

=head1 AUTHORS

Mohit Gambhir

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2009

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
