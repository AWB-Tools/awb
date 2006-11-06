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

#############################################################
#
# Miss. functions
#
#############################################################


#############################################################
# Name: AskUser
# Description: Generic function which asks users which 
#               specific benchmarks they want to look at.
#               Acceptable answers are all, or a specific
#                benchmark. 
#############################################################
AskUser <- function (BENCHMARKS) {
  if (interactive() == TRUE) {
    cat("BENCHMARKS ARE:\n")
    for (i in BENCHMARKS) {
      cat(i)
      cat("\n")
    }
    cat("Specific benchmark? (<benchmark name> or \"all\" or <carriage return>)", ": ", sep="")
    reply <- scan(what=character(0), n=1, quiet=TRUE)
  }
  else {
    return (c())
  }

  if (is.character(reply) && length(reply) != 0 && 
     (reply != "n" || reply == "N")) { 
    if (reply == "all" || reply == "ALL" || reply == "All") {
      return (BENCHMARKS)
    }
    else {
      goodbm <- grep (reply, BENCHMARKS) 
      if (length(goodbm) != 0) {
        return (reply)
      }
    }
  }
  return (c())
}

############################################################
############################################################
##################                   #######################
##################RESOURCE HISTOGRAMS#######################
##################                   #######################
############################################################
############################################################
############################################################


#############################################################
# Name: GenericResourcePlot
# Description: Generic function to do all resource plots. 
#               X-axis represents the number of entries in 
#               resource, and Y-axis shows pecentage of time. 
# Plots: 
#  1.) Bar Plot of INTmean and FPmean  on same chart. 
#  2.) Line plot of all INT benchmarks, including INTmean. 
# 3.) Line plot of all FP benchmarks, including FPmean
#############################################################
GenericResourcePlot <- function (PARAM, XTITLE, YTITLE, TITLE,
    DATA = "norm", INPATH = "./", OUTPATH = INPATH, LEGLOC=NE, 
    INTBM=INTBM, FPBM=FPBM) {

  # Plot the Integer values. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="LINE", TITLES=c(XTITLE, YTITLE, TITLE), 
    COMBINENAME="INT", LEGLOC=LEGLOC)

  # Plot FP benchmarks. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="LINE", TITLES=c(XTITLE, YTITLE, TITLE),
    COMBINENAME="FP", LEGLOC=LEGLOC)

  # Plot the means. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c("INTmean", "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, 
    COMBINE=TRUE, STACKED=FALSE, TITLES=c(XTITLE, YTITLE, TITLE),
    COMBINENAME="MEAN", LEGLOC=LEGLOC)
}

#############################################################
# Name: GenericResourceMunge
#  Type: Internal Function.  Should never be called directly. 
# Purpose: This function takes an INPATH, and a PARAM, 
#           and reads and munges the data and returns a lst.  
#           The way it munges the parameters is specific to 
#           RESOURCE Utilization data. 
# Inputs:
#    PARAM, INPATH, OUTPATH, INT benchmarks, FP benchmarks
# Outputs:
#    List of data frames, one per benchmark.
#############################################################
GenericResourceMunge <- function (PARAM, INPATH, OUTPATH, INT, FP, LEGLOC=NEW) {
  cat ("\nPARAM="); print (PARAM)
  warning("GENERATING PARAM.norm FILE")
  return (GenericMungeData(PARAM, INPATH=INPATH, OUTPATH=OUTPATH, 
        INTBENCHMARKS=INT, FPBENCHMARKS=FP))
}

#############################################################
# Name: inumsInFlight
# Description: Number of INUMS In Flight during any cycle
#############################################################
inumsInFlight <- function (DATA = c(), INPATH = "./", OUTPATH = INPATH, 
                INTBM=INT95, FPBM=FP95) {
  param <- "inumsInFlight"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  # Initialize all variables
  title <- "Number of Inums In Flight"
  xtitle <- "Number of Inums"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, LEGLOC=NW, 
    INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# Name: QueueChunkUtilization
# Description: Queue utilization
#############################################################
QueueChunkUtilization <- function (DATA = c(), INPATH = "./", OUTPATH = INPATH, 
                INTBM=INT95, FPBM=FP95) {
  param <- "QueueChunkUtilization"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  # Initialize all variables
  title <- "Queue Utilization By Chunks"
  xtitle <- "Number of Entries in Queue"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, LEGLOC=N,
    INTBM=INTBM, FPBM=FPBM)
}
  
