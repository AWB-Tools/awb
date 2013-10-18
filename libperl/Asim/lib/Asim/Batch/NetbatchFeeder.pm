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
# * @brief NetbatchFeeder.pm : Module for controlling batch jobs run using the Netbatch Feeder
# *
# * @author Carl Beckmann
# *
# *
# *****************************************************************************
#

package Asim::Batch::NetbatchFeeder;
use warnings;
use strict;
use File::Basename;
require File::Temp;
use File::Temp ();

our @ISA = qw(Asim::Batch::Base Asim::Batch::Netbatch);

=head1 NAME

Asim::Batch::NetbatchFeeder - Library for manipulating netbatch jobs via the NB feeder

=head1 SYNOPSIS

    use Asim::Batch::Base;
    use Asim::Batch::Netbatch;
    use Asim::Batch::NetbatchFeeder;

=head1 DESCRIPTION

This module provides an object to submit jobs into Netbatch via the Netbatch feeder.
By using the NB feeder instead of raw Netbatch commands,
the jobs are visible and can be manipulated using the nbflow GUI.

=head1 METHODS

The following public methods are supported:

=over 4

=item $batch = Asim::Batch::NetbatchFeeder-E<gt>new( pool          => <pool name>,

                                       queue         => <queue name>,
                                       class         => <batch class>,
                                       pool_build    => <pool name>,
                                       queue_build   => <qslot name>,
                                       class_build   => <batch class>,
                                       taskfile      => <NB feeder task file>,
                                       workarea      => <NB feeder work area>,
                                       nbfeeder_opts => <more NB feeder options>
                                     );

Create a new batch object.

=cut

################################################################

sub new {
  my $this = shift;
  my $args = {@_};
  my $class = ref($this) || $this;
  my $self = {};
  bless	$self, $class;
  
  foreach my $key (keys %{$args}) {
    $self->{$key} = $args->{$key};
  }

  $self->_set_default_queue($args->{queue});
  $self->_set_default_pool($args->{pool});
  $self->{class} = $args->{class} || '@';

=pod

If omitted, the optional pool_build, queue_build, and class_build arguments
default to the non-build arguments pool, queue, and class, respectively.

=cut

  $self->{pool_build}  = $args->{pool_build}  || $self->{pool};
  $self->{queue_build} = $args->{queue_build} || $self->{queue};
  $self->{class_build} = $args->{class_build} || $self->{class};
  
  $self->{task_no} = 0;
  $self->{last_build_task} = -1;
  $self->{task_list} = [];
  $self->{cur_task} = undef;

  return $self;
}

################################################################

=item $batch-E<gt>scheduler( max_waiting_jobs    => <n>,

               max_running_jobs    => <n>
             );

Local scheduler to wait until appropriate to submit a job.
The Netbatch feeder handles job submissions, so this routine just returns immediately.

It remembers any scheduling parameters passed in as arguments,
which are used later when launching the feeder to actually submit the jobs.

=cut

sub scheduler {
  my $self = shift;
  my $args = {@_};

  foreach my $key (keys %{$args}) {
    $self->{$key} = $args->{$key};
  }
}

################################################################

=item $batch-E<gt>submit($command, $batch_log, $submit_log, $build [, $dep_task])

Submit a new batch command to be launched into the batch system.

The $submit_log argument is ignored (for compatibility with other batch libraries).

If the $build argument is 1, then this is assumed to be a build job,
and all subsequent non-build jobs up to the next build job
are assumed to be dependent on it.
Also, the '_build' versions of pool, queue, and class are used to run this job.

The optional $dep_task argument specifies a previous task on which this job depends.

=cut

sub submit {
  my $self       = shift;
  my $command    = shift;
  my $batch_log  = shift ||  "/dev/null";
  my $submit_log = shift || ">/dev/null"; # not used
  my $build      = shift || 0;
  my $dep_task   = shift || -1;
  chomp $command;

  if (! defined $self->{cur_task}) {
    $self->begin_task($build, 0, $dep_task);
  } elsif ($build) {
    $self->end_task();
    $self->begin_task(1, 0, $dep_task);
  }

  $self->add_job($command, $batch_log);
  
  if ($build) {
    $self->end_task();
    $self->{last_build_task} = $self->{task_no};
  } else {
    $self->{last_build_task} = -1;
  }

  return 0;
}

################################################################

=item $taskno = $batch-E<gt>submit_depend($command, $batch_log, $submit_log, $build [,$dep_task])

Submit a new batch command to be launched into the batch system.

This is just like submit(), except that it returns a task number.

=cut

sub submit_depend {
  my $self = shift;
  $self->submit(@_);
  return $self->{task_no};
}

################################################################                                                                                                                                                                             

=item $batch-E<gt>submission_complete($resdir, $command)

Arrange to run $command at the end of execution, after all other build and run jobs,
with a log file completion_command.log in results directory $resdir.

This routine must be called after all other build and run jobs have been submitted.
It actually invokes the Netbatch feeder to launch all previously submitted jobs.

=cut

