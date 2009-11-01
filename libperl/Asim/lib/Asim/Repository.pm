#
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
#


package Asim::Repository;
use warnings;
use strict;

use Asim::Repository::Svn;
use Asim::Repository::Cvs;
use Asim::Repository::BitKeeper;
use Asim::Repository::Copy;
use Asim::Repository::Git;

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});


=head1 NAME

Asim::Repository - Library for manipulating a CVS/Svn/Bitkeeper/Git repositry containing an Asim repository.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item $repository = Asim::Repository-E<gt>new(packagename => <name of package>,
                                        method => [cvs|svn|bitkeeper|git],
                                        access => <accessinfo>,
                                        module => <CVSmodule>,
                                        tag => <CVStag>,
                                        browseURL => <browseURL>,

                                        target => <targetdirectory>,
                                        changes=> <changesfilename>);


Create a new repository object.

=cut

################################################################

sub new {
  my $this = shift;

  my $class = ref($this) || $this;
  my $self;

  $self = {@_};

  bless	$self, $class;

  #
  # Determine what type of repository we have
  #
  Asim::Repository::Copy::set_type     ( $self ) ||
  Asim::Repository::Cvs::set_type      ( $self ) ||
  Asim::Repository::BitKeeper::set_type( $self ) ||
  Asim::Repository::Svn::set_type      ( $self ) ||
  Asim::Repository::Git::set_type      ( $self );

  return $self;
}

################################################################

=item $repository-E<gt>create()

Create a new package repository. 

=cut

################################################################

sub create {
  print <<'EOF';

Perform the following manual steps:

1) Create a new package, maybe with something like:

       % asim-shell new package <pkgname>


2) If necessary, create a CVS repository probably with something like:

       % cvs -d <name>@<node>:/cvsroot/<dir> init

   This might not be necessary if you use something like sourceforge which
   will create a cvs repository for you.


3) By default the CVS module name will be <dir> as specified above.
   If you want this package to be be a module of the repository with another name
   you may need to update CVSROOT/admin/modules to include package as a CVS module.

   See the CVS instructions for instructions...


4) Add the package to the asim core package and re-install it.

       a) Add package to <workspace>/src/asim-simcore/etc/asim.pack file.

       b) Check in changes to asim-simcore.

       c) Reinstall asim-simcore. Probably that means:
               ./configure
               make
               make install

          Check <workspace>/src/asim-simcore/INSTALL if you want to be sure.


5) Import the package with something similar to:

       % cd <workspaceroot>/src/asim-<pkgname>
       % cvs -d <name>@<node>:/cvsroot/<dir> import asim-<pkgname> <vendor> start


6) If you want anonymous access:

       % # Check out CVSROOT
       % cd <scratch-directory>
       % cvs -d <name>@<node>:/cvsroot/<dir> checkout CVSROOT
       % cd CVSROOT

       % # Create the 'readers' file
       % echo 'anonymous' >readers
       % cvs add readers
       % touch passwd

       % # Create the 'writers' file
       % touch writers
       % cvs add writers

       % # Create the 'passwd' file
       % touch passwd
       % cvs add passwd
       % htpasswd -c passwd anonymous
       Password: <secret-password>
       % # make sure passwd file looks like "anonymous:sh32njsgjs"

       % # Commit changes
       % cvs commit

7) To work on this package you need to delete it and check it out, e.g.:

       % rm -rf <workspaceroot>/src/asim-<pkgname>
       % asim-shell checkout package <pkgname>


EOF

return 1;
}

################################################################

=item $dir = $repository-E<gt>checkout([$user])

Perform necessary checks before checking out a repository. 

=cut

################################################################
sub checkout {

  my $self = shift;
  my $archive = $self;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  # First check if the target dir exists
  # and if it is writable

  if (! -d "$targetdir" || ! -w "$targetdir") {
    ierror("Malformed workspace - check if src directory is present and if it is writable.\n");
    return undef;
  }

  # Make sure we have a packagename

  if (! defined($self->packagename())) {
    ierror("No packagename for this package.\n");
    return undef;
  }

  # Remove pre-existing package

  if (-d $package_dir) {
    print "Removing old version of package\n";
    system("rm -rf $package_dir");
  }
  
  print "Beginning checkout\n";

}

################################################################

=item $dir = $repository-E<gt>clone([$user])

Clone a distributed repository. 
Returns the directory the result was cloned into.

=cut

################################################################
sub clone {
  
  my $self = shift;
  my $archive = $self;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  # First check if the target dir exists
  # and if it is writable

  if (! -d "$targetdir" || ! -w "$targetdir") {
    ierror("Malformed workspace - check if src directory is present and if it is writable.\n");
    return undef;
  }

  # Make sure we have a packagename

  if (! defined($self->packagename())) {
    ierror("No packagename for this package.\n");
    return undef;
  }

  # Remove pre-existing package
  if (-d $package_dir) {
    print "Removing old local repository\n";
    system("rm -rf $package_dir");
  }

  print "Beginning clone\n";
}


################################################################

=item $workspace-E<gt>browse()

Open a grapichal browser on the repository

=cut

################################################################

sub browse {
  my $self = shift;
  my $location = $self->{browseURL} || return undef;

  #
  # TBD: Get repository browser from configuration option
  #

  my $status = system ("firefox $location &");
  return $status?undef:1;

}


################################################################

=item $packagename = $repository-E<gt>packagename()

Returns the name of the package stored in this repository

=cut

################################################################

sub packagename {
  my $self = shift;
  my $packagename;

  $packagename = $self->{packagename} || return undef;

  return $packagename;
}

################################################################

=item $checkoutdir = $repository-E<gt>checkoutdir()

Returns the directory the repository would be checked out into.

=cut

################################################################

sub checkoutdir {
  my $self = shift;
  my $target_dir;

  $target_dir = $self->{target} || return undef;

  # Note:
  #  Portions of checkout depend on exactly this concatentation...

  return $self->_checkoutbasedir() . "/$target_dir";
}


sub _checkoutbasedir {
  my $self = shift;

  return $Asim::default_workspace->src_dir();
}

################################################################

=item $modDB-E<gt>dump()

Textually dump the contents of the object

=cut

################################################################


sub dump {
  my $self = shift;
  my $level = shift;

  printf "Repository type: %s\n",             _safe_string($self->{method});
  printf "Repository access: %s\n",           _safe_string($self->{access});
  printf "Repository module: %s\n",           _safe_string($self->{module});
  printf "Repository tag: %s\n",              _safe_string($self->{tag});
  printf "Repository browseURL: %s\n",        _safe_string($self->{browseURL});
  printf "Repository target directory: %s\n", _safe_string($self->{target});
  printf "Repository changes file: %s\n",     _safe_string($self->{changes});

}


sub _safe_string {
  my $value = shift;

  if (! defined($value)) {
     return "undef";
  }

  return $value;
}

################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Repository: Error - $message";

  return 1;
}


=back

=head1 BUGS

The complex (and unchecked) arguments to 'new' makes me uncomfortable
with it as a public interface. At present only RepositoryDB needs to call
it, but still...

Checkouts go to default target directory. Nor does checkout allow non-anonymous checkouts.
Checkout has very little error checking...


=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
