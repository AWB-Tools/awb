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
use strict;

use File::Basename;

use Asim::Lock;
use Asim::Repository;
use Asim::Repository::DB;


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

# Branch wide variables

our $HOSTNAME;
our $DATE;
our $DATE_UTC;

our $TMPDIR;
our $HOST;
our $USER;

our $PROBLEM;


# Documentation of the public functions is in Package.pm


sub release {
  my $self = shift;
  my $version = shift ||
    ierror("Release: Version must be specified\n") && return 0;
  my $version_major;
  my $version_minor;
  my $patchlevel;

  my $name = $self->name();
  my $location = $self->location();
  my $oldcsn = $self->csn();
  my $newcsn;

  my @files;
  
  if ( $self->type() ne 'cvs' ) {
    ierror("Release not implemented for packages of type " . $self->type() . "\n");
    return 0;
  }

  if ($version =~ m/^(\d+)\.(\d+)$/) {
    $version_major = $1;
    $version_minor = $2;
    $patchlevel = 0;
  } else {
    ierror("Release: I don't understand version specification '$version'.\n".
           "         Version needs to be of form <major>.<minor>\n");
    return 0;
  }

  # branch has a special name: # <package>-V<version_major>_<version_minor>
  # Note: dot (".") is not a valid character in CVS tag names!
  my $branchname = "${name}-V${version_major}_${version_minor}";

  # Release instance name is: <package>-V<version>_<patchlevel>
  my $patchname = "${branchname}_${patchlevel}";

  print "Release package '$name' as version '$version'\n";

  #
  # Note: The release process is very similar to the branch process, in
  # fact we do create a specially named branch with a for a release, but
  # we do not do all the other associated operations, in partucular we
  # do not create a new lock for the released version.
  #
  # TODO: much of the code could probably be shared between branch and
  # release if factored appropriately.
  #

  #########      ###########      ###########      ###########      ###########

  $self->step("Sanity checks", 0);

  # TBD: We use the default reposistory DB... should that be selectable...

  my $repositoryDB = Asim::Repository::DB->new();

  if (!defined($repositoryDB)) {
    ierror("Release: Failed to open repositoryDB file\n");
    return 0;
  }

  if (-e "$location/$MYMERGEPOINTS") {
    ierror("Release: Cannot release a branch\n");
    return 0;
  }

  # Check that there doesn't already exist a branch with this name.
  my @branchtags = $self->find_branchtags($MYTAGS);
  my $tag;
  foreach $tag (@branchtags) {
    if ($tag eq $branchname) {
      ierror("Release: Cannot create release $version.\n".
             "         CVS branch named $branchname already exists.\n");
      return 0;
    }
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Lock repository");

  $self->acquire_lock()
    || ierror("Release: Could not acquire lock\n") && return ();

  #########      ###########      ###########      ###########      ###########

  $self->step("Check that no updates needed in this package");
  
  if (! $self->up_to_date() ) {
    ierror("Release: Package not up to date - try a commit first\n");
    $self->release_lock();
    return 0;
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Update package dependencies");
  
  # Deal with the new dependencies:
  # let the user select a new dependeny for each old one; note that
  # the referenced dependencies have to exist in the repositoryDB in
  # order to be selectable;
  # we do this before we issue any cvs operations on the repository,
  # so the user can back out without adverse effect;

  my @dependencies = $self->dependencies();

  if ($#dependencies >= 0) {
    $, = " "; # array element separator

    print "\nPackage '$name' lists the following package dependencies:\n".
          "  @dependencies\n".
          "You need to update these dependencies now.\n\n";

    my @new_dependencies;

    # get all known repository tags
    my @repositories = $repositoryDB->directory();

    # let the user update each dependency
    foreach my $dep (@dependencies) {
      my $dep_sans = $dep;
      $dep_sans =~ s/\/.*$//; # strip tag ID off dependency name

      # filter list of all repositories with the ones that match the
      # basename (= name w/o tag ID) of the current dependency
      my @repository_select_list;
      foreach my $rep (@repositories) {
        # add all repositories with the same basename
        if ($rep =~ m/^$dep_sans(\/.*)?$/) {
          push @repository_select_list, $rep;
        }
      }
      # let the user choose a new dependency
      my $new_dep = Asim::choose_name(
        "Pick new dependency for current dependency '$dep'", @repository_select_list);
      push @new_dependencies, $new_dep;
    }

    print "\nIn '$name' release '$version', the current package dependencies\n".
          "  @dependencies\n".
          "will be replaced with the new package dependencies\n".
          "  @new_dependencies\n";

    if (! Asim::choose_yes_or_no("Is this correct?", "yes", "yes")) {
      $self->release_lock();
      return 0;
    }
   
    $self->dependencies(@new_dependencies);
  }

  #########      ###########      ###########      ###########      ###########

  print "\n";
  print "Release: Checked out package '$name' is about to get fixated as release '$version'.\n";
  print "         This process also creates the CVS branch '$branchname'.\n";
  print "         You cannot undo this, even manually!\n";
  print "\n";
  
  if (! Asim::choose_yes_or_no("Are you sure you want to proceed", "no", "yes")) {
    $self->release_lock();
    return 0;
  }

    
  # start transaction
  Asim::Xaction::start();

  #########      ###########      ###########      ###########      ###########

  $self->step("Cvs branch repository / package");

  if (! $self->cvs_command("tag -b $branchname")) {
    ierror("Branch tagging failed - repository may be corrupted\n");
    Asim::Xaction::abort();
    return 0;
  }

  if (! $self->cvs_command("update -P -r $branchname")) {
    ierror("Checkout of branch failed - this package is trash --- delete it\n");
    Asim::Xaction::abort();
    return 0;
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Change package file for the branch....");

  $self->tag($branchname);

  if (! ($newcsn = $self->csn("CSN-${branchname}-1"))) {
    branch_failure("Internal error - cannot obtain CSN\n");
    Asim::Xaction::abort();
    return 0;
  }

  $self->save();

  #########      ###########      ###########      ###########      ###########

  $self->step("Replace mytags with fresh one");

  if (system("echo $newcsn >$location/$MYTAGS")) {
    branch_failure("Failed to create $MYTAGS");
    Asim::Xaction::abort();
    return 0;
  }


  #########      ###########      ###########      ###########      ###########

  $self->step("Add mergetags");

  if (system("echo $oldcsn >>$location/$MYMERGEPOINTS")) {
    branch_failure("Failed to create $MYMERGEPOINTS");
    Asim::Xaction::abort();
    return 0;
  }

  if (! $self->cvs_command("add $MYMERGEPOINTS")) {
    branch_failure("Could not cvs add $MYMERGEPOINTS");
    Asim::Xaction::abort();
    return 0;
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Edit .pack file entry for branch");

  # TODO: For one, we have to find the/all .pack files, and we should
  # make a change consistent with above change in the admin/packages/*
  # file. What is the .pack file contents used for these days as
  # compared to the admin/packages/* file? Can they actually point to
  # different versions without breaking things?

  print "This is up to you... make sure it is consistent with admin/packages\n";


  #########      ###########      ###########      ###########      ###########

  $self->step("Edit changes file");

  # Add an entry to the changes file and let the user manually add
  # some comments

  $self->edit_changes_for_release($branchname, $version, $patchname);

  #########      ###########      ###########      ###########      ###########

  # Note: We are done with all changes to files in this package and
  # can commit it now, so the first CSN that we get will have all
  # admin files appropriately changed for this release.

  $self->step("Cvs commit branch");

  #
  # Use the final commit operation of Asim::Package::commit
  # That way we are more likely to get the semantics right
  #

  if (! $self->commit_archive()) {
    ierror("Release: Branch commit failed\n");
    Asim::Xaction::abort();
    return 0;
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Cvs tag this release with initial patch level ${patchlevel}");

  if (! $self->cvs_command("tag $patchname")) {
    ierror("Initial release tagging failed - repository may be corrupted\n");
    Asim::Xaction::abort();
    return 0;
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Unlock repository");

  $self->release_lock();

  Asim::Xaction::commit();

  return 1;
}


sub edit_changes_for_release {
  my $self = shift;
  my $branchname = shift;
  my $version = shift;
  my $patchname = shift;

  my $base        = $self->location();
  my $changes     = "$base/$CHANGES";

  my $csn         = $self->csn();

  my $date;
  my $user;

  print "Release: Generating a new 'changes' file...\n";

  Asim::Xaction::add_file($changes);

  safe_append(*CHANGES, $changes)
    || ierror("Cannot open $changes\n") && return ();

  #
  # Update the changes file
  #
  $date = `date`;
  chop $date;
  $user = $ENV{'USER'};

  print CHANGES
    "\n----------\n".
    "$user\tDate: $date\tCSN: $csn\n\n".
    "This package has been released as version $version.\n".
    "A version branch has been created with CVS branch tag $branchname.\n".
    "You have left the CVS trunk and you are now in this version branch!\n\n".
    "The first patchlevel (CVS instance tag) will be tagged $patchname.\n";

  safe_close(*CHANGES);

  # let the user add arbitrary comments to the changes file
  return $self->edit_changes_manual();
}


#
# branch a package from the HEAD development trunk,
# or from wherever you checked out your working copy.
#
sub branch {
  my $self = shift;
  my $branchname = shift ||
    ierror("Branch: Branch name must be specified\n") && return 0;

  my $name = $self->name();
  my $location = $self->location();
  my $lockname = "asim-$branchname";
  my $oldcsn = $self->csn();
  my $newcsn;

  my @files;

  print "Make a branch to the repository\n";

  #########      ###########      ###########      ###########      ###########

  $self->step("Sanity checks", 0);

  $self->sanity_stage();

  # in the old CVS way of doing things, branching from a branch was not allowed.
  # In SVN we don't have this restriction.
  if ( $self->type() ne 'svn' ) {
    if (-e "$location/$MYMERGEPOINTS") {
      ierror("Branch: Cannot branch a branch\n");
      return 0;
    }
  }

  # Check that there doesn't already exist a branch with this name.
  my @branchtags = $self->find_branchtags($MYTAGS);
  my $tag;
  foreach $tag (@branchtags) {
    if ($tag eq $branchname) {
      ierror("Branch: Cannot create branch $branchname.\n".
             "         A branch named $branchname already exists.\n");
      return 0;
    }
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Lock repository");

  $self->acquire_lock()
    || ierror("Branch: Could not acquire lock\n") && return ();


  #########      ###########      ###########      ###########      ###########

  $self->step("Check that no updates needed in this package");

  # in the old CVS way of doing things, we had to commit all changes before
  # creating a branch.  With SVN we don't have this restriction,
  # but we'll still issue a warning:  
  if (! $self->up_to_date() ) {
    if ( $self->type() eq 'svn' ) {
      iwarn("Branch: Package not up to date - consider commiting changes first\n");
    } else {
      ierror("Branch: Package not up to date - try a commit first\n");
      $self->release_lock();
      return 0;
    }
  }

  print "\n";
  print "Branch: This checked out package tree is about to be converted into a branch\n";
  print "\n";
  
  if (! Asim::choose_yes_or_no("Are you sure you want to proceed", "no", "yes")) {
    $self->release_lock();
    return 0;
  }
    
  #########      ###########      ###########      ###########      ###########

  $self->step("Branch repository and convert current package into new repository");
  
  Asim::Xaction::start();

  # hack alert!
  # Remove this IF statement by crating type-specific methods in Cvs.pm, Svn.pm, etc.
  if ( $self->type() eq 'cvs' ) {
  
    ##########     CVS    ##########
  
    $self->cvs_command("tag -b $branchname")
      || ierror("Branch tagging failed - repository may be corrupted\n")
         && Asim::Xaction::abort()
	 && return 0;

    $self->cvs_command("update -P -r $branchname")
      || ierror("Checkout of branch failed - this package is trash --- delete it\n")
         && Asim::Xaction::abort()
	 && return 0;

  } elsif ( $self->type() eq 'svn' ) {
   
    ##########     SVN    ##########
  
    my $cur_url    = $self->get_working_url();

    my $use_tags = Asim::choose_yes_or_no("Put branch in tags/ instead of branches/?", "no", "no");
    my $branch_dir = $use_tags?"/tags/":"/branches/";

    my $branch_url = $self->get_repository_url() . $branch_dir . $branchname;

    # create a comment file header, nicely formatted so it gets past SVN pre-commit hook:
    $self->{commentfile} = "$TMPDIR/asimbranch_comment.$$.txt";

    open COMMENT, '>'.$self->{commentfile};
    my $csn = $self->csn();
    chomp(my $date     = `$DATE`);
    chomp(my $date_utc = `$DATE_UTC`);
    printf COMMENT "%-10s  Date: %s  CSN: %s\n%-10s        %s\n\n", $USER, $date, $csn, "", $date_utc;
    print  COMMENT "";
    print  COMMENT "Branch from revision: $csn\n";
    print  COMMENT "From URL:             $cur_url\n";
    print  COMMENT "To URL:               $branch_url\n";
    print  COMMENT "";
    close  COMMENT;

    Asim::invoke_editor("--eof", $self->{commentfile});

    # if we're not up to date, copy the currently checked out revision
    my $rev_arg = '';
    if (! $self->up_to_date() ) {
      $rev_arg = '-r ' . $self->get_working_revision();
    }
    
    # create the branch in the repository, using the supplied branch name:
    $self->svn("copy $cur_url $rev_arg $branch_url --file " . $self->{commentfile});
    $self->svn("switch $branch_url");
 
  } else {

    ierror("Branch not implemented for packages of type " . $self->type() . "\n");
  }

  #########      ###########      ###########      ###########      ###########

  $self->step("Unlock repository");
  
  Asim::Xaction::commit();

  $self->release_lock();

  #
  # Now create a new admin/packages/<packagename> file for the branch,
  # where <packagename> is the new branch name.  And we also create a new lock
  # just for this branch, and a new set of CSN's starting at 1.
  #
  # But for SVN we will skip this step, and just share the package name,
  # lock, and CSNs with the trunk.  Note that since the CSNs are synced to
  # the SVN version numbers we don't have a choice about that...
  #
  if ( $self->type() eq 'svn' ) {
    return 1;
  }


  # We don't lock the new repository because noone should know about it yet...

  #########      ###########      ###########      ###########      ###########

  $self->step("Create and then acquire lock for this package....");

  my $lock = Asim::Lock->new($lockname);
  
  $lock->create_lock() ||
    ierror("Could not create lock for branch ($lockname)\n") && return 0;

  $lock->acquire_lock() ||
    ierror("Could not acquire lock for branch ($lockname)\n") && return 0;

  #########      ###########      ###########      ###########      ###########

  $self->step("Create package file for the branch....");

  system("(cd $location/$PACKAGES; mv $name $branchname)")
    && (branch_failure("Failed to create $PACKAGES/$branchname file") && return 0);

  $self->open("$location/$PACKAGES/$branchname");

  $self->name($branchname);
  $self->tag($branchname);

  $newcsn = $self->csn(1)
    || branch_failure("Internal error - cannot obtain CSN\n") && return 0;

  $self->lockname($lockname);

  $self->save();

  $self->cvs_command("add $PACKAGES/$branchname")
    || branch_failure("Could not cvs add $PACKAGES/$branchname file") && return 0;

  $self->cvs_command("remove $PACKAGES/$name")
    || branch_failure("Could not cvs remove $PACKAGES/$name file") && return 0;

  #########      ###########      ###########      ###########      ###########

  $self->step("Replace mytags with fresh one");

  system("echo $newcsn >$location/$MYTAGS")
    && (branch_failure("Failed to create $MYTAGS") && return 0);


  #########      ###########      ###########      ###########      ###########

  $self->step("Add mergetags");

  system("echo $oldcsn >>$location/$MYMERGEPOINTS")
    && (branch_failure("Failed to create $MYMERGEPOINTS") && return 0);

  $self->cvs_command("add $MYMERGEPOINTS")
    || branch_failure("Could not cvs add $MYMERGEPOINTS") && return 0;;


  #########      ###########      ###########      ###########      ###########

  $self->step("Cvs commit branch");

  #
  # Use the final commit operation of Asim::Package::commit
  # That way we are more likely to get the semantics right
  #

  $self->commit_archive()
    || ierror("Branch commit failed\n") && return 0;


  #########      ###########      ###########      ###########      ###########

  $self->step("Unlock the package....");

  
  $lock->release_lock() ||
    ierror("Could not release lock for branch ($lockname)\n") && return 0;

  #########      ###########      ###########      ###########      ###########

  $self->step("Add .pack file entry for branch");

  print "This is up to you...\n";

  return 1;
}


sub branch_failure {
  my $message = shift;

  print "\n";
  print "Branch: I was in the midst of creating some standard files needed by the branch, but something\n";
  print "Branch: went wrong. You may be able to fix the files, add the ones I did not get to,\n";
  print "Branch: and then commit the changes by hand --- Find an ASIM wizard since I can't help you\n";
  print "\n";
  print "Branch: For what it is worth, the problem was: ------- $message ------\n";
  print "\n";
  print "Branch: If you cannot fix the problem, note the branch name is already in use in the repository\n";
  print "Branch: so you need to pick a new one now...\n";
  print "\n";

  return 1;
}



sub merge {
  my $self = shift;
  my $explicitcsn = shift;

  my $name = $self->name();
  my $location = $self->location();

  my $oldmergecsn;
  my $newmergecsn;
  
  if ( $self->type() ne 'cvs' ) {
    ierror("Merge not implemented for packages of type " . $self->type() . "\n");
    return 0;
  }

  print "Merge changes in the main trunk into a branch\n";


  #########      ###########      ###########      ###########      ###########

  $self->step("Check that we have an updated package", 0);

  if (! -e "$location/$MYMERGEPOINTS") {
    ierror("Merge: Does not appear to be a branch - no $MYMERGEPOINTS\n");
    return 0;
  }
    
  if (! $self->up_to_date() ) {
    ierror("Merge: Package not up to date - try to update and/or commit first\n");
    return 0;
  }


  #########      ###########      ###########      ###########      ###########

  $self->step("Protect certain files that are not to be merged");

  Asim::Xaction::start();
  Asim::Xaction::protect_file("$location/$MYTAGS");
  Asim::Xaction::protect_file("$location/$MYMERGEPOINTS");
  Asim::Xaction::protect_file("$location/$IPCHIST");


  #########      ###########      ###########      ###########      ###########

  $self->step("Get csn of previous merge point");

  #
  # Get last mergepoint
  #
  $oldmergecsn = $self->get_last_csn("$location/$MYMERGEPOINTS");

  if (! $oldmergecsn) {
    merge_failure("Merge: Malformed csn ($oldmergecsn) in $MYMERGEPOINTS");
    Asim::Xaction::abort();
    return 0;
  }


  #########      ###########      ###########      ###########      ###########

  if (! defined($explicitcsn)) {
    $self->step("Get most recent csn of the main trunk");

    #
    # Force checkout of $MYTAGS from main trunk
    #
    system("rm $location/$MYTAGS");
    $self->cvs_command("update -A $MYTAGS");

    #
    # Get latest tag
    #
    $newmergecsn = $self->get_last_csn("$location/$MYTAGS");

    #
    # Restore orignal $MYTAGS for branch
    #
    system("rm $location/$MYTAGS");
    $self->cvs_command("update -r $name $MYTAGS");
  } else {
    $newmergecsn = $explicitcsn;
  }

  if (! $newmergecsn) {
    merge_failure("Malformed new mergepoint csn in $MYTAGS of main trunk\n");
    Asim::Xaction::abort();
    return 0;
  }

  if ($oldmergecsn eq $newmergecsn) {
    ierror("Old and new mergepoints ($oldmergecsn) identical - no merge needed\n");
    Asim::Xaction::abort();
    return 0;
  }    


  #########      ###########      ###########      ###########      ###########


  $self->step("Merging changes from maintrunk csn $oldmergecsn to csn $newmergecsn!!!");

  $self->cvs_command("update -dP -j $oldmergecsn -j $newmergecsn");


  #########      ###########      ###########      ###########      ###########

  $self->step("Restore files that were clobbered by merge");

  Asim::Xaction::commit();


  #########      ###########      ###########      ###########      ###########

  $self->step("Update $MYMERGEPOINTS");

  #TODO: Implement this with safe_append...
  #BUG:  If we can't do the following future merges will be corrupted....

  system("echo $newmergecsn >>$location/$MYMERGEPOINTS")
    && merge_failure("Could not update $MYMERGEPOINTS - do not commit\n");;

  #########      ###########      ###########      ###########      ###########

  #TODO: Fix changes file rationally so user doesn't have to do it...

  #########      ###########      ###########      ###########      ###########

  $self->step("Merge is finished");

  print "Well the merge is finished, now fix whatever problems you see.\n";
  print "Then start a regression and get this puppy checked in,\n";
  print "because you DO NOT have a lock out on it...\n";

  return 1;
}


sub merge_failure {
  my $message = shift;
  
  print "\n";
  print "Merge: I was in the midst of merging the main trunk of the repository with your branch\n";
  print "Merge: but something went wrong. You may be able to fix the problem, but you did just start\n";
  print "Merge: with a clean package, so I suggest you just wipe it clean, check it out again, and\n";
  print "Merge: and try again.\n";
  print "\n";
  print "To help avoid failing again the problem was: $message\n";

  return 1;
}



################################################################
#
# Function: get_last_csn
#
# Get the highest CSN out of a file of CSNs.
#
################################################################
      
sub get_last_csn {
  my $self = shift;
  my $file = shift;

  my $csn;

  #
  # Get last csn from file
  #   This used to require a sort, but I am now depending
  #   on the fact that CSNs are in ascending order in the file.
  #
  $csn = `tail -1l $file`;
  chomp $csn;

  #
  # Check for old style csn that was just a number
  #
  if ($csn =~ /^d+$/) {
    # Old style CSN
    $csn = "CSN$csn";
    return $csn;
  }

  #
  # Check for malformed CSN
  #
  if (! $csn =~ /^CSN-.*-\d+$/) {
    return ();
  }

  return $csn;
}


1;
