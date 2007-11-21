#
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


set AwbNamespace workbench

namespace eval workbench {
    namespace export AwbInit BatchProgress

    #
    # Benchmark commands that direct how to run the performance model
    #
    variable bmScript
    variable bmCmds

    #
    # The stop event (insts or cycles, and a time) that satisfies the
    # last "AwbRun" command, signaling us to read the next set of
    # benchmark commands.
    #
    variable bmStop
    
    #
    # File to dump stats to on exit.
    #
    variable dumponexitfile
    
    #
    # Available threads...
    #
    variable threads

    #
    # Starting simulation time
    #
    variable startTime

    #
    # Some static state for the progress messages so performance in the last
    # interval can be printed.
    #
    variable lastProgressTime
    variable lastProgressInstrs
    variable lastProgressMacroTime
    variable lastProgressMacroInstrs

    # Seed of the random number generator
    variable randomSeed

    # create a namespace for the public AWB command interface functions:
    # The idea here is to overlay the Awb* commands that AWB exports as
    # its external interface. The workbench::interface::Awb* version of these
    # functions will get executed when called externally (from the AWB 
    # command file). The workbench::Awb* verions of these commands will
    # be used internally to implement the actual functionality.
    namespace eval interface { }
}

###################################################################
#
# AwbMarker
# AwbRun
# AwbExit
# AwbProgress
# AwbSkip
# AwbSkipUntil  .... backward compatibility ... use markers now!
# AwbSample
# AwbStats
# AwbEvents
# AwbProtect
#
# These commands must be provided for benchmark configurations.
#
###################################################################
proc workbench::interface::AwbMarker { args } {
    variable ::workbench::bmCmds
    append bmCmds "{AwbMarker $args}\n"
}

proc workbench::interface::AwbRun { type amount } {
    variable ::workbench::bmCmds
    append bmCmds "{AwbRun $type $amount}\n"
}

proc workbench::interface::AwbExit {} {
    variable ::workbench::bmCmds
    append bmCmds "{AwbExit}\n"
}

proc workbench::interface::AwbSkip { uid inst {markerID -1}} {
    variable ::workbench::bmCmds
    append bmCmds "{AwbSkip $uid $inst $markerID}\n"
}

proc workbench::interface::AwbSkipUntil { uid when } {
    puts "***************** AwbSkipUntil usage deprecated! *****************"
    puts "Please use AwbMarker <tid> set <markerID> pc <addr>"
    puts "           AwbSkip <tid> marker <markerID>"
    puts "instead."
    puts "Note: if <addr> is a symbol, use \[PmSymbol find <tid> <symbol>\]"
    puts "      to look it up in the symbol table first."
    puts "******************************************************************"
    puts ""
    variable ::workbench::bmCmds
    set m 0
    if {[catch {expr $when} msg] == 0} {
        # "when" is numeric
        set addr $when
    } else {
        # "when" is not numeric
        set addr [PmSymbol find $uid $when]
    }
    append bmCmds "{AwbMarker $uid set $m pc $addr}\n"
    append bmCmds "{AwbSkip $uid 0 $m}\n"
} 

proc workbench::interface::AwbNewSkip { uid inst {markerID -1}} {
    variable ::workbench::bmCmds
    append bmCmds "{AwbNewSkip $uid $inst $markerID}\n"
}

proc workbench::interface::AwbBeginDrain {} {
    variable ::workbench::bmCmds
    append bmCmds "{AwbBeginDrain}\n"
}

proc workbench::interface::AwbEndDrain {} {
    variable ::workbench::bmCmds
    append bmCmds "{AwbEndDrain}\n"
}

proc workbench::interface::AwbSample { warmup run skip { iter forever } } {
    variable ::workbench::bmCmds
    set len [llength $skip]
    if { $len == 1 } {
        append bmCmds "{AwbSample $warmup $run $skip $iter}\n"
    } else {
        append bmCmds "{AwbSample $warmup $run {$skip} $iter}\n"
    }    
}

proc workbench::interface::AwbProgress { type period } {
    variable ::workbench::bmCmds
    append bmCmds "{AwbProgress $type $period}\n"
}

proc workbench::interface::AwbStats { onoffdump { file "stats.out" } } {
    variable ::workbench::bmCmds
    append bmCmds "{AwbStats $onoffdump $file}\n"
}

