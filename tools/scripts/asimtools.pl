:
eval 'exec perl "$0" ${1+"$@"}'
       if 0;

#
# Copyright (C) 2002-2006 Intel Corporation
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


# *****************************************************************
# *                                                               *

# Authors: Pritpal Ahuja, Artur Klauser

use Net::FTP;  # FTP client interface
use XML::DOM;  # XML parser with DOM interface

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
sub lock_cmd {
  my (%args) = @_;
  my @result;

  #
  # set defaults
  #
  if (! defined $args{'info'}) {
    my $hostname = `/usr/bin/hostname`;
    chomp $hostname;
    $args{'info'} = "$ENV{USER}\@$hostname";
  }

  #
  # cook up the lock request URL
  #
  my $url = "http://asim.intel.com/home/asim/admin/lock/?"; 
  foreach my $arg (keys %args) {
    $url .= "&$arg=$args{$arg}";
  }

  my $cmd = "wget --proxy=off --quiet -O - \"'$url'\"";
  open LOCK, "$cmd |" 
    or return ($args{'name'} => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error opening secure connection to asim.intel.com"
               });
  my @lock_response = <LOCK>;
  close LOCK
    or return ($args{'name'} => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error closing secure connection to asim.intel.com"
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
  # lock manager response syntax:
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

##############################################################################
# acquire a repository lock
#
# Input parameters:
#   name:   <lock-name>     ... name of lock to acquire
#
# Results:
#   none
#   Execution exits when a problem is encountered.
##############################################################################
sub lock_repository {
  my $name = shift;
  my %result = lock_cmd (name => $name,
                         cmd  => "lock"
                        );
  if (! defined $result{$name} || ref $result{$name} ne HASH) {
    print "Fatal internal error in lock client\n";
    exit -1;
  }

  my %lock = %{$result{$name}};
  if ( $lock{'code'} == 2 ) {
    print "Sorry, a Commit lock is already set ($lock{msg})\n";
    print "Please try again some time later (wait at least 10 minutes)\n";
    print "You can check the lock status at\n";
    print "  http://asim.intel.com/home/asim/admin/lock/\n";
    exit -1;
  } elsif ($lock{'code'} == 1) {
    print "Error: lock request failed\n";
    print "$lock{msg}\n";
    cleanup_and_exit("Panic! Could not get lock $mylock: $!\n");
  }
}

##############################################################################
# release a repository lock
#
# Input parameters:
#   name:   <lock-name>     ... name of lock to acquire
#
# Results:
#   none
##############################################################################
sub unlock_repository {
  my $name = shift;
  my %result = lock_cmd (name => $name,
                         cmd  => "unlock"
                        );
  if (! defined $result{$name} || ref $result{$name} ne HASH) {
    print "Fatal internal error in lock client\n";
    exit -1;
  }

  my %lock = %{$result{$name}};
  if ( $lock{'code'} == 2 ) {
    print "Internal error! Could not unlock $name - not locked\n";
    print "$lock{msg}\n";
    print "You can check the lock status at\n";
    print "  http://asim.intel.com/home/asim/admin/lock/\n";
  } elsif ($lock{'code'} == 1) {
    print "Error: unlock request failed\n";
    print "$lock{msg}\n";
  }
}

##############################################################################
# low-level CSN function
#
# Input parameters are interpreted as a hash with following values:
#   name   => <csn-name>    ... the name of the lock to operate on
#   value  => <int>         ... new value of CSN
#   cmd    => get           ... get current serial number
#             set           ... set new serial number
#
# Return value is also a hash of hashes. The first level hash is indexed by
# lock name, pointing to an anonymous result hash for each lock.
#   <name> => { <result> }
# Each result hash has the following contents:
#   code   => <num>         ... 0 .. OK
#                               1 .. error in request
#                               2 .. operation error
#   name   => <csn-name>    ... same as in input
#   value  => <int>         ... new value of CSN
#   msg    => <string>      ... additional message to user passed back
#                               from the csn server
##############################################################################
sub csn_cmd {
  my (%args) = @_;
  my @result;

  #
  # cook up the csn request URL
  #
  my $url = "http://asim.intel.com/home/asim/admin/csn/?";
  foreach my $arg (keys %args) {
    $url .= "&$arg=$args{$arg}";
  }

  #
  #
  my $cmd = "wget --quiet -O - \"'$url'\"";
  open CSN, "$cmd |" 
    or return ($args{'name'} => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error opening secure connection to asim.intel.com"
               });
  my @csn_response = <CSN>;
  close CSN
    or return ($args{'name'} => {
                 code => 1,            # error
                 name => $args{'name'},
                 msg  => "error closing secure connection to asim.intel.com"
               });

  # debug code
  #print "CSN Response\n";
  #print "@{csn_response}";

  #
  # create the XML parser and pick out the csn response
  #
  my $parser = new XML::DOM::Parser( ErrorContext => 3 );
  my $doc;
  eval {
    $doc = $parser->parsestring ("@{csn_response}");
    # $doc = $parser->parsefile ($url);
  };
  if ($@) {
    # parse error occured
    chomp $@;
    push @result, ($args{'name'} => {
                     code => 1,            # error
                     name => $args{'name'},
                     msg  => "XML parse error in csn manager response\n$@\n"
                   });
    return @result;
  }

  #
  # csn manager response syntax:
  #   <csn name=STRING code=INT value=STRING>STRING</csn>
  #
  # note: we should get exaclty 1 node back
  #
  my @nodelist = $doc->getElementsByTagName ("csn");
  if ($#nodelist >= 0) {
    foreach my $node (@nodelist) {
      my $res = {};
      foreach my $attr (qw/name code value/) {
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
                     msg  => "no <csn> element found in csn manager response"
                   });
  }

  #$doc->printToFile ("out.xml");
  # cleanup parser
  $doc->dispose;

  return @result;
}

