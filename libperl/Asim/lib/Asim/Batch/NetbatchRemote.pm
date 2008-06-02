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
# *****************************************************************************
# *
# * @brief Batch.pm : Module for controlling batch jobs run on netbatch remotley
# *
# * @author Joel Emer
# *
# *****************************************************************************
#


package Asim::Batch::NetbatchRemote;
use warnings;
use strict;

our @ISA = qw(Asim::Batch::Netbatch Asim::Batch::Base);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


my $BATCH_STATUS_COMMAND = "nbqstat";
my $BATCH_SUBMIT_COMMAND = "nbq";

=head1 NAME

Asim::Batch::Netbatch - Library for manipulating netbatch batch jobs

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
                                    queue  => <queuename>,
                                    class => <batch class>,
                                   );


Create a new batch object object.


=cut

################################################################

sub new {
  my $this = shift;
  my $args = {@_};

  my $class = ref($this) || $this;
  my $self;

  $self = {};

  $self->_set_tempdir($args->{temp});

  $self->_set_default_queue($args->{queue});
  $self->_set_default_pool($args->{pool});
  $self->{class} = $args->{class} || "TBD";

  $self->_set_flags($args->{flags});

  $self->{quiet} = $args->{quiet} || 0;

  $self->{env} = $args->{env} || {};

  bless	$self, $class;

  return $self;
}

################################################################

=item $repository-E<gt>check_pool()

Check that the requested pool exits.

=cut

################################################################

sub check_pool {
  my $self = shift;

  return 1;
}

################################################################

=item $batch-E<gt>check_space($dir, $size)

Add a check that $dir is has at least $size MB available for
the run. Try to use the Netbatch class scheduler if available
then use a asim specific pre-execution batch script.

Setting a $size of 0 cancels the check.

=cut

################################################################


sub check_space {
  my $self = shift;
  my $dir = shift;
  my $size = shift;

  return 1;
}


################################################################

=item $batch-E<gt>checks_threads($threads)

Add a check that the batch job is run on a processor with at
least $threads threads.

Setting threads to 1 (or 0) cancels the check.

=cut

################################################################

sub check_threads {
  my $self = shift;
  my $threads = shift;

  return 1;
}

################################################################

=item $batch-E<gt>scheduler();

Local scheduler to wait until its appropriate to submit a job

=cut

################################################################

sub scheduler {
  my $self = shift;

  return 1;
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

  my $flags_queue   = $self->{flags_queue};
  my $flags_space   = $self->{flags_space};
  my $flags_threads = $self->{flags_threads};
  my $flags_extra   = $self->{flags_extra};

  my $flags = "$flags_queue $flags_space $flags_threads $flags_extra";

  my $env = $self->{env};
  my %save;


  # Overwrite some environment variables,
  # saving the old values

  foreach my $name (keys(%{$env})) {
    if (defined($env->{$name})) {
      my $value = $env->{$name};

      $save{$name} = $ENV{$name};
      $ENV{$name} = $value;
    }
  }

  my $status = 0;
  my $nbcommand = "$BATCH_SUBMIT_COMMAND $flags -J $batch_log $command $submit_log";

  $status = system($nbcommand);
  print "$nbcommand\n" if ($DEBUG);

  # restore ENV variables after submission

  foreach my $name (keys(%{$env})) {
    if (defined($env->{$name})) {
      $ENV{$name} = $save{$name};
    }
  }

 return $status;
}


=back

=head1 BUGS

A number of methods do not work, e.g., check_{pool,space,thread},
since the plain Netbatch versions of these actions don't work for
remote execution and they haven't yet been implemented.

Setting up the files on the remote node are completely the
responsibility of the user of this class.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2008

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
