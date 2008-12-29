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

package Asim::Package::Cvs;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Package);

our $SSH_AGENT_KILL_CMD = "";



=head1 NAME

Asim::Package::Cvs - Class to maniputate a checked out CVS repository.


=head1 SYNOPSIS

use Asim::Package::Cvs;

Asim::Package::Cvs::init();

my $cvs = Asim::Package->new("~/cvsdir");

Asim::Package::Cvs->set_type( $cvs );

$cvs->update();


=head1 DESCRIPTION

This is a class to allow a directory to be treated as 
a checked out CVS repository. 

This is a subclass of Asim::Package.  After creating an instance of Asim::Package,
you can call the set_type() method here to check whether the package is in a CVS
repository and set it to this subclass if so.  Set_type will return 1 if it is a
CVS package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $cvs = Asim::Package::Cvs::set_type( $package )

If $package is a CVS repository, set the object's subclass to Asim::Package::Cvs and return 1.
Otherwise return 0.

=cut

################################################################

sub set_type {
  my $self = shift;
  my $location = $self->location();
  # if there is a "CVS" subdirectory, this is a CVS checkout:
  if (-e "$location/CVS") {
    bless $self;
    return 1;
  }
  return 0;
}

################################################################

=item $package-E<gt>type()

Return type of package - "cvs" in this case.

=cut

################################################################

sub type {
  return 'cvs';
}

################################################################

=item $cvs = Asim::Package::Cvs::init()

Global init of CVS module.

=cut

################################################################

sub init {
  #
  # CVS_SERVER messes us up...
  #
  if (defined($ENV{CVS_SERVER})) {
    Asim::Package::iwarn("Unsetting CVS_SERVER environment variable - it messes up our CVS commands\n");
    delete $ENV{CVS_SERVER};
  }

  #
  # CVS_RSH probably needs to be set
  #
  if (! defined($ENV{CVS_RSH})) {
    Asim::Package::iwarn("Environment variable CVS_RSH not set - this is probably a problem\n");
  }

  #
  # Check CVS version
  #
  my @cvsv = grep(/\(CVS\)\s+\d+\.\d+/, (`cvs -v`));
  $cvsv[0] =~ /(\d+\.\d+)/;
  if ($1 < 1.1) {
    Asim::Package::iwarn("Possible incompatible CVS version looking for >=1.1, found ($1)\n");
  }


}

################################################################

=item $cvs-E<gt>update()

Update this package from the repository

=cut

################################################################

sub update {
  my $self = shift;

  my $location = $self->location();

  my $status = system("cd $location; cvs -f -q update -dP");

  if ($status) {
      return undef;
  }

  return 1;
}

