#
# Copyright (C) 2003-2009 Intel Corporation
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

package Asim::Bundle;
use warnings;
use strict;

use File::Basename;
use File::Spec;
use Cwd 'realpath';

use Asim::Inifile;


our $DEBUG = (($ENV{ASIM_DEBUG} || 0) >= 1) || defined($ENV{ASIM_DEBUG_BUNDLE});

our @ISA = qw(Asim::Base);

our %a =  ( name =>                 [ "name",
                                      "SCALAR" ], 
            description =>          [ "description",
                                      "SCALAR" ],
          );

=head1 NAME

Asim::Bundle - Library for manipulating an AWB bundle

=head1 SYNOPSIS

use Asim::Bundle;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an AWB bundle.
A bundle is just a set of respositories, specified in the usual
<REPOSITORY>/<VERSION> form. Bundles also have some attributes, such
as a type and a status.

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $bundle = Asim::Bundle-E<gt>new($inifile,[$name])

Create a new bundle object, based on the bundle descriptions in
$file with $name. The default name is "default".

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = _initialize();

  bless	$self, $class;

  my $inifile = shift || return undef;
  my $name    = shift || "default";


  $self->{inifile} = $inifile;
  $self->{name} = $name;

  return $self;
}

sub _initialize {
  my $self = { accessors => \%a,
               version => 1.0,
	       filename => "",
	       name => "",
	       inifile => {},
	     };

  return $self;
}



################################################################

=item $bundle-E<gt>accessors()

Return a list of accessor functions for this object

This looks broken...

=cut

################################################################

sub accessors {

  return qw( name
             description 
           );

}



################################################################

=item $bundle-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename { 
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $bundle-E<gt>version([$value])

Set workspace "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","VERSION",$_[1]) || "1.0"; }



################################################################

=item $bundle-E<gt>name([$value])

Always return workspace "name".

=cut

################################################################

sub name {
   my $self = shift;

   return $self->{name};
}


################################################################

=item $bundle-E<gt>description([$value])

Always return current bundle "description".

=cut

################################################################

sub description {
    my $self = shift;

    return $self->{inifile}->get($self->{name}, "Type") || "";
}

################################################################

=item $bundle-E<gt>type([$value])

Always return current bundle "type".

=cut

################################################################

sub type {
    my $self = shift;

    return $self->{inifile}->get($self->{name}, "Type") || "Unkwown";
}


################################################################

=item $bundle-E<gt>status([$value])

Always return current bundle "status".

=cut

################################################################

sub status {
    my $self = shift;

    return $self->{inifile}->get($self->{name}, "Status") || "Unknown";
}


################################################################

=item $bundle-E<gt>packages([$value])

Always return current bundle "packages".

=cut

################################################################

sub packages {
    my $self = shift;

    my $list = $self->{inifile}->get($self->{name}, "Packages") || return ();

    return split(" ", $list);
}




################################################################

=item $bundle-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base.

################################################################


################################################################

=item $bundle-E<gt>dump()

Dump the awb configuration in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;
  my @list;

  my $version = $self->version()     || "";
  my $name    = $self->name()        || "";

  print "Version: $version\n";
  print "Name: $name\n";

  return;
}




################################################################
#
# Internal error utility function
#
################################################################

sub debug {
  my $message = shift;

  if ($DEBUG) {
    print "Bundle: $message";
  }

  return 1;
}

sub ierror {
  my $message = shift;

  print "Bundle: Error - $message";

  return 1;
}

sub iwarn {
  my $message = shift;

  print "Bundle: Warning - $message";

  return 1;
}

###########################################################################

=back

=head1 BUGS

This class is awkward since new() takes a $inifile instead of a file name, since
bundleid come from a collections of inifiles instead of just one. Thus, for
example, one cannot update or save a bundle.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Intel, 2009

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
