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

use File::Basename;
use File::Spec;
use Cwd 'realpath';

use Asim::Base;
use Asim::Inifile;
use Asim::UnionDir;

use Asim::Workspace::Template;


our $DEBUG = (($ENV{ASIM_DEBUG} || 0) >= 1) || defined($ENV{ASIM_DEBUG_WORKSPACE});

our $DEBUG_OPTIONS = $DEBUG 
                   || defined($ENV{ASIM_DEBUG_OPTIONS});


our @ISA = qw(Asim::Base Asim::UnionDir);

our %a =  ( name =>                 [ "name",
                                      "SCALAR" ], 
            description =>          [ "description",
                                      "SCALAR" ],
          );

# set this flag to force a check for the AWB package,
# if the workspace is created prior to the AWB split:
our $check_for_awb_package = 0;

=head1 NAME

Asim::Workspace - Library for manipulating ASIM workspace

=head1 SYNOPSIS

use Asim::Workspace;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an ASIM workspace.
A workspace is actually the root of a directory tree containing
a file structure like the following:

   +-- awb.config          File with workspace configuration
   |
   +--+ build              Directory containing builds 
   |  |
   |  +--+ model1          Model build directory
   |  |
   |  +--+ model2          Model build directory
   |
   +--+ src                Directory of containing packages
   |  |
   |  + asim-core          Package direcctory
   |  |
   |  + asim-feeder        Package direcctory
   |  |
   |  + asim-private       Package direcctory
   |
   +--+ var                Various cached information


Use of a "config" directory in the root of the workspace is deprecated, in
preference to using to a private package, e.g., asim-private.

The awb.config file current contains the following:

  [Global]
  # Display splash (1) or not (0)
  SPLASH=1

  [Vars]
  private=src/asim-
  shared=/usr/local/share/asim/src/
  components={private,test,test2,test3,pm,core,feeder}

  [Paths]
  # Directory containing ASIM source tree
  ASIMDIR=$(private)pm

  # Directory containing actual benchmarks
  BENCHMARKDIR=/usr/local/share/asim/benchmarks

  # Path where we search for ASIM files
  SEARCHPATH=$(private)$(components):$(shared)$(components)/dev

  [Package]
  # Configure flags for all packages in this workspace
  CONFIGFLAGS=

  # Make flags for all packages in this workspace
  MAKEFLAGS=

  [Build]
  # Make flags for all model builds
  MAKEFLAGS=

  # Compiler (GEM | GNU)
  COMPILER=GEM

  # Do parallel make (1) or not (0)
  PARALLEL=1

  # DEBUG or OPTIMIZE
  BUILDTYPE=OPTIMIZE

  # Build models with (1) or without (0) support for events
  EVENTS=0


=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $workspace = Asim::Workspace-E<gt>new([$file])

Create a new workspace object, optionally reading configuration 
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
    if (! $self->open($_[0] ) ) {
      return undef;
    }
  }

  return $self;
}

sub _initialize {
  my $self = { accessors => \%a,
               version => 1.2,
	       filename => "",
	       inifile => {},
	       options => {}
	     };

  return $self;
}


################################################################

=item $workspace-E<gt>create($file)

Create a performance workspace file $file and surrounding
workspace directories.


=cut

################################################################


# Implemented in Asim::Workspace::Template


################################################################

=item $workspace-E<gt>open($file)

Parse a performance awb.config file $file and populate the attributes
of the workspace object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;
  my $inifile;

  # If user just specified a directory try awb.config as the file

  $file = Asim::Util::expand_tilda($file);
  $file = File::Spec->rel2abs($file);
  if (-e $file) {
    $file = realpath($file);
  }

  if ( -d $file ) {
    $file = $file . "/awb.config";
  }

  $inifile = Asim::Inifile->new($file) || return undef;

  $self->{rootdir}  = dirname($file);
  $self->{filename} = $file;
  $self->{inifile}  = $inifile;

  $self->_rehash_path();

  $self->{packageDB} = Asim::Package::DB->new($self);

  return $self->path();
}


################################################################

=item $workspace-E<gt>rehash()

Rehash all the information about a workspace. At present that
just means the search path. Maybe we should reread the awb.config
file as well.

=cut

################################################################

sub rehash {
  my $self = shift;

  #
  # Rehash the search path
  #

  $self->_rehash_path()
    || return undef;

  #
  # Rehash the packageDB
  #
  $self->{packageDB}->rehash()
    || return undef;

  return 1;
}



################################################################

=item $workspace-E<gt>save()

Save the workspace information

=cut

################################################################

