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


package Asim::Regression;
use warnings;
use strict;

use Asim::Base;
use Asim::Package;

use Net::FTP;
use File::Spec;

our @ISA = qw(Asim::Base);

our %a =  ( name        =>          [ "name",
                                      "SCALAR" ],
            description =>          [ "description",
                                      "SCALAR" ],
            location    =>          [ "location",
                                      "SCALAR" ]
      );


our @dirlist = qw( SUBMITTED DONE PENDING ERROR);

#
# Locations of directories
#
our $STATS  = "STATS";


our $TMPDIR;

our $TAR;
our $GZIP;


=head1 NAME

Asim::Regression - Library for manipulating an Asim regression test
on an Asim package. 

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item $regression = Asim::Regression-E<gt>new();


Create a new package object.

=cut

################################################################

sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = _initialize();

  bless	$self, $class;

  #
  # Parse package if given
  #
  if (@_) {
    $self->open($_[0]) || return undef;
    $self->check()     || return undef;
  }

  return $self;
}


sub _initialize {
  my $self = {  accessors => \%a,
		filename  => "",
                location  => "",
             };

  return $self;
}




################################################################

=item $package-E<gt>open($file)

Parse package file file $file and populate the attributes
of the package object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $location = shift;

  my @flist;

  #
  # make sure something seems to be here
  #
  if (! -d "$location/$STATS") {
    return 0;
  }

  #
  # Remember the location of the regression
  #
  $self->{location} = $location;

  #
  # Count number of files in ./ERROR ./DONE ./PENDING ./SUBMITTED
  #
  foreach my $dir ( @dirlist ) {
    @flist = glob "$location/$dir/*";
    $self->{"$dir"} = $#flist + 1;
  }


  #
  # Let's grep for the word IPC in the STATS directory
  #
  my %stats;
  my $file;
  my $metric;
  my $value;

  print "Searching for relevent metrics...\n";

  CORE::open(STATS,"(cd $location/$STATS; zgrep Overall_ *.stats* /dev/null) |") 
    || ierror("can't fork grep: $!\n") && return ();

  my $overall_metrics = "IPC|OPC";

  while ( <STATS> ) {
    if ( /Overall_($overall_metrics)/ ) {
      ( $file, $metric, $value ) = /(.*).stats.*:.*Overall_($overall_metrics):\s+(\d+\.\d+)/;
      $stats{"$file"}{$metric} = $value;
    } 

  }

  CORE::close(STATS);
  
  $self->{stats} = \%stats;


  return $self;
}

sub check {
  my $self = shift;

  #
  # Get $TMPDIR
  #
  
  $TMPDIR = Asim::get_tmpdir();

  if ( ! defined($TMPDIR)) {
    ierror("Sorry \$TMPDIR is not set and /tmp does not exist");
    return 0;
  }
  
  # TODO: We shouldn't just rely on these being in the path

  $TAR = "tar";
  $GZIP = "gzip";

  return 1;
}


sub pending {
  my $self = shift;

  return $self->{PENDING};
}

sub error {
  my $self = shift;

  return $self->{ERROR};
}

sub done {
  my $self = shift;

  return $self->{DONE};
}

sub submitted {
  my $self = shift;

  return $self->{SUBMITTED};
}

#
#

sub statfiles {
  my $self = shift;

  my $REGRESSIONDIR = $self->location();
  my @statfiles = glob("$REGRESSIONDIR/$STATS/*.stats*");

  return (@statfiles);
}



sub stats {
  my $self = shift;

  return $self->{stats};
}


sub report  {
  my $self = shift;
  my $FILE = shift;
  my $ostats_ref = shift;
  my $metric = shift;
  my $long = shift || 0;


  print $FILE "\n\nRegression Summary: ";

  #
  # Print number of files in ./ERROR ./DONE ./PENDING ./SUBMITTED
  #
  foreach my $dir ( @dirlist ) {
    print $FILE "$dir: " . $self->{"$dir"} . "  ";
  }

  #
  # Create a nice report of the evolution of IPC (and any other stats that you might
  # want to collect) and add it to the $FILE file we have been creating all along.
  # We assume that there are a set of predefined fields that we always collect. These
  # are defined in @statsfields
  #
  my %ostats = %{$ostats_ref};
  my %nstats = %{$self->{stats}};

  my @jointbench;

  my @statsfields = ( $metric );

  #
  # Because 'stats' and 'ostats' might contain different benchmarks, we have to create
  # a unique list of benchmarks to be printed out in the table
  #
  @jointbench = keys %ostats;
  foreach my $bench ( keys %nstats ) {
    if ( ! grep(/$bench/, @jointbench) ) {
      print "Matched $bench\n";
      push @jointbench, $bench;
    }
  }
  #
  # Dump information (in either short of long format)
  #
  printf $FILE "\n\n$metric Variation Table:\n";
  printf $FILE "%-35s ", "benchmark";
  print  $FILE "|   Current   | Your Commit | % Variation |\n";


  foreach my $bench ( sort @jointbench ) {
    printf $FILE "%-35s |", $bench if ! $long;
    printf $FILE "%-35s |%13s|%13s|%13s|\n", $bench, " ", " ", " " if $long;

    my $old;
    my $new;
    my $variation;

    foreach my $field ( @statsfields ) {
      my $oldnum = $ostats{$bench}{$field} || 0;
      my $newnum = $nstats{$bench}{$field} || 0;

      if ( $oldnum > 0 && $newnum > 0 ) {
        $variation = (100.0 * ($newnum - $oldnum)) / $oldnum;
        $variation = sprintf "%4.1f", $variation;
      }
      elsif ( $oldnum > 0 && $newnum == 0 ) {
        $variation = "ERROR";
      }
      elsif ( $oldnum == 0 && $newnum > 0 ) {
        $variation = "NEW ENTRY";
      }
      else {
        $variation = "    ";
      }
      $old =  $oldnum > 0 ? sprintf "%6.3f", $oldnum : "    ";
      $new =  $newnum > 0 ? sprintf "%6.3f", $newnum : "    ";
      
      printf $FILE "   %10s%22s |", $field if ($long);

      printf $FILE "    %6s   |    %6s   |   %6s    |\n", 
             $old, $new, $variation if ( $long || ($field eq $metric) );


    }
  }

}


