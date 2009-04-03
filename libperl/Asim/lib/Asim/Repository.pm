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


package Asim::Repository;
use warnings;
use strict;

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});


=head1 NAME

Asim::Repository - Library for manipulating a CVS repositry containing
an Asim repository.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################


=head1 METHODS

The following public methods are supported:

=over 4

=item $repository = Asim::Repository-E<gt>new(packagename => <name of package>,
                                        method => [cvs|svn|bitkeeper],
                                        access => <accessinfo>,
                                        module => <CVSmodule>,
                                        tag => <CVStag>,
                                        browseURL => <browseURL>,

                                        target => <targetdirectory>,
                                        changes=> <changesfilename>);


Create a new repository object.

=cut

################################################################

sub new {
  my $this = shift;

  my $class = ref($this) || $this;
  my $self;

  $self = {@_};

  bless	$self, $class;

  return $self;
}


################################################################

=item $repository-E<gt>create()

Create a new package repository. 

=cut

################################################################

sub create {
  print <<'EOF';

Perform the following manual steps:

1) Create a new package, maybe with something like:

       % asim-shell new package <pkgname>


2) If necessary, create a CVS repository probably with something like:

       % cvs -d <name>@<node>:/cvsroot/<dir> init

   This might not be necessary if you use something like sourceforge which
   will create a cvs repository for you.


3) By default the CVS module name will be <dir> as specified above.
   If you want this package to be be a module of the repository with another name
   you may need to update CVSROOT/admin/modules to include package as a CVS module.

   See the CVS instructions for instructions...


4) Add the package to the asim core package and re-install it.

       a) Add package to <workspace>/src/asim-simcore/etc/asim.pack file.

       b) Check in changes to asim-simcore.

       c) Reinstall asim-simcore. Probably that means:
               ./configure
               make
               make install

          Check <workspace>/src/asim-simcore/INSTALL if you want to be sure.


5) Import the package with something similar to:

       % cd <workspaceroot>/src/asim-<pkgname>
       % cvs -d <name>@<node>:/cvsroot/<dir> import asim-<pkgname> <vendor> start


6) If you want anonymous access:

       % # Check out CVSROOT
       % cd <scratch-directory>
       % cvs -d <name>@<node>:/cvsroot/<dir> checkout CVSROOT
       % cd CVSROOT

       % # Create the 'readers' file
       % echo 'anonymous' >readers
       % cvs add readers
       % touch passwd

       % # Create the 'writers' file
       % touch writers
       % cvs add writers

       % # Create the 'passwd' file
       % touch passwd
       % cvs add passwd
       % htpasswd -c passwd anonymous
       Password: <secret-password>
       % # make sure passwd file looks like "anonymous:sh32njsgjs"

       % # Commit changes
       % cvs commit

7) To work on this package you need to delete it and check it out, e.g.:

       % rm -rf <workspaceroot>/src/asim-<pkgname>
       % asim-shell checkout package <pkgname>


EOF

return 1;
}

################################################################

=item $dir = $repository-E<gt>checkout([$user])

Check out a repository. 
Returns the directory the result was checked out into.

=cut

