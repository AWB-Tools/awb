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

#
# ATTENTION MAINTAINERS!
#
# This perl module contains routines that implement various commands
# in asim-shell.
#
# HOW TO ADD NEW COMMANDS TO ASIM-SHELL
#
# Let's say you want to add a new command "lubricate driveshaft" to
# asim-shell, such that you can type "lubricate driveshart" at the asim>
# prompt, or "asim-shell lubricate driveshaft" at the unix prompt.
# To add this command, do the following:
#
# 1. Update the array %COMPOUNDCOMMANDS in the file AsimShell.pm
#    in the directory above this one, if the new command is a compound command.
#    For examp,e if "lubricate driveshaft" and "lubricate hingemounts" were
#    both legal commands, you would add the following entry to this array:
#
#        lubricate => [ qw(driveshaft hingemounts) ],
#
# 2. If appropriate, add the command switches to the array %OPTIONS in the
#    file AsimShell.pm in the directory above this one. 
#
# 3. You should also update the file Completion.pm in this directory,
#    to deal with command-line completion.  Usually this involved developing
#    a list of alternative module, package, or other such names, and returning
#    something appropriate in the attempted_completion routine, usually by
#    calling the max_common function (follow the pattern in the existing code).
#
#    SEE THE NOTE in Completion.pm on package commands that take multiple
#    package names or the name "all" as arguments!!
#
# 4. Create a new subroutine in this file named "lubricate_driveshaft".
#    Note the underscore "_" in the name. The individual 'words' of the 
#    command will be passed in as the arguments of the function, i.e., as
#    @_. You can use GetOptions() to parse switches out of @_.
#
# 5. Finally, edit the Help.pm file in this directory, to add the command
#    to what is displayed in the "asim-shell help" command.
#

package AsimShell;
use warnings;
use strict;

use Asim;
use Asim::Inifile;

use Getopt::Long;
use File::Spec;
use File::Glob;
use File::Basename;
use Term::ReadLine;

#
# Control variables
#
our $show_warnings;


#
# Default data values
#
our $default_repositoryDB;
our $default_packageDB;
our $default_modelDB;
our $default_moduleDB;

our $default_package;
our $default_lock;
our $default_model;
our $default_benchmark;
our $default_module;


our $default_user = $ENV{USER};

our @failures = ();

################################################################
#
#  Implemetation of individual commands
#
################################################################
=head1 NAME

AsimShell:: Library of shell commands

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides the commands that are used by the Asim shell

=cut

################################################################


=head1 FUNCTIONS

The following public functions are supported:

=over 4

=item $success = AsimShell::cd([$dir]);

cd to $dir or root of workspace by default

=cut


sub cd {
  my $dir = shift || Asim::rootdir();

  chdir $dir 
    || shell_error("Cd to $dir failed\n") && return ();

  return 1;
}

sub pwd {

  system "pwd";
  return 1;
}

sub ls {
  my @args = @_;

  system( "ls " . join(" ", @args));

  return 1;
}


################################################################
#
# Workspace functions
#
################################################################

sub set_workspace {
  my $path = shift 
    || Asim::choose_filename("Enter base directory of workspace")
    || shell_error("Path must be specified to switch workspaces\n")  && return ();

  my $workspace;

  #
  # Check if there it is legal directory
  #
  $path = Asim::Util::expand_tilda($path);
  if (! -d $path) {
    shell_error("A legal path must be specified to switch workspaces\n");
    return ();
  }

  #
  # We're not set up to back out of the following, so failures are fatal.
  #

  Asim::open($path)
    || die("FAILURE: Couldn't switch to new workspace - shell internal state inconsistent - exit now!!!\n");

  set_packageDB()
    || die("FAILURE: Failed configuring package database - internal state inconsistent\n");


  # Update recfile with new workspace...
  # ... this directory is going to be tilda and symlink expanded. Is that what we want?

  $Asim::rcfile->workspace(Asim::rootdir());
  $Asim::rcfile->save();


  #
  # Do some workspace sanity checks
  #

  my $benchmarkdir = $Asim::default_workspace->benchmark_dir();

  if (! -d $benchmarkdir) {
    print "---------------------------------------------------------------------------\n";
    print "WARNING:\n";
    print "  The benchmark directory specified in your awb.config does not exist.\n";
    print "\n";
    print "  BENCHMARKDIR=$benchmarkdir\n";
    print "\n";
    print "  Update your awb.config file or you may not be able to run any benchmarks.\n";
    print "---------------------------------------------------------------------------\n";
  }

  $default_package = undef;

  $default_model = undef;
  $default_benchmark = undef;

  $default_module = undef;

  return 1;
}

sub create_workspace {
  new_workspace(@_);
}

sub new_workspace {
  my $path = shift
    || shell_error("Path must be specified to create a workspace\n")  && return ();

  my $workspace;

  $workspace = Asim::Workspace->new();

  $workspace->create($path)
    || shell_error("Workspace creation failed\n") && return ();

  set_workspace($path);
  return 1;
}

sub clone_workspace {
  my $workspacedir =  $Asim::default_workspace->rootdir();

  my $path;
  my $link = 0;

  local @ARGV = @_;

  my $status;
  my $command;

  # Parse options

  $status = GetOptions( "link!"      => \$link);

  return 0 if (!$status);

   $path = $ARGV[0];

   if (! defined($path) || $path eq "") {
      shell_error("Target path for clone must be specified\n");
      return undef;
   }

   if (-e $path) {
      shell_error("New workspace directory cannot already exist\n");
      return undef;
   }

   $command = "cp -r" . ($link?"l":"") . " $workspacedir $path";

   print "Cloning new workspace...\n";
   $status = system($command);
   if ($status) {
     shell_error("Clone of new workspace failed\n");
     return undef;
   }

    print "\n";
    print "To switch to the new workspace type:\n";
    print "    set workspace $path\n";
    print "\n";

    return 1;
}

sub rehash_workspace {
  my $workspace = get_workspace() || return ();

  print "Rehashing workspace....\n";

  $workspace->rehash();

  print "Done.\n";

  return 1;
}


sub edit_workspace {
  my $workspace = get_workspace() || return ();

  $workspace->edit()
    || shell_error("Workspace edit failed\n") && return ();

  if (Asim::choose_yes_or_no("Save workspace")) {
    $workspace->save()
      || shell_error("Package save failed\n") && return ();
  }

  return 1;
}


sub show_workspace {
  my $workspace = get_workspace() || return ();

  $workspace->dump();

  return 1;
}

################################################################
#
# Workspace utility functions
#
################################################################


sub get_workspace {
  my $path = shift;

  if (defined($path) && $path) {
    set_workspace($path)
    || shell_error("Workspace open failed ($path)\n") && return ();
  }

  return $Asim::default_workspace
    || set_workspace()
    || shell_error("No workspace or default workspace defined\n") && return ();

}