proc workbench::interface::AwbEvents { onofffilename { file "Events.drl.gz" } } {
    variable ::workbench::bmCmds
    append bmCmds "{AwbEvents $onofffilename $file}\n"
}

proc workbench::interface::AwbProtect { script } {
    variable ::workbench::bmCmds
    # remove comment lines
    #regsub -all {\s*#.*$} $script "" script
    # convert legal TCL script with CRs into legal TCL script with ";"s
    regsub -all "\n" $script "; " script
    append bmCmds "{AwbProtect {$script}}\n"
}

###################################################################
# support routines for AWB command scripts
###################################################################
proc workbench::interface::InstClass {name {opc 0}} {
  switch $name {
    callsys -
    syscall -
    systemcall {return "0x00000083 0xffffffff"}
    halt       {return "0x00000000 0xffffffff"}
    inta       {return "[expr 0x10 << 26] 0xfc000000"}
    intl       {return "[expr 0x11 << 26] 0xfc000000"}
    ints       {return "[expr 0x12 << 26] 0xfc000000"}
    intm       {return "[expr 0x13 << 26] 0xfc000000"}
    opc -
    opcode     {return "[expr $opc << 26] 0xfc000000"}
    default    {puts "InstClass: don't understand name \"$name\""; exit 1}
  }
}

    
###################################################################
#
# AwbInit
#
# Initialize awb for 'mode' (must be "batch" for this workbench).
#
###################################################################
proc workbench::AwbInit { win mode cmds {cmdsData "NULL"} } {
    variable bmScript
    variable bmCmds

    if { $mode != "batch" } {
        puts stdout "ERROR: Simple workbench only provides batch mode"
        exit 1
    }

    #
    # First of all, initialize the random number generator
    #
    RandomInit

    #
    # Read 'cmds' to get the benchmark commands
    #
    if { $cmds == "" } {
        set bmScript ""
        set bmCmds ""
    } else {
        if { $cmdsData == "NULL" } {
            set bmScript [ReadFile $cmds]
            if { $bmScript == "NULL" } {
                puts "ERROR: Unable to read awb command file $cmds"
                exit
            }
        } else {
            # data is supplied directly
            set bmScript $cmdsData
        }
        # old AWB command files have all Awb* commands wrapped in curly
        # braces, which makes the file not a legal TCL program - arghh!
        # get rid of these braces - note this is a heuristic only and might
        # not be perfect; if it breaks on your command file just remove the
        # braces by hand...
        regsub -all {{\s*(Awb[^\}]*)\s*}} $bmScript {\1} bmScript

        # now execute bmScript in the workbench::interface namespace
        # to generate the internal sequence of AWB commands in bmCmds
        namespace eval interface {
            if {[catch $::workbench::bmScript message]} {
                puts "Error executing awb command file"
                puts $message
                exit
            }
        }
    }

    set bmStop(TYPE) ""
    variable dumponexitfile ""

    variable startTime
    set startTime [clock seconds]
    variable lastProgressTime
    set lastProgressTime $startTime
    variable lastProgressInstrs
    set lastProgressInstrs 0
    variable lastProgressMacroTime
    set lastProgressMacroTime $startTime
    variable lastProgressMacroInstrs
    set lastProgressMacroInstrs 0
    
    #
    # If we have 'bmCmds', instruct the performance model to stop at the
    # beginning of cycle 0. Then we will start processing 'bmCmds'.
    # If we don't have any 'bmCmds' then we just start the model running.
    #
    if { $bmCmds == "" } {
        PmSchedule start
    } else {
        AwbRun cycle 0
    }
}


###################################################################
#
# ReadFile
#
# Open 'fn' for reading, and return it's contents.
#
###################################################################
proc workbench::ReadFile { fn } {
    #
    # If file does not exist or is a directory, then fail.
    #
    if { ![file exists $fn] || [file isdirectory $fn] } {
        return NULL
    }

    if [catch { open $fn r } fileId] {
        return NULL
    }

    set str [read $fileId]
    close $fileId

    return $str
}