################################################################
sub checkout {
  my $self = shift;
  my $archive = $self;
  my $user = shift || "anonymous";

  my $method = $archive->{method};

  my $access = $archive->{access} || return undef;
  my $tag    = $archive->{tag}    || return undef;
  my $module = $archive->{module} || return undef;
  my $target = $archive->{target} || return undef;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;

  # First check if the target dir exists
  # and if it is writable

  if (! -d "$targetdir" || ! -w "$targetdir") {
    ierror("Malformed workspace - check if src directory is present and if it is writable.\n");
    return undef;
  }

  # Make sure we have a packagename

  if (! defined($self->packagename())) {
    ierror("No packagename for this package.\n");
    return undef;
  }

  # Remove pre-existing package

  if (-d $package_dir) {
    print "Removing old version of package\n";
    system("rm -rf $package_dir");
  }

  print "Beginning checkout\n";
  
  # first parse the tag into a branch name and CSN number
  
  my $tag_branch = '';
  my $tag_csn = 0;
  if      ( $tag =~ m/^CSN-(.+)-([0-9\.]+)$/ ) {
    $tag_branch = $1;
    $tag_csn    = $2;
  } elsif ( $tag =~     m/^(.+):([0-9\.]+)$/ ) {
    $tag_branch = $1;
    $tag_csn    = $2;
  } elsif ( $tag =~          m/^([0-9\.]+)$/ ) {
    $tag_csn    = $1;
  } elsif ( $tag =~                m/^(.+)$/ ) {
    $tag_branch = $1;
  }
  if ( $tag_branch eq $self->packagename() ) {
    $tag_branch = '';
  }
  
  # now perform method-specific actions.
  # FIX FIX!! move into derived classes eventually, just like in Package.pm

  if ( $method eq "cvs") {

    # Do a checkout via CVS

    if ($user ne "anonymous") {
      $access =~ s/:pserver:anonymous/$user/;

      if (! $ENV{CVS_RSH} ) {
        ierror("Warning: Environment variable CVS_RSH not set.\n",
             "Warning: I've set it to \"ssh2\", which should be what you want.\n");

        $ENV{CVS_RSH} = "ssh2";
      }
    }

    my $cmd = "";
    if ($tag ne "HEAD") {
      $cmd = "(cd $targetdir; cvs -f -d $access checkout -P -d $target -r $tag $module)";
    } else {
      $cmd ="(cd $targetdir; cvs -f -d $access checkout -P -d $target $module)";
    }
    printf STDERR "Executing: %s\n", $cmd;
    $status = system($cmd);


    if ($status) {
      if ($user eq "anonymous" &&
          Asim::choose_yes_or_no("Do you want to try a 'cvs login' and retry the checkout","no","no")) {
	if (!system("(cd $targetdir; cvs -f -d $access login $module)")) {
	  return $self->checkout($user);
        }
      }
      ierror("Cvs checkout failed\n");
      return undef;
    }

  } elsif ( $method eq "pserver" ) {

    # Do a checkout via pserver access to CVS

    if ($user ne "anonymous") {
      $access =~ s/anonymous/$user/;
    }

    if ($tag ne "HEAD") {
      $status = system("(cd $targetdir; cvs -f -d $access checkout -P -d $target -r $tag $module)");
    } else {
      $status = system("(cd $targetdir; cvs -f -d $access checkout -P -d $target $module)");
    }

    if ($status) {
      if ($user eq "anonymous" &&
          Asim::choose_yes_or_no("Do you want to try a 'cvs login' and retry the checkout","no","no")) {
	if (!system("(cd $targetdir; cvs -f -d $access login $module)")) {
	  return $self->checkout($user);
        }
      }
      ierror("Cvs checkout failed\n");
      return undef;
    }

  } elsif ( $method eq "copy" ) {

    # Do a 'checkout' via a copy of a publically available version

    # Force / at end of access
    $access =~ s-([^/])$-$1/-;

    if ( ! -d $access) {
      ierror("No public package to copy available at $access\n");
      return undef;
    }

    # Copy the package
    #   Remember to exclude all repository administration files (CVS/, .svn/, SCCS/)
    #   This must match the method Asim::Package::Copy::update()

    $status =  system("rsync -av --exclude=CVS/ --exclude=.svn/ --exclude=SCCS/ $access $package_dir");
    $status |= system("echo $access >$package_dir/COPY");

    if ($status) {
      ierror("Package rsync failed\n");
      return undef;
    }

  } elsif ( $method eq "bitkeeper" ) {

    # FIX!
    # deal with branches, which in the case of BK are just separate clones...

    my $url = "$access";
    if ($user ne "anonymous") {
      $url =~ s/anonymous/$user/;
    }

    my $cmd;
    if ($tag eq "HEAD") {
      $cmd ="(cd $targetdir; bk clone $url $target)";
    } else {
      $cmd ="(cd $targetdir; bk clone -r$tag $url $target)";
    }
    printf STDERR "Executing: %s\n", $cmd;
    $status = system($cmd);

  } elsif ( $method eq "svn" ) {
  
    ### get the URL

    my $url = "$access";
    if ( ! $tag_branch || $tag_branch eq 'HEAD' ) {
      $url .= '/trunk';
    } else {

      # Non-head checkout,
      # where tag is a general string.
      # Look in both "tags" and "branches" to find the label and check that one out:
      my $is_tag_or_branch = 0;
      foreach my $subdir ( 'tags', 'branches' ) {
	open LIST, "svn list $url/$subdir |";
	while ( <LIST> ) {
          if ( m/^$tag_branch\// ) {
            # Pick up the tag directory
            $url .= "/$subdir/$tag_branch";
            $is_tag_or_branch = 1;
	    last;
          }
	}
	close LIST;
      }
      # couldn't find it in either tags or branches?
      # Tag must be a date, so try checking out using it as a date string:
      if ( ! $is_tag_or_branch ) {
        if ( $tag_csn ) {
          ierror("branch or tag $tag_branch not found!\n");
        } else {
          $url .= '/trunk';
          $tag_csn = "{\"$tag\"}";
        }
      }

    }
    
    ### construct the checkout command
    
    my $cmd = "(cd $targetdir; svn checkout ";
    if ( $tag_csn ) {
      $cmd .= "-r $tag_csn ";
    }
    $cmd   .= "--username $user $url $target)";
    
    ### execute the command

    printf STDERR "Executing: %s\n", $cmd;
    $status = system($cmd);

  } else {
    ierror("Error: Asim::Repository->checkout() - Unimplemented checkout method ($method)\n");
    return undef;
  }

  print("Package checked out at $package_dir\n") if ($DEBUG);

  #
  # Change the permissions on the directory just checked out,
  # to keep it private to the current user:
  #
  system("chmod 0700 $package_dir");

  #
  # Rehash the workspace since we have a new package in it
  #
  $Asim::default_workspace->rehash();

  #
  # We return the base directory so that the caller can 
  # see if it is in the path...since we do not add it.
  #
  return $package_dir;
}