#############################################################
# Name: LowerQueueInstUtilization
# Description: Lower Queue Inst Utilization
#############################################################
LowerQueueInstUtilization <- function (DATA = c(), INPATH = "./", 
          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "LowerQueueInstUtilization"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Lower Queue Utilization By Instructions"
  xtitle <- "Number of Insts in Lower Queue"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
    INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# Name: UpperQueueInstUtilization
# Description: Upper Queue Inst Utilization
#############################################################
UpperQueueInstUtilization <- function (DATA = c(), INPATH = "./", 
          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "UpperQueueInstUtilization"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Upper Queue Utilization By Instructions"
  xtitle <- "Number of Insts in Upper Queue"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
    INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# Name: QueueInstUtilization
# Description: Queue Inst Utilization
#############################################################
QueueInstUtilization <- function (DATA = c(), INPATH = "./", 
                OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "QueueInstUtilization"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Queue Utilization By Instructions"
  xtitle <- "Number of Insts in Queue"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
    INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# Name: QueueChunkDifference
# Description: Difference in the number of Chunks between Upper and Lower Queue
#############################################################
QueueChunkDifference <- function (DATA = c(), INPATH = "./", 
                OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "QueueChunkDifference"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Difference in Chunks Between Upper and Lower Queues"
  xtitle <- "Difference in Chunks"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
    INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# Name: LowerQueueChunkUtilization
# Description: Lower Queue Chunk Utilization
#############################################################
LowerQueueChunkUtilization <- function (DATA = c(), INPATH = "./", 
                        OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "LowerQueueChunkUtilization"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Lower Queue Utilization By Chunks"
  xtitle <- "Number ofC hunks in Lower Queue"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
    INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# Name: UpperQueueChunkUtilization
# Description: Upper Queue Chunk Utilization
#############################################################
UpperQueueChunkUtilization <- function (DATA = c(), INPATH = "./", 
                OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "UpperQueueChunkUtilization"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Upper Queue Utilization By Chunks"
  xtitle <- "Number of Chunks in Upper Queue"
  ytitle <- "Percent of Total Execution Time" 

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
    INTBM=INTBM, FPBM=FPBM)
}

##############################################################
# Name: All Resource Stats
# Description: Uses the generic function to plot all resource
#               stats
##############################################################
PlotAllResourceStats <- function(INPATH="./", OUTPATH = INPATH) {
  QueueChunkUtilization(INPATH=INPATH, OUTPATH=OUTPATH)
  UpperQueueChunkUtilization(INPATH=INPATH, OUTPATH=OUTPATH)
  LowerQueueChunkUtilization(INPATH=INPATH, OUTPATH=OUTPATH)
  QueueInstUtilization(INPATH=INPATH, OUTPATH=OUTPATH)
  LowerQueueInstUtilization(INPATH=INPATH, OUTPATH=OUTPATH)
  UpperQueueInstUtilization(INPATH=INPATH, OUTPATH=OUTPATH)
  QueueChunkDifference(INPATH=INPATH, OUTPATH=OUTPATH)
}

##########################################################
##########################################################
##################                          ##############
##################BY INST TYPE VS. TPUSTATS ##############
##################                          ##############
##########################################################
# These functions should be used for any histogram where 
#  the rows or columns are instruction types, and the
# other dimension is the TPU number. 
##########################################################
##########################################################

#############################################################
# Name: GenericInstTypeTpuPlot
# Description: Generic function to do all histograms where 
#               the rows or columns represent instruction
#               types.  
# Plots: 
#    One plot of INTmean and FPmean with X-Axis being the 
#    inst type. 
#############################################################
GenericInstTypeTpuPlot <- function (PARAM, YTITLE, TITLE,
    DATA = "norm", INPATH = "./", OUTPATH = INPATH, BYROW=TRUE, 
    INTBM=INT95, FPBM=FP95) {

  # Plot INTmean
  GenericPlotData(PARAM, DATALST=DATA, 
    BENCHMARKS=c("INTmean", "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=BYROW, 
    TITLES=c("", YTITLE, TITLE),   LEGLOC=NE)

  # Check about benchmarks with user.
  bm <- AskUser(c(INTBM, FPBM))

  while (length(bm) != 0) {
    GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=bm, 
      INPATH=INPATH, OUTPATH=OUTPATH, BYROW=BYROW, 
      TITLES=c("", YTITLE, TITLE),   LEGLOC=NE)
    
    bm <- AskUser(c(INTBM, FPBM))
  }
}

#############################################################
# Name: GenericInstTypeTpuMunge
#  Type: Internal Function.  Should never be called directly. 
# Purpose: This function takes an INPATH, and a PARAM, 
#           and reads and munges the data and returns a lst.  
#           The way it munges the parameters is specific to 
#           histograms where inst-type is the columns or rows. 
# Inputs:
#    PARAM, INPATH, OUTPATH, INT benchmarks, FP benchmarks
# Outputs:
#    List of data frames, one per benchmark.
#############################################################
GenericInstTypeTpuMunge <- function (PARAM, INPATH, OUTPATH, INT, FP, NORM=TRUE) {
  cat ("\nPARAM="); print (PARAM)
  warning("GENERATING PARAM.norm FILE")
  return (GenericMungeData(PARAM, INPATH=INPATH, OUTPATH=OUTPATH, 
                              INTBENCHMARKS=INT, FPBENCHMARKS=FP, NORM=NORM))
}

#############################################################
# Name: Src1ProducerForFloads
# Purpose: Producer of source value for FP Loads
#############################################################
Src1ProducerForFloads <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "Src1ProducerForFloads"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, 
      OUTPATH, INTBM, FPBM)
  }
    
  title <- "FP Loads Producer Categorized by Inst Type"
  ytitle <- "Percent of All FP Load Source Register"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: Src1ProducerForIloads
# Purpose: Producer of source value for INT Loads
#############################################################
Src1ProducerForIloads <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "Src1ProducerForIloads"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, 
      OUTPATH, INTBM, FPBM)
  }
    
  title <- "INT Loads Producer Categorized by Inst Type"
  ytitle <- "Percent of All INT Loads"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: timeFromIqToIssue
