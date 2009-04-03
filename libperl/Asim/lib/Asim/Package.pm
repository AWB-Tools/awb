#
# *****************************************************************
# *                                                               *
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


package Asim::Package;
use warnings;
use strict;

use File::Spec;
use Cwd 'realpath';

use Asim::BuildTree;
use Asim::Lock;
use Asim::Base;
use Asim::Inifile;

use Asim::Package::Copy;
use Asim::Package::Cvs;
use Asim::Package::BitKeeper;
use Asim::Package::Svn;  
use Asim::Package::Util;
use Asim::Package::Commit;
use Asim::Package::Branch;
use Asim::Package::Template;
use Asim::Package::Stats;


our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_PACKAGE});


our @ISA = qw(Asim::BuildTree Asim::Lock Asim::Base Asim::Inifile);

our %a =  ( name          =>   [ "name",
                                 "SCALAR" ],
            description   =>   [ "description",
                                 "SCALAR" ],
            location      =>   [ "location",
                                 "SCALAR" ],
            dependencies  =>   [ "dependencies",
                                 "ARRAY" ],
            csn           =>   [ "csn",
                                 "SCALAR"],
            tag           =>   [ "tag",
                                 "SCALAR"],
            lockname      =>   [ "lockname",
                                 "SCALAR"],
            buildorder    =>   [ "buildorder",
                                 "SCALAR"],
            maketarget    =>   [ "maketarget",
                                 "SCALAR"],
            type          =>   [ "type",
                                 "SCALAR"],
          );

# Well known file locations (relative to $self->location())

our $CHANGES       = "changes";

our $PACKAGES      = "admin/packages";
our $MYTAGS        = "admin/mytags";
our $MYMERGEPOINTS = "admin/mymergepoints";

# Called ipchist even though it can contain other metrics...
our $IPCHIST       = "admin/ipchist";

our $CONFIG_BM     = "config/bm";
our $CONFIG_PM     = "config/pm";

our $PM            = "modules";

our $GOLDSTATS     = "regtest/gold";

=head1 NAME

Asim::Package - Library for manipulating an Asim package, ie., a 
checked out (or newly created) ASIM source tree. 

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an Asim package.
A package is a configuration file, and a stylized directory tree
containing a file structure like the following:

   +-- changes             File with a change log for the package
   |
   +--+ admin              Directory containing administrative info
   |  |
   |  +-- mytags           History of CVS tags for this package
   |  |
   |  +-- mymergepoints    History of CVS mergepoints (for a branch)
   |  |
   |  +-- ipchist          History regression info (for models)
   |  |
   |  +--+ packages        Package configuration directory
   |     |
   |     + name            Configuration file for package "name".
   |
   +--+ config             Directory containing configurations
   |  |
   |  +--+ bm              Directory containing benchmark configs
   |  |
   |  +--+ pm              Directory containing benchmark configs
   |     |
   |     + name            Directory of configurations for package "name"
   |
   +--+ pm                 Directory containing performance model
      |
      + name               Directory for model "name"


=head1 PRINCIPLES OF OPERATION

Some principles of operation for packages.

=head2 Package File

The package configuration file contains the following:

  [Global]
  Name=test
  Tag=HEAD
  Description=Test package
  CSN=CSN-core-468
  Version=1.0
  Regtest=0  

=head2 CSNs

Each version of a package has a CSN of the form
CSN-<package>-<number>.  The current version number is held in the
admin/packages/<package> file, and a history of these numbers is held
in the admin/mytags file.

=head2 Branches

A package may be branched and later merged with the main trunk of the
repository with the branch and merge operations. A branch always
exists in its own private CSN space. Merges are managed using the file
admin/mymergepoints which holds a history of the CSNs of the main
trunk against which the branch was merged.

=head2 Tag Syntax

When checking out or updating packages in a workspace, specific versions
or branches of the package may be checked out by specifyng an optional tag.
If the tag is "HEAD" or is omitted, the latest version of the trunk
is checked out.  To check out a specific version of the main trunk,
the syntax for the tag is:

  CSN-<packagename>-<number>

