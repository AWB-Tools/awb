#
# *****************************************************************************
# *
# * @brief BuildTree/Configuration.pm  
# *
# * @author Carl Beckmann
# *
# * Copyright (c) 2008 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::BuildTree::Configuration;
use warnings;
use strict;
use Asim::Util;

# the kinds of info we can hope to get from get_item() calls:
our @ItemList = qw(
    machine os os_release os_release2 cxx_compiler cxx_compiler_version
);

=head1 NAME

Asim::BuildTree::Configuration - Class to query a build tree's configuration information

=head1 SYNOPSIS

use Asim::BuildTree;
use Asim::BuildTree::Configuration;

my $cfg = $build_tree->get_configuration();


=head1 DESCRIPTION

This object class is used to query information about the output of running "configure"
or its equivalent on a source tree.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $cfg = Asim::BuildTree::Configuration-E<gt>new($location);

Create a new build tree configuration object.
The mandatory argument is the location of the build tree (the root of the directory tree).

=cut

################################################################

sub new {
    my $this = shift;
    my $class = ref($this) || $this;
    my $self = {};

    bless	$self, $class;

    # defaults
    $self->{location} = shift;
    $self->{type} = 'unknown';
    
    # try parsing config.log autoconf output if it exists:
    $self->parse_autoconf_config_log($self->{location} . '/config.log');

    return $self;
}

#
# internal routine to parse autoconf config.log file.
# Returns 1 if config.log exists and was parsed correctly,
# and sets the type to autoconf.
#
sub parse_autoconf_config_log {
    my $self = shift;
    my $filename = shift;
    if (! -e $filename) {return 0};
    open CONFIGLOG, $filename || return 0;

    # if config.log exists, set type to autoconf, and parse the file:
    my $state = 'BEGIN';
    my %variables;
    while ( <CONFIGLOG> ) {
        chomp;

        # parse the "Platform" section
        if ($state eq 'Platform') {
            if      (m/^uname\s+\-m\s*=\s*([0-9A-Za-z_]+)/) {
                $self->{machine} = $1;
            } elsif (m/^uname\s+\-s\s*=\s*([0-9A-Za-z_]+)/) {
                $self->{os}  = $1;
            } elsif (m/^uname\s+\-r\s*=\s*(.*)/) {
                ($self->{os_release} = $1) =~ s/\s*$//;
                if ($self->{os_release} =~ m/^([0-9]+\.[0-9]+)/) {
                    $self->{os_release2} = $1;
                }
#           } elsif (m/^uname\s+\-v\s*=\s*([0-9A-Za-z_]+)/) {
#               $self->{os_version} = $1;
            }

        # parse the "Core tests" section
        } elsif ($state eq 'Core tests') {
            if (m/^configure:[0-9]+:\s*checking\s+for\s+(.*)\s*$/) {
                # FIX! these work for GCC but probably nothing else!!
                my $check_for_item = $1;
                if ($check_for_item eq 'g++') {
                    $_ = <CONFIGLOG>;
                    $_ = <CONFIGLOG>;
                    if (m/^configure:[0-9]+:\s*result:\s+(.*)\s*$/) {
                        $self->{cxx_compiler} = $1;
                    }
                } elsif ($check_for_item eq 'C++ compiler version') {
                    $_ = <CONFIGLOG>;
                    $_ = <CONFIGLOG>;
                    if (m/\(GCC\)\s*([0-9][0-9\.]*)/) {
                        $self->{cxx_compiler_version} = $1;
                    }
                }
            }

        # parse the "Output variables" section
        } elsif ($state eq 'Output variables') {
            if (m/^([0-9A-Za-z_]+)\s*=\s*\'(.*)\'/) {
                $variables{$1} = $2;
            }
        }

        # state changes:
        if (m/^\#\#\s*([^\.]+)\.\s*\#\#/) {
            $state = $1;
        }
    }
    close CONFIGLOG;

    # on success, set the type and the log file name, and return 1:
    $self->{type} = 'autoconf';
    $self->{config_log} = $filename;
    $self->{variables} = \%variables;
    return 1;
}

################################################################

=item $cfg-E<gt>get_item($name)

Get various core configuration items.
The following values for $name are currently recognized:

=over 4

=item machine

Get the machine architecture of the host that configure was run on,
which is usually the output of "uname -m",
e.g. "i386", "x86_64", etc.

=item os

Get the operating system kernel of the host that configure was run on,
which is usually the output of "uname -s",
e.g. "Linux"

=item os_release

Get the operating system release of the host that configure was run on,
which is usually the output of "uname -r",
e.g. "2.6.5-7.276.PTF.196309.1-smp"

=item os_release2

Same as above, but limited to a major and minor release number, e.g. "2.6"

=item cxx_compiler

Get the name of the C++ compiler, e.g. "gcc"

=item cxx_compiler_version

Get the C++ compiler version, e.g. "3.4.3"

=back

=cut

################################################################

sub get_item {
    my $self = shift;
    my $name = shift;
    return $self->{$name};
}

################################################################

=item $cfg-E<gt>get_item_list()

Return the list of item names that are available to the get_item() method.

=cut

################################################################

sub get_item_list {
    my $self = shift;
    my @item_list = ();
    foreach my $item ( @ItemList ) {
        if (defined($self->{$item})) {
            push @item_list, $item
        }
    }
    return @item_list;
}

################################################################

=item $cfg-E<gt>get_variable($name)

Get the configuration variable named $name.
In the case of autoconf-configured packages,
these are all the variables configured with the autoconf AC_SUBST macro.

Note that if the output variable has been defined, even if just to the
empty string, then get_variable() will return a value.
But if the output variable has not been defined at all,
then this function returns undef.
Thus programs can distinguish between undef and the empty string
when checking package configurations against one another.

=cut

################################################################

sub get_variable {
    my $self = shift;
    my $name = shift;
    my $variables = $self->{variables};
    return $variables->{$name};
}

################################################################

=item $cfg-E<gt>get_variable_list()

Return the list of variable names that are available to the get_variable() method.

=cut

################################################################

sub get_variable_list {
    my $self = shift;
    return sort keys %{$self->{variables}};
}

=back

=head1 BUGS

Currently cannot handle packages built using cons, scons, or anything other than automake.

Getting cxx_compiler_version using get_item() probably only works for GCC at present.

=head1 AUTHORS

Carl Beckmann

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2008

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;