###################################################################
#
# AwbMarker
# AwbRun
# AwbExit
# AwbProgress
# AwbSkip
# AwbSkipUntil
# AwbSample
# AwbStats
# AwbEvents
# AwbProtect
#
# These commands must be provided for benchmark configurations.
#
# now the real implementations of the exposed AWB commands
#
###################################################################
proc workbench::AwbMarker { tid cmd markerID subcmd args } {
    variable threads

    switch $cmd {
        set
          {
            if {$subcmd == "inst" && [llength $args] == 1} {
              # needs one level of unpacking
              set args [lindex $args 0]
            }
            eval PmMarker $cmd [ThdDesc $threads($tid)] $markerID $subcmd $args
          }
        clear
          {eval PmMarker $cmd [ThdDesc $threads($tid)] $markerID $subcmd $args}
        default
          { error "AwbMarker: Unknown cmd \"$cmd\", expecting set or clear." }
    }
}

proc workbench::AwbRun { type amount } {
    variable bmStop
    
    if { ($type != "cycle") && ($type != "inst") && ($type != "marker") && ($type != "nanosecond") } {
        error "AwbRun: Unknown run type \"$type\", expecting cycle, inst, nanosecond or marker."
    }
    
    if { $type == "cycle" } {
        set stopTime [expr [PmControl cycle] + $amount]
        PmSchedule stop cycle_once $stopTime
    } elseif { $type == "inst" } {
        set stopTime [expr [PmControl globalcommitted] + $amount]
        PmSchedule stop inst_once $stopTime
    } elseif { $type == "nanosecond" } {
        set stopTime [expr [PmControl nanosecond] + $amount]
        PmSchedule stop nanosecond_once $stopTime
    } elseif { $type == "marker" } {
        # we don't schedule a stop when we run to marker, since we have
        # no information as to when the marker will be reached
        set stopTime [expr [PmControl committedMarkers] + 1]
        PmControl commitWatchMarker $amount
    } else {
        error "workbench::Run: Unknown run type \"$type\", expecting cycle, inst, nanosecond or marker."
    }
    
    set bmStop(TYPE) $type
    set bmStop(TIME) $stopTime

    PmSchedule start
}

proc workbench::AwbExit { } {
    PmSchedule exit now
    PmSchedule start
}

proc workbench::AwbSkip { uid inst {markerID -1}} {
    set currTime [PmControl cycle]
    UnscheduleThread $uid cycle_once $currTime
    SkipThread $uid $inst $markerID cycle_once [expr $currTime + 1]
    ScheduleThread $uid cycle_once [expr $currTime + 2]
}


###################################################################
### LUK_FIX_SAMPLING:
## New procedures for implementing sampling:
##    AwbNewSkip, AwbBeginDrain, AwbEndDrain, ChangeThreadState
###################################################################
proc workbench::AwbNewSkip { uid inst {markerID -1}} {
    set currTime [PmControl cycle]
    ChangeThreadState $uid idle
    SkipThread $uid $inst $markerID cycle_once $currTime
    ChangeThreadState $uid running
}


proc workbench::AwbBeginDrain {} {
## This stops the performance model from fetching and switches on
## a flag that indicates being drained.
     PmControl beginDrain
}


proc workbench::AwbEndDrain {} {
## This resumes the fetching and switches off
## a flag that indicates being drained.
     PmControl endDrain
}

proc workbench::ChangeThreadState { uid newState } {
    variable threads

    #
    # If 'uid' == "all", then we change the state of all threads.
    #
    if { $uid == "all" } {
        foreach tn [array names threads] {
           SetThdState threads($tn) $newState
        }
    } else {
        if { ![info exists threads($uid)] } {
            error "ChangeThreadState: Don't know about thread $uid"
        }    
        SetThdState threads($uid) $newState
    }
}


