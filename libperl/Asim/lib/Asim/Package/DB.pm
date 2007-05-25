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


package Asim::Package::DB;
use warnings;
use strict;

use File::Basename;


our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_PACKAGE});


=head1 NAME

Asim::Package::DB - Library for manipulating a DB of active ASIM
packages, i.e., the packages checked out and visible in the AWB
search path.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $packageDB = Asim::Package::DB-E<gt>new()

Create a new package database by searching the path specified
by $awbconfig and locating the visible packages, i.e., the files
visible in .../admin/packages/*.

=cut

################################################################


sub new {
  my $this = shift;
  my $workspace = shift || $Asim::default_workspace;

  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  $self->{workspace} = $workspace;
  $self->_initialize();

  return $self;
}

sub _initialize {
  my $self = shift;

  $self->rehash();

  # maybe create each of the packages here....
}


################################################################

=item $packageDB-E<gt>rehash()

Rehash the list packages in the packageDB

In specific, rehash populates the following strcuctures:

 $self->{packages} -
      a reference to a list of package names

 $sel->{path} -
      a hash mapping package names to the directory containing it

=cut

################################################################

sub rehash {
  my $self = shift;
  my $workspace = $self->{workspace};

  my @packagedirs;
  my $admindir;
  my $path;

  print "Rehashing packages\n" if ($DEBUG);

  @{$self->{packages}} = ();
  $self->{path} = {};

  @packagedirs = $workspace->listdir("admin/packages");
  if (! @packagedirs) {
    return undef;
  }

  foreach my $p (@packagedirs) {
    # . .. and version control files, .e.g., CVS, .svn, are already removed
    $p =~ /~$/ && next;
    $p =~ /^\#/ && next;

    print("Rehashing package at: $p\n") if ($DEBUG);

    # Add name of package to list

    push(@{$self->{packages}}, $p);

    # Remember the path to the root of the package (with trailing /)

    $admindir = "admin/packages/$p";

    $path = $workspace->resolve($admindir);
    $path =~ s/${admindir}$//;

    $self->{path}->{$p} = $path;
  }

#  while( my ($k, $v) = each %{$self->{path}} ) {
#    print "key: $k, value: $v.\n";
#  }

  return 1;
}

################################################################

=item $packageDB-E<gt>directory()

List packages in the packageDB. Returns a list of names. 
Use get_package to convert to a package object.

=cut

################################################################

sub directory {
  my $self = shift;

  return (sort @{$self->{packages}});
}

################################################################

=item $packageDB-E<gt>file2package($file)

Return the name of the file containing the given $file.

=cut

################################################################

sub file2package {
  my $self = shift;
  my $file = shift;

  $file = $self->{workspace}->resolve($file);

  foreach my $k (keys %{$self->{path}}) {
    my $v = $self->{path}->{$k};

    if ( $file =~ /^$v/) {
      return $k;
    }
  }

  return undef;
}

################################################################

=item $packageDB-E<gt>duplicates()

List packages in the packageDB that appear more than once in
the search path.

=cut

################################################################

sub duplicates {
  my $self = shift;
  my %packages;
  my @dups = ();

  foreach my $s ($self->{workspace}->path()) {
    if (defined $s) {
      foreach my $p (glob "$s/admin/packages/*") {
      
        $p = basename($p);
        $p =~ /~$/   && next;
        $p =~ /^\#/  && next;
        $p =~ /^CVS$/ && next;
        $p =~ /^\.svn$/ && next;
        $p =~ /^\.$/   && next;
        $p =~ /^\.\.$/ && next;
      
        if (defined($packages{$p})) {
          push(@dups, $p)
        }
      
        $packages{$p} = 1;
      }
    }
  }

  return (sort @dups);
}


################################################################

=item $packageDB-E<gt>get_package($packagename)

Return a package object with the given name.

=cut

################################################################

sub get_package {
  my $self = shift;
  my $name = shift;

  my $location;
  my $package;

  $location = $self->{workspace}->resolve("admin/packages/$name")
    || return undef;

  $package = Asim::Package->new($location);

  return $package;
}

################################################################

=item $packageDB-E<gt>get_package_by_dirname($dir)

Return a package object for the package in the given directory.

=cut

################################################################

sub get_package_by_dirname {
  my $self = shift;
  my $dir = shift;

  foreach my $p (glob("$dir/admin/packages/*")) {

    my $b = basename($p);

    $b =~ /~$/   && next;
    $b =~ /^\#/  && next;
    $b =~ /^CVS$/  && next;
    $b =~ /^\.svn$/  && next;
    $b =~ /^\.$/   && next;
    $b =~ /^\.\.$/ && next;

    print("Found package at: $p\n") if ($DEBUG);

    return Asim::Package->new($p);
  }

  return undef;
}

################################################################

=item $packageDB-E<gt>dump()

Textually dump the contents of the packageDB object.

=cut

################################################################


sub dump {
  my $self = shift;
  my $level = shift || "";

  print $level . "Packages:\n";
  foreach my $i ( $self->directory() ) {
    print "$level  $i\n";
  }

}

=back

=head1 BUGS

Too numerous to list

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