sub add_path {
  my $targetdir = shift;
  my $forceadd = shift;
  my $uniquefile;

  # Hack to find a unique file in the package...
  ($uniquefile) = grep(!/^CVS$/, glob("$targetdir/admin/packages/[a-z]*"));

  if (! $uniquefile || ! Asim::unresolve($uniquefile)) {
    print "\n";

    if (! (defined($forceadd) || $forceadd)) {
      print "New package does not appear to be in your ASIM SEARCHPATH  ($targetdir)\n";
      if (! Asim::choose_yes_or_no("Do you want to add it", "yes", "yes")) {
	#
	# No! - Well package is not going to be in path path so we cannot do anything more...
	#
        print "NOT adding $targetdir to ASIM SEARCHPATH\n";
	return 0;
      }
    }

    print "Adding $targetdir to ASIM SEARCHPATH\n";

    $Asim::default_workspace->prepend_path($targetdir);
    Asim::save()  || return ();
  }

  return 1;
}


################################################################
#
# Repository DB functions
#
################################################################

sub set_repositoryDB {
  my $packfile_path_string = shift;
  my @packfile_path;
  my $repositoryDB;

  if (defined($packfile_path_string)) {
    @packfile_path = split(":", $packfile_path_string);
  } else {
    @packfile_path = ();
  }

  $repositoryDB = Asim::Repository::DB->new(@packfile_path) 
      || shell_error("Failed to open repositoryDB files (" . join(",",@packfile_path) . "\n") && return undef;

  $default_repositoryDB = $repositoryDB;

  return $default_repositoryDB;
}

sub rehash_repositories {
  my $repositoryDB = get_repositoryDB() || return undef;

  print "Rehashing repository database...\n";

  $repositoryDB->rehash();

  print "Done.\n";

  return 1;
}


sub list_repositories {
  my $repositoryDB = get_repositoryDB() || return undef;

  $repositoryDB->dump();
  return 1;
}


sub get_repositoryDB {

  return $default_repositoryDB
    || shell_error("No repositoryDB or default repositoryDB defined\n") && return undef;
}


################################################################
#
# Repository functions
#
################################################################
#

sub create_repository {
  new_repository(@_);
}

sub new_repository {

  Asim::Repository->create();

}

sub show_repository {
  my $repositoryname  = shift;
  my $repositoryDB = get_repositoryDB() || return undef;

  my $repository = $repositoryDB->get_repository($repositoryname)
    || shell_error("Repository ($repositoryname) was missing or malformed in packfile\n") && return undef;

  $repository->dump();
}

################################################################
#
# Bundle functions
#
################################################################

sub list_bundles {

  print join("\n", $default_repositoryDB->bundle_directory()) . "\n";

  return 1;
}

#
# return the list of packages in a bundle.
# Common suborutine used  by many commands below,
# NOT an asim-shell command!!
# FIX FIX ?!? create a new perl class for dealing with bundles??
#
sub bundle_packages_ {
  my $bundlename = shift;
  my $bundleid;

  chomp($bundlename);
  ($bundlename, $bundleid) = (split("/", $bundlename), undef);
  $bundleid = "default" unless defined $bundleid;  

  my @flist = $default_repositoryDB->bundle_files($bundlename);
  (@flist == 0) && shell_error("No bundle file found for `$bundlename`!\n") && return ();
  
  my $inifile = Asim::Inifile->new();
  foreach my $file (@flist) {
      $inifile->include($file) || ($show_warnings && print("Cannot read file `$file` to collect bundle list!\n"));
  }
  
  my $items = $inifile->get($bundleid);
  if (!defined $items) {
      shell_error("Bundle id `$bundleid` not found!\n") && return ();
  }
  elsif ((defined $items->{Status}) && ($items->{Status} eq "Failure")) {
      if (!Asim::choose_yes_or_no("This bundle has failure status.  Do you want to get it", "no", "no")) {
	  return 0;
      }
  }
  my @packages = split(" ", $items->{Packages});
  (@packages == 0) && shell_error("Bundle id `$bundleid` did not result in any package!\n") && return ();

  return @packages;
}

sub checkout_bundle {
  my $user;
  my $addpath = 1;
  my $build = 1;
  my $golden;
  my $status;
  my $bundlename;
  local @ARGV = @_;
  my @packages;
  my $runstatus;

  # Parse options

  $status = GetOptions( "user=s"     => \$user,
		        "build!"     => \$build,
		        "addpath!"   => \$addpath,
			"golden"     => \$golden);
  return 0 if (!$status);

  # Bundle name is remaining argument
  
  $bundlename = shift @ARGV;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  #
  # Determine the list of packages to check out from the bundle file
  #
  @packages = bundle_packages_( $bundlename );
  
  print "Checking out: " . join(" ", @packages) . "\n";

  #
  # Regenerate the arguments for checkout_package
  #     This is not a very aesthetic way of doing this
  #     adding a subfunction to do the checkout would be better
  #
  my @args = ();

  $default_user = $user
    || Asim::choose("Repository user you want to do the checkout (e.g., your login name or anonymous)", $default_user)
    || shell_error("Must select user for checkout\n") && return ();

  push(@args, "--user=$default_user");

  if (defined($addpath)) {
    push(@args, ("--" . ($addpath?"":"no") . "addpath"));
  }

  # Do not build since we do all the builds later...
  push(@args, "--nobuild");

  my $pkg;
  my @plist;

  foreach my $b (@packages) {
    print "--- checkout package $b " . join(" ",@args) . "\n";
    $pkg = checkout_package($b, @args);

    if (! defined($pkg)) {
      _add_failure($b, "checkout failed");
      next;
    }

    push(@plist, $pkg);
  }

  if ($build) {
    configure_and_build_packages(@plist);
  }

  $status = _report_failures();

  return $status;
}

# Hugely redundant with checkout_bundle

sub use_bundle {
  my $addpath = 1;
  my $build = 1;
  my $status;
  my $bundlename;
  local @ARGV = @_;
  my @packages;
  my $runstatus;

  # Parse options

  $status = GetOptions( "build!"   => \$build,
		        "addpath!" => \$addpath);
  return 0 if (!$status);

  # Bundle name is remaining argument
  
  $bundlename = shift @ARGV;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  #
  # Determine the list of packages to check out from the bundle file
  #	
  @packages = bundle_packages_( $bundlename );
  
  print "Copying out: " . join(" ", @packages) . "\n";

  #
  # Regenerate the arguments for checkout_package
  #     This is not a very aesthetic way of doing this
  #     adding a subfunction to do the checkout would be better
  #
  my @args = ();

  if (defined($addpath)) {
    push(@args, ("--" . ($addpath?"":"no") . "addpath"));
  }

  # Do not build since we do all the builds later...
  push(@args, "--nobuild");

  my $pkg;
  my @plist;

  foreach my $b (@packages) {
    print "--- use package $b " . join(" ",@args) . "\n";
    $pkg = use_package($b, @args);

    if (! defined($pkg)) {
      _add_failure($b, "use failed");
      next;
    }

    push(@plist, $pkg);
  }

  if ($build) {
    configure_and_build_packages(@plist);
  }

  $status = _report_failures();

  return $status;
}