sub save {
  my $self = shift;
  my $file = shift
    || $self->filename();

  return $self->{inifile}->save($file);
}



################################################################

=item $workspace-E<gt>accessors()

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

=item $workspace-E<gt>rootdir()

Filename of source of module information

=cut

################################################################

sub rootdir {
  my $self = shift;

  return $self->{rootdir};
}


################################################################

=item $workspace-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename { 
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $workspace-E<gt>version([$value])

Set workspace "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","VERSION",$_[1]) || "1.0"; }



################################################################

=item $workspace-E<gt>name([$value])

Set workspace "name" to $value if supplied. 
Always return workspace "name".

=cut

################################################################

sub name        { return $_[0]->_accessor("Global","Name",$_[1]); }

################################################################

=item $workspace-E<gt>description([$value])

Set workspace "description" to $value if supplied. 
Always return current workspace "description".

=cut

################################################################

sub description { return $_[0]->_accessor("Global","Description",$_[1]); }


################################################################

=item $workspace-E<gt>build_dir()

Return the awb build directory

=cut

################################################################

sub build_dir {
  my $self = shift;

  return $self->rootdir() . "/build/default";
}


################################################################

=item $workspace-E<gt>run_dir()

Return the awb run directory

=cut

################################################################

sub run_dir {
  my $self = shift;

  return $self->rootdir() . "/run";
}

################################################################

=item $workspace-E<gt>src_dir()

Return the awb build directory

=cut

################################################################

sub src_dir {
  my $self = shift;

  return $self->rootdir() . "/src";
}

################################################################

=item $workspace-E<gt>benchmark_dir()

Return the awb benchmark directory

=cut

################################################################

sub benchmark_dir {
  my $self = shift;

  return $self->{inifile}->get("Paths","BENCHMARKDIR");
}

################################################################

=item $workspace-E<gt>log_dir()

Return the awb build directory

=cut

################################################################

sub log_dir {
  my $self = shift;

  return $self->rootdir() . "/var/log";
}


################################################################

=item $workspace-E<gt>awb_package_configflags()

Return the awb Package config flags

=cut

################################################################

sub awb_package_configflags {
  my $self = shift;

  return $self->get_option("Package","CONFIGFLAGS");
}

################################################################

=item $workspace-E<gt>awb_package_makeflags()

Return the awb Package make flags

=cut

################################################################

sub awb_package_makeflags {
  my $self = shift;

  return $self->get_option("Package","MAKEFLAGS");
}

################################################################

=item $workspace-E<gt>path()

Return the awb search path

Note we keep parallel version of the path
   1) in the Inifile:: unexpanded
   2) in the UnionDir:: expanded


This method returns 2

=cut

################################################################

sub path {
  my $self = shift;

  return $self->Asim::UnionDir::path(@_);
}


################################################################

=item $workspace-E<gt>unexpanded_path()

Return the awb search path as seen in the awb.file

Note we keep parallel version of the path
   1) in the Inifile:: unexpanded
   2) in the UnionDir:: expanded


This method returns 1

=cut

################################################################

sub unexpanded_path {
  my $self = shift;

 return $self->_accessor_array("Paths","SEARCHPATH",":");

}


################################################################

=item $workspace-E<gt>set_path($packagedir [,$packagedir]...)

Set the asim search path to the list of package directories
provided. Variable substitution and cross product expansion
are performed on each $packagedir argument provided.

=cut

################################################################

sub set_path {
  my $self = shift;
  my @path = @_;

  #
  # Note we keep parallel version of the path
  #   1) in the Inifile:: unexpanded
  #   2) in the UnionDir:: expanded
  #
  $self->_rehash_path(@path)
    || return undef;

  $self->_accessor_array_set("Paths","SEARCHPATH",":",@path);

  #
  # Rehash the packageDB, too
  #
  $self->{packageDB}->rehash()
    || return undef;


  return 1;
}



################################################################

=item $workspace-E<gt>prepend_path($dir)

Prepend $dir to the path

=cut

################################################################

sub prepend_path {
  my $self = shift;
  my $dir = shift;

  my $rdir;
  my @unexpanded_path;

  $rdir = $self->_relativize_dir($dir);

  @unexpanded_path = $self->unexpanded_path();
  unshift(@unexpanded_path, $rdir);

  return $self->set_path(@unexpanded_path);
}


################################################################

=item $workspace-E<gt>postpend_path($dir)

Postpend $dir to the path

=cut

################################################################

