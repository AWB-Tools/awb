:
eval 'exec perl -w "$0" ${1+"$@"}'
       if 0;

## geChecksum.pl
##
## Author: Oscar Rosell
##
## Generates the function DumpChecksums(STATE_OUT).
## Takes a directory name as a parameter and recursively
## calculates the MD5 for each .cpp and .h file.
## The output is sent to STDOUT.
##
## Copyright (C) 2004-2006 Intel Corporation
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
##



my $dir = shift;

my @source_files = `find $dir -name "*.cpp"`;
my @header_files = `find $dir -name "*.h"`;

my %file_to_hash;

foreach my $file (@source_files)
{
    $md5_output = `md5sum $file`;
    $md5_output =~/^(\S+)\s/;
    chomp($file);
    $file_to_hash{$file} = $1;
}

foreach my $file (@header_files)
{
    $md5_output = `md5sum $file`;
    $md5_output =~/^(\S+)\s/;
    chomp($file);
    $file_to_hash{$file} = $1;
}

printf("#include \"asim\/stateout.h\"\n");
printf("\n");
printf("void DumpChecksums(STATE_OUT state)\n");
printf("\{\n");

my $var_counter = 0;
foreach my $key (keys(%file_to_hash))
{
    $value = $file_to_hash{$key};
    $key =~/\/([^\/]*)$/;
    my $file = $1;
    my $type = "string";
    my $name = "Checksum_$file";
    my $desc = $key;

    printf("state->AddScalar(\"$type\",\"$name\",\"$desc\",\"$value\");\n");
}

printf("\}\n");
