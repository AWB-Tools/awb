#
# *****************************************************************************
# *
# * @brief Svn.pm : utility methods to handle SVN repositories
# *
# * @author Sailashri Parthasarathy
# *
# Copyright (C) 2006 Intel Corporation
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

package Asim::Package::Svn;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Package);


=head1 NAME

Asim::Package::Svn - Class to manipulate a checked out Svn repository.

=head1 SYNOPSIS

use Asim::Package::Svn;

Asim::Package::Svn::init();

my $svn = Asim::Package::Svn->new("~/svndir");

$svn->update();


=head1 DESCRIPTION

This is a class to allow a directory to be treated as 
a SVN repository. 

This is a subclass of Asim::Package.  After creating an instance of Asim::Package,
you can call the set_type() method here to check whether the package is in a SVN
repository and set it to this subclass if so.  Set_type will return 1 if it is a
SVN package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

=item $cvs = Asim::Package::Svn::set_type( $package )

If $package is an SVN repository, set the object's subclass to Asim::Package::Svn and return 1.
Otherwise return 0.

=cut

sub set_type {
  my $self = shift;
  my $location = $self->location();
  # If there is a ".svn" subdirectory, then this is Subversion:
  if (-e "$location/.svn") {
    bless $self;
    return 1;
  }
  return 0;
}

=item $package-E<gt>type()

Return type of package - "svn" in this case.

=cut

sub type {
  return 'svn';
}

=item $svn = Asim::Package::Svn:init()

Global init of Svn module.

=cut

sub init {
  # Deal with any SVN environment variables
  1;
}

=item $svn-E<gt>update([version])

Update this package from the repository.

If an optional version string is specified,
update the package to that version.
The version specifier can be 'HEAD', the name of a branch,
a CSN label 'CSN-pgname-number' or the name of a tag.

=cut

sub update {
  my $self = shift;
  my $version = shift;

  my $location = $self->location();
  my $command = 'update';
  my $args = '';
  
  if ( $version ) {
    # a version to update to was explicitly specified.
    
    # if the version is a fixed version, i.e. a CSN number,
    # then get the SVN revision number this corresponds to.
    if ( $self->is_fixed_version( $version ) ) {
      $args .= '-r ' . $self->get_revision_number( $version );
    }
    
    # get the URL of the specified version, and use SVN "switch"
    # command if it differs from the currently checked out URL.
    # NOTE! that we do not simply compare the version against 'HEAD' here,
    # because we might be on a branch and we want to update to 'HEAD',
    # which means we want to do an 'svn switch' to get back to the trunk.

    my $url = $self->get_version_url( $version );
    if ( $url ne $self->get_working_url() ) {
      $command = 'switch ' . $url;
    }
  }

  # do the update
  
  my $status = system("cd $location; svn $command $args");

  if ($status) {
      return undef;
  }

  return 1;
}

=item $svn-E<gt>pull([version])

#Alias to update

=cut

sub pull {

  my $self = shift;
  my $revision = shift;

  return $self->update($revision);

}

=item $svn-E<gt>status()

Check on the SVN status of each file in the current package

Return a array with one element for each file returned 
by 'svn status' addin state:

Each array element contains:

 ($directory, $filename, $status, $reprev, $date)

=cut

