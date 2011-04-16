################################################################
#
# @brief Asim/GenCFG/Auto.pm : An additional layer around GenCFG
#   to make it really easy to generate CFG files and associated
#   setup and run scripts for benchmark configuration on the fly.
# 
# @author Carl Beckmann
#
# Copyright (c) 2006-7 Intel Corporation, all rights reserved.
# THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
# IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
#
################################################################

package Asim::GenCFG::Auto;

use warnings;
use strict;
use Asim;
use Asim::GenCFG;
use File::Temp qw/ tempfile tempdir /;
use File::Basename;
use File::Spec;


our $DEBUG = ($ENV{ASIM_DEBUG} || 0) >= 2;

=head1 NAME

Asim::GenCFG::Auto - Support for building workload CFG data, and setup and run scripts, on the fly.

=head1 SYNOPSIS

    use Asim::GenCFG::Auto;
    Asim:GenCFG::Auto::Generate( [optional args...] );

=head1 DESCRIPTION

This module builds upon the methods in Asim::GenCFG to make it very easy to generate
benchmark configurations in a small number of lines of code.

It is used to generate benchmark CFG data, using as input only the "name" of the virtual
CFG file to be generated.  A typical application is to extract a portion of that file name,
use it to look up the location of a trace file, and generate an Asim benchmark configuration
file for that trace.

A reasonable benchmark configuration can be generated with just 2 lines of code:

    use Asim::GenCFG::Auto;
    Asim:GenCFG::Auto::Generate();

In this case, a default "setup" and "run" script are generated
that use fetch-trace to get the bencmark into the local trace cache and supply this as a
--feeder argument to the Asim simulator.

The caller can specify an alternate setup script by supplying a file name,
or by supplying the literal text of a setup and run script pair.
The caller can also specify any of the arguments given to Asim::GenCFG::add()
(which is called by Asim::GenCFG::Auto::Generate()) to change system-, feeder-,
or general flags, or change the name or type of the benchmark.

=cut

################################################################

=head2 The Generate() command

The Generate() function is called with a list of named arguments as follows:

    Asim::GenCFG::Auto::Generate( name => value, ... );
    
The arguments to Generate() are the same as the arguments to Asim::GenCFG::add(),
with a few additions and changes:

=over 4


=item setup

"Setup" is a mandatory argument to Asim::GenCFG:add(), but it is an optional argument to Generate().
If omitted, a setup and a run script will be generated from text supplied as the "setup_script"
and "run_script" arguments, or from built-in defaults.
These scripts are called by B<amc> to setup the benchmark directory in the user's workspace,
and to run the model, respectively.

B<amc> calls the setup script as follows:

  <setup_script> <user_args> <setup_script_dir> <benchmark_run_dir>
  
where "<setup_script> <user_args>" is what is specified as the "setup" argument.
If <user_args> are omitted, the setup script is just called with two arguments,
the directory containing the setup script,
and the destination directory where the benchmark will be run.

B<amc> calls the run script as follows:

  <run_script> <args_from_models_file>

where <ags_from_model_file> are anything that follows the model binary
on the line in the asim-run experiments file.


=item setup_script

Supply the literal text for a setup script.
This is actually a template, wherein certain macros of the form
B<@macroname@> will be sustituted, see below.


=item run_script

Supply the literal text for a run script.
Subject to macro substitution as described below.

=back 4

=cut

################################################################

our $SUBST_PATH;
our $SUBST_NAME;
our $SUBST_RUNSCRIPT = 'run';
our $SetupScriptTemplate;
our $RunScriptTemplate;
our @VirtualPath;
our @Directory = ();

BEGIN {
  Asim::init();

  ##
  ## First, parse the arguments to extract a benchmark "name" and "path"
  ## according to what is in the .benchmarks file:
  ##    /path/to/this/script.cfx/path/to/benchmark.cfg
  ## where "name" is "benchmark" and "path" is "/path/to/this/benchmark"
  ##

  if ( $ARGV[1] ) {
    # get absolute path of the trace, which follows the --emit arg:
    ($SUBST_PATH = $ARGV[1]) =~ s/\.cfg$//;

    # strip the directory off to get a name:
    ($SUBST_NAME = $SUBST_PATH) =~ s/^.*\///;

    # get the directory tree as an array:
    @VirtualPath = split '/', $SUBST_PATH;
    $#VirtualPath--;
  }
}

