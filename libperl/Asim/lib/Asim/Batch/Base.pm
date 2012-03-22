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


package Asim::Batch::Base;
use warnings;
use strict;

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


=head1 NAME

Asim::Batch::Base - Base methods for manipulating batch jobs

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

  die("Asim::Batch::*->new() must be implemented in a batch system specific metohd\n");

  bless	$self, $class;

  return $self;
}


################################################################

=item $batch-E<gt>check_pool()

Check that the requested pool exits.

=cut

################################################################

sub check_pool {
  return 1;
}

################################################################

=item $batch-E<gt>check_space($dir, $size)

Check that $dir has size > $size.

=cut

################################################################

sub check_space {
  my $self = shift;

  # We don't know how to check space...shouldn't be fatal
  return 1;
}


################################################################

=item $batch-E<gt>scheduler();

Local scheduler to wait until its appropriate to submit a job

=cut

################################################################

sub scheduler {
  my $self = shift;

  # We don't know how to wait. So just let it go...
  # ... if we need to sleep, we could use the method below

  return 1;
}

# -----------------------------------------------------------------------------
#
# Utility function to sleep a short interval
#
sub _sleep_sample_period()
{
  my $self = shift;
  my $min_sample_period = shift || 600;

  my $time_stamp = `date`;
  chomp($time_stamp);

  my $duration = rand;
  $duration = $min_sample_period + int($duration * 1200);
	
  $self->print_info("\t\t$time_stamp: Going to sleep for $duration seconds\n");
  system("sleep $duration");
  return;
}


################################################################

=item $batch-E<gt>submit($command)

Submit a new batch command.

=cut

################################################################

sub submit {

  die("This class can't submit a job!!!\n");
  return 0;
}


################################################################

=item $batch-E<gt>get_queue()

Get the queue name

=cut

################################################################

sub get_queue {
  my $self = shift;

  return $self->{queue};
}

################################################################

=item $batch-E<gt>get_pool()

Get the pool name

=cut

################################################################

sub get_pool {
  my $self = shift;

  return $self->{pool};
}

################################################################

=item $batch-E<gt>print_info($line)

Print some info

=cut

################################################################

sub print_info {
  my $self = shift;

  my $quiet = $self->{quiet};
  my $timepidstr = $self->_get_timepid_str();

  while (my $line = shift) {
    chomp $line;
    if (!$quiet) {
      print "$timepidstr $line\n";
    }
  }
    
  return;
}

# -----------------------------------------------------------------------------
sub _get_timepid_str
{
  my $self = shift;

  my $printtimes = 1;
  my $timestr = "";

  if ($printtimes) {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
    $timestr = sprintf "%02d:%02d:%02d ",$hour,$min,$sec;
  }
    
  my $pid = $$;
  my $pidstr = sprintf "%05d", $pid;
    
  return $timestr.$pidstr;
}

################################################################

=item $repository-E<gt>check_threads($nthreads)

Derived classes can implement this if they want the batch system
to check to ensure that the batch host has enough cores to run
the given number of worker threads.

The argument is the number of worker threads used by the simulation.
Returns 1 on success.

=cut

################################################################

sub check_threads {
  return 1;
}

################################################################

=item $batch-E<gt>reserve_all_threads($reserve)

Derived classes can implement this if they want the batch system
to reserve all of the cores on the batch host machine for the
simulation to use exclusively.

The argument is a boolean, indicating whether or not to
reserve all cores.  Returns 1 on success.

=cut

################################################################

sub reserve_all_threads {
  return 1;
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
