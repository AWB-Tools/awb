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

=cut

################################################################

=item $cvs = Asim::Package::Svn::set_type( $package )

If $package is an SVN repository, set the object's subclass to Asim::Package::Svn and return 1.
Otherwise return 0.

=cut

################################################################

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

################################################################

=item $package-E<gt>type()

Return type of package - "svn" in this case.

=cut

################################################################

sub type {
  return 'svn';
}

################################################################

=item $svn = Asim::Package::Svn:init()

Global init of Svn module.

=cut

################################################################


sub init {
  # Deal with any SVN environment variables
  1;
}

################################################################

=item $svn-E<gt>update()

Update this package from the repository

=cut

################################################################

sub update {
  my $self = shift;

  my $location = $self->location();
  print "Updating package from SVN repository \n";
  eval {print `(cd $location; svn update)`};

  if ($@) {
      warn $@;
      return undef;
  }

  return 1;
}

################################################################
#
# Function: status
#
# Check on the SVN status of each file in the current package
#
# Return a array with one element for each file returned 
# by 'svn status' addin state:
#
# Each array element contains:
#
#  ($directory, $filename, $status, $reprev, $date)
#
################################################################

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
      ierror("status: 'svn status' failed on $location!\nSorry, commiting is not possible\n");
      return undef;
    }

    # Example status is of this format. We are interested in the ones shown 
    # by arrows
    # M   *   2   2   spartha1   stuff/things.c 
    # ^   ^   ^                        ^
    # |   |   |                        |

    if ( /^(.?)\s+(\*?)\s*(\d+)\s+(\d+)\s+(\S+)\s+(\S+)$/ ) {

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

################################################################
#
# Persistent CVS command
#
#   Remote cvs has the habit of giving up (timeout) sometimes;
#   We keep retrying a number of times before we give up too
#
################################################################

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

################################################################
#
# SVN command
#
################################################################


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

################################################################

=item $svn-E<gt>commit_check()

A set of checks to make sure that we have a 
reasonable chance of succeeding at a commit

=cut

################################################################

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

################################################################

=item $package-E<gt>increment_csn()

Increment serial number CSN of package.
Return the current (updated) serial number (CSN)  of package.

This overloads the default version in Package.pm,
because we have some special-case code to keep the CSN numbers
in sync with the SVN revision numbers.

=cut

################################################################

sub increment_csn {
  my $self = shift;

  # get the current serial number tag:
  my $csn = $self->csn();
  
  # Force an update to ensure revision no. & csn no. are in sync
  system("svn update");

  # open a temporary file to read the repository status
  my $location = $self->location();
  my $tmp_svn_status = "/tmp/asim-shell-svn-status.$$";
  system("cd $location;svn status -v > $tmp_svn_status 2>&1");

  if (!CORE::open(SVN,"< $tmp_svn_status") ) {
    ierror("Can't cat status file: $!\n");
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

=back

=head1 BUGS

=head1 AUTHORS

Sailashri Parthasarathy based on Bitkeeper.pm by
  Oscar Rosell based on Cvs.pm by
    Joel Emer, Roger Espasa, Artur Klauser and  Pritpal Ahuja

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2006

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
