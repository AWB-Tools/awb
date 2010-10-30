#
# *****************************************************************************
# *
# * @brief BitKeeper.pm : utility methods to handle BK repositories
# *
# * @author Mohit Gambhir
# *
# * Copyright (c) 2009 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::Repository::BitKeeper;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::BitKeeper - Class to access and manipulate an BitKeeper repository.

=head1 SYNOPSIS

use Asim::Repository::BitKeeper;

Asim::Repository::BitKeeper::checkout();

=head1 DESCRIPTION

This is a class to allow access to an bk repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here to check whether the package type in asim.pack is BK and set it to this subclass if so.  Set_type will return 
1 if it is a BK package.  If it returns 0, you should keep checking other repository types.

=cut 

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $bk = Asim::Repository::BitKeeper::set_type( $package )

If $package is an BK repository, set the object's subclass to Asim::Repository::BitKeeper and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;
  # If method is bitkeeper, then this is a BK repository:
  if ( $self->{method} eq "bitkeeper" ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of package - "bk" in this case.

=cut

#################################################################################

sub type {
  return 'bitkeeper';
}

#################################################################################

=item $bk = Asim::Repository::BitKeeper:init()

Global init of BitKeeper module.

=cut

#################################################################################

sub init {
  # Deal with any BK environment variables
  1;
}

#################################################################################

=item $dir = $repository-E<gt>checkout([$user])

Check out a bitkeeper repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || "anonymous";

  my $access = $archive->{access} || return undef;
  my $tag    = $archive->{tag}    || return undef;
  my $module = $archive->{module} || return undef;
  my $target = $archive->{target} || return undef;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;

  # Call Asim::Repository::checkout to perform pre checkout validation
  if ( ! $self->SUPER::checkout() ) {
    return undef;
  }
  # FIX!
  # deal with branches, which in the case of BK are just separate clones...

  my $url = "$access";
  if ($user ne "anonymous") {
    $url =~ s/anonymous/$user/;
  }
  my $cmd;
  if ($tag eq "HEAD") {
    $cmd ="(cd $targetdir; bk clone $url $target)";
  } else {
    $cmd ="(cd $targetdir; bk clone -r$tag $url $target)";
  }
  printf STDERR "Executing: %s\n", $cmd;
  $status = system($cmd);

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

  print "Repository::BK: Error - $message";

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
