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


package Asim::Repository::DB;
use warnings;
use strict;

use Asim::Inifile;
use Asim::Package;
use Asim::Base;

use File::Basename;
use File::Find;

our @ISA=qw(Asim::Base);

our $VERSION = 1.1;
our $Default_packfile = "/usr/local/etc/asim/asim.pack";


=head1 NAME

Asim::Repository::DB - Library for manipulating a DB of ASIM CVS
repositories, i.e., a set of repositories defined in a .pack file.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $repositoryDB = Asim::Repository::DB-E<gt>new([$packfile])

Create a new repository database by reading the .pack file $packfile
Default packfile is /usr/local/share/asim/etc/asim.pack.

=cut

################################################################


sub new {
  my $this = shift;
  my $packfile = shift || $Default_packfile;

  if (! -e $packfile) {
    return 0;
  }

  my $class = ref($this) || $this;
  my $self = _initialize($packfile);

  bless	$self, $class;

  $self->rehash() || return undef;

  return $self;
}

sub _initialize {
  my $packfile = shift;

  my $self = {};

  $self->{filename} = $packfile;

  return $self;
}

################################################################

=item $repositoryDB-E<gt>rehash()

Rehash the repository DB.

=cut

################################################################

sub rehash {
  my $self = shift;
  my $version;
  our $VERSION;

  $self->{inifile} = Asim::Inifile->new($self->{filename});

  $version = $self->{inifile}->get("Global","Version");

  if ( $version > $VERSION) {
    ierror("Invalid packfile version - found $version - need $VERSION\n");
    return undef;
  }

  return 1;
}


################################################################

=item $module-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}


################################################################

=item $repositoryDB-E<gt>name([$value])

Set repositoryDB "name" to $value if supplied. 
Always return repositoryDB "name".

=cut

################################################################

sub name        { return $_[0]->_accessor("Global","Name",$_[1]); }

################################################################

=item $repositoryDB-E<gt>description([$value])

Set repositoryDB "description" to $value if supplied. 
Always return current repositoryDB "description".

=cut

################################################################

sub description { return $_[0]->_accessor("Global","Description",$_[1]); }

################################################################

=item $repositoryDB-E<gt>directory([$bundle])

List repositorys in the repositoryDB. 
Optionally restricting to repositories in $bundle.

=cut

################################################################

