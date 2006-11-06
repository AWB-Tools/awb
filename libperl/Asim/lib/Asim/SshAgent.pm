#
# *****************************************************************************
# *
# * @brief SshAgent.pm : utility methods to handle SSH agent
# *
# * @author Oscar Rosell
# *
# Copyright (C) 2005-2006 Intel Corporation
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

sub is_ssh_agent_running {
  my $exit_status;
  my $error;
  if (CORE::open(SSH,"ssh-add -l 2>&1 |") ) {
    $exit_status = 0;
    while (<SSH>) {
      if (m/agent has no (identities|keys)/i) {
        $exit_status = 1;
        $error = $_;
        last; # can't find valid keys
      } elsif (m/(failed to connect|could not open a connection)/i) {
        $exit_status = 1;
        $error = $_;
        last; # can't connect to ssh-agent - not running?
      }
    }
    CORE::close(SSH);
  } else {
    $exit_status = 1; # open failed ?
  }

  return $exit_status;
}

sub start_ssh_agent {
  ##FIXME 11/14/03 Mark Charney: Make this ssh2/ssh-agent2 friendly!
  #
  # if we have done this already, we don't do it again
  #
  if ($SSH_AGENT_KILL_CMD ne "") {
    iwarn("temporary ssh-agent already started - not starting again\n");
    return;
  }

  #
  # start an ssh-agent and put it into the environment
  #
  my @ssh_agent = `ssh-agent -c`;
  foreach (@ssh_agent) {
    if (m/^setenv (\S+)\s+(\S+);/) {
      $ENV{$1} = $2;
    }
  }

  #
  # figure out if we are dealing with OpenSSH or F-Secure-SSH
  #
  my $is_openssh = 0;
  my @ssh_version = `ssh -V 2>&1`;
  foreach (@ssh_version) {
    if (m/OpenSSH/) {
      $is_openssh = 1;
      last;
    }
  }

  #
  # add a key to the ssh-agent
  #
  if ($is_openssh) {
    # for OpenSSH we explicitly say we want the Version-2 key,
    # otherwise we might default to a Version-1 key (useless for us)
    system("ssh-add ~/.ssh/id_dsa");
  } else {
    system("ssh-add"); # this really invokes ssh-add2 via a symlink
  }

  #
  # remember that we have to kill the temporary ssh-agent
  #
  if ($is_openssh) {
    $SSH_AGENT_KILL_CMD = "ssh-agent -k";
  } else {
    $SSH_AGENT_KILL_CMD = "kill $ENV{SSH2_AGENT_PID}";
  }
}


###########################################################################

# kill temporary ssh-agent on exit
END {
  if ($SSH_AGENT_KILL_CMD ne "") {
    `$SSH_AGENT_KILL_CMD`;
  }
}

###########################################################################

# Make perl happy...

1;
