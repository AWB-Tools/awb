#
# *****************************************************************************
# *
# * @brief Svn.pm : utility methods to handle SVN repositories
# *
# * @author Mohit Gambhir
# *
# * Copyright (c) 2009 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::Repository::Svn;
use warnings;
use strict;

use File::Basename;
use File::Temp qw/ tempfile tempdir /;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::Svn - Class to access and manipulate an Svn repository.

=head1 SYNOPSIS

use Asim::Repository::Svn;

Asim::Repository::Svn::checkout();

=head1 DESCRIPTION

This is a class to allow access to an svn repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here to check whether the package type in asim.pack is SVN and set it to this subclass if so.  Set_type will return 
1 if it is a SVN package.  If it returns 0, you should keep checking other repository types.

=cut

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $svn = Asim::Repository::Svn::set_type( $package )

If $package is an SVN repository, set the object's subclass to Asim::Repository::Svn and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;
  # If method is svn, then this is Subversion:
  if ( $self->{method} eq "svn" ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of package - "svn" in this case.

=cut

#################################################################################

sub type {
  return 'svn';
}

#################################################################################

=item $svn = Asim::Repository::Svn:init()

Global init of Svn module.

=cut

#################################################################################

sub init {
  # Deal with any SVN environment variables
  1;
}

#################################################################################

=item $dir = $repository-E<gt>create()

Create a repository from a package.

=cut

################################################################################

sub create {
  my $self    = shift;
  my $package = shift || return undef;

  my $type = $self->type();
  my $url = $self->{access};

  my $name = $package->name();
  my $location = $package->location();

  my $tmpdir;
  my $status;

  #
  # Create a packfile for the new repository
  #

  print "Creating packfile\n";

  $status = $self->create_packfile($name);
  return undef if (! $status);

  #
  # Create skeleton repository contents and import them
  #

  print "Creating skeleton of repository\n";

  $tmpdir = tempdir(CLEANUP => 1);

  mkdir("$tmpdir/trunk");
  mkdir("$tmpdir/tags");
  mkdir("$tmpdir/branches");

  $status = system("rsync -av --exclude .svn $location/ $tmpdir/trunk");
  return undef if ($status);

  print "Importing skeleton into repository\n";

  $status = system("svn import -m 'Repository skeleton' $tmpdir $url");
  return undef if ($status);

  return 1;
}


#################################################################################

=item $dir = $repository-E<gt>checkout([$user])

Check out a svn repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || "anonymous";

  my $access = $archive->{access} || return undef;
  my $tag    = $archive->{tag}    || return undef;
  my $target = $archive->{target} || return undef;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;

  # Call Asim::Repository::checkout to perform pre checkout validation
  if ( ! $self->SUPER::checkout() ) {
    return undef;
  }

  # first parse the tag into a branch name and CSN number
  
  my $tag_branch = '';
  my $tag_csn = 0;
  if      ( $tag =~ m/^CSN-(.+)-([0-9\.]+)$/ ) {
    $tag_branch = $1;
    $tag_csn    = $2;
  } elsif ( $tag =~ m/^(.+):([0-9\.]+)$/ ) {
    $tag_branch = $1;
    $tag_csn    = $2;
  } elsif ( $tag =~          m/^([0-9\.]+)$/ ) {
    $tag_csn    = $1;
  } elsif ( $tag =~                m/^(.+)$/ ) {
    $tag_branch = $1;
  }
  if ( $tag_branch eq $self->packagename() ) {
    $tag_branch = '';
  }
  
  ### get the URL
  ### FIXME: modify svn method to check if there is any distributed repository in the workspace

  print("Checkout: branch=($tag_branch) csn=($tag_csn)\n") if ($DEBUG);

  my $url = "$access";
  if ( ! $tag_branch || $tag_branch eq 'HEAD' ) {
    $url .= '/trunk';
  } else {

    # Non-head checkout,
    # where tag is a general string.
    # Look in both "tags" and "branches" to find the label and check that one out:
    my $is_tag_or_branch = 0;
    foreach my $subdir ( 'tags', 'branches' ) {
      open LIST, "svn list $url/$subdir |";
      while ( <LIST> ) {
        if ( m/^$tag_branch\// ) {
          # Pick up the tag directory
          $url .= "/$subdir/$tag_branch";
          $is_tag_or_branch = 1;
	  last;
        }
      }
      
      close LIST;
      last if ($is_tag_or_branch);

    }
    # couldn't find it in either tags or branches?
    # Tag must be a date, so try checking out using it as a date string:
    if ( ! $is_tag_or_branch ) {
      if ( $tag_csn ) {
        ierror("branch or tag $tag_branch not found!\n");
      } else {
        $url .= '/trunk';
        $tag_csn = "{\"$tag\"}";
      }
    }
  }
    
  print "$tag_csn\n";
  print "$tag_branch\n";
  ### construct the checkout command
    
  my $cmd = "(cd $targetdir; svn checkout ";
  if ( $tag_csn ) {
    $cmd .= "-r $tag_csn ";
  }
  $cmd   .= "--username $user $url $target)";
  
  ### execute the command
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

  print "Repository::Svn: Error - $message";

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
