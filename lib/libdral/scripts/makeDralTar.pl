:
eval 'exec perl -w "$0" ${1+"$@"}'
       if 0;

#############################################################################
# Copyright (C) 2004-2006 Intel Corporation
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
# Brief:  Create default tar
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
    $lineLength = length( $inLine );
    chomp $inLine;
    # some characters need quoting
    #
    # replace '\' with '\\'
    $inLine =~ s/[\\]/\\\\/g;
    # replace '"' with '\"'
    $inLine =~ s/["]/\\"/g;
    # remove DOS end-of-line crap @!#$%
    # $inLine =~ s/\x0d//;
    $inLine =~ s/[\0]/\\000/g;

    $outLine = "$inLine\\n";
    $oldSize = $outSize;
    $outSize += $lineLength;
    push @out, "\tmemcpy($bufName + $oldSize, \"$outLine\", $lineLength);\n";
  }
  close( IN )
    or die "Error closing $inFileName\n$!";

  return $outSize, @out;
}


#
# Create a source file defining the default (built-in) tar. This
# basically creates a very long string containing the whole TCL code of
# the default tar for this model.
#
sub CreateDefaultWorkbench {
  my @inFiles = shift;

  #my $outFileName = "prova_tar.cpp";
  my $strName = "tar";

  # Open output file for writing and output the header info.
  #open( OUT, ">$outFileName" )
  #  or die "Can't open $outFileName for write\n$!";

  my @tar;
  my $tarSize = 0;

  foreach $file (@inFiles) {
    my ($size, @strings) = BuildCStringFromFile( $file, $strName );
    push @tar, @strings;
    $tarSize += $size;
  }

  # generate header
  print "/*************************************************\n";
  print " * This source file is automatically generated\n";
  print " * to define the default TAR file\n";
  print " *************************************************/\n";
  print "\n";
  print "#include <string.h>\n";
  print "#include \"asim/dralServer.h\"\n";
  print "\n";
  print "char * DRAL_SERVER_CLASS::GetTar (UINT32 * size) {\n";
  print "\tconst unsigned int tarSize = $tarSize;\n";
  print "\t*size = tarSize;\n";
  print "\tchar *$strName = new char[tarSize];\n";
  print "\t*$strName = 0;\n";

  # ... contents ...
  print @tar;

  # ... trailer
  print "\treturn($strName);\n";
  print "}\n";

  #close( OUT )
  #  or die "Error closing $outFileName\n$!";
}

#----------------------------------------------------------------------------
# main stuff
#----------------------------------------------------------------------------

@tarFiles = @ARGV;

printf STDERR "creating default tar for @tarFiles\n";
CreateDefaultWorkbench(@tarFiles);

# vim:set filetype=perl:
