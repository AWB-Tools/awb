#!/usr/bin/env perl
# -*- perl -*-

#############################################################################
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
# Recipient is granted a non-sublicensable copyright license under
# Intel copyrights to copy and distribute this code internally only.
# This code is provided "AS IS" with no support and with no
# warranties of any kind, including warranties of MERCHANTABILITY,
# FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY
# INFRINGEMENT. By making any use of this code, Recipient agrees that
# no other licenses to any Intel patents, trade secrets, copyrights
# or other intellectual property rights are granted herein, and no
# other licenses shall arise by estoppel, implication or by operation
# of law. Recipient accepts all risks of use.
#############################################################################
# 
# Author: Artur Klauser
# Brief:  Create default workbench compiled into PM
# 

#
# Convert input file inFileName into a sequence of C code that generates
# the contents of the file as string in char* buffer bufName. Space to
# hold the string in the buffer is dynamically allocated.
# The characters '\' and '"' are quoted to pass through correctly.
#
sub BuildCStringFromFile {
  my $inFileName = shift;
  my $bufName = shift;

  my $inLine;
  my $outLine;
  my @out;
  my $outSize = 0;

  open( IN, "<$inFileName" )
    or die "Can't open $inFileName for read\n$!";

  while ($inLine = <IN>) {
    chomp $inLine;
    # some characters need quoting
    #
    # replace '\' with '\\'
    $inLine =~ s/[\\]/\\\\/g;
    # replace '"' with '\"'
    $inLine =~ s/["]/\\"/g;
    # remove DOS end-of-line crap @!#$%
    $inLine =~ s/\x0d//;

    $outLine = "$inLine\\n";
    $outSize += length( $outLine );
    push @out, "\tstrcat($bufName, \"$outLine\");\n";
  }
  close( IN )
    or die "Error closing $inFileName\n$!";

  return $outSize, @out;
}


#
# Create a source file defining the default (built-in) workbench. This
# basically creates a very long string containing the whole TCL code of
# the default workbench for this model.
#
sub CreateDefaultWorkbench {
  my @inFiles = shift;

  my $outFileName = "simpleWb.cpp";
  my $strName = "wb";

  # Open output file for writing and output the header info.
  open( OUT, ">$outFileName" )
    or die "Can't open $outFileName for write\n$!";

  my @workbench;
  my $workbenchSize = 0;

  foreach $file (@inFiles) {
    my ($size, @strings) = BuildCStringFromFile( $file, $strName );
    push @workbench, @strings;
    $workbenchSize += $size;
  }

  # generate header
  print OUT "/*************************************************\n";
  print OUT " * This source file is automatically generated\n";
  print OUT " * to define the default TCL workbench\n";
  print OUT " *************************************************/\n";
  print OUT "\n";
  print OUT "#include <string.h>\n";
  print OUT "#include \"asim/mesg.h\"\n";
  print OUT "#include \"simpleWb.h\"\n";
  print OUT "\n";
  print OUT "char * AWB_WbInit (void) {\n";
  print OUT "\tconst unsigned int wbSize = $workbenchSize;\n";
  print OUT "\tchar *$strName = new char[wbSize + 1];\n";
  print OUT "\t*$strName = 0;\n";

  # ... contents ...
  print OUT @workbench;

  # ... trailer
  print OUT "\tif (strlen($strName) >= wbSize) { VERIFYX(false); }\n";
  print OUT "\treturn($strName);\n";
  print OUT "}\n";

  close( OUT )
    or die "Error closing $outFileName\n$!";
}

#----------------------------------------------------------------------------
# main stuff
#----------------------------------------------------------------------------

@workbenchFiles = @ARGV;

printf "creating workbench for @workbenchFiles\n";
CreateDefaultWorkbench(@workbenchFiles);

# vim:set filetype=perl:
