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


package Asim;

#
# Globally available utility functions
#
sub enable_signal {
  $SIG{'INT'} = \&accept_signal;
}

sub disable_signal {
  $SIGN{'INT'} = 'IGNORE';
}


@cleanup_handlers = ();

sub push_signal_handler {
  my $handler = shift;

  push(@cleanup_handlers, $handler);
}

sub pop_signal_handler {
  pop(@cleanup_handlers);
}


sub accept_signal {
  my $signame = shift;

  print "\n";
  print "Aborting due to ^C...\n";

  foreach my $i (@cleanup_handlers) {
    eval $i;
  }

  Asim::Xaction::abort();

  $| = 0;

  exit 1;
}


sub reject_signal {
  my $signame = shift;

  print "Sorry no signal allowed now\n";
}


# $SIG{'__DIE__'} = \&accept_signal;

1;
