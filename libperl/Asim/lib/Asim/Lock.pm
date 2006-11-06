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


# Authors: Pritpal Ahuja, Artur Klauser, Joel Emer


package Asim::Lock;
use warnings;
use strict;

use Asim::Base;


our @ISA = qw(Asim::Base);

our %a =  ( lockname =>             [ "lockname",
                                      "SCALAR" ], 
      );


our $nolocks = 0;            #allow global disabling of lock access
our $LOCKURL = "http://asim.intel.com/home/asim/admin/lock";

=head1 NAME

Asim::Lock - Library for manipulating ASIM locks

=head1 SYNOPSIS

use Asim::Lock;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $lock = Asim::Lock-E<gt>new([$lockname])

Create a new lock object with the given name for the lock.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = _initialize();

  bless	$self, $class;

  #
  # Handle input if given
  #
  if (@_) {
    $self->{lockname} = $_[0];
  }

  return $self;
}


sub _initialize {

  my $self = {   accessors => \%a,
		 lockname => "" ,
		 description => "",
             };
     
  return $self;
}

################################################################

=item $lock = Asim::Lock-E<gt>accessors()

Return list of accessor methods.

=cut

################################################################

sub accessors {
  return qw(lockname);
}

sub lockname {
  my $self = shift;
  my $name = shift;

  if (defined($name)) {
    $self->{lockname} = $name;
  }

  if (! defined($self->{lockname})) {
    $self->{lockname} = $self->name();
  }

  return $self->{lockname};
      
}

################################################################

=item $lock = Asim::Lock-E<gt>create_lock()

Create a new lock. Name of lock is name user provided to new.

=cut

################################################################


sub create_lock {
  my $self = shift;
  my $name = $self->lockname();

  print "Creating lock: $name\n";

  my %result = lock_cmd (name => $name,
                         cmd  => "create"
                        );
  if (! defined $result{$name} || ref $result{$name} ne "HASH") {
    print "Fatal internal error in lock client\n";
    return 0;
  }

  my %lock = %{$result{$name}};

  if ( $lock{'code'} == 2 ) {
    print "Internal error! Could not create lock $name\n";
    print "$lock{msg}\n";
    print "You can check the lock status at\n";
    print "  $LOCKURL\n"; 
    $self->{lockstatus} = $lock{msg};
    return 0;
  } elsif ($lock{'code'} == 1) {
    print "Error: create lock request failed\n";
    print "$lock{msg}\n";
    $self->{lockstatus} = $lock{msg};
    return 0;
  }

  $self->{lockstatus} = $lock{msg};
  return 1;
}

################################################################

=item $lock = Asim::Lock-E<gt>delete_lock()

Delete the lock. Name of lock is name user provided to new.

=cut

################################################################

sub delete_lock {
  my $self = shift;
  my $name = $self->lockname();

  print "Deleting lock: $name\n";

  my %result = lock_cmd (name => $name,
                         cmd  => "delete"
                        );
  if (! defined $result{$name} || ref $result{$name} ne "HASH") {
    print "Fatal internal error in lock client\n";
    return 0;
  }

  my %lock = %{$result{$name}};

  if ( $lock{'code'} == 2 ) {
    print "Internal error! Could not delete lock $name\n";
    print "$lock{msg}\n";
    print "You can check the lock status at\n";
    print "  $LOCKURL\n"; 
    $self->{lockstatus} = $lock{msg};
    return 0;
  } elsif ($lock{'code'} == 1) {
    print "Error: delete lock request failed\n";
    print "$lock{msg}\n";
    $self->{lockstatus} = $lock{msg};
    return 0;
  }

  $self->{lockstatus} = $lock{msg};
  return 1;
}


################################################################

=item $lock = Asim::Lock-E<gt>acquire_lock()

Acquire the lock.

=cut

################################################################