sub tar_and_save {
  my $self = shift;
  my $tarfile = shift;

  my $location = $self->location();

  my $local_tarfile;

  my $ok;

  #
  # Now, tar the whole directory tree starting at the regtest point the user
  # has given us, gzip it and store it away
  #
  print "\n";
  print "Taring and compressing your regtest directory...\n";

  $local_tarfile = "$TMPDIR/$tarfile";
    
  $ok = system("(cd $location; $TAR cvf - . 2>/dev/null) | $GZIP > $local_tarfile");
  if ( $ok != 0 ) {
    ierror("Unexpected error when taring the regtest directory: $!\n");
    return 0;
  }

  #
  # we dump the tar ball into an ftp incoming directory;
  # some cron jobs will pick them up later and move them to their
  # final resting place in $tarfile
  #
  my $ftp_server = "vssad.shr.intel.com"; 
  my $ftp_user = "anonymous";
  my $ftp_pass = "";
  my $ftp_dir = "incoming/asim-regtestlog";

  eval { ftp_put($local_tarfile,
                 $ftp_server,$ftp_user,$ftp_pass,$ftp_dir,$tarfile) };

  unlink $local_tarfile;

  if ($@) {
    ierror("Unexpected error when FTPing tarfile to".
         " ftp://$ftp_server/$ftp_dir/$tarfile\n$@");

    return 0;
  }


  print "...Done.\n";
  print "\n";

  $self->{has_tarfile} = 1;

  return 1;
}


##############################################################################
# ftp_put
#
# Input parameters:
#   local_filename
#   ftp_server
#   ftp_user
#   ftp_password
#   ftp_dir
#   remote_filename
#
# Results:
#   Execution exits when a problem is encountered.
##############################################################################
sub ftp_put {
  my $local_filename = shift;
  my $ftp_server = shift;
  my $ftp_user = shift;
  my $ftp_password = shift;
  my $ftp_dir = shift;
  my $remote_filename = shift;

  #
  # set defaults
  #
  if (($ftp_user eq "anonymous" || $ftp_user eq "ftp") &&
      (! defined $ftp_password || $ftp_password eq ''))
  {
    my $hostname = `/usr/bin/hostname`;
    chomp $hostname;
    $ftp_password = "$ENV{USER}\@$hostname";
  }

  my $ftp = Net::FTP->new($ftp_server, Debug => 0);
  die "ftp open failed"  if (! defined $ftp);
  $ftp->login($ftp_user, $ftp_password) or die "ftp login failed\n$!";
  $ftp->cwd($ftp_dir) or die "ftp cwd failed\n$!";
  $ftp->binary() or die "ftp type binary failed\n$!";
  # we initially put things in a temporary filename (dash prefix)
  # to make sure that scripts on the other end can ignore files that
  # are not completely sent yet; after the put completes, we rename the
  # file remotely to the correct name
  $ftp->put($local_filename, "-$remote_filename") or die "ftp put failed\n$!";
  $ftp->rename("-$remote_filename","$remote_filename") or die "ftp rename failed\n$!";
  $ftp->quit;
}



# sub run {
# chomp ($REGTESTBIN = `$RESOLVER tools/regtest.pl`);
# if ($? != 0) {
#   die "ERROR:\n$REGTESTBIN\n";
# }
# ###@REGTESTRUN = ( "-level R -rparams 1" );
# @REGTESTRUN = ( );
#
# ##print "2. Running regression test to check AINT and ATF...\n";
#
# foreach my $run ( @REGTESTRUN ) {
#  $ok = system("$REGTESTBIN $run");
#  if ( $ok != 0 ) {
#   cleanup_and_exit("Regression test failed! Sorry, commiting is not possible\n");
#  }
# }
#
#}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Regression: Error - $message";

  return 1;
}

1;
