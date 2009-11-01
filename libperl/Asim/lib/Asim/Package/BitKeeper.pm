#
# *****************************************************************************
# *
# * @brief BitKeeper.pm : utility methods to handle BK repositories
# *
# * @author Oscar Rosell
# *
# Copyright (C) 2005-2006 Intel Corporation
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

package Asim::Package::BitKeeper;
use warnings;
use strict;

use File::Basename;
use File::Temp qw(tempfile tempdir);

our @ISA = qw(Asim::Package);

our $SSH_AGENT_KILL_CMD = "";



=head1 NAME

Asim::Package::BitKeeper - Class to manipulate a checked out BitKeeper repository.

=head1 SYNOPSIS

use Asim::Package::BitKeeper;

Asim::Package::BitKeeper::init();

my $bk = Asim::Package::BitKeeper->new("~/bkdir");

$bk->update();


=head1 DESCRIPTION

This is a clase to allow a directory to be treated as 
a BitKeeper repository. 

This is a subclass of Asim::Package.  After creating an instance of Asim::Package,
you can call the set_type() method here to check whether the package is in a BK
repository and set it to this subclass if so.  Set_type will return 1 if it is a
BK package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $cvs = Asim::Package::BitKeeper::set_type( $package )

If $package is an BK repository, set the object's subclass to Asim::Package::BitKeeper and return 1.
Otherwise return 0.

=cut

################################################################

sub set_type {
  my $self = shift;
  my $location = $self->location();
  # if there is an "SCCS" subdirectory, then this must be BitKeeper
  if (-e "$location/SCCS") {
    bless $self;
    return 1;
  }
  return 0;
}

################################################################

=item $package-E<gt>type()

Return type of package - "bitkeeper" in this case.

=cut

################################################################

sub type {
  return 'bitkeeper';
}

################################################################

=item $bk = Asim::Package::BitKeeper::init()

Global init of BitKeeper module.

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
}

################################################################

=item $bk-E<gt>update()

Update this package from the repository

=cut

################################################################

sub update {
  my $self = shift;

  my $location = $self->location();
  
  # we cannot do a bk pull unless we have all our local changes checked in locally
  my $local_changes = `cd $location; bk sfiles -vc`;
  if ( $local_changes ne '' ) {
      Asim::Package::ierror( "Cannot pull from BK repository until the following changes are checked in locally:\n" .
                             "\n" . $local_changes . "\n" .
                             "Please execute \"bk citool\" to check your changes in locally first!\n" );
      return undef;
  }

  my   $retcode  = system "cd $location; bk pull";
  if ( $retcode != 0 ) {
      Asim::Package::ierror( "BitKeeper \"pull\" command returned error code: $retcode\n" );
      return undef;
  }

  return 1;
}

################################################################

=item $bk-E<gt>pull()

Update this package from the repository

=cut

################################################################