sub postpend_path {
  my $self = shift;
  my $dir = shift;

  my $rdir;
  my @unexpanded_path;

  $rdir = $self->_relativize_dir($dir);

  @unexpanded_path = $self->unexpanded_path();
  push(@unexpanded_path, $rdir);

  return $self->set_path(@unexpanded_path);
}

################################################################
#
#  $dir = workspace->_relativize_dir($dir)
#
#  If possible make $dir relative to workspace. Otherwise
#  return the original value.
#
################################################################

sub _relativize_dir {
  my $self = shift;
  my $dir = shift;

  my $absdir;
  my $rootdir;

  $absdir = File::Spec->rel2abs($dir);
  $absdir = realpath($absdir);

  $rootdir = $self->rootdir();

  if ( $absdir =~ /$rootdir\/(.*)/ ) {
    return $1;
  }

  return $dir;
}


################################################################

=item $workspace-E<gt>remove_path_by_packagename($file)

Remove a directory from the path corresponding to a particular
package.

=cut

################################################################

sub remove_path_by_packagename {
  my $self = shift;
  my $packagename = shift;

  my $packageDB  = $self->packageDB()
                   || return undef;

  my $package    = $packageDB->get_package($packagename)
                   || return undef;

  my $packagedir = $package->location();

  print "Removing package at: $packagedir\n" if ($DEBUG);

  my @path = $self->path();
  my $idx  = 0;

  # Search for matching path in searchpath
  #     Note: both @path and $packagedir are already canonicalized with 'realpath'.

  for my $d (@path) {
    print "Comparing package path to: $d\n" if ($DEBUG);

    if ( $d eq $packagedir) {
      my $rindex = @{$self->{epath2path}}[$idx];

      if (! ($rindex =~ /[0-9]+/)) {
        iwarn("Cannot remove searchpath element generated with crossproduct expansion\n");
        return undef;
      }

      return $self->_remove_path_by_index($rindex);
    }

    $idx++;
  }
}


sub _remove_path_by_index {
  my $self = shift;
  my $index = shift;

  my @path = $self->unexpanded_path();

  splice(@path, $index, 1);

  return $self->set_path(@path);
}


################################################################
#
# AWB Searchpath Utility Functions
#
# Create a searchpath for the workspace
#      Multiple representations of the searchpath are maintained:
#           1) @path the path elements from the orginal text in the awb.config file
#           2) @epath the variable substituted, cross product expanded
#              and canonicalized version
#
#       @epath2path contains the index in @path for each element in @epath
#       iff the mapping is 1:1.
#
################################################################

