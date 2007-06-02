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

package Asim::Edit::Interactive;
use warnings;
use strict;

our $debug = 0;

=head1 NAME

Asim::Edit::Interactive - command line based user interactive module

=head1 SYNOPSIS

use Asim::Edit::Interactive;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $editcontrol = Asim::Edit::Interactive()

Create a new editcontrol object.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  $self->{term} = $AsimShell::term;

  return $self;
}



################################################################

=item $editcontrol-E<gt>mode()

Return mode of this edit control.

=cut

################################################################


sub mode {
  my $self = shift;

  return "interactive";

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

  my $term = $self->{term};

  $input = $term->readline("$prompt [$default]: ") || $default;
  chomp $input;

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
  our @OPTIONS;

  my $term = $self->{term};

  $prompt = "$question [$default]?: ";

  while (1) {
    @OPTIONS = ("yes", "no");
    $term->Attribs->{attempted_completion_function} = \&list_completion;
    $input = $term->readline($prompt) || $default;
    chomp $input;

    if (!defined($input)) {
      return undef;
    }

    if ($input eq "") {
      $input = $default;
    }

    if ($input =~ /[nN][oO]?/) {
      return 0;
    } elsif ($input =~ /[Yy](es)?/) {
      return 1;
    } elsif ($input eq "?") {
      print "Just enter yes or no\n";
    }
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

  my $term = $self->{term};

  my $index = 1;
  my $default_index;

  my $input;
  my $answer;

  our @OPTIONS = ();
  my %option_map;

  # No default yet...
  $default_index = "";

  #
  # List alternatives
  #
  foreach my $m (@choices) {

    my $name = ref($m)?($m->name()):$m;

    push(@OPTIONS, $name);
    $option_map{"$name"} = $index-1;

    print "[" . $index++ . "] " .  $name . "\n";
  }

  while (1) {

    $term->Attribs->{attempted_completion_function} = \&list_completion;
    $input = $term->readline("$prompt [$default_index]: ");

    #
    # Exit on EOF
    #
    if (! defined($input)) {
      return undef;
    }

    #
    # Crop out whitespace
    #
    $input =~ s/^\s+//;
    $input =~ s/\s+$//;

    #
    # Got something, so check if number or matching string
    #
    if ( $input =~ /^\d+$/) {
      $answer = $input-1;
    } elsif ( defined($option_map{"$input"}) ) {
      $answer = $option_map{"$input"};
    } elsif ( $input eq "?") {
      print "Enter a number of an entry from the list\n";
      print "or, the full text of an entry - command completion is available\n";
      next;
    } else {
      print "Not a valid answer - for help type \'?\'\n";
      next;
    }
    #
    # Return legal answer
    #
    if (defined($choices[$answer])) {
      return ref($choices[$answer])?($choices[$answer]):$answer;
    }
  }
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
  my $term = $self->{term};


  $term->Attribs->{attempted_completion_function} = \&file_completion;
  $input = $term->readline("$prompt [$default]: ") || $default;
  chomp $input;

  if (! defined($input)) {
    return undef;
  }

  # Remove leading/trailing whitespace...
  $input =~ s/^\s+//;
  $input =~ s/\s+$//;

  return $input;
}



###############################################################
#
# Some standard input completion functions
#
###############################################################

sub file_completion {
  return ();
}


sub list_completion {
  my ($text, $line, $start, $end) = @_;
  my @list;

  our @OPTIONS;

  @list = grep /^$text/, @OPTIONS;
  return (max_common($text, @list), @list);
}



###############################################################
#
# A function to find a max match - 
#
# Find the maximum common extension of $start in @list, i.e.,
# if all the members of @list (which already start with the
# string $start) continue with the same letter (or letters)
# return the string starting with $start and extended with 
# that set of common letters.
#
#
###############################################################

sub max_common {
  my $start = shift;
  my $start_length = length($start);
  my @list = @_;

  #
  # If there is nothing in the list
  # there is nothing to do...
  #
  if (!@list) {
    return $start;
  }

  #
  # If there is only one entry in the list
  # then return that element...
  #
  if ($#list == 0) {
    return $list[0];
  }
  
  #
  # Return if first string in @list is not
  # longer then $start string
  # 
  if (length($list[0]) <= $start_length) {
    return $start;
  }

  #
  # If all members of the list have a common
  # next character we extend the match by 
  # that letter.
  #
  my $c = substr($list[0], $start_length, 1);

  foreach my $s (@list) {
    if (length($s) == $start_length) {
      return $start;
    }

    if (substr($s,$start_length,1) ne $c) {
      return $start;
    }
  }

  #
  # We've extended the match by one letter,
  # so we try for one more.
  #
  return max_common($start . $c, @list);
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

  TODO: The dance around $term->Attrib is really ugly

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

# Make perl happy...

1;