where <number> is a "commit serial number" or a global revision number.
For revision control system such as SVN that support a global revision number
for the entire repository, a bare revision number is also recognized:

  <number>

To check out the latest version on a branch, the tag is simply the
branch name:

  <branchname>

To check out a specific version on a branch, the syntax is:

  CSN-<branchname>-<number>

For revision control system that support a global revision number,
the following equivalent tag syntax also is recognized:

  <branchname>:<number>
  
By convention, branch names are ALL UPPER CASE, with underscores, and
optional dates in YYYYMMDD format, e.g.:

  FEATURE_XYZ_BRANCH_20070723

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item Asim::Package::init();


Do any one-time static initializations required by this package.

=cut

################################################################

sub init {
  # initialize the Cvs, Svn, and BitKeeper handlers:
  Asim::Package::Cvs::init();
  Asim::Package::Svn::init();
  Asim::Package::BitKeeper::init();
}

################################################################


=item $package = Asim::Package-E<gt>new([$file]);


Create a new package object, optionally opening package defined by $file, 
where $file is the package file for the package, i.e., the file in the
directory admin/packages.

=cut

################################################################

sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  #
  # Initialize package
  #
  $self->_initialize();

  #
  # Parse package if given
  #
  if (@_) {
    print "Opening package $_[0]\n" if ($DEBUG);

    $self->open($_[0])  || return ();
    $self->check($_[0]) || return ();
  }

  return $self;
}


sub _initialize {
  my $self = shift;

  $self->{accessors} = \%a;
  $self->{filename}  = "";
  $self->{location}  = "";
  $self->{inifile}   = {};

  return $self;
}


################################################################

=item Asim::Package::create($package [,$path])

Create a skeleton for a new package named $package. 
Place it in $path, if specified.

=cut

################################################################

# Implemented in Asim::Package::Template


################################################################

=item $package-E<gt>open($file)

Parse package file file $file and populate the attributes
of the package object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;

  my $location;
  my $inifile;

  #
  # Cannonicalize the filename
  #
  $file = $Asim::default_workspace->resolve($file) || return ();
  $self->{filename} = $file;

  #
  # Check that file exists
  #
  if (! -e $file) {
    ierror("No package file at $file\n");
    return 0;
  }

  #
  # Find root directory of the package
  #
  $location = File::Spec->rel2abs($file);
  $location =~ s/$PACKAGES\/.*//;
  $location = realpath($location);

  $self->{location} = $location;

  #
  #  Parse the configuration file
  #
  $inifile = Asim::Inifile->new($file)
    || ierror("Illegal package file - $file\n") && return 0;
  $self->{inifile} = $inifile;

  #
  # Determine what type of package we have
  #
  Asim::Package::Copy::set_type     ( $self ) ||
  Asim::Package::Cvs::set_type      ( $self ) ||
  Asim::Package::BitKeeper::set_type( $self ) ||
  Asim::Package::Svn::set_type      ( $self ) ||
  Asim::Package::set_type           ( $self );

  return $self;

}


################################################################

=item Asim::Package::set_type( $package )

Set the object's package type to Unknown and return 1.

=cut

################################################################

sub set_type {
  my $self = shift;
  $self->{type} = "unknown";
  return 1;
}


################################################################

=item Asim::Package::destroy()

Destroy (removing all files) for the package

=cut

################################################################

sub destroy {
  my $self = shift;
  my $location = $self->location
    || ierror("Corrupt package - no location\n") && return 0;

  system("rm -rf $location");

  $self->_initialize();
}

################################################################

=item $package-E<gt>check()

Check the consistency of the package

=cut

################################################################


