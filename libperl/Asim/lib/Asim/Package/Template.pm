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


package Asim::Package;
use warnings;
use strict;

use File::Path;

# Package wide variables (defined in Package.pm)

our @ISA;

our $CHANGES;

our $PACKAGES;
our $MYTAGS;
our $MYMERGEPOINTS;
our $IPCHIST;

our $CONFIG_BM;
our $CONFIG_PM;

our $PM;

our $GOLDSTATS;

=head1 NAME

Asim::Package::Template - Asim Package Template functions

=head1 SYNOPSIS

use Asim::Package;

TBD

=head1 DESCRIPTION

Utility functions....

=over 4

=cut


################################################################

=item Asim::Package::create($package, [$path])

Create a skeleton for a new package named $package. 
Place it in $path, if specified.

(Copy of documentation from Package.pm)

=cut

################################################################

sub create {
  my $self = Asim::Package->new();

  my $package = shift
    || return ();

  my $where = shift
    || $Asim::default_workspace->rootdir() . "/src";

  my $root = "$where/$package";

  if (-e $root) {
    ierror("A file already exists at $root - No package created\n");
    return ();
  }

  mkpath "$root" || return 0;

  #
  # Standard files
  #
  _touch("$root/changes");


  #
  # Create standard administrative files
  #
  my $filename = "$root/$PACKAGES/$package";
  mkpath "$root/$PACKAGES";
  _touch($filename);

  _touch("$root/$MYTAGS");

  #
  # Create configuration file skeleton
  #
  mkpath "$root/$CONFIG_BM";
  system "echo \"User defined benchmark configurations go here\" >$root/$CONFIG_BM/README";

  mkpath "$root/$CONFIG_PM/$package";
  system "echo \"User defined model configurations go here\" >$root/$CONFIG_PM/$package/README";

  #
  # Create place for actual performance package
  #
  mkpath "$root/$PM/$package";
  system "echo \"Model modules go here\" >$root/$PM/$package/README";


  #
  # Initalize info for workspace
  #
  $self->open($filename);
  $self->version(1.0);
  $self->name($package);
  $self->description("Description of $package");
  $self->csn(0);
  $self->lockname("asim-$package");
  $self->tag("HEAD");
  $self->prerelease(1);
  $self->buildorder(1000);
  $self->save();

  #
  # Check the rationality of the package
  #
  $self->check() || return 0;

  return $self;
}

sub create_regtest {
  my $self = shift;

  my $location = $self->location();

  #
  # Create place for regtests
  #
  mkpath "$location/$GOLDSTATS";
  system "echo \"Gold stats from regression tests go here\" >$location/$GOLDSTATS/README";
  
  #
  # Create empty ipchist file
  #
  _touch("$location/$IPCHIST");

  $self->regtest(1);
  $self->save();
  }

#
# Unix-style touch of a file
#
sub _touch {
  `touch $_[0]`;
}

=back

=head1 BUGS

None known.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
