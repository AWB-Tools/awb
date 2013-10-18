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


# Commit wide variables defined in  Commit.pm

our $HOSTNAME;
our $DATE;
our $DATE_UTC;

our $TMPDIR;
our $HOST;
our $USER;

our $PROBLEM;

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

If an optional branch string is specified,
update the package to that branch.
The version specifier can be the name of a branch or the name of a tag.

=cut

sub pull {
  my $self = shift;
  my $branch = shift || $self->get_current_branch();

  my $url = $self->{url} || "origin";
  my $location = $self->location();
  my $found = 0;
  # check if the branch exists in the remote repository
  open LIST, "cd $location; git ls-remote --heads $url |";
  while (<LIST>) {
    if ( m/^(.+)(refs\/heads\/$branch)$/ ) {
      $found = 1;
      last;
    }
  }
  close LIST;
  if (! $found ) {
    # branch does not exist in the remote repository
    Asim::Package::ierror ("Branch $branch does not exist in the remote repository. \
    You probably checked out a specific commit tag\
    Consider checking out master branch and then pull\
    awb-shell checkout package <name>/master\
    awb-shell pull package name\
    If you have local changes then consider merging with master\n")  ;
    return undef;
  } 

  my $command = "pull $url $branch";
  my $status = $self->git($command);

  if ($status) {
      return undef;
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

  #
  # If user specified a commitlog make sure it exists
  #
  if (defined($commitlog_file)) {
    if ( ! -r $commitlog_file ) {
      commit_failure("Sorry: user specified commitlog ($commitlog_file) is not readable or does not exist");
      return 0;
    }
  }


 $self->commit_check($commitlog_file)
   || commit_failure("Cannot commit from this state") && return 0;

  print "Commit: Status " . $self->{status} . "\n";

  if ($self->{status} eq "No-commit-needed") {
    return 0;
  }

  $self->isprivate()
    || commit_failure("You are using a shared package that is not up-to-date.\n" .
                      "Check out the package or get the shared package updated.\n") &&  return 0;

   # Actual commit stage
   print "Commit: I am about to commit your changes. \n\n";

  if (! Asim::choose_yes_or_no("Do you really want to proceed and commit","response_required","yes")) {
    $self->{status} = "";
    return 0;
  }

  # Do the actual commit
  my $command = "commit -a ";
  if ( defined($commitlog_file) ) {
    $command = $command . " -F $commitlog_file";
  }
  if (! $self->git_command($command)) {
    ierror("Commit: Fatal error during git commit: \n".
            "Commit: Your commit might be partially done \n",
            "Commit: and the state of your directory is probably messed up.\n");
    return 0;
  }

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

=item $git-E<gt>push_check()

A set of checks to make sure that we have a 
reasonable chance of succeeding at a push

=cut

sub push_check {
  my $self = shift;

  $self->{status} = "Push-needed";
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
    # Hack in case the remotes/origin/master ssh command asks for a password
    #
    if ( /password:/) {
      print "$_\n";
    }
    
    if ( /^#\s+(.+):\s+(.+)$/ ) {
      $status = $1;
      $file = $2;
    }
    
    if ($status and $file) {
       my ($filnam, $dir) = fileparse($file);
       chop $dir if ($dir =~ m/\/$/); # remove trailing "/" from directory
       push(@files, [$dir, $filnam, $status, '', '']); # no rev# or date information from Git
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
  $command = "(cd $location; ".$self->repo_cmd()." $command)";
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

  if ($url =~ m/devtools.intel.com/) {
    return 1;
  }

  return 0;
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

=item $git->get_working_hash()

Return the git commit hash that the working copy was checked out with.

This returns a 40 character hex hash to identify the commit

=cut

sub get_working_hash {
  my $self = shift;
  my $location = $self->location();
  my $md5hash = `cd $location; git rev-parse HEAD`;
  chomp $md5hash;
  $md5hash;
}


=item $package-E<gt>baseline_tag([<use_csn>])

Return a tag that can be used later
to retrieve exactly the version of the package that is currently
checked out in the working copy (minus any uncommitted changes).

In git repositories (unlike in CVS/SVN) we do not need branch name
and tag to get to a particular state in the rpeository. Instead we can 
use the hash tag that gets generated for every commit . The hash is
unique for each commit across all branches as long as the contents of
the two states are different.

=cut

sub baseline_tag
{
  my $self = shift;
  my $use_csn = shift;
  my $md5hash  = $self->get_working_hash();
  return $md5hash;
}


###############################################################
#
# Function: push
#
#    This is the umbrella function for performing a hierarchical
#    push of a package and the packages it depends on.
#
#    1) Sanity checking
#       a) make sure global environemnt is ready
#       a) get packages dependent on this one
#       b) make sure they are visible in path
#
#    2) Run regression
#
#    3) Lock Repositories
#       a) lock repositories
#       b) abort if packages are not up to date
#
#    4) Prepare each package to push
#       a) package up regression results
#       b) obtain new CSN and update
#
#    5) Commit each package
#
#    6) Unlock Repositories
#
################################################################

sub push_package {
  my $self = shift;
  my $only_self = shift || 0;

  my $stop_push = 0;
  my $success = 0;

  if (! defined ($self->{url})) {
    ierror("Push: No url defined. Dont know where to push!") && return 0;
  }
  my @all;
  print "Push: Starting a push of package: " . $self->name() . "\n";
  print "Push: Target URL " . $self->{url} . "\n";

#########      ###########      ###########      ###########      ###########

  $self->step("Sanity Check", 0);
  $self->banner();

  $self->sanity_stage()
    || return ();

  # Find dependent packages that are checked out....

  if ($only_self) {
    @all = ($self);
  } else {
    @all = ();

    foreach my $dp ($self->get_dependent_packages()) {
      if ( ! $dp->isprivate()) {
        print "Push: Skipping private dependent package: " . $dp->name() . "\n";
        next;
      }

      push(@all, $dp);
    }
  }

#########      ###########      ###########      ###########      ###########

  $self->step("Lock the relevant repositories");
  
  $self->SUPER::lockall(@all) || return 0;

#########      ###########      ###########      ###########      ###########

  $self->step("Check if packages are ready to be pushed");

  foreach my $p (@all) {
    $p->banner();

    if (! $p->check_stage()) {
      $stop_push = 1;
    }
  }

  ## Instead of exiting when the first package is not up-to-date,
  ## check the status of all packages and then exit.
  if ($stop_push == 1) {
    $self->SUPER::unlockall(@all);
    return ();
  }

#########      ###########      ###########      ###########      ###########

  $self->step("Do necessary updates of package in preparation for a push");

  Asim::Xaction::start();

  foreach my $p (@all) {
    $p->banner();

    if (! $p->prepare_stage()) {
      unlockall(@all);
      Asim::Xaction::abort();
      return ();
    }
  }

#########      ###########      ###########      ###########      ###########

  $self->step("Do actual push");

  print "Push: I am about to push your changes. This is your last chance...\n\n";

  if (! Asim::choose_yes_or_no("Do you really want to proceed and push","response_required","yes")) {
    $self->{status} = "";
    $self->SUPER::unlockall(@all);
    Asim::Xaction::abort();
    return 0;
  }

  Asim::disable_signal();

  foreach my $p (@all) {
    $p->banner();
    $success = $p->push_stage();
    if (! ($success)) {
        $self->SUPER::unlockall(@all);
        Asim::Xaction::abort();
        Asim::enable_signal();
        return 0;
    }
  }

  Asim::Xaction::commit();

#########      ###########      ###########      ###########      ###########

  $self->step("Unlock the repositories");

  $self->SUPER::unlockall(@all);

  Asim::enable_signal();

  return 1;
}

################################################################
#
# Function: sanity_stage
#
# A set of onetime checks to make sure that we have a 
# reasonable chance of succeeding at a push
#
################################################################

sub sanity_stage {
  my $self = shift;
  
  $| = 1;

  #
  # Some utility programs
  #
  # TODO: We shouldn't just rely on these being in the path
  #
  $HOSTNAME = "hostname";
  $DATE = "date";
  $DATE_UTC = "date -u";

  #
  # Some global variables
  #
  $HOST = `$HOSTNAME`;
  chop $HOST;

  $USER = $ENV{'USER'};

  #
  # Indicate we haven't seen big problems yet
  #
  $PROBLEM = 0;

  print "Push: You seem to have a reasonable environment to push your changes\n";

  return 1;
}

################################################################
#
# Function:
#
#
#
################################################################

sub check_stage {
  my $self = shift;

  #
  # Check on the git status of the package
  #
  our $checkindex++;

  $self->push_check() 
      || push_failure("Cannot push from this state") && return 0;
  
  print "Push: Status " . $self->{status} . "\n";

  if ($self->{status} eq "No-push-needed") {
    return 1;
  }

  $self->isprivate()
    || push_failure("You are using a shared package that is not up-to-date.\n" .
                      "Check out the package or get the shared package updated.\n") && return 0;

  return 1;
}


################################################################
#
# Function: 
#
# 
#
################################################################

sub prepare_stage {
  my $self = shift;

  if ($self->{status} eq "No-push-needed") {
    print "Push: No push needed for this package, so nothing to do here...\n";
    system("rm $self->{reportfile}");
    return 1;
  }

  $self->update_gold_results()
    || push_failure("Unable to update gold stats") && return 0;

  $self->do_we_want_regression_results()
    || push_failure("Unable to get regression results") && return 0;

  $self->get_regression_results()
    || push_failure("Unable to get regression results") && return 0;

  $self->update_ipchist()
    || push_failure("Unable to update ipchist file") && return 0;

  return 1;
}


################################################################
#
# Function: 
#
# 
#
################################################################

sub push_stage {
  my $self = shift;

  if ($self->{status} eq "No-push-needed") { 
    print "Push: No push needed for this package, so nothing to do here...\n";
    $self->{status} = "";
    return 1;
  }

  $self->push_archive()
    || ierror("Trouble with git push\n") && return 0;

  $self->push_regtest()
    || ierror("Trouble with push of regtest\n") && return 0;

  $self->{status} = "";
  return 1;
}

################################################################
#
# Function: 
#
# 
#
################################################################


sub push_archive {
  my $self = shift;
  my $command;
  my $success;

  my $push_url = $self->{url};

  $self->save();

  #
  # Execute 'git push' to see all the changes that have been made.
  #
  print "Push: Really pushing....\n";

  my $branch = $self->{branch};
  if (!defined($branch)) {
      $branch = $self->get_current_branch();
      $self->{branch} = $branch;
  }
  # push changes from local repository into a remote repository
  $command = "push $push_url $branch";
  if (! $self->git_command($command)) {
    ierror("Push: Fatal error during git push. Exiting! \n");
    return 0;
  }
  
  return 1;
}

sub push_regtest {
  my $self = shift;
  my $regression = $self->{regression}
    || return 1;

  my $tarfile = $self->{tarfile};

  #
  # Tar and save regression results
  #
  $regression->tar_and_save($tarfile);

  return 1;
}

################################################################
#
# Functions to call in the event of problems
#
################################################################

sub push_failure {
  my $message = shift;

  print "\n";
  print "Push: I am sorry but there were problems with your push attempt\n";
  print "Push: so if you can fix the problem you can try to push again\n";
  print "\n";
  print "Push: The error I received was: ------ $message -----\n";
  print "\n";

  return 1;
}



=back



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