# Purpose: Plot time from IQ to Issue of inst by inst type
#############################################################
timeFromIqToIssue <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "timeFromIqToIssue"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, 
      OUTPATH, INTBM, FPBM)
  }
    
  title <- "Cycles From IQ Entry To Inst Issue"
  ytitle <- "Percent of All Instructions Issued"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE)
}

#############################################################
# Name: timeFromMappedToIssue
# Purpose: Plot time from Inst Map to Inst Issue by inst type
#############################################################
timeFromMappedToIssue <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "timeFromMappedToIssue"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Cycles From Inst Mapped To Inst Issue"
  ytitle <- "Percent of All Instructions Issued"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: timeFromDataRdyToIssue
# Purpose: Plot time from Inst Map to Inst Issue by inst type
#############################################################
timeFromDataRdyToIssue <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "timeFromDataRdyToIssue"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Cycles From Inst Ready To Inst Issue"
  ytitle <- "Percent of All Instructions Issued"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: CommittedInstructions
# Purpose: Plot chart of committed instruction stream 
#    as a function of the instruction type.  
#    For this histogram, rows are instruction types, and
#    cols are TPUs.
#############################################################
CommittedInstructions <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "CommittedInstructions"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Committed Instructions"
  ytitle <- "Percent of Total Committed Instructions" 

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: UnCommittedInstructions
# Purpose: Plot chart of uncommitted instruction stream 
#    as a function of the instruction type.  
#    For this histogram, rows are instruction types, and
#    cols are TPUs.
#############################################################
UnCommittedInstructions <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "UnCommittedInstructions"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Non Committed Instructions"
  ytitle <- "Percent of Total Non Committed Instructions" 

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: UnIssuedDataRdyInstByType
# Purpose: Plot the histogram which shows how many instructions
#    of what type went unissued. 
#############################################################
UnIssuedDataRdyInstByType <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "UnIssuedDataRdyInstByType"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }
    
  title <- "Non Issued Data Ready Instructions"
  ytitle <- "Percent of Total Non Issued Instructions" 

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: successfulFollowMeAttempts
# Purpose: Plot the histogram which shows how many instructions
#    of what type went unissued. 
#############################################################
successfulFollowMeAttempts <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "successfulFollowMeAttempts"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM, NORM=FALSE)
  }
    
  title <- "Successful Instances of Follow-Me Classified by Instruction"
  ytitle <- "Successful Follow-Me Attempts"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}

#############################################################
# Name: failedFollowMeAttempts
# Purpose: Plot the histogram which shows how many follow
#    me attempts failed by inst type. 
#############################################################
failedFollowMeAttempts <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95) {
  # Initialize all variables
  param <- "failedFollowMeAttempts"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, OUTPATH, INTBM, FPBM, NORM=FALSE)
  }
    
  title <- "Failed Instances of Follow-Me Classified by Instruction Type"
  ytitle <- "Failed Follow-Me Attempts"

  GenericInstTypeTpuPlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH)
}


####################################################
####################################################
##################           #######################
##################BASIC STATS#######################
##################           #######################
####################################################
# These functions should be used for plotting      #
# non-histogram statistics.                        #
####################################################
####################################################

#############################################################
# Name: GenericSimplePlot
# Description: Generic function to do all plots for all
#               basic stats such as IPC, etc. 
#               Each plot is a bar chart with no legend for
#               X-axis. 
# Plots: 
#  1.) Bar Plot of INT and INTmean
#  2.) ar Plot of FP and FPmean
#############################################################
GenericSimplePlot <- function (PARAM, YTITLE, TITLE, DATA="raw", 
                               INPATH="./", OUTPATH = INPATH, INTBM=INT95,
                 FPBM=FP95) {

  xtitle <- " "

  # Plot the Integer values. 
  if (interactive() == TRUE) {
    # If we're interactive, then ask user for legend location. 
    legloc <- c()
  }
  else {
    legloc <- N
  }
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="BAR", TITLES=c(xtitle, YTITLE, TITLE), 
    COMBINENAME="INT", LEGLOC=legloc)

  # Plot the FP values
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="BAR", TITLES=c(xtitle, YTITLE, TITLE), 
    COMBINENAME="FP", LEGLOC=legloc)
}