#
# update a bundle, being careful to update to the exact version specified in the bundle.
# On a branch or on the trunk, update to the latest version, but if a package is
# specified in the bundle with a certain CSN version number, keep exactly that version
# checked out (this is like CVS "sticky tags")
#
sub update_bundle {

  # Parse options
  my $build = 1;
  local @ARGV = @_;
  my $status = GetOptions( "build!" => \$build );
  return 0 if (!$status);

  # Bundle name is remaining argument
  my $bundlename = shift;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  # Determine the list of packages to check out from the bundle file
  my @packages = bundle_packages_( $bundlename );
  print "Updating: " . join(" ", @packages) . "\n";

  # Regenerate the arguments for update_package
  # Do not build since we do all the builds later...
  my @args = ("--nobuild");
  my @plist = ();
  foreach my $b (@packages) {
    print "--- update package $b " . join(" ",@args) . "\n";
    update_package($b, "--noreport", @args);
    (my $name, my $pkgvers) = split('/',$b);
    push(@plist, get_package($name));
  }

  if ($build) {
    configure_and_build_packages(@plist);
  }

  $status = _report_failures();

  return $status;
}


sub show_bundle {
  my $bundlename = shift;
  my @packages;
  my $runstatus;

  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }

  #
  # Determine the list of packages to check out from the bundle file
  #	      
  @packages = bundle_packages_( $bundlename );

  print join("\n", @packages) . "\n";

  return 1;
}


sub choose_bundle {
  # package and bundle name syntax are the same so choose_package works...
    my @bundles = $default_repositoryDB->bundle_directory();
    my @fulllist;
    foreach my $bundle (@bundles) {
	my @bundleids = $default_repositoryDB->bundle_ids($bundle);
	foreach my $element (@bundleids) {
	    push(@fulllist, "$bundle/$element");
	}
    }
    
    return choose_package(@fulllist);
}


################################################################
#
# Package DB functions
#
################################################################


sub set_packageDB {
  my @dups;

  $default_packageDB = Asim::packageDB()
    || shell_error("No packageDB or default packageDB defined\n") && return ();

  @dups = $default_packageDB->duplicates();
  if ($show_warnings && @dups) {
    print "---------------------------------------------------------------------------\n";
    print "WARNING:\n";
    print "  The following packages appear multiple times in your ASIM search path:\n";
    print "\n";
    print "  " . join(" ", @dups) . "\n";
    print "\n";
    print "  Having a package appear multiple times in your ASIM search path can lead\n";
    print "  to unexpected behavior, and it is recommended you edit your awb.config\n";
    print "  file to remove the duplicated packages.\n";
    print "---------------------------------------------------------------------------\n";
  }

  return $default_packageDB;
}

sub rehash_packages {

  print "Rehashing package database...\n";

  $default_packageDB->rehash();

  print "Done.\n";

  return 1;
}

sub list_packages {

  $default_packageDB->dump();

  return 1;
}

sub baseline_packages {

  # Parse options
  my $force_csn = 0;
  local @ARGV = @_;
  my $status = GetOptions( "csn!" => \$force_csn );
  return 0 if (!$status);

  foreach my $name ( $default_packageDB->directory() ) {
    my $package = get_package($name);
    print $name, '/', $package->baseline_tag( $force_csn ), ' ';
  }
  print "\n";
  return 1;
}


sub get_packageDB {
  return $default_packageDB;
}



################################################################
#
# Package functions
#
################################################################


#
# Package constructors
#

sub create_package {
  new_package(@_);
}

sub new_package {
  my $name = shift
    || Asim::choose("Package name:")
    || shell_error("Package name must be specified\n") && return ();

  my $dir = shift;
  my $package;

  print "Creating package $name...\n";

  $package = Asim::Package::create($name, $dir) 
    || shell_error("Package creation failed\n") && return ();

  if (Asim::choose_yes_or_no("Do you want to run regression tests on this package")) {
    $package->create_regtest();
  }

  #
  # Check and if it is necessary to add package to search path...
  #     (return if package is not going to be on the path)
  #
  add_path($package->location(), 1);

  #
  # We've got a new package now, so it might be good to rehash the packageDB
  #
  $default_packageDB->rehash();

  #
  # Switch to new package
  #
  $default_package = $package;

  return 1;
}

#######################################################
sub checkout_package {
  my $user = undef;
  my $build = 1;
  my $addpath = 1;
  my $status;
  local @ARGV = @_;

  #
  # Parse options
  #
  $status = GetOptions( "user=s"   => \$user,
		        "build!"   => \$build,
		        "addpath!" => \$addpath);
  return undef if (!$status);

  #
  # Repository name is remaining argument
  #
  my $repositoryname = shift @ARGV
    || choose_repository()
    || shell_error("Repository name must be selected!\n") && return undef;


  #
  # Get repository object, and determine checkout directory
  #
  my $repository;

  $repository = $default_repositoryDB->get_repository($repositoryname) 
    || shell_error("Repository ($repositoryname) was missing or malformed in packfile\n") && return undef;

  my $targetdir = $repository->checkoutdir()
    || shell_error("Could not determine target for checkout\n") && return undef;

  #
  # Check if targetdir already exists
  #
  if (-e $targetdir) {
    print "A package already exists in $targetdir and may have modified files in it!\n";

    if (! Asim::choose_yes_or_no("Do you want to overwrite package at $targetdir", "no", "yes")) {
      return $default_packageDB->get_package_by_dirname($targetdir);
    }
  }

  #
  # Check if package of this type already exists in searchpath
  #

  my $target_packagename = $repository->packagename();

  if (! -e $targetdir && defined($default_packageDB->get_package($target_packagename))) {
    print "A package of this name ($target_packagename) already exists in your searchpath!\n";
    my $q = "Do you want to remove the reference to this package from the searchpath";

    if (Asim::choose_yes_or_no($q, "yes", "yes")) {
      my $workspace = $Asim::default_workspace;

      $workspace->remove_path_by_packagename($target_packagename);
      $workspace->save();
    }
  }

  #
  # Determine user to do checkout
  #
  $default_user = $user
    || Asim::choose("Repository user you want to do the checkout (e.g., your login name or anonymous)", $default_user)
    || shell_error("Must select user for checkout\n") && return undef;


  #
  # Do the checkout and get package object for checked out package
  #
  $repository->checkout($default_user) 
    || shell_error("Could not check out package ($repositoryname)\n") && return undef;

  my $package = $default_packageDB->get_package_by_dirname($targetdir)
    || shell_error("No legal package at $targetdir\n") && return undef;

  #
  # Optionally build the package
  #
  if (defined($build) && $build ) {
    $package->configure();
    $package->build();
  }

  #
  # Check and if it is necessary to add package to search path...
  #     (return if package is not going to be on the path)
  #
  if ($addpath) {
    add_path($targetdir, $addpath) || return $package;
  }

  #
  # We've got a new package now, so it might be good to rehash the packageDB
  #
  $default_packageDB->rehash();

  #
  # Return the package we checked out
  #
  return $package;
}