sub check {
  my $self = shift;

  my $location = $self->location
    || ierror("Corrupt package - no location\n") && return 0;

  #
  # Check for standard files
  #
  foreach my $f ($MYTAGS) {
    if (! -e "$location/$f" && ! -e $f) {
      ierror("Corrupted package - Missing file $f\n");
      return 0;
    }
  }

  #
  # Check for a GOLD STATS directory
  #
  if ($self->regtest() && ! -d "$location/$GOLDSTATS") {
    ierror("Corrupted package - Missing $GOLDSTATS directory\n");
    return 0;
  }

  #
  # Consistency checks
  #
  if (! $self->csn() ) {
    ierror("Malformed CSN - fix in package file and $MYTAGS\n");
    return 0;
  }

  return 1;
}


################################################################

=item $package-E<gt>save()

Save the package information

=cut

################################################################

sub save {
  my $self = shift;
  my $file = shift
    || $self->filename();

  return $self->{inifile}->save($file);
}


################################################################

=item $workspace-E<gt>browse()

Open a grapichal browser on the package


=cut

################################################################

# Implemented in Base.pm


################################################################

=item $package-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    my $self = shift;

    return qw(name description location dependencies csn tag lockname buildorder maketarget type);
}


################################################################

=item $module-E<gt>filename()

Filename of package information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $package-E<gt>version([$value])

Set package "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","Version",$_[1]); }



################################################################

=item $package-E<gt>name([$value])

Set package "name" to $value if supplied. 
Always return model "name".

=cut

################################################################

sub name        { return $_[0]->_accessor("Global","Name",$_[1]); }

################################################################

=item $package-E<gt>description([$value])

Set package "description" to $value if supplied. 
Always return current package "description".

=cut

################################################################

sub description { return $_[0]->_accessor("Global","Description",$_[1]); }


################################################################

=item $package-E<gt>dependencies([@list])

Set package despendencies to @list if supplied. 
Always return current package dependency list

BUG: Cannot pass an empty dependency list

=cut

################################################################

sub dependencies {
  my $self = shift;
  my @list = (@_);

  if (defined($_[0])) {
    $self->_accessor_array_set("Global", "Dependencies", " ", @list);
  }
  return $self->_accessor_array("Global", "Dependencies", " ");
}

################################################################

=item $package-E<gt>regress_dependencies([@list])

Set package regress dependencies to @list if supplied. 
Always return current package regress dependency list

BUG: Cannot pass an empty dependency list

=cut

################################################################

sub regress_dependencies {
  my $self = shift;
  my @list = (@_);

  if (defined($_[0])) {
    $self->_accessor_array_set("Global", "Regress_Dependencies", " ", @list);
  }
  return $self->_accessor_array("Global", "Regress_Dependencies", " ");
}

################################################################

=item $package-E<gt>get_dependent_packages([@list])

Recursively get the packages dependent on $self and
append to @list if supplied. 

=cut

################################################################