sub pull {
  my $self = shift;

  return $self->update();
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
  my @files = ();
  my $location = $self->location();

  # invoke BK to get status of all files, from the top of the repository:
  open BKSTATUS, "cd $location; bk sfiles -E |"
    || ( Asim::Package::ierror("BitKeeper::status could not open bk status output!\n")
       && return ()
       );
  
  # parse output.  We should get one line per file.
  while ( <BKSTATUS> ) {
    if ( $_ =~ m/^(.)(.)(.)(.)\s+(.*)$/ ) {
      my $locked   = ( $1 eq 'l' ) ? 1 : 0;
      my $unlocked = ( $1 eq 'u' ) ? 1 : 0;
      my $junk     = ( $1 eq 'j' ) ? 1 : 0;
      my $extra    = ( $1 eq 'x' ) ? 1 : 0;
      my $changed  = ( $2 eq 'c' ) ? 1 : 0;
      my $pending  = ( $3 eq 'p' ) ? 1 : 0;
      chomp( my $dirfile = $5 );
      my $dir      = '';
      my $file     = '';
      my $reprev   = '0.0';
      my $workdate = '';
      my $status   = 'Up-to-date';
      
      # decode status bits in CVS-like status string:
      if ( $changed && $pending ) {
        $status = 'Needs Merge';
      } elsif ( $changed ) {
        $status = 'Locally Modified';
      } elsif ( $pending ) {
        if ( $dirfile =~ m/^BitKeeper\/deleted/ ) {
          $status = 'Locally Removed';
        } else {
          $status = 'Locally Added';
        }
      } elsif ( $extra ) {
        $status = 'Unknown';
      } elsif ( $junk ) {
        # I don't know about this !!?!
        $status = 'Zombie';
      }
      
      # parse directory and file, removing "/SCCS/s." from the path:
      if      ( $dirfile =~ m/^(.*)SCCS\/s\.(.*)$/ ) {
	$file = $2;
        ($dir = $1) =~ s/\/$//;
        # locally removed files are a special case:
        if ( $dirfile =~ m/BitKeeper\/deleted\/SCCS\/s\.\.del\-(.*)\~[0-9a-f]+/ ) {
          $file = $1;
          $dir = '?';
        }
      } elsif ( $dirfile =~ m/^(.*)\/(.*)$/ ) {
        # not a repository file, in a subdirectory
	$file = $2;
        ($dir = $1) =~ s/\/$//;
      } elsif ( $dirfile =~ m/^([^\/]*)$/ ) {
        # not a repository file, no subdirectory
	$file = $1;
      }
      # force directory to "." if it's empty:
      if ( ! $dir ) { $dir = '.'; }

      # if we managed to parse status for this line, add it to the output list:
      push(@files, [$dir, $file, $status, $reprev, $workdate]);
    }
  }
  close BKSTATUS;

  # return the parsed list of files:
  return @files;
}

############################################################
#
# $package->files_status_unknown()
#
# Return a list of files in this package with status "unknown",
# i.e. those files that are not in the repository.  Ignore any files
# in .cvsignore (or equivalent) lists, i.e. do not include them as
# "unknown" files.
#
# This is a BK-specific implementation based on a call to "bk sfiles"
# instead of using the status() which might be slow since it returns
# a large list of all repository files.
#
############################################################

sub files_status_unknown {
  my $self = shift;
  my @unknown_files = ();
  my $location = $self->location();

  # invoke "bk sfiles" to get status of all files, from the top of the repository:
  open BKUNKNOWN, "cd $location; bk sfiles -x |"
    || ( Asim::Package::ierror("BitKeeper::files_status_unknown could not open bk sfiles output!\n")
       && return ()
       );
  while ( <BKUNKNOWN> ) {
    chomp;
    push @unknown_files, $_;
  }
  close BKUNKNOWN;

  return @unknown_files;
}


################################################################
#
# Persistent BK command
#
#   In case a remote BK command times out on us,
#   we keep retrying a number of times before we give up too
#
################################################################

