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


use strict;


package awb_util;

use File::Basename;

use awb_runlog;

our $model_file = undef;
our $model = undef;

our $benchmark_file = undef;
our $benchmark = undef;

our $build_switches = "";
our $run_switches = "";

our $apm_editor = $ENV{APM_EDITOR} || "apm-edit";

# TBD - a real benchmark editor
our $abm_editor = $ENV{EDITOR} || "emacs";

our $debug = 1;

sub init {
  print "Initing awb\n" if $debug;

  return();
}

sub set_model {
  $model_file = shift;
  $model = undef;

  if (defined($model_file)) {
    $model = Asim::Model->new($model_file);
  }

  if (! defined($model)) {
    print "Undefined Model\n" if $debug;
    $model_file = undef;
    $model = undef;
    return undef;
  }

  print "Model = $model_file\n" if $debug;
  print $model->filename() . "\n" if $debug;

  return $model;
}


sub set_benchmark {
  $benchmark_file = shift;
  $benchmark = undef;

  if (defined($benchmark_file)) {
    if ($benchmark_file =~ /\.cfx/) {
      # Script generated name
      my ($cfx, $args) = cfx_split($benchmark_file);

      $benchmark = $Asim::default_workspace->resolve($cfx);
      if ($benchmark ne '') {
        $benchmark .= "/$args";
      }
    }
    else {
      #
      # Remove dummy region suffix (_r[n])
      #
      (my $region) = ($benchmark_file =~ /(_r[0-9]*)$/);
      my $real_file = $benchmark_file;
      $real_file =~ s/\_r[0-9]*$//;

      $benchmark = $Asim::default_workspace->resolve($real_file);

      #
      # Put the suffix back.
      #
      $benchmark = $benchmark . $region if (defined($region));
    }
  }

  if (! defined($benchmark)) {
    print "Undefining Benchmark\n" if $debug;
    $benchmark_file = undef;
    $benchmark = undef;
    return undef;
  }
}

sub set_build_switches {
  $build_switches = shift;
}


sub set_run_switches {
  $run_switches = shift;
}


#
# Helper function for benchmark_regions.  Cache information about an entire
# CFX file's pseudo directory with one call invocation of the script.  This
# makes awb run much faster.
#

our %regions_cache = ();

sub update_cfx_region_cache {
  my $cfxBase = shift;
  my $cfx = shift;
  my $args = shift;

  # In the typical case we don't need to know the true number of regions --
  # just whether there are multiple regions.  CFX scripts can answer this
  # without having to read each CFG file.  Since awb will likely ask about
  # all workloads in the directory we populate the cache for everything.
  my $cfxdir = $args;
  $cfxdir =~ s/\/.*$//;
  my $expireTime = time() + 60 + rand() * 60;
  open(CFX, "${cfx} --listflags |") || die("Open ${cfx} failed");
  while (<CFX>) {
    chomp($_);
    my $wrk = $_;
    $wrk =~ s/ .*//g;
    $wrk = "${cfxBase}/${wrk}";
    my $isMulti = 0;
    if (($_ =~ /--queryregions/) || ($_ =~ /--regions/)) {
      $isMulti = 2;
    }
    $regions_cache{$wrk}[0] = $isMulti;
    $regions_cache{$wrk}[1] = $expireTime;
    $regions_cache{$wrk}[2] = 0;
  }
  close(CFX);
}

sub check_region_cache {
  my $cfg = shift;
  my $computeRegions = shift;

  #
  # Maintain a hash of previous requests since the GUI makes frequent repeated
  # requests and looking up the number of regions is slow.
  #
  if (exists($regions_cache{$cfg})) {
    # Array element [2] is 1 if the true number of regions is stored in the hash.
    # The incoming request is either 0 or 1.  This test makes sure that the true
    # number of regions is returned when needed.  We do this because computing
    # the true number of regions is expensive and sometimes the GUI just needs
    # to know whether a workloads has 1 or multiple regions.
    if ($computeRegions <= $regions_cache{$cfg}[2]) {
      # Cache entries are valid for a limited time case config. changes
      # Use a random number to avoid invalidating all entries in a directory at
      # once.
      if (time() < $regions_cache{$cfg}[1]) {
        return (1, $regions_cache{$cfg}[0]);
      }
    }
  }

  return (0, 0);
}

