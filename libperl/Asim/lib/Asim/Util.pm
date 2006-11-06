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


package Asim::Util;
use warnings;
use strict;

our $DEBUG = ($ENV{ASIM_DEBUG} || 0) >= 3;
our $RESOLVER = "awb-resolver";

=head1 NAME

Asim::Util - Random Asim utility functions

=head1 SYNOPSIS

use Asim::Util;

TBD

=head1 DESCRIPTION

Utility functions....

=over 4

=cut

################################################################

=item Asim::Util::resolve($file)

Resolve file $FILE. 
Algorithm:

    Step 1: if its a real file just return it.

    Step 2: use awb-resolve to find it.

=cut

################################################################

sub resolve {
  my $file = shift;
  my $result;

  chomp ($result = `$RESOLVER $file 2>&1`);
  if ($?) {
    die "Error:Resolve of $file failed with result \n$result\n";
  }

  return $result;
}


################################################################

=item Asim::Util::parse_file_or_params($input)

Retrieve parameters from either a string of values or from a file. 

=cut

################################################################
sub parse_file_or_params {
  my($string) = shift;
  my(@param);

  $string = expand_tilda($string);
  if (! -d $string && open RDPARAM, "<$string") {
    while (<RDPARAM>) {
      if (!m/^#/ && m/\S+/) {
        chop;
        s/^\s*(\S+)\s*/$1/;
        push @param, $_;
      }
    }
    close RDPARAM;
  } else {
    @param = split ' ', $string;
  }
  
  return @param;
}

################################################################

=item Asim::Util::expand_regexp($input_array, $reg_exp_array)

Return every entry in @input_array that matches every entry in @reg_exp_array

=cut

################################################################
sub expand_regexp {
  my $input = shift;
  my @regexp = @_;
  
  my @all;
  foreach my $str (@regexp) {
    my @values;
    $str =~ s/\s*//g;
    if ($str =~ /^\/(.*)\/[i]*$/) {
      # We have a regular expression that has to be expanded. 
      $str =~ s/^\/(.*)$/$1/;
      if ($str =~ /(.*)\/i/) {
      # Case where we have some flags for regexp
	@values = grep (/$1/i, @{$input});
      }
      else {
        $str =~ s/(.*)\//$1/;
        @values = grep (/$str/, @{$input});
      }
      push (@all, @values);
    }
    else {
      push (@all, $str);
    }
  }

  # Make all params unique.
  @all = sort @all;
  my $prev = "";
  @all = grep ($_ ne $prev && (($prev) = $_), @all);
  return @all;
}

################################################################

=item Asim::Util::expand_shell($file)

Expand given input by evaluating ~ and environment variables. 

=cut

################################################################

sub expand_shell {
  my($file) = shift;

  $file = expand_tilda ($file);
  $file = expand_env ($file);

#  $file = `echo $file`;
  chomp ($file);
  return $file;
}

################################################################

=item Asim::Util::expand_env($file)

Expand environment variables to actual value.

=cut

################################################################

sub expand_env {
  my($file) = shift;

  my @vars;
  
  
  if ( @vars = $file =~ /\${*(\w+)/g ) {
    foreach my $var ( @vars ) {
      ( defined $ENV{$var} ) || die "Unable to translate environment variable $var.\n";
      $file =~ s/\$$var/$ENV{$var}/;
    }
  }
  
  return $file;
}

################################################################

=item Asim::Util::expand_tilda($file)

Expand '~' to actual path. 

=cut

################################################################

sub expand_tilda {
  my($file) = shift;
  my($user, $home);

  debug("expand_tilda: Expanding $file\n");

  $home = "";
  if ($file =~ m|^~/|) {
    if (defined $ENV{HOME}) {
      $home = $ENV{HOME};
    } elsif (defined $ENV{USER}) {
      $user = $ENV{USER};
    } elsif (defined $ENV{LOGNAME}) {
      $user = $ENV{LOGNAME};
    } else {
      die( "Who the hell are you?\n");
    }
    if ($home eq "") {
      $home = get_homedir( $user );
    }
    $file =~ s|^~|$home|;
  } elsif ($file =~ m|^~([^/]+)/|) {
    $user = $1;
    $home = get_homedir( $user );
    $file =~ s|^~([^/]+)|$home|;
  }

  debug("expand_tilda: Expanded to $file\n");
  return $file;
}

################################################################

=item Asim::Util::get_asimrc_val($group, $item, $def)

Read a value from asimrc file. 
Inputs are: asim object, group, item, and default value

=cut

################################################################
sub get_asimrc_val
{
  my $group = shift;
  my $item = shift;
  my $def = shift;
  my $val;

  $val = $Asim::rcfile->get($group, $item);
  
  # Is this value really defined, or are we just getting some white spaces....
  if ($val)
  {
    $val =~ s/\"//g;
    $val =~ s/^\s*(.*)\s*$/$1/;
  }
  
  if (!defined($val) || ($val eq ""))
  {
    $val = $def;
  }
  
  return $val;
}




################################################################

=item Asim::Util::get_homedir($file)

=cut

################################################################

sub get_homedir {
  my($user) = shift;
  my($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$dir,$shell,$expire);

  ($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$dir,$shell,$expire) =
    getpwnam( $user )  or die("Can't find user $user\n") ;

  if (!defined $name) {
    die("Can't find user $user\n");
  }
  return $dir;
}



################################################################

=item Asim::Util::issane()

Check if the environment is sane. Return 0 if no problems, otherwise
return explanation of problems.

=cut

################################################################

sub issane {
  #
  # Initialization checks
  #
 
  chomp (my $msg = `$RESOLVER .`);
  if ($? != 0) {
    die "Can't find $RESOLVER or $RESOLVER failed - check your PATH environemt variable\n$msg\n";
  }

  #
  # Check CVS version
  #
  my @cvsv = grep(/\(CVS\)\s+\d+\.\d+/, (`cvs -v`));
  $cvsv[0] =~ /(\d+\.\d+)/;
  if ($1 < 1.1) {
    die "Possible incompatible CVS version looking for >=1.1, found ($1)\n";
  }

}

issane();


###########################################################################




###########################################################################


################################################################
#
# Internal error utility function
#
################################################################

sub debug {
  my $message = shift;

  if ($DEBUG) {
    print "Util: $message";
  }

  return 1;
}

sub iwarn {
  my $message = shift;

  print "Util: Warning: - $message";

  return 1;
}

sub ierror {
  my $message = shift;

  print "Util: Error - $message";

  return 1;
}


1;


=back

=head1 BUGS

=head1 AUTHORS

Joel Emer, Artur Klauser

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut
