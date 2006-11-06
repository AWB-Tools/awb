:
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


#
# random.pl
#
# Purpose: random stress test for SMT workloads under ASIM/ev8 model;
#          We pick a random number of threads [2..$CPU_THREADS],
#          each with a random benchmark (from the Spec95 suite),
#          and each thread skips a random number of instructions [1..$MAX_SKIP].
#
# Author: Artur Klauser
#

# defined constants
$CPU_THREADS = 4;       ## max number of concurrent threads
$MAX_SKIP = 1000000;    ## max number of instructions to skip in each thread

# file and directory names
$awbcmds_template = "awbcmds.mt-template";
$awbcmds_file     = "awbcmds";
$run_template     = "run";
$run_file         = "run";

$debug = 0;

# get command line arguments
if ($#ARGV != 1) {
  usage_and_exit();
} else {
  $basedir = shift @ARGV;   # directory where all files live
  $setupdir = shift @ARGV;  # directory that we are about to set up now
}

sub usage_and_exit {
  print "ERROR: missing directory on command line\n";
  print "Usage: $0 <base_directory> <setup_dir>\n";
  print "\n";
  exit(1);
}

# prefix filenames with directories
$awbcmds_template = "$basedir/$awbcmds_template";
$awbcmds_file     = "$setupdir/$awbcmds_file";
$run_template     = "$basedir/$run_template";
$run_file         = "$setupdir/$run_file";

##
## SPEC 95 benchmarks
##
$lsp_files = `cd $basedir; ls *.lsp`;
$lsp_files =~ s/\n/ /g;

##
## all input files that need be copied for each benchmark
##
%FILES = (
## SpecInt
"compress" => 'compress compress.in',
"ijpeg"    => 'ijpeg vigo.ppm',
"gcc"      => 'gcc 2cp-decl.i',
"go"       => 'go 9stone21.in',
"li"       => "li au.lsp $lsp_files",
"m88ksim"  => 'm88ksim ctl.in dcrand.lit dhry.lit dcrand.big dhry.big',
"perl"     => 'perl scrabbl.pl scrabbl.in dictionary',
"vortex"   => 'vortex vortex.in lendian.wnv lendian.rnv persons.1k',
## SpecFP
"applu"    => 'applu applu.in',
"apsi"     => 'apsi apsi.in',
"fpppp"    => 'fpppp natoms.in',
"hydro2d"  => 'hydro2d hydro2d.in HYDRO2D.MODEL',
"mgrid"    => 'mgrid mgrid.in',
"su2cor"   => 'su2cor su2cor.in SU2COR.MODEL',
"swim"     => 'swim swim.in',
"tomcatv"  => 'tomcatv tomcatv.in TOMCATV.MODEL',
"turb3d"   => 'turb3d turb3d.in',
"wave5"    => 'wave5 wave5.in'
);

##
## command string to run each benchmark
##
%RUN = (
## SpecInt
"compress" => './compress < compress.in > compress.$OUT',
"ijpeg"    => './ijpeg -image_file vigo.ppm -compression.quality 90'.
              ' -compression.optimize_coding 0'.
              ' -compression.smoothing_factor 90 -difference.image 1'.
              ' -difference.x_stride 10 -difference.y_stride 10 -verbose 1'.
              ' -GO.findoptcomp  >vigo.$OUT',
"gcc"      => './gcc -quiet -funroll-loops -fforce-mem -fcse-follow-jumps'.
              ' -fcse-skip-blocks -fexpensive-optimizations -fstrength-reduce'.
              ' -fpeephole -fschedule-insns -finline-functions'.
              ' -fschedule-insns2 -O 2cp-decl.i -o 2cp-decl.$OUT',
"go"       => './go 50 21 9stone21.in > 9stone21.$OUT',
"li"       => "./li au.lsp $lsp_files " . '> li.$OUT',
"m88ksim"  => './m88ksim -c < ctl.in > ctl.$OUT',
"perl"     => './perl scrabbl.pl < scrabbl.in > scrabbl.$OUT',
"vortex"   => './vortex vortex.in > vortex.$OUT',
## SpecFP
"applu"    => './applu < applu.in > applu.$OUT',
"apsi"     => './apsi > apsi.$OUT',
"fpppp"    => './fpppp < natoms.in > natoms.$OUT',
"hydro2d"  => './hydro2d < hydro2d.in > hydro2d.$OUT',
"mgrid"    => './mgrid < mgrid.in > mgrid.$OUT',
"su2cor"   => './su2cor < su2cor.in > su2cor.$OUT',
"swim"     => './swim < swim.in',
"tomcatv"  => './tomcatv < tomcatv.in > tomcatv.$OUT',
"turb3d"   => './turb3d < turb3d.in > turb3d.$OUT',
"wave5"    => './wave5 < wave5.in > wave5.$OUT'
);

@programs = (sort keys %RUN);

$| = 1;  # always flush writes to STDOUT

##
## we setup one random test in the basedir; the configuration for the test
## can be found in the "run" and "awbcmds" files that are generated;
##

# read in templates
open IN, "<$awbcmds_template"
  or die "can't open awbcmds template $awbcmds_template\n$!";
@awbcmds_template = <IN>;
close IN;

open IN, "<$run_template"
  or die "can't open run template $run_template\n$!";
@run_template = <IN>;
close IN;

# decide how many benchmarks to run
$nBench = int (2 + rand ($CPU_THREADS - 1));

# generate random skipping
for ($i = 0; $i < $nBench; $i++) {
  $skip[$i] = int (1 + rand $MAX_SKIP);
}

# create run file from template
$commandline = '$model $genFlags --feeder feeder \\'."\n";
for ($i = 0; $i < $nBench; $i++) {
  # pick a new random benchmark
  $progNum[$i] = int (rand ($#programs + 1));

  # setup commandline for this benchmark
  $commandline .= 
    '-p "' . $RUN{$programs[$progNum[$i]]} . '" \\'."\n";
  $commandline =~ s/(\S+)\$OUT/$1$i.out/;

  # copy input files required for this benchmark
  foreach $file (split / /, $FILES{$programs[$progNum[$i]]}) {
    system "ln -s -f $basedir/$file $setupdir/$file";
  }
}
$commandline .= '$feedFlags --system $sysFlags';
open OUT, ">$run_file";
foreach (@run_template) {
  s/\$commandline/$commandline/;
  s/(awbcmds.mt)/$awbcmds_file/;
  s/(\$model)/env -i $1/;
  print OUT $_;
}
close OUT;
chmod 0755, "$run_file";

# create awbcmds file from template
open OUT, ">$awbcmds_file";
foreach (@awbcmds_template) {
  if (m/AwbSkip (\d+)/) {
    # only produce a AwbSkip line for the benchmarks that we actually run
    if ($1 >= $nBench) {
      next;
    }
    s/(AwbSkip (\d+)) \d+/$1 $skip[$2]/;
    print OUT $_;
  } else {
    print OUT $_;
  }
}
close OUT;

if ($debug) {
  for ($i = 0; $i < $nBench; $i++) {
    printf "    $programs[$progNum[$i]] skip $skip[$i]";
    printf "\n"  if ($i < $nBench - 1);
  }
  printf "\n";
}

exit(0);

##-----------------------------------------------------------------------------
