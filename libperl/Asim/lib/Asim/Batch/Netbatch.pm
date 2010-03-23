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
# * @brief Batch.pm : Module for controlling batch jobs run on netbatch
# *
# * @author Joel Emer
# *
# *****************************************************************************
#

package Asim::Batch::Netbatch;
use warnings;
use strict;
use File::Basename;

our @ISA = qw(Asim::Batch::Base);

# Number of jobs submitted this cycle. 
#   TBD: This should be paritioned by pool/queue

our $JOBS_SUBMITTED_CURRENT_CYCLE = 0;

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


my $BATCH_STATUS_COMMAND = "nbqstat";
my $BATCH_SUBMIT_COMMAND = "nbq";

my $NETBATCH_PRE_NM = "tools/scripts/netbatch-pre";


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
                                    ...
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
  bless	$self, $class;

  $self->_set_temp($args->{temp});

  $self->_set_default_queue($args->{queue});
  $self->_set_default_pool($args->{pool});
  $self->{class} = $args->{class} || "TBD";

  $self->_set_flags($args->{flags});

  $self->{quiet} = $args->{quiet} || 0;

  return $self;
}

#
# Set the temp dir
#
sub _set_temp {
  my $self = shift;
  my $default = shift;

  if (defined($default)) {
    $self->{temp} = $default;
    return $default;
  }

  $self->{temp} = "/tmp";
  return $self->{temp};
}

#
# Determine the current queue (QSLOT)
#
sub _set_default_queue {
  my $self = shift;
  my $default = shift;

  if (defined($default)) {
    $self->{queue} = $default;
    return $default;
  }

  my $CURRENT_QSLOT;

  if (defined $ENV{NBQSLOT}) {
    $CURRENT_QSLOT = $ENV{NBQSLOT};
  }
  else {
    # It's difficult to determine the default qslot. For now die...
    die("No queue (QSLOT) specified - or available from environment (NBQSLOT)\n");
  }

  $self->{queue} = $CURRENT_QSLOT;
  return $CURRENT_QSLOT;
}

#
# Determine the current pool
#
sub _set_default_pool {
  my $self = shift;
  my $default = shift;

  if (defined($default)) {
    $self->{pool} = $default;
    return $default;
  }

  my $CURRENT_POOL;

  if (defined $ENV{NBPOOL}) {
    $CURRENT_POOL = $ENV{NBPOOL};
  }
  else {
    # Determine default pool using netbatch command.
    my $queue = $self->{queue};
    my $TMP_DIR = $self->{temp};

    system("$BATCH_STATUS_COMMAND -w  slot=$queue >$TMP_DIR/pool.$$");
    open (POOL_FILE, "< $TMP_DIR/pool.$$");
    while (<POOL_FILE>) {
      if ( /NetBatch\s*QUEUE\s*status\s*-\s*pool\s*(\S+)\s*$/ ) {
	$CURRENT_POOL = $1;
	last;
      }
    }
    # Cleanup
    close (POOL_FILE);
    system("rm $TMP_DIR/pool.$$");
  }

  $self->{pool} = $CURRENT_POOL;
  return $CURRENT_POOL;
}


sub _set_flags {
  my $self = shift;
  my $default = shift;

  # Set the basic flags for selecting pool/queue

  my $queue = $self->{queue};
  my $pool  = $self->{pool};
  my $class = $self->{class};

  $self->{flags_queue} = "-P $pool -Q $queue";
  $self->{flags_queue} .= " -C $class" if ($class ne "TBD");

  # Set flags for checking space

  $self->{flags_space} = "";


  # Set flags for checking threads

  $self->{flags_threads} = "";

  # Set extra user specified flags

  if (defined($default)) {
    $self->{flags_extra} = $default;
  } else {
    $self->{flags_extra} = "";
  }

  return $self->{flags_extra};
}


################################################################

=item $repository-E<gt>check_pool()

Check that the requested pool exits.

=cut

################################################################

