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

package Asim::Package::Git;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Package);

=head1 NAME

Asim::Package::Git - Class to manipulate a checked out Git repository.

=head1 SYNOPSIS

use Asim::Package::Git;

Asim::Package::Git::init();

my $git = Asim::Package::Git->new("~/gitdir");

$git->push();

$git->pull();


=head1 DESCRIPTION

This is a class to allow a directory to be treated as 
a Git repository. 

This is a subclass of Asim::Package.  After creating an instance of Asim::Package,
you can call the set_type() method here to check whether the package is in a Git
repository and set it to this subclass if so.  Set_type will return 1 if it is a
Git package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

=item $cvs = Asim::Package::Git::set_type( $package )

If $package is an Git repository, set the object's subclass to Asim::Package::Git and return 1.
Otherwise return 0.

=cut

sub set_type {
  my $self = shift;
  my $location = $self->location();
  # If there is a ".git" subdirectory, then this is Subversion:
  if (-e "$location/.git") {
    bless $self;
    return 1;
  }
  return 0;
}

=item $package-E<gt>type()

Return type of package - "git" in this case.

=cut

sub type {
  return 'git';
}

=item $git = Asim::Package::Git:init()

Global init of Git module.

=cut

sub init {
  # Deal with any Git environment variables
  1;
}

=item $git-E<gt>pull([version])

Fetch form or merge with another repository or local branch.

If an optional version string is specified,
update the package to that version.
The version specifier can be 'HEAD', the name of a branch or the name of a tag.

=cut

sub pull {
  my $self = shift;
  my $version = shift || $self->get_current_branch();

  my $location = $self->location();
  my $url = $self->{url} || "origin";
  my $command = "pull $url $version";

  my $status = $self->git($command);

  if ($status) {
      return undef;
  }

  # Also do a git-remote update to fetch all newly created branches
  
  $command = "remote update -p";

  $status = $self->git($command);
  if ($status) {
   iwarn("Could not update new branch information.\n" .
         "There may be new branches in remote repository that were not fetched\n");
  }
  return 1;
}

=item $git-E<gt>update([version])

Update the package by pulling from parent repository.

=cut

sub update {
  
  my $self = shift;

  return $self->pull();
}

=item $git-E<gt>commit([version])

Commit local changes in the workspace to the local git repository

=cut

sub commit {
  my $self = shift;
  my $only_self = shift || 0;
  my $commitlog_file = shift;

  print "Commit: Starting a commit on package: " . $self->name() . "\n";

  ## Sanity stage
  $self->SUPER::sanity_stage($commitlog_file) 
+1    || return();

  my $TMPDIR = Asim::get_tmpdir();

  if ( ! defined($TMPDIR)) {
    commit_failure("Sorry: neither \$ASIMTMPDIR or \$TMPDIR were not found and /tmp does not exist");
    return 0;
  }

  # Check stage
  our $checkindex++;


 $self->commit_check($commitlog_file)
   || commit_failure("Cannot commit from this state") && return 0;

  print "Commit: Status " . $self->{status} . "\n";

  if ($self->{status} eq "No-commit-needed") {
    return 0;
  }

  $self->isprivate()
    || commit_failure("You are using a shared package that is not up-to-date.\n" .
                      "Check out the package or get the shared package updated.\n") &&  return 0;

  # Prepare stage
  # git commit does not increment csn or modify the changes file. It will be done at
  # push time
  
  # If there is a commit log file then write its contents into the comment file
  # otherwise invoke users favorite editor to write commit comments
   my $tmpfile_comment = "$TMPDIR/asimcommit_comment.$$.$checkindex.txt";
   if (! defined($commitlog_file)) {
      Asim::invoke_editor("--eof", $tmpfile_comment)
        || commit_failure("Could not edit changes file") && return undef;
    } else {
      system("cat $commitlog_file >>$tmpfile_comment");
    }

   # Actual commit stage
   print "Commit: I am about to commit your changes. \n\n";

  if (! Asim::choose_yes_or_no("Do you really want to proceed and commit","response_required","yes")) {
    $self->{status} = "";
    system ("rm -f $tmpfile_comment");
    return 0;
  }

  # Do the actual commit
  my $command = "commit -a -F $tmpfile_comment";
    if (! $self->git_command($command)) {
      ierror("Commit: Fatal error during git commit: \n".
             "Commit: Your commit might be partially done \n",
             "Commit: and the state of your directory is probably messed up.\n");
      system ("rm -f $tmpfile_comment");
      return 0;
    }
  
  system ("rm -f $tmpfile_comment");
  return 1;
  
}

=item $git-E<gt>commit_check()

A set of checks to make sure that we have a 
reasonable chance of succeeding at a commit

=cut

sub commit_check {
  my $self = shift;
  my $reportfile = shift || "/dev/null";

  print "Commit: Checking the status of your source...\n";

  $self->{status} = "No-commit-needed";
  $self->{must_have_regression} = 0;
  $self->{regtestdir} = undef;

  my @files = $self->status();

  if (! defined($files[0])) { 
    # if no files came up in git status then the workspace is upto date
    $self->{status} = "No-commit-needed";
    return 1;
  }
  
  my $fail = 0;

  foreach my $i (@files) {
    my ($file, $status) = @{$i};
    my $VERBOSE = 1;

    if ( $status =~ /unmerged/ || $status =~ /unknown/)
    {
      print ("Commit: File: $file has unacceptable state \"$status\" \n");
      $fail = 1;
    }
    else {
      $self->{status} = "Commit-needed";
    }
  }

  if ($fail) {
    print "\n";
    print "Commit: Sorry, one or more files are not up-to-date with respect to the repository\n";
    print "Commit: Before attempting to commit again try:\n";
    print "\n";
    print "      (1) update package " . $self->name() . "\n";
    print "      (2) merge your changes (if any)\n";
    print "      (3) re-run the regression test\n";
    print "      (4) re-run commit\n\n";

    $self->{status} = "Not-ready-to-commit";
    return 0;
  }

  return 1;
}

