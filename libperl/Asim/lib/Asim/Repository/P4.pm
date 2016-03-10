#
# *****************************************************************************
# *
# * @brief P4.pm : utility methods to handle P4 repositories
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


package Asim::Repository::P4;
use warnings;
use strict;

use File::Basename;
use Sys::Hostname;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::P4 - Class to access and manipulate an P4 repository.

=head1 SYNOPSIS

use Asim::Repository::P4;

Asim::Repository::P4::checkout();

=head1 DESCRIPTION

This is a class to allow access to an p4 repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here
to check whether the package type in asim.pack is P4 and set it to this subclass if so.
Set_type will return  1 if it is a P4 package.  
If it returns 0, you should keep checking other repository types.

=cut

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $p4 = Asim::Repository::P4::set_type( $package )

If $package is an P4 repository, set the object's subclass to Asim::Repository::P4 and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;

  # If method is p4, then this is p4:

  if ( $self->{method} eq "p4" ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of repository - "p4" in this case.

=cut

#################################################################################

sub type {
  return 'p4';
}

#################################################################################

=item $p4 = Asim::Repository::P4:init()

Global init of P4 module.

=cut

#################################################################################

sub init {
  # Deal with any P4 environment variables
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

Check out a p4 repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || undef;

  my $access = $archive->{access} || return undef;
  my $tag    = $archive->{tag}    || return undef;
  my $target = $archive->{target} || return undef;
  my $pkg_name   = $archive->{packagename}  || return undef;

  my $status;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  # Do any special handling to for the username - $user
  # Currently P4 does nothing with $user

  # Create a clientname for p4
  my $p4clientname = "awb-" . hostname() . "." . basename(Asim::rootdir()) . ".$target";

  print "Clientname = $p4clientname\n";

  # Parse access into server and path

  my $p4server;
  my $p4path;

  ($p4path, $p4server) = split(/@/, $access);

  print "Path = $p4path\nServer= $p4server\n";

  # TBD: There needs to be more error checking here! 
  #
  #   - Need to check $target was created successfully
  #   -  Need to check P4 server exists
  #   - Need to check client creation was successful ...

  # Create the target directory

  print "Create the directory for the package: $package_dir\n";

  mkdir($package_dir);

  # Create the .p4client file

  print "Create .p4client file\n";

  my $p4configfile = $ENV{"P4CONFIG"} || "Undefined";

  open P4CLIENT, ">$package_dir/$p4configfile";
  print P4CLIENT "P4PORT=$p4server\n";
  print P4CLIENT "P4CLIENT=$p4clientname\n";
  close P4CLIENT;

  # Create a p4 client on the server

  open P4CLIENT, "|p4 client -i";
  print P4CLIENT "# Perforce client auto-generated from AWB\n";
  print P4CLIENT "\n";
  print P4CLIENT "Client:\t$p4clientname\n";
  print P4CLIENT "\n";
  print P4CLIENT "Owner:\t$user\n";
  print P4CLIENT "\n";
  print P4CLIENT "Host:\t". hostname. "\n";
  print P4CLIENT "\n";
  print P4CLIENT "Description:\n";
  print P4CLIENT "    Client for Created by AWB on behalf of $user\n";
  print P4CLIENT "\n";
  print P4CLIENT "Root:	$package_dir\n";
  print P4CLIENT "\n";
  print P4CLIENT "Options: noallwrite noclobber nocompress unlocked nomodtime normdir\n";
  print P4CLIENT "\n";
  print P4CLIENT "SubmitOptions:\tsubmitunchanged\n";
  print P4CLIENT "LineEnd:\tlocal\n";
  print P4CLIENT "View:\n";
  print P4CLIENT "\t$p4path/... //$p4clientname/...\n";
  close P4CLIENT;

  my $cmd = "(cd $package_dir; " . $self->repo_cmd() . " sync)";
  
  printf STDERR "Executing: %s\n", $cmd;
  $status = system($cmd);

  print("Package checked out at $package_dir\n") if ($DEBUG);
 
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

  print "Repository::P4: Error - $message";

  return 1;
}


=back

=head1 BUGS

=head1 AUTHORS

Joel Emer, Mohit Gambhir

=head1 COPYRIGHT

Copyright (c) 2016 Nvidia Corporation
Copyright (c) 2009 Intel Corporation

=cut

1;
