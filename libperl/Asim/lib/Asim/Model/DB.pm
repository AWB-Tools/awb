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

package Asim::Model::DB;
use warnings;
use strict;


=head1 NAME

Asim::Model::DB - Library for manipulating a database of all available models.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $modelDB = Asim::Model::DB-E<gt>new($basedir)

Create a new model database by searching the directory tree based at
$basedir for all model specification files.

=cut

################################################################


sub new {
  my $this = shift;
  my $basedir = shift || "config/pm" || return 0;

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
  # Place for cache of modelDB
  #
  if (! -d "$rootdir/var") {
    mkdir "$rootdir/var";
  }

  $self->{cache} = "$rootdir/var/modelDB";

  #
  # Set base of model search
  #
  $self->{basedir} = $basedir;

  #
  # Scan the directory tree
  #
  $self->{models} = {};
  $self->_readdb();
}


sub _readdb {
  my $self = shift;
  my $dbfile = $self->{cache};
  my $status;
  my $line;
  my $provides;
  my $filename;

  $self->{models} = {};

  $status = open(MODELDB, "<$dbfile");
  if (! $status) {
    $self->rehash();
    return;
  }

  while ($line=<MODELDB>) {
    chomp($line);
    ($provides, $filename) = split(":",$line);
    $self->_add_entry($provides, $filename);
  }

  close(MODELDB);
}

sub _writedb {
  my $self = shift;
  my $dbfile = $self->{cache};

  open(MODELDB, ">$dbfile") || return;

  foreach my $p (keys %{$self->{models}}) {
    foreach my $m (@{$self->{models}->{$p}}) {
      print MODELDB "$p:$m\n";
    }
  }

  close(MODELDB);
}

################################################################

=item $modelDB-E<gt>rehash()

Rehash the model database to reflect possible changes in the
database.

=cut

################################################################

sub rehash {
  my $self = shift;
  my $basedir = $self->{basedir};

  # Clear out the old list...and rescan tree

  $self->{models} = {};
  $self->_findmodels($basedir);

  # Save cached copy

  $self->_writedb();
  return 1;
}


################################################################

# Reference to a hash of model names


sub _findmodels {
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

      # Recursively scan for models
      $self->_findmodels($relname, $modnames);

    } elsif ( $fname =~ /.*\.apm$/) {

      # Add the model....
      #
      # Warning: This sequence is a hack dthat depends on a lot of knowledge
      # of the structure of a model fle in order to improve the performance
      # of the scan.
      #

      my $model = Asim::Inifile->new($absname);

      if (! defined($model)) {
        iwarn("Could not open model file: $absname\n");
        next;
      }

      my $modname         = $model->get("Global", "Name");

      my $rootmodname     = $model->get("Model", "model");
      my $rootmodfile     = $model->get($rootmodname, "File");
      my $rootmodule      = Asim::Module->new($rootmodfile);

      if (! defined($rootmodule)) {
        iwarn("Could not open module at root of model : $absname\n");
        next;
      }

      my $modprovides     = $rootmodule->provides();

      if ($modprovides eq "model") {
        # We don't want to record model
        next;
      }

      if (defined($modnames->{$modname})) {
        iwarn("Duplicate model name encountered ($modname) in " .
              $modnames->{$modname} . " and " . $relname . "\n");
        next;
      }

      $modnames->{$modname} = $relname;

      $self->_add_entry($modprovides, $relname);
    }
  }
}

sub _add_entry {
  my $self = shift;
  my $provides = shift;
  my $filename = shift;

  if (!defined($self->{models}->{$provides})) {
    $self->{models}->{$provides} = ();
  }

  push(@{$self->{models}->{$provides}}, $filename);

}


################################################################

=item $modelDB-E<gt>path()

Path of source of modelDB information.

=cut

################################################################

sub path {
  my $self = shift;

  return $self->{basedir};
}



################################################################

=item $modelDB-E<gt>model_types()

Return a list of all the model types

=cut

################################################################

sub model_types {
  my $self = shift;
  my @model_types = keys(%{$self->{models}});

  return @model_types;
}


################################################################

=item $modelDB-E<gt>find($modeltype)

Return a list of models of ASIM type $modeltype

=cut

################################################################

sub find {
  my $self = shift;
  my $modeltype = shift;
  my @models = ();

  foreach my $f (@{$self->{models}->{$modeltype}}) {
    my $m = Asim::Model->new($f);

    if (!defined($m)) {
      iwarn("Model cache contains non-existant model ($f).\n");
      iwarn("Try refreshing modelDB cache.\n");
      next;
    }

    push(@models, $m);
  }

  return (@models);
}






################################################################

=item $modelDB-E<gt>dump()

Textually dump the contents of the object

=cut

################################################################


sub dump {
  my $self = shift;
  my $level = shift;

  print "Dumping ModelDB\n";

  my @names;

  foreach my $p (keys %{$self->{models}}) {
    foreach my $m (@{$self->{models}->{$p}}) {
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

  print "Asim::Model::DB:: Warning: - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Asim::Model::DB:: Error - $message";

  return 1;
}

=back

=head1 BUGS

This file is almost exactly the same as Module::DB. They probably should be unified.

The search for models in _findmodels does an expensive opens of the model file. 
The expense is due to fact that the open will open all the module files, and we 
don't actually need to that info for this search.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