proc workbench::AwbSample { warmup run skip { iter forever } } {
    variable bmCmds

    # Check if random skipping is wanted
    # 
    set len [llength $skip]

    if { $len == 2 } {
       # random sampling
       set min  [lindex $skip 0]
       set max  [lindex $skip 1]
       if {($min < 0) || ($max < 0) || ($min > $max)} {
          puts "ERROR: the range of random skipping is invalid\n"
          exit
       }
       set skipamount [Random $min $max]
    } else {
       # ordinary sampling
       if {$len != 1} {
          puts "ERROR: the skip argument of AwbSample must be either a single number or a list of 2 numbers in the form {min, max} for random skipping"
          exit
       }
       set skipamount $skip
    }    

    ### puts "\nAwbSample: skipamount = $skipamount\n"    
    #
    # Don't do anything if there is no sampling left to do.
    #
    if { (($warmup > 0) || ($run > 0) || ($skipamount > 0)) &&
         (($iter == "forever") || ($iter > 0)) } {
         #
         # Reinsert AwbSample command...
         #
         if { $iter != "forever" } {
             set iter [expr $iter - 1]
         }

         set bmCmds [linsert $bmCmds 0 [list AwbSample $warmup $run $skip $iter]]
    
         #
         # Insert new commands into 'bmCmds' to perform the
         # sampling steps. Since we're inserting into the beginning
         # of the commands we insert in reverse order.
         #
         if { $skipamount > 0 } {
### LUK_FIX_SAMPLING:
##             set bmCmds [linsert $bmCmds 0 [list AwbSkip all $skip]] ## do not use AwbSkip because it unschedules and re-schedules threads
#### In the following command, we drain the pipeling for 1000 cycles, which should be enough for Arana
               set bmCmds [linsert $bmCmds 0 [list AwbStats off] [list AwbBeginDrain] [list AwbRun cycle 1000] [list AwbNewSkip all $skipamount] [list AwbEndDrain]]  
         } 
         
         if { $run > 0 } {
             set bmCmds [linsert $bmCmds 0 [list AwbStats on] [list AwbRun inst $run] [list AwbStats off]]
               
         }

         if { $warmup > 0 } {
             set bmCmds [linsert $bmCmds 0 [list AwbStats off] [list AwbRun inst $warmup]]
         }
     }
}

proc workbench::AwbProgress { type period } {
    PmSchedule progress $type $period
}

proc workbench::AwbStats { onoffdump { file "stats.out" } } {
    switch $onoffdump {

        dump {
            PmState dump $file
        }

        dumponexit {
            variable dumponexitfile $file
        }
        
        on {
            PmState unsuspend
        }
        
        off {
            PmState suspend
        }
        
        default {
            error "workbench: Unknown enable type \"$onoffdump\", expecting on or off"
        }
    }
}

proc workbench::AwbEvents { onofffilename { file "Events.drl.gz" } } {
    switch $onofffilename {
    
        on {
            PmEvent on 
        }
        off {
            PmEvent off 
        }
        filename {
            PmEvent filename $file
        }
        default {
            error "workbench: Unknown enable type \"$onofffilename\", expecting on, off, or filename file_name"
        }
    }
}

###################################################################
# a'la Latex' protect for fragile commands. "script" is an arbitrary
# TCL script and can contain interactive Pm* commands; "script" should
# NOT, however, contain Awb* interface commands;
###################################################################
proc workbench::AwbProtect { script } {
    set isError [catch $script msg]

    if {$isError} {
        puts "Error executing AwbProtect $script\n"
        puts $msg
        exit
    } else {
        puts $msg
    }
}


###################################################################
#
# ProcessBmCmds
#
# Process and remove commands from 'bmCmds'. We process commands
# upto and including the first "AwbRun" command.
#
###################################################################
proc workbench::ProcessBmCmds { } {
    variable bmCmds
    variable bmStop

    set bmStop(TYPE) ""

    #
    # Don't use a foreach loop here sinve evaling 'cmd' can
    # change 'bmCmds'.
    #

    while { [llength $bmCmds] > 0 } {
        set cmd [lindex $bmCmds 0]
        set bmCmds [lreplace $bmCmds 0 0]

        eval $cmd
        if { [lindex $cmd 0] == [string trim "AwbRun"] ||
             [lindex $cmd 0] == [string trim "AwbExit"] } {
            return
        }
    }

    #
    # start interactive AWB shell
    #
    # see if we can get interactive commands
    set stdout_buffering [fconfigure stdout -buffering]
    fconfigure stdout -buffering none
    puts -nonewline "awbsh> "
    while {[gets stdin cmd]} {
        catch $cmd msg
        puts "$msg"
        if { [lindex $cmd 0] == [string trim "AwbRun"] ||
             [lindex $cmd 0] == [string trim "AwbExit"] } {
            break
        }
        puts -nonewline "awbsh> "
    }
    fconfigure stdout -buffering $stdout_buffering
    # make sure we get control back if no stop event is scheduled yet
    if { $bmStop(TYPE) == "" } {
        PmSchedule start
        PmSchedule stop now
    }
}