=item $git-E<gt>commit_check()

A set of checks to make sure that we have a 
reasonable chance of succeeding at a push

=cut

sub push_check {
  my $self = shift;

  $self->{status} = "Commit-needed";
  return 1;
}

=item $git-E<gt>status()

Check on the Git status of each file in the current package

Return a array with one element for each file returned 
by 'git status' addin state:

Each array element contains:

 ($directory, $filename, $status, $reprev, $date)

=cut

sub status {
  my $self = shift;
  my $location = $self->location();
  my @files = ();

  # Execute 'git status' to see all the changes that have been made.
  
  my $tmp_git_status = "/tmp/awb-shell-git-status.$$";

  system("cd $location; git status >$tmp_git_status 2>&1");
 
  if (! CORE::open(Git,"< $tmp_git_status") ) {
    Asim::Package::ierror("status: Can't launch 'git status 1': $!\n");
    return undef;
  }

  # Parse the Git status

  my $fail = 0;

  my $VERBOSE = 0;

  while (<Git>) {
    my $file = "";
    my $status = "";

    print "git status returned: $_" if ( $VERBOSE );
    #
    # Hack in case thremotes/origin/mastere ssh command asks for a password
    #
    if ( /password:/) {
      print "$_\n";
    }
    
    if ( /^#\s+(.+):\s+(.+)$/ ) {
      $status = $1;
      $file = $2;
    }
    
    if ($status) {
       push(@files, [$file, $status]);
    }
  }
  CORE::close(Git);
  system("rm $tmp_git_status");
  
  return (@files);
}


=item $git->git_command( <command> )

Issue an Git command robustly.

=cut

sub git_command {
  my $self = shift;
  my $command = shift;

  our $NUM_RETRY;

  my $ret;

  #
  # Command retries for cvs
  #
  if (! defined($NUM_RETRY)) {
    $NUM_RETRY = 5;
  }

  #
  # Try the command -- repeat a few times on failures
  #
  foreach my $retry (1..$NUM_RETRY) {
    $ret = $self->git($command);
    if ($ret == 0) {
      last;
    }

    if ($retry < $NUM_RETRY) {
      printf "Caught error during git operation - retrying %d more times\n",
      $NUM_RETRY - $retry;
      sleep 5;
    } else {
      return 0;
    }
  }

  return 1;
}

=item $git->git( <command> )

Issue an GIT command directly.  Don't retry if it fails.

=cut

sub git {
  my $self = shift;
  my $command = shift;
  #
  # Make sure we are at the root of the checked out files
  #
  my $location = $self->location; 
  my $ret;

  my $VERBOSE = 1;

  #
  # Now do it
  #
  $command = "(cd $location; git $command)";
  print "$command\n" if $VERBOSE;

  $ret = system("$command") >> 8;

  return $ret;
}

=item $git->is_public_repo( <url> )

Check to see if the URL is that of a public git repository

=cut

sub is_public_repo {
  my $self = shift;
  my $url = shift;

  if ($url =~ m/testgitbare/) {
  #if ($url =~ m/cameroon.intel.com/) {
    return 1;
  }

  return 0;
}

=item $git->increment_csn( )

Get current CSN tag and increment the revision number by 1

=cut

sub increment_csn {
  my $self = shift;
  my $location = $self->location();
  my $csn;
  my $rev;
 
  # Force a pull to ensure we get the latest csn in the log
  #system("cd $location; git pull");
  #$csn = `cd $location && git describe --tags --abbrev=0`;
  
  if ( $csn =~ m/^CSN-.*-(\d+)$/ ) {
    $rev = $1+1;
  } else {
    # Last tag generated was not a CSN tag. Try again to retrive the last CSN tag
    open LIST, "cd $location && git tag -l | ";
    # git tag generates the list in alphabetical order. It would have been much 
    # easier if the list was in chronological order. Find the latest revision 
    # number
    $rev = 0;
    while (<LIST>) {
      if ( m/^CSN-.*-(\d+)$/ ) {
        if ($rev <= $1) {
          $rev = $1+1;
        }
      }
    }
    close LIST;
  } 
  
  # increment the revison number, and replace it in the tag
  my $name = $self->name();
  my $branch = $self->{branch}; 
  if ($branch eq 'master') {
    $csn = "CSN-$name-$rev";
  } else {
    $csn = "CSN-$branch-$rev";
  }

  # return the updated tag
  return $csn;
}

=item $git->get_current_branch( )

Get currently active branch from the working copy

=cut

sub get_current_branch {
  my $self = shift;

  my $location = $self->location();
  open LIST, "cd $location && git branch -a |";
  while (<LIST>) {
    if ( m/^\*\s+(.+)/ ) {
      return $1;
    }
  }
  close LIST;
  return undef;
}

=head1 AUTHORS

Mohit Gambhir based on Svn.pm
  Sailashri Parthasarathy, Carl Beckmann based on Bitkeeper.pm by
    Oscar Rosell based on Cvs.pm by
      Joel Emer, Roger Espasa, Artur Klauser and  Pritpal Ahuja

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2009

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
