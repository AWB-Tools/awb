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


package Asim::Package;
use warnings;
use strict qw(refs vars);

use Asim::Lock;
use Asim::Package::Util;

use File::Basename;


# Package wide variables (defined in Package.pm)

our @ISA;

our $CHANGES;

our $PACKAGES;
our $MYTAGS;
our $MYMERGEPOINTS;
our $IPCHIST;

our $CONFIG_BM;
our $CONFIG_PM;

our $PM;

our $GOLDSTATS;

# Commit wide variables

our $HOSTNAME;
our $DATE;
our $DATE_UTC;

our $EDITOR;
our $EDITOR_OPTIONS;

our $TMPDIR;
our $HOST;
our $USER;

our $PROBLEM;

#
# Member variables used by only by commit
#
# $self->{status}               - summary of cvs status report
# $self->{reportfile}           - cvs status reportfile
# $self->{commentfile}          - svn commit log

# $self->{must_have_regression} - what it says
# $self->{regtestdir}           - directory containing regression test
# $self->{regression}           - regression test object
# $self->{tarfile}              - name for regression test tarfile 
#


# Documentation of the public functions is in Package.pm


################################################################
#
# Function: commit
#
#    This is the umbrella function for performing a hierarchical
#    commit of a package and the packages it depends on.
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
#    4) Prepare each package to commit
#       a) package up regression results
#       b) obtain new CSN and update
#       c) update changes file
#       d) 
#
#    5) Commit each package
#
#    6) Unlock Repositories
#
################################################################

sub commit {
  my $self = shift;
  my $only_self = shift || 0;

  my $stop_commit = 0;

  my @all;
  print "Commit: Starting a commit of package: " . $self->name() . "\n";

#########      ###########      ###########      ###########      ###########

  $self->step("Sanity Check", 0);
  $self->banner();

  $self->sanity_stage()
    || return ();

  # Find dependent packages that are checked out....

  if ($only_self) {
    @all = ($self);
  } else {
    @all = ($self->get_dependent_packages());
  }

#########      ###########      ###########      ###########      ###########

  $self->step("Lock the relevant repositories");

  lockall(@all) || return 0;


#########      ###########      ###########      ###########      ###########

  $self->step("Check if packages are ready to commit");

  foreach my $p (@all) {
    $p->banner();

    if (! $p->check_stage()) {
      $stop_commit = 1;
    }
  }

  ## Instead of exiting when the first package is not up-to-date,
  ## check the status of all packages and then exit.
  if ($stop_commit == 1) {
    unlockall(@all);
    return ();
  }

#########      ###########      ###########      ###########      ###########

  $self->step("Do necessary updates of package in preparation for commit");

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

  $self->step("Do actual commit");

  print "Commit: I am about to commit your changes. This is your last chance...\n\n";

  if (! Asim::choose_yes_or_no("Do you really want to proceed and commit","response_required","yes")) {
    $self->{status} = "";
    unlockall(@all);
    Asim::Xaction::abort();
    return 0;
  }

  Asim::Xaction::commit();

  Asim::disable_signal();

  foreach my $p (@all) {
    $p->banner();
    $p->commit_stage();
  }

#########      ###########      ###########      ###########      ###########

  $self->step("Unlock the repositories");

  unlockall(@all);

  Asim::enable_signal();

  return 1;
}

 
 
################################################################
#
# The next four functions are the major stages in 
# the commit process
#
################################################################

################################################################
#
# Function: sanity_stage
#
# A set of onetime checks to make sure that we have a 
# reasonable chance of succeeding at a commit
#
################################################################

