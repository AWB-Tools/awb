#
# *****************************************************************************
# *
# * @brief Public.pm : utility methods to handle copy repositories
# *
# * @author Joel Emer
# *
# * Copyright (c) 2011 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::Repository::Public;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::Public - Class to access and manipulate a public repository.

=head1 SYNOPSIS

use Asim::Repository::Public;

Asim::Repository::Public::checkout();

my $repo = Asim::Repository::Public->new("~/public-package");

$copy->update();


=head1 DESCRIPTION

This is a class to allow access to a public repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here to check whether the package type in public
set it to this subclass if so.  Set_type will return 
1 if it is a public package.  If it returns 0, you should keep checking other repository types.

=cut

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $copy = Asim::Repository::Public::set_type( $package )

If $package is a public repository, set the object's subclass to 
Asim::Repository::Public and return 1. Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;

  #
  # If method is public, then this is a public package:
  #
  if ( $self->{method} eq "public" ) {
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
  return 'public';
}

#################################################################################

=item $copy = Asim::Repository::Public:init()

Global init of Public module.

=cut

#################################################################################

sub init {
  # Deal with any Public environment variables
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
  my $package_dir = $self->{access};

  #
  # Check if publically installed package actually exists
  #
  if (! -d $package_dir ) {
    return undef;
  }
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

  print "Repository::Public: Error - $message";

  return 1;
}


=back

=head1 BUGS

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2009

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
