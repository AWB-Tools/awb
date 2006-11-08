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

=item $repository = Asim::Repository-E<gt>new(method => cvs, 
                                        access => <accessinfo>,
                                        module => <CVSmodule>,
                                        tag => <CVStag>,

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

  my $access = $archive->{access} || return 0;
  my $tag    = $archive->{tag}    || return 0;
  my $module = $archive->{module} || return 0;
  my $target = $archive->{target} || return 0;

  my $targetdir = $self->_checkoutbasedir();
  my $package_dir = $self->checkoutdir();

  my $status;

  # First check if the target dir exists
  # and if it is writable

  if (! -d "$targetdir" || ! -w "$targetdir") {
    print "Error: Malformed workspace. \n";
    print "Check if src directory is present and if it is writable.\n";
    return undef;
  }

  # Remove pre-existing package

  if (-d $package_dir) {
    print "Removing old version of package\n";
    system("rm -rf $package_dir");
  }

  print "Beginning checkout\n";

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
      return 0;
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
      return 0;
    }

  } elsif ( $method eq "copy" ) {

    # Do a 'checkout' via a copy of a publically available version

    # Force / at end of access
    $access =~ s-([^/])$-$1/-;

    $status =  system("rsync -av --exclude=CVS $access $package_dir");
    $status |= system("echo $access >$package_dir/COPY");

    if ($status) {
      ierror("Package rsync failed\n");
      return 0;
    }

  } elsif ( $method eq "bitkeeper" ) {

    my $url = "$access";
    if ($user ne "anonymous") {
      $url =~ s/anonymous/$user/;
    }

    my $cmd ="(cd $targetdir; bk clone $url $target)";
    printf STDERR "Executing: %s\n", $cmd;
    $status = system($cmd);

  } elsif ( $method eq "svn" ) {

    my $url = "$access";

    # By default, checkout latest revision
    # Elsif tag specified, get the corresponding revision no.
    # since tag no. = revision no. for svn

    if ($tag eq "HEAD") {

      # Head checkout

      $url.="/trunk";
      my $cmd ="(cd $targetdir; svn checkout --username $user $url $target)";
      printf STDERR "Executing: %s\n", $cmd;
      $status = system($cmd);
    } else {

      # Non-head checkout

      my $tmp_svn_list = "/tmp/asim-shell-svn-list.$$";  
      my $tmp_url = "$url/tags/$tag";  
      system ("svn list $tmp_url > $tmp_svn_list 2>&1");
      CORE::open(TMP, "< $tmp_svn_list");

      while (<TMP>) {  
        if (/non-existent/) {
          # Pick up the corresponding revision no.
          my @tmp;
          $url.="/trunk";
          @tmp = split(/-/,$tag); # Tag:CSN-axp-no.
          # Pick up the revision no.
          if ($tmp[0] =~ /CSN/) {
            my $cmd ="(cd $targetdir; svn checkout -r $tmp[2] --username $user $url $target)";
            printf STDERR "Executing: %s\n", $cmd;
            $status = system($cmd);
          } else {
          # Else pick up that date
            my $date ="{\"$tag\"}";
            my $cmd ="(cd $targetdir; svn checkout -r $date --username $user $url $target)";
            printf STDERR "Executing: %s\n", $cmd;
            $status = system($cmd);
          }
        } else {
          # Pick up the tag directory
          $url.="/tags/$tag";
          my $cmd ="(cd $targetdir; svn checkout --username $user $url $target)";
          printf STDERR "Executing: %s\n", $cmd;
          $status = system($cmd);
        }
        last;
      }

      CORE::close(TMP);
      system ("rm $tmp_svn_list");
    }
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
