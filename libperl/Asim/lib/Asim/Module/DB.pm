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

package Asim::Module::DB;
use warnings;
use strict;


=head1 NAME

Asim::Module::DB - Library for manipulating a database of all available modules.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $moduleDB = Asim::Module::DB-E<gt>new($basedir)

Create a new module database by searching the directory tree based at
$basedir for all module specification files.

=cut

################################################################


sub new {
  my $this = shift;
  my $basedir = shift || "." || return 0;

  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  $self->_initialize($basedir);

  return $self;
}

sub _initialize {
  my $self = shift;
  my $basedir = shift || ".";
  my $rootdir = $Asim::default_workspace->rootdir();

  #
  # Place for cache of moduleDB
  #
  if (! -d "$rootdir/var") {
    mkdir "$rootdir/var";
  }

  $self->{cache} = "$rootdir/var/moduleDB";

  #
  # Set base of module search
  #
  $self->{basedir} = $basedir;

  #
  # Scan the directory tree
  #
  $self->{modules} = {};
  $self->_readdb();
}


sub _readdb {
  my $self = shift;
  my $dbfile = $self->{cache};
  my $status;
  my $line;
  my $provides;
  my $filename;

  $self->{modules} = {};

  $status = open(MODULEDB, "<$dbfile");
  if (! $status) {
    $self->rehash();
    return;
  }

  while ($line=<MODULEDB>) {
    chomp($line);
    ($provides, $filename) = split(":",$line);
    $self->_add_entry($provides, $filename);
  }

  close(MODULEDB);
}

sub _writedb {
  my $self = shift;
  my $dbfile = $self->{cache};

  open(MODULEDB, ">$dbfile") || return;

  foreach my $p (keys %{$self->{modules}}) {
    foreach my $m (@{$self->{modules}->{$p}}) {
      print MODULEDB "$p:$m\n";
    }
  }

  close(MODULEDB);
}

################################################################

=item $moduleDB-E<gt>rehash()

Rehash the module database to reflect possible changes in the
database.

=cut

################################################################

sub rehash {
  my $self = shift;
  my $basedir = $self->{basedir};

  # Clear out the old list...and rescan tree

  $self->{modules} = {};
  $self->_findmodules($basedir);

  # Save cached copy

  $self->_writedb();
  return 1;
}


################################################################

# Reference to a hash of module names


sub _findmodules {
  my $self = shift;
  my $dir = shift;
  my $modnames = shift || {};

  # Get the list of files in the current directory.

  my @filenames = $Asim::default_workspace->listdir($dir);

  foreach my $fname (@filenames) {
    next if $fname eq '.';
    next if $fname eq '..';

    my $relname = "$dir/$fname";

    # remove ./ prefix...
    $relname =~ s/^\.\///;

    my $absname = $Asim::default_workspace->resolve($relname);

    if (!defined($absname)) {
      iwarn("Filename unresolvable: $relname - possible dangling link\n");
      next;
    }

    if ( -d $absname ) {

      # Recursively scan for modules
      $self->_findmodules($relname, $modnames);

    } elsif ( $fname =~ /.*\.awb$/) {

      # Add the module....
      my $module = Asim::Module->new($relname);

      if (!defined($module)) {
        iwarn("Unparsable .awb file: $relname\n");
        next;
      }

      my $modname = $module->name();
      my $modfile = $module->filename();

      if (defined($modnames->{$modname})) {
        iwarn("Duplicate module name encountered ($modname) in " .
              $modnames->{$modname} . " and " . $modfile . "\n");
      }

      $modnames->{$modname} = $modfile;

      $self->_add_entry($module->provides(), $module->filename());
    }
  }
}

sub _add_entry {
  my $self = shift;
  my $provides = shift;
  my $filename = shift;

  if (!defined($self->{modules}->{$provides})) {
    $self->{modules}->{$provides} = ();
  }

  push(@{$self->{modules}->{$provides}}, $filename);

}


################################################################

=item $moduleDB-E<gt>path()

Path of source of moduleDB information.

=cut

################################################################

sub path {
  my $self = shift;

  return $self->{basedir};
}



################################################################

=item $moduleDB-E<gt>module_types()

Return a list of all the module types

=cut

################################################################

sub module_types {
  my $self = shift;
  my @module_types = keys(%{$self->{modules}});

  return @module_types;
}


################################################################

=item $moduleDB-E<gt>find($moduletype)

Return a list of modules of ASIM type $moduletype

=cut

################################################################

sub find {
  my $self = shift;
  my $moduletype = shift;
  my @modules = ();

  foreach my $f (@{$self->{modules}->{$moduletype}}) {
    my $m = Asim::Module->new($f);

    if (!defined($m)) {
      iwarn("Module cache contains non-existant module ($f).\n");
      iwarn("Try refreshing moduleDB cache.\n");
      next;
    }

    push(@modules, $m);
  }

  return (@modules);
}






################################################################

=item $moduleDB-E<gt>dump()

Textually dump the contents of the object

=cut

################################################################


sub dump {
  my $self = shift;
  my $level = shift;

  print "Dumping ModuleDB\n";

  my @names;

  foreach my $p (keys %{$self->{modules}}) {
    foreach my $m (@{$self->{modules}->{$p}}) {
      push(@names, "$p:\t $m");
    }
  }

  foreach my $n (sort @names) {
    print "$n\n";
  }

}


################################################################
#
# Internal error utility function
#
################################################################

sub iwarn {
  my $message = shift;

  print "Asim::Module::DB:: Warning: - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Asim::Module::DB:: Error - $message";

  return 1;
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