sub sanity_stage {
  my $self = shift;


  $| = 1;

  #
  # Make sure we have an editor
  #
  $EDITOR = $ENV{EDITOR}
    || commit_failure("Environment variable EDITOR must be set") && return 0;

#  if (! -x $EDITOR) {
#    commit_failure("Environment variable EDITOR must be executable");
#    return 0;
#  }


  #
  # Check for CVS_RSH
  #
  if (! $ENV{CVS_RSH} ) {
    iwarn("Potential problem: No environment variable CVS_RSH\n" .
          "I've set it to \"ssh2\", which should be what you want for ASIM");

    $ENV{CVS_RSH} = "ssh2";
  }

  #
  # Check for a /tmp directory
  #
  $TMPDIR = $ENV{'TMPDIR'};
  if ( ! defined($TMPDIR) || ! -d $TMPDIR ) {
    $TMPDIR = "/tmp"
  }
  if ( ! -d $TMPDIR ) {
    commit_failure("Sorry: \$TMPDIR is not set and /tmp does not exist");
    return 0;
  }

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

  print "Commit: You seem to have a reasonable environment\n";

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

  my $tmpfile;
  my $tmpfile_comment;

  #
  # Check on the CVS status of the package
  #
  #     This is a bit messy because it creates a file 
  #     with a report on the modified files that is
  #     appended to the changes file later
  #
  our $checkindex++;

  $tmpfile = "$TMPDIR/asimcommit.$$.$checkindex.txt";
  $self->{reportfile} = $tmpfile;

  $tmpfile_comment = "$TMPDIR/asimcommit_comment.$$.$checkindex.txt";
  $self->{commentfile} = $tmpfile_comment;

  $self->commit_check($tmpfile)
      || commit_failure("Cannot commit from this state") && return 0;

  print "Commit: Status " . $self->{status} . "\n";

  if ($self->{status} eq "No-commit-needed") {
    return 1;
  }

  $self->isprivate()
    || commit_failure("You are using a shared package that is not up-to-date.\n" .
                      "Check out the package or get the shared package updated.\n") && return 0;

  $self->archive_writable()
    || commit_failure("You do not appear to be able to write the archive\n") && return 0;

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

  if ($self->{status} eq "No-commit-needed") {
    print "Commit: No commit needed for this package, so nothing to do here...\n";
    system("rm $self->{reportfile}");
    return 1;
  }

  $self->update_gold_results()
    || commit_failure("Unable to update gold stats") && return 0;

  $self->do_we_want_regression_results()
    || commit_failure("Unable to get regression results") && return 0;

  $self->get_regression_results()
    || commit_failure("Unable to get regression results") && return 0;

  $self->update_csns()
    || commit_failure("Unable to update serial numbers") && return 0;

  $self->edit_changes_for_commit()
    || commit_failure("Unable to edit changes") && return 0;

  $self->update_ipchist()
    || commit_failure("Unable to update ipchist file") && return 0;

  return 1;
}


################################################################
#
# Function: 
#
# 
#
################################################################

sub commit_stage {
  my $self = shift;

  if ($self->{status} eq "No-commit-needed") { 
    print "Commit: No commit needed for this package, so nothing to do here...\n";
    $self->{status} = "";
    return 1;
  }

  $self->commit_archive()
    || ierror("Trouble with cvs commit\n") && return 0;

  $self->commit_regtest()
    || ierror("Trouble with commit of regtest\n") && return 0;

  $self->{status} = "";
  return 1;
}



################################################################
#
# The following functions are the in-order sub-stages for
# the commit process. 
#
# IMPORTANT NOTICE: I tried to write them so that the
# commit process for a package just consists of calling 
# each of the following functions in the order they are 
# written below!!!!
#
# Unfortunately, one can't just call them in order when
# there are multiple dependent packages being committed and
# the calls for different packages get interleaved by
# major stage...
#
# In addition, to be safe one should perform some of
# the operations under an abortable transaction so you
# can back out and others under the appropriate locks.
# See the above routine for an example of this...
#
################################################################

#
# cvs_check(), svn_check(), bk_check() moved to
# Cvs.pm, Svn.pm and BitKeeper.pm, respectively,
# and renamed commit_check().
#

################################################################
#
# Function: partial_match
#
# Little utility function - ignore it...
#
################################################################