sub acquire_lock {
  my $self = shift;
  my $name = $self->lockname();

  print "Locking repository: $name\n";

  if ($nolocks) {
    $self->{lockstatus} = "Locks disabled: Success guaranteed";
    return 1;
  }

  my %result = lock_cmd (name => $name,
                         cmd  => "lock"
                        );
  if (! defined $result{$name} || ref $result{$name} ne "HASH") {
    print "Fatal internal error in lock client\n";
    return 0;
  }

  my %lock = %{$result{$name}};

  if ( $lock{'code'} == 2 ) {
    print "Sorry, a Commit lock is already set ($lock{msg})\n";
    print "Please try again some time later (wait at least 10 minutes)\n";
    print "You can check the lock status at\n";
    print "  $LOCKURL\n"; 
    $self->{lockstatus} = $lock{msg};
    return 0;
  } elsif ($lock{'code'} == 1) {
    print "Error: lock request failed\n";
    print "$lock{msg}\n";
    $self->{lockstatus} = $lock{msg};
    return 0;
  }

  # TODO: Add a handler to unlock in event of abort...

  $self->{lockstatus} = $lock{msg};
  return 1;
}


################################################################

=item $lock = Asim::Lock-E<gt>release_lock()

Release the lock.

=cut

################################################################

sub release_lock {
  my $self = shift;
  my $name = $self->lockname();

  print "Unlocking repository: $name\n";

  if ($nolocks) {
    $self->{lockstatus} = "Locks disabled: Success guaranteed";
    return 1;
  }

  my %result = lock_cmd (name => $name,
                         cmd  => "unlock"
                        );
  if (! defined $result{$name} || ref $result{$name} ne "HASH") {
    print "Fatal internal error in lock client\n";
    return 0;
  }

  my %lock = %{$result{$name}};

  if ( $lock{'code'} == 2 ) {
    print "Internal error! Could not unlock $name - not locked\n";
    print "$lock{msg}\n";
    print "You can check the lock status at\n";
    print "  $LOCKURL\n"; 
    $self->{lockstatus} = $lock{msg};
    return 0;
  } elsif ($lock{'code'} == 1) {
    print "Error: unlock request failed\n";
    print "$lock{msg}\n";
    $self->{lockstatus} = $lock{msg};
    return 0;
  }

  # TODO: Remove handler that did unlock in event of abort...

  $self->{lockstatus} = $lock{msg};
  return 1;
}



################################################################

=item $lock = Asim::Lock-E<gt>status_lock()

Obtain status of lock.

=cut

################################################################

sub status_lock {
  my $self = shift;
  my $name = $self->lockname();


  if ($nolocks) {
    $self->{lockstatus} = "Locks disabled: Success guaranteed";
    return 1;
  }

  my %result = lock_cmd (name => $name,
                         cmd  => "status"
                        );
  if (! defined $result{$name} || ref $result{$name} ne "HASH") {
    print "Fatal internal error in lock client\n";
    return 0;
  }

  my %lock = %{$result{$name}};

  if ( $lock{'code'} == 2 ) {
    print "Internal error! Could not get status of lock $name\n";
    print "$lock{msg}\n";
    print "You can check the lock status at\n";
    print "  $LOCKURL\n"; 
    $self->{lockstatus} = $lock{msg};
    return 0;
  } elsif ($lock{'code'} == 1) {
    print "Error: lock status request failed\n";
    print "$lock{msg}\n";
    $self->{lockstatus} = $lock{msg};
    return 0;
  }

  $self->{lockstatus} = $lock{msg};

  return $lock{msg};
}

################################################################

=item $lock = Asim::Lock-E<gt>dump()

Dump information on lock.

=cut

################################################################

sub dump {
  my $self = shift;
  my $status = $self->status_lock();

  $self->Asim::Base::dump();
  print "Status: $status\n";
}


################################################################

=back

The following are global functions:

=over 4

=cut

################################################################


=item $lock = Asim::Lock::directory()

Return a list of the names of the locks available. Note, this
functions caches the lock list.

=cut

################################################################

our @LOCKS = ();


