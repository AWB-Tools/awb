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

package AsimShell;
use warnings;
use strict;

use File::Spec;
use Term::ReadLine;
use Text::ParseWords;

use Asim;
use Asim::Signal;

use AsimShell::Completion;
use AsimShell::Commands;
use AsimShell::Help;

#
# Control variables
#
our $show_warnings;

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
  ( add       => [ qw(public_package package) ],
    baseline  => [ qw(packages) ],
    branch    => [ qw(package) ],
    build     => [ qw(model package) ],
    cd        => [ qw(model package) ],
    checkout  => [ qw(bundle package) ],
    clone     => [ qw(workspace) ],
    commit    => [ qw(package) ],
    configure => [ qw(model package) ],
    clean     => [ qw(model package regression) ],
    create    => [ qw(repository workspace package model module lock) ],
    cvs       => [ qw(package) ],
    delete    => [ qw(lock package) ],
    edit      => [ qw(workspace package model module) ],
    help      => [ qw(abstractions code commands examples quickstart) ],
    install   => [ qw(package) ],
    list      => [ qw(bundles repositories packages locks models modules) ],
    lock      => [ qw(package lock) ],
    make      => [ qw(model package) ],
    merge     => [ qw(package) ],
    mode      => [ qw(batch interactive) ],
    new       => [ qw(repository workspace package model module lock) ],
    nuke      => [ qw(model) ],
    setup     => [ qw(model) ],
    set       => [ qw(repositoryDB workspace package lock model benchmark module) ],
    shell     => [ qw(package) ],
    status    => [ qw(package) ],
    rehash    => [ qw(repositories locks packages models modules workspace) ],
    regtest   => [ qw(package) ],
    release   => [ qw(package) ],
    show      => [ qw(bundle workspace package lock model module repository) ],
    run       => [ qw(model regression) ],
    unlock    => [ qw(package lock) ],
    unset     => [ qw(package lock model module) ],
    update    => [ qw(bundle package) ],
    use       => [ qw(bundle package) ],
    verify    => [ qw(regression) ],
    svn       => [ qw(package) ],
  );


our @COMMANDS= ( keys %COMPOUNDCOMMANDS, 
                 qw(pwd ls awb help exit quit)
               );

#
# These should be of a form the GetOptions can use directly!!!
#
our %OPTIONS = (
    clone_workspace    => [ "--link" ],

    checkout_bundle    => [ "--user=", "--user", "--build", "--nobuild", 
                            "--addpath", "--noaddpath", "--golden" ],
    use_bundle         => [ "--build", "--nobuild", "--addpath", "--noaddpath", "--golden" ],

    checkout_package   => [ "--user=", "--user", "--build", "--nobuild", 
                            "--addpath", "--noaddpath" ],
    use_package        => [ "--build", "--nobuild", "--addpath", "--noaddpath" ],
    commit_package     => [ "--dependent", "--nodependent", "--commitlog" ],
    status_package     => [ "--verbose", "--noverbose" ],
    update_package     => [ "--build", "--nobuild" ],
    install_package    => [ "--source" ],

    clean_model        => [ "--builddir" ],
    nuke_model         => [ "--builddir" ],
    ncfg_model         => [ "--builddir" ],
    configure_model    => [ "--builddir" ],
    build_model        => [ "--builddir", "--buildopt", "--options" ],
    setup_model        => [ "--builddir", "--rundir" ],
    run_model          => [ "--builddir", "--rundir", "--runopt", "--options" ],

    # Most of these switches go directly to regression.launcher command (need to manually synced)

    run_regression     => [ "--regdir",
                            "--runtype=regression", "--runtype=nightly",
                            "--runstyle=pruned", "--runstyle=list",
                            "--runcmds",
                            "--compress=none", "--compress=gzip", "--compress=bzip2",
                            "-queue",
                            "--remote", "--noremote", "--remoteroot", "--remoteregdir",
                            "--rerun",
                            "--dryrun", "--nodryrun",
                            "--help", "--options" ],
  );


=head1 NAME

AsimShell - Interactive shell to manipulate Asim objects

=head1 SYNOPSIS

use Asim;
use AsimShell;

Asim::init();
AsimShell::init();
AsimShell::shell();


=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item AsimShell::init([mode => batch|interactive]
                      [warning => 0|1]);