sub get_dependent_packages {
  my $self = shift;
  my @p = @_;

  push(@p, $self);

  my $packageDB = Asim::Package::DB->new();
  my $n;
  my $t;
  my $p;

  foreach my $i ($self->dependencies()) {
    ($n, $t) = split(/\//, $i);

    # to avoid infinite loop when there are cycles in the dependence graph,
    # do not recurse on packages already added to the list:
    if ( grep {$_->name() eq $n} @p ) { next; }

    print "Found dependency on $n\n";
    
    $p = $packageDB->get_package($n)
      || iwarn("Could not find package ($n)\n") && next;

    # TODO: check tag...

    @p = ($p->get_dependent_packages(@p));

  }

  return (@p);
}


################################################################

=item $package-E<gt>get_regress_dependent_packages([@list])

Recursively get the dependent packages for regression on $self and
append to @list if supplied.

=cut

################################################################

sub get_regress_dependent_packages {
  my $self = shift;
  my @p = @_;

  push(@p, $self);

  my $packageDB = Asim::Package::DB->new();
  my $n;
  my $t;
  my $p;

  foreach my $i ($self->regress_dependencies()) {
    ($n, $t) = split(/\//, $i);
    
    # to avoid infinite loop when there are cycles in the dependence graph,
    # do not recurse on packages already added to the list:
    if ( grep {$_->name() eq $n} @p ) { next; }

    print "Found dependency on $n\n" if ($DEBUG);
    
    $p = $packageDB->get_package($n);

    # TODO: check tag...

    if (defined $p) {
	@p = ($p->get_regress_dependent_packages(@p));
    }
    else {
	# the caller is left to handle the offending package
	push(@p, $n);
    }
  }

  return (@p);
}

################################################################

=item $package-E<gt>csn([$value])

Optionally update serial number CSN of package to $value.
Return the current (updated) serial number (CSN)  of package.

Note that for packages that use the revision control system SVN,
which supports atomic commits with a single revision number for the
entire package, asim-shell keeps the CSN number equal to the SVN
revision number, by construction.

=cut

################################################################

sub csn { 
  my $self = shift;
  my $value = shift;

  my $csn;
  my $name = $self->name();

  if (defined($value)) {
    if ($value =~ /^\d+$/) {
      # Upgrade csn to include name
      $value = "CSN-$name-$value";
    }

    if (! ($value =~ /^CSN-.*-\d+$/) ) {
      ierror("Trying to assign malformed CSN ($value)\n");
      return ();
    }
  }

  $csn = $self->_accessor("Global","CSN",$value); 

  if (! defined($csn)) {
    ierror("Undefined CSN\n");
    return ();
  }
    
  if ($csn =~ /^\d+$/) {
    # Upgrade csn to include name
    $csn = "CSN-$name-$csn";
  }

  if (! ($csn =~ /^CSN-.*-\d+$/) ) {
    ierror("Malformed CSN ($csn)\n");
    return ();
  }

  return $csn;
}

################################################################

=item $package-E<gt>increment_csn()

Increment serial number CSN of package.
Return the current (updated) serial number (CSN)  of package.

=cut

################################################################

sub increment_csn {
  my $self = shift;

  my $csn;
  my $num;

  $csn = $self->csn();

  #
  # Extract number from end and add one
  #
  $csn =~ /^CSN-.*-(\d+)$/;
  $num = $1+1;

  $csn =~ s/\d+$/$num/;

  $csn = $self->csn($csn);

  return $csn;
}

################################################################

=item $package-E<gt>tag([$tag])

Optionally update string indicating the tag of the package $tag.
Return the current value of the tag

=cut

################################################################

sub tag { return $_[0]->_accessor("Global","Tag",$_[1]) || "HEAD"; }

################################################################

=item $package-E<gt>baseline_tag([<use_csn>])

Return a tag (see "Tag Syntax", above) that can be used later
to retrieve exactly the version of the package that is currently
checked out in the working copy (minus any uncommitted changes).

It is similar in spirit to the tag() or csn() methods, except that
it is read-only (it cannot be used to update tag or CSN data), and
if you are on the trunk or a branch, it does not return "HEAD" or
the branch name, but rather the specific version number of the
working copy.

The default behavior, which works for CVS checkouts or on
"copy" packages that are copies of HEAD, is to return the CSN,
which has the revision number and the branch name in it.
Derived classes may override this, but must always return a tag
conforming to the legal tag syntax.

If the optional <use_csn> flag is passed in as '1', then force
the use of CSN information from the admin/packages file, even if
more accurate information is available by querying the revision
control system directly.

=cut

################################################################

sub baseline_tag
{
  my $self = shift;
  my $use_csn = shift; # ignored
  return $self->csn();
}

################################################################

=item $package-E<gt>label($tag)

Label the currently checked out version of the repository
with the given $tag.  Returns 1 on success.

=cut

################################################################

sub label {
  my $self = shift;
  ierror('Package::label() not implemented for package type ' . $self->type() . "\n");
  return 0;
}

################################################################

=item $package-E<gt>branch_name()

Return the name of the branch that this repository was checked
out of.  If it was checked out of the main trunk, return the
value 'HEAD'.

=cut

################################################################

sub branch_name
{
  my $self = shift;
  return 'HEAD';
}

################################################################

=item $package-E<gt>buildorder([$value])

Set package "buildorder" to $value if supplied. 
Always return model "buildorder".

=cut

################################################################

sub buildorder { return $_[0]->_accessor("Global","Buildorder",$_[1]) || 10000; }

################################################################

=item $package-E<gt>maketarget([$value])

Set package "maketarget" to $value if supplied. 
Always return model "maketarget".

=cut

################################################################

sub maketarget { return $_[0]->_accessor("Global","MakeTarget",$_[1]); }


################################################################

=item $package-E<gt>regtest([$value])

Optionally update boolean indicating if package has a regtest to $value.
Return the current state of the boolean.

=cut

################################################################

sub regtest { return $_[0]->_accessor("Global","Regtest",$_[1]); }


################################################################

=item $package-E<gt>stats_metric([$value])

Optionally update value indicating which stat is tracked for this
package. Return the current state of the value. Default is 'IPC';

=cut

################################################################

# Implemented in Stats.pm


################################################################

=item $package-E<gt>location([$value])

Optionally update location of package to $value.
Return the current (updated) location of package.

=cut

################################################################

# Implemented in Asim::Base

################################################################

=item $package-E<gt>lockname()

Get name of lock;

=item $package-E<gt>acquire_lock()

Acquire lock on this package. 
Return the current state of the lock.


=item $package-E<gt>release_lock()

Release lock on this package. 
Return the current state of the lock.

=cut

################################################################

sub lockname {
  my $self = shift;
  my $value = shift;

  my $retval;

  $retval = $self->_accessor("Global","Lock",$value);

  if (! defined($retval)) {
    $value = "asim-" . $self->name();
    $retval = $self->_accessor("Global","Lock",$value);
  }

  return $retval;
}

# Others implemented in Asim::Package::Lock

################################################################

=item $package-E<gt>type()

Return type of package - cvs, svn, bitkeeper etc.

=cut

################################################################

sub type {
  return 'unknown';
}


################################################################

=item $package-E<gt>isprivate()

Return true if this package is private, i.e., is in the directory
<ASIMLOCAL>/src/

=cut

################################################################

sub isprivate {
  my $self = shift;

  my $srcdir = $Asim::default_workspace->src_dir();
  my $location = $self->location();

  if ( $location =~ /${srcdir}/ ) {
      return 1;
  }

  return ();
}





################################################################

=item $package-E<gt>isfrom($file)

Return true if specified file is from this package.

=cut

################################################################

sub isfrom {
  my $self = shift;
  my $file = shift;

  die "Error: Asim::Package->isfrom() - Unimplemented\n";

  return ();
}


################################################################

=item $package-E<gt>update()

Update this package from the repository.
This default one just prints an error, subclasses should provide their
own repository-specific version of this.

=cut

sub update {
  ierror("Cannot update unknown package type\n");
  return undef;
}

################################################################

=item $package-E<gt>up_to_date()

Return 1 if and only if the checked out package is up-to-date
with the contents of the repository.

=cut

sub up_to_date {
  my $self = shift;

  my @files = $self->status();
  if (! defined $files[0]) {
    ierror("No files in archive\n");
    return 0;
  }

  foreach my $i (@files) {
    my ($dir, $file, $status, $reprev, $workdate) = @{$i};

    if ( ! ($status =~ /Up-to-date/ )) {
      ierror("Package not - Up-to-date\n");
      return 0;
    }
  }

  return 1;
}

################################################################

=item $package-E<gt>print_not_up_to_date()

Function to print_not_up_to_date information when destroying a package.

=cut

sub print_not_up_to_date {
  my $self = shift;

  my $status = 1;

  my @files = $self->status();
  if (! defined $files[0]) {
    print "No files\n";
    return 0;
  }

  foreach my $i (@files) {
    my ($dir, $file, $status, $reprev, $workdate) = @{$i};

    if ( ! ($status =~ /Up-to-date/ )) {
      print "$dir/$file - $status  $reprev $workdate\n";
      if ($status eq "Zombie") {
        print "  Somebody has removed this file, but you are trying to change it\n";
      }
      $status = 0;
    }
  }

  return $status;
}

##########################################################33

=item $package-E<gt>status()

Function to provide file status information.
This default one just prints an error, subclasses should provide their
own repository-specific version of this.

=cut

sub status {
  ierror("Cannot get status on unknown package type\n");
  return undef;
}

################################################################

=item $package-E<gt>get_change_set()

Return a list of files in this package that have been modified or added,
i.e. that are not up-to-date with the repository.  This is typically used
to determined the pruned set of regressions that must be run before committing.

=cut

sub get_change_set {
  my $self = shift;

  my @files;
  my @changeset = ();

  @files = $self->status();
  if (! defined $files[0]) {
    print "No files\n";
    return @changeset;
  }

  foreach my $i (@files) {
    my ($dir, $file, $status, $reprev, $workdate) = @{$i};

    if (! ($status =~ /Up-to-date/ )) {
      push(@changeset, "$dir/$file");
    }
  }
  return @changeset;
}

##########################################################33

=item $package-E<gt>files_status_unknown()

Return a list of files in this package with status "unknown",
i.e. those files that are not in the repository.  Ignore any files
in .cvsignore (or equivalent) lists, i.e. do not include them as
"unknown" files.

=cut

#
# This is a generic implementation based on a call to status(),
# subclasses can implement specialized versions for better performance!
#
sub files_status_unknown {
  my $self = shift;
  my @unknown_files = ();
  foreach my $statfile ( $self->status() ) {
    (my $directory, my $filename, my $status) = @$statfile;
    if ( $status eq 'Unknown' ) {
      my $path = $directory . '/' . $filename;
      push @unknown_files, $path;
    }
  }
  return @unknown_files;
}

##########################################################33

=item $package-E<gt>files_missing_from_repository( @file_list )

Return the subset of the given file list that does not
exist in the repository or have not been locally added.
The only exception is that if a file is in a .cvsignore pattern
(or equivalent for SVN or BitKeeper), do not add it to the returned list.

=cut

sub files_missing_from_repository {
  my $self = shift;
  my @file_list = @_;
  my @missing_files = ();
  foreach my $unknown ( $self->files_status_unknown() ) {
    if ( grep( { $_ eq $unknown } @file_list ) ) { push @missing_files, $unknown }
  }
  return @missing_files;
}

################################################################

=item $package-E<gt>configure()

Configure this package

=cut

################################################################

# Implemented in Asim::BuildTree

################################################################

=item $package-E<gt>build()

Build this package

=cut

################################################################

# Implemented in Asim::BuildTree

################################################################

=item $package-E<gt>install()

Install this package

=cut

################################################################

# Implemented in Asim::BuildTree

################################################################

=item $package-E<gt>commit($only_self) <and friends...>

Commit this package to the repository. If $only_self is set
only commit this package, otherwise commit dependent packages
as well.

=cut

################################################################

# Implemnted in Asim::Package::Commit

################################################################

=item $package-E<gt>release($version) <and friends...>

Create a new release of this package as $version

=cut

################################################################

# Implemnted in Asim::Package::Branch

################################################################

=item $package-E<gt>dump()

Textually dump the contents of the package

=cut

################################################################

# Implemented in Asim::Base


################################################################
#
# Internal error utility function
#
################################################################

sub iwarn {
  my $message = shift;

  print "Package: Warning: - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Package: Error - $message";

  return 1;
}


=back


=head1 BUGS

The new function would be more useful if it took either a package name
or a directory name. Or should it only take a directory name and you get
packages by name via packageDB->get_package();

The three-way diff function used in branch merges could stand some
improvements in the handling of directories that have been moved.

=head1 AUTHORS

Joel Emer, Roger Espasa, Artur Klauser, Pritpal Ahuja, Saila Parthasarathy, Carl Beckmann


=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2001-09

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