sub directory {
  my $self = shift;
  my $bundle = shift;

  my @list = sort(grep(!/^Global$/, $self->{inifile}->get_grouplist()));

  #
  # Optionally only select repositories in a particular bundle
  #
  if (defined($bundle)) {
    my @rawlist;
    my $bundles;
    my $r;

    @rawlist = @list;
    @list = ();

    for $r (@rawlist) {
      next if !($r =~ /\//);
      $bundles = $self->_get_item($r, "Bundles");
      if ( defined($bundles) && $bundles =~ /$bundle/) {
	push(@list, $r);
      }
    }
  }

  return (@list);
}

################################################################

=item $repositoryDB-E<gt>public_directory()

List public packages

=cut

################################################################

sub public_directory {
  my $self = shift;

  my $dir = Asim::Packagedir(). "/";
  my @dirs = glob("${dir}*/*");
  my @names; 

  foreach my $i (@dirs) {
    $i =~ s/$dir//;
    push @names, $i;
  }	

  return @names;
}

################################################################

=item $repositoryDB-E<gt>get_repository($repositoryname)

Return a repository object with the given name.

=cut

################################################################

sub get_repository {
  my $self = shift;
  my $fullname = shift;

  my $name;
  my $tag;
  my $cvstag;

  my $desc;
  my $method;
  my $access;
  my $module;
  my $target;
  my $changes;

  my $repository;


  ($name, $tag) = (split("/",$fullname), "HEAD");

  # Make CVS happy by converting  .'s to _'s in tag

  $cvstag = $tag;
  $cvstag =~ s/\./_/g;

  $desc    = $self->_get_item($fullname, "Description");
  $method  = $self->_get_item($fullname, "Method") || return undef;

  # Following will eventually be conditional on "Method";

  if (($method eq "cvs") || ($method eq "pserver") || ($method eq "bitkeeper") || ($method eq "svn")) {
    $access  = $self->_get_item($fullname, "Access")  || return undef;
    $module  = $self->_get_item($fullname, "Module")  || return undef;
    $target  = $self->_get_item($fullname, "Target")  || return undef;
    $changes = $self->_get_item($fullname, "Changes");

    $repository = Asim::Repository->new(method  => $method, 
                                        access  => $access,
                                        module  => $module,
                                        tag     => $cvstag,
                                        target  => $target,
                                        changes => $changes);

  } else {
    ierror("Unknown access method $method");
    return undef;
  }

   return $repository;
}

################################################################

=item $repositoryDB-E<gt>get_public_repository($repositoryname)

Return a public repository object with the given name.

=cut

################################################################

sub get_public_repository {
  my $self = shift;
  my $fullname = shift;

  my $name;
  my $tag;
  my $cvstag;

  my $desc;
  my $method;
  my $access;
  my $module;
  my $target;
  my $changes;

  my $repository;


  ($name, $tag) = (split("/",$fullname), "HEAD");

  # Make CVS happy by converting  .'s to _'s in tag

  $method = "copy";
  $access = Asim::Packagedir() . "/" . $name . "/" . $tag;
  $desc   = "Unknown";

  $repository = Asim::Repository->new(method  => "copy",
                                      access  => $access,
                                      module  => "asim-$name",
                                      tag     => $tag,
                                      target  => "asim-$name",
                                      changes => "changes");

  return $repository;
}

################################################################

=item $repositoryDB-E<gt>bundle_directory()

List bundles in the bundles area.

=cut

################################################################

sub bundle_directory {
  my $self = shift;
  our @files;  
  
  sub wanted {
      push(@files, $File::Find::name) if (-f $File::Find::name);
  }

  find(\&wanted, Asim::Sysconfdir() . "/asim/bundles");

  # also read in custom bundles in the user's area 
  my $userbundles = Asim::Util::expand_tilda("~/.asim/bundles");
  if (-d $userbundles) {
      find(\&wanted, $userbundles);
  }

  my $bundlelist = {};
  foreach my $f (@files) {
      my ($base, $path, $type) = fileparse($f, '\..*');
      $bundlelist->{$base} = 1 if (defined($type) && $type ne "");
  }

  return sort(keys %$bundlelist);
}

################################################################

################################################################

=item $repositoryDB-E<gt>bundle_files($bundlename)

Get list of files that provide bundle given by $bundlename

=cut

################################################################

sub bundle_files {
  my $self = shift;
  my $bundle = shift;
  our @files; 
  my @flist;
  
  find(\&wanted, Asim::Sysconfdir() . "/asim/bundles");

  # also read in custom bundles in the user's area 
  my $userbundles = Asim::Util::expand_tilda("~/.asim/bundles");
  if (-d $userbundles) {
      find(\&wanted, $userbundles);
  }

  my $bundlelist = {};
  foreach my $f (@files) {
      my ($base, $path, $type) = fileparse($f, '\..*');
      if (defined($type) && $type ne "" && $base eq $bundle) {
	  push(@flist, $f);
      }
  }

  return @flist;
}

################################################################

################################################################

=item $repositoryDB-E<gt>bundle_files($bundlename)

Get list of ids available for bundle $bundlename

=cut

################################################################

sub bundle_ids {
  my $self = shift;
  my $bundlename = shift;
 
  my @flist = $self->bundle_files($bundlename);
  (@flist == 0) && return ();
  
  #
  # Read bundle files and collect ids
  #
  my $inifile = Asim::Inifile->new();
  foreach my $file (@flist) {
      $inifile->include($file);
  }
  
  return $inifile->get_grouplist();
}

################################################################

#
# Get information from inifile,
#   First, look in group [$name/$tag]
#   Then, look in group [$name]
#
sub _get_item {
  my $self = shift;
  my $fullname = shift;
  my $item = shift;
  my $default = shift;

  my $name;
  my $tag;

  my $value;

  ($name, $tag) = (split("/",$fullname), "HEAD");

  $value = $self->{inifile}->get("$name/$tag", $item)  
             || $self->{inifile}->get("$name", $item)
             || $default;

  return $value;
}


################################################################

=item $repositoryDB-E<gt>dump()

Textually dump the contents of the repositoryDB object.

=cut

################################################################


sub dump {
  my $self = shift;
  my $level = shift || "";

  my $desc;

  print $level . "Name: " . $self->name() . "\n";
  print $level . "Description: " . $self->description() . "\n";

  print $level . "Packages:\n";
  foreach my $i ( $self->directory()) {
    $desc =  $self->{inifile}->get($i, "Description") || "";

    printf "$level %-20s - %s\n", $i,$desc;
  }

}

################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Repository::DB: Error - $message";

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
