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

package PlotShell;
use warnings;
use strict;

use File::Spec;
use Term::ReadLine;
use Text::ParseWords;

use Asim;
use Asim::Signal;

use PlotShell::Commands;
use PlotShell::Help;

#
# Default data values
#
our $VERSION=0.1;

our $default_repositoryDB;
our $default_packageDB;
our $default_moduleDB;

our $default_package;
our $default_lock;
our $default_model;
our $default_benchmark;
our $default_module;



#
# Interrupt handling
#
Asim::enable_signal();

#$SIG{'INT'} = \&shell_abort;

#sub shell_abort {
#  my $signame = shift;
#  
#  Asim::cleanup_and_exit("...Aborting due to ^C...\n");
#}


#
# Terminal input
#
our $term;
#$attribs = $term->Attribs;
#$attribs->{attempted_completion_function} = \&attempted_completion;


# our $OUT = $term->OUT || STDOUT;


#
# Parsing
#
our %COMPOUNDCOMMANDS = 
  ( help      => [ ],
    list      => [ qw(params experiments benchmarks histograms scalars groups restrictions mappings stats plot) ], 
    list_stats=> [ qw(options) ],
    list_plot => [ qw(options legendloc dimensions) ],
    get       => [ qw(data) ],
    set       => [ qw(basedir experiments benchmarks group params runstyle print plot) ],
    set_plot  => [ qw(legendloc width height ymin ymax output landscape portrait type title ylable xlabel) ],
    plot      => [ qw(data ucurve scurve) ],
    unrestrict=> [ qw(params experiments benchmarks) ],
    restrict  => [ qw(params experiments benchmarks) ],
    group     => [ qw(params experiments benchmarks) ],
    sort      => [ qw(params benchmarks experiments) ],
    define    => [ qw(params benchmarks) ],
    foreach   => [ ],
    dump      => [ qw(history) ],
    print     => [ qw(data restricted) ],
    print_restricted=> [ qw(data) ],
  );


our @COMMANDS= ( keys %COMPOUNDCOMMANDS, 
                 qw(cd awb status help exit quit)
               );


=head1 NAME

PlotShell - Interactive shell to grab, print and plot ASIM data

=head1 SYNOPSIS

use PlotShell;

PlotShell::init();
PlotShell::shell();


=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item PlotShell::init();

=cut

################################################################

sub init {
  my $runmode = shift || "interactive";

  if ($runmode eq "interactive") {
    mode_interactive();
  } else {
    mode_batch();
  }

  return 1;
}

################################################################

=item PlotShell::shell();

Run an interactive shell to perform data extraction activites.
Type help to see available commands.

=cut

################################################################


sub shell {
  my $prompt = "plot> ";
  my $command = "";

  # Force interactive mode in case it hasn't been set

  mode_interactive();


SHELLCOMMAND:
  
  while () {

    $term->Attribs->{attempted_completion_function} = \&attempted_completion;
    $_ = $term->readline($prompt) ;

    last SHELLCOMMAND unless defined $_;
    chomp;

    #
    # Strip leading/trailing whitespace
    #
    s/^\s+//;
    s/\s+$//;

    #
    # If we end with a '\', then this is a multi-line command
    #
    if (/^([\s\S]*)\\/) {
      $command = "$command" . "$1";
      $prompt = "";
      next SHELLCOMMAND;
    }
    else {
      $prompt = "plot> ";
      $command = "$command" . "$_";
    }

    #
    # Skip empty commands and commands that start with #
    #
    if ( $command =~ /^$/ || 
	$command =~ /^\s*#/ ) {
      $command = "";
      next SHELLCOMMAND;
    }

    #
    # Translate '?' and 'h' to 'help'
    #
    if (($command =~ /^\?/) || ($command =~ /^h$/)) {
      $command = 'help';
    }

    #
    # Parse exit commands
    #
    if ($command =~ /^(?:q(?:uit)?|bye|exit)$/i) {
      last SHELLCOMMAND;
    }
    
    #
    # Parse shell escape
    #
    if ($command =~ /^\!/) {
      $command =~ s/^\!//;
      my $eval = $command;
      eval {print `$eval` };
      shell_error($@) if $@;
      $command = "";
      next SHELLCOMMAND;
    }

    #
    # Normal command
    #
    my @line = shellwords($command);
    run_command(@line);
    $command = "";

  }

  return 1;
}