################################################################

=item $workspace-E<gt>browse()

Open a grapichal browser on the repository

=cut

################################################################

sub browse {
  my $self = shift;
  my $location = $self->{browseURL} || return undef;

  #
  # TBD: Get repository browser from configuration option
  #

  my $status = system ("firefox $location &");
  return $status?undef:1;

}


################################################################

=item $packagename = $repository-E<gt>packagename()

Returns the name of the package stored in this repository

=cut

################################################################

sub packagename {
  my $self = shift;
  my $packagename;

  $packagename = $self->{packagename} || return undef;

  return $packagename;
}

################################################################

=item $checkoutdir = $repository-E<gt>checkoutdir()

Returns the directory the repository would be checked out into.

=cut

################################################################

sub checkoutdir {
  my $self = shift;
  my $target_dir;

  $target_dir = $self->{target} || return undef;

  # Note:
  #  Portions of checkout depend on exactly this concatentation...

  return $self->_checkoutbasedir() . "/$target_dir";
}


sub _checkoutbasedir {
  my $self = shift;

  return $Asim::default_workspace->src_dir();
}

################################################################

=item $modDB-E<gt>dump()

Textually dump the contents of the object

=cut

################################################################


sub dump {
  my $self = shift;
  my $level = shift;

  printf "Repository type: %s\n",             _safe_string($self->{method});
  printf "Repository access: %s\n",           _safe_string($self->{access});
  printf "Repository module: %s\n",           _safe_string($self->{module});
  printf "Repository tag: %s\n",              _safe_string($self->{tag});
  printf "Repository browseURL: %s\n",        _safe_string($self->{browseURL});
  printf "Repository target directory: %s\n", _safe_string($self->{target});
  printf "Repository changes file: %s\n",     _safe_string($self->{changes});

}


sub _safe_string {
  my $value = shift;

  if (! defined($value)) {
     return "undef";
  }

  return $value;
}

################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Repository: Error - $message";

  return 1;
}


=back

=head1 BUGS

The complex (and unchecked) arguments to 'new' makes me uncomfortable
with it as a public interface. At present only RepositoryDB needs to call
it, but still...

Checkouts go to default target directory. Nor does checkout allow non-anonymous checkouts.
Checkout has very little error checking...


=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
