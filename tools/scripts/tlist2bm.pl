#!/usr/intel/bin/perl

use lib '/nfs/site/proj/dpg/arch/perfhome/perllib';
use Trace_list;

use Getopt::Std;

#
#  Some default values
#
$sim_length = "variable_skip";


#
#  Save the command-line for later
#
$cmd_line = join ' ', ($0, @ARGV);

#
#  Process the command-line
#
getopts('ncl:dh');

$dont_check_traces = $opt_c;
$debug = $opt_d;


if ($opt_h or scalar(@ARGV)<1) {
    print "  USAGE:  $0 [options] <tracelist>\n\n";
    print "          Convert a PDX-style trace-list into an ASIM CFX-style benchmark list\n\n";
    print "          Options:\n";
    print "             -l <length>   Specify trace_length (default='$default_length')\n";
    print "             -n            Just dump the trace file names\n";
    print "             -c            Don't check to be sure that the trace file is available\n";
    print "             -d            Print debugging information\n";
    print "             -h            This help\n\n";

    exit;
}

if ($opt_l) {
    $sim_length = $opt_l;
}
$len_attrib = "SIMLEN_".$sim_length;

#==========================================================================================

$section_delim    = '%';
$subsection_delim = '@';
$space_delim      = ':';
$script = "tools/scripts/bm/tracecache/run-with-params.cfx/";

%have_trace = ();
%len_start_knob = ();
%len_end_knob = ();
%len_skip_knob = ();
%num_logical_threads = ();
%twolit_traces = ();

$infile = shift;

$list = new Trace_list;
$list->add_file($infile);

@pdx_traces = $list->fetch("ALL");

%asim_trace_map = ();
%skip_trace = ();


$num_pdx = scalar(@pdx_traces);
$count = 0;
foreach $pdx_t (@pdx_traces) {

    trace_info($pdx_t);

    if ($debug) {
        print STDERR "    Trace: $pdx_t\n";

        print STDERR "      length_start: $len_start_knob{$pdx_t}\n";
        print STDERR "      length_end:   $len_end_knob{$pdx_t}\n";
        print STDERR "      length_skip:  $len_skip_knob{$pdx_t}\n";
        print STDERR "      threads:      $num_logical_threads{$pdx_t}\n";

        if (my $t2 = $twolit_traces{$pdx_t}) {
            foreach my $tn (@{$t2}) { 
                print STDERR "      TWOLIT Trace: $tn\n";
            }
        }
    }


    #
    #  Convert the PDX full paths to local paths (option for simple tracenames later?)
    #
    if (my $tl_ref = $twolit_traces{$pdx_t}) {
        print STDERR "  Checking TWOLIT: $pdx_t\n" if $debug;

        foreach $tn (@{$tl_ref}) { 
            my $tracename = just_tracename($tn);
            my $cmd = "fetch-trace --nofetch $tracename 2>&1 | head -n 1";
            my ($t1) = `$cmd`;
            chomp $t1;
            print STDERR "    fetch-trace returned: $t1\n" if $debug;
            
            if ($t1 =~ /\# trace .* not found at/) {
                print STDERR "    WARNING: can't locate trace '$tracename'  Skipping $t\n";
                $skip_trace{$pdx_t} = 1;
                last;
            }
            else {
                $asim_trace_map{$tn} = $t1;
            }
        }
    }
    else {
        print STDERR "  Checking trace: $pdx_t\n" if $debug;

        my $tracename = just_tracename($pdx_t);
        my $cmd = "fetch-trace --nofetch $tracename 2>&1 | head -n 1";
        my ($t1) = `$cmd`;
        chomp $t1;
        print STDERR "    fetch-trace returned: $t1\n" if $debug;
            
        if ($t1 =~ /not found at/) {
            print STDERR "    WARNING: can't locate trace '$tracename'  Skipping $t\n";
            $skip_trace{$pdx_t} = 1;
        }
        else {
            $asim_trace_map{$pdx_t} = $t1;
        }
    }

    $count++;
    print STDERR "\rProcessed $count / $num_pdx";

    if ($debug) {
        print STDERR "-------------------------------------------------------\n";
    }

}
print STDERR "\n";


#
#  Let's make a note of how we got here
#
$num_skipped   = scalar keys %skip_trace;
$num_converted = $num_pdx - $num_skipped;
print "#---------------------------------------------------------------------------------------------\n";
print "#\n";
print "#  Converted to ASIM BM format with '$cmd_line'\n";
print "#  Traces configured for simulation length: '$len_attrib'\n";
print "#\n";
print "#  Converted $num_converted of $num_pdx traces\n";
if ($num_skipped) {
    print "#  (skipping $num_skipped TLIST trace(s) due to missing file):\n";
    foreach my $t (keys %skip_trace) {
        print "#      $t\n";
    }
}
print "#\n";
print "#---------------------------------------------------------------------------------------------\n";


#
#  Grab any leading comments out of the original file, and transfer them to the new.
#
open IN, "<$infile";
while ($line = <IN>) {
    if ($line =~ /^\#/) {
        print $line;
    }
}
close IN;

#
#  Convert these traces and their info into BM lines
#
#  Assume TLIST params are given in macro-instructions
#
print "\n";
foreach $pdx_t (@pdx_traces) {
    my $line = $script;

    # skip the ones we made a not of earlier
    if ($skip_trace{$pdx_t}) {
        print "# Skipping $pdx_t\n";

        next;
    }

    $feeder_flags = "";
    $model_flags = "";
    $system_flags = "";

    #
    #  The translation between ('start', 'skip', 'end') and ASIM knobs is based
    #  on the contents of the '/nfs/site/proj/dpg/arch/bin/create_willy_jobfile' script
    #
    if ($len_start_knob{$pdx_t} > 0) {
        $feeder_flags .= " -st ".$len_start_knob{$pdx_t};   # use feeder to skip until start pt
    }

    if ($len_end_knob{$pdx_t}) {
        $model_flags .= " -m ".$len_end_knob{$pdx_t};   # end after 'n' macro insts
    }

    if ($len_skip_knob{$pdx_t}) {
        $model_flags .= " -rsm ".$len_skip_knob{$pdx_t};  # reset stats after 'r' macro insts
    }

    if ($opt_n) {
        $line = "";
        $section_delim = "";
    }

    #
    #  Output trace-spec sections
    #

    $twolit_ref = $twolit_traces{$pdx_t};
    if (scalar(@{$twolit_ref})) {
        foreach my $p (@{$twolit_ref}) {
            $line .= $section_delim . $asim_trace_map{$p};
        }
    }
    else {
        $line .= $section_delim . $asim_trace_map{$pdx_t};
    }

    if (not $opt_n) {
        #
        #  Output params subsections
        #
        $line .= $section_delim;                      # Start of section
        
        # Build up subsection
        $line .= $subsection_delim . $model_flags;
        $line .= $subsection_delim . $feeder_flags;
        $line .= $subsection_delim . $system_flags;
        
        $line .= $subsection_delim . $section_delim;   # End of subsection & section

        $line .= ".cfg";
    }

    # get rid of any extraneous spaces, and add the CFG that ASIM needs
    $line =~ s/ /$space_delim/g;
    print "$line\n";
}

exit;

# trace_info() finds stuff out about a trace and set it in some globals..
#              It also returns the trace name so that the trace list can be created.
#
sub trace_info {
    ($_) = @_; 

    if (!($have_trace{$_})) {

	my($att);

        if (defined $ { $att = $list->attrib($_) }{'NTHREADS'}) {
            $num_logical_threads{$_} = $att->{'NTHREADS'};
	} else {
            $num_logical_threads{$_} = 1;
        }

        $have_trace{$_} = 1;

        if (defined $ { $att = $list->attrib($_) }{$len_attrib}) {
	
            $$att{$len_attrib} =~ s/_//g;
            
	    
            if ($$att{$len_attrib} =~ /(\d+);(\d+);(\d+)/) {
                
		$len_start_knob{$_} = $1;
                $len_end_knob{$_} = $3;
                $len_skip_knob{$_} = $2;
            } elsif ($$att{$len_attrib} =~ /(\d+);(\d+)/) {
                
		$len_start_knob{$_} = '';
                $len_end_knob{$_} = $2;
                $len_skip_knob{$_} = $1;
            } elsif ($$att{$len_attrib} =~ /^(\d+)$/) {
		$len_start_knob{$_} = '';
                $len_end_knob{$_} = $1;
                $len_skip_knob{$_} = '';
            } else {
		$len_start_knob{$_} = '';
                $len_end_knob{$_} = '';
                $len_skip_knob{$_} = '';
            }
        } else {
	    $len_start_knob{$_} = '';
            $len_end_knob{$_} = '';
            $len_skip_knob{$_} = '';
        }

        $att = undef;

        my $twolit_trace_variable_base = "TWOLIT_TRACE";
        my $twolit_trace_variable = 0;
        my $traces = 0;
        my @list;

        while (defined $ { $att = $list->attrib($_) }{$twolit_trace_variable_base.$twolit_trace_variable} ){
            push @list, $$att{$twolit_trace_variable_base.$twolit_trace_variable};
            $twolit_trace_variable++; 
            $traces++;
        }
        if ($traces != 0) {
            $twolit_traces{$_} = \@list;
        }

        $att = undef;

        if (defined $ { $att = $list->attrib($_) }{TRACE_TIME}) {
            $trace_times{$_} = $$att{TRACE_TIME};
        } else {
            $trace_times{$_} = 0;
        }

#==================================
#  Not used for ASIM runs...
#
#==================================
#        print $_."\n";
#        print ${$list->attrib($_)}{$command_attrib}."\n";

#        $att = undef;
#        if ($command_attrib && !$ignore_command_attrib
#	    && (defined $ { $att = $list->attrib($_) }{$command_attrib})) {
#            $command_knobs{$_} = $$att{$command_attrib};
#        } else {
#            $command_knobs{$_} = '';
#        }

#        $att = undef;

#        if ($golden_attrib 
#	    && (defined $ { $att = $list->attrib($_) }{$golden_attrib})) {
#            $golden_root{$_} = $$att{$golden_attrib};
#        } else {
#            $golden_root{$_} = '';
#        }
    }

    $_;
}


sub just_tracename {
    my $fullname = shift;

    my @parts = split /\//, $fullname;

    return $parts[$#parts];
}