sub partial_match
{
 my $dir = $_[0];
 my $aref =  $_[1];

 foreach my $dangerdir ( @$aref ) {
  if ( $dir =~ $dangerdir ) {
   return 1;
  }
 }
 return 0;
}



################################################################
#
# Function: 
#
# 
#
################################################################

sub archive_writable {

  # TODO: check if archive is writable.

  print "Commit: We should be checking if you can write the archive,\n";
  print "Commit: but I don't know how to do that just now...\n";
 
  return 1;
}


################################################################
#
# Function: 
#
# 
#
################################################################


sub update_gold_results {
  my $self = shift;

  my $location = $self->location();

  my $REGRESSIONDIR;
  my $regression;

  #
  # Test if this package does ANY regression
  # If not - just skip this step
  #

  if (!$self->regtest()) {
    return 1;
  }

  #
  # Ask the user whether he has copied his new stats into the gold directory. 
  # If not, he might want us to do it for him. 
  #
  print "\n";
  print "Commit: Before we proceed, do you have new STATS that you should copy into the gold/ directory \n";
  print "Commit: If you forgot to do so, just answer 'y' and I will do it for you.\n";
  print "Commit: If you have already done it, you can simply reply 'n'.\n\n";

  if (Asim::choose_yes_or_no("Do you want to copy stats")) {
    #
    # Get a path to the REGRESSION DIR
    #
    print "\n";
    print "Commit: Okay... but note this is not in lieu of doing a CVS add for new benchmarks\n";
    print "Commit: In any event I need a path to your regression test directory...\n";
    print "Commit: I need the REGRESSION test directory, NOT the path of the STATS directory.\n";
    print "\n";

    $REGRESSIONDIR = $Asim::default_workspace->ask_for_regtestdir();
    $self->{regtestdir} = $REGRESSIONDIR;

    $regression = Asim::Regression->new($REGRESSIONDIR)
      || ierror("Commit: '$REGRESSIONDIR does not contain regression results\n") && return 0;

    #
    # Copy the STATS
    #
    my @statfiles;
    my $ok;

    #
    # Get the names of the stat files
    #
    @statfiles = ($regression->statfiles());

    # TODO: Make sure there really are some files...

    foreach my $f (@statfiles) {
      my $b = basename($f);
 
      Asim::Xaction::add_file("$location/$GOLDSTATS/$b");

      $ok = system("cp -f $f $location/$GOLDSTATS");
      if ( $ok != 0 ) {
        ierror("Commit: Unexpected error when copying the STATS directory into the GOLD directory: $!\n");
        return 0;
      }
    }
  }

  return 1;
}



################################################################
#
# Function: do_we_want_regresssion_results
#
#    This is a lot of gyrations just to find out if we want
#    and/or need to update the regression results, and to
#    get the regtest object if we haven't already.
#
################################################################


sub do_we_want_regression_results {
  my $self = shift;
  my $need_regression = $self->{must_have_regression};
  my $already_have_regression = defined($self->{regtestdir});

  my $REGRESSIONDIR;

  #
  # Test if this package does ANY regression
  # If not - just skip this step
  #

  if (!$self->regtest()) {
    return 1;
  }

  if ( ! $already_have_regression && ! $need_regression ) {
    print "\n";
    print "Commit: I usually need a path to the regression test that you used to validate a commit\n";
    print "Commit: But your changes don't seem to affect the performance\n";
    print "Commit: model, so you can type <CR> and still successfully commit if you\n";
    print "Commit: actually did not run the regression test (but don't get used to it :-))\n\n";

    $REGRESSIONDIR = Asim::choose_filename("Enter name of regression test directory");
  }

  elsif ( $already_have_regression && ! $need_regression ) {
    print "\n";
    print "Commit: Your changes don't seem to affect the performance model.\n";
    print "Commit: Thus, you need not provide a regression test. However, you already\n";
    print "Commit: gave me a path to your REGRESSION directory previously. Therefore,\n";
    print "Commit: I will use that anyway.\n";
  }

  elsif ( ! $already_have_regression  && $need_regression ) {
    print "\n";
    print "Commit: I NEED a path to the regression test that you used to validate this commit.\n\n";

    $REGRESSIONDIR = "++++";
    while (! -d $REGRESSIONDIR) {
      $REGRESSIONDIR = $Asim::default_workspace->ask_for_regtestdir();
    }
  } 

  elsif ( $already_have_regression && $need_regression ) {
    print "\n";
    print "Commit: This commit MUST be validated with a regression test. Since you already gave me\n";
    print "Commit: a path to the regression test I will be using that path.\n";
  }

  else {
    die("Commit: Internal error - Please report to Dr. Boole\n");
  }

  if (defined($REGRESSIONDIR)) {
    $self->{regtestdir} = $REGRESSIONDIR;
  }

  return 1;
}