###################################################################
#
# CheckBmCmds
#
# Check to see if we should process the next set of BmCmds.
#
###################################################################
proc workbench::CheckBmCmds { } {
    variable bmStop
    
    if { $bmStop(TYPE) == "cycle" } {
        if { [PmControl cycle] == $bmStop(TIME) } {
            ProcessBmCmds
        }
    } elseif { $bmStop(TYPE) == "inst" } {
        if { [PmControl globalcommitted] >= $bmStop(TIME) } {
            ProcessBmCmds
        }
    } elseif { $bmStop(TYPE) == "marker" } {
        if { [PmControl committedMarkers] >= $bmStop(TIME) } {
            ProcessBmCmds
        }
    } elseif { $bmStop(TYPE) == "nanosecond" } {
        if { [PmControl nanosecond] >= $bmStop(TIME) } {
            ProcessBmCmds
        }
    } else {
        ProcessBmCmds
    }
}


###################################################################
#
# BatchProgress
#
# Called when we are in batch mode to process the pending progress
# events.
#
###################################################################
proc workbench::BatchProgress { progs } {
    variable dumponexitfile

    #
    # Process each progress event.
    #
    foreach { progress args } $progs {
        switch $progress {
            cycle -
            nanosecond -
            macroinst -
            inst {
                ShowProgress
            }
            
            start {
            }
            
            stop {
                #
                # Check to see if we need to process any 'bmCmds'.
                #
                CheckBmCmds
            }
                    
            threadbegin {
                ThreadBegin [lindex $args 0] [lindex $args 1]
            }
            
            threadend {
                ThreadEnd [lindex $args 0] [lindex $args 1]
            }
            threadblock {
#				nothing here yet!
                puts "THREADBLOCK\n"
            }
            threadunblock {
#				nothing here yet!
                puts "THREADUNBLOCK\n"
            }
            
            exit {
                if { $dumponexitfile != "" } {
                    PmState dump $dumponexitfile
                    # tell whoever may concern that TCL wants to exit now
                    if { [info commands TclExit] != "" } {
                        TclExit
                    }
                }
            }
            
            default {
                error "workbench::BatchProgress: Unexpected progress type $progress"
            }
        }
    }
}


###################################################################
#
# ShowProgress
#
# Write mode progress to stdout
#
###################################################################
proc workbench::ShowProgress { } {
    # Print a global summary
    set str [format "Time: %.0f ns (%.2f ns/s), Total Instrs: %.0f (%.1f i/s) MacroInstrs: %.0f (%.1f mi/s)" [PmControl nanosecond] [Nps] [TotalInstrs] [Ips] [TotalMacroInstrs] [Mips]]
    puts "$str"

    set ncpus [PmControl numcpus]
    for {set i 0} {$i < $ncpus} {incr i} {
        set is_active [PmControl active $i]
        if {$is_active == 1} {
            set ipc [Ipc $i]
            set mipc [MIpc $i]
            if {$ipc == $mipc} {
                set str [format "CPU %3d:  Cycle: %.0f \tIPC: %.4f \tCPS: %.0f" $i [PmControl cpucycle $i] $ipc [Cps $i]]
            } else {
                set str [format "CPU %3d:  Cycle: %.0f \tIPC: %.4f \tM_IPC: %.4f \tCPS: %.0f" $i [PmControl cpucycle $i] $ipc $mipc [Cps $i]]
            }
            puts "$str"
        }
    }
}


###################################################################
#
# Cps
#
# Return the current cycles per second of the model
#
###################################################################
proc workbench::Cps { cpunum } {
    variable ::workbench::startTime

    set tim [clock seconds]
    set cyc [PmControl cpucycle $cpunum]
    set dif [expr $tim - $startTime]

    if { $dif == 0 } {
        return 0
    } else {
        return [expr $cyc.0 / $dif.0]
    }
}


###################################################################
#
# Nps
#
# Return the current nanoseconds per second of the model
#
###################################################################
proc workbench::Nps { } {
    variable ::workbench::startTime

    set tim [clock seconds]
    set nan [PmControl nanosecond]
    set dif [expr $tim - $startTime]

    if { $dif == 0 } {
        return 0.0
    } else {
        return [expr $nan.0 / $dif.0]
    }
}

