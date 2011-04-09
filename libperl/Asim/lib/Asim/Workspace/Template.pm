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

package Asim::Workspace;
use warnings;
use strict;

use File::Path;
use File::Spec;

our $DEBUG;

=head1 NAME

Asim::Workspace::Template - Asim Workspace Template functions

=head1 SYNOPSIS

use Asim::Workspace;

TBD

=head1 DESCRIPTION

Utility functions....

=over 4

=cut


################################################################

=item Asim::Workspace::create()

Create a skeleton for a new workspace named $workspace. 
Place it in $path, if specified.

=cut

################################################################

sub create {
    my $self = shift;
    my $path = shift
      || ierror("FAILURE: Workspace directory must be provided\n") && return 0;

    my $dir;

    debug("Creating workspace $path\n");

    $path = Asim::Util::expand_tilda($path);
    $path = File::Spec->rel2abs($path);

    if ( -e $path ) {
      ierror("FAILURE: Workspace directory already exists ($path)\n");
      return 0;
    }

    #
    # Root of workspace
    #
    mkpath "$path" 
      || ierror("FAILURE: Could not create workspace directory ($path)\n") && return 0;

    #
    # Rest of workspace subdirectories 
    #    (I assume these will be successful if the previous mkpath worked)
    #
    mkpath "$path/src";

    $dir = "$path/src/private/config/bm/private";
    mkpath $dir;
    system "echo 'Directory for private benchmark configuration files' >$dir/README";

    $dir = "$path/src/private/config/pm/private";
    mkpath $dir;
    system "echo 'Directory for private model configuration files' >$dir/README";

    mkpath "$path/build";
    mkpath "$path/run";
    mkpath "$path/var";
    mkpath "$path/var/log";

    $self->_create_awbconfig("$path/awb.config");

    $ENV{AWBLOCAL} = $path;

    $self->open($path);

    return 1;
}


sub _create_awbconfig {
  my $self = shift;
  my $awbconfig = shift;

  open(AWBCONFIG, ">$awbconfig");
  print AWBCONFIG <<"EOF";
[Global]
VERSION=1.4
Class=Asim::Workspace
# Display splash (1) or not (0)
SPLASH=1

[Vars]
private=src
shared=$Asim::packagedir
components={private,awb,asimcore}

[Paths]
# Directory containing ASIM source tree
ASIMDIR=\$(shared)/$Asim::package/$Asim::release

# Directory containing actual benchmarks
BENCHMARKDIR=$Asim::datadir/benchmarks

# Directory to do model builds in
#BUILDDIR=build

# Path where we search for ASIM files
SEARCHPATH=\$(private)/private:\$(shared)/awb/$Asim::release

[Package]
# Configure flags for all packages in this workspace
CONFIGFLAGS=

# Make flags for all packages in this workspace
MAKEFLAGS=

[Build]
# Make flags for all model builds
MAKEFLAGS=

# Compiler (GEM | GNU)
COMPILER=GNU

# Do parallel make (1) or not (0)
PARALLEL=1

# DEBUG or OPTIMIZE
BUILDTYPE=OPTIMIZE

# Build models with (1) or without (0) support for events
EVENTS=0
EOF

  close(AWBCONFIG);

}

#
# Unix-style touch of a file
#
sub _touch {
  `touch $_[0]`;
}

################################################################
#
# Internal error utility function
#
################################################################

# From Workspace.pm

=back

=head1 BUGS

None?

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