################################################################
#
# Function: 
#
# 
#
################################################################

sub get_regression_results {
  my $self = shift;

  my $regtestdir = $self->{regtestdir}
    || return 1;

  my $regression;

  print "Commit: Checking the status of your regression tests...\n";


  #
  # Okay we're going to do something with regression results
  #

  #
  # IMPORTANT.... Get the old stat results
  #
  $self->read_stats();

  #
  # IMPORTANT.... Get the new regression results
  #
  $regression = Asim::Regression->new($regtestdir);
  $self->{regression} = $regression;
  #
  # Review regression results
  #
  my $pending   = $regression->pending();
  my $error     = $regression->error();
  my $done      = $regression->done();
  my $submitted = $regression->submitted();
  my $total;

  #
  # Make sure user is aware of what he is doing by asking for files that have
  # not completed their execution fully successfully
  #
  print "\n";
  print "SUMBITTED $submitted\n";
  print "DONE $done\n";
  print "PENDING $pending\n";
  print "ERROR $error\n";

  if ( $pending > 0 ) {
    print "\n";
    print "Commit: You have $pending PENDING benchmarks!!!!!!!.\n";
    Asim::choose_yes_or_no("Do you really want me to proceed anyway?","no")
      || return 0;
  }

  if ( $error > 0 ) {
    print "\n";
    print "Commit: You have $error benchmarks in the ERROR directory!!!!!!!.\n";
    Asim::choose_yes_or_no("Do you really want me to proceed anyway?","no")
      || return 0;
  }

  $total = $pending + $error + $done;
  #### if ( $total != $submitted ) {
  ####  print "\n";
  ####  print "Commit: Something strange happened: PENDING+ERROR+DONE ($total) != SUBMITTED ($submitted)!!!!!!!.\n";
  ####  Asim::choose_yes_or_no("Do you really want me to proceed anyway?","no")
  ####    || return 0;
  #### }

  #
  # Print report
  #
  $regression->report(*STDOUT, $self->stats(), $self->stats_metric(), 1);

  Asim::choose_yes_or_no("\n\nIs this variation acceptable to be commited in","no","yes")
    || return 0;

}

################################################################
#
# Function: 
#
# 
#
################################################################

sub update_csns {
  my $self = shift;

  my $location = $self->location();
  my $csn;
  my $mytags;

  print "Commit: Updating CSN's and tags...\n";

  #
  # Bug: Although the csn in the file is restored on 
  #      an abort, the csn in the PERL Asim::Package
  #       object itself is not restored!!!!
  #
  Asim::Xaction::add_file($self->filename);

  $csn = $self->increment_csn();

  # mytags is used by asimmerge and asimbranch
  # mytags is simply a history of prior tags

  $mytags = "$location/$MYTAGS";

  Asim::Xaction::add_file($mytags);

  safe_append("MYTAGS", $mytags)
    || ierror("Commit: $MYTAGS file does not exist\n") && return 0;

  print MYTAGS "$csn\n";

  safe_close("MYTAGS");

  return 1;
}


################################################################
#
# Function: 
#
# 
#
################################################################