sub directory {
  if ($#LOCKS > 0) {
    return @LOCKS;
  }
 
  my %result = lock_cmd ( cmd  => "status" );

  foreach my $name (sort keys %result) {

    if (ref $result{$name} ne "HASH") {
      print "Fatal internal error in lock client\n";
      return 0;
    }

    my %lock = %{$result{$name}};

    if ( $lock{'code'} == 2 ) {
      print "Internal error! Could not get status of lock $name\n";
      print "$lock{msg}\n";
      print "You can check the lock status at\n";
      print "  $LOCKURL\n"; 
      return 0;
    } elsif ($lock{'code'} == 1) {
      print "Error: lock status request failed\n";
      print "$lock{msg}\n";
      return 0;
    }

    push(@LOCKS, $name);
  }

  return @LOCKS;

#  my $result = lock_cmd ( cmd  => "status" );

#  if (defined $result) {
#    return (sort keys %{$result});
#  } else {
#    return ();
#  }
}
################################################################


=item $lock = Asim::Lock::rehash()

Rehash the directory of locks.

=cut

################################################################

sub rehash {
  @LOCKS = ();
}

################################################################

=item $lock = Asim::Lock::enable_locks()

Global function to enable use of remote locks.

=cut

################################################################


sub enable_locks {
  $nolocks = 0;
  return 1;
}

################################################################

=item $lock = Asim::Lock::disable_locks()

Global function to diable use of remote locks.

=cut

################################################################

sub disable_locks {
  $nolocks = 1;
  return 1;
}


=back


=head1 BUGS

Method names need not end in '_lock'.

Utility functions should be separated out into a separate module.

Asim::Lock::directory function should be put in a Asim::Lock::DB package.

Code should not be sprinkled with hard-code node name.

=head1 AUTHORS

Joel Emer and Artur Klauser.

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

# Faked locking function not using XML parser
# (just plain regexp matching and hoping that lock manager output
# lines are very well behaved - hey, we control them :)
sub lock_cmd {
  my (%args) = @_;
  my @result;

  my $attr_str;
  #
  # Set defaults
  #
  if (! defined $args{'info'}) {
    #
    # Get hostname
    #
    # TODO: Get hostname without command
    my $hostname = `hostname`;
    if (! defined($hostname) || ! $hostname) {
      ierror("Could not ascertain hostname");
      return 0;
    }
    chomp $hostname;

    #
    # Construct username
    #
    $args{'info'} = "$ENV{USER}\@$hostname";
  }

  #
  # cook up the lock request URL
  #
  my $url = "$LOCKURL/lock.php?";
  foreach my $arg (keys %args) {
    $url .= "&$arg=$args{$arg}";
  }

  # printf "URL: %s\n", $url;
  #
  #
  my $cmd = "wget --proxy=off --quiet -O - \"$url\""; 
  #printf "CMD: %s\n", $cmd;

  my $hashname = $args{'name'} || "NONE";

  open LOCK, "$cmd |" 
    or return ($hashname => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error opening secure connection to asim"
               });
  my @lock_response = <LOCK>;
  close LOCK
    or return ($hashname => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error closing secure connection to asim"
               });

  # debug code
  # print "Lock Response\n";
  # print "@{lock_response}";

  #
  # lock manager response syntax (per lock):
  #   <lock name=STRING code=INT status=STRING>STRING</lock>
  #
  my @nodelist;
  foreach (@lock_response) {
    if (m|(<lock .*</lock>)|) {
      push @nodelist, $1;
    }
  }

#print @nodelist;

  if ($#nodelist >= 0) {
    foreach my $node (@nodelist) {
#print "$node\n";
      my $res = {};
      foreach my $attr (qw/name code status/) {
        if ($node =~ m|<lock .*$attr='([^']*)'[^>]*>|) {
          $attr_str = $1;
        } else {
          $attr_str = "";
        }
#print "$attr = $attr_str\n";
        $$res{$attr} = $attr_str  if ($attr_str ne "");
      }
      if ($node =~ m|<lock .*>(.+)</lock>|) {
        $$res{'msg'} = $1;
#print "msg = $1\n";
      }
      my $attr = "name";
      if ($node =~ m|<lock .*$attr='([^']*)'[^>]*>|) {
        $attr_str = $1;
      }
      push @result, ($attr_str => $res);
    }
  } else {
    push @result, ($hashname => {
                     code => 1,            # error
                     name => $args{'name'},
                     msg  => "no <lock> element found in lock manager response"
                   });
  }

  return @result;
}