#
# Add a publically installed package to your workspace
#
#   Code is too redunant with checkout_package...
#
sub use_package {
  my $build = 1;
  my $addpath = 1;
  my $status;
  local @ARGV = @_;

  #
  # Parse options
  #
  $status = GetOptions( "build!"   => \$build,
		        "addpath!" => \$addpath);
  return undef if (!$status);

  #
  # Repository name is remaining argument
  #
  my $repositoryname = shift
    || choose_public_package()
    || shell_error("Failed to find public package\n") && return ();
  my $recurse = shift || 1;

  #
  # Get repository object, and determine checkout directory
  #
  my $repository;

  $repository = $default_repositoryDB->get_public_repository($repositoryname)
    || shell_error("Repository ($repositoryname) was missing or malformed in packfile\n") && return undef;

  my $targetdir = $repository->checkoutdir()
    || shell_error("Could not determine target for checkout\n") && return undef;

  #
  # Check if targetdir already exists
  #
  if (-e $targetdir) {
    print "A package already exists in $targetdir and may have modified files in it!\n";

    if (! Asim::choose_yes_or_no("Do you want to overwrite package at $targetdir", "no", "yes")) {
      return $default_packageDB->get_package_by_dirname($targetdir);
    }
  }

  #
  # Do the checkout and get package object for checked out package
  #
  $repository->checkout()
    || shell_error("Could not add package ($repositoryname)\n") && return undef;

  my $package = $default_packageDB->get_package_by_dirname("${targetdir}")
    || shell_error("No legal package at ${targetdir}/HEAD\n") && return undef;

  #
  # Optionally build the package
  #
  if (defined($build) && $build ) {
    $package->configure();
    $package->build();
  }

  #
  # Check and if it is necessary to add package to search path...
  #     (return if package is not going to be on the path)
  #
  if ($addpath) {
    add_path("${targetdir}", $addpath) || return $package;
  }

  #
  # We've got a new package now, so it might be good to rehash the packageDB
  #
  $default_packageDB->rehash();

  #
  # Return the package we checked out
  #
  return $package;
}

sub delete_package {

  while ( my $name = shift ) {
    my $package = get_package($name) || return ();

    if (! $package->isprivate() ) {
      shell_error("Cannot delete public packages\n");
      return ();
    }

    $package->print_not_up_to_date();

    if (Asim::choose_yes_or_no("Really delete checked out package $name","n","y")) {
      $package->destroy();
      rehash_packages();
    }
  }

  return 1;
}



#
# Package path operations
#
sub add_public_package {
  my $file = shift
    || choose_public_package()
    || shell_error("Failed to find public package\n") && return ();
  my $recurse = shift || 1;

  $file = Asim::Packagedir() . "/$file";

  add_package($file, $recurse) || return ();

  return 1;
}

#
# This used to be used by add_public_package, but may now be broken...
#
sub add_package {
  my $file = shift
    || Asim::choose_filename("Select package directory to add to path")
    || shell_error("Failed find package directory\n") && return ();
  my $recurse = shift || 1;

  $file =  File::Spec->rel2abs($file);

  if (! -e "$file/admin") {
      shell_error("There doesn't appear to be a legal package at $file\n") && return ();
  }

  $Asim::default_workspace->prepend_path($file);
  Asim::save() || return ();

  rehash_packages();

  if ($recurse) {
    add_dependent_packages($file);
  }

  return 1;
}

sub add_dependent_packages {
  my $pname = shift;
  my $package;

  # print "Trying to open $pname\n";
  # HACK - open the package based at $pname

  my @packages = grep(!/CVS/, glob("$pname/admin/packages/[a-z]*"));

  if (@packages == 0) {
    return undef;
  }

  $package = get_package(basename($packages[0])) 
    || return undef;

  foreach my $d ($package->dependencies()) {
    my $status = check_dependent_package($d);

    if (defined($status) && $status eq 0) {
      add_dependent_packages($d);
    }
  }

  return 1;
}

#
# configure, make, build, install, or clean one or more packages,
# returning 1 if all succeeded.  These all take as arguments a list
# of package names, or "all" or "*" may be the first and only argument
# and denotes to act on all pacages in the workspace.
#
sub configure_package {
  # get the list of packages
  my @package_names = _expand_package_names(@_);

  my $status;

  while ( my $name = shift @package_names) {
    _print_package_start("configure", $name);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("configure", $name, "(No such package)");
      next;
    }

    # TBD: Should we filter out non-private packages?

    $package->configure() || _add_failure($name, "configure failed");

    _print_package_finish("configure", $name);
  }

  $status = _report_failures();

  return $status;
}

sub make_package {
  # get the list of packages
  my @package_names = _expand_package_names(@_);

  my $status;

  while ( my $name = shift @package_names) {
    _print_package_start("make", $name);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("make", $name, "(No such package)");
      next;
    }

    # TBD: Should we filter out non-private packages?

    $package->build() || _add_failure($name, "make failed");

    _print_package_finish("make", $name);
  }

  $status = _report_failures();

  return $status;
}

sub build_package {
  # get the list of packages
  my @package_names = _expand_package_names(@_);

  my @all_packages = ();
  my $status;

  while ( my $name = shift @package_names) {

    # No start/finish brackets here...

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      next;
    }

    push(@all_packages, $package);
  }

  configure_and_build_packages(@all_packages);

  $status = _report_failures();

  return $status;
}

sub install_package {
  my    $target = "install";
  local @ARGV   = @_;

  # Parse options

  my $status = GetOptions( "source" => sub {$target = "install-src-public"});
  return undef if (!$status);

  # get the list of packages
  my @package_names = _expand_package_names(@ARGV);

  while ( my $name = shift @package_names ) {
    _print_package_start("install", $name);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("install", $name, "No such package");
      next;
    }

    $package->install($target) || _add_failure($name, "install failed");

    _print_package_finish("install", $name);
  }

  $status = _report_failures();

  return $status;
}

sub clean_package {
  # get the list of packages
  my @package_names = _expand_package_names(@_);

  my $status;

  while ( my $name = shift @package_names) {

    _print_package_start("clean", $name);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("clean", $name, "(No such package)");
      next;
    }

    if ( ! $package->isprivate() ) {
      print "Cannot clean a non-private package\n";
      _add_failure($name, "not private");
      _print_package_finish("clean", $name, "(Not private)");
      next;
    }

    $package->clean()              || _add_failure($name, "clean failed");

    _print_package_finish("clean", $name);
  }

  $status = _report_failures();

  return $status;
}