#############################################################
# Name: GenericSimpleMunge
#  Type: Internal Function.  Should never be called directly. 
# Purpose: This function takes an INPATH, and a PARAM, 
#           and reads and munges the data and returns a lst.  
#           The way it munges the parameters specific to 
#           simple stats.
# Inputs:
#    PARAM, INPATH, OUTPATH, INT benchmarks, FP benchmarks, 
# Outputs:
#    List of data frames, one per benchmark.
#############################################################
GenericSimpleMunge <- function (PARAM, INPATH, OUTPATH, INT, FP) {
  cat ("\nPARAM="); print (PARAM)
  warning("GENERATING PARAM.norm FILE")
  return (GenericMungeData(PARAM, INPATH=INPATH, OUTPATH=OUTPATH, INT, FP))
}
#############################################################
# Overall IPC
#############################################################
Overall_IPC <- function (DATA=c(), INPATH="./", OUTPATH = INPATH, 
                INTBM=INT95, FPBM=FP95) {
  param <- "Overall_IPC"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericSimpleMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Overall IPC"
  ytitle <- "IPC"

  GenericSimplePlot(param, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, 
  INTBM=INTBM, FPBM=FPBM)
}


##########################################################
##########################################################
##################                           ##############
##################        BY REG TYPE        ##############
##################                           ##############
##########################################################
# These functions should be used for any histogram where 
#  the rows or columns represent all 64 architected 
# register numbers. 
##########################################################
##########################################################

#############################################################
# Name: GenericRegPlot
# Description: Generic function to do all histograms where 
#               the rows or columns represent architectural
#               register numbers.
# Plots: 
#    One plot of each benchmark plus the INTmean and FPmean 
#    with X-Axis being cycles. 
#############################################################
GenericRegPlot <- function (PARAM, TITLE, 
    DATA = "norm", INPATH = "./", OUTPATH = INPATH, BYROW=TRUE,
    CUMULATIVE=FALSE, 
    INTBM=INT95, FPBM=FP95) {

  # Plot only the means for now. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS = c("INTmean", "FPmean"),
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=BYROW, CUMULATIVE=CUMULATIVE,
    TITLES=c("Cycles", "Percent of Total Accesses", TITLE),   
    LEGLOC=N)

  bm <- AskUser(c(INTBM, FPBM))
  while (length(bm) != 0) {
    GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=bm,
      INPATH=INPATH, OUTPATH=OUTPATH, BYROW=BYROW, CUMULATIVE=CUMULATIVE,
      TITLES=c("Cycles", "Percent of Total Accesses", TITLE),   
      LEGLOC=N)
    
    bm <- AskUser(c(INTBM, FPBM))
  }
}

#############################################################
# Name: GenericRegMunge
#  Type: Internal Function.  Should never be called directly. 
# Purpose: This function takes an INPATH, and a PARAM, 
#           and reads and munges the data and returns a lst.  
#           The way it munges the parameters is specific to 
#           histograms where registers make up the columns or
#           rows of the stat.  It is assumed that each column
#           represents one register.  If this is not the case,
#           then REGCOL should be false. 
# Inputs:
#    PARAM, INPATH, OUTPATH, INT benchmarks, FP benchmarks, 
#    REGCOL
# Outputs:
#    List of data frames, one per benchmark.
#############################################################
GenericRegMunge <- function (PARAM, INPATH, OUTPATH, 
    INT, FP, REGCOL=TRUE) {
  
  # First create the list of register to merge
  mergelist <- AddToMergeList(c(2:9,23:26,28,29))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(10:15))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(17:22))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(33:34))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(35:42))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(43:48, 55:63))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(49:54))
  mergenames <- c("Itempreg", "Isavedreg", "Iargreg", "Ffval", 
    "Fsavedreg", "Ftempreg", "Fargreg")  

  cat ("\nPARAM="); print (PARAM)
  warning("GENERATING PARAM.norm FILE")

  return (GenericMungeData(PARAM, INPATH=INPATH, OUTPATH=OUTPATH, 
                              INTBENCHMARKS=INT, FPBENCHMARKS=FP, 
                              SUMCOL=mergelist, 
                              SUMCOLNAME=mergenames))
}

#############################################################
# Name: GenericIntRegMunge
#  Type: Internal Function.  Should never be called directly. 
# Purpose: This function takes an INPATH, and a PARAM, 
#           and reads and munges the data and returns a lst.  
#           The way it munges the parameters is specific to 
#           histograms where registers make up the columns or
#           rows of the stat.  Furthermore, this function only
#           evaluates integer registers and not all registers.
#           It is assumed that each column represents one register.  
#           If this is not the case and each ROW represents one
#           register, then REGCOL should be false. 
# Inputs:
#    PARAM, INPATH, OUTPATH, INT benchmarks, FP benchmarks, 
#    REGCOL
# Outputs:
#    List of data frames, one per benchmark.
############################################################
GenericIntRegMunge <- function (PARAM, INPATH, OUTPATH, 
    INT, FP, REGCOL=TRUE) {
  
  # First create the list of register to merge
  mergelist <- AddToMergeList(c(2:9,23:26,28,29))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(10:15))
  mergelist <- AddToMergeList(MERGELIST=mergelist, c(17:22))
  mergenames <- c("Itempreg", "Isavedreg", "Iargreg")

  cat ("\nPARAM="); print (PARAM)
  warning("GENERATING PARAM.norm FILE")

  return (GenericMungeData(PARAM, INPATH=INPATH, OUTPATH=OUTPATH, 
                              INTBENCHMARKS=INT, FPBENCHMARKS=FP, 
                              SUBSETCOL = c(1,31), 
                              SUMCOL=mergelist, 
                              SUMCOLNAME=mergenames))
}

#############################################################
# CyclesSinceRegAvailByIloadL1Miss
#############################################################
CyclesSinceRegAvailByIloadL1Miss <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByIloadL1Miss"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register Avail. and Usage for Int Loads Which Miss L1"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
        INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# CyclesSinceRegAvailByIloadL1MissI1
#############################################################
CyclesSinceRegAvailByIloadL1MissI1 <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByIloadL1MissI1"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register Avail. and Usage for Int Loads Which Miss L1 (First Issue)"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
      INTBM=INTBM, FPBM=FPBM, CUMULATIVE=TRUE)
}

#############################################################
# CyclesSinceRegAvailByFloadL1Miss
#############################################################
CyclesSinceRegAvailByFloadL1Miss <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByFloadL1Miss"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register Avail. and Usage for FP Loads/L1 Miss"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
      INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# CyclesSinceRegAvailByFloadL1MissI1
#############################################################
CyclesSinceRegAvailByFloadL1MissI1 <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByFloadL1MissI1"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register Avail. and Usage for FP Loads/L1 Miss (First Load)"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
        INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# CyclesSinceRegAvailByFload
#############################################################
CyclesSinceRegAvailByFload <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByFload"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register Avail. and Usage for FP Loads"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
          INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# CyclesSinceRegAvailByIload
#############################################################
CyclesSinceRegAvailByIload <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByIload"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register Avail. and Usage for INT Loads"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
        INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# CyclesSinceRSSAvailByFload
#############################################################
CyclesSinceRSSAvailByFload <- function (DATA=c(),
                                    INPATH="./",
                                    OUTPATH = INPATH,
                                    INTBM=INT95, FPBM=FP95)
{

  param <- "CyclesSinceRSSAvailByFload"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to
  # calculate data.
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register and Store Sets Avail. and Usage for FP Loads"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
        INTBM=INTBM, FPBM=FPBM, CUMULATIVE=TRUE)
}

#############################################################
# CyclesSinceRSSAvailByIload
#############################################################
CyclesSinceRSSAvailByIload <- function (DATA=c(),
                                    INPATH="./",
                                    OUTPATH = INPATH,
                                    INTBM=INT95, FPBM=FP95)
{

  param <- "CyclesSinceRSSAvailByIload"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to
  # calculate data.
  if (length(DATA) == 0) {
    DATA <- GenericIntRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Register and Store Sets Avail. and Usage for INT Loads"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
          INTBM=INTBM, FPBM=FPBM, CUMULATIVE=TRUE)
}

#############################################################
# CyclesSinceRegAvailByProd
#############################################################
CyclesSinceRegAvailByProd <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByProd"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Reg Avail. and Use Categorized By Producer Inst"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, 
        INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# CyclesSinceRegAvailByCons
#############################################################
CyclesSinceRegAvailByCons <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByCons"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Reg Avail. and Use Categorized By Consumer Inst"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
          INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# CyclesSinceRegAvailByReg
#############################################################
CyclesSinceRegAvailByReg <- function (DATA=c(), 
                                    INPATH="./", 
                                    OUTPATH = INPATH, 
                                    INTBM=INT95, FPBM=FP95) 
{

  param <- "CyclesSinceRegAvailByReg"
  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericRegMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  title <- "Cycles Between Reg Avail. and Use"
  GenericRegPlot(param, title, DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH,
          INTBM=INTBM, FPBM=FPBM)
}


##########################################################
##########################################################
##################                           ##############
##################     1 COL HIST TYPE      ##############
##################     X-axis = time        ##############
##################                           ##############
##########################################################
# These function represent histograms where there is only
# one column. They use the munge and plot functions 
# defined for resources.  
##########################################################
##########################################################
#############################################################
# Name: GenericSingleColPlot
# Description: Generic function to do all resource plots. 
#               X-axis represents the number of entries in 
#               resource, and Y-axis shows pecentage of time. 
# Plots: 
#  1.) Bar Plot of INTmean and FPmean  on same chart. 
#  2.) Line plot of all INT benchmarks, including INTmean. 
# 3.) Line plot of all FP benchmarks, including FPmean
#############################################################
GenericSingleColPlot <- function (PARAM, XTITLE, YTITLE, TITLE,
    DATA = "norm", INPATH = "./", OUTPATH = INPATH, XMAX=50, 
    INTBM=INT95, FPBM=FP95) {

  # Plot the Integer values. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="LINE", TITLES=c(XTITLE, YTITLE, TITLE), 
    COMBINENAME="INT", LEGLOC=N)

  # Plot FP benchmarks. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="LINE", TITLES=c(XTITLE, YTITLE, TITLE),
    COMBINENAME="FP", LEGLOC=N)

  # Plot the means. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c("INTmean", "FPmean"), 
    COMBINE=TRUE, PTYPE="LINE", BYROW=FALSE, 
    INPATH=INPATH, OUTPATH=OUTPATH, STACKED=FALSE,
    TITLES=c(XTITLE, YTITLE, TITLE), COMBINENAME="MEAN", LEGLOC=N)

  # Now zoom in to the interesting area. 
  # Plot the Integer values. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="LINE", TITLES=c(XTITLE, YTITLE, TITLE), 
    COMBINENAME="INT", LEGLOC=N, XMAX=XMAX)

  # Plot FP benchmarks. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="LINE", TITLES=c(XTITLE, YTITLE, TITLE),
    COMBINENAME="FP", LEGLOC=N, XMAX=XMAX)

  # Plot the means. 
  GenericPlotData(PARAM, DATALST=DATA, BENCHMARKS=c("INTmean", "FPmean"), 
    COMBINE=TRUE, PTYPE="LINE", BYROW=FALSE, 
    INPATH=INPATH, OUTPATH=OUTPATH, STACKED=FALSE, XMAX=XMAX,
    TITLES=c(XTITLE, YTITLE, TITLE), COMBINENAME="MEAN", LEGLOC=N)
}

#############################################################
# CyclesSinceRegAvail
#############################################################
CyclesSinceRegAvail <- function (DATA=c(), 
                                 INPATH="./", 
                                 OUTPATH = INPATH, 
                                 INTBM=INT95, FPBM=FP95,
                                 XMAX=40) {

  param <- "CyclesSinceRegAvail"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Cycles Since Register Became Available"
  xtitle <- "Cycles"
  ytitle <- "Percent of All Events"

  GenericSingleColPlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, XMAX=XMAX,
    INTBM=INTBM, INTFP=INTFP)
}

#############################################################
# DiffBetweenSrcRegAvail
#############################################################
DiffBetweenSrcRegAvail <- function (DATA=c(), 
                                 INPATH="./", 
                                 OUTPATH = INPATH, 
                                 INTBM=INT95, FPBM=FP95,
                                 XMAX=40) {

  param <- "DiffBetweenSrcRegAvail"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Difference Between Register Availability for Multiple Src Insts"
  xtitle <- "Cycles"
  ytitle <- "Percent of All Events"

  GenericSingleColPlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, XMAX=XMAX, 
    INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# numDataRdyInstrPerCycle
#############################################################
numDataRdyInstrPerCycle <- function (DATA=c(), 
                               INPATH="./", 
                               OUTPATH = INPATH, 
                               INTBM=INT95, FPBM=FP95, 
                               XMAX=40) {

  param <- "numDataRdyInstrPerCycle"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Number of Data Ready Instructions Per Cycle"
  xtitle <- "Number of Instructions"
  ytitle <- "Percent of Execution Time"

  GenericSingleColPlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, XMAX=XMAX,
    INTBM=INTBM, FPBM=FPBM)
}

##########################################################
##########################################################
##################                          ##############
##################     MISS One Col  DATA   ##############
##################     X-axis != time       ##############
##################                          ##############
##########################################################
# These function represent histograms where there is only
# one column. They use the munge and plot functions 
# defined for resources when possible. 
##########################################################
##########################################################

#############################################################
# LowerQueueFull
#############################################################
LowerQueueFull <- function (DATA=c(), 
                                 INPATH="./", 
                                 OUTPATH = INPATH, 
                                 INTBM=INT95, FPBM=FP95) {

  param <- "LowerQueueFull"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Number of Instructions in Upper Queue When Lower Queue Full"
  xtitle <- "Entries in Upper Queue"
  ytitle <- "Percent of All Events"

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, LEGLOC=N,
    INTBM=INTBM, FPBM=FPBM)
}

#############################################################
# UpperQueueFull
#############################################################
UpperQueueFull <- function (DATA=c(), 
                             INPATH="./", 
                             OUTPATH = INPATH, 
                             INTBM=INT95, FPBM=FP95) {

  param <- "UpperQueueFull"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Number of Instructions in Lower Queue When Upper Queue Full"
  xtitle <- "Entries in Lower Queue"
  ytitle <- "Percent of All Events"

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, LEGLOC=N,
    INTBM=INTBM, FPBM=FPBM)
}
#############################################################
# timeFromMappedToDataRdy
#############################################################
timeFromMappedToDataRdy <- function (DATA=c(), 
                             INPATH="./", 
                             OUTPATH = INPATH, 
                             INTBM=INT95, FPBM=FP95) {

  param <- "timeFromMappedToDataRdy"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Time from Instruction Mapping to Data Ready"
  xtitle <- "Cycles"
  ytitle <- "Percent of All Events"

  GenericResourcePlot(param, xtitle, ytitle, title, 
    DATA=DATA, INPATH=INPATH, OUTPATH=OUTPATH, LEGLOC=N,
    INTBM=INTBM, FPBM=FPBM)
}


#############################################################
# instrIssuedPerCycle
#############################################################
instrIssuedPerCycle <- function (DATA=c(), 
                             INPATH="./", 
                             OUTPATH=INPATH,
                             INTBM=INT95, FPBM=FP95) {

  param <- "instrIssuedPerCycle"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Instructions Issued Per Cycle"
  xtitle <- "Number of Instructions"
  ytitle <- "Percent of Execution Time"

  # Plot the Integer values. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=TRUE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="BAR", TITLES=c(xtitle, ytitle, title), 
    COMBINENAME="INT")

  # Plot FP benchmarks. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=TRUE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="BAR", TITLES=c(xtitle, ytitle, title),
    COMBINENAME="FP")

  # Plot the means. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c("INTmean", "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, 
    COMBINE=TRUE, STACKED=FALSE, TITLES=c(xtitle, ytitle, title),
    COMBINENAME="MEAN")
}

#############################################################
# RegCacheHitRate
#############################################################
RegCacheHitRate <- function (DATA=c(), 
                             INPATH="./", 
                             OUTPATH=INPATH,
                             INTBM=INT95, FPBM=FP95, XMAX=100) {

  param <- "RegCacheHitRate"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Register Cache Hit Rate as Function of Cache Size"
  xtitle <- "Entries in Register Cache"
  ytitle <- "Hit Rate"

  # Plot the Integer values. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, PTYPE="LINE", CUMULATIVE=TRUE,
    COMBINE=TRUE, COMBINENAME="AllINT",
    TITLES=c(xtitle, ytitle, title), BYROW=FALSE)


  # Plot FP benchmarks. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"),
    INPATH=INPATH, OUTPATH=OUTPATH, PTYPE="LINE", CUMULATIVE=TRUE,
    COMBINE=TRUE, COMBINENAME="AllFP",
    TITLES=c(xtitle, ytitle, title), BYROW=FALSE)

  # Zoom in 
  # Plot the Integer values. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, PTYPE="LINE", CUMULATIVE=TRUE,
    COMBINE=TRUE, COMBINENAME="INT BENCHMARKS", XMAX=XMAX,
    TITLES=c(xtitle, ytitle, title), BYROW=FALSE)


  # Plot FP benchmarks. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"),
    INPATH=INPATH, OUTPATH=OUTPATH, PTYPE="LINE", CUMULATIVE=TRUE,
    COMBINE=TRUE, COMBINENAME="FP BENCHMARKS", XMAX=XMAX, 
    TITLES=c(xtitle, ytitle, title), BYROW=FALSE)

  bm <- AskUser(c(INTBM, FPBM))
  while (length(bm) != 0) {
    GenericPlotData(param, DATALST=DATA, BENCHMARKS=bm, 
      INPATH=INPATH, OUTPATH=OUTPATH, PTYPE="LINE", CUMULATIVE=TRUE,
      TITLES=c(xtitle, ytitle, title), BYROW=FALSE, LEG=FALSE) 

    GenericPlotData(param, DATALST=DATA, BENCHMARKS=bm, 
      INPATH=INPATH, OUTPATH=OUTPATH, PTYPE="LINE", CUMULATIVE=TRUE,
      XMAX=XMAX, TITLES=c(xtitle, ytitle, title), BYROW=FALSE, LEG=FALSE) 
    bm <- AskUser(c(INTBM, FPBM))
  }
}

#############################################################
# numMappedInst
#############################################################
numMappedInst <- function (DATA=c(), 
                           INPATH="./", 
                           OUTPATH=INPATH,
                           INTBM=INT95, FPBM=FP95) {

  param <- "numMappedInst"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericResourceMunge(param, INPATH, OUTPATH, INTBM, FPBM)
  }

  # Initialize all variables
  title <- "Number of Mapped Instructions"
  xtitle <- "Number of Instructions"
  ytitle <- "Percent of Execution Time"

  # Plot the Integer values. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(INTBM, "INTmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=TRUE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="BAR", TITLES=c(xtitle, ytitle, title), 
    COMBINENAME="INT")

  # Plot FP benchmarks. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c(FPBM, "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=TRUE, STACKED=FALSE,
    COMBINE=TRUE, PTYPE="BAR", TITLES=c(xtitle, ytitle, title),
    COMBINENAME="FP")

  # Plot the means. 
  GenericPlotData(param, DATALST=DATA, BENCHMARKS=c("INTmean", "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, 
    COMBINE=TRUE, STACKED=FALSE, TITLES=c(xtitle, ytitle, title),
    COMBINENAME="MEAN")
}


##########################################################
##########################################################
##################                          ##############
##################     MISS Multi Col  DATA ##############
##################     X-axis != time       ##############
##################                          ##############
##########################################################
# These function represent histograms where there are 
# multiple columns. They use the munge and plot functions 
# defined for Inst Type stats, when possible. 
##########################################################
##########################################################
#############################################################
# Name: timeFromDataRdyToIssuebyPicker
# Purpose: Plot time from Data Ready to Issue Categorized
#          
#############################################################
timeFromDataRdyToIssuebyPicker <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95,
                          XMAX=30) {
  # Initialize all variables
  param <- "timeFromDataRdyToIssuebyPicker"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, 
      OUTPATH, INTBM, FPBM)
  }
    
  title <- "Cycles From Data Ready To Issue Classified By Picker"
  ytitle <- "Percent of All Instructions Issued"

  
  # Plot INTmean and FPmean as lines. 
  GenericPlotData(param, DATALST=DATA, 
    BENCHMARKS=c("INTmean", "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, PTYPE="LINE",
    TITLES=c("Cycles", ytitle, title), LEGLOC=N)

  # Plot the zoomed data. 
  GenericPlotData(param, DATALST=DATA, 
    BENCHMARKS=c("INTmean", "FPmean"), 
    INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, PTYPE="LINE",
    TITLES=c("Cycles", ytitle, title), LEGLOC=N, XMAX=XMAX)

  bm <- AskUser(c(INTBM, FPBM))
  while (length(bm) != 0) {
    GenericPlotData(param, DATALST=DATA, 
      BENCHMARKS=bm,
      INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, 
      PTYPE="LINE",
      TITLES=c("Cycles", ytitle, title), LEGLOC=N)

    # Plot the zoomed data. 
    GenericPlotData(param, DATALST=DATA, 
      BENCHMARKS=bm,
      INPATH=INPATH, OUTPATH=OUTPATH, BYROW=FALSE, PTYPE="LINE",
      TITLES=c("Cycles", ytitle, title), LEGLOC=N, XMAX=XMAX)

    bm <- AskUser(c(INTBM, FPBM))
  }
}

#############################################################
# Name: UniqueSrcRegUsedPerCycle
# Purpose: Plot data for the number of unique source registers
#	   used per cycle as a function of the issue width
#          
#############################################################
UniqueSrcRegUsedPerCycle <- function (DATA=c(), INPATH="./", 
                          OUTPATH = INPATH, INTBM=INT95, FPBM=FP95, XMAX=XMAX) {
  # Initialize all variables
  param <- "UniqueSrcRegUsedPerCycle"

  # If no DATA is specified either as a list, or strings
  # "norm" or "raw", then it is assumed that we have to 
  # calculate data. 
  if (length(DATA) == 0) {
    DATA <- GenericInstTypeTpuMunge(param, INPATH, 
      OUTPATH, INTBM, FPBM)
  }
    
  title <- "Number of Unique Source Registers Required Per Cycle"
  ytitle <- "Percent of Total Execution Time"
  xtitle <- "Number of Unique Source Registers"

  
  # Plot INTmean data
  GenericPlotData(param, DATALST=DATA, 
    BENCHMARKS=c("INTmean"),
    INPATH=INPATH, OUTPATH=OUTPATH, 
    TITLES=c(xtitle, ytitle, title), LEGLOC=NE, XMAX=XMAX)

  # Plot FPmean data
  GenericPlotData(param, DATALST=DATA, 
    BENCHMARKS=c("FPmean"),
    INPATH=INPATH, OUTPATH=OUTPATH, 
    TITLES=c(xtitle, ytitle, title), LEGLOC=NE, XMAX=XMAX)

  bm <- AskUser(c(INTBM, FPBM))
  while (length(bm) != 0) {
    GenericPlotData(param, DATALST=DATA, 
      BENCHMARKS=bm,
      INPATH=INPATH, OUTPATH=OUTPATH, 
      TITLES=c(xtitle, ytitle, title), LEGLOC=NE, XMAX=XMAX)

    bm <- AskUser(c(INTBM, FPBM))
  }
}