sub edit_changes_for_commit {
  my $self = shift;

  my $base        = $self->location();
  my $changes     = "$base/$CHANGES";

  my $csn         = $self->csn();
  my $tarfile     = "";

  my $reportfile  = $self->{reportfile} || "/dev/null";
  my $regression  = $self->{regression};

  my $date;
  my $date_utc;

  print "Commit: Generating a new 'changes' file...\n";

  Asim::Xaction::add_file($changes);

  safe_append(*CHANGES, $changes)
    || ierror("Cannot open $changes\n") && return ();

  #
  # Update the changes file
  #
  $date = `$DATE`;
  chop $date;
  $date_utc = `$DATE_UTC`;
  chop $date_utc;

  if (defined($regression)) {
    $tarfile = "reglog.$csn.tar.gz";
    # Remember tarfilename...
    $self->{tarfile} = $tarfile;
  }

  print CHANGES "\n----------\n";
  if (($self->type() eq "cvs") || ($self->type() eq "bitkeeper")) {
  #  print CHANGES "$USER\tDate: $date\tCSN: $csn\tTAR: $tarfile\n\n";
  printf CHANGES "%-10s  Date: %s  CSN: %s\n%-10s        %s\n\n",
    $USER, $date, $csn, "", $date_utc;
  }
  safe_close(*CHANGES);

  if (($self->type() eq "svn")) {
  my $tmp  = "/tmp/asim-shell-rpt-file.$$";
  CORE::open(RPT, "<$reportfile");
   
  CORE::open(RPT_TMP, ">$tmp");
  printf RPT_TMP "%-10s  Date: %s  CSN: %s\n%-10s        %s\n\n",
    $USER, $date, $csn, "", $date_utc;
  while (<RPT>) {
    printf RPT_TMP $_;
  }
  printf RPT_TMP "\nDO NOT DELETE THIS LINE! ADD COMMENTS BELOW:\n";
  CORE::close(RPT);
  CORE::close(RPT_TMP);
  system ("mv -f $tmp $reportfile");
 }

  #
  # Concatentate report of modified files
  #
  if (($self->type() eq "cvs") || ($self->type() eq "bitkeeper")) {
    system("cat $reportfile >>$changes");
    system("rm $reportfile");
  }

  #
  # Concatenate regtest report...
  #
  if (defined($regression)) {
    safe_append(*CHANGES, $changes)
      || ierror("Cannot open $changes\n") && return ();

    $regression->report(*CHANGES, $self->stats(), $self->stats_metric());
    
    safe_close(*CHANGES);
  }

  # let the user add arbitrary comments to the changes file
  return $self->edit_changes_manual();
}


################################################################
#
# Function: 
#
# 
#
################################################################