#
# Internal utility function...
#
sub configure_and_build_packages {
  my @plist = @_;
  my $status;

  @plist = sort { $a->buildorder() <=> $b->buildorder() } @plist;

  for my $p (@plist) {
    _print_package_start("configure/build", $p->name(), "(" .$p->buildorder().")");

    # TBD: Should we filter out non-private packages?

    $p->configure() || _add_failure($p->name(), "configure failed") && next;
    $p->build()     || _add_failure($p->name(), "make failed");

    _print_package_finish("configure/build", $p->name());
  }

  # Note we don't _report_failures() here. The caller must do that.

  if ($#failures >= 0) {
    return undef;
  }

  return 1;
}

###################################################################

=item $success = AsimShell::check_dependent_package($needed);

Utility function to check for existence of package $needed (specified
as text string "package/tag". If package is not in search path, offer
to add the public copy of the package or check it out. Returns 1 if 
package already existed, 0 if it was made available or () on a failure.

=cut

sub check_dependent_package {
  my $needed = shift;

  my $missing_package = 0;
  my $version_mismatch = 0;

  if ($show_warnings) {
    print("Checking package dependency: $needed\n");
  }

#
# Separate package name of form 'name/tag' into 'name' and 'tag'
#
  $needed =~ /([^\/]*)(\/(.*))*/;
  my $needed_package = $1;
  my $needed_tag = $3 || "HEAD";

#
# Check if a needed package is among the active packages
#
  my @active_packages = $default_packageDB->directory();

  if (!grep /^$needed_package$/, @active_packages) {
    #
    # The needed package is not among the active packages
    #
    $missing_package = 1;
    print "A package ($needed) we depend on is not available\n";
  } else {
    #
    # The needed package is in the active packages, 
    # but we still need to check if needed tag matches
    #
    my $cur_package = $default_packageDB->get_package($needed_package)
       || shell_error("No such package $needed_package\n") && return ();
    my $cur_tag = $cur_package->tag();

    if ($cur_tag eq $needed_tag) {
      return 1;
    }

    $version_mismatch = 1;
    print "The dependent package ($needed_package) has the wrong tag --- ";
    print "looking for $needed_tag found $cur_tag\n";
  }

  #
  # Offer to check out missing package
  #

  if (Asim::choose_yes_or_no("Would you like to use the global version","no","no")) {
    add_public_package($needed, 0) && return 0;
  }

  if (Asim::choose_yes_or_no("Would you like to check it out","no","yes")) {
    checkout_package($needed) || return ();
    return 0;
  }

  return ();
}



#
# choose_repository()
#
sub choose_repository() {

  return choose_package($default_repositoryDB->directory());
}


#
# Maybe getting the list of public packages should be part of Package::DB???
#
sub choose_public_package {

  return choose_package($default_repositoryDB->public_directory());
}


#
# choose_package
#
#   Let user select a repository of the form <name>[/<version>]
#   from the given list
#

sub choose_package {
  my @packagelist = @_;

  my %p;
  my $package;
  my @v;
  my $version;

  # Determine the distinct package names (independent of version)

  foreach my $r (@packagelist) {
    my $t = $r;
    $t =~ s/\/.*//;
    $p{$t} = 1;
  }


  $package = Asim::choose_name("Pick package", sort keys %p )
    || return undef;

  # Get the versions of the selected package

  foreach my $r (@packagelist) {
    if ($r =~ /^${package}\// || $r =~ /^${package}$/ ) {
      push @v, $r;
    }
  }

  # If there's only one matching package - use it

  if ( @v == 1) {
    return $v[0];
  }

  # Otherwise, ask user which version of the package

  $version = Asim::choose_name("Pick version", sort @v)
    || return undef;

  return "$version";
}

#
# Package manipulations
#

sub set_package {
  my $name = shift;

  $default_package = undef;
  $default_package = get_package($name);

  return $default_package;
}

sub unset_package {
  $default_package = undef;
}



sub regtest_package {
  my $name = shift;
  my $package = get_package($name) || return ();

  my $location = $package->location();

  system("cd $location; regtest.pl");

  return 1;
}


sub edit_package {
  my $name = shift;
  my $package = get_package($name) || return ();

  $package->edit()
    || shell_error("Package edit failed\n") && return ();

  if (Asim::choose_yes_or_no("Save package")) {
    $package->save()
      || shell_error("Package save failed\n") && return ();
  }

  return 1;
}

#
# Show information about one or more packages

sub show_package {
  # get the list of packages
  my @package_names = _expand_package_names(@_);

  my $status;

  foreach my $name (@package_names) {
    _print_package_start("show", $name);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("show", $name, "(No such package)");
      next;
    }

    $package->dump();

    _print_package_finish("show", $name);
    if ( @_ ) { print "\n" }
  }

  $status = _report_failures();

  return $status;
}

#
# update one or more packages, returning 1 if all succeeded.
#
sub update_package {
  my $build = 1;
  my $report = 1;

  my @build_list = ();
  local @ARGV = @_;

  # Parse options

  my $status = GetOptions( "build!"  => \$build,
                           "report!" => \$report);
  return undef if (!$status);

  # get the list of packages
  my @package_names = _expand_package_names(@ARGV);

  while ( my $pkgspec = shift @package_names) {
    # each package in the list could be a simple name,
    # or a name and version or branch identifier, like this:
    #   <pkgname>
    #   <pkgname>/HEAD
    #   <pkgname>/CSN-<pkgname>-<number>
    #   <pkgname>/<branchname>
    my $name;
    my $version;
    ($name,$version) = split('/',$pkgspec);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      next;
    }

    _print_package_start("update", $name, "(" . $package->type() . " repository)");

    if (!$package->isprivate()) {
      print("Cannot update a non-private package\n");
      _print_package_finish("update", $name, "(skipped)");
      next;
    }

    # update the package to the specified version
    $package->update($version) || _add_failure($package->name(),"update failed");

    if ($build) {
      # if we are also building, add the package to the list of ones to build
      push @build_list, $package;
    }

    _print_package_finish("update", $name);
  }

  # if updating more than one package, and building,
  # do the builds after you have checked everything out.
  while ( my $package = shift @build_list ) {
    _print_package_start("configure/build", $package->name());

    $package->configure() || _add_failure($package->name(),"configure failed");
    $package->build()     || _add_failure($package->name(),"make failed");

    _print_package_finish("configure/build", $package->name());
  }

  if ($report) {
    $status = _report_failures();
  } else {
    $status = ($#failures < 0) || undef;
  }

  return $status;
}



