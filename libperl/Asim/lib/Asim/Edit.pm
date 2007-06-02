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
use warnings;
use strict;

use Asim::Edit::Interactive;
use Asim::Edit::Noninteractive;

#
# Globally setting this could help with non-interactive use of the script...
#
our $editcontrol;
our $debug = 0;


###############################################################
#
# Mode setting function
#
###############################################################

#
# Return current mode
# Optionally set mode to "interactive" or "batch".
#

sub mode {
  my $mode = shift;
  my $oldmode;

  if (! defined($mode)) {
    die("Asim::mode:: attempt to switch to undefined editing mode\n");
  }

  print "Switching edit mode to <$mode>\n" if ($debug);

  if (! defined($editcontrol)) {
    $oldmode = "interactive";
  } else {
    $oldmode = $editcontrol->mode();
  }

  if (($mode =~ /[Ii]nteractive/) || ($mode eq "1")) {
    $editcontrol = Asim::Edit::Interactive->new();
  }
  elsif (($mode =~ /[Bb]atch/)   || ($mode eq "0")) {
    $editcontrol = Asim::Edit::Noninteractive->new();
  }
  else {
    die("Attempt to switch to unknown editing mode <$mode>\n");
  }

  print "Switched edit mode to <" . $editcontrol->mode() . ">\n" if ($debug);

  return $oldmode;
}


sub choose {
  return $editcontrol->choose(@_);
}


sub choose_yes_or_no {
  return $editcontrol->choose_yes_or_no(@_);
}


sub choose_name {
  return $editcontrol->choose_name(@_);
}

sub choose_from_list {
  return $editcontrol->choose_from_list(@_);
}


sub choose_filename {
  return $editcontrol->choose_filename(@_);
}


#
# A trampoline to max_common for compatibility
#

sub max_common {
  return Asim::Edit::Interactive::max_common(@_);
}

# Make perl happy...

1;
