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
use Asim::Batch::Netbatch;
use Asim::Batch::NetbatchRemote;

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


=head1 METHODS

The following public methods are supported:

=over 4

=item $batch = Asim::Batch-E<gt>new(type,
                                    queue  => <queuename>,
                                    class => <batch class>,
                                   );


Create a new batch object object.

=cut

################################################################

sub new {
  my $this = shift;
  my $type = shift;

  if ( $type eq "LOCAL") {
    return Asim::Batch::Local->new(@_);
  } elsif ( $type eq "NETBATCH") {
    return Asim::Batch::Netbatch->new(@_);
  } elsif ( $type eq "NETBATCH_REMOTE") {
    return Asim::Batch::NetbatchRemote->new(@_);
  } else {
    return undef;
  }
}


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
