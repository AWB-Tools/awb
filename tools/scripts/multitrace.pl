#!/usr/intel/bin/perl

#
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


# Author : Koushik Chakraborty
use Getopt::Long;
use File::stat;
#$Getopt::Long::order = $REQUIRE_ORDER;

sub usage_and_exit() {
    print "USAGE : $0 <optional_flags> <threads> <tracefilesuffix> <Benchmarklist> \n";
    print "OPTIONS\n--tracedir <directory> : the place where the run and setup file will be put\n";
    print "\tdefault: $OPT{tracedir}\n";
    print "--configdir <directory> : the place where the configuration file will be written\n";
    print "\tdefault: $OPT{configdir}\n";
    print "--asimtrace <directory> : toplevel directory for all traces\n\tdefault: $OPT{asimtrace}\n";
    print "--fileprefix : prefix for config file\n\tdefault : $OPT{fileprefix}\n";
    print "\nQUICK EXAMPE:\n\t$0 8 05 TPC-C gzip bzip2 gcc\n";
    print "\n\n\t - this will create 8 threads, 2 each from the benchmarks TPC-C, gzip\n";
    print "\tbzip2 and gcc and put them in the directory $OPT{tracedir}/05\n\t and create";
    print " the configuration script \n\t$OPT{configdir}/$OPT{fileprefix}_8t_05.cfg\n";
    
    exit;
}

sub write_setup_file {
    my ($files) = @_;
    $set_up_file = "$newDir/setup";
    open WRFILE, ">$set_up_file" || die "Can't open file for writing setup script";
    print WRFILE "#!/bin/sh\n\n";
    print WRFILE "################################################\n#\n";
    print WRFILE "#   Benchmark setup for multiple threads \n#\n";
    print WRFILE "#   Usage : setup <srcdir> <destdir>\n#\n#";
    print WRFILE "    Setup benchmark to run in <destdir>\n";
    print WRFILE "################################################\n\n";
    print WRFILE "if [ \$\# -ne 2 ]; then\n     echo Usage \$0 \"<srcdir> <destdir>\"\n";
    print WRFILE "    exit 1\nfi\n";
    print WRFILE "\nsrcdir=\$1\ndestdir=\$2\n\n";
    foreach $file (@$files) {
        print WRFILE "/bin/ln -f -s $file \$destdir/\n";
    }
    print WRFILE "\n/bin/cp -f \$srcdir/run \$destdir/\n";
    close WRFILE;
    chmod 0755, $set_up_file;
}


sub write_run_file {
    my ($files) = @_;
    $run_file = "$newDir/run";
    print "run file=$run_file\n";
    open WRFILE, ">$run_file" || die "Can't open file for writing run script";
    print WRFILE "#!/bin/sh\n\n";
    print WRFILE "#################################################\n";
    print WRFILE "#\n# Benchmark exec script\n#\n";
    print WRFILE "# Script to run benchmark. Following variables will be \n";
    print WRFILE "# defined when this script is run.\n";
    print WRFILE "#\n# model\n# genFlags\n# sysFlags\n# feedFlags\n#\n";
    print WRFILE "#################################################\n";
    print WRFILE "\n\$model \$genFlags --feeder ";
    foreach $file (@$files) {
        print WRFILE "$file ";
    }
    print WRFILE " \$feedFlags --system \$systemFlags\n";
    close WRFILE;
    chmod 0755, $run_file;
}

sub write_config_file() {
    $mixer= $OPT{fileprefix} . "_$ARGV[1]";
    $config_file="$OPT{configdir}/$mixer.cfg";
    open CFGFILE, ">$config_file" || die "Couldn't open $config_file file";
    print CFGFILE "BmAdd {\n\t$mixer\n\t$mixer\n\t{$mixer}\n\tgtrace\n\t";
        print CFGFILE "/nfs_shr/tang23/asim/work/kchakrab/models/dev/src/asim-ipf/customtraces/mixed/$ARGV[1]/setup\n";
        print CFGFILE "\t{}\n\t{}\n\t{}\n\t{\n\t\tAwbStats dumponexit $mixer.stats\n\t\tAwbStats on\n\t\tAwbRun inst -1\n";
    print CFGFILE "\t\tAwbExit\n\t}\n}\n";
    close CFGFILE;
}    
    
    
%OPT = (
    "tracedir" => ".",
    "configdir" => ".",
    "asimtrace" => "/legacy/traces/asim/gtrace",
    "fileprefix" => "mixed",
    "help"   => 0
);


