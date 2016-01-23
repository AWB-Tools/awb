#
# *****************************************************************************
# *
# * @brief P4.pm : utility methods to handle P4 repositories
# *
# * @author Joel Emer
# *
# Copyright (C) 2016 Nvidia Corporation
# Copyright (c) 2009 Intel Corporation
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

package Asim::Package::P4;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Package);


=head1 NAME

Asim::Package::P4 - Class to manipulate a checked out P4 repository.

=head1 SYNOPSIS

use Asim::Package::P4;

Asim::Package::P4::init();

my $p4 = Asim::Package::P4->new("~/p4dir");

$p4->update();


=head1 DESCRIPTION

This is a class to allow a directory to be treated as a P4 repository. 

This is a subclass of Asim::Package.  After creating an instance of Asim::Package,
you can call the set_type() method here to check whether the package is in a P4
repository and set it to this subclass if so.  Set_type will return 1 if it is a
P4 package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

=item $pkg = Asim::Package::P4::set_type( $package )

If $package is an P4 repository, set the object's subclass to Asim::Package::P4 and return 1.
Otherwise return 0.

=cut

sub set_type {
  my $self = shift;
  my $location = $self->location();

  # If there is a "P4CONFIG" file, then this is a P4 package:
  my $p4configfile = $ENV{P4CONFIG} || "Not*Defined";

  if (-e "$location/$p4configfile") {
    bless $self;
    return 1;
  }

  return 0;
}

=item $package-E<gt>type()

Return type of package - "p4" in this case.

=cut

sub type {
  return 'p4';
}

=item $p4 = Asim::Package::P4:init()

Global init of P4 module.

=cut

sub init {
  # Deal with any P4 environment variables
  1;
}

=item $p4-E<gt>update([version])

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
    Asim::Package::ierror("update: P4 can't update to a specified version\n");
    return undef;
} 

  # do the update
  
  my $status = system("cd $location; p4 $command $args");

  if ($status) {
      return undef;
  }

  return 1;
}

=item $p4-E<gt>pull([version])

#Alias to update

=cut

sub pull {

  my $self = shift;
  my $revision = shift;

  return $self->update($revision);

}

=item $p4-E<gt>commit_check()

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
    # if no files came up in p4 status then the workspace is upto date
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

=item $p4-E<gt>status()

Check on the P4 status of each file in the current package

Return a array with one element for each file returned 
by 'p4 status' addin state:

Each array element contains:

 ($directory, $filename, $status, $reprev, $date)

=cut

sub status {
  my $self = shift;
  my $location = $self->location();
  my @files = ();

  # Currently unimplmeneted
  Asim::Package::ierror("status: P4 status unimplemented - assuming success\n");
  return ();

  # Execute 'p4 status' to see all the changes that have been made.
 
  my $tmp_p4_status = "/tmp/awb-shell-p4-status.$$";

  system("cd $location; p4 status >$tmp_p4_status 2>&1");
 
  if (! CORE::open(P4,"< $tmp_p4_status") ) {
    Asim::Package::ierror("status: Can't launch 'p4 status 1': $!\n");
    return undef;
  }

  # Parse the P4 status

  my $fail = 0;

  my $VERBOSE = 0;

  while (<P4>) {
    my $file = "";
    my $status = "";

    print "p4 status returned: $_" if ( $VERBOSE );
    #
    # Hack in case the remotes/origin/master ssh command asks for a password
    #
    if ( /password:/) {
      print "$_\n";
    }
    
    if ( /^#*\s+(.+):\s+(.+)$/ ) {
      $status = $1;
      $file = $2;
    }
    
    if ($status and $file) {
       my ($filnam, $dir) = fileparse($file);
       chop $dir if ($dir =~ m/\/$/); # remove trailing "/" from directory
       push(@files, [$dir, $filnam, $status, '', '']); # no rev# or date information from P4
    }
  }
  CORE::close(P4);
  system("rm $tmp_p4_status");
  
  return (@files);
}

################################################################

=item $package-E<gt>destroy()

Destroy (removing all files) for the package

=cut

################################################################

sub destroy {
  my $self = shift;
  my $location = $self->location
    || ierror("Corrupt package - no location\n") && return 0;

  # P4 needs to delete the client
  
  my $p4client = $self->p4_client();

  print "Destroy the p4 client for the package: $p4client\n";

  $self->p4("client -d $p4client");

  # Then we just call our parent to do the rest

  return Asim::Package::destroy($self);
}

=item $p4->p4_client()

Return the p4 clientname for this package.
    This is p4-only functionality.

=cut

sub p4_client {
  my $self = shift;
  #
  # Make sure we are at the root of the checked out files
  #
  my $location = $self->location; 

  my $p4config = $ENV{"P4CONFIG"} || "Not*Defined";

  # Search the P4CONFIG file for the line that looks like P4CLIENT=<clientname>
  # and extract out <clientname>

  open P4CLIENT, "<$location/$p4config";
  while (my $input = <P4CLIENT>) {
      chomp $input;

      if ($input =~ /^P4CLIENT=/) {
	  $input =~ s/P4CLIENT=//;
	  close P4CLIENT;
	  return $input;
      }
  }

  close P4CLIENT;

  return undef;
}

=item $p4->p4( <command> )

Issue an P4 command directly.  Don't retry if it fails.

=cut

sub p4 {
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
  $command = "(cd $location; p4 $command)";
  print "$command\n" if $VERBOSE;

  $ret = system("$command") >> 8;

  return $ret;
}

=back

=head1 BUGS

=head1 AUTHORS

    Joel Emer, Mohit Gambhir

=head1 COPYRIGHT

Copyright (c) Nvidia Corporation, 2016 
Copyright (c) Intel Corporation, 2009


=cut

1;