##############################################################################
# get the current CSN
#
# Input parameters:
#   name:   <csn-name>     ... name of CSN
#
# Results:
#   value:  <int>          ... current CSN value
#   Execution exits when a problem is encountered.
##############################################################################
sub get_csn {
  my $name = shift;
  my %result = csn_cmd (name => $name,
                        cmd  => "get"
                        );
  if (! defined $result{$name} || ref $result{$name} != HASH) {
    print "Fatal internal error in csn client\n";
    exit -1;
  }

  my %csn = %{$result{$name}};
  if ($csn{'code'} != 0) {
    print "Error: csn get request failed\n";
    print "$csn{msg}\n";
    print "You can check the csn status at\n";
    print "  http://asim.intel.com/home/asim/admin/csn/\n";
    cleanup_and_exit("Panic! Could not get csn: $!\n");
  } else {
    return $csn{'value'};
  }
}

##############################################################################
# set the current CSN
#
# Input parameters:
#   name:   <csn-name>     ... name of CSN
#   value:  <int>          ... new value for CSN
#
# Results:
#   value:  <int>          ... new CSN value
#   Execution exits when a problem is encountered.
##############################################################################
sub set_csn {
  my $name = shift;
  my $value = shift;
  my %result = csn_cmd (name  => $name,
                        value => $value,
                        cmd   => "set"
                        );
  if (! defined $result{$name} || ref $result{$name} != HASH) {
    print "Fatal internal error in csn client\n";
    exit -1;
  }

  my %csn = %{$result{$name}};
  if ($csn{'code'} != 0) {
    print "Error: csn set request failed\n";
    print "$csn{msg}\n";
    print "You can check the csn status at\n";
    print "  http://asim.intel.com/home/asim/admin/csn/\n";
    cleanup_and_exit("Panic! Could not set csn: $!\n");
  } else {
    return $csn{'value'};
  }
}

