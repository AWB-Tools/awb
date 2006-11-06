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

package Asim::Package;
use warnings;
use strict;

#@ISA=(Asim::Base);

use File::Basename;

# Documentation of the public functions is in Package.pm

################################################################
#
# Function: 
#
# 
#
################################################################

sub safe_append {
  my $handle = shift;
  my $file = shift;
  
  #
  # Make sure 'changes' is writeable
  #
  if (! -e $file) {
    die("safe_append: called with file that does not exist ($file)\n");
  }

  chmod 0644, $file
    || ierror("safe_append: $file not writable\n") && return ();

  no strict 'refs';
  CORE::open($handle, ">> $file") 
    || ierror("FAILURE: Could not open $file\n") && return ();

  return 1;
}


sub safe_close {
  my $handle = shift;
  
  no strict 'refs';
  CORE::close($handle);
}





################################################################


sub banner {
  my $self = shift;
  
  my $name = $self->name();
  my $status = $self->{status} || "";

  print "=======================================================\n";
  print "         $name - $status\n";
  print "=======================================================\n";

  return 1;
}


################################################################

our $stepnum = 0;

sub step {
  my $self = shift;
  my $message = shift;
  my $newstep = shift;

  if (defined($newstep)) {
    $stepnum = $newstep;
  }

  $stepnum++;

  print "\n";
  print "Step $stepnum: $message\n";
  print "\n";

  return 1;
}

1;
