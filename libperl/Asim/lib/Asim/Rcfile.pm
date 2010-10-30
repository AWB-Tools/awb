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


package Asim::Rcfile;
use warnings;
use strict;

use File::Basename;
use File::Spec;

use Asim::Base;
use Asim::Inifile;

our @ISA = qw(Asim::Base);


our %a =  ( workspace =>            [ "workspace",
                                      "SCALAR" ],
          );

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_RCFILE});

=head1 NAME

Asim::Rcfile - Library for manipulating ASIM rcfile

=head1 SYNOPSIS

use Asim::Rcfile;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an ASIM rcfile.
Primarily this rcfile provides a way of setting a default $AWBLOCAL.

The .asimrc file currently contains the following:

  [Global]
  Version=1.0
  Workspace=<directory path>


=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $rcfile = Asim::Rcfile-E<gt>new([$file])

Create a new rcfile object, optionally reading configuration 
file $file to populate the object.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = _initialize();

  bless	$self, $class;

  #
  # Parse file if given
  #
  if (@_) {
    if (! $self->open(@_) ) {
      return ();
    }
  }

  return $self;
}

sub _initialize {
  my $self = { accessors => \%a,
	       filename => "",
	       inifile => {},
	     };

  return $self;
}


################################################################

=item $rcfile-E<gt>create($file)

Create a new rcfile

=cut

################################################################


# Implemented in Asim::Rcfile::Template


################################################################

=item $rcfile-E<gt>open($file)

Parse an rcfile and populate the attributes
of the rcfile object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;
  my $file2 = shift;

  my $inifile;
  my $inifile2;

  # Open local rcfile

  $file = File::Spec->rel2abs($file);

  # If rcfile doesn't exist create an emppty one

  if (! -e $file) {
    if (! -d dirname($file)) {
      mkdir dirname($file);
    }

    CORE::open(RC, "> $file") || return ();
    CORE::close(RC);
  }

  $inifile = Asim::Inifile->new($file);
  print "Rcfile: User rcfile: $file\n" if ($DEBUG);

  $self->{filename} = $file;
  $self->{inifile} = $inifile;


  # Open global rcfile

  $file2 = File::Spec->rel2abs($file2);
  print "Rcfile: Global rcfile: $file2\n" if ($DEBUG);

  $inifile2 = Asim::Inifile->new($file2);

  $self->{filename2} = $file2;
  $self->{inifile2} = $inifile2;


  return 1;
}


################################################################

=item $rcfile-E<gt>save()

Save the rcfile information

=cut

################################################################

sub save {
  my $self = shift;
  my $file = shift
    || $self->filename();

  return $self->{inifile}->save($file);
}


################################################################

=item $module-E<gt>accessors()

Return a list of accessor functions for this object

This looks broken...

=cut

################################################################

sub accessors {

  return qw( version
             workspace
           );

}


=item $module-E<gt>filename()

Filename of source of rcfile information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $rcfile-E<gt>version([$value])

Set rcfile "version" to $value if supplied.
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","Version",$_[1]); }



################################################################

=item $rcfile-E<gt>get($grouplist, $item)

Return value from [$group] where $group is a member of the
comma separated list $grouplist for $item. Check first in local
rcfile and then in global rcfile.

=cut

################################################################

sub get {
  my $self = shift;
  my $grouplist = shift;
  my $item = shift;

  my @groups = split(",", $grouplist);

  my $newval;

  foreach my $group (@groups) {
    $newval = $self->{inifile}->get($group,$item);
    last if (defined($newval));
  }

  if (!defined($newval)) {
    if (!defined($self->{inifile2})) {
      print STDERR "Option [$grouplist] $item = undef (not in user inifile/no global inifile)\n" if ($DEBUG);
      return undef;
    }

    foreach my $group (@groups) {
      $newval = $self->{inifile2}->get($group,$item);
      last if (defined($newval));
    }

    if (! defined($newval)) {
      print STDERR "Option [$grouplist] $item = undef (neither user or global inifile)\n" if ($DEBUG);
      return undef;
    }

    print STDERR "Option [$grouplist] $item = $newval (Global inifile)\n" if ($DEBUG);
    return $newval;
  }

  print STDERR "Option [$grouplist] $item = $newval (User inifile)\n" if ($DEBUG);
  return $newval;
}

################################################################



################################################################

=item $rcfile-E<gt>workspace([$value])

Set rcfile "workspace" to $value if supplied.
Always return rcfile "workspace".

=cut

################################################################

sub workspace        { return $_[0]->_accessor("Global","Workspace",$_[1]); }

################################################################


=item $rcfile-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base.

################################################################

=item $rcfile-E<gt>edit()

Edit the rcfile

=cut

################################################################

sub edit {
  my $self = shift;

  $self->Asim::Base::edit();
}


################################################################

=item $model-E<gt>dump()

Dump the rcfile information in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;
  my @list;

  my $version   = $self->version()     || "";
  my $workspace = $self->workspace()   || "";

  print "Version: $version\n";
  print "Workspace: $workspace\n";

  return 1;
}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Rcfile: Error - $message";

  return 1;
}

###########################################################################

=back

=head1 BUGS


=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
