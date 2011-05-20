
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
#    For example, if "lubricate driveshaft" and "lubricate hingemounts" were
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
use Asim::Fork;

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

# these are the items we display or check on a "show configuration" or
# "verify configuration" command.  We may want to make configurable via asimrc, eventually:
our @default_buildtree_config_itemlist = qw(machine os_release2 cxx_compiler cxx_compiler_version);
our @default_buildtree_config_varlist  = qw(CXX ARCHFLAGS);

our $default_user = $ENV{USER};

our @failures = ();


Asim::Fork::init("asim-shell", 1);

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

sub cp {
  my $file1 = shift;
  my $file2 = shift;
  $file1 = $Asim::default_workspace->resolve($file1);
  # file2 requires only the directory to be resolved
  (my $file2volume, my $file2dir , my $file2file) = File::Spec->splitpath( $file2 );
  my $file2path = $Asim::default_workspace->resolve($file2dir);
  system "cp $file1 $file2path/$file2file";
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
  my $name = shift;
  my $type; 
  my $url;
  my $browse_url;
  my $status;

  my $package = get_package($name) || return ();

  $type =       Asim::choose_name("Repository type:", "cvs", "svn")
                || return undef;

  $url  =       Asim::choose("Repository URL")
                || return undef;

  $browse_url = Asim::choose("Repository browse URL")
                || "undefined";
         

  #
  # Create a repository object that can create the actual repository
  #
  my $repo = Asim::Repository->new(packagename => $name,
                                   method      => $type,
                                   access      => $url,
                                   module      => "undefined",
                                   tag         => "HEAD",
                                   browseURL   => $browse_url,
                                   target      => $name,
                                   changes     => "changes");

  #
  # Create the repository and populate it with the package
  #
  if (! Asim::choose_yes_or_no("Do you really want to import the pacakge",
			       "response_required",
			       "yes")) {
    print "Package import aborted\n";
    return undef;
  }


  $status = $repo->create($package);
  return undef if (! $status);

  #
  # Rehash the world so we see the new repository
  #
  rehash();
}