Initialize the shell, optionally setting the mode to
interactive or batch, and turing printing of warnings
on or off.

=cut

################################################################

sub init {
  my %switches = @_;

  # Configure mode

  my $runmode = $switches{mode} || "interactive";

  if ($runmode eq "interactive") {
    mode_interactive();
  } else {
    mode_batch();
  }

  # configure warnings

  $show_warnings = 1;
  if (defined($switches{warnings})) {
    $show_warnings = $switches{warnings};
  }

  return 1;
}

################################################################

=item AsimShell::shell();

Run an interactive shell to perform Asim-related activites.
Type help to see available commands.

=cut

################################################################


sub shell {
  my $prompt = "asim> ";

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
    # Skip empty commands
    #
    next SHELLCOMMAND if /^$/;

    #
    # Translate '?' and 'h' to 'help'
    #
    $_ = 'help' if /^\?/;
    $_ = 'help' if /^h$/;

    #
    # Parse exit commands
    #
    if (/^(?:q(?:uit)?|bye|exit)$/i) {
      last SHELLCOMMAND;
    }

    #
    # Parse shell escape
    #
    if (/^\!/) {
      s/^\!//;
      my $eval = $_;
      eval {print `$eval` };
      shell_error($@) if $@;
      next SHELLCOMMAND;
    }

    #
    # Normal command
    #
    my @line = shellwords($_);
    run_command(@line);

  }

  return 1;
}


################################################################

=item AsimShell::run_command($command [$argument...]);

Run asim shell command $commands with optional arguments $argument...
See AsimShell::help() to see list of available commands.

=cut

################################################################


sub run_command {
  my $command = shift;
  my @line = @_;
  my $status;

  #
  # Do nothing if there isn't a command
  #
  if (! defined($command) || ($command =~ /^\s*$/)) {
    return 1;
  }

  #
  # Check if next argument combines to form a compound command
  #
  if ( defined($line[0]) && defined(&{$command . "_" . $line[0]})) {
    $command = $command . "_" . shift @line;
  }

  #
  # Check if command really exists
  #
  if ( ! defined(&$command)) {
    shell_error("No such command: $command\n");
    return 0;
  }

  #
  # Run it --- cancelling strict 'refs'...
  #
  no strict 'refs';

  eval { $status = &${command}(@line) };
  if ($@) {
    print "asim-shell: command trapped - $@";
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


# Mode functions

sub mode_batch {

  Asim::mode("Batch");

  return 1;
}

sub mode_interactive {

  if (!defined($term) ) {
    $term = Term::ReadLine->new('AsimShell');;
    ##FIXME 11/12/03 Mark Charney:  disabled, was not ssh2/ssh-agent2 friendly
    ## check_ssh_agent();
  }

  Asim::mode("Interactive");

  return 1;
}

sub check_ssh_agent {
  if (Asim::is_ssh_agent_running() != 0) {
    print "An ssh-agent does not appear to be running.\n";
    print "Asim-shell might require you to type your ssh passphrase many times.\n";
    print "An ssh-agent could avoid having to retype the passphrase.\n";
    if (Asim::choose_yes_or_no("Do you want to start a temporary ssh-agent now", "yes", "no")) {
      Asim::start_ssh_agent();
    }
  }
}


# Help is implemented in AsimShell::Help


sub shell_error {
  my $message = shift;

  print "Asim-shell: FAILURE - $message";
  return 1;
}  



=back
=head1 More Information

Additional information on the commands can be found in Help.pm.

=head1 BUGS

Setting a new workspace does not get a new version of asim-shell...
maybe we need to do an exec or re-resolve everything...

Need to automatically scan to create a default moduleDB. 

Many command line completion ideosyncracies: Command line completion
should complete up to greatest common unique character. It should be
able to do command line completion using Asim UnionDir path. And it
should be able to do command specific completion.

I cannot decide if run_command should expect a parsed command line or not...

It would be better if delete_package checked for changed or added files
in the package before going ahead and deleting it. When checkout_package
sees that the target directory already exists it does not check for
changed or added files either.

Update package all does not honor package dependencies. The order is
simply alphabetical that could lead to out-of-order builds. Things are
working okay for now since 'core' is coming first.

You need to already be running asim-shell to see the help_getting_started
information...

A lot more error checking is needed.

After running a unix shell under the asim-shell the program gets suspended!

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