#
# Return the number of regions in a benchmark.  If $computeRegions is true
# then return an exact number, otherwise just return a non-zero value for
# benchmarks with regions.  For some configurations the setup program must
# be run to determine the number of regions, which may be slow.  Pass
# 0 for $computeRegions unless you need to know the exact number.
#

sub benchmark_regions {
  my $cfg = shift;
  my $computeRegions = shift;

  if (! ($cfg =~ /.cfg$/)) {
    return 0;
  }

  my ($valid, $value) = check_region_cache($cfg, $computeRegions);
  if ($valid) {
    return $value;
  }

  my $path = '';
  my $cfgFile = '';

  if ($cfg =~ /\.cfx/) {
    # Script generated cfg file
    my ($cfxBase, $args) = cfx_split($cfg);
    my $cfx = $Asim::default_workspace->resolve("$cfxBase");
    return 0 if (!defined($cfx) || $cfx eq '');

    if (! $computeRegions) {
      update_cfx_region_cache($cfxBase, $cfx, $args);
      ($valid, $value) = check_region_cache($cfg, $computeRegions);
      if ($valid) {
          return $value;
      }
    }

    $cfgFile = "${cfx} --emit ${args} |";
    $path = "${cfx}/${args}";
  }
  else {
    $path = $Asim::default_workspace->resolve("$cfg");
    return 0 if (!defined($path) || $path eq '');
    $cfgFile = $path;
  }

  #
  # Must read the .cfg file and look for region tags
  #
  open(CFG, $cfgFile) || die("Open $cfgFile failed");

  my $lNum = -1;
  my $setupCmd = '';
  my $queryRegions = 0;
  my $nRegions = 0;

  while (<CFG>) {
    chomp($_);
    s/^ *//;
    s/} *//;

    if ($_ =~ /^BmAdd/) {
      #
      # Found first tag line.  The setup script name is 5 lines later.
      # Lovely format, eh?
      #
      $lNum = 0;
    }
    else {
      $lNum += 1 if ($lNum >= 0);
    }

    # Line 5 after the BmAdd has the setup command
    if ($lNum == 5) {
      $setupCmd = $_;
    }

    # --queryregions:  ask the setup script how many regions in the benchmark
    if ($_ =~ /--queryregions/) {
      $queryRegions = 1;
    }

    # --regions:  set the number of regions explicitly
    (my $nr) = ($_ =~ /--regions *([0-9]*)/);
    if (defined($nr)) {
      $nRegions = $nr;
    }
  }
  close(CFG);

  # Make valid time somewhat random to keep all entries from invalidating at once
  $regions_cache{$cfg}[1] = time() + 60 + rand() * 60;
  $regions_cache{$cfg}[2] = 1;

  if ($nRegions > 0) {
    $regions_cache{$cfg}[0] = $nRegions;
    return $nRegions;
  }

  if ($queryRegions) {
    #
    # Must run setup script to find out how many regions there are.
    # This allows for complex SoftSDV benchmark setups including
    # support for multiple sites with different checkpoints and
    # the same benchmark .cfg file describing multiple SoftSDV
    # releases.
    #

    if ($computeRegions == 0) {
      #
      # Caller doesn't need an exact (slow) answer.  Just indicate .cfg
      # file has regions.
      #
      $regions_cache{$cfg}[0] = 2;
      $regions_cache{$cfg}[2] = 0;
      return 2;
    }

    my $script = $setupCmd;
    $script =~ s/  *.*//;
    $script = $Asim::default_workspace->resolve($script);
    my $args = $setupCmd;
    $args =~ s/^[^ ]* *//;
    my $dir = dirname($path);
    open(SC, "${script} ${args} --queryregions ${dir} |") ||
        die("Can't query regions for ${cfg}");
    $nRegions = <SC>;
    chomp($nRegions);
    close(SC);
    if ($nRegions eq '' || $nRegions =~ /[^0-9]/) {
        print "Unexpected response for --queryregions, ${cfg}\n";
    }
    $regions_cache{$cfg}[0] = $nRegions;
    return $nRegions;
  }

  $regions_cache{$cfg}[0] = 0;
  return 0;
}