sub status {
  my $self = shift;
  my $location = $self->location();
  my @files = ();

  # Execute 'svn status' to see all the changes that have been made.
  
  my $tmp_svn_status = "/tmp/asim-shell-svn-status.$$";

  if (system("cd $location; svn status -u -v >$tmp_svn_status 2>&1")) {
    Asim::Package::ierror("status: Can't launch 'svn status': $!\n");
    return undef;
  }
  if (! CORE::open(SVN,"< $tmp_svn_status") ) {
    Asim::Package::ierror("status: Can't launch 'svn status': $!\n");
    return undef;
  }

  # Parse the SVN status

  my $workrev = "0.0";
  my $repfile;
  my $stickytag;
  my $stickydate;
  my $fail = 0;

  my $VERBOSE = 0;

  while (<SVN>) {
  
    my $dir = "";
    my $file = "";
    my $status = "";
    my $workdate = "";
    my $reprev = "0.0";
    my $path = "";

    print "svn status returned: $_" if ( $VERBOSE );
    #
    # Hack in case the ssh command asks for a password
    #
    if ( /password:/) {
      print "$_\n";
    }
    #
    # Bobbie seems to always get a CVS status 'abort' :-) So we better take care of it
    #
    if ( /abort/ ) {
      Asim::Package::ierror("status: 'svn status' failed on $location!\nSorry, commiting is not possible\n");
      return undef;
    }

    # Example status is of this format. We are interested in the ones shown 
    # by arrows
    # M   *   2   2   spartha1   stuff/things.c 
    # ^   ^   ^                        ^
    # |   |   |                        |

    if ( /^\s?(.?)\s+(\*?)\s*(\d+)\s+(\d+)\s+(\S+)\s+(\S+)$/ ) {

      # Convert status into CVS status equivalent
      if (($1 eq 'M') and !($2)) {
	$status = 'Locally Modified';
      }
      elsif (($1 eq 'D') and !($2)) {
	$status = 'Locally Removed';
      }
      elsif (($1 eq 'M') and ($2 eq '*')) {
	$status = 'Needs Merge';
      }
      elsif ($2 eq '*') {
	$status = 'Needs Checkout';
      }
      elsif (($1 eq 'C') and !($2)) {
	$status = 'Conflict';
      }
      elsif (($1 eq '?') or ($1 eq 'X') or ($1 eq 'I')) {
        $status = 'Unknown';
      }
      # check this one
      elsif ($1 eq '~') {
	$status = 'Needs Checkout';
      }
      elsif ($1 eq '!') {
	$status = 'Needs Checkout';
      }
      elsif (!($1 and $2)) {
	$status = 'Up-to-date';
      }
    $reprev = $3;   # Indicates the revision no
    $path = $6;
   }

   # The below statuses have a different format
   elsif (/^(A)\s+(\d+)\s+(\?)\s+(\?)\s+(\S+)/)  {
      $status = 'Locally Added';
      $reprev = $2;
      $path = $5;
    }

   elsif (/^(A)\s+(\+)\s+(\-)\s+(\?)\s+(\?)\s+(\S+)/)  {
      $status = 'Locally Added';
      $reprev = $3;
      $path = $6;
    }

   # for files that were added through svn
   # but removed without letting svn know
   elsif (/^(!)\s+(\d+)\s+(\?)\s+(\?)\s+(\S+)/)  {
      $status = 'Needs Checkout';
      $reprev = $2;
      $path = $5;
   }

   # for dirs that were added through svn
   # but removed without letting svn know
   elsif (/^(!)\s+(\?)\s+(\?)\s+(\?)\s+(\S+)/)  {
      $status = 'Needs Checkout';
      $reprev = $2;
      $path = $5;
   }

   ## for modified locked files
   elsif (/^(M)\s+(K)\s*(\d+)\s+(\d+)\s+(\S+)\s+(\S+)/) {
      $status = 'Locally Modified';
      $reprev = $3;  
      $path = $6;
   }
   
   elsif (/^(\?)\s+(\S+)/) {
      $path = $2;
      $file = basename($path);
      # Don't display svn specific, VI specific and junk files like:
      # abc.mine, abc.r13, abc.swp, #abc
      if (!(($file =~ /.mine/) || ($file =~ /.r(\d+)/) || ($file =~ /.swp/) || ($file =~ /#(\S+)/))) {
	$status = 'Unknown';
      }
   }
    
   # Check this one
   elsif (/^(X)\s+(\S+)/) {
      $status = 'Unknown';
      $path = $2;
   }

   # Will not enter this loop
   # since --no-ignore option is not passed to status
   elsif (/^(I)\s+(\S+)/) {
      $status = 'Unknown';
      $path = $2;
   }
    
   $dir = dirname($path);  # Indicates the dir
   $file = basename($path);# Indicates the file 
   if (-d "$location/$path") {
      $file = $file."/";
   }
 
   # This is to gate the push. Else, data is pushed 
   # even if none of the above conditions hold true.
    if ($status) {
       push(@files, [$dir, $file, $status, $reprev, $workdate]);
   }
   $workdate = "";
   $reprev = "0.0";
   $status = undef;
  }
  CORE::close(SVN);
  system("rm $tmp_svn_status");

  return (@files);
}

=item $svn->svn_command( <command> )

Issue an SVN command robustly.

Remote cvs had the habit of giving up (timeout) sometimes;
This implementation keeps retrying a number of times before we give up too.

=cut

sub svn_command {
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
    $ret = $self->svn($command);
    if ($ret == 0) {
      last;
    }

    if ($retry < $NUM_RETRY) {
      printf "Caught error during svn operation - retrying %d more times\n",
      $NUM_RETRY - $retry;
      sleep 5;
    } else {
      return 0;
    }
  }

  return 1;
}

=item $svn->svn( <command> )

Issue an SVN command directly.  Don't retry if it fails.

=cut

sub svn {
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
  $command = "(cd $location; svn $command)";
  print "$command\n" if $VERBOSE;

  $ret = system("$command") >> 8;

  return $ret;
}

=item $svn->get_working_url()

Return the URL that the working copy was checked out with.
This will usually look like: http://some/path/trunk
or maybe http://some/path/branches/<branch_name>.

=cut

sub get_working_url
{
  my $self = shift;
  my $location = $self->location();
  my $url = `cd $location; svn info | grep URL`;
  chomp $url;
  $url =~ s/^URL:\s*//;
  $url;
}

=item $svn->get_repository_url()

Return the URL of the top level of the repository,
which is just the working URL, with the "/trunk"
or "/branches/<branch_name>" removed.

=cut

sub get_repository_url
{
  my $self = shift;
  my $url = $self->get_working_url();
  $url =~ s/\/trunk$//;
  $url =~ s/\/branches\/.*$//;
  $url =~ s/\/tags\/.*$//;
  $url;
}

=item $svn-E<gt>get_version_url(<version>)

Return the SVN URL for the given version, i.e. the URL
used to check out the given branch, tag, or trunk.
The <version> argument is any accepted version tag syntax, e.g.:

    HEAD
    <branchname>
    CSN-<packagename>-<number>
    CSN-<branchname>-<number>
    <number>
    <branchname>:<number>

=cut

sub get_version_url {
  my $self    = shift;
  my $version = shift;
  
  # get the top-level URL of the repository
  my $repo_url = $self->get_repository_url();
  
  # extract branch name if any
  if    ( $version =~ m/^CSN-([a-z_0-9]+)-[0-9]+$/i ) { $version = $1; }
  elsif ( $version =~     m/^([A-Z_0-9]+):[0-9]+$/i ) { $version = $1; }
  
  # if it looks like a tag or a branch, search for it:
  if ( $version && $version ne 'HEAD' ) {
    foreach my $subdir ( 'tags', 'branches' ) {
      open LIST, "svn list $repo_url/$subdir |";
      while ( <LIST> ) {
        if ( m/^$version\// ) {
          return "$repo_url/$subdir/$version";
        }
      }
      close LIST;
    }
  }
  
  # if all else fails, just return the HEAD URL
  return "$repo_url/trunk";
}

=item $svn-E<gt>is_fixed_version(<version>)

Does the given version string specify a fixed version,
such as a "CSN" label or an SVN revision number?

=cut

sub is_fixed_version {
  my $self    = shift;
  my $version = shift;
  
  if ( $version =~ m/^CSN-.*-[0-9]+$/ ) { return 1; }
  if ( $version =~     m/^.*:[0-9]+$/ ) { return 1; }
  if ( $version =~        m/^[0-9]+$/ ) { return 1; }
  return 0;
}

=item $svn-E<gt>get_revision_number(<version>)

Return the SVN revision number associated with the given version.

This returns an integer that you can use as a "-r" argument
to various svn commands.

If is_fixed_version would have returned 0, the result is undefined.

=cut

sub get_revision_number {
  my $self    = shift;
  my $version = shift;
  
  if ( $version =~ m/^CSN-.*-([0-9]+)$/ ) { return $1; }
  if ( $version =~     m/^.*:([0-9]+)$/ ) { return $1; }
  if ( $version =~        m/^([0-9]+)$/ ) { return $1; }
  return undef;
}

=item $svn->get_working_revision()

Return the SVN revision number that the working copy was checked out with.

This returns an integer that you can use as a "-r" argument
to various svn commands.

=cut

sub get_working_revision {
  my $self = shift;
  my $location = $self->location();
  my $rev = `cd $location; svn info | grep Revision`;
  chomp $rev;
  $rev =~ s/^Revision:\s*//;
  $rev;
}

=item Svn::is_branch_url(<URL>)

This static class function return 1 if and only if
the given repository URL is for a branch of the package.

=cut

sub is_branch_url {
  my $url = shift;
  return ( $url =~ m/\/branches\// );
}

=item Svn::is_tag_url(<URL>)

This static class function return 1 if and only if
the given repository URL is for a tagged version of the package.

=cut

sub is_tag_url {
  my $url = shift;
  return ( $url =~ m/\/tags\// );
}

=item Svn::branch_name_from_url(<URL>)

This static class function extracts the branch name from the given URL.
Returns undef if is_branch_url() would have returned 0

=cut

sub branch_name_from_url {
  my $url = shift;
  if ( $url =~ m/\/branches\/([^\/]+)/ ) {
    return $1;
  }
  return undef;
}

=item Svn::tag_name_from_url(<URL>)

This static class function extracts the tagged version name from the given URL.
Returns undef if is_tag_url() would have returned 0

=cut

sub tag_name_from_url {
  my $url = shift;
  if ( $url =~ m/\/tags\/([^\/]+)/ ) {
    return $1;
  }
  return undef;
}

=item $package-E<gt>baseline_tag([<use_csn>])

Return a tag that can be used later
to retrieve exactly the version of the package that is currently
checked out in the working copy (minus any uncommitted changes).

In Asim SVN repositories (unlike in CVS) we do not maintain the branch
name and revision numbers as part of the CSN in the admin/packages file.
Instead, we extract the branch or tag name from the working copy's URL,
and we explicitly get the working revision number, by querying SVN
on the working copy's top-level directory.

If the optional <use_csn> flag is passed in as '1', then use the
CSN information from the admin/packages file instead of querying
SVN directly to get the working copy revision number.  Sometimes these
version can get out of sync, e.g. because of checkins on other branches,
or because people "cheat" and do not use asim-shell to commit.

=cut

sub baseline_tag
{
  my $self = shift;
  my $use_csn = shift;
  my $url  = $self->get_working_url();
  my $rev  = $self->get_working_revision();
  if      ( is_tag_url( $url ) ) {
    return tag_name_from_url( $url );
  } elsif ( is_branch_url( $url ) ) {
    return branch_name_from_url( $url ) . ':' . $rev;
  } elsif ( $use_csn ) {
    return $self->csn();
  } else {
    return $rev;
  }
}

=item $svn->find_branchtags( [<filename>] )

Search a repository file for all branch tags.

The optional filename argument is ignored
(for backward compatibility with the old CVS version).

=cut

sub find_branchtags
{
  my $self = shift;
  my $filename = shift || '';  # optional argument -- ignored
  my @branches = ();
  my $url = $self->get_repository_url() . '/branches';
  foreach my $branch ( `svn list $url` ) {
    if ( $branch =~ m/non-existent/ ) { last; }
    chomp $branch;
    $branch =~ s/\/\s*$//;
    push @branches, $branch;
  }

  return @branches;
}

=item $svn->find_labels()

Search a repository file for all symbolic tags (labels).

For SVN repositories, this returns a list of all subdirectory names
underneath the "tags" directory below the repository root.

=cut

sub find_labels
{
  my $self = shift;
  my @labels = ();
  my $url = $self->get_repository_url() . '/tags';
  foreach my $label ( `svn list $url` ) {
    if ( $label =~ m/non-existent/ ) { last; }
    chomp $label;
    $label =~ s/\/\s*$//;
    push @labels, $label;
  }
  return @labels;
}

=item $svn-E<gt>branch_name()

Return the name of the branch that this repository was checked
out of.  If it was checked out of the main trunk, return the
value 'HEAD'.

=cut

sub branch_name
{
  my $self = shift;
  my $url  = $self->get_working_url();
  if ( is_branch_url( $url ) ) {
    return branch_name_from_url( $url );
  }
  return 'HEAD';
}

=item $svn-E<gt>commit_check()

A set of checks to make sure that we have a 
reasonable chance of succeeding at a commit

=cut

sub commit_check {
  my $self = shift;
  my $reportfile = shift || "/dev/null";

  # Change this to 1 if you want the really strict version of the checks

  my $STRICT_CHECK = 0;

  my  @DANGEROUS_DIRS = (  
                         "base",
                         "feeders",
                         "pm"
                        );


  print "Commit: Checking the status of your source...\n";

  $self->{status} = "No-commit-needed";
  $self->{must_have_regression} = 0;
  $self->{regtestdir} = undef;

  my @files = $self->status();

  if (! defined($files[0])) { 
    Asim::Package::ierror("Commit: Could not determine status of files\n");
    $self->{status} = "Indeterminate-status";
    return 0;
  }
  

  # Create the file that will hold the status report

  if (!CORE::open(TMP,"> $reportfile")) {
    Asim::Package::ierror("check: Cannot open $reportfile\n");
    return 0;
  }

  my $dir_printed = "";
  my $fail = 0;

  foreach my $i (@files) {
    my ($dir, $file, $status, $reprev, $workdate) = @{$i};

    #
    # Parse the SVN status while enforcing the following rules:
    #
    #  a. No file is in the "Needs Patch" State 
    #  b. No file is in the "Needs Checkout" State
    #  c. If any file modified happens to be in a directory that matches the
    #     array DANGEROUS_DIRS, then the user WILL be required to provide (:-))
    #     a regression test to perform this commit.
    #  d. No file is in the "Unresolved Conflict" State
    #     - deleted repository files that have been modified locally
    #  e. No file is in the "Zombie" (pseudo) State
    #     - like d. but on pre 1.11.2 CVS servers that have a bug
    #

    my $VERBOSE = 1;

   if ( $status =~ /Needs Checkout/ || $status =~ /Conflict/ ||
	$status =~ /Needs Merge/ ||
         ($STRICT_CHECK == 1 && $status =~ /Conflict/) )
    {
      print ("Commit: File: $dir/$file has unacceptable state \"$status\" \n");
#      if ($status eq "Zombie" || $status eq "Unresolved Conflict") {
#        print "Commit:       You are trying to check in changes for this file\n";
#        print "Commit:       that somebody else has already deleted!\n";
#        print "Commit:       You need to DELETE YOUR FILE MANUALLY.\n";
#      }
      $fail = 1;
    }
    #
    # If file is ok, then dump it to report file only if it is NOT already up-to-date
    #
    elsif ( (!($status =~ /Up-to-date/)) && (!($status =~ /Unknown/)) ) {
      $self->{status} = "Commit-needed";

      print TMP "\nDirectory $dir\n" if ( $dir ne $dir_printed );
      $dir_printed = $dir;
      printf TMP " %-25s %-20s %8s %s\n", $file, $status, $reprev, $workdate;

      #
      # Is this file in a dangerous directory ?
      #
      my $tdir = $dir;
      ###print "Is file $file in dir $tdir dangerous?\n";
      if ( $tdir ne "." && Asim::Package::partial_match($tdir,\@DANGEROUS_DIRS) ) {
        print "Commit: Changes in $tdir/$file can only be committed if validated by regression test\n";
        $self->{must_have_regression} = 1;
      }
    }
  }

  CORE::close(TMP);

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

=item $package-E<gt>increment_csn()

Increment serial number CSN of package.
Return the current (updated) serial number (CSN)  of package.

This overloads the default version in Package.pm,
because we have some special-case code to keep the CSN numbers
in sync with the SVN revision numbers.

=cut

sub increment_csn {
  my $self = shift;

  # get the current serial number tag:
  my $csn = $self->csn();
  
  # open a temporary file to read the repository status
  my $location = $self->location();
  my $tmp_svn_status = "/tmp/asim-shell-svn-status.$$";
 
  # Force an update to ensure revision no. & csn no. are in sync
  system("cd $location; svn update");

  system("cd $location;svn status -v > $tmp_svn_status 2>&1");

  if (!CORE::open(SVN,"< $tmp_svn_status") ) {
    Asim::Package::ierror("Can't cat status file: $!\n");
    return undef;
  }

  my $rev;
  while (<SVN>) {
    if (/^(\S?)\s+(\d+)(.*)$/) {
      # extract the revision number from the file status output
      $rev = $2;
      last;
    }
  }
  CORE::close(SVN);
  system("rm $tmp_svn_status");

  # increment the revison number, and replace it in the tag
  $rev++;
  $csn =~ s/\d+$/$rev/;
  $csn = $self->csn($csn);

  # return the updated tag
  return $csn;
}

=item $package-E<gt>label($tag [,<existing>=0])

Label the currently checked out version of the repository
with the given $tag.  Returns 1 on success.

For SVN packages, this is similar to creating a branch in the repository,
under the directory tags/$tag.

If the optional flag <existing> is set to 1, then an existing tag
will be moved to the current revision.  If the flag is omitted or
passed as zero, then an error will occur if the tag already exists.

An existing tag will not be moved if it already exists on the same
revision of the same working URL.  In this case, a warning is printed,
but this routine still returns 1 for success.

=cut

sub label {
  my $self = shift;
  my $labelname = shift ||
    Asim::Package::ierror("Label: tag name must be specified\n") && return 0;
  my $existing = shift || 0;

  print "\nLabel the repository with a symbolic tag\n\n";

  $self->step('Sanity checks', 0);
  
  # check the syntax of the tag
  $labelname =~ m/^[a-z0-9_]+$/i ||
    Asim::Package::ierror("Label: tag must contain only alphanumeric and underscore characters\n") && return 0;

  # check the commit environment, since we need to commit a copy to the repository
  $self->sanity_stage();

  # Check that there doesn't already exist a branch or tag with this name.
  my $found = 0;
  foreach my $tag ( $self->find_branchtags(), $self->find_labels() ) {
    if ($tag eq $labelname) {
      $found = 1;
      last;
    }
  }
  if ($existing) { # we are moving an existing tag - it had better exist already!
    if (!$found) {
      Asim::Package::ierror("Label: A tag named \"$labelname\" does not exist yet.\n")
        && return 0;
    }
  } else {         # we are creating a new tag - it had better not exist yet!
    if ($found) {
      Asim::Package::ierror("Label: A tag or branch named \"$labelname\" already exists.\n")
        && return 0;
    }
  }

  my $cur_url = $self->get_working_url();
  my $tag_url = $self->get_repository_url() . '/tags/' . $labelname;
  my $csn = $self->csn();
  my $rev = $self->get_working_revision();
  
  # if you're moving an existing tag, don't bother moving it if you've already tagged the same revision.
  # check this by looking at the last log entry for the destination URL,
  # and scan for the formatted message generated further below:
  if ($existing && $found) {
    my $log_rev = 0;
    my $log_from = '';
    my $log_to = '';
    open DESTLOG, "svn log $tag_url --limit 1 |";
    while (<DESTLOG>) {
      if (/Tag revision:\s+([0-9]+)/) {
        $log_rev = $1;
      } elsif (/From URL:\s+(\S+)/) {
        $log_from = $1;
      } elsif (/To URL:\s+(\S+)/) {
        $log_to = $1;
      }
    }
    if ($log_rev == $rev && $log_from eq $cur_url && $log_to eq $tag_url) {
      Asim::Package::iwarn("Not applying tag $labelname since it already exists on revision $rev\n");
      return 1;
    }
  }

  $self->step('Lock repository');

  $self->acquire_lock()
    || Asim::Package::ierror("Label: Could not acquire lock\n") && return 0;

  $self->step('Check that no updates needed in this package');

  $self->up_to_date() ||
    Asim::Package::iwarn("Branch: Package not up to date - consider updating and/or commiting changes first\n");

  print "\nLabel: The checked out revision of the repository is about to be labelled with tag \"$labelname\"\n\n";
  Asim::choose_yes_or_no('Are you sure you want to proceed', 'no', 'yes') ||
    $self->release_lock() && return 0;

  $self->step('Label the repository');
  
  Asim::Xaction::start();
  
  chomp(my $date     = `$Asim::Package::DATE`);
  chomp(my $date_utc = `$Asim::Package::DATE_UTC`);

  # if we are moving an existing tag, we must remove the old one first
  # or we'll just end up copying into a trunk/ subdirectory there!!
  if ($existing && $found) {
    my $logmesg = $Asim::Package::USER."  Date: $date  CSN: $csn : moving tag $labelname to new revision $rev";
    $self->svn("rm $tag_url -m \"$logmesg\"");
  }

  # create a comment file header, nicely formatted so it gets past SVN pre-commit hook:
  $self->{commentfile} = "$Asim::Package::TMPDIR/asim_label_comment.$$.txt";
  open COMMENT, '>'.$self->{commentfile};
  printf COMMENT "%-10s  Date: %s  CSN: %s\n%-10s        %s\n\n", $Asim::Package::USER, $date, $csn, '', $date_utc;
  print  COMMENT "        Tag revision: $rev\n";
  print  COMMENT "        From URL:     $cur_url\n";
  print  COMMENT "        To URL:       $tag_url\n";
  close  COMMENT;
  Asim::invoke_editor("--eof", $self->{commentfile})
    unless Asim::mode() eq 'batch';

  # create the branch in the repository, using the supplied branch name:
  $self->svn("copy $cur_url -r $rev $tag_url --file " . $self->{commentfile});

  $self->step('Unlock repository');

  Asim::Xaction::commit();
  $self->release_lock();
  return 1;
}

=back

=head1 BUGS

=head1 AUTHORS

Sailashri Parthasarathy, Carl Beckmann based on Bitkeeper.pm by
  Oscar Rosell based on Cvs.pm by
    Joel Emer, Roger Espasa, Artur Klauser and  Pritpal Ahuja

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2006-09

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