#
# print status of files in (a) package(s),
# i.e. modified, needs-update, etc.
#
sub status_package {
  local @ARGV = @_;
  my $verbose = 0;

  # Parse options
  my $status = GetOptions( "verbose!" => \$verbose) || return undef;

  # get the list of packages
  my @packages = _expand_package_names(@ARGV);

  # print status for each package in turn
  foreach my $name ( @packages ) {

    _print_package_start("status", $name);

    my $package = get_package( $name );

    if (! defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("status", $name, "(No such package)");
      next;
    }

    if ( ! $package->isprivate() ) {
      # shared packages, incorporated by reference:
      printf("Package %s is a shared package\n", $name );
      _print_package_finish("status", $name, "(Skipped)");
      next;
    }

    # private packages, in the user's workspace:
    foreach my $fstate ( $package->status() ) {

      if (! defined($fstate)) {
        print("Could not determine status - skipping package\n");
        _add_failure($package->name(), "could not determine status");
        next;
      }

      (my $dir, my $file, my $state, my $version) = @$fstate;

      if ( $verbose || $state ne 'Up-to-date' ) {
        if ( $dir ) { $dir .= '/'; }
        printf("%-16s  %-6s  %s%s\n", $state, $version, $dir, $file);
      }
    }

    _print_package_finish("status", $name);
  }

  $status = _report_failures();

  return $status;
}

#
# commit one or more packages, returning 1 if all succeeded
#
sub commit_package {
  my $deps = 1;
  my $commitlog_file = undef;

  local @ARGV = @_;

  # Parse options

  my $status = GetOptions( "dependent!"     => \$deps,
			   "commitlog=s"    => \$commitlog_file,
			 );
  return undef if (!$status);

  if ((Asim::mode() eq "batch") && (!defined $commitlog_file)) {
      print "Batch commit requires commitlog file argument.\n";
      return 0;
  }
  elsif ((Asim::mode() eq "batch") && (! -f $commitlog_file)) {
      print "Batch commit requires a valid commitlog file.\n";
      return 0;
  }

  # Package names are remaining arguments
  while ( my $name = shift @ARGV ) {

    # handle special case of "all packages"
    if (defined($name) && ($name eq "*" || $name eq "all")) {
      return commit_all_packages($deps, $commitlog_file);
    }

    my $package = get_package($name) || return undef;
    $package->commit(!$deps, $commitlog_file) || return undef;
  }
  
  return 1;
}


# Commits all packages in workspace

sub commit_all_packages {
  my $deps = shift;
  my $commitlog_file = shift;

  my @plist;

  #
  # Generate list of checked-out packages
  #
  foreach my $pname ($default_packageDB->directory()) {
    my $p = get_package($pname);

    if ($p->isprivate()) {
      push(@plist, $p);
    } else {
      print "Skipping public package $pname\n";
    }
  }

  #
  # Do a commit on each package
  #
  for my $p (@plist) {
        print "Commiting package " . $p->name() . "\n\n";
	$p->commit(!$deps, $commitlog_file) || return undef;
	print "\n";
  }

  return 1;
}

sub release_package {
  my $name = shift if (defined($_[1]));
  my $package = get_package($name) || return ();
  my $version = shift
    || Asim::choose("Enter name for release (format: <major#>.<minor#>")
    || shell_error("Version for release must be specified\n") && return ();

  return $package->release($version)
    || shell_error("Release of package failed\n") && return ();
}


# TODO: Awful argument convention...

sub branch_package {
  my $name = shift if (defined($_[1]));
  my $package = get_package($name) || return ();
  my $bname = shift
    || Asim::choose("Enter name for branch")
    || shell_error("Name for branch must be specified\n") && return ();

  #
  # Create a new branch
  #
  $package->branch($bname)
    || shell_error("Branch of package failed\n") && return ();

  #
  # We've got a new package now, so it might be good to rehash the packageDB
  #
  $default_packageDB->rehash();

  return 1;
}

sub merge_package {
  my $name = shift;
  my $package = get_package($name) || return ();
  my $csn = shift;

  return $package->merge($csn);
}


sub cvs_package {
  my $name = shift;
  my $package = get_package($name) || return ();
  my @command = @_;

  return $package->cvs("@command");
}

sub svn_package {
  my $name = shift;
  my $package = get_package($name) || return ();
  my @command = @_;

  return $package->svn("@command");
}

sub lock_package {
  my $name = shift;
  my $package = get_package($name) || return ();
  my $status;

  $status = $package->acquire_lock();
  print "Status: " . $package->status_lock() . "\n";

  return $status
}


sub unlock_package {
  my $name = shift;
  my $package = get_package($name) || return ();
  my $status;

  $status = $package->release_lock();
  print "Status: " . $package->status_lock() . "\n";

  return $status
}


sub cd_package {
  my $name = shift;
  my $package = get_package($name) || return ();

  my $location = $package->location();

  chdir $location || shell_error("Cd to $location failed\n");
}

sub shell_package {
  my $name = shift;
  my $package = get_package($name) || return ();
  my $location = $package->location();

  print "\n";
  print "Current working directory is: $location\n";
  print "\n";

  system "(cd $location; $ENV{SHELL} -i)";
  return 1;
}



#
# Package utility functions
#

#
# Expand a list a package name arguments honoring 'all' and '*'
# TBD: If the list is empty we could ask the user for a name
# TBD: We could do more error checking here
#
sub _expand_package_names {

  # Check for empty list

  if (! @_ ) {
    my $package = get_package() || return ();

    return ($package->name());
  }

  # Check if 'all' or '*'

  if ( $#_ == 0 && ($_[0] eq 'all' || $_[0] eq '*' )) {
    return ( $default_packageDB->directory() );
  }

  # Default just use what we were given

  return (@_);
}


sub _print_package_start {
  my $action = shift;
  my $name = shift;
  my $comment = shift || "";

  print "----------------------------------------------\n";
  print " Starting $action on $name $comment\n";
  print "----------------------------------------------\n";
}

sub _print_package_finish {
  my $action = shift;
  my $name = shift;
  my $comment = shift || "";

  print "----------------------------------------------\n";
  print " Finishing $action on $name $comment\n";
  print "----------------------------------------------\n";
}


#
# Get a package based on its name
# TBD: The behavior of this method should match _expand_package_names()
#
sub get_package {
  my $name = shift;
  my $package = undef;

  if (!defined($name) && !defined($default_package)) {
    $name = Asim::choose_name("Select package", $default_packageDB->directory())
    || shell_error("No package or default package defined\n") && return ();
  }

  if (defined($name) && $name) {
    $package = $default_packageDB->get_package($name)
      || shell_error("Failed to open package ($name)\n") && return ();
  }

    return $package 
      || $default_package
      || shell_error("No package or default package defined\n") && return ();
}



################################################################
#
# LockDB  functions
#
################################################################


sub rehash_locks {
  my $lockDB = Asim::Lock->new();

  print "Rehashing locks....\n";

  $lockDB->rehash();

  print "Done.\n";

}

