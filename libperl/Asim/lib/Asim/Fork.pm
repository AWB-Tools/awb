#
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
# * @brief Fork.pm : Forked prcoess control
# *
# * @author Brian Slecta, Joel Emer
# *
# *****************************************************************************
#


package Asim::Fork;
use warnings;
use strict;

use POSIX ":sys_wait_h";

# Global constants

our $delay = 0;

# Variables preserved in fork

our $process_name = "process";
our $wait_for_children = 1;

our $quiet = 0;
our $printtimes = 1;

# Variables about our children

our @children = ();
our %child_log;

our $exit_code;


our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_FORK});


=head1 NAME

Asim::Fork - Library of subroutines for manipulating forked proceses

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate forked processes. Basically
it allows one to create a forked process, signal that the forked process
is done and wait for all forked process to finish.

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item $batch = Asim::Fork::controlled_fork(
                                    queue  => <queuename>,
                                    class => <batch class>,
                                   );


Create a new batch object object.

=cut

################################################################

sub init {
  $process_name = shift;
  $wait_for_children = shift;
  $quiet = shift || 0;
  $printtimes = shift || 1;
}


# -----------------------------------------------------------------------------
# fork with some throttling.  any process started with this needs to exit with
# controlled_exit().  increment the reference count.

sub controlled_fork {
  my $dofork = shift  || return 1;
  my $log = shift;
  my $forkname = shift;
  my $wait = shift;
  # zero throttle means infinite, basically
  my $throttle = shift || 9999999;

  # let's throttle a bit if specified
  if ($delay > 0 ) { sleep($delay); };

  # loop while checking the maxchildren.  dont exceed the max.
  while (1) {
    # $child_count_lock->shlock();
    # if ($child_count < $dofork) {
    #     $done = 1;
    #     $child_count = $child_count + 1;
    # }
    # $child_count_lock->shunlock();

    my $num_children = count_children(1);

    print_debug("Attempting to fork with num_children=$num_children and throttle=$throttle\n");

    if ($num_children < $throttle) {
      # we are allowed to breed a new child.
      my $pid = fork();

      if ($pid == 0) {
        # this is the child

        $process_name .= "->".$forkname;
        $wait_for_children = $wait;
        @children = ();
        $exit_code = 0;

        print_debug("Starting forked pid $$ as $process_name with PPID ".getppid()."\n");

        if (defined $log) {
#          open(LOGFILE, ">>$log") || die ("Unable to open $log\n");
#          *STDOUT = *LOGFILE;
#          *STDERR = *LOGFILE;

          close(STDOUT);
          open(STDOUT, ">$log");
          close(STDERR);
          open(STDERR, ">&STDOUT");

          print_info("Running in fork...\n");
        }
      }
      else {
        print_info("Forked pid $pid\n");
        print_debug("Adding pid $pid to list of children (".@children.")\n");

        push @children, $pid;

        if (defined($log)) {
          $child_log{$pid} = $log;
        }

      }

      # $child_count_lock = tie $child_count, 'IPC::Shareable', undef, { destroy => 0 };

      return $pid;
    }

    # no more children allowed now.  let's wait and try again.

    print_debug("Too many children right now, sleeping for a bit...\n");

    # assume about 0.2 sec per child - should probably have some running estimate

    my $excess = $num_children  - $throttle;

    sleep(1 + int($excess / 5));
  }
}

# -----------------------------------------------------------------------------
# exit a thread that was started with controlled_fork().  decrement the
# reference count.
sub controlled_exit {
  my $dofork = shift;
  my $exit_code = shift;

  if (! $dofork) {
    return $exit_code;
  }

  # BE CAREFUL WITH PRINTING MESSAGES HERE
  # THIS MIGHT BE CALLED VERY EARLY

  # we are in forking mode, so deal with ref count and exit

  # decrement the reference count
  # $child_count_lock->shlock();
  # $child_count = $child_count - 1;;
  # $child_count_lock->shunlock();

  # exit with the specified args
  $exit_code += wait_for_children();

  print_info("Exiting with Exit Code $exit_code\n");
  print_debug("Exiting fork had PPID ".getppid()."\n");

  exit $exit_code;
}



sub wait_for_children {
    my $ecode = 0;


    if (! $wait_for_children) {
      print_debug("No wait needed\n");
      return $ecode;
    }

    print_debug("Starting to wait for children\n");

    my $num = 0;

    for (my $child = wait(); $child != -1; $child = wait()) {
      my $child_ecode = $? >> 8;
      my $child_signum = $? & 127;
      my $child_dumped_core = $? & 128;

      $num++;

      print_info("Done Waiting For Child PID $child with Exit Code $child_ecode\n");

      if ($child_ecode != 0) {
        _error_log($child);
      }

      $ecode += $child_ecode;
    }

    print_debug("Finished waiting for $num children\n");

    return $ecode;
}


# -----------------------------------------------------------------------------
# searches through the list of children and counts the number of active
# children in the list.  will reap the completed children from the list if that
# param is passed as true.
sub count_children {
  my ($reap) = @_;

  my $num_alive = 0;
  my $num_dead = 0;

  my $children_exit_codes = 0;
  my @alive = ();

  foreach my $child (@children) {
    my $p = waitpid($child, WNOHANG);

    if ($p < 0) {
      # no such child
    }
    elsif ($p == 0) {
      # still running
      $num_alive++;
      push @alive, $child;
    }
    else {
      # child is diseased
      if (defined $reap && $reap)
      {
        my $child_ecode = $? >> 8;
        my $child_signum = $? & 127;
        my $child_dumped_core = $? & 128;

        $num_dead++;

        if ($child_ecode != 0) {
          _error_log($child);
        }

        # sum up the exit codes from the children and this process
        $children_exit_codes += $child_ecode;
      }
    }
  }

  if (defined $reap && $reap) {
    my $num_alive = @alive;

    print_debug("Reaped $num_dead children from list - exits codes were $children_exit_codes\n");
    print_debug("New child list size is $num_alive\n");

    $exit_code += $children_exit_codes;
    @children = @alive;
  }

  return $num_alive;
}


sub _error_log {
  my $pid = shift;
  my $log = $child_log{$pid};

  if (defined($log)) {
    print "################################################################\n";
    print "ERROR: Starting error log from child $pid\n";
    print "################################################################\n";

    open (ERRORLOG, "<$log");
    while (<ERRORLOG>) {
      print $_;
    }

    print_info("End of fork - $pid\n");

    print "################################################################\n";
    print "ERROR: Finished error log from child $pid\n";
    print "################################################################\n";
  }

}


# -----------------------------------------------------------------------------
#
# A series of stylized print routnes
#         Partially copied from asim-run for output style consisency
#         TBD: Unify all this log printing stuff
# -----------------------------------------------------------------------------

sub print_info
{
  if ($quiet) {
    return;
  }

  my $timepidstr = get_timepid_str();

  while (my $line = shift) {
    chomp $line;
    print "$timepidstr $line\n";
  }

  return;
}

sub print_debug
{
  if (!$DEBUG) {
    return;
  }

  my $timepidstr = get_timepid_str();

  while (my $line = shift) {
    chomp $line;
    print "$timepidstr $line\n";
  }

  return;
}


# -----------------------------------------------------------------------------
sub get_timepid_str
{
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

Joel Emer copied this code from code in asim-run written by Brian Slecta.

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2008

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
