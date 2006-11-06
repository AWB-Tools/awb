#
# *****************************************************************
# *                                                               *
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


package PlotShell;
use warnings;
use strict;


sub help {
  my $sigpipe;
  if (defined $SIG{PIPE}) {
    $sigpipe = $SIG{PIPE};
  }
  $SIG{PIPE} = "IGNORE";
  
  if (defined $sigpipe) {
    $SIG{PIPE} = $sigpipe;
  }
  else {
    $SIG{PIPE} = "DEFAULT";
  }
#  open(HELP, "| $::MORE_CMD");
  open(HELP, ">/tmp/help$$");

  print HELP <<"END";

Usage:

  plot-shell [<plot-shell-command>]

Commands:

  ##############################################################################
  # Options for getting and printing data
  ##############################################################################
  set basedir <basedir>            - set default base dir for all exps

  set experiments <names>|<filename>|<regexp[/i]> 
  set benchmarks <names>|<filename>|'common'|<regexp[/i]>           
  set params <names>|<filename>|</regexp/[i]> IF YOU HAVE LIBXML PATCH
    ***** OR *****
  set params <names>|<filename>|<string*>|<*string*> IF YOU DO NOT HAVE LIBXML PATCH
 

                                   - There are a number of ways to provide 
                                     experiments, benchmarks, and params. 
                                   <names>: a space separated list of [experiment|
                                     benchmark|param] names
                                   <filename>: a file containing a list of 
                                     [experiments|benchmarks|params], one per line. 
                                   </regexp/>: A regular expression used to search
                                     available experiments, benchmarks or params. 
                                     For benchmarks and experiments any legal perl 
                                     regexp is legal. In the case of params, certain
                                     restritions apply: 

                                     1.) If libxml has patch that allows
                                         posix compatible regular expressions:

                                         o The regexp must be POSIX compatible.  
                                         o The optional 'i' suffix informs plot-shell 
                                           to be case insensitive.  

                                     2.) If libxml is NOT patched, then 
                                         params can only be supplied as follows:
                                         
                                         o <string>*
                                         o *<string>*
                                         o ILLEGAL: *<string>


          EX: set experiments /.*/

	      NOTE: This indicates that every directory under <basedir> should be
		    considered an experiments directory. 

          EX: set benchmarks /^mac.*|^tpcc.*/

              Get only the benchmarks which start with string 'mac' and string 'tpcc'

          EX: set experiments /tang_uni.*/

              Look at experiments that contain the string tang_uni.  For experiments
              that start with string tang_uni, specify:  /^tang_uni.*/
                    

          EX: WHEN LIBXML HAS BEEN PATCHED:
              set params /IPC/i /^Dcache.*/ RetiredInstructions

              The command above grabs all params containing string 'IPC',
              (regardless of case), all params starting with string Dcache and 
              the param 'RetiredInstructions'.  The <reg exp>/i command tells 
	      plot-shell to be case insensitive for the given regular expression.  

          EX: WHEN LIBXML HAS NOT BEEN PATCHED:
              set params *IPC* *ipc* *Ipc* Dcache* RetiredInstructions

              The glob expressions are case sensitive.  To get the same 
              behavior as the regular expression command, all reasonable 
              permutations of IPC are specified.  


  list stats options               - Show current values for gathering stats
  list params                      - Show current params.  If data has not yet
                                     been extracted, then it lists the params
				     specified by the user.  Otherwise, it lists
				     the parameters found in the stats files.  
  list scalars                     - Show current scalar params.
  list histograms                  - Show current histogram params.

  list benchmarks                  - Same as list params, but for benchmarks
  list experiments                 - Same as list params, but for experiments
  get data                         - Extract the data from the given set of 
                                     experiments, benchmarks and parameters
  print data [filename]            - Print data in some reasonable format to 
                                     stdout or filename.

  print restricted data [filename] - Print restricted data to stdout or filename.
                                     data is restricted via the 
				     'restrict <params|experiments|benchmarks>'
				     commands. If no restrictions are in place,
				     then this command reduces to the general 
				     print data command.  Use the command
				     'list restrictions' to determine if any 
				     restrictions are in place. 


  
  ##############################################################################
  # Options for sorting computing new data
  ##############################################################################

  ***
  *** WARNING: The following commands can only be called after running 'get data'
  ***

  sort params                      - The sort commands will cause the printing 
  sort benchmarks                    and plotting of data to be alphanumeric in  
  sort experiments                   order.  Note that if you used the 'all'
                                     option to set experiments and benchmarks, 
                                     then they appear sorted by default. 
  
  define params \$<new_param> = f(\$<param1>, \$<param2>, ....)
                                   - Define a new param with name new_param which is 
                                     computed based on a function (f) of the given
                                     parameters.  This command can only be used
                                     after the execution of 'get data'.

            ******************** WARNING *** WARNING *** WARNING ***************************
            ****                                                                        ****
            ****         Each param specified in equation must be preceded by a \$      ****
            ****         Params specified within special functions do not need to       ****
            ****         be preceded by a \$.                                           ****
            ****                                                                        ****
            ********************************************************************************

            Ex: Given parameters BP_Misses and BP_Accesses, 

                define params \$miss_rate = \$BP_Misses / \$BP_Accesses

            This will synthesize a new parameter called miss_rate, and
            compute the appropriate values. The synthesized parameter
            can now be plotted just like any other parameter. Note that 
            any any type of parameter can be specified on the right hand side
            of equation.  If the params used on RHS are histograms, then 
            computation is performed on each field of the histogram.  

            Any algebraic function can be used to compute new parameters. Also, 
            the following special functions are available....

  SPECIAL FUNCTIONS:

    NORMALIZE( exp=<experiment name>, bm=<benchmark name>, par=<parameter name>,
               norm_na=<value> ))

            INPUTS: Parameter name (required)
                    Experiment name (optional if benchmark is specified)
                    Benchmark name (optional if parameter is specified)
                    norm_na (optional): result value if normalization base
                                        does not exist (== *NA*); default = 0;

            PURPOSE: This function normalizes the value of the parameter 
                     specified to the value for a given experiment, benchmark,
                     or both.  If only a benchmark is specified, then the 
                     new param value is computed relative to the specified
                     benchmark within the same experiment.  If only an 
                     experiment is specified, then the new param value is
                     computed relative to the specified experiment for each
                     benchmark.  If both are specified, then the new param
                     value for every benchmark in every experiment is relative
                     to the benchmark and experiment specified. 

            EX: define params \$rel_IPC = NORMALIZE(exp=base, param=Overall_IPC)

                     This command computes the IPC for each benchmark relative
                     to the IPC for the same benchmark in the experiment called
                     'base'. 

            EX: define params \$rel_gcc = NORMALIZE (bm=gcc, par=Overall_IPC)
   
                     Compute the IPC of each benchmark in each experiment relative
                     to the IPC of gcc within each experiment.  For example, 
                     the IPC of tpcc for experiment base is computed relative to 
                     the IPC of gcc for experiment base, while the IPC of tpcc for
                     experiment foobar is computed relative to the IPC of gcc for 
                     experiment foobar. 

    PDF(<histogram parameter>)     
    
            INPUTS: Histogram parameter


            PURPOSE: Computes the probability distribution function for the 
                     elements in the histogram. 

            EX: define params \$pdf_issued = PDF(InstsIssuedPerCycle);

                     The histogram pdf_issued now contains information about
                     what percentage of time N instructions were issued per
                     cycle.  pdf_issued is the same size (rows and columns) as
                     InstsIssuedPerCycle. 

    CDF(<histogram parameter>)     
    
            INPUTS: Histogram parameter


            PURPOSE: Computes the cumulative distribution function for the 
                     elements in the histogram. 

            EX: define params \$cdf_issued = CDF(InstsIssuedPerCycle);

                     The histogram cdf_issued now contains information about
                     what accumulated percentage of time N instructions were 
                     issued percycle.  cdf_issued is the same size 
                     (rows and columns) as InstsIssuedPerCycle. 

    EXTRACT(rows=<row specification syntax>, cols=<col specification syntax>, 
            par=<param name>)

            INPUTS: 
                     o par = Histogram parameter
                     o rows = List of rows specified using the name of rows in histogram
                     o cols = List of cols specified using the name of cols in histogram
                     
            PURPOSE: To extract the given rows and/or cols from a histogram to create
                     a new histogram or scalar value.  If both the number of rows and the
                     number of cols extracted = 1, then the resulting value is a scalar. 

            Row|Col Specification Syntax:
                     o Individual rows (or cols) are specified using ':'

                       EX: To extract rows labled row1, row3, and row4, specifiy
                           rows=row1:row3:row4

                     o To extract a range of rows (or cols), use '..': 

                       EX: To extract rows row1, row3 through row14 inclusive, and row16
                           rows=row1:row3..row14:row16 


            EXAMPLES: 
                     o define params \$sub_hist = EXTRACT(rows=A:B:C..E:G, par=full_hist)
                       This extracts all rows labeled A, B, C through E, and G from 
                       histogram full_hist.  Since columns aren't specified, all columns
                       in full_hist are extracted. 

                     o define params \$sub_hist = EXTRACT(cols=Bouncing:Invalid, par=msgActivity)
                       Extract all columns labeled Bouncing or Invalid from histogram 
                       msgActivity.  All rows are extracted. 

                     o define params \$sub_hist = EXTRACT(cols=Bouncing, rows=Ring1, par=msgActivity)
                       Extract one element from histogram msgActivity located at row label Ring1, and
                       col label Bouncing.  The resulting param sub_hist will be a scalar value.

    COMBINE(</regexp/[i]|string>, </regexp/[i]|string>, ..., </regexp/[i]|string>)
 
            PURPOSE: Combines scalar or 1-dimensional histogram parameters to produce 
                     a single row, multiple column histogram (when inputs are scalar params),
                     or a 2-D histogram (when inputs are 1-D histograms).  
  
            INPUTS: One or more regular expression or string inputs.  Regular expressions are
                    expressed as /<perl reg exp string>/[i]. The optional 'i' suffix informs
                    plot-shell to ignore case. In the case of histogram inputs, the size and 
                    row and col labels of the histograms must match exactly.  

            EXAMPLES:
                    o define params \$new_hist = COMBINE(/DcacheFill.*|DcacheMiss.*/)
                      Combines scalar parameters that contain the string DcacheFill and 
                      DcacheMiss into new histogram. 
                    o define params \$new_hist = COMBINE(CPU_0_Overall_IPC, /^instsIssued$/i)
                      Combines the scalar params named CPU_0_Overall_IPC, and all scalar 
                      params that match string instsIssued regardless of case.  

    REDUCE(<op>, </regexp/[i]|string>, </regexp/[i]|string>, ... </regexp/[i]|string>)

            PURPOSE: Takes the parameters (scalar or histogram) that match the regular 
                     expression or explicit string given and operates on them with the
                     given op.  This function is especially useful when trying to combine 
                     data across multiple slices. As with all regular expression inputs, 
                     the /i features tells plot-shell to ignore case. 

            INPUTS: 
                    o <op>: Any algebraic operation (+, -, /, *)
                    o <Regular Expression|string>: Any Perl regular expression encased in /.../
                      or an explicit string. 

            EXAMPLES: 
                    o define params \$all_msgs = REDUCE(+, /.*msgActivity/)
                      Sum up all values for params containing the string msgActivity.
                      If the parameters in question are histograms, the resulting
                      values will also be a histogram. 
                 
                    o define params \$all_DcacheMisses = REDUCE(+, /retiredinstructions/i)
                      Sum up any parameters containing the string retiredinstructions 
                      (regardless of case). 
                      
  define benchmark \$<new_bm> = f(\$<bm1>, \$<bm2>, ....)
                                   - Define a new benchmark with name new_bm 
                                     which is computed based on a function (f) 
                                     of the given benchmarks bm1, bm2, ...
                                     This command can only be used
                                     after the execution of 'get data'. 

            This command computes the given function for all parameters
            specified with the 'set params' command.  

            EX: define benchmark \$gcc = \$gcc_00 + \$gcc_01

            This generates a new benchmark gcc that is a sum of the params
            for gcc_00 and gcc_01 specified by the 'set params' command.  
            The benchmark gcc is created for each experiment specified.  
            If a particular param is a histogram, then the funtion requested
            ('+' in this case) is computed entry by entry on the histogram. 
            For instance, 

                row R, col C in histogram H for gcc =
                       row R, col C for histogram H in gcc_00 +
                       row R, col C for histogram H in gcc_01 +


  ##############################################################################
  # PLOT FORMAT OPTIONS
  ##############################################################################
  set plot output <x11|eps|gif>    - What type of plot do we want. Filename
           <filename>                is required for non X11 outputs. 
  set plot legendloc               - Place legend at given point on plot
           <top|bottom|left|right>

  list plot legendloc              - List options for legend location

  set plot width [<value>]         - Set plot width in inches
                                     Width is reset to default if value is missing
  set plot height [<value>]        - Set plot height in inches
                                     Height is reset to default if value is missing
  set plot ymax [<value>]          - Provide a max Y value for plot
                                     Ymax is reset to default if value is missing
  set plot ymin [<value>]          - Provide a min Y value for plot
                                     If ymin >= ymax, then ymin will be
                                     set to the largest of ymax-1 or 0. 
                                     Ymin is reset to default if value is missing.


  set plot xlabel [<label>]        - Labels for X & Y axis and title.  If label or 
  set plot ylabel [<label>]          title is missing, then the default values
  set plot title [<title>]           are used.

  ##############################################################################
  # PLOT & PRINTING DATA OPTIONS
  #
  # WARNING: The following plot commands can only be called after executing 
  # 'get data'
  ##############################################################################
  set group <name> <values>        - Specify a synthetic group.  Values
                                     consist of a space separated list of 2 
                                     or more entries.  No regular expressions
                                     are allowed. 

            Ex: set group bpred bimodal gshare
            Ex: set group bpsize 4k 8k 16k 32k

  group experiments|benchmarks|params <name> 
        </regexp/[i]|string>, </regexp/[i]|string>, .... , </regexp/[i]|string>
                                   - Create a group of params|benchmarks|
                                     experiments using the given PERL regular 
                                     expression or string.  This is useful when 
                                     trying to isolate a set of entries without
                                     having to specify each name individually.
                                     Regular expressions syntax is: /<regexp>/[i].
                                     The /i syntax at the end of each reg exp 
                                     string is optional, and indicates that case
                                     should be ignored. 

           Ex: To only look at Dcache params

               group params dcache /dcache.*/i
               restrict params group[dcache]

           Ex: To only look at mcf and tpcc_020207a benchmarks

               group benchmarks mcf_tpcc /^mcf.*/ tpcc_020207a
               restrict benchmarks group[mcf_tpcc]

  restrict <data dim> <pattern>     - Synthesize a name for given dimension 
                                     using groups specified with the   
                                     'set group' command.  Synthesis
                                     pattern contains group names specified
                                     as group[....] and strings. Once this
                                     command has been called, the data dimension
                                     can no longer be used to map to a plotting
                                     dimension.  To map a plotting dimension 
                                     directly to a data dimension, run the 
                                     'unrestrict <data dimension>' command. 

            Ex: Given: Plot groups bpred and bpsize as specified above....

                restrict experiments group[bpsize] "_" group[bpred]

                This generates the experiment names 4k_bimodal, 4k_gshare, 
                8k_bimodal, 8k_gshare, etc....

            Ex: To limit the experiments plotted to 4k_bimodal

                restrict experiments "4k_bimodal"

  unrestrict <data dimension>      - Undo a given synthetic name for data
                                     dimension.  Once the data dimension is
                                     unrestricted, it can be directly mapped 
                                     to a plotting dimension.  


            Ex: unrestrict experiments

  set plot type <bar|box|line>     - Defines type of plot and provides
                <plot dim=data dim>  mapping from data dimensions to 
                                     plotting dimensions. 

                                   
                                     
            Plot Type   Plot Dimensions         Required Dimensions
            ---------------------------------------------------------------
            bar         bar, cluster, stack     bar
            box         bar, box, cluster       box, bar
            line        line, xaxis             xaxis

            Data Dimensions: 
                 Concrete:  experiments, benchmarks, params, rows, cols
                 Synthetic: Specified by command 'set group <name>'

            Ex: set plot type bar bar=benchmarks cluster=experiments
 
            Ex: set plot type box bar=experiments box=benchmarks
    
            Ex: set plot type box box=InstsIssuedPerCycle!rows bar=benchmarks
                NOTE: Here, the rows of a particular histogram is specified 
                      as the data dimension.  The syntax is 

                      <histogram param name>!<rows|cols>

            Ex: Given: The groups bpred and bpsize as specified

                       set group bpred bimodal gshare
                       set group bpsize 4k 8k 16k 32k

                and experiments are restrictd with ....

                       restrict experiments group[bpsize] "_" group[bpred]

                you can specify the following:

                       set plot type bar bar=bpred cluster=bpsize

                The following is ILLEGAL unless you first run 
                'unrestrict experiments'

                       ****ILLEGAL****
                       set plot type bar bar=experiments cluster=benchmarks 

  list plot dimensions             - List the available data dimensions.
                                     This command is only valid after the 
                                     execution of the 'extract data' command 

  list plot options                - Show current options for plotting stats
  list groups                      - Show available groups
  list restrictions                - Show specified restrictions of concrete 
                                     dimensions with synthetic ones.
  list mappings                    - List the mappings of plot to data dimensions
  plot data                        - Plot data to specified output (output
                                     specified by plot output)  

  plot scurve [line=<name>]        - This command plots data sorted by the value,
                                     and is only relevant to line plots.  If there
                                     is only one line, or if a line is not specified, 
                                     the data is sorted according to the first line
                                     in the data file.  If a line=<name> is specified, 
                                     then the data will be sorted according to the 
                                     values for <name>.

  plot ucurve [line=<name>]        - Similar to the plot scurve command, but plots a 
                                     ucurve.  All values below 1.0 are inverted in a
                                     ucurve. 

      EXAMPLE: 
               Experiments are "base" and "16kDCache", and we want to plot the
               IPC for the two experiments.  The following commands will plot 
               data sorted according to the IPC for experiments "base".  

               > set experiments base 16kDCache
               > ......... (a bunch more commands to extract data)
               > set plot type line xaxis=benchmarks line=experiments
               > plot scurve 
                 ......... to plot a ucurve
               > plot ucurve

               If we want to plot data sorted by experiment 16kDCache, then

               > set plot type line xaxis=benchmarks line=experiments
               > plot scurve line=16kDCache
                 .......... or
               > plot ucurve line=16kDCache


  ##############################################################################
  # Loop command
  ##############################################################################
  foreach <var name> [<list of values>] command1; command2; command3;....; commandN
                                   - This loops across the list of values given 
                                     and executes the commands. 

  EX: 

          foreach i [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15] \\ 
              restrict params SLICE_[i]::NumDirtyBytes; \\
              set plot type bar bar=SLICE_[i]::NumDirtyBytes!rows; \\
              set plot title SLICE_[i]::NumDirtyBytes; \\
              plot data

          This loops across slice numbers 0-15 and executes the commands given. 
          The result is 16 graphs, one per slice.  This foreach construct does 
          a simple loop and replace.  For every value of i, replace [i] with 
          that value. 

          NOTES: 

              o  The <list of values> provided are just strings.  In other words
                 no regular expression expansion is performed on the values.  

              o  The <var name> can be any alphanumeric string. 

              o  The commands are separated by a ";".  You can have one or more
                 commands.

              o  The "\\" tells plot-shell that the command extends over multiple 
                 lines. 
  

  ##############################################################################
  # Miscellaneous commands
  ##############################################################################

  dump history <filename>          - Dumps the history of the commands 
                                     executed in plot-shell to either filename,
                                     if specified, or stdout.
  ! <shell command>                - execute a shell command
  cd <dir>                         - cd to directory
  pwd                              - print current directory     
  help                             - display this message
  exit                             - exit shell
  quit                             - exit shell


  Note command completion and editing exists at the command prompt 
  and for most of the user input prompts. You may need to install
  the perl module Term::Readline::GNU for this to work...