sub submission_complete {
  my $self       = shift;
  my $resdir     = shift;
  my $command    = shift;

  # finish writing the current task:
  if (defined $self->{cur_task}) {
    $self->end_task();
  }

  # add a task to run the command at the end:
  if (defined $command) {
    $self->begin_task(0, 1);
    $self->add_job($command, "$resdir/completion_command.log", 0);
    $self->end_task();
  }
  
=pod

If no taskfile arg was given when $batch was created,
a temp file will be created for the taskfile.

=cut

  unless (defined $self->{taskfile}) {
    $self->{taskfh} = File::Temp->new();
    $self->{taskfile} = $self->{taskfh}->filename;
  }

  # create the task file:
  $self->emit_taskfile();

=pod

If no workarea arg was given when $batch was created,
then the work area directory name defaults to $resdir/FeederWork .

=cut

  my $workarea = $self->{workarea} || File::Spec->catfile($resdir, 'FeederWork');
  
  my $feeder_args = "--work-area $workarea --terminate-on-finish";
  $feeder_args .= ' --max-jobs '   .$self->{max_running_jobs} if defined $self->{max_running_jobs};
  $feeder_args .= ' --max-waiting '.$self->{max_waiting_jobs} if defined $self->{max_waiting_jobs};
  $feeder_args .= ' '              .$self->{nbfeeder_opts}    if defined $self->{nbfeeder_opts};

  # now submit it to the netbatch feeder:
  my $taskfile = $self->{taskfile};
  my $runcmd = "nbfeeder start $feeder_args --task $taskfile";
  $self->print_info("Running command: $runcmd");
  system($runcmd) || return 0;

  return 1;
}

################################################################

=item $batch-E<gt>begin_task($build_task, $last_task [, $dep_task]);

Start creating a new task definition in the task file.

The two optional arguments indicate whether this is a build task,
and whether it is the final task that runs after all others complete.

The optional $dep_task argument specifies a previous task that this one depends on.

Internal routine, not intended to be called from outside this module.

=cut

sub begin_task {
  my $self  = shift;
  $self->{task_no}++;

  my $task = {};
  $task->{build} = shift || 0;
  $task->{last}  = shift || 0;
  $task->{task_no}         = $self->{task_no};
  $task->{last_build_task} = $self->{last_build_task};
  
  my $dep_task = shift || -1;
  $task->{last_build_task} = $dep_task if $dep_task > 0;

  if ($task->{build}) {
    $task->{pool}  = $self->{pool_build};
    $task->{queue} = $self->{queue_build};
    $task->{class} = $self->{class_build};
  } else {
    $task->{pool}  = $self->{pool};
    $task->{queue} = $self->{queue};
    $task->{class} = $self->{class};
  }
  $task->{job_list} = [];

  $self->{cur_task} = $task;
}

################################################################

=item $batch-E<gt>add_job($command, $batch_log);

Finish the current task definition in the task file.

Internal routine, not intended to be called from outside this module.

=cut

sub add_job {
  my $self = shift;
  my $job = {};
  $job->{command}   = shift;
  $job->{batch_log} = shift || "/dev/null";
  push @{$self->{cur_task}->{job_list}}, $job;
}

################################################################

=item $batch-E<gt>end_task();

Finish the current task definition in the task file.

Internal routine, not intended to be called from outside this module.

=cut

sub end_task {
  my $self = shift;
  push @{$self->{task_list}}, $self->{cur_task};
  $self->{cur_task} = undef;
}

################################################################                                                                                                                                                                             

=item $batch-E<gt>emit_taskfile();

Create the Netbatch feeder Task File,
using job information previously submitted via submit() and submission_complete() calls.

Internal routine, not intended to be called from outside this module.

=cut

sub emit_taskfile {
  my $self    = shift;

  return unless $self->{taskfile};
  open  TASKFILE, '>>'.$self->{taskfile} || return;
  
  foreach my $task ( @{$self->{task_list}} ) {
  
    # emit the task header

    print TASKFILE 'Task TSK', $task->{task_no}, "\n";
    print TASKFILE "{\n";
    print TASKFILE '  Queue ', $task->{pool}, "\n";
    print TASKFILE "  {\n";
    print TASKFILE '    Qslot ', $task->{queue}, "\n";
    print TASKFILE "  }\n";

    if ($task->{last}) {
      # last task depends on all others before it:
      for (my $i = 1; $i < $task->{task_no}; $i++) {
        print TASKFILE '  DependsOn TSK', $i, '[OnSuccess]', "\n";
      }

    } elsif ($task->{last_build_task} > 0) {
      # depend on previous build task, if any:
      print TASKFILE '  DependsOn TSK', $task->{last_build_task}, '[OnSuccess]', "\n";
    }

    print TASKFILE "  Jobs\n";
    print TASKFILE "  {\n";
    
    # emit the list of jobs

    foreach my $job ( @{$task->{job_list}} ) {
      print TASKFILE '    nbjob run --class ', $task->{class},
                         ' --log-file ',       $job->{batch_log},
                         ' ',                  $job->{command},
                         "\n";
    }
    
    # emit the task trailer

    print TASKFILE "  }\n";
    print TASKFILE "}\n";

  }

  close TASKFILE;
}

################################################################                                                                                                                                                                             

=back

=head1 BUGS

On build failures, the .SUBMITTED files for simulation runs are not deleted or converted to .ERROR files,
so the old non-nbflow-GUI way of looking for failures can be confusing.

The scheduler args max_asim_jobs, max_jobs_per_period, and min_sample_period
are not supported as they are in the Asim::Batch::Netbatch library.

=head1 AUTHORS

Carl Beckmann

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2013

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