sub list_locks {
  my $lockDB = Asim::Lock->new();

  foreach my $n ($lockDB->directory()) {
    my $l = Asim::Lock->new($n);
    print "$n - " . $l->status_lock() . "\n";
  }
}


################################################################
#
# Lock  functions
#
################################################################
sub create_lock {
  new_lock(@_);
}

sub new_lock {
  my $name = shift
    || Asim::choose("Lock name:")
    || shell_error("Lock name must be specified\n") && return ();

  my $lock;

  print "Creating lock $name...\n";

  $lock = Asim::Lock->new($name)
    || shell_error("Lock instantation failed\n") && return ();

  $lock->create_lock()
    || shell_error("Lock creation failed\n") && return ();

  #
  # Switch to new lock as default
  #
  $default_lock = $lock;

  return 1;
}

sub delete_lock {
  my $name = shift;
  my $lock = get_lock($name) || return ();
  my $status;

  $status = $lock->delete_lock();
  print "Status: $status\n";

  return $status;
}


#
# Lock manipulations
#

sub set_lock {
  my $name = shift;

  $default_lock = undef;
  $default_lock = get_lock($name);

  return $default_lock;
}

sub unset_lock {
  $default_lock = undef;
}

sub lock_lock {
  my $name = shift;
  my $lock = get_lock($name) || return ();
  my $status;

  $status = $lock->acquire_lock();
  print "Status: " . $lock->status_lock() . "\n";

  return $status;
}


sub unlock_lock {
  my $name = shift;
  my $lock = get_lock($name) || return ();
  my $status;

  $status = $lock->release_lock();
  print "Status: " . $lock->status_lock() . "\n";

  return $status;
}


sub show_lock {
  my $name = shift;
  my $lock = get_lock($name) || return ();

  $lock->dump();

  return 1;
}


#
# Lock utility functions
#

sub get_lock {
  my $name = shift;
  my $lock;

  if (!defined($name) && !defined($default_lock)) {
    $name = choose_lock()
      || shell_error("Failed to get lock\n") && return ();
  }

  if (defined($name) && $name) {
    $lock = Asim::Lock->new($name)
      || shell_error("Failed to get lock ($name)\n") && return ();
  }

  return $lock
    || $default_lock 
    || shell_error("No lock defined\n") && return ();

}


sub choose_lock {
  my $lockDB = Asim::Lock->new();
  my @locks = ($lockDB->directory());

  return Asim::choose_name("Select lock", @locks);
}


################################################################
#
# ModelDB functions
#
################################################################


sub set_modelDB {
  my $file = shift;

  $default_modelDB = undef;
  $default_modelDB = get_modelDB($file);

  return $default_modelDB;
}

sub rehash_models {
  my $file = shift;
  my $modelDB = get_modelDB($file) || return();

  print "Rehashing model database...\n";

  $modelDB->rehash();

  print "Done.\n";

  return 1;
}

sub list_models {
  my $file = shift;
  my $modelDB = get_modelDB($file) || return();

  $modelDB->dump();

  return 1;
}


sub get_modelDB {
  my $modelDB;

  if (!defined($default_modelDB)) {
    $modelDB = Asim::Model::DB->new()
      || shell_error("No modelDB defined\n") && return ();
  }

  return $modelDB
    || $default_modelDB
    || shell_error("No modelDB defined\n") && return ();

}


################################################################
#
# Model functions
#
################################################################


sub set_model {
  my $file = shift;

  $default_model = undef;
  $default_model = get_model($file);

  return $default_model;
}


sub unset_model {
  $default_model = undef;
}

sub create_model {
  new_model(@_);
}

sub new_model {
  my $file = shift;
  my $model = Asim::Model->new();

  $model->edit();
  $model->dump();

  $default_model = $model;

  return 1;
}

sub edit_model {
  my $file = shift;
  my $model = get_model($file) || return ();

  $model->edit();
# _edit_object($model);

  # Put a save of the object here...
  return 1;
}

sub clean_model {
  my $file;
  my $builddir="";
  my $model;
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir);

  # Model is remaining argument

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  $status = $model->clean("--builddir" => "$builddir");

  return $status;
}

sub nuke_model {
  my $file;
  my $builddir="";
  my $model;
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir);

  # Model is remaining argument

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  $status = $model->nuke("--builddir" => "$builddir");

  return $status;
}

sub ncfg_model {
  my $file;
  my $builddir="";
  my $model;
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir);

  # Model is remaining argument

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  $status = $model->nuke("--builddir" => "$builddir");
  return if (! $status);

  $status = $model->configure("--builddir" => "$builddir");

  return $status;
}

sub configure_model {
  my $file;
  my $builddir="";
  my $model;
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir);

  # Model is remaining argument

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  $status = $model->configure("--builddir" => "$builddir");

  return $status;
}

sub build_model {
  my $file;
  my $model;
  my $options="";
  my $builddir="";
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir,
                        "buildopt=s"  => \$options,
                        "options=s"  => \$options);

  # Model is remaining option

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  # Build it...

  $status = $model->build("--builddir" => "$builddir",
                          "--buildopt" => "$options");

  return $status;
}

sub make_model {
  build_model(@_);
}

#
# Setup model - parameter defaulting
#
#   setup_model                 -> setup_model ($default_model || prompt user) $default_benchmark
#   setup_model BENCHMARK       -> setup_model ($default_model || prompt user) BENCHMARK
#   setup_model MODEL BENCHMARK -> setup MODEL BENCHMARK
#

sub setup_model {
  my $model_file = "";
  my $benchmark_file;

  my $model;
  my $benchmark;
  my $builddir = "";
  my $rundir = "";

  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir,
                        "rundir=s"  => \$rundir);

  # Model and/or benchmark are remaining argv values

  if (defined $ARGV[1] && defined $ARGV[2]) {
    $model_file = shift @ARGV;
  }
  $benchmark_file = shift @ARGV;

  $model = get_model($model_file) || return ();
  $benchmark = get_benchmark($benchmark_file) || return ();

  print "Trying to set up benchmark $benchmark\n";

  $status = $model->setup($benchmark,
                          "--builddir" => "$builddir",
                          "--rundir" => "$rundir");

  return $status;
}

#
# Run model - parameter defaulting
#
#   run_model                 -> run_model ($default_model || prompt user) $defaul_benchmark
#   run_model BENCHMARK       -> run_model ($default_model || prompt user) BENCHMARK
#   run_model MODEL BENCHMARK -> run MODEL BENCHMARK
#

sub run_model {
  my $model_file = "";
  my $benchmark_file;

  my $model;
  my $benchmark;
  my $builddir = "";
  my $rundir = "";
  my $options = "";

  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir,
                        "rundir=s"  => \$rundir,
                        "runopt=s" => \$options,
                        "options=s" => \$options);

  # Model and/or benchmark are remaining argv values

  if (defined $ARGV[1] && defined $ARGV[2]) {
    $model_file = shift @ARGV;
  }
  $benchmark_file = shift @ARGV;

  $model = get_model($model_file) || return ();
  $benchmark = get_benchmark($benchmark_file) || return ();

  print "Trying to run benchmark $benchmark\n";

  $status = $model->run($benchmark,
                        "--builddir" => "$builddir",
                        "--rundir" => "$rundir",
                        "--runopt" => "$options");

  return $status;
}