Environment:

  AWBLOCAL
    If AWBLOCAL is set, it is used to set the default base directory. 

  SHELL
    The SHELL environment variable is used as the program to start
    for the "shell package" command.

  TMPDIR
    Various procedures in asim-shell need to create temporary files.
    TMPDIR points to the directory where these temporary files will be
    created in.

END
    

  #
  # If we don't do this, help breaks if you do not scroll past the first page. 
  # Read man perlfunc for info on 'close for pipes' if you want to know more. 
  #
  close (HELP);

  system ("cat /tmp/help$$ | $::MORE_CMD");
  system ("rm /tmp/help$$ > /dev/null 2>&1 &");


# Now that we're not directly piping to the more command, we don't need the following.
#  my $sigpipe;
#  if (defined $SIG{PIPE}) {
#    $sigpipe = $SIG{PIPE};
#  }
#  $SIG{"PIPE"} = 'IGNORE';
#  close(HELP);

  
#  if (defined $sigpipe) {
#    $SIG{PIPE} = $sigpipe;
#  }
#  else {
#    $SIG{PIPE} = "DEFAULT";
#  }

  return 1;
}



sub help_examples {
  open(HELP, "| $::MORE_CMD");

  print HELP <<"END";

Eventually there will be a collection of examples here, but not yet.
Getting started:


END

  close(HELP);
  return 1;
}


BEGIN {
  #
  # determine which pager program the user prefers; if none is set
  # we fall back to use "more"
  #
  if (defined $ENV{"PAGER"} && $ENV{"PAGER"} ne "") {
    $::MORE_CMD = $ENV{"PAGER"};
  } else {
    $::MORE_CMD = "more";
  }
}

1;