sub Generate {
  if ( $ARGV[0] ne '--emit' ) {
    # handle AWB browsing commands
    GenDirectory();
    return;
  }

  # if the trace argument looks like a path, rather than a simple name,
  # we need to prepend a '/' which gets stripped by the caller:
  if ( $SUBST_PATH =~ m/\// ) {
    $SUBST_PATH = '/' . $SUBST_PATH;
  }
  
  ##
  ## get args, with defaults
  ##

  my %params = (
    name         => $SUBST_NAME,
    info         => $SUBST_NAME,
    file         => $SUBST_NAME,
    tree         => \@VirtualPath,
    feeder       => 'archlib',
    setup_script => $SetupScriptTemplate,
    run_script   => $RunScriptTemplate
  );
  my %overrides = @_;
  foreach my $argname ( keys %overrides ) {
    $params{$argname} = $overrides{$argname};
  }
  
  ##
  ## maybe generate setup script from the template
  ##

  if ( $params{'setup'} ) {
    # if the user specified a setup script filename,
    # make sure they didn't also specify setup script text:
    if ( $overrides{'setup_script'} ) {
      die "please specify only one of \"setup\" or \"setup_script\" arguments\n";
    }
  } else {
    # generate run scripts from default or supplied text:
    (my $fp, $SUBST_RUNSCRIPT) = tempfile();
    print $fp TemplateSubstitutions( $params{'run_script'} );
    close $fp;
    system("chmod +x $SUBST_RUNSCRIPT");
    # generate setup scripts from default or supplied text:
    ($fp, my $setup_script) = tempfile();
    print $fp TemplateSubstitutions( $params{'setup_script'} );
    close $fp;
    system("chmod +x $setup_script");
    $params{'setup'} = $setup_script;
  }

  ##
  ## now generate the virtual .cfg file:
  ##
  
  # make substitutions in config file parameters
  foreach my $key ( keys %params ) {
    $params{$key} = TemplateSubstitutions($params{$key});
  }
  
  # generate the config file
  my $gcfg = Asim::GenCFG->new();
  $gcfg->add( %params );
  $gcfg->action(@ARGV);
}

################################################################

=head2 Default setup and run scripts

The default setup script uses fetch-trace to get the trace cache location
and soft-links the trace files into the benchmark run directory:

    #!/bin/sh
    trace=`fetch-trace @BENCHMARKPATH@ | tail -1`
    ln -sf $trace* $2/
    cp -f @RUNSCRIPT@ $2/run

The default run script passes the name of the trace in the --feeder section,
and prints the simulator command before executing it.
It adds /p/asim/i386_linux24/bin to the search path
so archlib-based feeders can find the script B<asd2rf>.

    #!/bin/sh
    PATH=$PATH:/p/asim/i386_linux24/bin
    command="$model $genFlags --feeder -t @BENCHMARKNAME@ $feedFlags --system $sysFlags"
    echo $command
    eval $command

Note that B<amc> adds lines to the run script after the first line
to set the $model, $genFlags, $feedFlags, and $sysFlags.

=cut

$SetupScriptTemplate =
'#!/bin/sh
trace=`fetch-trace @BENCHMARKPATH@ | tail -1`
ln -sf $trace* $2/
cp -f @RUNSCRIPT@ $2/run
';

$RunScriptTemplate =
'#!/bin/sh
PATH=$PATH:/p/asim/i386_linux24/bin
command="$model $genFlags --feeder -t @BENCHMARKNAME@ $feedFlags --system $sysFlags"
echo $command
eval $command
';

################################################################

=head2 Substitutions

The following macros will be substituted when generating the actual setup and run scripts:

=over 4

=item @BENCHMARKNAME@

The "name" of the generated benchmark configuration file, without the ".cfg" suffix,
i.e. everything following the ".cfx", stripped of directory specifiers.
For example, specifying "/path/to/script.cfx/path/to/benchname.cfg"
in the asim-run benchmarks file will yield @BENCHMARKNAME@ = "benchname",

