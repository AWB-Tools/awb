#
# *****************************************************************************
# *
# * @brief Cvs.pm : utility methods to handle CVS repositories
# *
# * @author Mohit Gambhir
# *
# * Copyright (c) 2009 Intel Corporation, all rights reserved.
# * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
# *
# *****************************************************************************
#

package Asim::Repository::Cvs;
use warnings;
use strict;

use File::Basename;

our @ISA = qw(Asim::Repository);

our $DEBUG =  0
           || defined($ENV{ASIM_DEBUG})
           || defined($ENV{ASIM_DEBUG_REPOSITORY});

=head1 NAME

Asim::Repository::Cvs - Class to access and manipulate an Cvs repository.

=head1 SYNOPSIS

use Asim::Repository::Cvs;

Asim::Repository::Cvs::checkout();

my $cvs = Asim::Repository::Cvs->new("~/cvsdir");

$cvs->update();


=head1 DESCRIPTION

This is a class to allow access to an cvs repository

This is a subclass of Asim::Repository.  After creating an instance of 
Asim::Repository, you can call the set_type() method here to check whether the package type in asim.pack is CVS and set it to this subclass if so.  Set_type will return 
1 if it is a CVS package.  If it returns 0, you should keep checking other repository types.

=head1 METHODS

The following methods are supported:

=over 4

#################################################################################

=item $cvs = Asim::Repository::Cvs::set_type( $package )

If $package is an CVS repository, set the object's subclass to Asim::Repository::Cvs and return 1.
Otherwise return 0.

=cut

################################################################################

sub set_type {
  my $self = shift;
  # If method is cvs, then this is CVS repository
  if ( $self->{method} eq "cvs" || $self->{method} eq "pserver"  ) {
    bless $self;
    return 1;
  }
  return 0;
}

#################################################################################

=item $repository-E<gt>type()

Return type of package - "cvs" in this case.

=cut

#################################################################################

sub type {
  return 'cvs';
}

#################################################################################

=item $cvs = Asim::Repository::Cvs:init()

Global init of Cvs module.

=cut

#################################################################################

sub init {
  # Deal with any CVS environment variables
  1;
}

#################################################################################

=item $dir = $repository-E<gt>checkout([$user])

Check out a cvs repository. 
Returns the directory the result was checked out into.

=cut

################################################################################

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

  # Call Asim::Repository::checkout to perform pre checkout validation
  if ( ! $self->SUPER::checkout() ) {
    return undef;
  }
  if ($user ne "anonymous") {
    if ($method eq "cvs") {
      $access =~ s/:pserver:anonymous/$user/;

      if (! defined($ENV{CVS_RSH}) ) {
        ierror("Environment variable CVS_RSH not set.\n" .
               "I've set it to \"ssh\", which should be what you want.\n");

        $ENV{CVS_RSH} = "ssh";
      }
    }
    elsif ($method eq "pserver") {  
      $access =~ s/anonymous/$user/;
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

################################################################################
#
# Internal error utility function
#
################################################################################

sub ierror {
  my $message = shift;

  print "Repository::Cvs: Error - $message";

  return 1;
}

=back

=head1 BUGS

=head1 AUTHORS

Mohit Gambhir

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2009

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
