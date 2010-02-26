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
# * @brief Batch.pm : Module for controlling batch jobs running on condor
# *
# * @author Kermin Fleming
# *
# *****************************************************************************
#


package Asim::Batch::Condor;
use warnings;
use strict;

our @ISA = qw(Asim::Batch::Base);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_BATCH});


=head1 NAME

Asim::Batch::Condor - Library for manipulating locally run batch jobs

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

  # Add my stuff here

  # need to know tempdir.

  # seems we have to crack the arguments and exec statement a little bit.
  # also, condor needs permissions on the run directories.
  
  open(OUTFILE, "> ${batch_log}.job");

  print OUTFILE "Universe = vanilla\n";
  print OUTFILE "GetEnv = True\n";
 
  my $env = $self->{env};
  my $env_vars = "";

  foreach my $name (keys(%{$env})) {
      if (defined($env->{$name})) {
	  my $value = $env->{$name};
          $env_vars = $env_vars . " $name=$value ";
      }
  }

  # because condor wants cmd and args seperate, we must crack the 
  # cmd from awb.  Not terribly robust, since awb should be doing this 
  # already

  my @commands = split(/\s/,$command);

  # need to pick up asim-batch from the environment
  my $asimbatch = `which asim-batch`;

  print OUTFILE "executable = $asimbatch\n";

  shift(@commands);

  my $flags = $self->{flags};
  unshift(@commands,$flags);
  my $newflags = join(" ", @commands);

  # remove " from the file
  $newflags =~ s/([^\\])\"/$1\\\"/g;

  print OUTFILE "arguments = $newflags \n";

  # get rid of any extra things in the submit log....
  my @submit_logs = split(/\s/,$command);  

  print OUTFILE "log = $batch_log.log\n";

  print OUTFILE "output = $batch_log\n";

  print OUTFILE "error = $batch_log.error\n";

  print OUTFILE "requirements = Memory>750 && Arch==\"X86_64\" && (isPublic || isCSG)\n";



  print OUTFILE "queue\n";

  close OUTFILE;

  system("condor_submit $batch_log.job"); 

  system("echo Submitted ${batch_log}.job");


  return 0;
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
# Thread check function
#
################################################################

sub check_threads {
    my $self = shift;

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
