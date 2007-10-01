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


package Asim::Inifile;
use warnings;
use strict;

use Fcntl ':flock';
use File::Copy;
use File::stat;

my $debug = 0;

=head1 NAME

Asim::Inifile - Library for manipulating ASIM configuration files

=head1 SYNOPSIS

use Asim::Inifile;

$inifile = Asim::Inifile->new("file.ini");
$value = $inifile->get("Global","Version");
$inifile->put("Global","Version",$value+1);
$inifile->save("filenew.ini");


=head1 DESCRIPTION

This module provides an object to manipulate a windows style ini file
with groups of values that can be interogated by the program.

File looks like:

 [Group1]
 NAME1=VALUE1
 NAME2=VALUE2

 [Group2]
 NAME3=VALUE3
 NAME4=VALUE4
 ....

=cut

################################################################

################################################################
# WARNING: 
#    The charactistics (especially file parsing) of this
#    object must stay in sync with the tcl file ini.tcl
#
################################################################

################################################################

=head1 METHODS

The following public methods are supported:

=cut
=over 4

################################################################

=item $inifile = Asim::Inifile-E<gt>new_as_class($file)

Open an inifile then determine its class and return an
object of the appriate class

=cut

################################################################

sub new_as_class {
  my $this = shift;
  my $file = shift;

  # Determine class of object $file contains

  my $inifile = Asim::Inifile->new($file)        || return undef;
  my $class   = $inifile->get("Global", "Class") || return undef;

  # Reopen $file as a new object of class obtained from $file.

  my $object;

  eval("\$object = $class->new(\$file)");

  return $object;
}


################################################################

=item $inifile = Asim::Inifile-E<gt>new($file)

Create a new configuration file database from $file.

=cut

################################################################

sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $file = shift;

  my $self = {};

  bless	$self, $class;

  $self->{filename} = $file;
  $self->{data} = {};

  if (defined($file)) {
    $self->open($file) || return undef;
  }

  $self->modified(0);
  return $self;
}

our $include_regexp = "^#include[ \t]+\"([^\"]+)";
our $comment_regexp = "^[ \t]*[#;]";
# should we allow this to be so liberal a string...
our $group_regexp = "^[[]([A-Za-z0-9/\(\)\+ :;,_.-]+)[]]\$";
our $item_regexp = "^([A-Za-z0-9_-]+) *= *(.*) *\$";
our $blank_regexp = "^[ \t]*\$";

sub open {
  my $self = shift;
  my $file = shift;

  local *M;
  my $more = '';
  my $line;
  my $curgroup;

  my $data = $self->{data};

  print "Asim::Inifile - Opening inifile = $file\n" if $debug;

  CORE::open(M, "< $file") || return undef;

  my $stat = stat($file);
  $self->{mtime} = $stat->mtime();

  while (<M>) {
    chomp;
    #
    # Check if ends in backslash (\)
    #
    if (/(\\\\)*\\$/) {
      s/^[ \t]+/ /g;        # clean up leading blanks
      s/[ \t]*\\$//g;       # clean up trailing blanks
      $more .= $_;
      next;
    } else {
      s/^[ \t]+/ /g;        # clean up leading blanks
      $line = $more . $_;
      $more = "";
    }

    if ($line =~ /$group_regexp/) {
      #
      # We're in a group now
      #
      $curgroup = $1;
      next;

    } elsif ($line =~ /$item_regexp/) {
      #
      # We've got an item
      #
      $data->{"$curgroup"}{"$1"} = $2;
      print "Asim::Inifile - [$curgroup] $1 = $2\n" if $debug;
      next;

    } elsif ($line =~ /$include_regexp/) {
      #
      # Handle #include file
      #
      $self->open($1) || return undef;
      next;

    } elsif ($line =~ /$comment_regexp/) {
      #
      # Handle comment
      #
      next;

    } elsif ($line =~ /$blank_regexp/) {
      #
      # We have a blank line
      #
      next;

    } else {
      #
      # We have garbage
      #
      warn "Warning: Garbage in inifile $file - line ---->$line<----";
      return undef;
    }
  }

  CORE::close(M);

#  $self->modified(0);
  return 1;
}

################################################################

=item $inifile-E<gt>include($fileglob)

Extend the inifile information with the information from the
files specified with $fileglob.

Note: A save will incorporate all data into a single file

=cut

################################################################


sub include {
  my $self = shift;
  my $fileglob = shift;

  foreach my $i (glob($fileglob)) {
    $self->open($i) || return undef;
  }

  return 1;
}

################################################################

=item $inifile-E<gt>save([$file])

Save the inifile information into the original file or optionally
into the specified $file atomically.

Note: the we try to preserve formatting and comments from the 
original inifile as best we can. Only #include and $self->include()
operations are guaranteed to be lost.

=cut

################################################################


