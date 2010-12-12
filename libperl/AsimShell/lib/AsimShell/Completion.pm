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

#
# ATTENTION MAINTAINERS!
#
# See the header comments in Commands.pm for how to add commands
# to asim-shell.
#
# IF YOU ARE ADDING A NEW PACKAGE COMMAND that takes multiple package names
# as argument, or that takes "all" or "*" to denote all packages,
# make sure you update the is_multi_package_command() or
# package_command_takes_all_arg() routines in this file accordingly!!
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
  my $text  = shift;     # The text word being completed
  my $line  = shift;     # The entire line
  my $start = shift;     # The start index of $text in $line
  my $end   = shift;     # The end index of $text in $line

  my $prefix;
  my @list;

  $prefix = substr($line, 0, $start);

  # Strip leading spaces (now $start and $end are wrong!!!)

  $line =~ s/^\s*//;
  $prefix =~ s/^\s*//;

  $term->Attribs->{completion_append_character} = " ";

  if ($prefix eq "") {
    #
    # Handle first symbol of command
    #
    @list = grep /^$text/, @COMMANDS;

    return (max_common($text, @list), @list);

  } elsif ($text =~ /--[^=]*$/) {
    #
    # Handle command switches (needs to be early)
    #
    my $command = build_command($prefix);
    @list = grep /^$text/, @{$OPTIONS{$command}};

    return (max_common($text, @list), @list);

  } elsif ( $prefix =~ /^(\w+)\s+$/) {

    #
    # Handle second symbol of command (if first is a legal keyword)
    #

    if ( ! defined($COMPOUNDCOMMANDS{$1})) {
      # Not a comound command - default to filename completion
      return ();
    }

    my @subcommands = @{$COMPOUNDCOMMANDS{$1}};
    @list = grep /^$text/, @subcommands;
    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /^new\s+bundle\s+$/      ||
           $prefix =~ /^checkout\s+bundle\s+$/ ||
           $prefix =~ /^use\s+bundle\s+$/      ||
           $prefix =~ /^show\s+bundle\s+$/     ||
           $prefix =~ /^clone\s+bundle\s+$/    ||
           $prefix =~ /^pull\s+bundle\s+$/     ||
           $prefix =~ /^update\s+bundle\s+$/   ){
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

  } elsif ($prefix =~ /^checkout\s+package\s+$/ ||
           $prefix =~ /^use\s+package\s+$/      ||
           $prefix =~ /^show\s+repository\s+$/  ){
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

  } elsif ($prefix =~ /^add\s+public_package\s+$/) {
    #
    # Handle packages that can be checked out
    #
    my @packages = $default_repositoryDB->public_directory();
    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif (   ($prefix =~ /^(\w+)\s+package(\s+[a-zA-Z][\w-]+)*\s+$/ && is_multi_package_command( $1 ))
           || ($prefix =~ /^((show)|(verify))\s+configuration/                                        )
          ) {
    #
    # Handle commands that take one or more package names as arguments
    #
    my @packages = $default_packageDB->directory();

    # "all" is only valid as first argument
    if ($prefix =~ /^\s*(\w+)\s+package\s+$/ && package_command_takes_all_arg( $1 ) ) {
      push(@packages, "all");
    }

    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /^run\s+regression(\s+[a-zA-Z][\w-]+)*\s+$/) {
    #
    # Handle regression commands that take one or more package names as arguments
    #
    my @packages = $default_packageDB->directory();

    # "all" and "default" are only valid as first argument
    if ($prefix =~ /^\s*run\s+regression\s+$/ ) {
      push(@packages, "default", "all");
    }

    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /package\s+$/               ||
           $prefix =~ /^new\s+repository\s+$/     ||
           $prefix =~ /^create\s+repository\s+$/  ){

    #
    # Handle commands that take a single package name
    #
    my @packages = $default_packageDB->directory();

    # optionally add "all" 
    if ($prefix =~ /^\s*(\w+)\s+package\s+$/ && package_command_takes_all_arg( $1 ) ) {
      push(@packages, "all");
    }

    @list = grep /^$text/, @packages;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /model\s+$/  ||
           $prefix =~ /benchmark\s+$/ ||
           $prefix =~ /module\s+$/ ) {
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

  } elsif ($prefix =~ /lock\s+$/) {
    #
    # Handle lock names
    #
    my @locks = Asim::Lock->new()->directory();
    @list = grep /^$text/, @locks;

    return (max_common($text, @list), @list);

  } elsif ($prefix =~ /--user\s*$/ || $prefix =~ /--user=$/ ) {
    #
    # Handle user on --user switch
    #
    return $term->completion_matches($text,
                                     $term->Attribs->{'username_completion_function'});

  } elsif ($prefix =~ /--commitlog\s*$/ || $prefix =~ /--commitlog=$/ ) {
    #
    # Handle user on --commitlog switch - filename completion
    #
    return ();

  } elsif (0) {
    #
    # TBD: Figure out how to handle string options to other switches (following = or not)
    #

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


#
# return 1 if the given command is a package
# command that takes multiple packages as arguments
#
sub is_multi_package_command {
  my $cmd = shift;
  return ( $cmd =~ m/^((status)|(delete)|(show)|(update)|(commit)|(configure)|(build)|(make)|(clean)|(pull)|(push))$/ );
}

#
# return 1 if the package command takes "all" as an argument
#
sub package_command_takes_all_arg {
  my $cmd = shift;
  return ( $cmd =~ m/^((status)|(show)|(update)|(commit)|(configure)|(build)|(make)|(clean)|(run)|(pull)|(push))$/ );
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
