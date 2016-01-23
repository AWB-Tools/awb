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
use Asim::Repository::Hg;
use Asim::Repository::P4;
use Asim::Repository::Public;

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});


=head1 NAME

Asim::Repository - Library for manipulating a CVS/Svn/Bitkeeper/Git/Hg/P4 repositry containing an Asim repository.

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
                                        method => [cvs|svn|bitkeeper|git|hg],
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
  Asim::Repository::Git::set_type      ( $self ) ||
  Asim::Repository::Hg::set_type       ( $self ) ||
  Asim::Repository::P4::set_type       ( $self ) ||
  Asim::Repository::Public::set_type   ( $self ) ;

  return $self;
}

################################################################

=item $repository-E<gt>create()

Create a new package repository. 

=cut

################################################################

sub create {
  my $self = shift;

  ierror("Internal error Asim::Repository:Create should have been implemented in subtype\n");
  return undef;
}


################################################################

=item $repository-E<gt>create_packfile($name)

Create a new packfile named $name for this repository. 

FIXME: This method knows too much about the internal structure
       of the repository object and the structure of the packfile

=cut

################################################################

sub create_packfile {
  my $self = shift;
  my $name = shift;

  my $packfile;
  my $status;

  $packfile = "$ENV{HOME}/.asim/repositories.d/$name.pack";

  if (-e $packfile) {
    print "Packfile $packfile already exists\n";
    return undef;
  }
  
  #
  # Create packfile and fill in with values
  #

  my $inifile = Asim::Inifile->new();

  #
  # FIXME: Don't rely on knowing values from hash
  # FIXME: Get a real description from somewhere
  #

  foreach my $i ("$name", "$name/HEAD") {
    $inifile->put($i, "Description", "$name package");
    $inifile->put($i, "Method",      $self->{method});
    $inifile->put($i, "Access",      $self->{access});
    $inifile->put($i, "Module",      $self->{module});
    $inifile->put($i, "Tag",         $self->{tag});
    $inifile->put($i, "Target",      $self->{target});
    $inifile->put($i, "BrowseURL",   $self->{browseURL});
    $inifile->put($i, "Changes",     $self->{changes});
  }

  $status = $inifile->save($packfile);

  if (! defined($status)) {
    print("Packfile ($packfile) save failed\n");
    return undef;
  }

  return 1;
}

#################################################################################

=item $repository-E<gt>repo_cmd()

Return the repository access command (cvs, svn, git, ...) to use.

It defaults to 'method' if unspecified.

=cut

#################################################################################

sub repo_cmd {
  my $self = shift;
  return $self->{repo_cmd} if defined $self->{repo_cmd};
  return $self->{method};
}

#################################################################################

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

=item $tag = $repository-E<gt>tag()

Returns the tag of the package stored in this repository

=cut

################################################################

sub tag {
  my $self = shift;
  my $tag;

  $tag = $self->{tag} || return undef;

  return $tag;
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
  printf "Repository access command: %s\n",   _safe_string($self->{repo_cmd});

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
