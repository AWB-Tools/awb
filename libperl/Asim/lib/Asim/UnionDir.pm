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

package Asim::UnionDir;
use warnings;
use strict;



our $DEBUG = ($ENV{ASIM_DEBUG} || 0) >= 2;

use File::Basename;
use File::Spec;
use Cwd 'realpath';

=head1 NAME

Asim::UnionDir - Implements a class like Plan 9 UnionDirs.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides a set of methods to view a set of directories
as if they were combined into a single directory path. Sort of a
sh-style $PATH for ordinary files.


=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $uniondir = Asim::Uniondir-E<gt>new([@searchpath])

Create a new ASIM parameter

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  if ($_[0]) {
    $self->set_path(@_);
  }    

  return $self;
}


################################################################

=item $uniondir-E<gt>path([@searchpath])

Retrun the current awb search path

=cut

################################################################

sub path {
  my $self = shift;

  return @{$self->{path}};
}

################################################################

=item $uniondir-E<gt>set_path([@searchpath])

Set the current awb search path.

Note: elements of the path are converted to remove symbolic links.

=cut

################################################################

sub set_path {
  my $self = shift;
  my @value = (@_);

  @{$self->{path}} = map {_safe_realpath($_)} @value;

  return @{$self->{path}};
}


sub _safe_realpath {
  my $path = shift;

  if (! -e $path) {
    iwarn("Setting path with non-existant path: $path\n");
    return $path;
  }

  return realpath($path);
}

################################################################

=item $uniondir-E<gt>resolve($file)

Resolve $file using the awb search path into an absolute 
file name.

=cut

################################################################

sub resolve {
  my $self = shift;
  my $file = shift;

  return undef if ! defined($file);

  #
  # Don't turn around the lookup order, otherwise it depends on the
  # user's CWD if a path will be resolved as relative from CWD or via
  # the UnionDir searchpath; results my be different if one of the two
  # crosses a symlink.
  #

  #
  # Look in each element of path for file
  #
  foreach my $p ($self->path()) {
    if (-e "$p/$file") {
      return "$p/$file";
    }
  }

  #
  # Otherwise, if file just exists make it absolute,
  # remove symbolic links aliases and return it
  #
  if (-e $file) {
    $file = _delinkify($file);
    return $file;
  }

  #
  # Not found...
  #
  return undef;
}

################################################################

=item $uniondir-E<gt>unresolve($file)

Unresolve absolute filename $file using the awb search path into a
relative file name. Returns null for unresolvable filenames. Note, 
further that the mapping guarantees that the following invariant
is true:

  $absfile eq $uniondir->resolve($uniondir->unresolve($absfile))

Otherwise a null is returned. Thus, if a file is hidden under the
shadow of another file, unresolve will return null rather than a
relative path that would resolve to another file.

Note, $absfile is converted to remove symbolic link aliases as has
already been done to the search path.

=cut

################################################################

sub unresolve {
  my $self = shift;
  my $absfile = shift;
  my $resolve;

  debug("Unresolve file (original): $absfile\n");

  $absfile = _delinkify($absfile);

  debug("Unresolve file (absolute): $absfile\n");

  foreach my $p ($self->path()) {
    debug("Checking path: $p\n");

    if ( $absfile =~ /^$p\/(.*)/ ) {

      $resolve = $self->resolve($1) || next;
      debug("Possible match...($1 -> $resolve)?..\n");

      if ( $resolve eq $absfile) {
        debug("...Yes, got it!\n");
	return $1;
      }

      debug("...Nope, continue looking\n");
    }
  }

  return undef;
}



################################################################

# Utility function to canonicallize a filename including
# converting links into real directories.

sub _delinkify {
  my $file = shift;
  my $dir;

  $file = File::Spec->rel2abs($file);
  if (-d $file) {
    $file = realpath($file);
  } else {
    $dir = dirname($file);
    if (-d $dir) {
      $dir = realpath($dir);
    }
    $file = join("/", $dir, basename($file));
  }
  return $file;
}

################################################################

=item $uniondir-E<gt>listdir($path)

Return the list of names at the given path for the union of the
uniondir pathes. Duplicates filenames from different pathes are
supressed, so they are only returned once.

=cut

################################################################

sub listdir {
  my $self = shift;
  my $dir = shift;

  my $curdir;
  my %list;

  foreach my $p ($self->path()) {
    $curdir = "$p/$dir";
    if (-e $curdir && opendir(DIR,$curdir)) {
      foreach my $n (readdir(DIR)) {
        next if ($n eq ".");
        next if ($n eq "..");
        next if ($n eq "CVS");
        next if ($n eq ".svn");  
        next if ($n eq "SCCS");

	if (! defined($list{"$n"})) {
	  $list{"$n"} = 1;
	}
      }
      closedir(DIR);
    }
  }

  return (sort keys %list);
}

################################################################

=item $uniondir-E<gt>glob($pattern)

Return the 'glob' of the given partial path pattern using
the awb search path.

=cut

################################################################

sub glob {
  my $self = shift;
  my $pattern = shift;

  my @ans = ();

  debug("Globing for: $pattern\n");

  foreach my $p ($self->path()) {
    foreach my $f (CORE::glob("$p/$pattern")) {
      $f =~ s/$p\///;
      if (! grep(/^$f$/, @ans)) {
        debug("Glob matched: $f\n");
        push(@ans, $f);
      }
    }
  }

 return @ans;
}


################################################################

=item $model-E<gt>dump()

Dump the awb configuration in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;
  my @list;

  print "Search path: \n";
  @list = $self->path();
  foreach my $i (@list) {
    print "  $i\n";
  }

  return 1;
}


################################################################
#
# Internal error utility function
#
################################################################

sub debug {
  my $message = shift;

  if ($DEBUG) {
    print "Uniondir: $message";
  }

  return 1;
}

sub iwarn {
  my $message = shift;

  print "Uniondir: Warning - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Uniondir: Error - $message";

  return 1;
}


=back

=head1 BUGS

Hope it really matches Artur's TCL code semantics.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;