sub show_repository {
  my $repositoryname  = shift           || return undef;
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

#
# Bundle utility functions: alphanumeric sort module posted at perlmonks.org
#

sub _alphanum_sort
{
    my $x = uc(shift);
    my $y = uc(shift);
    
    if (!($x =~ /\d+(\.\d+)?/)) 
    {
        return $x cmp $y;
    }
    my $xBefore = $`;
    my $xMatch = $&;
    my $xAfter = $';
    if (!($y =~ /\d+(\.\d+)?/)) {
        return $x cmp $y;
    }
    if ($xBefore eq $`) {
        if ($xMatch == $&) {
            return _alphanum_sort($xAfter, $');
        } else {
            return $xMatch <=> $&;
        }
    } else {
        return $x cmp $y;
    }
}

sub alphanum_sort
{
    _alphanum_sort($a, $b);
}

sub list_bundles {

  print join("\n", $default_repositoryDB->bundle_directory()) . "\n";

  return 1;
}

#
# Bundle functions
#

#
# return the list of packages in a bundle.
# Common suborutine used  by many commands below,
# NOT an asim-shell command!!
# 
#
sub bundle_packages_ {
  my $bundlename = shift;
  my $golden = shift;
  my $bundleid;
  my @packages = ();

  chomp($bundlename);
  ($bundlename, $bundleid) = (split("/", $bundlename), undef);
  shell_error("Bundle id '$bundleid' cannot be used with golden qualifier!\n") && return () if (defined $bundleid && ($golden == 1));
  $bundleid = "default" unless defined $bundleid;  

  my @flist = $default_repositoryDB->bundle_files($bundlename);
  (@flist == 0) && shell_error("No bundle file found for `$bundlename`!\n") && return ();

  my $inifile = Asim::Inifile->new();
  foreach my $file (@flist) {
      $inifile->include($file) || ($show_warnings && print("Cannot read file `$file` to collect bundle list!\n"));
  }

  if ($golden == 1)
  {
      my @grouplist = $inifile->get_grouplist();
      # this would be simpler if perl hash implementation preserves insertion order
      foreach my $bundleid (reverse(sort{alphanum_sort}(@grouplist)))
      {
	  my $items = $inifile->get($bundleid);
	  if ((defined $items->{Status}) && ($items->{Status} eq "Success"))
	  {
	      @packages = split(" ", $items->{Packages});
	      (@packages == 0) && shell_error("Recent successful bundle id `$bundleid` did not result in any package!\n") && return ();
	      print "#\n# Picked bundle id: $bundleid\n#\n";
	      last;
	  }
      }
      (@packages == 0) && shell_error("No bundle id with success status found!\n") && return ();      
  }
  else 
  {
      my $items = $inifile->get($bundleid);
      if (!defined $items) {
	  shell_error("Bundle id `$bundleid` not found!\n") && return ();
      }
      elsif ((defined $items->{Status}) && ($items->{Status} eq "Failure")) {
	  if (!Asim::choose_yes_or_no("This bundle has failure status.  Do you want to get it", "no", "no")) {
	      return 0;
	  }
      }
      @packages = split(" ", $items->{Packages});
      (@packages == 0) && shell_error("Bundle id `$bundleid` did not result in any package!\n") && return ();
  }

  return @packages;
}

sub checkout_bundle {
  my $user;
  my $addpath = 1;
  my $build = 1;
  my $golden = 0;
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
  @packages = bundle_packages_( $bundlename, $golden );
  
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
  my $copy = 0;
  my $golden = 0;
  my $status;
  my $bundlename;
  local @ARGV = @_;
  my @packages;
  my $runstatus;

  # Parse options

  $status = GetOptions( "build!"   => \$build,
		        "addpath!" => \$addpath,
			"copy!"    => \$copy,
			"golden"   => \$golden,
			);
  return 0 if (!$status);

  # Bundle name is remaining argument
  
  $bundlename = shift @ARGV;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  #
  # Determine the list of packages to check out from the bundle file
  #	
  @packages = bundle_packages_( $bundlename, $golden );
  
  print "Using: " . join(" ", @packages) . "\n";

  #
  # Regenerate the arguments for checkout_package
  #     This is not a very aesthetic way of doing this
  #     adding a subfunction to do the checkout would be better
  #
  my @args = ();

  if (defined($addpath)) {
    push(@args, ("--" . ($addpath?"":"no") . "addpath"));
  }

  if (defined($copy)) {
    push(@args, ("--" . ($copy?"":"no") . "copy"));
  }

  # Do not build since we do all the builds later...
  push(@args, "--nobuild");

  my $pkg;
  my @plist;

  foreach my $b (@packages) {
    _print_package_start("use", $b);

    print "--- use package $b " . join(" ",@args) . "\n";

    $pkg = use_package($b, @args);

    if (! defined($pkg)) {
      _add_failure($b, "use failed");
      next;
    }

    push(@plist, $pkg);

    _print_package_finish("use", $b);
  }

  if ($build) {
    configure_and_build_packages(@plist);
  }

  $status = _report_failures();

  return $status;
}

sub clone_bundle {
  my $user;
  my $addpath = 1;
  my $build = 1;
  my $status;
  my $bundlename;
  local @ARGV = @_;
  my @packages;
  my $runstatus;
  my $url = undef;

  # Parse options

  $status = GetOptions( "user=s"     => \$user,
		        "build!"     => \$build,
		        "url=s"     => \$url,
		        "addpath!"   => \$addpath );

  return 0 if (!$status);

  # Bundle name is remaining argument
  
  $bundlename = shift @ARGV;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  #
  # Determine the list of packages to clone from the bundle file
  #
  @packages = bundle_packages_( $bundlename );
  
  print "Cloning: " . join(" ", @packages) . "\n";

  #
  # Regenerate the arguments for clone_package
  #     This is not a very aesthetic way of doing this
  #     adding a subfunction to do the checkout would be better
  #
  my @args = ();

  $default_user = $user
    || Asim::choose("Repository user you want to do the checkout (e.g., your login name or anonymous)", $default_user)
    || shell_error("Must select user for checkout\n") && return ();

  push(@args, "--user=$default_user");
  push(@args, "--url=$url");

  if (defined($addpath)) {
    push(@args, ("--" . ($addpath?"":"no") . "addpath"));
  }

  # Do not build since we do all the builds later...
  push(@args, "--nobuild");

  my $pkg;
  my @plist;

  foreach my $b (@packages) {
    print "--- clone package $b " . join(" ",@args) . "\n";
    $pkg = clone_package($b, @args);

    if (! defined($pkg)) {
      _add_failure($b, "clone failed");
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
  my $golden = 0;
  local @ARGV = @_;
  
  my $status = GetOptions("build!"     => \$build,
			  "golden"     => \$golden);
  return 0 if (!$status);

  # Bundle name is remaining argument
  my $bundlename = shift @ARGV;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  # Determine the list of packages to check out from the bundle file
  my @packages = bundle_packages_( $bundlename, $golden );
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

sub pull_bundle {

  # Parse options
  my $build = 1;
  my $url = undef;
  local @ARGV = @_;
  my $status = GetOptions( "build!" => \$build,
                            "--url=s" => \$url );
  return 0 if (!$status);

  # Bundle name is remaining argument
  my $bundlename = shift;
  if (!defined $bundlename) {
      $bundlename = choose_bundle() || shell_error("Bundle name must be selected!\n") && return ();
  }
  
  # Determine the list of packages to update from the bundle file
  my @packages = bundle_packages_( $bundlename );
  print "Pulling: " . join(" ", @packages) . "\n";

  # Regenerate the arguments for pull_package
  # Do not build since we do all the builds later...
  my @args = ("--nobuild", "--url=$url");
  my @plist = ();
  foreach my $b (@packages) {
    print "--- pull package $b " . join(" ",@args) . "\n";
    pull_package($b, "--noreport", @args);
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
  @packages = bundle_packages_( $bundlename, 0 );

  my $bundle = $default_repositoryDB->get_bundle($bundlename);

  print "Bundle:   $bundlename\n";
  print "Type:     " . $bundle->type() . "\n";
  print "Status:   " . $bundle->status() . "\n";
  print "Packages: " . join(" ", $bundle->packages()) . "\n";

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


sub new_bundle {

  # Parse options
  local @ARGV = @_;
  my $model      = '';
  my $experiment = '';
  my $packages   = '';
  my $type       = 'release';
  my $status     = 'Unknown';
  my $file       = '';
  my $install    = 0;
  my $head       = 0;
  my $options_ok = GetOptions(
    'model=s'      => \$model,
    'experiment=s' => \$experiment,
    'packages=s'   => \$packages,
    'type=s'       => \$type,
    'status=s'     => \$status,
    'file=s'       => \$file,
    'install!'     => \$install,
    'head!'        => \$head
  );
  return 0 if (!$options_ok);

  # --model, --experiment, and --packages are mutually exclusive
  if (($model && $experiment) || ($model && $packages) || ($experiment && $packages)) {
    shell_error("Please specify only one of --model, --experiment, or --packages!\n") && return ();
  }

  # --file and --install are mutually exclusive
  if ($file && $install) {
    shell_error("Please specify only one of --file or --install!\n") && return ();
  }
  
  # type must be 'baseline' or 'release'
  if ($type ne 'baseline' && $type ne 'release') {
    shell_error("Type must be either 'baseline' or 'release'!\n") && return ();
  }
  
  # --status only used if --type=baseline
  if ($type ne 'baseline' && $status ne 'Unknown') {
    shell_error("Option --status only valid if --type=baseline!\n") && return ();
  }
  
  # get and parse the name and tag of the bundle the user wants to create:
  my $bundle_arg = shift @ARGV;
  (my $name, my $tag, my @dummy) = split /\//, $bundle_arg;
  if (!defined $tag) {$tag='default'}
  
  # make sure the bundle doesn't exist already
  my @bundle_dir = $default_repositoryDB->bundle_ids($name);
  if (0 < grep {$_ eq $tag} @bundle_dir) {
    if (!Asim::choose_yes_or_no("Bundle $name/$tag already exists.  Overwrite", "no", "no")) {
      return ();
    }
  }
  
  # get the list of packages that make up the bundle
  my @package_list = ();
  my $runtype;
  if ($model) {
    # get the list of packages that $model depends on.
    # The special value '.' designates the current default model:
    my $workspace = get_workspace() || return ();
    my $root = $workspace->rootdir();
    if ($model eq '.') { $model = undef; }
    my $model_obj = get_model($model) || return ();
    $model = $model_obj->filename();
    print "# getting package dependencies of model $model\n"; # DEBUG
    chomp($packages = `model-coverage --workspace=$root --model=$model --print=packages`);
    @package_list = split ' ', $packages;

  } elsif ($experiment) {
    # get the list of packages that all models in $experiment depend on
    if ($experiment =~ m/^([^\/]+)\/([^\/]+)$/) {
      my $package = $1;
      $runtype = $2;
      my $workspace = get_workspace() || return ();
      my $root = $workspace->rootdir();
      my $models_file = "experiments/$package/$runtype/$runtype.models";
      Asim::resolve($models_file)
       || shell_error("Cannot find models file $models_file!\n") && return ();
      print "# getting package dependencies of experiment $models_file\n"; # DEBUG
      chomp(my $pkg_names = `model-coverage --workspace=$root --package=$package --experiments=$runtype --print=packages`);
      @package_list = split ' ', $pkg_names;
    } else {
      shell_error("Badly formed experiment $experiment, should be: <package>/<runtype>!\n") && return ();
    }

  } elsif ($packages) {
    # the list of packages is specified literally
    @package_list = split ',', $packages;

  } else {
    # otherwise get the list of all packages in the current workspace
    @package_list = $default_packageDB->directory();
  }
  
  # get the destination filename:
  my $show_bundle = 0;
  if (!$file) {
    $show_bundle = 1;
    my $loc;
    if ($install) {
      $loc = Asim::Sysconfdir() . '/asim/bundles/';
    } else {
      $loc = Asim::Util::expand_tilda('~/.asim/bundles/');
    }
    $file = "$loc$type/$name.$type";
  }
  
  # append the bundle info to the file
  print "# writing bundle information to file $file\n"; # DEBUG
  unless (-e $file) {
    my $dir = dirname($file);
    system "mkdir -p $dir";
    system "touch $file" # create empty file if it doesn't exist
  }
  my $file_obj = Asim::Inifile->new($file) || return ();
  if ($type eq 'baseline') {
    if ($experiment) {$file_obj->put($tag, 'Type', $runtype)}
    $file_obj->put($tag, 'Status', $status);
  }
  my $packages_string = '';
  foreach my $name (@package_list) {
    my $package = get_package($name);
    my $revision;
    if ($head) {
      $revision = $package->branch_name();
    } else {
      $revision = $package->baseline_tag(0);
    }
    $packages_string .= "$name/$revision ";
  }
  $file_obj->put($tag, 'Packages', $packages_string);
  $file_obj->save($file) || return ();
  
  # show the user what you just created (unless you wrote it to nonstandard location)
  if ($show_bundle) {show_bundle($bundle_arg)}
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
  if (-e $targetdir && $repository->{type} ne 'distributed') {
    #
    # Package already exists in workspace
    #

    ### TBD: Check if the version is the same as the one being checked out...

    print "A package already exists in $targetdir and may have modified files in it!\n";

    if (! Asim::choose_yes_or_no("Do you want to overwrite package at $targetdir", "no", "yes")) {
      return $default_packageDB->get_package_by_dirname($targetdir);
    }
  }

  #
  # Check if package of this type already exists in searchpath
  #

  my $target_packagename = $repository->packagename();

  if (! -e $targetdir && defined($default_packageDB->get_package($target_packagename)) && $repository->{type} ne 'distributed') {
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
    maybe_check_package_configurations();
  }

  #
  # Check and if it is necessary to add package to search path...
  #     (return if package is not going to be on the path)
  #
  if ($addpath && ($repository->{type} ne 'distributed' || !defined($default_packageDB->get_package($package->name())))) {
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

sub clone_package {
my $user = undef;
  my $build = 1;
  my $addpath = 1;
  my $url = undef;
  my $status;
  local @ARGV = @_;

  #
  # Parse options
  #
  $status = GetOptions( "user=s"   => \$user,
		        "build!"   => \$build,
		        "addpath!" => \$addpath,
                        "url=s"    => \$url);

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
  
  my $repository = $default_repositoryDB->get_repository($repositoryname) 
    || shell_error("Repository ($repositoryname) was missing or malformed in packfile\n") && return undef;

  if ($repository->{type} ne 'distributed') {
     shell_error("Can only clone a distributed repository. $repositoryname is a $repository->{method} repository.\n") && return undef;
  }
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
  # Determine user to do a clone

  $default_user = $user
    || Asim::choose("Repository user you want to do the checkout (e.g., your login name or anonymous)", $default_user)
    || shell_error("Must select user for checkout\n") && return undef;


  # change repository url in case user specified a url on the command line
  $repository->{access} = $url if ( defined ($url) ) ;

  #
  # Do the clone and get package object for cloned package
  #
  $repository->clone($default_user) 
    || shell_error("Could not check out package ($repositoryname)\n") && return undef;

  my $package = $default_packageDB->get_package_by_dirname($targetdir)
    || shell_error("No legal package at $targetdir\n") && return undef;

  #
  # Optionally build the package
  #
  if (defined($build) && $build ) {
    $package->configure();
    $package->build();
    maybe_check_package_configurations();
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
# either via a copy or just a searchpath reference
#
#   Code is too redunant with checkout_package...
#
sub use_package {
  my $build = 1;
  my $copy = 0;
  my $addpath = 1;
  my $targetdir;
  my $status;
  local @ARGV = @_;

  #
  # Parse options
  #
  $status = GetOptions( "build!"   => \$build,
                        "copy!"    => \$copy,
		        "addpath!" => \$addpath);
  return undef if (!$status);

  #
  # Can only build if we are making a copy
  #
  $build &= $copy;

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
  my $target_packagename;
  my $local_checkoutdir;
  my $existing_package;

  $repository = $default_repositoryDB->get_public_repository($repositoryname, $copy)
    || shell_error("Repository ($repositoryname) not available on system\n") && return undef;


  $target_packagename = $repository->packagename();


  $local_checkoutdir = $repository->checkoutdir()
    || shell_error("Could not determine target for checkout\n") && return undef;


  $existing_package = $default_packageDB->get_package($target_packagename);

  #
  # Check if we already have the right version available
  #
  if (defined($existing_package) && ($existing_package->tag() eq $repository->tag())) {
    print "Desired version already exists in workspace\n";
    return $existing_package;
  }

  #
  # Clean up existing local copies of package in workspace and searchpath references
  #
  # Scenarios:
  #
  #         Old       New      Action
  #         ----      ----     -------
  #         Checkout  Copy     delete package and leave searchpath alone
  #         Checkout  Public   delete package and leave searchpath alone!
  #         Copy      Copy     delete package and leave searchpath alone
  #         Copy      Public   delete package and leave searchpath alone!
  #         Public    Copy     delete searchpath reference to package
  #         Public    Public   delete searchpath reference to package
  #

  #
  # Check if workspace local checkout directory already exists
  #
  if (-e $local_checkoutdir) {
    my $package = $default_packageDB->get_package_by_dirname($local_checkoutdir);

    if (! defined($package)) {
      print "A non-package exists at $local_checkoutdir\n";
      print "Please delete manually and retry this command\n";
      return undef;
    }

    if (! Asim::choose_yes_or_no("Do you want to delete package at $local_checkoutdir", "no", "yes")) {
      print "Operation aborted!!\n";
      return $package;
    }
      
    $package->destroy();
  }


  #
  # Check if package of this type already exists in searchpath
  #
  #    Note: A reference to a workspace local copy in the searchpath isn't removed because
  #             1) if we are doing a copy it will be reused and
  #             2) if we are going to reference a global copy it won't hurt
  #

  if (defined($default_packageDB->get_package($target_packagename)) ) {
    print "A package of this name ($target_packagename) already exists in your searchpath!\n";
    my $q = "Do you want to remove the current reference to this package from the searchpath";

    if (Asim::choose_yes_or_no($q, "yes", "yes")) {
      my $workspace = $Asim::default_workspace;

      $workspace->remove_path_by_packagename($target_packagename);
      $workspace->save();
    }
  }

  #
  # Do the checkout and get package object for checked out package
  #
  $targetdir = $repository->checkout()
    || shell_error("Could not add package ($repositoryname)\n") && return undef;

  my $package = $default_packageDB->get_package_by_dirname("${targetdir}")
    || shell_error("No legal package at ${targetdir}\n") && return undef;

  #
  # Optionally build the package
  #
  if ($build ) {
    $package->configure();
    $package->build();
    maybe_check_package_configurations();
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
  
  maybe_check_package_configurations();

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
  maybe_check_package_configurations();

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

  my $dofork = 0;
  my $logdir = $Asim::default_workspace->log_dir();
  my $status;

  @plist = sort { $a->buildorder() <=> $b->buildorder() } @plist;

  my $last_buildorder = 1;

  for my $p (@plist) {
    my $pname = $p->name();
    my $buildorder = $p->buildorder();

    # On change of buildorder priority - wait for all lower priority builds

    if ($buildorder != $last_buildorder) {
      print "Waiting for all builds of priority $last_buildorder\n";
      Asim::Fork::wait_for_children() && _add_failure("unknown", "forked configure/make failed");;

      $last_buildorder = $buildorder;
    }

    _print_package_start("configure/build", $pname, "($buildorder)");

    # TBD: Should we filter out non-private packages?

    unless (Asim::Fork::controlled_fork($dofork, "$logdir/build.$pname.log", "build.$pname", 1)) {

      $status = $p->configure();

      if (!$status) {
        _add_failure($p->name(), "configure failed");
      } else {

        $status = $p->build();
        if (! $status) {
          _add_failure($p->name(), "make failed");
        }
      }

      Asim::Fork::controlled_exit($dofork, ! (defined($status) && ($status)));
    }

    _print_package_finish("configure/build", $p->name());
  }

  Asim::Fork::wait_for_children() && _add_failure("unknown", "forked configure/make failed");
  
  maybe_check_package_configurations();

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
  print "Obsolete: Use {clean,run,verify} regression instead\n";

  return undef;
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
# upgrade one or more packages, returning 1 if all succeeded.
#
sub upgrade_package {
  my $build = 1;
  my $rehash = 1;
  my $report = 1;

  my @build_list = ();
  local @ARGV = @_;

  # Parse options

  my $status = GetOptions( "build!"  => \$build,
                           "rehash!" => \$rehash,
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

    my $tag = $package->tag();

    #
    # Hack to figure out if this is an old package that should be upgraded
    #
    my $legacy_package = !$package->isprivate() && 
                         grep(!/^\/.*\/asim\/packages\//, $package->location());


    _print_package_start("upgrade", "$name/$tag", "(" . $package->type() . " repository)");

    if (! ($tag =~ /v[0-9][0-9].*/) && ! $legacy_package) {
      #
      # The current package isn't a release...
      #
      print "Skipping upgrade - current package is not a well-defined release ($tag)\n";
      _add_failure($name,"version in workspace not a standard release");

    } elsif ($package->isprivate()) {
      #
      # Handle a checked out package in workspace
      #
      checkout_package("$name/STABLE") 
	|| _add_failure($name,"checkout failed");


      if ($build) {
	# if we are also building, add the package to the list of ones to build
	push @build_list, $package;    
      }
    } else {
      #
      # Handle a shared public repository
      #
      use_package("$name/STABLE") 
	|| _add_failure($name,"attempt to use shared public package failed");
    }

    _print_package_finish("update", "$name/$tag");
  }

  if ($rehash) {

    # Rehash the model and module database.
    # If we had a finer-grained rehash then we could do
    # this on a per-package basis.

    rehash_modules();
    rehash_models();
  }

  # if updating more than one package, and building,
  # do the builds after you have checked everything out.

  configure_and_build_packages(@build_list);


  if ($report) {
    $status = _report_failures();
  } else {
    $status = ($#failures < 0) || undef;
  }

  return $status;
}

#
# update one or more packages, returning 1 if all succeeded.
#
sub update_package {
  my $build = 1;
  my $rehash = 1;
  my $report = 1;

  my @build_list = ();
  local @ARGV = @_;

  # Parse options

  my $status = GetOptions( "build!"  => \$build,
                           "rehash!" => \$rehash,
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
  # We will not commit dependent packages for git repositories. 
    }

    _print_package_finish("update", $name);
  }

  if ($rehash) {

    # Rehash the model and module database.
    # If we had a finer-grained rehash then we could do
    # this on a per-package basis.

    rehash_modules();
    rehash_models();
  }

  # if updating more than one package, and building,
  # do the builds after you have checked everything out.

  configure_and_build_packages(@build_list);


  if ($report) {
    $status = _report_failures();
  } else {
    $status = ($#failures < 0) || undef;
  }

  return $status;
}

#
# pull one or more packages, returning 1 if all succeeded.
#
sub pull_package {
  my $build = 1;
  my $report = 1;
  my $url = undef;
  my $branch = undef;

  my @build_list = ();
  local @ARGV = @_;

  # Parse options

  my $status = GetOptions( "build!"  => \$build,
                           "report!" => \$report, 
                           "url=s"   => \$url);

  return undef if (!$status);

  # get the list of packages
  my @package_names = _expand_package_names(@ARGV);

  while ( my $pkgspec = shift @package_names) {
    # each package in the list could be a simple name,
    # or a name and version or branch identifier, like this:
    #   <pkgname>
    #   <pkgname>/HEAD
    #   <pkgname>/<branchname>
    #   <pkgname>/<tag>
    my $name;
    my $version;
    ($name,$version) = split('/',$pkgspec);

    my $package = get_package($name);

    if (! defined($package)) {
      _add_failure($name, "no such package");
      next;
    }

    _print_package_start("pull", $name, "(" . $package->type() . " repository)");

    if (!$package->isprivate()) {
      print("Cannot update a non-private package\n");
      _print_package_finish("update", $name, "(skipped)");
      next;
    }
    
    # set package url
    $package->{url} = $url;

    # update the package to the specified version
    $package->pull($version) || _add_failure($package->name(),"update failed");

    if ($build) {
      # if we are also building, add the package to the list of ones to build
      push @build_list, $package;
    }

    _print_package_finish("pull", $name);
  }

  # if updating more than one package, and building,
  # do the builds after you have checked everything out.

  configure_and_build_packages(@build_list);


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


#
# push one or more packages to a remote distributed repository, 
# returning 1 if all succeeded
#
sub push_package {
  my $deps = 1;
  my $url = undef;

  local @ARGV = @_;

  # Parse options

  my $status = GetOptions( "dependent!"     => \$deps,
			   "url=s"          => \$url
			 );

  return undef if (!$status);

  my $name;
  my $branch;

  # Package names are remaining arguments
  while ( my $fullname = shift @ARGV ) {
    ($name, $branch) = (split("/",$fullname));
    # handle special case of "all packages"
    if (defined($name) && ($name eq "*" || $name eq "all")) {
      return push_all_packages($deps, $url);
    }
    if (! defined ($url) ) {
      my $repository = $default_repositoryDB->get_repository($name) 
    || shell_error("Cannot retrieve URL to push package $name \n Use --url to specify location to push to \n") && return undef;
      $url = $repository->{access};
      if (! defined( $url) ) {
          shell_error("Cannot retrieve URL to push package $name \n Use --url to specify location to push to \n") && return undef;
      }
    }
    my $package = get_package($name) || return undef;
    $package->{url} = $url;
    $package->{branch} = $branch;
    $package->push_package(!$deps) || return undef;
  }
  
  return 1;
}

# Commits all packages in workspace

sub push_all_packages {
  my $deps = shift;

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

  my $repository = undef;
  my $url = undef;
  #
  # Do a push on each package
  #
  for my $p (@plist) {
        print "Pushing package " . $p->name() . "\n\n";
        $repository = $default_repositoryDB->get_repository($p->name()) 
    || shell_error("Cannot retrieve URL to push package $p->name() \n Use --url to specify repository URL \n") && return undef;
        $url = $repository->{access};
        if (! defined( $url) ) {
          shell_error("Cannot retrieve URL to push package $p->name() \n Use --url to specify location to push to \n") && return undef;
        }
        $p->{url} = $url;
	$p->push_package(!$deps) || return undef;
	print "\n";
  }

  return 1;
}

#
# Revert one or more packages
#
sub revert_package {
  my @package_names = _expand_package_names(@_);

  foreach my $name (@package_names) {
    _print_package_start("revert", $name);

    my $package = get_package($name);

    unless (defined($package)) {
      _add_failure($name, "no such package");
      _print_package_finish("revert", $name, "(No such package)");
      next;
    }

    unless ($package->isprivate()) {
      printf("Package %s is a shared package\n", $name );
      _print_package_finish("revert", $name, "(Skipped)");
      next;
    }

    $package->revert();

    _print_package_finish("revert", $name);
    if ( @_ ) { print "\n" }
  }

  return _report_failures();
}


sub tag_package {
  local @ARGV = @_;
  my $name = shift if (defined($_[1]));
  my $existing = 0;
  my $status = GetOptions( "existing!" => \$existing );
  my $package = get_package($name) || return ();
  my $tag = shift
    || Asim::choose("Enter label (format: alphanumeric or underscore characters only)")
    || shell_error("Label must be specified\n") && return ();

  return $package->label($tag, $existing)
    || shell_error("Label package failed\n") && return ();
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

  $package->isprivate()
    || shell_error("Cannot merge a shared package\n") && return ();

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
# Build configuration functions
#
################################################################

sub show_configuration {
  local @ARGV = @_;
  my $verbose = 0;

  # Parse options
  my $status = GetOptions( "verbose!" => \$verbose) || return undef;

  # get the list of packages
  my @packages = _expand_package_names(@ARGV);

  # print status for each package in turn
  foreach my $name ( @packages ) {
    my $package = get_package($name);
    my $configuration = $package->get_configuration();
    _print_package_start('show configuration', $name, '');

    my @itemlist = @default_buildtree_config_itemlist;
    my @varlist  = @default_buildtree_config_varlist;
    if ($verbose) {
      @itemlist = $configuration->get_item_list();
      @varlist  = $configuration->get_variable_list();
    }
    foreach my $item (@itemlist) {
      my $value = $configuration->get_item($item);
      if (defined($value)) {print $item, ' : ', $value, "\n"}
    }
    if ($verbose) {print "----------------------------------------------\n"}
    foreach my $var (@varlist) {
      my $value = $configuration->get_variable($var);
      if (defined($value)) {print $var, ' = ', $value, "\n"}
    }

    _print_package_finish('show configuration', $name, '');
  }
}

sub verify_configuration {
  local @ARGV = @_;
  my $exhaustive = 0;

  # Parse options
  my $status = GetOptions( "exhaustive!" => \$exhaustive) || return undef;

  # get the list of packages
  my @packages = _expand_package_names(@ARGV);
  if ($#packages < 1) {
    # only zero or one package?  trivial success!
    return 1;
  }

  # check each package in turn
  my $success = 1;
  my %itemvalues;
  my %varvalues;
  foreach my $pkgname ( @packages ) {
    my $package = get_package($pkgname);
    my $configuration = $package->get_configuration();

    my @itemlist = @default_buildtree_config_itemlist;
    my @varlist  = @default_buildtree_config_varlist;
    if ($exhaustive) {
      @itemlist = $configuration->get_item_list();
      @varlist  = $configuration->get_variable_list();
    }

    foreach my $item (@itemlist) {
      my $value = $configuration->get_item($item);
      if (defined($value)) {
        if (defined($itemvalues{$item})) {
          if ($value ne $itemvalues{$item}) {
            if ($success) {print "---------------------------------------------------------------------------\n"}
            $success = 0;
            print_configuration_mismatch($item,$itemvalues{$item},$value,$pkgname);
          }
        } else {
          $itemvalues{$item} = $value;
        }
      }
    }

    foreach my $var (@varlist) {
      my $value = $configuration->get_variable($var);
      if (defined($value)) {
        if (defined($varvalues{$var})) {
          if ($value ne $varvalues{$var}) {
            if ($success) {print "---------------------------------------------------------------------------\n"}
            $success = 0;
            print_configuration_mismatch($var,$varvalues{$var},$value,$pkgname);
          }
        } else {
          $varvalues{$var} = $value;
        }
      }
    }

  }
  
  if (! $success) {
    print "WARNING:\n";
    print "  The configuration of packages in your workspace appears inconsistent.\n";
    print "  Consider using \'clean package\' followed by \'build package\' on the\n";
    print "  mismatching packages noted above.\n";
    print "---------------------------------------------------------------------------\n";
  }
  return $success;
}

sub print_configuration_mismatch {
  my $itemname = shift;
  my $expected = shift;
  my $actual   = shift;
  my $pkgname  = shift;
  print "WARNING: in ", $pkgname, ": \'", $itemname, "\' mismatch,",
        " expected \'", $expected, "\', got \'", $actual, "\'\n";
}

# internal utility function that gets called whenever we change package configurations
sub maybe_check_package_configurations {
  if ($AsimShell::check_package_configurations) {
    return verify_configuration('all');
  } else {
    return 1;
  }
}

################################################################
#
# regression DB functions
#
################################################################

sub list_regressions {

  # Like all of the regression programs, this routine assumes far too 
  # much about the strcutre of the regressions

  my $regdir = $Asim::default_workspace->rootdir(). "/run/regtest";

  system("ls -t1 $regdir");

  print "\n";
  print "All regression commands will operate on the newest of these\n";
}


################################################################
#
# regression functions
#
################################################################

sub run_regression {

  local @ARGV = @_;

  my $cleanup = 0;

  my $status;

  # Parse options (passing through raw regression.launcher switches)

  Getopt::Long::Configure("pass_through");

  $status = GetOptions( "cleanup!"    => \$cleanup);

  Getopt::Long::Configure("default");

  return 0 if (!$status);

  # Create base regression command

  my $command = "regression.launcher ";

  # Then, add expanded package name list

  my @package_names = grep(!/^[-]/, @ARGV);

  if ($#package_names >= 0) {
    @package_names = _expand_package_names(@package_names);

    if ($package_names[0] ne "default") {
      $command .= " --package=" . join(",",@package_names);
    }
  }

  # Then, add on regression switches

  $command  .= " " . join(" ", grep(/^[-]/, @ARGV));

  # Do the requested actions

  if ($cleanup) {
    clean_regression();
  }

  print "% $command\n";
  system($command);

  return 1;

}

sub verify_regression {

  system("regression.verifier");
  return 1;
}

sub clean_regression {

  system("regression.cleanup");
  return 1;
}


sub delete_regression {

  system("regression.cleanup --purge");
  return 1;
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
  my $persist=0;
  my $model;
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir,
                        "persist!"   => \$persist );

  # Model is remaining argument

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  $status = $model->nuke("--builddir" => "$builddir");
  return if (! $status);

  $status = $model->configure("--builddir" => "$builddir",
                              "--persist"  => "$persist" );

  return $status;
}

sub configure_model {
  my $file;
  my $builddir="";
  my $persist=0;
  my $model;
  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "builddir=s" => \$builddir,
                        "persist!"   => \$persist );

  # Model is remaining argument

  $file = shift @ARGV;
  $model = get_model($file) || return ();

  # Remember this model

  $default_model = $model;

  # Configure the model

  $status = $model->configure("--builddir" => "$builddir",
                              "--persist"  => "$persist" );

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

  # Remember this model

  $default_model = $model;

  # Build it...

  $status = $model->build("--builddir" => "$builddir",
                          "--buildopt" => "$options");

  return $status;
}

sub make_model {
  build_model(@_);
}


sub setup_model {
  print "The command 'setup model' is has been replaced by 'setup benchmark'\n";
  print "See 'help commands' for more information\n";

  return undef;
}

sub run_model {
  print "The command 'run model' is has been replaced by 'run benchmark'\n";
  print "See 'help commands' for more information\n";

  return undef;
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


sub update_model {
  my @files = @_;

  if ($#files == -1) {
    $files[0] =  Asim::choose_filename("Select model configuration file")
      || shell_error("Failed to open model file\n") && return ();
  }
  
  my $status = 1;

  foreach my $file (@files) {
    if (!Asim::Model::Update::update($file)) {
      $status = undef;
    }
  }
  
  return $status;
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

#
# Setup model - parameter defaulting
#
#   setup_benchmark                 -> setup_model $default_benchmark
#   setup_benchmark BENCHMARK       -> setup_model BENCHMARK
#

sub setup_benchmark {
  my $benchmark_file;

  my $model;
  my $benchmark;

  my $model_file = undef;
  my $builddir = "";
  my $rundir = "";

  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "model=s"    => \$model_file,
                        "builddir=s" => \$builddir,
                        "rundir=s"  => \$rundir);

  # Model and/or benchmark are remaining argv values

  $benchmark_file = shift @ARGV;
  $benchmark = get_benchmark($benchmark_file) || return ();

  $model = get_model($model_file) || return ();


  # Remember benchmark

  $default_benchmark = $benchmark;

  print "Trying to set up benchmark $benchmark\n";

  $status = $model->setup($benchmark,
                          "--builddir" => "$builddir",
                          "--rundir" => "$rundir");

  return $status;
}

#
# Run model - parameter defaulting
#
#   run_benchmark             -> run_model $defaul_benchmark
#   run_benchmark BENCHMARK   -> run_model BENCHMARK
#

sub run_benchmark {
  my $benchmark_file;

  my $model_file = undef;
  my $builddir = "";
  my $rundir = "";
  my $options = "";

  my $model;
  my $benchmark;

  my $status;
  local @ARGV = @_;

  # Parse options

  $status = GetOptions( "model=s"    => \$model_file,
                        "builddir=s" => \$builddir,
                        "rundir=s"   => \$rundir,
                        "runopt=s"   => \$options,
                        "options=s"  => \$options);

  # Model and/or benchmark are remaining argv values

  $benchmark_file = shift @ARGV;
  $benchmark = get_benchmark($benchmark_file) || return ();

  $model = get_model($model_file) || return ();

  print "Trying to run benchmark $benchmark\n";

  $status = $model->run($benchmark,
                        "--builddir" => "$builddir",
                        "--rundir" => "$rundir",
                        "--runopt" => "$options");

  return $status;
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

# only works for global params - for now. 
sub set_parameter {
  my $modelfile = shift;
  my $parameter_name = shift;
  my $parameter_value = shift;

  my $model_resolved_path = $Asim::default_workspace->resolve($modelfile);
  my $model = get_model($model_resolved_path);

  $model->setparameter($parameter_name, $parameter_value);
  $model->save();

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

sub replace_module {
  my $modelfile = shift;
  my $submodelfile = shift;

  my $model = get_model($modelfile);
  my $submodel = get_model($submodelfile);
  $model->smart_add_submodule($submodel);
  $model->save();
}

################################################################
#
# Submodel functions
#
################################################################
sub create_submodel {
  my $modelfile = shift;
  my $module = shift;
  my $submodelfile = shift;

  # we need to awb resolve the paths in case absolute paths are not given
  # submodle may not exist yet, so it needs extra handling
  (my $subvolume, my $subdir , my $subfile) = File::Spec->splitpath( $submodelfile );
  my $sub_resolved_path = $Asim::default_workspace->resolve($subdir);

  my $model_resolved_path = $Asim::default_workspace->resolve($modelfile);

  my $submodel = Asim::Model->new();

  my $model = get_model($model_resolved_path);
  my $submodule = $model->find_module_providing($module);
  $submodel->modelroot($submodule);

  $submodel->save("$sub_resolved_path/$subfile")
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