sub _rehash_path {
  my $self = shift;
  my @path = @_;

  if (! @path) {
    @path = split(":",$self->{inifile}->get("Paths","SEARCHPATH"));
  }

  my @epath = ();
  my @epath2path = ();

  my @tpath = ();
  my $pnum = 0;

  #
  # Do variable substition in path
  #
  @epath = $self->_substitute_array(@path);

  #
  # Do {a,b,c} expansion in path
  #

  $pnum = 0;
  @tpath = ();

  for my $d (@epath) {
    my @xpath = $self->_expand_cross($d);

    push(@tpath, @xpath);

    # Keep a map of the index in @path that correspends to the element in @epath

    if ($#xpath == 0) {
      push(@epath2path, $pnum);
    } else {
      push(@epath2path, ("X") x ($#xpath + 1));
    }
    $pnum++;
  }

  @epath = @tpath;
  $self->{epath2path} = \@epath2path;

  #
  # Make absolute
  #

  @tpath = ();

  foreach my $i (@epath) {

    if ( ! File::Spec->file_name_is_absolute($i)) {
      $i = $self->rootdir() . "/$i";
    }

    if (! -d $i) {
      print "Non-directory ($i) found in search path\n" if $DEBUG;
      next;
    }
    push(@tpath, $i);

  }

  @epath = @tpath;

  #
  # Put path into the UnionDir
  #
  $self->Asim::UnionDir::set_path(@epath);


  if ($DEBUG) {
    my $i = 0;
    while ($i <= $#epath) {
      print "  " . $epath[$i] . " @ " . $epath2path[$i] . "\n";
      $i++;
    }
  }


  return 1;
}



# Do variable substitution in an awb string

sub _substitute_array {
  my $self = shift;
  my @path = @_;
  my @epath = ();

  foreach my $i (@path) {
    $i = $self->_substitute($i);
    push(@epath, $i);
  }

  return @epath;
}

sub _substitute {
  my $self = shift;
  my $s = shift;

  while ($s =~ /\$\((\w*)\)/) {
      my $varname = $1;
      my $var = $self->_accessor("Vars", $varname);

      if (defined $var) {
          $s =~ s/\$\(\w*\)/$var/;
      }
      else {
          print "WARNING: Undefined variable '$varname' in workspace configuration file!\n";

          $s = "";
      }
  }

  return $s;
}

# Do cross product expansion of a value

sub _expand_cross {
  my $self = shift;
  my @path = @_;
  my @epath = ();

  my $changed = 1;

  while ($changed) {
    $changed = 0;
    foreach my $i (@path) {
      if ($i =~ /\{(.*)\}/) {
	$changed = 1;
	my @cross = split(/,/,$1);
	foreach my $j (@cross) {
	  my $k = $i;
	  $k =~ s/\{.*\}/$j/;
	  push(@epath, $k);
	}
      } else {
	push(@epath, $i);
      }
    }

    return @epath;
  }
}

################################################################

=item $workspace-E<gt>resolve($file)

Resolve $file using the awb search path into an absolute
file name.

=cut

################################################################

# Implemented in Asim::UnionDir

################################################################

=item $workspace-E<gt>unresolve($file)

Unresolve $file using the awb search path into a relative
file name.

=cut

################################################################

# Implemented in Asim::UnionDir

################################################################

=item $workspace-E<gt>listdir($path)

Return the list of names at the given path for the union of the awb
searchpath pathes. Duplicates filenames from different pathes are
supressed, so they are only returned once.

=cut

################################################################

# Implemented in Asim::UnionDir

################################################################

=item $workspace-E<gt>glob($file)

Return the 'glob' of the given partial path using the awb
search path.

=cut

################################################################

# Implemented in Asim::UnionDir

################################################################

=item $workspace-E<gt>packageDB($file)

Return the name of the package containing $file

=cut

################################################################

sub packageDB {
  my $self = shift;

  return $self->{packageDB};
}

################################################################

=item $workspace-E<gt>file2package($file)

Return the name of the package containing $file

=cut

################################################################

sub file2package {
  my $self = shift;
  my $file = shift;

  my $packageDB = $self->{packageDB};

  return $packageDB->file2package($file);
}

################################################################

=item $workspace-E<gt>get_option($grouplist, $item)

Return value from [$group] where $group is a member of $grouplist
for $item in one of several places. In priority order (from highest 
to lowest):

     <workspace>/awb.config
     $(HOME)/.asim/asimrc
     <INSTALLDIR>/etc/asim/asimrc

Note that $grouplist is a comma (,) separated list of groups
to look in sequentially.

Remember the set of options requested, so they can be returned with
list_options()

=cut

################################################################

sub get_option {
  my $self = shift;
  my $grouplist = shift;
  my $item = shift;
  my $def = shift;

  my @groups = split(",",$grouplist);

  my $val;

  foreach my $group (@groups) {
    $val = $self->get($group, $item);
    last if (defined($val));
  }

  if (defined($val)) {
    # Strip leading and trailing spaces and double quotes from around string

    $val =~ s/^\s*(\"?)(.*)\1\s*$/$2/;

    # Is this value not just an empty string
    
    if ($val ne "") {
      print STDERR "Option [$grouplist] $item = \'$val\' (in awb.config)\n" if $DEBUG_OPTIONS;

      $self->{options}->{$groups[0]}->{$item} = $val;
      return $val;
    }  
  }
  

  # Check in rcfile's - probably this should be in the workspace

  if (!(defined($Asim::rcfile)))
  {
      if (! defined($def)) {
        print STDERR "Option [$grouplist] $item = undef (no rcfile/no default)\n" if $DEBUG_OPTIONS;

	$self->{options}->{$groups[0]}->{$item} = "undef";
        return undef;
      }

      print STDERR "Option [$grouplist] $item = \'$def\' (no rcfile/default)\n" if $DEBUG_OPTIONS;

      $self->{options}->{$groups[0]}->{$item} = $def;
      return $def;
  }

  $val = $Asim::rcfile->get($grouplist, $item);
  
  if (defined($val)) {
    # Strip leading and trailing spaces and double quotes from around string

    $val =~ s/^\s*(\"?)(.*)\1\s*$/$2/;
#    $val =~ s/^\s*(.*)\s*$/$1/;

    # Is this value just an empty string

    if ($val ne "") {
      print STDERR "Option [$grouplist] $item = \'$val\' (user or global rcfile)\n" if $DEBUG_OPTIONS;

      $self->{options}->{$groups[0]}->{$item} = $val;
      return $val;
    }
  }

  # It wasn't in any files - so return default value, if any.
  
  if (! defined($def)) {
    print STDERR "Option [$grouplist] $item = undef (no default)\n" if $DEBUG_OPTIONS;

    $self->{options}->{$groups[0]}->{$item} = "undef";
    return undef;
  }

  print STDERR "Option [$grouplist] $item = \'$def\' (default)\n" if $DEBUG_OPTIONS;

  $self->{options}->{$groups[0]}->{$item} = $def;
  return $def;
}

################################################################

=item $workspace-E<gt>get($group, $item)

Return value from [$group] for $item in awb.config file

=cut

################################################################

sub get {
  my $self = shift;
  my $group = shift;
  my $item = shift;

  my $newval;

  $newval = $self->{inifile}->get($group,$item);

  if (!defined($newval)) {
    print STDERR "Option [$group] $item = undef (not in awb.config)\n" if ($DEBUG);
    return undef;
  }

  print STDERR "Option [$group] $item = $newval (awb.config)\n" if ($DEBUG);
  return $newval;
}



################################################################

=item $workspace-E<gt>list_options()

Return hash with all options used by this program

=cut

################################################################

sub list_options {
  my $self = shift;

  return $self->{options}
}


################################################################

=item $workspace-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base.

################################################################

=item $workspace-E<gt>edit()

Edit the workspace

=cut

################################################################

sub edit {
  my $self = shift;

  $self->Asim::Base::edit();
}

################################################################

=item $workspace-E<gt>ask_for_regtestdir()

Ask user to latest regtest directory

=cut

################################################################


sub ask_for_regtestdir {
  my $self = shift;

  my $proj_root = $self->rootdir();
  my $regtest_root = "$proj_root/build/regtest";

  my @regtests = (sort glob("$regtest_root/*"));
  my $guess = $regtests[$#regtests];

  return Asim::choose_filename("Regtest directory", $guess);
}

################################################################

=item $model-E<gt>upgrade()

Attempt to upgrade a workspace to the latest version.

=cut

################################################################


sub upgrade {
  my $self = shift;

  if ($self->version() == 1.0) {
    print "Upgrading workspace from version 1.0 to 1.1\n";

    mkdir $self->rootdir() . "/run";
    $self->version("1.1");
    $self->save();
  }

  if ($self->version() == 1.1) {
    print "Upgrading workspace from version 1.1 to 1.2\n";

    $self->{inifile}->put("Global", "Class", "Asim::Workspace");
    $self->version("1.2");
    $self->save();
  }

  if ($self->version() == 1.2) {
    print "Upgrading workspace from version 1.2 to 1.3\n";

    my $logdir = $self->log_dir();

    if (! -d $logdir) {
      mkdir $logdir;
    }

    $self->version("1.3");
    $self->save();
  }

  if ($self->version() == 1.3) {
    print "Upgrading workspace from version 1.3 to 1.4\n";

    # make sure we have AWB package, or ask user to add it
    $check_for_awb_package = 1;

    $self->version("1.4");
    $self->save();
  }

}

################################################################

=item $model-E<gt>dump()

Dump the awb configuration in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;
  my @list;

  my $version = $self->version()     || "";
  my $name    = $self->name()        || "";
  my $desc    = $self->description() || "";
  my $rootdir = $self->rootdir()     || "";

  print "Version: $version\n";
  print "Name: $name\n";
  print "Description: $desc\n";
  print "Workspace directory: $rootdir\n";

  $self->Asim::UnionDir::dump();
  $self->dump_options();

  return;
}



################################################################

=item $workspace-E<gt>dump_options()

Print list of options used by this program

=cut


################################################################

sub dump_options {
  my $self = shift;

  my $options = $self->list_options();

  print "Options in awb.config or asimrc:\n";
  foreach my $group (sort(keys(%$options))) {
    print "  [$group]\n";

    foreach my $item (sort(keys(%{$options->{$group}}))) {
      my $value = $options->{$group}->{$item};
      print "    $item=$value\n";
    }
  }

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
    print "Workspace: $message";
  }

  return 1;
}

sub ierror {
  my $message = shift;

  print "Workspace: Error - $message";

  return 1;
}

sub iwarn {
  my $message = shift;

  print "Workspace: Warning - $message";

  return 1;
}

###########################################################################

=back

=head1 BUGS

The the semantic for when are where to set AWBLOCAL seems overly complex to me...

Should argument to new (and friends) be an awb.config file or a root
path with a default concatentation of awb.config... or either...

The _parseing of a search path should make sure that all of the
checked out pieces are under the workspace.

Other values provided in awb.conf need to be made public.

Saved configuration files lose file format.

More error checking needed.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
