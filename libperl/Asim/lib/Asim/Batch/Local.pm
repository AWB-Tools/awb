#
# Copyright (C) 2003-2008 Intel Corporation
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
# * @brief Batch.pm : Module for controlling batch jobs run locally
# *
# * @author Joel Emer
# *
# *****************************************************************************
#


package Asim::Batch::Local;
use warnings;
use strict;

our @ISA = qw(Asim::Batch::Base);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


=head1 NAME

Asim::Batch::Local - Library for manipulating locally run batch jobs

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item $batch = Asim::Batch-E<gt>new(type,
                                   );


Create a new batch object object.


=cut

################################################################

sub new {
  my $this = shift;

  my $class = ref($this) || $this;
  my $self;

  $self = {@_};
  bless	$self, $class;

  return $self;
}


################################################################

=item $repository-E<gt>submit($command)

Submit a new batch command.

=cut

################################################################

sub submit {
  my $self       = shift;
  my $command    = shift;
  my $batch_log  = shift;
  my $submit_log = shift;
  my $block      = shift || 0;

  system("echo Running locally: $command >$submit_log");
  system("$command >$batch_log 2>&1");

  return 1;
}


################################################################

=item $batch-E<gt>get_queue()

Get the queue name

=cut

################################################################

sub get_queue {
  my $self = shift;

  return "N/A";
}

################################################################

=item $batch-E<gt>get_pool()

Get the pool name

=cut

################################################################

sub get_pool {
  my $self = shift;

  return "N/A";
}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Asim::Betch::Netbatch: Error - $message";

  return 1;
}


=back

=head1 BUGS

TBD

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2008

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
