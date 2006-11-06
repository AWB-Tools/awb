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

package AsimShell;
use warnings;
use strict;

use Asim;

use File::Spec;
use File::Glob;
use Term::ReadLine;

#
# Command globals
#
our $term;
our @COMMANDS;
our %COMPOUNDCOMMANDS;
our %OPTIONS;

#
# Default data values
#
our $default_repositoryDB;
our $default_packageDB;
our $default_moduleDB;

our $default_package;
our $default_lock;
our $default_model;
our $default_benchmark;
our $default_module;

#
# Utility function to help parsing
#
sub attempted_completion {
  my ($text, $line, $start, $end) = @_;
  my $prefix;
  my @list;

  # Strip leading spaces
  $line =~ s/^\s*//;

  $prefix = substr($line, 0, $start);

  $term->Attribs->{completion_append_character} = " ";

  if ($start == 0) {
    #
    # Handle first symbol of command
    #
    @list = grep /^$text/, @COMMANDS;
    return (max_common($text, @list), @list);

  } elsif ( $prefix =~ /^(\w+)\s*$/) {

    #
    # Handle second symbol of command (if first is a legal keyword)
    #
    if (defined($COMPOUNDCOMMANDS{$1})) {
      my @subcommands = @{$COMPOUNDCOMMANDS{$1}};
      @list = grep /^$text/, @subcommands;
    } else {
      @list = ();
    }
    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /checkout bundle\s$/ ||
           $prefix =~ /use bundle\s$/ ||
           $prefix =~ /show bundle\s$/) {
    #
    # Handle bundles that can be checked out
    #
      my @bundles;
      if (grep (/\//, $text)) {
	  my ($bundlename, $bundleid) = (split("/", $text), undef);	  
	  @bundles = $default_repositoryDB->bundle_ids($bundlename);
	  if (defined $bundleid) {
	      @list = grep /^$bundleid/, @bundles;
	  }
	  else {
	      @list = @bundles;
	  }
  	  foreach my $element (@list) {
  	      $element = $bundlename . "/" . $element;
  	  }
      }
      else {
	  @bundles = $default_repositoryDB->bundle_directory();
	  @list = grep /^$text/, @bundles;
	  foreach my $element (@list) {
  	      $element = $element . "/";
  	  }
	  $term->Attribs->{completion_append_character} = "";	  
      }
      
      return (max_common($text, @list), @list);

  } elsif ($prefix =~ /checkout package\s$/ ||
           $prefix =~ /use package\s$/      ||
           $prefix =~ /status package\s$/   ) {
    #
    # Handle packages that can be checked out
    #
    my @packages = $default_repositoryDB->directory();
    
    if (!grep (/\//, $text)) {
	my %p;
        # Determine the distinct package names (independent of version)
	foreach my $element (@packages) {
	    my $t = $element;
	    $t =~ s/\/.*//;
	    $p{$t} = 1;	
	}
	@packages = sort(keys %p);
	foreach my $element (@packages) {
	    $element = $element . "/";
	}
	$term->Attribs->{completion_append_character} = "";	
    }
    
    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /add public_package\s$/) {
    #
    # Handle packages that can be checked out
    #
    my @packages = $default_repositoryDB->public_directory();
    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /package\s$/) {
    #
    # Handle package names
    #
    my @packages = $default_packageDB->directory();
    if ($prefix =~ /update package/) {
      push(@packages, "all");
    }
    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /model\s$/  ||
           $prefix =~ /benchmark\s$/ ||
           $prefix =~ /module\s$/ ) {
    #
    # Handle Asim search path filename completion
    #
    foreach my $f (Asim::glob($text . "*")) {
      if (-d Asim::resolve($f) && !($f =~ /\/$/)) {
        # Its a directory so add a /
        push(@list, "$f/");
        # Assume the match does't end here...
        $term->Attribs->{completion_append_character} = "";
      } else {
        push(@list, "$f");
      }
    }

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /lock\s$/) {
    #
    # Handle lock names
    #
    my @locks = Asim::Lock->new()->directory();
    @list = grep /^$text/, @locks;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /--user\s*$/ || $prefix =~ /--user=[^ ]*$/ ) {
    #
    # Handle user on --user switch
    #
    return $term->completion_matches($text,
                                     $term->Attribs->{'username_completion_function'});

  } elsif ($text =~ /--[^=]*$/) {
    #
    # Handle command switches
    #
    my $command = build_command($prefix);
    @list = grep /^$text/, @{$OPTIONS{$command}};

    return (max_common($text, @list), @list);

  } elsif (0) {
    #
    # Another  model for other things to do...
    #
    $term->Attribs->{completion_word} = [qw(help test)];
    return $term->completion_matches($text, 
                                     $term->Attribs->{'list_completion_function'});

  } else {
    #
    # Default to filename completion
    #
    return ();
  }
}

#
# Build up a possibly compound command out of a line
#
sub build_command {
  my @line = shellwords shift;

  my $command = shift @line;

  #
  # Check if next argument combines to form a compound command
  #
  if ( defined($line[0]) && defined(&{$command . "_" . $line[0]})) {
    $command = $command . "_" . shift @line;
  }

  return $command;
}


###############################################################
#
# Find the maximum common extension of $start in @list, i.e.,
# if all the members of @list (which already start with the
# string $start) continue with the same letter (or letters)
# return the string starting with $start and extended with 
# that set of common letters.
#
# This is just a trampoline to Asim::max_common...
#
###############################################################

sub max_common {
  return Asim::max_common(@_);
 }

# make perl happy
1;
