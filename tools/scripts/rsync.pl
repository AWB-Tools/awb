eval 'exec perl -w "$0" ${1+"$@"}'
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

##################################################################################
# Author: Krishna Rangan
# Purpose:
#       Custom rsync script; presently for private use.  Will enhance later.
##################################################################################

use File::Basename;
use Getopt::Long;
use IO::Handle;
use Term::ANSIColor;

$status = GetOptions(   "filelist=s"     => \$filelist,
			"remote=s"       => \$remote,
			"local=s"        => \$localarea,
                        "verbose!"       => \$verbose
		    );

if (!$status || !$remote || !$filelist) {
    my $prog = basename($0);
    print STDERR "\t$prog: Insufficient switches; file list and remote host name are required.\n";
    exit 1;
}

# check inputs

if (!$localarea) {
    $localarea = ".";
}

(-f $filelist) || die ("File list $filelist does not exist.\n");
open(FL, "<$filelist") || die ("Cannot read $filelist\.");

# real work

my @files = <FL>;
my $nfiles = @files;
my $i = 0;

foreach $file (@files) {
    chomp $file;
    $file =~ /.*\/traces\/(.*)\/(.*)/;
    my $basename = $1;
    my $filename = $2;

    system("mkdir -p $basename");
    if ($i % 2) {
	print color 'reset';
    }
    else {
	print color 'green';
    }
    print("\nrsync-ing \'$file\' to \'$localarea/$basename/\' ... ") unless (!$verbose);
    system("rsync $remote:\"$file.\*\" $localarea/$basename\/");
    printf("%.2f%s", ($i/$nfiles) * 100, "\% complete.") unless (!$verbose);
    $i++;
}

exit 0;