################################################################
#
# Function: status
#
# Check on the CVS status of each file in the current package
#
# Return a array with one element for each file returned 
# by 'cvs status' addin state:
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

  #
  # Execute 'cvs status' to see all the changes that have been made.
  #
  # WARNING: (r2r) cvs status directly pumped into perl via a pipe 
  # has the habit of losing characters and garbling the output; not sure
  # if cvs or perl is the trouble maker, but its a problem so we dump into
  # a temp file instead - sigh.
  #
  my $tmp_cvs_status = "/tmp/asim-shell-cvs-status.$$";

  # Test if the ssh agent is running
  # If it is not, inform the user to enter their passphrase
  # else proceed as usual

  my $tmp_ssh_test = "/tmp/ssh_test.$$";

  system("ssh-add2 -l>& $tmp_ssh_test;");
  if (!CORE::open (SSH_TEST, "< $tmp_ssh_test")) {
    Asim::Package::ierror ("ssh_add2: Can't launch 'ssh-add2'!\n");
    return undef;
  }
  while(<SSH_TEST>) {
    if(/Failed to connect to authentication agent/) {
      print ("\nEnter passphrase if system hangs:\n");
    }
  }
  system("rm -rf $tmp_ssh_test");


  if (system("cd $location; cvs -f status >$tmp_cvs_status 2>&1")) {
    Asim::Package::ierror("status: Can't launch 'cvs status': $!\n");
    return undef;
  }
  if (! CORE::open(CVS,"< $tmp_cvs_status") ) {
    Asim::Package::ierror("status: Can't launch 'cvs status': $!\n");
    return undef;
  }

  #
  # Parse the CVS status
  #

  my $dir;
  my $file;
  my $status;
  my $workrev = "0.0";
  my $workdate = "";
  my $reprev = "0.0";
  my $repfile;
  my $stickytag;
  my $stickydate;
  my $fail = 0;

  my $VERBOSE = 0;

  while ( <CVS> ) {
    print "cvs status returned: $_" if ( $VERBOSE );
    #
    # Hack in case the ssh command asks for a password
    #
    if ( /password:/) {
      print "$_\n";
    }
    #
    # Bobbie seems to always get a CVS status 'abort' :-) So we better take care of it
    #

    # Ensure that abort is not a filename
    # The status format is ? <filename> for unknown files
    # and File: <filename> for versioned ones
    if ( /abort/ && !/\? (\S+)?abort(\S+)?/ && !/File: (\S+)?abort(\S+)?/) {
      Asim::Package::ierror("status: 'cvs status' failed on $location!\nSorry, commiting is not possible\n");
      return undef;
    }
  
    # Get current directory
    if ( /Examining (.*)/ ) {
      $dir = $1;

      print "Checking $dir\n" if ($VERBOSE);
    }

    # Clear all info that we should find anew after =+ separator line
    if ( /^={2,}$/ ) {
      $file = undef;
      $status = undef;
      $repfile = undef;
      $stickytag = undef;
      $stickydate = undef;
    }

    # Get current file name and its status
    if ( /File: (no file\s*)?(.*)\s+Status: (.*)/ ) {
      $file = $2;
      $status = $3;
      $file =~ s/\s+$//;
      if ( $status =~ m/Locally Added/ ) {
        # force some things to be defined
        $workrev = "";
        $workdate = "";
        $reprev = "";
        $repfile = "";
        $stickytag = "";
        $stickydate = "";
      }

      # TODO: Update status to reflect a file with a conflict...
      print "Checking $file\n" if ($VERBOSE);
    }

    # Get Working revision
    if ( /Working revision:\s+([\d\.]+)\s+(.*)/ ) {
      $workrev = $1;
      $workdate = $2;
    }

    # Get Repository revision
    if ( /Repository revision:\s+([\d\.]+)\s+(.*,v)/ ) {
      $reprev = $1;
      $repfile = $2;
    }

    # sticky tag
    if ( /Sticky Tag:\s+(.*)$/ ) {
      $stickytag = $1;
    }

    # sticky date
    if ( /Sticky Date:\s+(.*)$/ ) {
      $stickydate = $1;
    }

    # Check sticky options
    if ( /Sticky Options/ ) {
      # TODO: make sure we are in the right branch
    }

    #
    # All information read when we get to 'Sticky Options'.
    # Start enforcing checks
    #
    if ( /Sticky Options/ ) {
      # heuristic transmission error checker:
      # sometime - dunno why - cvs status output is clobbered; if any
      # of the required fields have not been parsed, we asume this is
      # such a case and return an error;
      if ( ! defined $dir || ! defined $file || ! defined $status ||
           ! defined $repfile || ! defined $stickytag || ! defined $stickydate)
      {
        Asim::Package::ierror("status: 'cvs status' possible transmission error for $location!\nSorry, commiting is not possible - try again\n");
        return undef;
      }
      # heuristic fix for CVS bug "remote repository zombies";
      # we use the following conditions to guess that we have a zombie:
      #   - sticky tag is NOT set, ie. = (none), ie. we're in the trunk
      #   - repository file is in an Attic directory
      #   - status does not indicate that it is a resurrected file
      # Note: we do have to check for sticky tag, since repositry files
      # are ligally residing in an Attic directory when they have been
      # deleted in the trunk, but are alive in a branch, and we must not
      # flag this as an error...
      if ($stickytag eq '(none)' && $repfile =~ m|/Attic/|) {
        if ($status =~ m/Locally Added/) {
          $status = "Resurrected";
        } else {
          $status = "Zombie";
        }
      }

      print "Found: $dir/$file - Status=$status - $reprev $workdate ($repfile)\n" if ( $VERBOSE );
      push(@files, [$dir, $file, $status, $reprev, $workdate]);
      $workdate = "";
      $reprev = "0.0";

    }
  }

  CORE::close(CVS);
  system("rm $tmp_cvs_status");

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