sub check_pool {
  my $self = shift;

  my $pool = $self->{pool};
  my $queue = $self->{queue};
  my $slot = $self->{slot};

  my $asim_runlogfl = "/tmp/netbatch.log.$$";

  return 1;

  if (system ("$BATCH_STATUS_COMMAND -P$pool  slot=$queue > $asim_runlogfl")) {
    # We couldn't find the pool...

    system ("cat $asim_runlogfl");
    system ("rm -f $asim_runlogfl");
    return 0;
  }

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

###########################################################
# Get the path of the Netbatch pre-exec script to test machine viability,
# but don't bother if we're not submitting as batch jobs,
# or if our version of netbatch is not high enough.
# FIXME: does not currently work with remote virtual netbatch
##########################################################

sub check_space {
  my $self = shift;
  my $dir = shift;
  my $size = shift;

  if ($size == 0) {
    $self->{flags_space} = "";
    return;
  }

  my $nbqver = `$BATCH_SUBMIT_COMMAND -v`;
  $nbqver =~ m/netbatch\s+release\s+([0-9]+)\.([0-9]+)\./;

  my $major = $1 || 0;
  my $minor = $2 || 0;

  # First see if we can use the smart class scheduler

  if ( $Asim::Batch::NETBATCH_PRE_USE_SMARTCLASS && (($major == 6 && $minor >= 3) || $major >= 7))  {
    $self->{flags_space} .= " -C \"fDS\(\'$dir\'\)\>$size\" ";
    return 1;
  }

  # Now see if we can use the pre-exec facility

  if ( $Asim::Batch::ENABLE_NETBATCH_PRE && $major >= 6 ) {
    my $netbatch_pre_scr = Asim::resolve($NETBATCH_PRE_NM);

    if (! -f $netbatch_pre_scr) {
      print "WARNING: Netbatch pre-exec script $NETBATCH_PRE_NM does not exist.\n";
    } else {
      # Generate a real path for pre-exec script (no links)
      my $filename = basename($netbatch_pre_scr);
      $netbatch_pre_scr = ::realpath(dirname($netbatch_pre_scr)) . "/$filename";

      if ( ! ( -f $netbatch_pre_scr )) {
	print "WARNING: Netbatch pre-exec script $NETBATCH_PRE_NM does not exist.\n";
      } else {
	# We're here because we found the pre-exec script file successfully.
	# So padd a pre-exec option to netbatch args.
	
	my $preexec_cmd = "$netbatch_pre_scr -tempdir $dir $size";
	$self->{flags_space} .= " --pre-exec \"$preexec_cmd\" ";
	return 1;
      }
    }

    return 0;
  }

  return 0;
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

  if ($threads <= 1) {
    $self->{flags_threads} = "";
    return 1;
  }

  # TBD: Do we need to make sure the class scheduler works!!!

  $self->{flags_threads} = " -C \"CPUCount>=$threads\" ";

  return 1;
}

################################################################

=item $batch-E<gt>scheduler($args);

Local scheduler to wait until its appropriate to submit a job.
Args are passed as a hash with a variety of keys:

         max_running_jobs
         max_waiting_jobs
         max_asim_jobs
         max_jobs_per_period
         min_sample_period

=cut

################################################################

sub scheduler {
  my $self = shift;

  my $args = {@_};
  my $MAX_RUNNING_JOBS    = $args->{max_running_jobs}    || 175;
  my $MAX_WAITING_JOBS    = $args->{max_waiting_jobs}    ||  10;
  my $MAX_ASIM_JOBS       = $args->{max_asim_jobs}       || 300;
  my $MAX_JOBS_PER_PERIOD = $args->{max_jobs_per_period} ||  10;
  my $MIN_SAMPLE_PERIOD   = $args->{min_sample_period}   || 600;

  my $queue = $self->{queue};
  my $pool  = $self->{pool};
  my $TMP_DIR = $self->{temp};

  my $user = $ENV{USER} || "UNKNOWN_USER";

  #
  # Algorithm for scheduling jobs
  # Schedule if:
  # a.) User has less than $MAX_WAITING_JOBS waiting jobs in queue
  # b.) Total ASIM jobs running is less than $MAX_ASIM_JOBS
  # c.) If the number of jobs owned by user is > MAX_RUNNING_JOBS, then
  #     user can submit at most $MAX_JOBS_PER_PERIOD jobs before going
  #     to sleep.
  #
  # Else, loop until we can submit another jobs

  while (1) {
    #
    # Check for waiting jobs
    #
    my $num_waiting = 0;

    # nbqstat will report jobs with either the hierarchical name (using the -n argument)
    # or the numerical slot name.  Depending on which format your asimrc file uses,
    # we need to check BOTH formats to make sure we really know how many waiting
    # jobs you have.

    system("$BATCH_STATUS_COMMAND -P$pool  -w -n slot=$queue >$TMP_DIR/wait.$$");
    system("$BATCH_STATUS_COMMAND -P$pool  -w slot=$queue    >>$TMP_DIR/wait.$$");

    open (WAIT_JOBS, "<$TMP_DIR/wait.$$");
    while (<WAIT_JOBS>) {
      chomp;
      if ( /\s+$queue\s+$user\s+/ ) {
	$num_waiting++;
      }
    }
    close (WAIT_JOBS);
    system("rm $TMP_DIR/wait.$$");

    #
    # Check for running jobs submitted by user.
    #
    my $num_user_running = 0;
    my $num_total_running = 0;

    system("$BATCH_STATUS_COMMAND -P$pool  -r -j -f  slot=$queue> $TMP_DIR/run.$$");
    open (RUN_JOBS, "<$TMP_DIR/run.$$");
    while (<RUN_JOBS>) {
      chomp;
      if (/^User: $user(.*)/) {
	my $jobname = <RUN_JOBS>;
	if ($jobname =~ /^Job Name:(.*)\.asimlog(.*)/) {
	  $num_total_running++;
	  $num_user_running++;
	}
      }
      elsif (/^Job Name:(.*)\.asimlog(.*)/) {
	$num_total_running++;
      }
    }
    close (RUN_JOBS);
    system("rm $TMP_DIR/run.$$");

    #
    # Given all the above information we decide whether to sleep
    # TBD: Maybe the following should be refactored so it can be shared...
    #
    $self->print_info(".... Number jobs waiting for $user: $num_waiting/$MAX_WAITING_JOBS\n");
    $self->print_info(".... Number jobs running for $user: $num_user_running/$MAX_RUNNING_JOBS\n");
    $self->print_info(".... Number jobs running for ASIM users: $num_total_running/$MAX_ASIM_JOBS\n");

    my $sleep = 0;

    if ($num_waiting > $MAX_WAITING_JOBS) {
      #
      # Exceeding limit.... go to sleep.
      #
      $self->print_info("........ Number of waiting jobs ($num_waiting) exceeds limit ($MAX_WAITING_JOBS)");
      $sleep = 1;
    }

    if ( $num_total_running > $MAX_ASIM_JOBS ) {
      #
      # Exceeding limit.... go to sleep.
      #
      $self->print_info("........ Number of running ASIM jobs ($num_total_running) exceeds $MAX_ASIM_JOBS.\n");
      $sleep = 1;
    }

    # If we've exceeded a given limit, then slow injection down.
    if (($num_user_running > $MAX_RUNNING_JOBS) &&
	($JOBS_SUBMITTED_CURRENT_CYCLE > $MAX_JOBS_PER_PERIOD)) {
      #
      # Exceeding limit.... go to sleep.
      #
      $self->print_info("........ Number of running ASIM jobs ($num_user_running) exceeds fast injection limit ($MAX_RUNNING_JOBS)");
      $self->print_info("........ Only $MAX_JOBS_PER_PERIOD can be submitted at any time once this limit is reached.\n");
      $sleep = 1;
    }

    if ($sleep == 1)
      {
	$self->_sleep_sample_period($MIN_SAMPLE_PERIOD);
	$JOBS_SUBMITTED_CURRENT_CYCLE = 0;
      }
    else
      {
	# Schedule job
	$JOBS_SUBMITTED_CURRENT_CYCLE++;
	return;
      }
  }
}



################################################################

=item $flags = Asim::Batch::Netbatch::consolidate_smartclass_expressions($flags)

Internal utility function to transform a series of class expressions
of the form "-C class1 -C class2 -C class3" into a boolean expression
of the form "-C class1&&class2&&class3" so Netbatch gets a single class
expression to digest.

=cut

################################################################

sub consolidate_smartclass_expressions {
  my $flags = shift;
  my @exprs = ();
  while ($flags =~ s/-C\s+(\S+)//) {
    my $exp = $1;
    # strip any enclosing single- or double-quotes:
    if ($exp =~ m/^\s*\"(.*)\"\s*$/) {$exp = $1}
    if ($exp =~ m/^\s*\'(.*)\'\s*$/) {$exp = $1}
    # enclose in parens if necessary
    if ($exp =~ m/[\*\/\+\-|&><=!]/) {
      unless ($exp =~ m/^\s*\(.*\)\s*$/) {$exp = '('.$exp.')'}
    }
    push @exprs, $exp;
  }
  # enclose the result in double quotes, and add remaining flags:
  return '-C "' . join('&&',@exprs) . '" ' . $flags;
}


################################################################

=item $batch-E<gt>submit($command, $batch_log, $submit_log, $block)

Submit a new batch command.

=cut

################################################################

sub submit {
  my $self       = shift;
  my $command    = shift;
  my $batch_log  = shift ||  "/dev/null";
  my $submit_log = shift || ">/dev/null";
  my $block      = shift || 0;

  my $flags_queue   = $self->{flags_queue};
  my $flags_space   = $self->{flags_space};
  my $flags_threads = $self->{flags_threads};
  my $flags_extra   = $self->{flags_extra};

  my $flags = "$flags_queue $flags_space $flags_threads $flags_extra"
            . ($block?" --block":"");

  $flags = consolidate_smartclass_expressions($flags);

  my $nbcommand = "$BATCH_SUBMIT_COMMAND $flags -J $batch_log $command $submit_log";
  my $status = 0;

  print "$nbcommand\n" if ($DEBUG);
  $status = system($nbcommand);

  return $status;
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
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Asim::Betch::Netbatch: Error - $message";

  return 1;
}


################################################################                                                                                                                                                                             
#                                                                                                                                                                                                                                            
# Run a command at the end of execution                                                                                                                                                                                                      
#                                                                                                                                                                                                                                            
################################################################                                                                                                                                                                             

sub submission_complete {
    my $self       = shift;
    my $resdir     = shift;
    my $command    = shift;


    if(defined $command) {
	print STDERR " Running locally, Netbatch not yet supported: $command > $resdir/completion_command.log";
	system("$command > $resdir/completion_command.log 2>&1");
    }

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