##############################################################################
# ftp_put
#
# Input parameters:
#   local_filename
#   ftp_server
#   ftp_user
#   ftp_password
#   ftp_dir
#   remote_filename
#
# Results:
#   Execution exits when a problem is encountered.
##############################################################################
sub ftp_put {
  my $local_filename = shift;
  my $ftp_server = shift;
  my $ftp_user = shift;
  my $ftp_password = shift;
  my $ftp_dir = shift;
  my $remote_filename = shift;

  #
  # set defaults
  #
  if (($ftp_user eq "anonymous" || $ftp_user eq "ftp") &&
      (! defined $ftp_password || $ftp_password eq ''))
  {
    my $hostname = `/usr/bin/hostname`;
    chomp $hostname;
    $ftp_password = "$ENV{USER}\@$hostname";
  }

  my $ftp = Net::FTP->new($ftp_server, Debug => 0);
  die "ftp open failed"  if (! defined $ftp);
  $ftp->login($ftp_user, $ftp_password) or die "ftp login failed\n$!";
  $ftp->cwd($ftp_dir) or die "ftp cwd failed\n$!";
  $ftp->binary() or die "ftp type binary failed\n$!";
  # we initially put things in a temporary filename (dash prefix)
  # to make sure that scripts on the other end can ignore files that
  # are not completely sent yet; after the put completes, we rename the
  # file remotely to the correct name
  $ftp->put($local_filename, "-$remote_filename") or die "ftp put failed\n$!";
  $ftp->rename("-$remote_filename","$remote_filename") or die "ftp rename failed\n$!";
  $ftp->quit;
}

##############################################################################
# initialize tools access
#
# Input parameters:
#   none
#
# Results:
#   none
#   Lots of global variables are set up.
##############################################################################

sub init_tools
{
  #
  # General Variables: You might need to change this when porting to
  # a different environment
  #
  $HOSTNAME = "/usr/bin/hostname";
  $TAR = "tar";
  $GZIP = "gzip -9";
  $DATE = "/usr/bin/date";

  $HOST = `$HOSTNAME`;
  chop $HOST;
  $USER = $ENV{'USER'};
  
  #
  # Check for CVSROOT. If CVSROOT is of the form <HOST>:<PATH>, then
  # break it up into $CVSHOST and $CVSROOT, and do a remote commit
  #
  $CVSROOT = $ENV{'CVSROOT'};
  if ( $CVSROOT =~ /ext:(.*)@(.*):(.*)/ || $CVSROOT =~ /(.*)@(.*):(.*)/ )
  {
    $CVSROOT = $3;
  }
  elsif ( $CVSROOT =~ /(.*):(.*)/ ) 
  {
    $CVSROOT = $2;
  }
}

sub cleanup_and_exit
{
 my($msg) = $_[0];

 unlink $tmpfile 	if ( $tmpfile );
 unlink $tarfile 	if ( $tarfile );

 unlock_repository('asim-pm');

 unlink $oipcfile		if ( $oipcfile );
 unlink "$TMPDIR/$oipcfile"	if ( $oipcfile );
 unlink $local_tarfile	if ( $local_tarfile );
 unlink "$TMPDIR/commitsn"	if ( -e "$TMPDIR/commitsn" );
 
 if ( defined $modchanges and $modchanges ) {
  print "Reverting 'changes' file to previous state...\n";
  unlink "$ASIMDIR/changes";
  system "mv $TMPDIR/changes.bak.$$ $ASIMDIR/changes";
 }

 if ( defined $new_goldstats and $new_goldstats ) {
  print "Reverting the gold stats directory ($GOLDSTATS) to previous state...\n";
  chdir $GOLDSTATS;
  system "rm -f *.stats.gz";
  system "cvs update -Pd .";
 }

 if ( defined $modipcfile and $modipcfile ) {
  print "Reverting the ${oipcfile} to previous state...\n";
  system "rm -f ${oipcfile}";
  # remote cvs can't handle absolute path to oipcfile
  system "cd $ASIMDIR/admin; cvs update ipchist";
 }

 print "$msg";
 exit -1;
}


#
# search a repository file for all branch tags
#
sub find_cvs_branchtags
{
  my $filename = shift;
  my @branches;

  open(CVSLOG,"cvs log $filename |")
    or die "can't run cvs log $filename\n$!";
  while (<CVSLOG>) {
    if (m/^symbolic names:$/) {
      $in_symnames = 1;
    } elsif ($in_symnames) {
      if (m/^\S/) {
        $in_symnames = 0;
      } elsif (m/\s+(\S+):.*\.0\./) {
        # (only) branch tags have a ".0." component in CVS revision number
        push @branches, $1;
      }
    }
  }
  close CVSLOG;
  return @branches;
}

## Need to return a "1"
1;
