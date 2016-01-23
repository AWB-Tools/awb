#
# *****************************************************************************
# *
# * @brief Hg.pm : utility methods to handle Hg (Mercurial) repositories
# *
# * @author Joel Emer
# *
# Copyright (c) 2016 Nvidia Corporation
# Copyright (c) 2009 Intel Corporation
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
# *****************************************************************************
#

package Asim::Repository::Hg;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::Hg - Class to access and manipulate an Hg repository.

=head1 SYNOPSIS

use Asim::Repository::Hg;

Asim::Repository::Hg::checkout();

=head1 DESCRIPTION

This is a class to allow access to an hg repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here
to check whether the package type in asim.pack is hg and set it to this subclass if so.
Set_type will return  1 if it is a hg package.  
If it returns 0, you should keep checking other repository types.

=cut

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $hg = Asim::Repository::Hg::set_type( $package )

If $package is an Hg repository, set the object's subclass to Asim::Repository::Hg and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;
  # If method is hg, then this is Subversion:
  if ( $self->{method} eq "hg" ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of package - "hg" in this case.

=cut

#################################################################################

sub type {
  return 'hg';
}

#################################################################################

=item $hg = Asim::Repository::Hg:init()

Global init of Hg module.

=cut

#################################################################################

sub init {
  # Deal with any Hg environment variables
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

Check out a hg repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || undef;

  my $access = $archive->{access} || return undef;

  if (defined($user)) {
    # Add username to protocols that can use it

    $access = add_user($access, $user);
  }

  my $tag    = $archive->{tag}    || return undef;
  my $target = $archive->{target} || return undef;
  my $pkg_name   = $archive->{packagename}  || return undef;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;
 
  if ( ! -e "$package_dir/.hg" ) {
    print "Local hg repository $target does not exist!\n";
    my $q = "Do you want to clone it using URL $access?";
    if (Asim::choose_yes_or_no($q, "yes", "yes")) {
      return $self->clone($user);
    }
  }

  # TBD: match tag to branch names for checkout

  if ($tag ne "HEAD") {
    ierror("Asim::Repository::Hg: Unsupported tag: $tag\n");
    return undef;
  }

  my $cmd = "(cd $package_dir; ".$self->repo_cmd()." checkout )";

  printf STDERR "Executing: %s\n", $cmd;
  $status = system($cmd);
  print("Package checked out at $package_dir\n") if ($DEBUG);
 
  # We return the base directory so that the caller can 
  # see if it is in the path...since we do not add it.
  #
  return $package_dir;
  
}

#################################################################################

=item $dir = $repository-E<gt>clone([$user])

Clone a hg repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub clone {

  my $self = shift;
  my $archive = $self;
  my $user = shift || undef;

  # URL of the repository to be cloned
  my $access = $archive->{access} || return undef;

  if (defined($user)) {
    # Add username to protocols that can use it

    $access = add_user($access, $user);
  }

  my $tag    = $archive->{tag}    || return undef;
  my $target = $archive->{target} || return undef;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;

  # Call Asim::Repository::checkout to perform pre checkout validation

  if ( ! $self->SUPER::clone() ) {
    return undef;
  }

  my $url = "$access";
  my $cmd = "(cd $targetdir; ".$self->repo_cmd()." clone $url $target )";
  
  ### execute the command
  printf STDERR "Executing: %s\n", $cmd;
  $status = system($cmd);
  print("Package cloned at $package_dir\n") if ($DEBUG);

  if ($tag ne 'HEAD') {
    $self->checkout($user);
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
# Utility function to add username into access URL
#
################################################################################

sub add_user {
  my $access = shift;
  my $user = shift;

  if ($access =~ m/@/) {
    # URL already contains a username. Do not add one
  }
  elsif ($access =~ m/file:\/\/\//) {
    # URL uses file protocol which does not require username
  }
  elsif ($access =~ m/^( )*\//) {
    # URL starts with a / so it must be using file protocol which does not require username
  }
  elsif ($access =~ m/:\/\//) {
    # URL contains protocol name. Username should be added after protocol
    $access =~ s/:\/\//:\/\/$user@/;
  }

  return $access;
}

################################################################################
#
# Internal error utility function
#
################################################################################

sub ierror {
  my $message = shift;

  print "Repository::Hg: Error - $message";

  return 1;
}


=back

=head1 BUGS

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Nvidia Corporation, 2016
Copyright (c) Intel Corporation, 2009

=cut

1;