sub save {
  my $self = shift;
  my $file = shift 
    || $self->filename()
    || ierror("No filename specified for save\n") && return ();

  # check for unneeded save

  if ((! $self->modified()) && ("$file" eq $self->filename())) {
      return 1;
  }
  

  my $tmpfile = "$file.$$";
  my $oldfile = $self->filename();

  # BUG we should update the filename here!!!!

  my $data = $self->{data};

  my $curgroup = undef;
  my @comments = ();
  my %groups_done = ();
  my %items_done;

  #
  # Open an temporary file that we will write to and
  # then move on top of the original file
  #
  CORE::open(INI, "> $tmpfile")
    || ierror("Inifile open failed ($tmpfile)\n") && return ();

  #
  # Go through original INI file updating lines that are there,
  # and adding lines that are missing;
  #

  defined($oldfile) || goto NEWFILE;
  CORE::open(OLDINI, "+< $oldfile") || goto NEWFILE;

  unless (flock(OLDINI, LOCK_EX | LOCK_NB)) {
      warn "Warning: File $oldfile already locked; waiting... \n";	# alarm 60;
      flock(OLDINI, LOCK_EX);
  }
  print "Locked file $oldfile\n" if $debug;  

  while (<OLDINI>) {
    chop;

    if (/$group_regexp/) {
      #
      # We're in a group now
      #

      #
      # Finish off items from previous group that weren't there...
      #
      if (defined($curgroup)) {
        print INI "# Rest of group - $curgroup\n" if $debug;
		
	foreach my $i ($self->get_itemlist($curgroup)) {
	  next if $items_done{$i};

	  print INI "# New item - $i\n" if $debug;
	  print INI "$i=" . $self->get($curgroup, $i) . "\n";
	}
      }

      #
      # Spill comments
      #
      print INI @comments;
      @comments = ();

      #
      # Start new group
      #
      $curgroup = $1;
      $groups_done{$curgroup} = 1;
      %items_done = ();

      if (defined($data->{"$curgroup"})) {
	print INI "# Existing group - $curgroup\n" if $debug;
        print INI "[$curgroup]\n";
      }
      next;

    } elsif (/$item_regexp/) {
      # We've got an item

      #
      # Spill comments;
      #
      print INI @comments;
      @comments = ();

      #
      # Print item
      #
      $items_done{$1} = 1;
      if (defined($data->{"$curgroup"}) && defined($data->{"$curgroup"}{"$1"})) {
	print INI "# Existing item - $1\n" if $debug;
        print INI "$1=" . $data->{"$curgroup"}{"$1"} . "\n";
      }
	
    } elsif (/$include_regexp/) {
      #
      # Ignore #include file
      # TODO: Is this the most reasonable semantic
      #
      next;

    } elsif (/$comment_regexp/) {
      #
      # Save comment for printing later
      #
      push(@comments, "$_\n");
      next;

    } elsif (/$blank_regexp/) {
      #
      # Save blank lines - squeezing out multiple blank lines
      #
      if ((scalar(@comments) == 0) ||
          (length($comments[$#comments]) > 1)) {
        push(@comments, "\n");
      }
      next;

    } else {
      #
      # We have garbage
      #
      die "Error: Garbage in inifile $file - line ---->$_<----";
    }
  }
  
  #
  # Clean up the leftover stuff 
  # (this is a copy of some of the new group code --- arggh)
  #
  # First, finish off items from previous group that weren't there...
  #
  if (defined($curgroup)) {
      foreach my $i ($self->get_itemlist($curgroup)) {
	  next if $items_done{$i};
	  
	  print INI "# New item - $i\n" if $debug;
	  print INI "$i=" . $self->get($curgroup, $i) . "\n";
      }
      
      #
      # Then, print the leftover comments
      #
      print INI @comments;
      
  }

  #
  # This loop spills any groups that didn't exist at all
  # in the original file. Thus if we just start here, we'll
  # do a full spill of all groups - but losing formatting...
  #
NEWFILE:
  foreach my $g ($self->get_grouplist()) {

    next if $groups_done{$g};

    print INI "\n";

    print INI "# New group - $g\n" if $debug;
    print INI "[$g]\n";

    foreach my $i ($self->get_itemlist($g)) {
      next if !defined($i);

      print INI "# New item - $i\n" if $debug;
      print INI "$i=" . $self->get($g, $i) . "\n";
    }
  }

  CORE::close(INI);

  if (defined($oldfile)) {
      if ("$file" ne $self->filename())  {	# write to a new file
	CORE::close(OLDINI);
 	system("mv $tmpfile $file");
      }
      else {	      # move the contents from the temporary file
	CORE::open(INI, "< $tmpfile") || ierror("Inifile open failed ($tmpfile); cannot update $oldfile\n") && return ();
	my @lines = <INI>;
	CORE::close(INI);      
	system("rm $tmpfile");
	  
	seek(OLDINI, 0, 0);	
	truncate(OLDINI, 0);	
	print OLDINI @lines;
	CORE::close(OLDINI);
	  
      }
      print "Unlocked file $oldfile\n" if $debug;
  }
  else {
      system("mv $tmpfile $file");
  }

  $self->{filename} = $file;

  my $stat = stat($file);
  $self->{mtime} = $stat->mtime();

  $self->modified(0);
  return 1
}

################################################################

=item $inifile-E<gt>backup()[$file])

Save the inifile (or optionally the specificed $file) into a
new file with a suffix of the form .~<version>~.

Note: backups are 0 extended up to 3 digits for nice lexical
sorts. If one has more than 999 backups, then probably nice
lexical sorts aren't going to make things more comprehensible.

=cut

################################################################


sub backup {
  my $self = shift;
  my $filename = shift | $self->filename();

  if (! -e $filename) {
    return undef;
  }

  my $v = "001";

  while (-e "$filename.~$v~") {
    $v = sprintf("%03d", $v+1);
  }

  copy($filename, "$filename.~$v~") || return undef;

  return 1;
}


################################################################

=item $inifile-E<gt>filename()

Filename of source of inifile information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $inifile-E<gt>get($group, [$item])

Return the value in group $group for item $item. If $item is not
specified return a reference to a hash for $group.

=cut

################################################################


sub get {
  my $self = shift;
  my $group = shift || return undef;
  my $item = shift;

  my $data = $self->{data};

  if ( defined($item) ) {
    return $data->{"$group"}{$item};
  } else {
    return $data->{"$group"};
  }
}

################################################################

=item $inifile-E<gt>put($group, $item, $value)

Set the value in group $group for item $item as $value

=cut

################################################################


sub put {
  my $self = shift;
  my $group = shift;
  my $item = shift;
  my $value = shift;

  my $data = $self->{data};

  if (!defined($value)) {
    print "Inifile: Put of  undefined value for $group/$item\n" if ($debug);
    return undef;
  }

  my $oldvalue = $data->{"$group"}{"$item"};

  if ((!defined($oldvalue)) || ($oldvalue ne "$value")) {
      #print "Setting [$group] $item=$value\n";
      $data->{"$group"}{"$item"} = $value;

      $self->modified(1);
  }

  return $value;
}

################################################################

=item $inifile-E<gt>remove($group, [$item])

Remove entry in group $group for item $item. If $item is not
specified remove the entire group $group.

=cut

################################################################


sub remove {
  my $self = shift;
  my $group = shift || return undef;
  my $item = shift;

  my $data = $self->{data};

  if ( defined($item) ) {
    $data->{"$group"}{$item} = undef;
  } else {
    $data->{"$group"} = undef;
  }

  $self->modified(1);
  return 1;
}

################################################################

=item $inifile-E<gt>get_grouplist()

Return the list of groups

=cut

################################################################

sub get_grouplist {
  my $self = shift;
  my $data = $self->{data};

  return (keys %{$data});
}

################################################################

=item $inifile-E<gt>get_itemlist($group)

Return the list of names used in a group

=cut

################################################################


sub get_itemlist {
  my $self = shift;
  my $group = shift;

  my $data = $self->{data};

  my @list = keys %{$data->{"$group"}};

  return @list;
}


################################################################

=item $inifile-E<gt>copy_group($oldgroup, $newgroup)

Copy all items in group $oldgroup to $newgroup

=cut

################################################################


sub copy_group {
  my $self = shift;
  my $oldgroup = shift;
  my $newgroup = shift;

  foreach my $i ($self->get_itemlist($oldgroup)) {
    $self->put($newgroup, $i, $self->get($oldgroup, $i));
  }

  return 1;
}

################################################################

=item $inifile-E<gt>flush()

Flush all associations

=cut

################################################################

sub flush {
  my $self = shift;

  $self->{data} = {};
  $self->modified(1);
}


################################################################

=item $object-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################


sub modified {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"modified"} = $value;
      print "Modified = $value\n" if ($debug);
  }

  return $self->{"modified"};
}


################################################################

=item $object-E<gt>file_modified()

Return a boolean indicating whether the file backing the inifile
has been modified

Note: this method is only meaningful if used under some file lock.

=cut

################################################################


sub file_modified {
  my $self = shift;

  my $stat = stat($self->filename()) || return undef;
  my $mtime = $stat->mtime();

  return $mtime != $self->{mtime};
}



################################################################

=item $inifile-E<gt>dump()

Dump the configuration file

=cut

################################################################


sub dump {
  my $self = shift;
  my $data = $self->{data};

  foreach my $i (sort keys %$data) {
    print "$i = $data->{$i}\n";
    foreach my $j (sort keys %{$data->{$i}}) {
      print "  $j = $data->{$i}{$j}\n";
    }
  }
}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
  my $message = shift;

  print "Inifile: Error - $message";

  return 1;
}

=back

=head1 BUGS

No support yet for creation and saving of a new inifile...

Format of multiline items is not preseved on save.

This class should be restructured so that one can derive other
Asim object classes from it directly.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