=item @BENCHMARKPATH@

The full pah of the generated benchmark configuration file, without the ".cfg" suffix,
i.e. everything following the ".cfx" including the directory specifiers.
This will include the leading "/" if there are one or more directories, i.e.
"/path/to/script.cfx/path/to/benchname.cfg" will yield @BENCHMARKPATH@ = "/path/to/benchname",
but "/path/to/script.cfx/benchname.cfg" will yield @BENCHMARKPATH@ = "benchname".

=item @SOURCEDIR@

What follows will be resolved using awb-resolver to an absolute path in the workspace.
For example, "@SOURCEDIR@/path/to/file" will be translated into something like
"<workspace-location>/src/asim-foo/path/to/file" if the file is in package "foo".

=item @RUNSCRIPT@

The absolute path to the run script file generated by this tool.
Typically, the setup script will just copy this run script into the benchmark run directory,
which is usually the last argument given to the setup script.

=back 4

=cut

sub TemplateSubstitutions {
  my $text = shift;
  $text =~ s/\@BENCHMARKNAME\@/$SUBST_NAME/g;
  $text =~ s/\@BENCHMARKPATH\@/$SUBST_PATH/g;
  $text =~ s/\@RUNSCRIPT\@/$SUBST_RUNSCRIPT/g;
  $text =~ s/\@SOURCEDIR\@\/(\w+)/Asim::resolve($1)/eg;
  return $text;
}

################################################################

=head2 The ShiftPath() command

The ShiftPath() command is used to get "hidden" arguments passed to the .cfx script as part of
the virtual path.  For example, if you want to create a .cfx script that is used as follows:

=over 4

your/script.cfx/arg1/arg2/actual/path/to/benchmark.cfg

=back 4

then you would call GenCFG::Auto::ShiftPath() once to get "arg1", another time to get "arg2",
and then "/actual/path/to/benchmark" would be left over to be substituted as @BENCHMARPATH@.

=cut

sub ShiftPath {
  my @path    = split '/', $SUBST_PATH;
  my $ret     = shift @path;
  $SUBST_PATH = join  '/', @path;
  return $ret;
}

################################################################

=head2 The ChopName(<regex>) command

The ChopName() command is used to get "hidden" arguments passed to the .cfx script as part of
the virtual name.  For example, if you want to create a .cfx script that is used as follows:

=over 4

your/script.cfx/actual/path/to/benchmark@args.cfg

=back 4

then you would call GenCFG::Auto::ChopName( '@.*$' ) once to get "args" off the end of the name,
leaving "/actual/path/to/benchmark" to be substituted as @BENCHMARPATH@, and "benchmark" to be
subsituted as @BENCHMARKNAME@.

=cut

sub ChopName {
  my $regex = shift;
  $SUBST_NAME =~ s/($regex)//;
  return $1;
}

################################################################

=head2 The Directory(list) command

The Directory() command is used to specify a directory of possible benchmark configurations
that the calling script can generate.  This allows .cfx scripts to be used with AWB,
which uses --dir and --list commands to create a GUI display of the virtual directory
provided by a .cfx script.

The argument to Directory() is simply a list of benchmark configs that this script generates,
which may be a simple list of names, or a list of directory-structured names with "/" as
the directory separator.

=cut

sub Directory {
  @Directory = @_;
}

# generate and emit browsing information required by AWB
sub GenDirectory {
  my $gcfg = Asim::GenCFG->new();
  foreach my $full ( @Directory ) {
    my $name = basename( $full );
    my $dir  = dirname ( $full );
    my @tree = File::Spec->splitdir( $dir );
    if ( $dir eq '.' ) { @tree = (); }
    $gcfg->add(
      name   => $name,
      tree   => \@tree,
      info   => "benchmark configuration $full",
      feeder => 'archlib',
      setup  => 'bogus setup script to be filled in later'
    );
  }
  $gcfg->action(@ARGV);
}

################################################################

=head1 AUTHORS

Carl Beckmann

=head1 COPYRIGHT

Copyright (c) Intel Corporation, 2006-7

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

1;