sub bk_command {
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
    $ret = $self->bk($command);
    if ($ret == 0) {
      last;
    }

    if ($retry < $NUM_RETRY) {
      printf "Caught error during bk operation - retrying %d more times\n",
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
# raw BK command
#
################################################################


sub bk {
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
  $command = "(cd $location; bk $command)";
  print "$command\n" if $VERBOSE;

  $ret = system("$command") >> 8;

  return $ret;
}

################################################################

=item $bk-E<gt>commit_check()

    Get a status report on the current cvs tree

    Returns 1 if commitable, 0 otherwise

    Side effects: 
         $bk->{status} gets one of:
                          Indeterminate-status
                          Ready-to-commit
                          Not-ready-to-commit
                          No-commit-needed

         $bk->{must_have_regression} is set

         $bk->{regtestdir} is cleared

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
  my $fail = 0;

  print "Commit: Checking the status of your source...\n";

  # if we don't run ssh-agent or no key is added, print an additional
  # warning message;
##FIXME 11/12/03 Mark Charney:  disabled for now -- not ssh-agent2 friendly
##   if (Asim::Cvs::is_ssh_agent_running() != 0) {
##     print "Commit: If we're hung here type your password...or better yet run ssh-agent\n";
##   }

  $self->{status} = "No-commit-needed";
  $self->{must_have_regression} = 0;
  $self->{regtestdir} = undef;

  print "Checking for updates needed...\n";

  $self->bk_command("changes -R > ./.tmp");
  my @r_changes =  `cat ./.tmp`;

  if (defined($r_changes[0])) { 
    $fail = 1;
  }
  else {
    #
    # Create the file that will hold the status report
    #
    if (!CORE::open(TMP,"> $reportfile")) {
      Asim::Package::ierror("check: Cannot open $reportfile\n");
      return 0;
    }

    print "Checking for commit needed...\n";
    my $dir_printed = "";
    $self->bk_command("changes -L > ./.tmp");
    my @l_changes =  `cat ./.tmp`;

    if (defined ($l_changes[0]))
    {
      $self->{status} = "Commit-needed";
    }
    CORE::close(TMP);
  }

  unlink("./.tmp");

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

=item $package-E<gt>has_local_changesets()

Return 1 if and only if the local repository has changesets
that are not in the parent repository

=cut

sub has_local_changesets
{
  my $self = shift;
  (my $fh, my $fname) = tempfile();
  system "bk changes -L >& $fname";
  open STATUS, $fname
    || ( Asim::Package::ierror("BitKeeper::has_local_changesets() could not open bk changes -L output!\n")
       && return ()
       );
  my $result = 0;
  while ( <STATUS> ) {
    if ( /ChangeSet/ ) {
      $result = 1;
      last;
    }
  }
  close STATUS;
  close $fh;
  return $result;
}

=item $package-E<gt>get_repository_location()

Return the location of the local repository as a string
of the form that can be used in a baseline tag.

=cut

sub get_repository_location
{
  my $self = shift;
  chomp(my $host = `hostname --long`);
  return $host . ':' . $self->location;
}

=item $package-E<gt>get_current_revision_or_tag()

Return the curren revision number of the repository,
or a symbolic tag if the current revision has been tagged.
This string can be used in a baseline tag.

=cut

sub get_current_revision_or_tag
{
  my $self = shift;
  my $location = $self->location();

  # look for the first ChangeSet mentioned in the bk log output, to get revision number:
  open STATUS, "cd $location; bk log |"
    || ( Asim::Package::ierror("BitKeeper::get_current_revision_or_tag() could not open bk log output!\n")
       && return ()
       );
  my $result;
  my $tag;
  while ( <STATUS> ) {
    if ( /ChangeSet\s+([0-9\.]+)/ ) {
      if (defined $result) {
        last;
      } else {
        $result = $1;
      }
    } elsif ( /TAG:\s*(.*)$/ && defined $result ) {
      $tag = $1;
      last;
    }
  }
  close STATUS;

  # if we couldn't extract a revision number, default to using CSN.
  # if we couldn't find a tag, just the revision number:
  return $self->csn() unless defined $result;
  return $result unless defined $tag;
  return $tag;
}

=item $package-E<gt>baseline_tag([<use_csn>])

Return a tag that can be used later
to retrieve exactly the version of the package that is currently
checked out in the working copy (minus any uncommitted changes).

In Asim BK repositories (unlike in CVS) we do not maintain the branch
name and revision numbers as part of the CSN in the admin/packages file.
Instead, we extract the branch or tag name and revision from the local
repository as follows.

If there are no changesets in the local repository that are not in
the parent repository, then we are assumed to be "on the head",
and we return the revision number of the repository, or the tag
if the current revision is tagged.

If there are changesets in the local repository that are not in the parent,
then the local repository is considered a "branch" and we return the
URL of the local repo, plus a local revision number or tag.

In either case, this is enough information to clone a copy of the
current local repository (minus any changes that have not been
locally checked in).

If the optional <use_csn> flag is passed in as '1', then use the
CSN information from the admin/packages file instead of querying
BK directly to get the working copy revision information.  But this CSN
number is probably not accurate, because commits to BK repositories
are not done via asim-shell, so this option (which is here for
compatibility with SVN) should probably not be used.

=cut

sub baseline_tag
{
  my $self = shift;
  my $use_csn = shift;
  if ( $use_csn ) {
    return $self->csn();
  }
  if ( $self->has_local_changesets() ) {
    return $self->get_repository_location()
      .':'.$self->get_current_revision_or_tag();
  } else {
    return $self->get_current_revision_or_tag();
  }
}

=back

=head1 BUGS

=head1 AUTHORS

Oscar Rosell, Carl Beckmann based on Cvs.pm by
Joel Emer, Roger Espasa, Artur Klauser and  Pritpal Ahuja

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2005-09

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
