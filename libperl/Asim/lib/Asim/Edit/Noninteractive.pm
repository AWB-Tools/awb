#
# Copyright (C) 2003-2006 Intel Corporation
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

package Asim::Edit::Noninteractive;
use warnings;
use strict;

our $debug = 0;

=head1 NAME

Asim::Edit::Noninteractive - command line based user interactive module

=head1 SYNOPSIS

use Asim::Edit::Noninteractive;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $editcontrol = Asim::Edit::Noninteractive()

Create a new editcontrol object.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  return $self;
}



################################################################

=item $editcontrol-E<gt>mode()

Return mode of this edit control.


=cut

################################################################


sub mode {
  my $self = shift;

  return "batch";

}


################################################################

=item $editcontrol-E<gt>choose($prompt, $default, $forced_default)


=cut

################################################################


sub choose {
  my $self = shift;
  my $prompt = shift;
  my $default = shift || "";
  my $forced_default = shift || $default;

  my $input;

  print "$prompt [$default]: $forced_default\n";
  $input = $forced_default;

  return $input;
}



################################################################

=item $editcontrol-E<gt>choose_yes_or_no($question, $default, $forced_default)


=cut

################################################################


sub choose_yes_or_no {
  my $self = shift;
  my $question = shift;
  my $default = shift || "no";
  my $forced_default = shift || $default;

  my $prompt;
  my $input;

  $prompt = "$question [$default]?: ";

  print "$prompt $forced_default\n";
  $input = $forced_default;

  if ($input =~ /[nN][oO]?/) {
    return 0;
  } elsif ($input =~ /[Yy](es)?/) {
    return 1;
  } else {
    die("Default for choose_yes_no must be yes or no\n");
  }
}


################################################################

=item $editcontrol-E<gt>choose_name($prompt, @choices)


=cut

################################################################

sub choose_name {
  my $self = shift;
  my $prompt = shift;
  my @choices = @_;

  my $index = $self->choose_from_list($prompt, @choices);

  if (! defined($index)) {
    return undef;
  }

  return $choices[$index];
}


################################################################

=item $editcontrol-E<gt>choose_from_list($prompt, @choices)

=cut

################################################################

sub choose_from_list {
  my $self = shift;
  my $prompt = shift;
  my @choices = @_;

  die("No noninteractive version of choose_from_list()\n");
}

################################################################

=item $editcontrol-E<gt>choose_filename($prompt, [$default])


=cut

################################################################


sub choose_filename {
  my $self = shift;
  my $prompt = shift;
  my $default = shift || "";

  my $input;

  print "$prompt [$default]: $default\n";
  $input = $default;

  # Remove leading/trailing whitespace...
  $input =~ s/^\s+//;
  $input =~ s/\s+$//;

  return $input;
}




################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Asim::Module:: Error - $message";

  return 1;
}

=back

=head1 BUGS


=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

# Make perl happy...

1;
