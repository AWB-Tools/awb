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

#@ISA=(Asim::Base);

use File::Basename;


our $IPCHIST;


# Documentation of the public functions is in Package.pm

sub stats {
  my $self = shift;

  return $self->{stats};
}

sub stats_metric {
  return $_[0]->_accessor("Commit","Metric",$_[1])
    || "IPC";
}

sub read_stats {
  my $self = shift;

  my %stats = ();

  # TODO: Get via package interface
  my $location = $self->location();
  my $ometricfile = "$location/$IPCHIST";
  my $metric = $self->stats_metric();

  #
  # Get the last commit metric info.
  #
  print "Retrieving $metric history from $ometricfile...\n";

  # Remove ipchist file and this line for deliberate BUG
  $self->{stats} = \%stats;

  #
  # Check for 'ipchist' file
  #
  if (! -e $ometricfile) {
    ierror("No $ometricfile file\n") && return 0
  }

  #
  # Make sure 'ipchist' is writeable
  #
  chmod 0644, "$ometricfile";

  #
  # First, find out which was the last CSN that provided metrics
  #
  my $high_sn;

  CORE::open(OMETRICS,"grep CSN $ometricfile | ") 
    || ierror("Can't grep on $ometricfile: $!\n") && return 0;

  while (<OMETRICS>) {
    ( $high_sn ) = /== CSN[^\d]*(\d+)/;
  }

  CORE::close(OMETRICS);

  if (! defined($high_sn)) {
    print "No old $metric information available\n";
    $self->{stats} = \%stats;
    return 1;
  }

  print "Last $metric report was recorded at CSN: $high_sn\n";

  #
  # Reopen the file and scan until we get to the desired Commit Serial Number. 
  #
  CORE::open(OMETRICS,"$ometricfile") 
    || ierror("can't open $metric history file $ometricfile: $!\n") && return 0;

  while ( <OMETRICS> ) {
    next if ( ! /== CSN[^\d]*$high_sn/ );
    last;
  }

  #
  # Now get the interesting data points for each benchmark. 
  # Note: if you want to add more stuff here (i.e., to collect PboxIPC for example),
  #       just add extra 'matching' lines and store all the information in the 
  #       'stats' table
  #
  my $bench;
  my $value;

  while ( <OMETRICS> ) {
    ($bench) = /^((A|B|M|CMP|MICRO)-[^ ]*)/; 
    ($value)   = /\s$metric\s+(\d+\.\d+)/;

    if (defined($bench) && defined($value)) {
      #print "Adding $bench/$metric = $value\n";
      $stats{"$bench"}{$metric} = $value;
    }
  }

  CORE::close(OMETRICS);

  $self->{stats} = \%stats;
  
  return 1;
}



sub write_stats {
  my $self = shift;

  my $location = $self->location();
  my $ometricfile = "$location/$IPCHIST";
  my $metric = $self->stats_metric();

  my $csn = $self->csn();
  my %stats = %{$self->stats()};


  my @statsfields = ($metric);
  my $data;

  CORE::open(OMETRICS,">> $ometricfile") 
    || ierror("Regest: Unable to update $metric history file $ometricfile: $!\n") && return 0;

  print OMETRICS "== CSN $csn\n";

  foreach my $bench ( sort keys %stats ) {
    next if ( ! ($bench =~ /^(A|B|M|CMP|MICRO)-/) );
    next unless ( $stats{"$bench"}{$metric} > 0 );

    printf OMETRICS "%-40s ", $bench;

    foreach my $field ( @statsfields ) {
      $data = sprintf "%.3f", $stats{"$bench"}{"$field"};
      print OMETRICS "$field $data ";
    }
    print OMETRICS "\n";
  }

  CORE::close(OMETRICS);

  return 1;
}


1;
