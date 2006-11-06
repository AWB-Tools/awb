#
# *****************************************************************************
# *
# * @brief GenCFG.pm : helper functions for building executable CFG file
# *                    generators.
# *
# * @author Michael Adler
# *
# Copyright (C) 2006 Intel Corporation
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

package Asim::GenCFG;
use warnings;
use strict;


our $DEBUG = ($ENV{ASIM_DEBUG} || 0) >= 2;

=head1 NAME

Asim::GenCFG - A set of subroutines for building workload CFG data from a script.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides a set of methods to generate workload CFG data
from a script.  A typical caller will create a list of workloads using add()
and then call the action() method, passing @ARGV.

=over 4

=cut

################################################################

=item $cfg = Asim::GenCFG-E<gt>new()

Create a new GenCFG object.

=cut

################################################################


sub new {
    my $this = shift;
    my $class = ref($this) || $this;
    my $self = {};

    bless $self, $class;

    return $self;
}


################################################################

=item $cfg-E<gt>add(name => <name>, ...)

Add a workload.

=over 4

=item name

Required:  name of the workload.

=item tree

An array describing the position of the workload in a hierarchy.  The
full name of a workload looks like a directory/file path using the
entries from the tree as directories and the workload name as the leaf.

=item info

Workload description.

=item feeder

Feeder type.

=item setup

Setup script and arguments.

=item genflags

General flags.

=item feedflags

Flags for the feeder.

=item sysflags

System flags.

=item Controller script:

The following arguments define the script to be passed to the controller.
Specify at most one controller script argument.  The "icount" and "ccount" arguments
offer an easy way to specify a script that runs for a defined length and
emits statistics on exit.  The "ctrl" argument allows specification of
arbitrary scripts.  If no argument is specified simulations will run until
the feeder forces an exit.

=over 4

=item icount

Run for the specified number of instructions, emit statistics and exit.

=item ccount

Run for the specified number of cycles, emit statistics and exit.

=item ctrl

An array of commands for the controller.

=back 4

=back 4

=cut

################################################################

sub add() {
    my $self = shift;
    my %param = @_;

    die("Workloads must have a 'name' parameter") if (! defined($param{name}));

    ##
    ## Define an error name in case of problems...
    ##
    my $errName = '';
    if (defined($param{tree})) {
        my @tree = @{$param{tree}};
        foreach my $i (0 .. $#tree) {
            $errName .= "$tree[$i]/";
        }
    }
    $errName .= "$param{name}.cfg";

    ##
    ## Hierarchy of workloads is stored in a hash starting from "this" (the
    ## root) in a fields named "h".
    ##
    ## Start by walking the tree to find the right level.
    ##

    my $h = $self;
    if (defined($param{tree})) {
        my @tree = @{$param{tree}};
        foreach my $i (0 .. $#tree) {
            if (! defined($h->{h}{$tree[$i]})) {
                # First time the level has been seen in the hierarchy.  Add
                # a new instance of the GenCFG tree.  Functions here will
                # walk it recursively.
                $h->{h}{$tree[$i]} = Asim::GenCFG->new();
            }
            $h = $h->{h}{$tree[$i]};
        }
    }

    ##
    ## $h now points to the right level.  Add an entry for the new workload
    ## under the field named "w".
    ##

    if (defined($h->{w}{$param{name}})) {
        die("Multiple definitions for $errName");
    }
    my $w = \%{$h->{w}{$param{name}}};

    ##
    ## $w now points to the workload details.  Populate the descriptor.
    ## Make sure that all fields are defined here so that other functions
    ## don't need to check whether a field is defined.
    ##
    $$w{info} = val_or_null($param{info});
    $$w{info} = $param{name} if ($$w{info} eq '');

    $$w{feeder} = val_or_err($param{feeder}, 'feeder', $errName);
    $$w{setup} = val_or_err($param{setup}, 'setup', $errName);
    $$w{genflags} = val_or_null($param{genflags});
    $$w{feedflags} = val_or_null($param{feedflags});
    $$w{sysflags} = val_or_null($param{sysflags});

    $$w{name} = $param{name};
    if ($$w{genflags} =~ /--queryregions/) {
        $$w{name} .= '%R';
    }

    ## Generate a controller script
    if (defined($param{ctrl})) {
        $$w{ctrl} = $param{ctrl};
    }
    elsif (defined($param{icount})) {
        $$w{ctrl} = [ "AwbStats dumponexit $$w{name}.stats",
                      "AwbStats on",
                      "AwbRun inst $param{icount}",
                      "AwbExit" ];
    }
    elsif (defined($param{ccount})) {
        $$w{ctrl} = [ "AwbStats dumponexit $$w{name}.stats",
                      "AwbStats on",
                      "AwbRun cycle $param{ccount}",
                      "AwbExit" ];
    }
    else {
        $$w{ctrl} = [ "AwbStats dumponexit $$w{name}.stats",
                      "AwbStats on",
                      "AwbRun inst -1",
                      "AwbExit" ];
    }
}


################################################################

=item $cfg-E<gt>emit(path)

Emit a CFG file.

=over 4

=item path

The full path to a workload.

=back 4

=cut

################################################################