sub edit_changes_manual {
  my $self = shift;

  my $base        = $self->location();
  my $changes     = "$base/$CHANGES";
  my $reportfile  = $self->{reportfile};
  my $tmp  = "/tmp/asim-shell-rpt-file.$$";
##  my $commentfile  = "/tmp/asim-shell-comment-file.$$";
  my $commentfile = $self->{commentfile};
  my $nextline;

  # Add the comment file to the list of files to 
  # be deleted on an abort
  Asim::Xaction::add_file("$commentfile");

  #
  # Make sure we have a changes file
  #
  if (! -e $changes) {
    print "\n\n";
    print "Oh no, the changes file ($changes) does not exist\n\n";
    return ();
  }

  #
  # Launch a window so that the user types in an explanation for his changes
  #
  print "\n\n";
  print "Launching your favorite editor on the 'changes' file ($changes)\n";
  print "Please type your comments at the *end* of the 'changes' file...\n\n";
  Asim::choose("Enter <CR> to edit changes file");

  if ($EDITOR =~ "gvim") {   
    $EDITOR_OPTIONS = "-f +"; # go to end of file; don't fork while starting GUI
  } elsif ($EDITOR =~ "vim?") {
    $EDITOR_OPTIONS = "+"; # go to end of file
  } elsif ($EDITOR =~ "x?emacs") {
    $EDITOR_OPTIONS = "+1000000"; # go to end of file (well, far back at least)
  } else {
    $EDITOR_OPTIONS = "";
  }

  if (($self->type() eq "cvs") || ($self->type() eq "bitkeeper")) {
    system("$EDITOR $EDITOR_OPTIONS $changes");
  }
  elsif (($self->type() eq "svn")) {

    system("$EDITOR $EDITOR_OPTIONS $reportfile");
    
    CORE::open(RPT, "<$reportfile");
    CORE::open(CMT, ">$commentfile");

    # Extract the comments to stick it into svn log
    printf CMT "CSN: %s\n%-10s\n", $self->csn(), "";

    while (<RPT>) {
      if(/DO NOT DELETE THIS LINE! ADD COMMENTS BELOW:/) {
        $nextline = <RPT>;
	while ($nextline) {
	  printf CMT "%s", $nextline;
	  $nextline = <RPT>;
	}
      }
    }
    CORE::close(CMT);
    CORE::close(RPT);

    # Remove the extra line "Do not delete ..." before appending to changelog
    # Also parse and remove 'Unknown' files
    CORE::open(RPT, "<$reportfile");
    CORE::open(RPT_TMP, ">$tmp");
    while (<RPT>) {
      if(!(/DO NOT DELETE THIS LINE! ADD COMMENTS BELOW:/ || /Unknown(\s+)(\d+)/)) {
	printf RPT_TMP "%s", $_;
      }
    }
    CORE::close(RPT);
    CORE::close(RPT_TMP);

    system ("mv -f $tmp $reportfile");
  }
  print "\n\n";

  if (($self->type() eq "svn")) {
    system("cat $reportfile >> $changes");
    system("rm $reportfile");
  }

  return 1;
}


################################################################
#
# Function: 
#
# 
#
################################################################


sub update_ipchist {
  my $self = shift;

  my $regression = $self->{regression}
    || return 1;

  my $location = $self->location();
  my $ipchist = "$location/$IPCHIST";

  Asim::Xaction::add_file($ipchist);

  #
  # Arggh... update stats...
  #
  $self->{stats} = $regression->stats();

  #
  # Write out stats
  #
  $self->write_stats();

}

################################################################
#
# Function: 
#
# 
#
################################################################


sub commit_archive {
  my $self = shift;
  my $command;
  my $success;

  my $csn = $self->csn();

##  my $commentfile = "/tmp/asim-shell-comment-file.$$";
  my $commentfile = $self->{commentfile};
  
  $self->save();

  #
  # Protect the script against use of '^C'
  #
  #$SIG{INT} = \&no_ctrl_c;

  #
  # Execute 'cvs commit' to see all the changes that have been made.
  #
  print "Commit: Really committing....\n";

  if ($self->type() eq "cvs")
  {
    # lets try to commit:
    $command = "commit -m\"${csn}\"";
    while (! $self->cvs_command($command)) {
      ierror("Commit: Fatal error during cvs commit: \n".
             "Commit: Your commit might be partially done \n",
             "Commit: and the state of your directory is probably messed up.\n");
    $success = $self->commit_dilemma("cvs $command");
    return 0 if (! defined($success));
    last if ($success);
    }
  }
  elsif ($self->type() eq "svn")
  {
    # lets try to commit:
    $command = "commit --file $commentfile";
    while (! $self->svn_command($command)) {
      ierror("Commit: Fatal error during svn commit: \n".
             "Commit: Your commit might be partially done \n",
             "Commit: and the state of your directory is probably messed up.\n");
    $success = $self->commit_dilemma("svn $command");
    return 0 if (! defined($success));
    last if ($success);
    }
  }
  else {
    ierror("No commit for non-cvs/non-svn packages currently implemented");
    return 0;
  }
  if ($self->type() eq "svn") {
     system ("rm $commentfile");
  }

  #
  # Tag this commit with its serial number
  #
   print "Commit: Tagging your changes...\n";

  if ($self->type() eq "cvs") {
    $command = "tag ${csn}";
    while (! $self->cvs_command($command)) {
      ierror( "Fatal error during cvs tag: \n" .
            "Your tag might be partially done ... try to complete manually\n");
      $success = $self->commit_dilemma("cvs $command");
      return 0 if (! defined($success));
      last if ($success);
    }
  }
  return 1;
}



