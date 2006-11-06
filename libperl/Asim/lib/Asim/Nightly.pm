#
# *****************************************************************
# *                                                               *
# Copyright (C) 2005-2006 Intel Corporation
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


package Asim::Nightly;
use warnings;
use strict;

use Asim::Inifile;

our %a =  ( name        =>          [ "name",
                                      "SCALAR" ],
            description =>          [ "description",
                                      "SCALAR" ],
            location    =>          [ "location",
                                      "SCALAR" ]
      );


=head1 NAME

Asim::Nightly - Utilities for nightly regressions

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides some functions to help with nightly regressions

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item $nightly = Asim::Nightly-E<gt>new();


Create a new nightly regression object.

=cut

################################################################

sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = _initialize();

  bless	$self, $class;

  #
  # Parse nightly if given
  #
  if (@_) {
    $self->{regression} = $_[0];
  } else {
    $self->{regression} = "NIGHTLY";
  }

  $self->open() || return undef;

  return $self;
}


sub _initialize {
  my $self = {  accessors => \%a,
		filename  => "",
                location  => "",
                inifile   => undef,
                mailto    => "joel.emer\@intel.com",
                program   => "unknown.program",
                logfile   => undef,
             };

  return $self;
}




################################################################

=item $nightly-E<gt>open($file)

Parse nightly regression file

=cut

################################################################


sub open {
  my $self = shift;
  my $location = $ENV{HOME} . "/.asim";
  my $ini;

  #
  # Remember the location of the nightly rc file
  #
  $self->{location} = $location;

  $ini = new Asim::Inifile("$location/regressionrc");
  $ini->include("$location/regression.d/*.regression");

  $self->{inifile} = $ini;

  return $self;
}

################################################################

=item $nightly-E<gt>set_program($program)

Set the program

=cut

################################################################

sub set_program {
  my $self = shift;
  my $program = shift;

  $self->{program} = $program;

  return 1;
}

################################################################

=item $nightly-E<gt>set_mailto($mailto)

Set the mailto address

=cut

################################################################

sub set_mailto {
  my $self = shift;
  my $mailto = shift;

  # $MAILTO || $MAILASIM || "joel.emer\@intel.com";
  $self->{mailto} = $mailto;

  return 1;
}
################################################################

=item $nightly-E<gt>set_logfile($file)

Set the logfile

=cut

################################################################

sub set_logfile {
  my $self = shift;
  my $logfile = shift;

  $self->{logfile} = $logfile;

  return 1;
}

################################################################

=item $nightly-E<gt>get_param($group, $name, $default)

Get a parameter from the regressionrc file

=cut

################################################################

sub get_param {
  my $self = shift;
  my $group = shift;
  my $name = shift;
  my $default = shift;
  my $value;
  my $include;

  $value = $self->_get_rcfile_param($group, $name);

  # If not found, use default or return failure

  if (! defined($value) ) {
    if (defined($default)) {
      $value = $default;
    } else {
      $self->exit_failure("missing regressionrc parameter: [$group] $name");
    }
  }

  $self->lprint("Parameter: [$group] $name = $value");

  return $value;
}

#
# Internal utility routine to handle 'includes' and global backup
# locations for finding an rcfile value
#
sub _get_rcfile_param {
  my $self = shift;
  my $group = shift;
  my $name = shift;

  my $ini;
  my $value;

  $ini = $self->{inifile};

  $value = $ini->get($group, $name);

  # Look in INCLUDE= section for backup default...

  my $include;

  if (! defined($value)) {
    if (defined($include = $ini->get($group, "INCLUDE"))) {
      return _get_rcfile_param($include, $name);
    }
  }

  # If not found yet, then look in [GLOBAL]

  if (! defined($value)) {
    $value = $ini->get("GLOBAL", $name);
  }

  return $value;
}

################################################################

=item $nightly-E<gt>lsystem($command)


 Run a command logging it to the logfile
    TBD: log to STDOUT also

=cut

################################################################

sub lsystem {
  my $self = shift;
  my $command = shift;
  my $status;
  my $LOGFL = $self->{logfile};

  $self->lprint("$command");


  if (defined($LOGFL)) {
    $command .= " >>$LOGFL 2>&1";
  }

  $status = system("$command");
  print "Status = $status\n";
  if ($status) {
    return undef;
  }

  return 1;
}

################################################################

=item $nightly-E<gt>lprint($message)

 Print a line to STDOUT and the logfile

=cut

################################################################

sub lprint {
  my $self = shift;
  my $line = shift;
  my $LOGFL = $self->{logfile};

  print "$line\n";
  if (defined($LOGFL)) {
    system("echo \'$line\' >>$LOGFL");
  }
}


################################################################

=item $nightly-E<gt>exit_failure($error)

Report a failure and exit

=cut

################################################################

sub exit_failure {
    my $self = shift;
    my $failure = shift || "";

    $self->report_failure($failure);
    exit(1);
}

################################################################

=item $nightly-E<gt>report_error($error)

 Report a failure

=cut

################################################################

sub report_failure {
  my $self = shift;
  my $failure = shift || "";
  my $to = $self->{mailto};
  my $prog = $self->{program};
  my $LOGFL = $self->{logfile};
  my $REGRESSION = $self->{regression};

  my $attachment = "";

  $self->lprint("*************************************");
  $self->lprint("$prog failure: mailing report...");
  $self->lprint("To: $to");
  $self->lprint("Message: $failure");

  if (defined($LOGFL)) {
    $attachment = $LOGFL;
  }

  $self->send_message( $to,
                "$prog failed - $REGRESSION",
                "$prog failure - $REGRESSION: $failure",
                $attachment);
}

################################################################

=item $nightly-E<gt>report_warning($warning)

 Report a warning

=cut

################################################################

sub report_warning {
    my $self = shift;
    my $warning = shift || "";
    my $to = $self->{mailto};
    my $prog = $self->{program};
    my $LOGFL = $self->{logfile};
    my $REGRESSION = $self->{regression};

    my $attachment = "";

    $self->lprint("*************************************");
    $self->lprint("$prog warning: mailing report...");
    $self->lprint("To: $to");
    $self->lprint("Message: $warning");

    if (defined($LOGFL)) {
	$attachment = $LOGFL;
    }

    $self->send_message( $to,
		  "$prog warning - $REGRESSION",
		  "$prog warning - $REGRESSION: $warning",
		  $attachment);
}


################################################################

=item $nightly-E<gt>send_message($to, $subject, $message, $attachment)

Send a message

=cut

################################################################

sub send_message {
  my $self = shift;
  my $to = shift;
  my $subject = shift;
  my $message = shift;
  my $attachment = shift || "";

  my $attach_cmd;

  if ( $attachment ne "/dev/null" && -f $attachment ) {
    $attach_cmd = "-a $attachment";
  } else {
      print "Attachment bad: $attachment\n";
      $message = "WARNING: COULD NOT FIND ATTACHMENT FILE: $attachment\n\n" .
                 $message;

      $attachment = "";
  }

# The mail system is deleteing .txt (and .t) attachments...
#  system("echo \'$message\' | mutt -s \'$subject\' $attach_cmd $to");

  my $mailcommand = "echo \'$message\' | paste -s -d '\n' - $ attachment | mutt -s \'$subject\' $to";
  print "$mailcommand\n";
  system($mailcommand);
}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Asim::Nightly: Error - $message";

  return 1;
}

1;