if (!GetOptions(\%OPT, 
                "tracedir=s",
                "configdir=s",
                "directory=s",
                "asimtrace=s",
                "fileprefix=s",
                "help!") ||
    $OPT{help}) {
        usage_and_exit();
}


$argcount=$#ARGV;
if ($argcount < 2) {
    usage_and_exit();
}

@Blist = [];
$count = 2;
while ($count <= $argcount) {
    push(@Blist, $ARGV[$count]);
    $count++;
}



print "tracedir  $OPT{tracedir} \nasimtrace $OPT{asimtrace} \nconfigdir = " , $OPT{configdir}, " \nArgument count=$#ARGV\n";

#exit;

$customdir=$OPT{tracedir};
#@Blist = ("TPC", "vpr", "mcf", "eon");
$numthreads = $ARGV[0];
$dirName=$ARGV[1];

$newDir="$customdir/$dirName";
print "Opening Dir $newDir \n";
mkdir $newDir;
$tbm = $numthreads/($#Blist+1);
print "Number of Threads  $ARGV[0], and tpbm = $tbm\n";
$threadCount = 0;
@configFiler=[];

$toplevel_dir=$OPT{asimtrace};
opendir DIR, $toplevel_dir;
$fileCount = 0;
foreach $file (readdir DIR) {
    $listCount = 0;
    while ($listCount <= $#Blist) {
        if ($file =~ /$Blist[$listCount]/) {
            $nextdir = "$toplevel_dir/$file";
            opendir DIR2, $nextdir;
            @lowFiles = [];
            foreach $file_2 (readdir DIR2) {
                if ($file_2 =~ /MCK/ || $file_2 =~ /^[0-9]/ || $file_2 =~ /mac/) {
                    push(@lowFiles, $file_2);
                }
            }
            closedir DIR2;
            $tc=0;
            @randomList = [];
            while ($#lowFiles > 0 && $tc < $tbm) {
                $random = int(rand($#lowFiles));
                print "size=$#lowFiles random=$random top = $file File Name = $lowFiles[$random+1]\n";
                $flag = 0;
                foreach $entry (@randomList) {
                    if ($entry == $random)
                    {
                        $flag = 1;
                    }
                }
                
                if ($flag == 0) {
                    push(@randomList, $random);
                    $leaf = $lowFiles[$random+1];
                    $leafdir = "$nextdir/$leaf";
                    opendir DIR3, $leafdir;
                    foreach $file_3 (readdir DIR3) {
                        if ($file_3 =~ /bz2$/) {
                            my $sb = stat("$leafdir/$file_3");
                            if ($sb->size > 100000000) {
                                print "File size of $leafdir/$file_3 = ", $sb->size, "\n";
                                push(@fileList, "$leafdir/$file_3");
                                push(@leafList, $file_3);
                                push(@configFiler, $leaf);
                                $tc++;
                            }
                        }
                    }
                    closedir DIR3;
                    
                }
            }
        }
        $listCount++;
    }
            
            
    $fileCount++;
}

#Create the Setup files
write_setup_file( \@fileList);
write_run_file( \@leafList);
write_config_file();
foreach $entry (@configFiler) {
    if ($entry =~ /^[0-9]/)
    {
        print "config/bm/Traces/GTrace/Tanglewood/TPC-C/tpcc_$entry.cfg\n";
    } else {
        print "config/bm/Traces/GTrace/Tanglewood/TPC-C/$entry.cfg\n";
    }
}

# Set up the configuration file
    
    