sub commit_regtest {
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

sub commit_failure {
  my $message = shift;

  print "\n";
  print "Commit: I am sorry but there were problems with your commit attempt\n";
  print "Commit: I should be able to back out all the changes I made to your files,\n";
  print "Commit: so if you can fix the problem you can try to commit again\n";
  print "\n";
  print "Commit: The error I received was: ------ $message -----\n";
  print "\n";

  return 1;
}


sub commit_dilemma {
  my $self = shift;
  my $command = shift;

  my $location = $self->location();

  my $prompt;
  my @options;
  my $choice;

  print "\n";
  print "Commit: Dear gentle user:\n";
  print "\n";

  if (! $PROBLEM ) {
    $PROBLEM = 1;

    print "Commit: I am sorry to report that we have a dilemma here. I have\n";
    print "Commit: tried to cvs commit your changes but failed. There is\n";
    print "Commit: nothing more that I can do to get the files into the repository\n";
    print "Commit: Unfortunately, a number of files in your package have\n";
    print "Commit: been updated to look like they should after being checked in.\n";
    print "Commit: So you should really try to check the changes cvs commited.\n";
    print "Commit: If you abort the commit - the CVS and file state will be INDETERMINATE\n";
    print "\n";

  } else {
    print "Commit: More problems...Reread my previous message\n";
    print "\n";
  }    


  print "Commit: The command I was running was:\n";
  print "Commit:         cd $location\n";
  print "Commit:         $command\n";
  print "\n";
    
  print "Commit: Now you must choose one of the following options.\n";
  print "\n";

  $prompt = "Choose an action";

  @options = (
	      "Please try the operation again",                          # 0
	      "I have sucessfully manually performed the operation",     # 1
	      "Abort out of the operation",                              # 2
	      );

  my $sure = 0;
  while (! $sure) {
    $choice = Asim::choose_from_list($prompt, @options);

    $sure = defined($options[$choice]);

    if ($sure && $choice == 0) {
      $sure = 1;
    }

    if ($sure && $choice == 2) {
      print "\n";
      print "Commit: WARNING: You are going to be left in an indeterminate state\n";
      print "Commit:          and the lock should remain set\n";
      print "\n";
      $sure = Asim::choose_yes_or_no("Are you sure you want to do this");
    }

    if (! $sure) {
      print "\n";
      print "Commit: You must make some choice..\n";
      print "\n";
    }
  }

  if ($choice == 0) {
    return 0;
  }

  if ($choice == 1) {
    return 1;
  }

  if ($choice == 2) {
    $self->{keep_lock} = 1;
    return undef;
  }


}


################################################################
#
# lockall/unlockall
#
# A couple of functions to lock and unlock a group of packages
#
################################################################

sub lockall {
  my @all = @_;

  my $name;
  my @done = ();

  foreach my $p (@all) {
    $name = $p->name();
    $p->banner();

    if (! $p->acquire_lock()) {
      ierror("Commit: Could not obtain lock for $name\n");
      unlockall(@done);
      return 0;
    }

    push(@done,$p);

    print "Locking $name: " . $p->status_lock() . "\n";
  }

  return 1;
}


sub unlockall {
  my @all = @_;

  my $name;

  foreach my $p (@all) {
    $name = $p->name();
    $p->banner();

    if (defined($p->{keep_lock}))  {
      $p->{keep_lock} = undef;
      print "Commit: Lock $name NOT being released - you will have to do it manually\n";
      next;
    }

    $p->release_lock()
      || ierror("Comit: Could not release lock for $name - you will have to do it manually\n");

    print "Unlocking $name: " . $p->status_lock() . "\n";
  }

  return 1;
}  


1;