sub cd_model {
  my $file = shift;
  my $model = get_model($file) || return ();

  my $location = $model->build_dir();

  chdir $location || shell_error("Cd to $location failed\n");
}


sub show_model {
  my $file = shift;
  my $model = get_model($file) || return ();

  $model->dump();

  return 1;
}


sub get_model {
  my $file = shift;
  my $model;

  if (!defined($file) && !defined($default_model)) {
    $file =  Asim::choose_filename("Select model configuration file")
      || shell_error("Failed to open model file\n") && return ();
  }

  if (defined($file) && $file) {
    $model = Asim::Model->new($file)
      || shell_error("Failed to open model file ($file)\n") && return ();

    $model = check_model_dependencies($model)
      || shell_error("Failed to satisfy model dependencies\n") && return ();
  }

  return $model
    || $default_model
    || shell_error("No model or default model defined\n") && return ();

}


sub check_model_dependencies {
  my $model = shift;

  my @needed_packages = $model->dependencies();

  foreach my $needed (@needed_packages) {
    my $status = check_dependent_package($needed);

    if (defined($status) && $status eq 0) {
      $model = $model->open($model->filename());
    }
  }

  return $model;
}

################################################################
#
# Benchmark functions
#
################################################################


sub set_benchmark {
  my $file = shift;

  $default_benchmark = get_benchmark($file);

  return $default_benchmark;
}

sub get_benchmark {
  my $file = shift;
  my $benchmark;

  if (!defined($file) && !defined($default_benchmark)) {
      shell_error("Sorry interactive benchmark selection unavailable\n") && return;
  }

  if (defined($file) && $file) {
    $benchmark =  $file
      || shell_error("Cannnot find benchmark $file") && return ();
  }

  return $benchmark
    || $default_benchmark
    || shell_error("No benchmark or default benchmark defined\n") && return ();

}

################################################################
#
# ModuleDB functions
#
################################################################



sub set_moduleDB {
  my $file = shift;

  $default_moduleDB = undef;
  $default_moduleDB = get_moduleDB($file);

  return $default_moduleDB;
}

sub rehash_modules {
  my $file = shift;
  my $moduleDB = get_moduleDB($file) || return();

  print "Rehashing module database...\n";

  $moduleDB->rehash();

  print "Done.\n";

  return 1;
}

sub list_modules {
  my $file = shift;
  my $moduleDB = get_moduleDB($file) || return();

  $moduleDB->dump();

  return 1;
}


sub get_moduleDB {
  my $file = shift;
  my $moduleDB;

  if (!defined($file) && !defined($default_moduleDB)) {
    $file = ".";
#   $file = Asim::choose_filename("Select moduleDB configuration file")
#     || shell_error("No moduleDB defined\n") && return ();
  }

  if (defined($file) && $file) {
    $moduleDB = Asim::Module::DB->new($file)
      || shell_error("No moduleDB defined for file ($file)\n") && return ();
  }

  return $moduleDB
    || $default_moduleDB
    || shell_error("No moduleDB defined\n") && return ();

}


################################################################
#
# Module functions
#
################################################################


sub set_module {
  my $file = shift;

  $default_module = undef;
  $default_module = get_module($file);

  return $default_module;
}

sub unset_module {
  $default_module = undef;
}

sub create_module {
  new_module(@_);
}

sub new_module {
  my $file = shift
    || Asim::choose_filename("Select module configuration file")
    || shell_error("Module filename must be specified\n") && return ();

  my $module = Asim::Module->new();

  $module->edit();
  $module->dump();

  if (Asim::choose_yes_or_no("Save module")) {
    $module->save($file)
      || shell_error("Module save failed\n") && return ();
  }

  $default_module = $module;

  return 1;
}

sub edit_module {
  my $file = shift;
  my $module = get_module($file) || return;

  $module->edit();
  $module->dump();

  if (Asim::choose_yes_or_no("Save module")) {
    $module->save()
      || shell_error("Module save failed\n") && return ();
  }

  return 1;
}


sub show_module {
  my $file = shift;
  my $module = get_module($file) || return;

  $module->dump();
  return 1;
}


sub get_module {
  my $file = shift;
  my $module;

  if (!defined($file) && !defined($default_module)) {
    $file =  Asim::choose_filename("Select module configuration file")
      || shell_error("No module or default module defined\n") && return ();
  }

  if (defined($file) && $file) {
    $module = Asim::Module->new($file) 
      || shell_error("Failed to open module file ($file)\n") && return ();      
  }

  return $module
    || $default_module
    || shell_error("No module or default module defined\n") && return ();
}

################################################################
#
# Miscelaneous functions
#
################################################################

sub awb {
  system "awb &";

  return 1;
}



sub rehash {
  rehash_workspace();
  rehash_models();
  rehash_modules();
  rehash_packages();
  rehash_repositories();
  rehash_locks();

  return 1;
}

sub status {
  my $version = Asim::Version();
  my $repositoryDB_file = "";
  my $workspace_file = "";
  my $package_location = "";
  my $packfile_path;
  my $model_file = "";
  my $moduleDB_file = "";
  my $module_file = "";

  $packfile_path     = join(":\n                       ",
                            $default_repositoryDB->packfile_path());

  $model_file        = $default_model->filename()       if $default_model;
  $module_file       = $default_module->filename()      if $default_module;
  $package_location  = $default_package->location()     if $default_package;

  print "\n";
  print "Asim Version:          $version\n\n";
  print "Current packfile_path: $packfile_path\n\n";
  print "Current workspace:     " . Asim::rootdir() . "\n\n";
  print "Current package:       $package_location\n";
  print "Current model:         $model_file\n";
  print "Current module:        $module_file\n";


  return 1;
}

################################################################
#
# Utility functions
#
################################################################

#
# Remember a failed operation on a package, repository, ....
#    Note the object must support a name() method.
#
sub _add_failure {
  my $name    = shift;
  my $message = shift;

  push(@failures, "$name -- $message");

  # TBD: We might want to return undef on some conditions,
  # like "interative" mode, and let the caller stop loops if
  # it wants.

  return 1;
}

#
# Report all failures and clear list of failures
#
sub _report_failures {

  if ( $#failures < 0) {
    @failures = ();
    return 1;
  }

  print "\n";
  print "##################################################################\n";
  print "Summary of failed operations:\n";
  print "\n";

  for my $failure  (@failures) {
    print "    $failure\n";
  }

  print "\n";
  print "##################################################################\n";
  print "\n";

  @failures = ();
  return undef;
}


1;