sub cvs_command {
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
    $ret = $self->cvs($command);
    if ($ret == 0) {
      last;
    }

    if ($retry < $NUM_RETRY) {
      printf "Caught error during cvs operation - retrying %d more times\n",
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
# CVS command
#
################################################################


sub cvs {
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
  $command = "(cd $location; cvs -f $command)";
  print "$command\n" if $VERBOSE;

  $ret = system("$command") >> 8;

  return $ret;
}


################################################################
#
# search a repository file for all branch tags
#
################################################################

sub find_branchtags
{
  my $self = shift;
  my $filename = shift;

  my $in_symnames;

  my $location = $self->location();
  my @branches;

  CORE::open(CVSLOG,"(cd $location; cvs -f log $filename) |")
    or die "can't run cvs log $filename\n$!";

  while (<CVSLOG>) {
    if (m/^symbolic names:$/) {
      $in_symnames = 1;
    } elsif ($in_symnames) {
      if (m/^\S/) {
        $in_symnames = 0;
      } elsif (m/\s+(\S+):.*\.0\./) {
        # (only) branch tags have a ".0." component in CVS revision number
        push @branches, $1;
      }
    }
  }
  CORE::close CVSLOG;

  return @branches;
}


################################################################

=item $cvs-E<gt>commit_check()

    Get a status report on the current cvs tree

    Returns 1 if commitable, 0 otherwise

    Side effects: 
         $self->{status} gets one of:
                          Indeterminate-status
                          Ready-to-commit
                          Not-ready-to-commit
                          No-commit-needed

         $self->{must_have_regression} is set

         $self->{regtestdir} is cleared

    Another side effect is that it writes a file with the status
=cut

################################################################

sub commit_check {
  my $self = shift;
  my $reportfile = shift || "/dev/null";

  #
  # Change this to 1 if you want the really strict version of the checks
  #
  my $STRICT_CHECK = 0;

  my  @DANGEROUS_DIRS = (  
                         "base",
                         "feeders",
                         "pm"
                        );



  print "Commit: Checking the status of your source...\n";

  # if we don't run ssh-agent or no key is added, print an additional
  # warning message;
##FIXME 11/12/03 Mark Charney:  disabled for now -- not ssh-agent2 friendly
##   if (Asim::Package::Cvs::is_ssh_agent_running() != 0) {
##     print "Commit: If we're hung here type your password...or better yet run ssh-agent\n";
##   }

  $self->{status} = "No-commit-needed";
  $self->{must_have_regression} = 0;
  $self->{regtestdir} = undef;

  my @files = $self->status();

  if (! defined($files[0])) { 
    Asim::Package::ierror("Commit: Could not determine status of files\n");
    $self->{status} = "Indeterminate-status";
    return 0;
  }
  
  #
  # Create the file that will hold the status report
  #
  if (!CORE::open(TMP,"> $reportfile")) {
    ierror("check: Cannot open $reportfile\n");
    return 0;
  }

  my $dir_printed = "";
  my $fail = 0;

  foreach my $i (@files) {
    my ($dir, $file, $status, $reprev, $workdate) = @{$i};

    #
    # Parse the CVS status while enforcing the following rules:
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

    if ( $status =~ /Needs Patch/ || $status =~ /Needs Checkout/ ||
         $status =~ /Unresolved Conflict/ || $status =~ /Zombie/ ||
         ($STRICT_CHECK == 1 && $status =~ /onflict/) )
    {
      print "Commit: File: $dir/$file has unacceptable state \"$status\"\n";
      if ($status eq "Zombie" || $status eq "Unresolved Conflict") {
        print "Commit:       You are trying to check in changes for this file\n";
        print "Commit:       that somebody else has already deleted!\n";
        print "Commit:       You need to DELETE YOUR FILE MANUALLY.\n";
      }
      $fail = 1;
    }
    #
    # If file is ok, then dump it to report file only if it is NOT already up-to-date
    #
    elsif ( ! ($status =~ /Up-to-date/) ) {
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

=back


=head1 BUGS

Some other repository operations are still spread around the 
rest of the Asim code. For example, the commit command is elsewhere
as is the checkout (although that might make sense).

Need to decide what methods should really be public.

Even though the ssh-agent might be running and contain a key, we
do not check if it is the right key for what we want to do.

Method 'update' should use 'cvs' utility method.


=head1 AUTHORS

Joel Emer, Roger Espasa, Artur Klauser and  Pritpal Ahuja

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


###########################################################################

# kill temporary ssh-agent on exit
END {
  if ($SSH_AGENT_KILL_CMD ne "") {
    `$SSH_AGENT_KILL_CMD`;
  }
}

###########################################################################

1;