#
# Utility function to help parsing
#
sub attempted_completion {
  my ($text, $line, $start, $end) = @_;
  my $prefix;
  my @list;

  $prefix = substr($line, 0, $start);
  if ($prefix =~ /^\s*\w+\s+\w+\s*/) {
    $prefix =~ s/^\s*(\w+)\s*(\w+)\s*/$1_$2/;
  }
  else {
    $prefix =~ s/^\s*//;
  }

  $line =~ s/^\s*//;

  if ($start == 0) {
    #
    # Handle first symbol of command
    #
    @list = grep /^$text[a-zA-Z]+$/, @COMMANDS;
    return ("", @list);

  } elsif ( $prefix =~ /^(\w+)\s*$/) {

    #
    # Handle second symbol of command (if first is a legal keyword)
    #
    if (defined($COMPOUNDCOMMANDS{$1})) {
      my @subcommands = @{$COMPOUNDCOMMANDS{$1}};
      @list = grep /^$text/, @subcommands;
    } else {
      @list = ();
    }
    return ("", @list);

  } elsif (0) {
    #
    # Just a model for other things to do...
    #
    return $term->completion_matches($text,
                                     $term->Attribs->{'username_completion_function'});

  } elsif (0) {
    #
    # Another  model for other things to do...
    #
    $term->Attribs->{completion_word} = [qw(help test)];
    return $term->completion_matches($text, 
                                     $term->Attribs->{'list_completion_function'});

  } else {
    #
    # Default to filename completion
    #
    return ();
  }
}


################################################################

=item PlotShell::run_command($command [$argument...]);

Run plot-shell command $commands with optional arguments $argument...
See PlotShell::help() to see list of available commands.

=cut

################################################################


sub run_command {
  my $command = shift;
  my @line = @_;
  my $status;

  #
  # Combine arguments until you find a working command
  #
  $command = &translate_key($command);
  while (! defined(&$command)) {
    if ( defined($line[0]) ) {
      $command = $command . "_" . &translate_key(shift @line);
    }
    else {
      shell_error("No such command: $command\n");
      return 0;
    }
  }

  #
  # Run it --- cancelling strict 'refs'...
  #
  no strict 'refs';

  eval { $status = &${command}(@line) };
  if ($@) {
    print "plot-shell: command trapped - $@";
    $status = 0;
  }

  #
  # Last chance to abort stuff - but it should have happened sooner...
  #
  if (! $status) {
    Asim::Xaction::abort();
  }

  return $status;
}


#
# Utility that helps translate from some possible user spellings. 
#
sub translate_key {
  my $key = shift;
  if ($key =~ /^par.*$/i) {
    return ("params");
  }
  elsif ($key =~ /^exp.*$/i) {
    return ("experiments");
  }
  elsif ($key =~ /^bench.*|^bm.*$/i) {
    return ("benchmarks");
  }
  elsif ($key =~ /^hist$/i) {
    return ("histograms");
  }
  elsif ($key =~ /^histogram.*/i) {
    return ("histograms");
  }
  elsif ($key =~ /^scalar.*/i) {
    return ("scalars");
  }
  else {
    return ("$key");
  }
}

# Mode functions

sub mode_batch {

  Asim::mode("Batch");

  return 1;
}

sub mode_interactive {

  Asim::mode("Interactive");
  if (!defined($term) ) {
    $term = Term::ReadLine->new('Asim Shell');;
  }

  return 1;
}

# Help is implemented in PlotShell::Help


sub shell_error {
  my $message = shift;

  print "Plot-shell: FAILURE - $message";
  return 1;
}  


=back

=head1 More Information

Additional information on the commands can be found in Help.pm.

=head1 BUGS

#Setting a new workspace does not get a new version of asim-shell...
#maybe we need to do an exec or re-resolve everything...

#Need to automatically scan to create a default moduleDB. 

#Many command line completion ideosyncracies: Command line completion
#should complete up to greatest common unique character. It should be
#able to do command line completion using Asim UnionDir path. And it
#should be able to do command specific completion.

#I cannot decide if run_command should expect a parsed command line or not...

#It would be better if delete_package checked for changed or added files
#in the package before going ahead and deleting it. When checkout_package
#sees that the target directory already exists it does not check for
#changed or added files either.

#You need to already be running asim-shell to see the help_getting_started
#information...

#A lot more error checking is needed.

#After running a unix shell under the asim-shell the program gets suspended!

=head1 AUTHORS

Srilatha Manne & Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
