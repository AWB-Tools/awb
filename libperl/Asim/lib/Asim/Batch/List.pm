#
# Copyright (C) 2011 Intel Corporation
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
# * @brief List.pm : Module for controlling batch jobs running on a list of
# *                  machines that I can ssh into
# *
# * @author Carl Beckmann
# *
# *****************************************************************************
#


package Asim::Batch::List;
use Asim::Fork;
use warnings;
use strict;
use POSIX ":sys_wait_h";

our @ISA = qw(Asim::Batch::Base);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


=head1 NAME

Asim::Batch::List - Library for manipulating batch jobs run on a list of machines via SSH

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to allow awb-run to launch jobs via SSH to an explicit
list of machines.  This module simply launches each job to another machine in the list,
waiting for each job to complete before launching the next one on that machine.
No underlying batch system is used, other than the ability to run a command on a
remote machine via "ssh".

=head1 METHODS

The following public methods are supported:

=over 4

=item $batch = Asim::Batch::List-E<gt>new(pool => $hostlist, slots_per_host => $n);

Create a new batch object object.
The arguments are a set of hash values.

The "pool" hash key specifies a string,
which is a list of host machines, separated by spaces, commas, colons or semicolons.
It specifies the list of machines that the jobs will run on, via ssh.

You can optionally specify the number of jobs that can run on each host
concurrently, using the "slots_per_host" hash key, which defaults to 1.

=cut

sub new {
    my $this = shift;

    my $class = ref($this) || $this;
    my $self;

    $self = {   @_,
                queue => "N/A",
                host_jobs => {},
                fork_num => 0
            };
    bless	$self, $class;

    # split the host list string into an actual list
    $self->ierror('No hosts => \$list_of_hostnames argument given') unless defined $self->{pool};
    my @host_list = split /[ ,:;]+/, $self->{pool};
    $self->{host_list} = \@host_list;

    # the number of concurrent jobs per host in the list defaults to one:
    $self->{slots_per_host} = 1 unless defined $self->{slots_per_host};
    
    # before we begin, remotely execute an 'uptime' command,
    # to see load on remote machines, and flush out any ssh login issues:
    if ($DEBUG) {
        foreach my $host (@{$self->{host_list}}) {
            print "HOST $host STATUS:\n";
            system("ssh $host uptime");
            # for each host, create a list of running jobs (initially empty):
            $self->{host_jobs}->{$host} = [];
        }
    }

    return $self;
}


=item $repository-E<gt>get_free_host()

Return the name of a free host, or <undef> if there are no free hosts.

=cut

sub get_free_host {
    my $self = shift;
    foreach my $host (@{$self->{host_list}}) {
        print "\nAsim::Batch::List::get_free_host() checking host $host\n" if $DEBUG;
        # first run through the list of jobs and find the ones still running
        my @stillrunning = ();
        foreach my $child (@{$self->{host_jobs}->{$host}}) {
            print " checking if running job $child completed\n" if $DEBUG;
            if (0 != waitpid($child, WNOHANG)) {
                print " Yes!  :-)\n" if $DEBUG;
            } else {
                print " No.  :-(\n" if $DEBUG;
                push @stillrunning, $child;
            }
        }
        @{$self->{host_jobs}->{$host}} = @stillrunning;
        # if there's room for one more on this machine, return it
        if (($#stillrunning + 1) < $self->{slots_per_host}) {
            print " host $host is available!  :-)\n" if $DEBUG;
            return $host;
        }
    }
    print "Asim::Batch::List::get_free_host() found no free host\n" if $DEBUG;
    return undef;
}


=item $batch-E<gt>scheduler($args);

Local scheduler to wait until it is appropriate to submit a job.
Args are passed as a hash with a variety of keys.
The "min_sample_period" key specifies the number of seconds to wait
if all hosts are busy, before checking again.
Returns the name of a free host upon exit.

=cut

sub scheduler {
    my $self = shift;

    my $args = {@_};
    my $MIN_SAMPLE_PERIOD = $args->{min_sample_period} || 600;
    
    while (1) {
        my $host = $self->get_free_host();
        return $host if defined $host;
        print "Asim::Batch::List:scheduler() sleeping until next job finishes\n";
        wait();
    }
}


=item $repository-E<gt>submit($command,$batch_log,$submit_log,$block)

Submit a new batch command.
This routine forks a process which invokes ssh to run the command on the remote host.
The caller returns immediately.

=cut

sub submit {
    my $self       = shift;
    my $command    = shift;
    my $batch_log  = shift;
    my $submit_log = shift;
    my $block      = shift || 0;

    my $status = 0;
    chomp $command;
    print "\nAsim::Batch::List::submit() got command $command\n" if $DEBUG;

    # find an available host, wait for it if necessary:
    my $host = $self->scheduler();
    return "all hosts are busy" unless defined $host;
    
    # fork a process that invokes ssh to run the command remotely
    my $forkname = 'fork' . ++$self->{fork_num};
    my $child = Asim::Fork::controlled_fork(1, '/dev/null', $forkname, 0);
    if ($child != 0) {
        # parent remembers child pid so we can wait on it
        push @{$self->{host_jobs}->{$host}}, $child;
    } else {
        # child executes the command
        my $ret = system("ssh $host $command >$batch_log 2>&1");
        Asim::Fork::controlled_exit(1, $ret >> 8);
    }

    return $status;
}


=item $repository-E<gt>submission_complete($resdir,$command)

Run a command at the end of execution 

=cut

sub submission_complete {
    my $self       = shift;
    my $resdir     = shift;
    my $command    = shift || '';

    print "Asim::Batch::List::submission_complete() in $resdir running $command\n" if $DEBUG;
}


=back

=head1 BUGS

This doesn't seem to work properly for building in batch mode.
Use only with --nobuildbatch for now.

=head1 AUTHORS

Carl Beckmann

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2011

=cut


1;