1;

__END__

##############################################################################
# low-level locking function
#
# Input parameters are interpreted as a hash with following values:
#   name => <lock-name>     ... the name of the lock to operate on
#   info => <string>        ... additional info that is recorded on the lock
#                               defaults to <user>@<host>
#   cmd  => status          ... report lock status
#           lock            ... try to acquire the lock
#           unlock          ... try to release the lock
#           create          ... create a new lock
#           delete          ... delete a lock (must be unlocked)
#
# Return value is also a hash of hashes. The first level hash is indexed by
# lock name, pointing to an anonymous result hash for each lock.
#   <name> => { <result> }
# Each result hash has the following contents:
#   code   => <num>         ... 0 .. OK
#                               1 .. error in request
#                               2 .. acquire/release failed
#   name   => <lock-name>   ... same as in input
#   status => <string>      ... return value of status command
#                               values: locked | unlocked
#   msg    => <string>      ... additional message to user passed back
#                               from the lock server
##############################################################################

use XML::DOM;     # XML parser with DOM interface


sub lock_cmd {
  my (%args) = @_;
  my @result;

  my $attr_str;
  #
  # Set defaults
  #
  if (! defined $args{'info'}) {
    #
    # Get hostname
    #
    # TODO: Get hostname without command
    my $hostname = `hostname`;
    if (! defined($hostname) || ! $hostname) {
      ierror("Could not ascertain hostname");
      return 0;
    }
    chomp $hostname;

    #
    # Construct username
    #
    $args{'info'} = "$ENV{USER}\@$hostname";
  }

  #
  # cook up the lock request URL
  #
  my $url = "$LOCKURL/lock.php?";
  foreach my $arg (keys %args) {
    $url .= "&$arg=$args{$arg}";
  }

  #
  #
  my $cmd = "wget --proxy=off --quiet -O - \"$url\""; 
  open LOCK, "$cmd |" 
    or return ($args{'name'} => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error opening secure connection to asim"
               });
  my @lock_response = <LOCK>;
  close LOCK
    or return ($args{'name'} => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error closing secure connection to asim"
               });

  # debug code
  #print "Lock Response\n";
  #print "@{lock_response}";

  #
  # create the XML parser and pick out the lock response
  #
  my $parser = new XML::DOM::Parser( ErrorContext => 3 );
  my $doc;
  eval {
    $doc = $parser->parsestring ("@{lock_response}");
    # $doc = $parser->parsefile ($url);
  };
  if ($@) {
    # parse error occured
    chomp $@;
    push @result, ($args{'name'} => {
                     code => 1,            # error
                     name => $args{'name'},
                     msg  => "XML parse error in lock manager response\n$@\n"
                   });
    return @result;
  }

  #
  # lock manager response syntax (per lock):
  #   <lock name=STRING code=INT status=STRING>STRING</lock>
  #
  my @nodelist = $doc->getElementsByTagName ("lock");
  if ($#nodelist >= 0) {
    foreach my $node (@nodelist) {
      my $res = {};
      foreach my $attr (qw/name code status/) {
        $attr_str = $node->getAttribute ($attr);
        $$res{$attr} = $attr_str  if ($attr_str ne "");
      }
      if ($node->hasChildNodes) {
        $$res{'msg'} = $node->getFirstChild->toString;
      }
      push @result, ($node->getAttribute ('name') => $res);
    }
  } else {
    push @result, ($args{'name'} => {
                     code => 1,            # error
                     name => $args{'name'},
                     msg  => "no <lock> element found in lock manager response"
                   });
  }

  #$doc->printToFile ("out.xml");
  # cleanup parser
  $doc->dispose;

  return @result;
}


1;
