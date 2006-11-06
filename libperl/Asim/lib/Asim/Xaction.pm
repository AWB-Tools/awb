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

package Asim::Xaction;
use warnings;
use strict;


#
# Some package wide globals
#

our $index = 0;

our @abort_log = ();
our @commit_log = ();

our $VERBOSE = 1;


=head1 NAME

Asim::Xaction - A library for running an transaction-like operation on
normal files in the filesystem.


=head1 SYNOPSIS

  Asim::Xaction::start()

  Asim::Xaction::add_file($file_being_created_or_modified_during_transaction);

  if ( $things_went_well ) {
    Asim::Xaction::commit();
  } else {
    Asim::Xaction::abort();
  }

=head1 DESCRIPTION

This is a simpleminded library that facilitates a tranaction-like
semantic on filesystem changes by allowing the user to bracket (in
time) a set of operations on an explicilty specified set of files as a
commitable or abortable transaction.  Thus, after starting a
transaction the user must explictly specify each file he/she is
creating or modifying during the transaction. The functions in this
libary will keep an undo log for each file so identified and if
necessary create backups of each file being updated. The user may then
ultimately commit or abort the transaction. On commit, the log and
backup files will be destroyed. On abort, the backups will be restored
and any newly created files will be removed.

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=cut

################################################################


################################################################

=item Asim::Xaction::start()

=cut

################################################################

sub start {

  if (@abort_log) {
    die("Xaction: Illegal nested transaction\n");
    exit 0;
  }

  push(@abort_log, 'echo "Transaction Aborted"');
  push(@commit_log, 'echo "Transaction Commited."');

  return 1;
}


################################################################

=item Asim::Xaction::add_path()

Uimplemented - add all new directorires in the path...

To make a tranactional mkpath...

=cut

################################################################

sub add_path {
  dir("Xaction: add_path - not implemented\n");
}

################################################################

=item Asim::Xaction::add_dir()

=cut

################################################################

sub add_dir {
  my $dir = shift;

  @abort_log || die("Xaction: add_file when not in a transaction");

  if (-e $dir) {
    return 1;
  }

  unshift(@abort_log, "rm -rf $dir");
    
}


################################################################

=item Asim::Xaction::add_file()

=cut

################################################################

sub add_file {
  my $file = shift;
  my $tempfile;

  @abort_log || die("Xaction: add_file when not in a transaction");

  if (-e $file) {
    #
    # File currently exists...
    #
    $tempfile = "/tmp/$index.$$.bak";
    $index++;

    #
    # Back up current version of file
    #
    system "cp $file $tempfile";

    #
    # Remove backup on commit
    #
    unshift(@commit_log, "rm -f $tempfile");

    #
    # Restore original file on abort
    #
    unshift(@abort_log, "mv $tempfile $file");

  } else {

    #
    # Do nothing on commit
    #

    #
    # Restore newly created file on abort
    #
    unshift(@abort_log, "rm -f $file");

  }

  return 1;
}


################################################################

=item Asim::Xaction::protect_file()

=cut

################################################################

sub protect_file {
  my $file = shift;
  my $tempfile;

  #
  # Arguments checks
  #
  @abort_log   || die("Xaction: protect_file when not in a transaction");

  (! -d $file) || die("Xaction: protect_file cannot take a directory");

  #
  # Handle existent and non-existent files
  #
  if (-e $file) {
    #
    # File currently exists...
    #
    $tempfile = "/tmp/$index.$$.bak";
    $index++;

    #
    # Back up current version of file
    #
    system "cp $file $tempfile";

    #
    # Restore backup on commit
    #
    unshift(@commit_log, "mv $tempfile $file");

    #
    # Restore original file on abort
    #
    unshift(@abort_log, "mv $tempfile $file");
  } else { 
    #
    # File does not exist
    #
    #
    # Remove file on commit
    #
    unshift(@commit_log, "rm -f $file");

    #
    # Remove file on abort
    #
    unshift(@abort_log, "rm -f $file");

  }
    

  return 1;
}

################################################################

=item Asim::Xaction::add_custom($commit_sub, $abort_sub)

Add user define commit and abort commands

How do I handle this for method calls or calls with arguments...

Unimplemented...

=cut

################################################################

sub add_custom {

  die("Add_custom: Unimplmented\n");

}
  
################################################################

=item Asim::Xaction::commit()

=cut

################################################################

sub commit {

  @abort_log || die("Xaction: commit when not in a transaction");

  foreach my $a (@commit_log) {
    print "$a\n" if ($VERBOSE);

    system($a)
      && die("Xaction: Commit bug...$a failed");
  }

  @commit_log = ();
  @abort_log = ();

  return 1;
}

################################################################

=item Asim::Xaction::abort()

=cut

################################################################

sub abort {

  @abort_log || return 0;

  foreach my $a (@abort_log) {
    print "$a\n" if ($VERBOSE);

    system($a)
      && die("Xaction: Abort bug...$a failed");
  }

  @commit_log = ();
  @abort_log = ();

  return 1;
}



=back

=head1 BUGS

Nested trasaction are not implemented.

File manipulations should be converted to use File::Copy (TODO).

There are probably security issues with using temp files...

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;