sub emit {
    my $self = shift;
    my $path = shift;

    # Break path into components
    die("No path specified to emit()") if ($path eq '');
    die("Path must end in .cfg") if (! ($path =~ /\.cfg$/));
    my @p = split('/', $path);

    my $cfgName = $p[$#p];
    $#p -= 1;
    $cfgName =~ s/\.cfg$//;

    # Walk the "directory" entries
    my $h = $self;
    my $dbgName = '';
    foreach my $i (0 .. $#p) {
        $dbgName = "${dbgName}$p[$i]/";
        die("Failed to find $dbgName in hierarchy") if (! defined($h->{h}{$p[$i]}));
        $h = $h->{h}{$p[$i]};
    }

    # Now at the right node ($h).  Look for the workload.
    die("${dbgName}${cfgName}.cfg is not defined") if (! defined($h->{w}{$cfgName}));
    my $wrk = \%{$h->{w}{$cfgName}};

    # Emit the CFG file
    print "BmAdd {\n";
    print "  $$wrk{name}\n";
    print "  $$wrk{name}\n";
    print "  {$$wrk{info}}\n";
    print "  $$wrk{feeder}\n";
    print "  $$wrk{setup}\n";
    print "  {$$wrk{genflags}}\n";
    print "  {$$wrk{feedflags}}\n";
    print "  {$$wrk{sysflags}}\n";
    print "  {\n";
    foreach my $c (0 .. $#{$$wrk{ctrl}}) {
        print "    $$wrk{ctrl}[$c]\n";
    }
    print "  }\n";
    print "}\n";
}


################################################################

=item $cfg-E<gt>enum(path)

Enumerate all CFG files at path and below.

=over 4

=item path

The root of the path to search.

=back 4

=cut

################################################################

sub enum {
    my $self = shift;
    my $path = shift;
    $path = '' if (! defined($path));

    # Walk the "directory" entries
    my @p = split('/', $path);
    my $h = $self;
    my $dbgName = '';
    foreach my $i (0 .. $#p) {
        $dbgName = "${dbgName}$p[$i]/";
        die("Failed to find $dbgName in hierarchy") if (! defined($h->{h}{$p[$i]}));
        $h = $h->{h}{$p[$i]};
    }

    $h->enum_r($dbgName);
}

################################################################

=item $cfg-E<gt>dir(path)

List level immediately below path.

=over 4

=item path

The root of the path to search.

=back 4

=cut

################################################################

sub dir {
    my $self = shift;
    my $path = shift;
    $path = '' if (! defined($path));

    # Walk the "directory" entries
    my @p = split('/', $path);
    my $h = $self;
    my $prefix = '';
    foreach my $i (0 .. $#p) {
        $prefix = "${prefix}$p[$i]/";
        die("Failed to find $prefix in hierarchy") if (! defined($h->{h}{$p[$i]}));
        $h = $h->{h}{$p[$i]};
    }

    if (defined($h->{w})) {
        foreach my $w (sort {"$a" cmp "$b"} keys %{$h->{w}}) {
            my $wrk = \%{$h->{w}{$w}};
            print "${prefix}${w}.cfg\n";
        }
    }

    if (defined($h->{h})) {
        foreach my $h (sort {"$a" cmp "$b"} keys %{$h->{h}}) {
            print "${prefix}${h}\n";
        }
    }
}

################################################################

=item $cfg-E<gt>action(@args)

Parse arguments and take action.  Arguments can be:

=over 4

=item --emit <cfg>

Emit a CFG file.

=item --list <root>

List all CFG files below root.

=item --dir <root>

List level below root.

=back 4

=cut

################################################################

sub action {
    my $self = shift;
    my @args = @_;

    usage() if ($#args < 0);
    if ($args[0] eq '--emit') {
        usage() if ($#args < 1);
        $self->emit($args[1]);
    }
    elsif ($args[0] eq '--list') {
        $self->enum($args[1]);
    }
    elsif ($args[0] eq '--dir') {
        $self->dir($args[1]);
    }
    else {
        usage();
    }
}

################################################################

##
## Helper functions
##

sub usage {
    print STDERR "Usage:  GenCFG [--emit <CFG>] [--list <path>] [--dir <path>]\n\n";
    print STDERR "    Must specify one of --emit, --list or --dir\n\n";
    print STDERR "    --emit <CFG>\n";
    print STDERR "      Emit CFG file.\n\n";
    print STDERR "    --list <path>\n";
    print STDERR "      List all available CFG files under path.\n";
    print STDERR "    --dir <path>\n";
    print STDERR "      Treat path like a directory and list the level below it.\n";
    exit(-1);
}


##
## enum_r -- private subroutine to implement enum() walk.
##
sub enum_r {
    my $self = shift;
    my $prefix = shift;

    $prefix = '' if (! defined($prefix));

    if (defined($self->{w})) {
        foreach my $w (sort {"$a" cmp "$b"} keys %{$self->{w}}) {
            my $wrk = \%{$self->{w}{$w}};
            print "${prefix}${w}.cfg\n";
        }
    }

    if (defined($self->{h})) {
        foreach my $h (sort {"$a" cmp "$b"} keys %{$self->{h}}) {
            $self->{h}{$h}->enum_r("${prefix}${h}/");
        }
    }
}

sub val_or_null($) {
    my $v = shift;

    if (defined($v)) {
        return $v;
    }
    else {
        return '';
    }
}


sub val_or_err($$$) {
    my $v = shift;
    my $n = shift;
    my $err = shift;

    if (defined($v)) {
        return $v;
    }
    else {
        die("$err:  must define field '$n'");
    }
}


################################################################

=back

=head1 AUTHORS

Michael Adler

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2006

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;

