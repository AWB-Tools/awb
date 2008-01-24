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

our $DEBUG=$Asim::Repository::DEBUG;


=head1 NAME

Asim::Repository::DB - Library for manipulating a DB of ASIM
repositories, i.e., a set of repositories defined in a set of
.pack files.

At present CVS, SVN and Bitkeeper repositories are supported.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $repositoryDB = Asim::Repository::DB-E<gt>new([$packfile]...)

Create a new repository database by reading each .pack file $packfile
If $packfile is a directory, then read all files of the form *.pack
in that directory.

Default packfile path is: 
        $HOME/.asim/repositories.d/
        $HOME/.asim/asim.pack
        <install-dir>/repositories.d/
        <install-dir>/asim.pack

=cut

################################################################


sub new {
  my $this = shift;
  my @packfile_path = @_;

  if ($#packfile_path < 0) {
    @packfile_path = ();
    push(@packfile_path, "$ENV{HOME}/.asim/repositories.d",
                         "$ENV{HOME}/.asim/asim.pack",
                         Asim::Sysconfdir() . "/asim/repositories.d",
                         Asim::Sysconfdir() . "/asim/asim.pack");
  }

  my $class = ref($this) || $this;
  my $self = _initialize(@packfile_path);

  bless	$self, $class;

  $self->rehash() || return undef;

  return $self;
}

sub _initialize {
  my @packfile_path = @_;

  my $self = {};

  $self->{packfile_path} = \@packfile_path;

  return $self;
}

################################################################

=item $repositoryDB-E<gt>rehash()

Rehash the repository DB.

=cut

################################################################

sub rehash {
  my $self = shift;
  my @dir;

  $self->{inifile} = Asim::Inifile->new();

  # Reverse order so items earlier in the list overwrite
  # items later in the list.

  foreach my $packfile (reverse @{$self->{packfile_path}}) {

    # For directories get all the *.pack files

    if (-d $packfile) {
      for my $i  (glob("$packfile/*.pack")) {
        $self->_add_packfile($i);
      }
    }

    # For regular files just open them

    elsif (-e $packfile) {
      $self->_add_packfile($packfile);
    }
  }

  # Check that we have some repositories to check out from

  @dir = $self->directory();
  if ($#dir < 0) {
    ierror("No repositories were found. No checkouts can be done\n");
    return undef;
  }

  return 1;
}


sub _add_packfile {
  my $self = shift;
  my $packfile = shift;
  my $status;
  my $version;

  our $VERSION;

  print "Adding packfile: $packfile\n" if $DEBUG;

  $status = $self->{inifile}->include("$packfile");

  if (!defined($status)) {
    ierror("Failed to include packfile: $packfile\n");
    return undef;
  }

  # Latest include will overwrite Version...so we can check each file

  $version = $self->{inifile}->get("Global","Version");

  if ( $version > $VERSION) {
    ierror("Invalid packfile version in $packfile - found $version - need $VERSION\n");
    return undef;
  }
}

################################################################

=item $repositoryDB-E<gt>packfile_path()

Array of packfiles (or packfile directories) containing repositry
information.

=cut

################################################################

sub packfile_path {
  my $self = shift;

  return @{$self->{packfile_path}};
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
  my $browseURL;
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
    $access    = $self->_get_item($fullname, "Access")    || return undef;
    $module    = $self->_get_item($fullname, "Module")    || return undef;
    $target    = $self->_get_item($fullname, "Target")    || return undef;
    $browseURL = $self->_get_item($fullname, "BrowseURL");
    $changes   = $self->_get_item($fullname, "Changes");

    # Note: 
    #   We depend on the packagename matching the package filename,
    #   i.e., the filename in the admin/package directory, 
    #   but we cannot check that here

    $repository = Asim::Repository->new(packagename => $name,
                                        method      => $method,
                                        access      => $access,
                                        module      => $module,
                                        tag         => $cvstag,
                                        browseURL   => $browseURL,
                                        target      => $target,
                                        changes     => $changes);

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

  $repository = Asim::Repository->new(packagename => $name,
                                      method      => "copy",
                                      access      => $access,
                                      module      => "asim-$name",
                                      tag         => $tag,
                                      target      => "asim-$name",
                                      changes     => "changes");

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

  # read in bundles from the system bundle directory
  my $systembundles = Asim::Sysconfdir() . "/asim/bundles";
  if (-d $systembundles) {
    find({wanted => \&wanted, follow => 1}, $systembundles);
  }

  # also read in custom bundles in the user's area 
  my $userbundles = Asim::Util::expand_tilda("~/.asim/bundles");
  if (-d $userbundles) {
      find({wanted => \&wanted, follow => 1},  $userbundles);
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

  find({wanted => \&wanted, follow => 1},  Asim::Sysconfdir() . "/asim/bundles");

  # also read in custom bundles in the user's area 
  my $userbundles = Asim::Util::expand_tilda("~/.asim/bundles");
  if (-d $userbundles) {
      find({wanted => \&wanted, follow => 1},  $userbundles);
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

Bundles should also support a path of locations where
bundles are defined.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