###################################################################
#
# Ipc
#
# Return the current ipc
#
###################################################################
proc workbench::Ipc { cpunum } {
    set cyc [PmControl cpucycle $cpunum]
    set ret [PmControl committed $cpunum]
    
    if { $cyc == 0 } {
        return 0.0
    }
    
    return [expr $ret.0 / $cyc.0]
}


###################################################################
#
# MIpc
#
# Return the current macro instruction ipc
#
###################################################################
proc workbench::MIpc { cpunum } {
    set cyc [PmControl cpucycle $cpunum]
    set ret [PmControl committedmacros $cpunum]
    
    if { $cyc == 0 } {
        return 0.0
    }
    
    return [expr $ret.0 / $cyc.0]
}


###################################################################
#
# TotalInstrs
#
# Return the number of instrs committed on all CPUs
#
###################################################################
proc workbench::TotalInstrs { } {
    set ninstrs 0.0
    set ncpus [PmControl numcpus]
    for {set i 0} {$i < $ncpus} {incr i} {
        set is_active [PmControl active $i]
        if {$is_active == 1} {
            set comm [PmControl committed $i]
            set ninstrs [expr $ninstrs + $comm.0]
        }
    }
    
    return $ninstrs
}


###################################################################
#
# TotalInstrs
#
# Return the number of instrs committed on all CPUs
#
###################################################################
proc workbench::TotalMacroInstrs { } {
    set ninstrs 0.0
    set ncpus [PmControl numcpus]
    for {set i 0} {$i < $ncpus} {incr i} {
        set is_active [PmControl active $i]
        if {$is_active == 1} {
            set comm [PmControl committedmacros $i]
            set ninstrs [expr $ninstrs + $comm.0]
        }
    }
    
    return $ninstrs
}


###################################################################
#
# Ips
#
# Return the rate at which instructions are completed.  The rate
# is since the last time Ips was called, not since the beginning
# of the run.  This lets us get a reasonable dynamic view of
# instrs/s and lets us factor out time spent in warm-up or loading
# a checkpoint.
#
###################################################################
proc workbench::Ips { } {
    variable ::workbench::lastProgressTime
    variable ::workbench::lastProgressInstrs

    set tim [clock clicks -milliseconds]
    set dif [expr $tim - $lastProgressTime]
    set lastProgressTime $tim

    set instrs [TotalInstrs]
    set newinstrs [expr $instrs - $lastProgressInstrs]
    set lastProgressInstrs $instrs

    if { $dif == 0 } {
        return 0.0
    } else {
        return [expr (1000.0 * $newinstrs) / $dif.0]
    }
}


###################################################################
#
# Mips
#
# Return the rate at which macro instructions are completed.  The rate
# is since the last time Ips was called, not since the beginning
# of the run.  This lets us get a reasonable dynamic view of
# instrs/s and lets us factor out time spent in warm-up or loading
# a checkpoint.
#
###################################################################
proc workbench::Mips { } {
    variable ::workbench::lastProgressMacroTime
    variable ::workbench::lastProgressMacroInstrs

    set tim [clock clicks -milliseconds]
    set dif [expr $tim - $lastProgressMacroTime]
    set lastProgressMacroTime $tim

    set instrs [TotalMacroInstrs]
    set newinstrs [expr $instrs - $lastProgressMacroInstrs]
    set lastProgressMacroInstrs $instrs

    if { $dif == 0 } {
        return 0.0
    } else {
        return [expr (1000.0 * $newinstrs) / $dif.0]
    }
}


###################################################################
#
# Thread functions
#
# The following functions control schedule, unscheduling, skipping,
# etc. of threads. These are likely common to all workbenches
# and could be shared.
#
###################################################################

###################################################################
#
# ThdUid
# ThdDesc
# ThdState
#
# Accessor functions to access parts of a thread
#
###################################################################
proc workbench::ThdUid { thd } { lindex $thd 0 }
proc workbench::ThdDesc { thd } { lindex $thd 1 }
proc workbench::ThdState { thd } { lindex $thd 2 }


###################################################################
#
# ThdNew
#
# Constructor for a thread
#
###################################################################
proc workbench::ThdNew { uid desc state } {
    return [list $uid $desc $state]
}


###################################################################
#
# SetThdState
#
# Modifiers for a thread
#
###################################################################
proc workbench::SetThdState { thdi state } {
    upvar $thdi thread

    set thread [lreplace $thread 2 2 $state]
}


###################################################################
#
# ThreadBegin
#
# Record that a new thread is beginning
#
###################################################################
proc workbench::ThreadBegin { uid tdesc } {
    variable threads
    
    if [info exists threads($uid)] {
        error "Multiple threadbegins for same thread uid"
    }

    set threads($uid) [ThdNew $uid $tdesc idle]

    #
    # For now we just immediately schedule the thread.
    #
    ScheduleThread $uid now 0
}


###################################################################
#
# ThreadEnd
#
# Record that a thread is ending
#
###################################################################
proc workbench::ThreadEnd { uid tdesc } {
    variable threads
    
    if { ![info exists threads($uid)] } {
        error "Got threadend for thread without previous threadbegin"
    }

    #
    # For now we just immediately unschedule the thread.
    #
    UnscheduleThread $uid now 0

    unset threads($uid)

    #
    # If there aren't any threads left, then we exit.
    #
    if { [array size threads] == 0 } {
        AwbExit
    }
}


###################################################################
#
# ScheduleThread
#
# Schedule thread 'uid' on the performance model.
#
###################################################################
proc workbench::ScheduleThread { uid trigger time } {
    variable threads

    #
    # If 'uid' == "all", then we schedule all idle threads.
    #
    if { $uid == "all" } {
        foreach tn [array names threads] {
            if { [ThdState $threads($tn)] == "idle" } {
                PmSchedule schedthread [ThdDesc $threads($tn)] $trigger $time
                SetThdState threads($tn) running
            }
        }
    } else {
        if { ![info exists threads($uid)] } {
            error "ScheduleThread: Don't know about thread $uid"
        }
    
        PmSchedule schedthread [ThdDesc $threads($uid)] $trigger $time
        SetThdState threads($uid) running
    }
}


###################################################################
#
# UnscheduleThread
#
# Remove thread 'uid' from the performance model.
#
###################################################################
proc workbench::UnscheduleThread { uid trigger time } {
    variable threads

    #
    # If 'uid' == "all", then we unschedule all running threads.
    #
    if { $uid == "all" } {
        foreach tn [array names threads] {
            if { [ThdState $threads($tn)] == "running" } {
                PmSchedule unschedthread [ThdDesc $threads($tn)] $trigger $time
                SetThdState threads($tn) idle
            }
        }
    } else {
        if { ![info exists threads($uid)] } {
            error "UnscheduleThread: Don't know about thread $uid"
        }
    
        PmSchedule unschedthread [ThdDesc $threads($uid)] $trigger $time
        SetThdState threads($uid) idle
    }
}


###################################################################
#
# SkipThread
#
# Skip thread 'uid', 'inst' instructions.
#
###################################################################
proc workbench::SkipThread { uid inst markerID trigger time } {
    variable threads
    
    #
    # If 'uid' == "all", then we find all the 'n' idle threads, and
    # skip each 'inst'/'n' instructions
    #
    if { $uid == "all" } {
        set idlecnt 0
        foreach tn [array names threads] {
            if { [ThdState $threads($tn)] == "idle" } {
                incr idlecnt
            }
        }

        if { $idlecnt != 0 } {
#         puts "\nSkipThread: call skipthread all\n"
            PmSchedule skipthread "all" [expr $inst / $idlecnt] $markerID $trigger $time
        }
    } else {
        if { ![info exists threads($uid)] } {
            error "SkipThread: Don't know about thread $uid"
        }
    
        PmSchedule skipthread [ThdDesc $threads($uid)] $inst $markerID $trigger $time
    }
}

######################################################################
# Procedures related to random number generation. Required for
# skipping random number of instructions in AwbSample.
# The following procedures are based on those on page 78 of the
# book "Practical Programming in Tcl and Tk, 2nd Edition".
######################################################################
proc workbench::RandomInit { } {
     variable randomSeed 
     set randomSeed 511
}

proc workbench::Random { min  max } {
     variable randomSeed
     # generate a new seed
     set randomSeed [expr ($randomSeed*9301 + 49297) % 233280]
     # generate a random value based on the new seed
     set rval [expr $randomSeed/double(233280)]
     # map the random value to the range
     set mapval [expr ($max - $min) * $rval + $min]
     # trancate to integer
     set retval [expr int($mapval)]
     return $retval
}
