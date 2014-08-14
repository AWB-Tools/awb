#
# *****************************************************************************
# *
# * @brief Git.pm : utility methods to handle Git repositories
# *
# * @author Mohit Gambhir
# *
# * Copyright (c) 2009 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::Repository::Git;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::Git - Class to access and manipulate an Git repository.

=head1 SYNOPSIS

use Asim::Repository::Git;

Asim::Repository::Git::checkout();

=head1 DESCRIPTION

This is a class to allow access to an git repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here to check whether the package type in asim.pack is Git and set it to this subclass if so.  Set_type will return 
1 if it is a Git package.  If it returns 0, you should keep checking other repository types.

=cut

=head1 METHODS

The following methods are supported:

#################################################################################

=over 4

=item $git = Asim::Repository::Git::set_type( $package )

If $package is an Git repository, set the object's subclass to Asim::Repository::Git and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;
  # If method is git, then this is Subversion:
  if ( $self->{method} eq "git" ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of package - "git" in this case.

=cut

#################################################################################

sub type {
  return 'git';
}

#################################################################################

=item $git = Asim::Repository::Git:init()

Global init of Git module.

=cut

#################################################################################

sub init {
  # Deal with any Git environment variables
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

Check out a git repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || undef;

  my $access = $archive->{access} || return undef;
  if (defined($user)) {if ($access =~ m/:\/\//) {
    # URL contains protocol name. Username should be added afetr protocol
        $access =~ s/:\/\//:\/\/$user@/;
    }
    else {
        $access = $user . "@" . $access;
    }
  }
  my $tag    = $archive->{tag}    || return undef;
  my $target = $archive->{target} || return undef;
  my $pkg_name   = $archive->{packagename}  || return undef;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;
 
  # parse the tag into a branch name and csn
  my $ref_obj = '';
  $ref_obj = $tag;

  if ( ! -e "$package_dir/.git" ) {
    print "Local git repository $target does not exist!\n";
    my $q = "Do you want to clone it using URL $access?";
    if (Asim::choose_yes_or_no($q, "yes", "yes")) {
      return $self->clone($user);
    }
  }

  my $cmd = "(cd $package_dir; ".$self->repo_cmd()." checkout ";
  my $found = 0;
  open LIST, "cd $package_dir; ".$self->repo_cmd()." show-ref |";
  while ( <LIST> ) {
    if  ($ref_obj eq 'HEAD') {
      $cmd .= "master ";
      $found = 1;
      last;
    } elsif ( m/^(.+)(refs\/heads\/$ref_obj)$/ ) {
      #the branch to be checked out is a local branch
      $cmd .= "$ref_obj ";
      $found = 1;
      last;
    } elsif ( m/^(.+)(refs\/remotes\/$ref_obj)$/ ) {
      # Not a local branch. Create a new tracking local branch 
      $cmd .= "-b $ref_obj $2 --track ";
      $found = 1;
      last;
    } elsif ( m/^(.+)(refs\/remotes\/tags\/$ref_obj)$/ ) {
      # its a remote tag. Create a local branch with the tag
      $cmd .= "-b $ref_obj $2 ";
      $found = 1;
      last;
    } elsif ( m/^(.+)(refs\/tags\/$ref_obj)$/ ) {
      # its a local tag. Create a local branch with the tag
      $cmd .= "-b $ref_obj $2 ";
      $found = 1;
      last;
    } elsif ( m/^(.+)(refs\/remotes\/origin\/$ref_obj)$/ ) {
      $cmd .= "-b $ref_obj $2 --track ";
      $found = 1;
      last;
    }
  }

  if (! $found ) {
    # if the reference object is not found using git show-refs then it must be a commit
    # hash tag that does not refer to the latest commit in the master branch. 
    # Send the commit tag in the git checkout command and home that git will handle it 
    # appropriately
    $cmd .= "-b asim-release $ref_obj ";
  }
  
  close LIST;
  $cmd .= ")";
  
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

Clone a git repository. 
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
    if ($access =~ m/@/)
    {
        # URL already contains a username. Do not add one
    }
    elsif ($access =~ m/file:\/\/\//)
    {
        # URL uses file protocol which does not require username
    }
    elsif ($access =~ m/^( )*\//)
    {
        # URL starts with a / so it must be using file protocol which does not require username
    }
    elsif ($access =~ m/:\/\//) {
        # URL contains protocol name. Username should be added after protocol
        $access =~ s/:\/\//:\/\/$user@/;
    }
    else {
        $access = $user . "@" . $access;
    }
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
  my $cmd = "(cd $targetdir; ".$self->repo_cmd()." clone -v ";
  
  $cmd   .= "$url $target)";

  ### execute the command
  printf STDERR "Executing: %s\n", $cmd;
  $status = system($cmd);
  print("Package cloned at $package_dir\n") if ($DEBUG);

  if ($tag ne 'HEAD') {
    $self->checkout($user);
  }

  #
  # Change the permissions on the directory just checked out,
  # to keep it private to the current user:
  #
  #system("chmod 0700 $package_dir");

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

  print "Repository::Git: Error - $message";

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
