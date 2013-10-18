# Copyright (C) 2004-2008 Intel Corporation
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
# *****************************************************************************
# *
# * @brief Batch.pm : Parent class of batch job manipulation modules
# *
# * @author Joel Emer
# *
# *****************************************************************************
#


package Asim::Batch;
use warnings;
use strict;

use Asim::Batch::Base;
use Asim::Batch::Local;
use Asim::Batch::Condor;
use Asim::Batch::Netbatch;
use Asim::Batch::NetbatchRemote;
use Asim::Batch::NetbatchFeeder;
use Asim::Batch::List;

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


=head1 NAME

Asim::Batch - Library for manipulating batch jobs

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate batch jobs, basically
it is just a wrapper around the batch control modules for various
batch systems.

=cut

################################################################

=head1 VARIABLES

The following global parameters control the behavior of this class:

=over 4

=item $Asim::Batch::ENABLE_NETBATCH_PRE

In Netbatch feeders, enable a pre-execution script to be run prior to the actual job,
to verify minimum disk space requirements on the compute resource.

This feature may not be available on older versions of Netbatch.

=back

=cut

our $ENABLE_NETBATCH_PRE = 1;

=over 4

=item $Asim::Batch::NETBATCH_PRE_USE_SMARTCLASS

In Netbatch feeders, enable the use of a "smart class" argument in the job submission command
to automatically check for minimum disk space requirements before assigning a job
to a compute resource.

This feature may not be available on older versions of Netbatch.
If available and enabled, this feature supersedes ENABLE_NETBATCH_PRE.

=back

=cut

our $NETBATCH_PRE_USE_SMARTCLASS = 1;

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $batch = Asim::Batch-E<gt>new(type,
                                    queue  => <queuename>,
                                    class => <batch class>,
                                   );


Create a new batch object object, where "type" is one of the following:

=over 4

=cut

################################################################

sub new {
  my $this = shift;
  my $type = shift;
  
=item 'LOCAL'

Run jobs on the local machine

=cut

  if ( $type eq "LOCAL") {
    return Asim::Batch::Local->new(@_);
  
=item 'NETBATCH'

Run jobs using raw Netbatch commands

=cut

  } elsif ( $type eq "NETBATCH") {
    return Asim::Batch::Netbatch->new(@_);
  
=item 'CONDOR'

Run jobs using Condor

=cut

  } elsif ( $type eq "CONDOR") {
    return Asim::Batch::Condor->new(@_);
  
=item 'NETBATCH'

Run jobs on a remote pool using raw Netbatch commands

=cut

  } elsif ( $type eq "NETBATCH_REMOTE") {
    return Asim::Batch::NetbatchRemote->new(@_);
  
=item 'HOST_LIST'

Run jobs on on a list of machines, using SSH commands

=cut

  } elsif ( $type eq "HOST_LIST") {
    return Asim::Batch::List->new(@_);
  
=item 'NBFEEDER'

Run jobs using the Netbatch Feeder

=cut

  } elsif ( $type eq "NBFEEDER") {
    return Asim::Batch::NetbatchFeeder->new(@_);
  } else {
    return undef;
  }
}

=back

=cut


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Batch: Error - $message";

  return 1;
}


=back

=head1 BUGS

Unknown

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2008

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