##
## Split a CFX path into the script and the argument parts
##
sub cfx_split {
  my $path = shift;

  my $cfx = $path;
  $cfx =~ s!\.cfx.*!.cfx!;

  my $args = $path;
  $args =~ s!.*\.cfx!!;
  $args =~ s!^/!!;

  return ($cfx, $args);
}


##
## List of subdirectory built by a CFG generator script.
##
our %cfx_cache = ();

sub cfx_list_dir {
  my $dir = shift;

  # Cache CFX directory listings since the GUI asks for them repeatedly and
  # looking up the directory is slow.  They expire after a while.
  if (exists($cfx_cache{$dir}) && (time() < $cfx_cache{$dir}[1])) {
      return @{$cfx_cache{$dir}[0]};
  }

  my ($cfx, $args) = cfx_split($dir);

  my @result;

  my $script = $Asim::default_workspace->resolve($cfx);
  if (-x $script) {
    #
    # Run the script to find the next level in the hierarchy
    #
    my $cmd = $script . " --dir " . $args;
    if (open(CFX, "${cmd} |")) {
      while (<CFX>) {
        chomp;
        my $leaf = $_;
        $leaf =~ s!.*/!!g;
        $result[++$#result] = $leaf;
      }
      close(CFX);
    }
  }

  # Cache the result
  $cfx_cache{$dir}[0] = \@result;
  $cfx_cache{$dir}[1] = time() + 60 + rand() * 60;

  return @result;
}


##
## Quick check to decide whether a path may have children.
##
sub is_dir {
  my $dir = shift;

  #
  # Is there is a .cfx name in the path it is a script that generates the
  # lower levels.  These levels aren't really directories.
  #
  if ($dir =~ /\.cfx/) {
    # Say a script path is expandable if not pointing to a .cfg leaf
    return 1 if (! ($dir =~ /\.cfg$/));
  }
  else {
    # Expandable if it is a directory
    return 1 if (-d $Asim::default_workspace->resolve("$dir"));
  }

  return 0;
}


sub glob_dir {
  my $dir = shift;
  my @subdirs = ();

  #
  # Is there is a .cfx name in the path it is a script that generates the
  # lower levels.  These levels aren't really directories.
  #
  if ($dir =~ /\.cfx/) {
    if (! ($dir =~ /\.cfg$/)) {
      for my $i (cfx_list_dir($dir)) {
        if (! ($i =~ /\.cfg$/)) {
          push(@subdirs, "$dir/$i");
        }
        else {
          #
          # .cfg files with multiple regions look like directories
          #
          if (benchmark_regions("$dir/$i", 0)) {
            push(@subdirs, "$dir/$i");
          }
        }
      }
    }
  }
  else {
    for my $i ($Asim::default_workspace->listdir($dir)) {
      if (-d $Asim::default_workspace->resolve("$dir/$i")) {
        push(@subdirs, "$dir/$i");
      }

      if (($i =~ /\.cfx$/) && -x $Asim::default_workspace->resolve("$dir/$i")) {
        #
        # .cfx files are scripts to generate what looks like a tree of .cfg
        # files.  Treat .cfx like a directory.
        #
        push(@subdirs, "$dir/$i");
      }

      if ($i =~ /\.cfg$/) {
        #
        # .cfg files with multiple regions look like directories
        #
        if (benchmark_regions("$dir/$i", 0)) {
          push(@subdirs, "$dir/$i");
        }
      }
    }
  }

  return (@subdirs);
}


sub glob_apm {
  my $dir = shift;
  my @models = ();

  for my $i ($Asim::default_workspace->listdir($dir)) {
    if ($i =~ /\.apm$/) {
      push(@models, "$dir/$i");
    }
  }
  return (@models);
}



sub glob_abm {
  my $dir = shift;
  my @benchmarks = ();

  if ($dir =~ /\.cfg$/) {
    #
    # A .cfg file that looks like a directory due to multiple regions
    #
    my $nRegions = benchmark_regions($dir, 1);
    if ($nRegions > 0) {
      if (defined($ENV{'SOFTSDV_SETUP'})) {
        # Setup must be able to configure the base configuration in addition
        # to regions.
        push(@benchmarks, "$dir");
      }

      for my $j (1..$nRegions) {
        push(@benchmarks, "${dir}_r${j}");
      }
    }
    else {
      push(@benchmarks, "$dir");
    }
  }
  else {
    my @files;

    #
    # Normal directory or CFX script.
    #
    if ($dir =~ /\.cfx/) {
      @files = cfx_list_dir($dir);
    }
    else {
      @files = $Asim::default_workspace->listdir($dir);
    }

    for my $i (@files) {
      if ($i =~ /\.cfg$/) {
        if (benchmark_regions("$dir/$i", 0) == 0) {
          push(@benchmarks, "$dir/$i");
        }
      }
    }
  }

  return (@benchmarks);
}


#
# Maybethese shouldn't be here, and they should output to a dialog....
#

sub new_model {
  return ! system("$apm_editor &");
}

#
# Actions to edit/open/shell at a model
#     Note: maybe these should be refactored into Model.pm
#

sub edit_model {
  return undef if (! defined($model));

  my $command = "$apm_editor " . $model->filename();

  return ! system("$command &");
}


sub open_model_container {
  return undef if (! defined($model));

  my $dirname = dirname(Asim::resolve($model->filename));

  return Asim::open_at($dirname);
}

sub shell_model_container {
  return undef if (! defined($model));

  my $dirname = dirname(Asim::resolve($model->filename));

  return Asim::shell_at($dirname);
}


sub nuke_model()
{
  my $w;  
  my $cmd;

  return undef
      unless (defined($model));

  $w = awb_runlog();

  $cmd = $model->nuke("--getcommand" => 1);
  $w->run($cmd);

  return 1;
}

sub config_model()
{
  my $w;
  my $cmd;

  return undef if (! defined($model));

  $w = awb_runlog();

  $cmd = $model->configure("--getcommand" => 1);
  $w->run($cmd);

  return 1;
}


sub clean_model()
{
  my $w;
  my $dir;

  return undef
      unless (defined($model));

  $dir = $model->build_dir();

  $w = awb_runlog();

  my $cmd = $model->clean("--getcommand" => 1);
  $w->run($cmd);

  return 1;
}

sub build_model()
{
  my $w;
  my $cmd;

  return undef
      unless (defined($model));

  $w = awb_runlog();
  $cmd = $model->build("--getcommand" => 1,
		       "--buildopt" => $build_switches);

  if ($build_switches =~ /dox/) {
    my $dir = $model->build_dir();
    my $html_cmd = system ("mozilla $dir/dox/html/index.html &");
    $w->run($cmd, $html_cmd);
  }
  else {
    $w->run($cmd);
  }

  return 1;
}


sub get_params_model()
{
  return () if (! defined($model));
  return () if (! defined($benchmark));

  my @params = ();

  my $w = awb_runlog();

  if ($model->type() eq "HAsim") {
    Qt::MessageBox::information(
            undef,
            "No support",
            "HAsim does not yet support setting parameters from the GUI.\n");
    return ();
  }
  elsif ($model->type() eq "Leap") {
    Qt::MessageBox::information(
            undef,
            "No support",
            "Leap does not yet support setting parameters from the GUI.\n");
    return ();
  }

  open(RUN, "amc --model $model_file --benchmark $benchmark --runopt \"$run_switches -listparams\" run |")
    || return ();

  while (<RUN>) {
    if (/dynamic param/) {
      last;
    }
  }

  while (<RUN>) {
    chomp($_);
    s/^ *//;
    s/= (.*)/= $1 [$1]/;
    push(@params, $_);
  }

  close(RUN);

  return (@params);
}


sub new_benchmark {
  return ! system("$abm_editor &");
}


sub edit_benchmark {
  my $command;

  return undef if (! defined($benchmark));

  $command = "$abm_editor $benchmark";

  return ! system("$command &");
}



sub setup_benchmark
{
  return undef if (! defined($model));
  return undef if (! defined($benchmark));

  my $w = awb_runlog();
  my $builddir = $model->build_dir();
  my $rundir   = $model->run_dir($benchmark);

  my $cmd = $model->setup($benchmark,
			  "--builddir" => $builddir,
			  "--rundir" => $rundir,
			  "--getcommand" => 1);
  $w->run($cmd);

  return 1;
}

sub run_model
{
  my $w;
  my $cmd;

  return undef if (! defined($model));
  return undef if (! defined($benchmark));

  $w = awb_runlog();

  $cmd = $model->run($benchmark,
		     "--getcommand" => 1,
		     "--runopt" => $run_switches
		     );
  $w->run($cmd);

  return 1;
}


sub view_statistics {
  return undef if (! defined($model));
  return undef if (! defined($benchmark));

  #
  # Find stats file
  #    TBD: Generalize this sequence
  #
  my $dir = $model->run_dir($benchmark);
  my $name = basename($dir);
  my $file = "$dir/$name.stats";

  if (! -e $file) {
    Qt::MessageBox::information(
            undef,
            "No input file",
            "Needed input file $file does not exist\n");
    return;
  }

  system("mozilla file:$file &");
}


sub view_stripchart {
  return undef if (! defined($model));
  return undef if (! defined($benchmark));

  my $dir = $model->run_dir($benchmark);
  my $file = "$dir/strip_chart.stb";

  if (! -e $file) {
    Qt::MessageBox::information(
            undef,
            "No input file",
            "Needed input file $file does not exist\n");
    return;
  }

  system("cd $dir; StripChartViewer $file &");
}

sub view_cycledisplay {
  my $adf = shift;
  my $adf_switch = "";

  return undef if (! defined($model));
  return undef if (! defined($benchmark));

  my $dir = $model->run_dir($benchmark);
  my $file = "$dir/Events.drl";

  if (! -e $file) {
    $file .= ".gz";
  }

  if (! -e $file) {
    Qt::MessageBox::information(
            undef,
            "No event trace (dral) file",
            "Needed input trace file $file does not exist\n");
    return;
  }

  if ($adf ne "") {
    $adf_switch = "-adf $adf";
  }

  system("cd $dir; dreams $adf_switch $file &");
}

#
# Following was intended to allow model and benchmark editing to
# operate as modal sub-dialogs, but it doesn't work....

sub runmodal
{
    our $debug = 0;
#    my $window = shift;
    my $command = shift;

    my $a = $main::app;

    my $running = 0;

    my $rin;
    my $win;
    my $ein;
    my $rout;
    my $timeout = 1;
    my $count = 0;

    my $nfound;
    my $timeleft;
    my $s;
    my $x;

    print "Runit\n" if $debug;

#    $window->show();
    $a->processEvents();

    open(SUBJOB, '-|', $command . " 2>&1") || die("Open failed");
    print "Open: OK\n" if $debug;

    $running = 1;

    $rin = $win = $ein = '';
    vec($rin,fileno(*SUBJOB),1) = 1;
#   vec($win,fileno(STDOUT),1) = 1;
    $ein = $rin | $win;

    while ($running) {

        ($nfound,$timeleft) = select($rout=$rin, undef, undef, $timeout);
        if (! defined($nfound)) {
            print "Select failed\n" if $debug;
            $running = 0;
            last;
        }

        print $count++ . ": Nfound = $nfound, Timeleft = $timeleft\n" if $debug;
        if ($nfound) {
            my $first = 1;
            if  ($first || $s) {
                $s = sysread(*SUBJOB, $x="", 1024);
                if (!defined($s) || ($first && ! $s)) {
                    print "End of file\n" if $debug;
                    $running = 0;
                    last;
                }
                $first = 0;
                print "$x\n" if $debug;
                $a->processEvents();
            }
        } else {
            print "Nothing to read\n" if $debug;
            $a->processEvents();
        }
    }

    close(SUBJOB);

}

1;
