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

#################################################
#################################################
############                      ###############
############  DATA READ FUNCTIONS ###############
############                      ###############
#################################################
#################################################

#################################################################
# Some Basic Definitions used all over the ASIM code. 
#################################################################
INT95 <- c("C-AINT-compress95", 
  "C-AINT-gcc95", 
  "C-AINT-go95", 
  "C-AINT-ijpeg95", 
  "C-AINT-li95", 
  "C-AINT-m88ksim95", 
  "C-AINT-perl95", 
  "C-AINT-vortex95")

# Define shortened names to use for each benchmark. 
INT95NAMES <- c()

# Chop off all C-AINT, chop off 95, cut it down to 5 characters, and
# then clean up vowels and cut it down to a minimum of 4 characters. 
INT95NAMES[INT95] <- 
	abbreviate(substr(sub(".*-.*-", "", 
		sub("95", "", INT95)), 1, 5), 4)
  
FP95<-c("C-AINT-applu95",
  "C-AINT-apsi95",
  "C-AINT-fpppp95",
  "C-AINT-hydro2d95",
  "C-AINT-mgrid95",
  "C-AINT-su2cor95",
  "C-AINT-swim95",
  "C-AINT-tomcatv95",
  "C-AINT-turb3d95",
  "C-AINT-wave595")

# Define shortened names to use for each benchmark. 
FP95NAMES <- c()
FP95NAMES[FP95] <- 
	abbreviate(substr(sub(".*-.*-", "", 
		sub("95", "", FP95)), 1, 5), 4)


YKINT00 <- c(
  "YKSPEC-AINT-bzip22000",
  "YKSPEC-AINT-crafty2000",
  "YKSPEC-AINT-eon2000",
  "YKSPEC-AINT-gap2000",
  "YKSPEC-AINT-gcc2000",
  "YKSPEC-AINT-gzip2000",
  "YKSPEC-AINT-mcf2000",
  "YKSPEC-AINT-parser2000",
  "YKSPEC-AINT-perlbmk2000",
  "YKSPEC-AINT-twolf2000",
  "YKSPEC-AINT-vortex2000",
  "YKSPEC-AINT-vpr2000"
)

# Define shortened names to use for each benchmark. 
YKINT00NAMES <- c()
YKINT00NAMES[YKINT00] <- 
	abbreviate(substr(sub(".*-.*-", "", 
		sub("2000", "", YKINT00)), 1, 5), 4)


YKFP00 <- c(
  "YKSPEC-AINT-ammp2000",
  "YKSPEC-AINT-applu2000",
  "YKSPEC-AINT-apsi2000",
  "YKSPEC-AINT-art2000",
  "YKSPEC-AINT-equake2000",
  "YKSPEC-AINT-facerec2000",
  "YKSPEC-AINT-fma3d2000",
  "YKSPEC-AINT-galgel2000",
  "YKSPEC-AINT-lucas2000",
  "YKSPEC-AINT-mesa2000",
  "YKSPEC-AINT-mgrid2000",
  "YKSPEC-AINT-sixtrack2000",
  "YKSPEC-AINT-swim2000",
  "YKSPEC-AINT-wupwise2000"
)

# Define shortened names to use for each benchmark. 
YKFP00NAMES <- c()
YKFP00NAMES[YKFP00] <- 
	abbreviate(substr(sub(".*-.*-", "", 
		sub("2000", "", YKFP00)), 1, 5), 4)

SW <- c(1,1)
S  <- c(2,1)
SE <- c(3,1)
NW <- c(1,3)
N  <- c(2,3)
NE <- c(3,3)
E <- c(3,2)
W <- c(1,2)
C <- c(2,2)

COLORS <- c("blue", "yellow", "black", "green", "magenta", "cyan", "red", "white")

AsimColors <- function (NUM, PTYPE="LINE") {
  if (PTYPE != "LINE") {
    col <- COLORS
  }
  else {
    # Can't use while in a line plot
    col <- COLORS[1:length(COLORS)-1]
  }

  if (NUM <= length(col)) {
    return (col[1:NUM])
  }
  else {
    col <- rep(col, ceiling(NUM/length(col)))
    return (col[1:NUM])
  }
}


#################################################################
# Name: ReadParamData
# Purpose: This R function takes a parameter and a path name and 
#          reads data from  file <PATH>/<PARAM>.R file.  The data is 
#          placed into a list of data.frames.  It returns a list 
#          of data frames which contain the raw data for all the 
#         benchmarks in file <PATH>/<PARAM>.R.  
#
# Inputs: 
#     PARAM: Parameter to read
#      PATH: The path to location of files <BENCHMARK NAME>.<PARAM>
#      
# Defaults: NONE      
#################################################################
ReadParamData <- function(PARAM, PATH) {
  # Check the incoming list of file names
  bnames <- c()
  lst<-list()

  bmlst <- list()
  filesfound <- 0

  fullname <- paste (PATH, PARAM, sep="/")
  fullname <- paste (fullname, "R", sep=".")

  # First, walk through the file and read the benchamrks
  # into a list. 

  # Step 1: Determine the number of rows per benchmark. 
  numrows <- scan(file=fullname, nlines=1)
  skiplines <- 1

  # Step 2: Read each benchmark. 
  bname <- scan (file=fullname, what="", nlines=1, skip=skiplines)
  skiplines <- skiplines + 1
  while (length(bname) != 0) {
    # Read the data for the benchmark. 
    lst[bname] <- list(read.table.ASIM(fullname, skip = skiplines, 
                                  nlines = numrows))
    skiplines <- skiplines + numrows

    # Get next benchmark, if any. 
    bname <- scan (file=fullname, what="", nlines=1, skip=skiplines)
    skiplines <- skiplines + 1
  }

  return (lst)
}

#################################################################
# Name: FilterParamData
# Purpose: This R function takes a vector of benchmark names
#          and a lst of data frames containing information per
#          benchmark and parses the list for the benchmarks
#          requested.  If the list doesn't contain a particular
#          benchmark, then a warning message is printed. 
# Inputs: 
#     DATALST: List of all data in file <PARAM>.R
#      BENCHMARKS: List of benchmarks requested
#  Defaults:
#      NONE
#  Outputs: 
#      Lst of data frames containing the benchmarks requested when
#      there was data for that benchmark in DATALST. 
#################################################################
FilterParamData <- function(DATALST, BENCHMARKS) {
  #
  # Check inputs
  if (length(DATALST) == 0 || length(BENCHMARKS) == 0) {
    stop("Must provide a list of data frames and a vector of benchmarks")
  }
  
  #
  # Determine which benchmarks are available in DATALST
  bnames <- names(DATALST)

  missing <- c()  
  for (i in 1:length(BENCHMARKS)) {
    # If the name is available in bnames, then v will
    # be a value of 1.  Else, it will be a numeric(0)
    v <- grep(BENCHMARKS[i] , bnames)

    if (length(v) != TRUE) {
      # This benchmark is not available
      cat("\nBENCHMARK="); print(BENCHMARKS[i])
      warning("DON'T HAVE DATA FOR BENCHMARK")
      missing <- c(missing, i)
    }
  }
  # Remove missing benchmarks for list of benchmarks. 
  if (length(missing) != 0) {
    BENCHMARKS <- BENCHMARKS[-missing]
  }

  # Create new list. 
  parsedlst <- DATALST[BENCHMARKS]

  return (parsedlst)
}

#################################################################
# Name: read.table.ASIM
# Purpose: This function is EXACTLY the same as the read.table 
#          function except that it let's the user specify the 
#          number of lines to read from file.  Since this is 
#          specific to ASIM, one shouldn't have to touch the 
#          other defaults. 
#
# Inputs: 
#     file: Name of file 
#     skip: Number of lines to skip
#      nlines: Number of lines to read. 
# Defaults: NONE      
#################################################################
read.table.ASIM <- function (file, header=FALSE, sep="", quote="\"\'", dec=".",
            row.names, col.names, as.is=FALSE,
            na.strings="NA", skip=0, nlines=0) {
  type.convert <- function(x, na.strings = "NA",
                            as.is = FALSE, dec = ".")
  .Internal(type.convert(x, na.strings, as.is, dec))

  ##  basic column counting and header determination;
  ##  rlabp (logical) := it looks like we have column names

  row.lens <- count.fields(file, sep, quote, skip)

  ##
  ## If we didn't specify a fixed number of lines, 
  ## then read the whole file.  
  if (nlines == 0) {
    nlines <- length(row.lens)  
  }
  else {
    ##
    ## Only consider the row lengths for the number
    ## of lines requested. 
    row.lens <- row.lens[1:nlines]
  }

  rlabp <- nlines > 1 && (row.lens[2] - row.lens[1]) == 1
  if(rlabp && missing(header))
    header <- TRUE

  if (header) { # read in the header
    col.names <- scan(file, what="", sep=sep, quote=quote, nlines=1,
                      quiet=TRUE, skip=skip)

    skip <- skip + 1
    row.lens <- row.lens[-1]
    nlines <- nlines - 1
  } 
  else if (missing(col.names))
    col.names <- paste("V", 1:row.lens[1], sep="")

  ##  check that all rows have equal lengths

  cols <- unique(row.lens)
  if (length(cols) != 1) {
    cat("\nrow.lens=\n"); print(row.lens)
    stop("all rows must have the same length.")
  }

  ##  set up for the scan of the file.
  ##  we read all values as character strings and convert later.

  what <- rep(list(""), cols)
  if (rlabp)
    col.names <- c("row.names", col.names)
  names(what) <- col.names
  data <- scan(file=file, what=what, sep=sep, quote=quote, skip=skip,
               na.strings=na.strings, quiet=TRUE, nlines=nlines)

  ##  now we have the data;
  ##  convert to numeric or factor variables
  ##  (depending on the specifies value of "as.is").
  ##  we do this here so that columns match up

  if(cols != length(data)) { # this should never happen
    warning(paste("cols =",cols," != length(data) =", length(data)))
    cols <- length(data)
  }

  if(is.logical(as.is)) {
    as.is <- rep(as.is, length=cols)
  } 
  else if(is.numeric(as.is)) {
    if(any(as.is < 1 | as.is > cols))
      stop("invalid numeric as.is expression")
    i <- rep(FALSE, cols)
    i[as.is] <- TRUE
    as.is <- i
  } 
  else if (length(as.is) != cols)
    stop(paste("as.is has the wrong length", length(as.is),"!= cols =", cols))
  for (i in 1:cols)
    data[[i]] <- type.convert(data[[i]], as.is = as.is[i], dec = dec)

  ##  now determine row names
  if (missing(row.names)) {
    if (rlabp) {
      row.names <- data[[1]]
      data <- data[-1]
    }
    else row.names <- as.character(1:nlines)
  } 
  else if (is.null(row.names)) {
    row.names <- as.character(1:nlines)
  } 
  else if (is.character(row.names)) {
    if (length(row.names) == 1) {
      rowvar <- (1:cols)[match(col.names, row.names, 0) == 1]
      row.names <- data[[rowvar]]
      data <- data[-rowvar]
    }
  } 
  else if (is.numeric(row.names) && length(row.names) == 1) {
    rlabp <- row.names
    row.names <- data[[rlabp]]
    data <- data[-rlabp]
  } 
  else stop("invalid row.names specification")

  ##  this is extremely underhanded
  ##  we should use the constructor function ...
  ##  don't try this at home kids

  class(data) <- "data.frame"
  row.names(data) <- row.names
  data
}

#########################################################
#########################################################
############                              ###############
############  DATA MANIPULATION FUNCTIONS ###############
############                              ###############
#########################################################
#########################################################

#################################################################
# Name: CatLstData
# Purpose: This takes all data frames contained in input list and
#         creates a new data frame which is the concatenation of
#          of all columns in the data frames of the input list. 
#          For example, if the input list contains 2 data frames
#         with 3 columns each, then the new data frame contains 
#         6 columns.  This assumes an equivalent number of rows 
#          in each data frame of the input list.  The new data 
#         frame is returned from function.  You generally use this
#         function when you have a bunch of one column histograms
#          where each column represents one benchmark.  If this
#          method is called after each benchmark's values are 
#          normalized, then each benchmark will hold equal weight. 
# Inputs: 
#      DATALST: List of data frames. 
#      BENCHMARKS: Concatenate only these names and not others. 
# Defaults: 
#      BENCHMARKS=names(DATALST).  This means that we cat all
#          data frames in input list. 
# Output: One data frame which is the combination of all data
#         frames in input list. 
#################################################################
CatLstData <- function(DATALST, BENCHMARKS=names(DATALST)) {
  if (length(DATALST) == 0) {
    stop("No data frames in list")
  }

  # This loop just walks through the list and makes sure that 
  # some constraints have been met. 
  datanames <- names(DATALST)
  numrows <- nrow(DATALST[[1]])
  newlst <- c()
  for (i in BENCHMARKS) {
    if ( length(grep(i, datanames)) == 0 ) {
      cat("\nBENCHMARK="); print(i)
      warning("DON'T HAVE DATA FOR BENCHMARK")
    }
    else {
      newlst[i] <- DATALST[i]

      # Attach the name of the benchmark to the column name.       
      # after stripping out [A,B,C,F,BESPEC]-[AINT|ATF]
      # and any numeric characters (like 95). 
      newname <- abbreviate(substr(sub("[a-zA-Z0-9]+-[a-zA-Z0-9]+-", "", 
        gsub("[0-9]+", "", i)), 1, 20), 10)

      names(newlst[[i]]) <- newname
      if (is.data.frame(newlst[[i]]) != TRUE) {
          stop("Entry in list is not a data frame")
      }
      if (numrows != nrow(newlst[[i]])) {
        stop("data arrays do not have the same number of rows")
      }
    }
  }
  return (data.frame(newlst))
}

#################################################################
# Name: SubsetLstData
# Purpose: This R function takes a list of data frames and produces 
#        a list of data frames which is a subset of the original 
#        data frames in list.   For example, it can take a list of 
#        data frames with 12 rows and 15 cols and produce square 
#        matrices containing the first 12 columns. You can request 
#        any subsection of the data frame using mincol, maxcol, 
#        minrow and maxrow.  
#       NOTE: mincol and minrow must be >= 1
#        NOTE: maxcol and maxrow must be <= the size of the original
#            data frame.  
# Inputs:
#   DATALST: List of data frames consisting of the matrices which 
#       you want to split. 
#    MINCOL, MAXCOL, MINROW, MAXROW: The new matrix will consist of 
#       the columns and rows specified by these variables
# Defaults: 
#    MINCOL = 1
#    MINROW = 1
#   MAXCOL = Maximum column of the first matrix of DATALST
#   MAXROW = Maximum row of the first matrix of DATALST
#################################################################
SubsetLstData <- function (DATALST, MINCOL=1, MINROW=1, 
        MAXCOL = ncol(DATALST[[1]]), MAXROW = nrow(DATALST[[1]])) {

  if (MINCOL < 1) {
    stop("Requested minimum column number less than 1")
  }

  if (MINROW < 1) {
    stop("Requested minimum column number less than 1")
  }

  if (MAXCOL > ncol(DATALST[[1]])) {
    stop("Requested max column number greater than matrix")
  }

  if (MAXROW > nrow(DATALST[[1]])) {
    stop("Requested max row number greater than matrix")
  }

  if (MINROW > MAXROW) {
    stop ("Requested min row > max row")
  }

  if (MINCOL > MAXCOL) {
    stop ("Requested min col > max col")
  }

  sublst <- list()
  for (i in 1:length(DATALST)) {
    sublst[i] <- list(as.data.frame(DATALST[[i]][MINROW:MAXROW, MINCOL:MAXCOL]))

    # For some reason, if MINCOL == MAXCOL, we get it back as a vector with
    # no labels.  If we cast it to a data frame, we get the concept of
    # row and col labels, but they're the wrong values.  Therefore, we have 
    # to go back and fix them.  This doesn't occur when MINROW=MAXROW
    if (MINCOL == MAXCOL) {
      names(sublst[[i]]) <- names(DATALST[[i]])[MINCOL]
      row.names(sublst[[i]]) <- row.names(DATALST[[i]])
    }
  }  

  names(sublst) <- names(DATALST)

  return(sublst)
}

#################################################################
# Name: SumColumns
# Purpose: This R function takes a list of data frames containing 
#     raw data and a list containing the columns of each data frame
#      to sum and does the following per data frame:
#     1. Takes the sum of the requested columns and creates a new 
#         column in data frame at end of data frame. 
#     2. Deletes the individual columns which were summed from each
#         Data frame.       
#     3. Labels the new columns with the names given by NEWNAME
#
#     It returns a list of data frames which have the requested columns
#     merged. 
# Inputs: 
#    DATALST: List of data frames for which you want the columns summed
#    MERGELIST: List of vectors specifying which columns you want merged
#   NEWCOLNAME: Vector of names for the new columns created by the 
#       summing procedure.
# Defaults: NONE
#################################################################

SumColumns <- function(DATALST, MERGELIST, NEWNAME) {
  if (is.list(DATALST) != TRUE) {
    stop("Need a list for the first parameter")
  }
  if (is.list(MERGELIST) != TRUE) {
    stop("Need a list of columns to merge for the second parameter")
  }
  if (length(MERGELIST) != length(NEWNAME)) {
    stop("Number of merges requested != number of column names")
  }

  newlst <- list()
  for (k in 1:length(DATALST)) {
    newMat <- DATALST[[k]]
    deleteCols <- c()
    # For each entry in mergelist
    for (j in 1:length(MERGELIST)) {
      mergedCols <- MERGELIST[[j]]

      # Create a new matrix consisting of all 
      # columns we merged.  We use this new matrix
      # to detele the merged columns. 
      deleteCols <- c(deleteCols, mergedCols)
      sumcol <- list()

      # Step 1:
      # Add up requested columns in matrix
      for (i in mergedCols) {
        if ( (i > length(newMat)) || i == 0) {
          stop("ERROR: requesting non-existent column")
        }
        if (length(sumcol) > 0) {
          sumcol <- sumcol + newMat[i]    
        }
        else {
          sumcol <- newMat[i]
        }
      }
    
      # Step 2:
      # Now that we calculated the sum of requested columns, 
      # add the new column to matrix.  
      newcol <- length(newMat) + 1;
      newMat[newcol] <- sumcol
      names(newMat)[newcol] <- NEWNAME[j]
    }

    # Step 3:
    # Remove the columns which were merged from matrix
    newMat <- newMat[-deleteCols];

    # Step 4:
    # Create a new list with the modified matrix
    newlst[k] <- list(newMat)
  }

  # Reassign list labels since creating a new list means
  # that we lost the old list's labels.
  names(newlst) <- names(DATALST)

  return (newlst)
}

#################################################################
# Name: SumRows
# Purpose: This R function takes a list of data frames containing 
#     raw data and a list containing the rows of each data frame
#     to sum and does the following per matrix:
#     1. Takes the sum of the requdested rows and creates a new 
#         row at bottom of data frame. 
#     2. Deletes the individual rows which were summed from each
#         Data frame.       
#     3. Labels the new rows with the names given by NEWNAME
#
#     It returns a list of data frames which have the requested 
#     rows merged. 
# Inputs: 
#    DATALST: List of data frames for which you want the rows summed
#    MERGELIST: List of vectors specifying which rows you want merged
#   NEWNAME: Vector of names for the new rows created by the 
#       summing procedure.
# Defaults: NONE
#################################################################

SumRows <- function(DATALST, MERGELIST, NEWNAME) {
  if (is.list(DATALST) != TRUE) {
    stop("Need a list for the first parameter")
  }
  if (is.list(MERGELIST) != TRUE) {
    stop("Need a list of rows to merge for the second parameter")
  }
  if (length(MERGELIST) != length(NEWNAME)) {
    stop("Number of merges requested != number of row names")
  }

  newlst <- list()
  for (k in 1:length(DATALST)) {
    newMat <- DATALST[[k]]
    deleteRows <- c()
    # For each entry in mergelist
    for (j in 1:length(MERGELIST)) {
      mergedRows <- MERGELIST[[j]]

      # Create a vector consisting of all 
      # rows we merged.  We use this vector
      # to delete the merged rows at end of 
      # this routine. 
      deleteRows <- c(deleteRows, mergedRows)
      sumrow <- list()

      # Step 1:
      # Add up requested rows in matrix
      for (i in mergedRows) {
        if ( (i > nrow(newMat)) || i == 0) {
          stop("ERROR: requesting non-existent row")
        }
        if (length(sumrow) > 0) {
          sumrow <- sumrow + newMat[i,]    
        }
        else {
          sumrow <- newMat[i,]
        }
      }
    
      # Step 2:
      # Now that we calculated the sum of requested rows,
      # add the new row to end of data frame. 
      newrow <- nrow(newMat) + 1;
      newMat[newrow,] <- sumrow
      row.names(newMat)[newrow] <- NEWNAME[j]
    }

    # Step 3:
    # Remove the rows which were merged from matrix
    # For some reason, this call deletes all labels
    # if this is a single column data frame.  Therefore, 
    # to fix the problem, I need to reattach the row
    # and column lables.  Note that the columns lables
    # are not modified, but the row labels are. 
    rowlab <- labels(newMat)[[1]]
    collab <- labels(newMat)[[2]]
    newMat <- newMat[-deleteRows,]
    rowlab <- rowlab[-deleteRows]
    row.names(newMat) <- rowlab
    names(newMat) <- collab
    

    # Step 4:
    # Create a new list with the modified matrix
    newlst[k] <- list(newMat)
  }

  # Reassign list labels since creating a new list means
  # that we lost the old list's labels.
  names(newlst) <- names(DATALST)

  return (newlst)
}

#################################################################
# Name: NormLst
# Purpose: This R function takes a list of data frames with each data frame
#   representing a benchmark and normalizes the values in each 
#   data frame.  Each entry in a data frame is normalize to the sum of 
#   all entries in data frame.  This function does not do any cross list
#   entry operations. The sum of all entries in each normalized data frame
#   is equal to 100.
# Inputs: 
#    DATALST: List of data frames which are to be normalized
# Defaults: NONE
# Output: A list of data frames with the sum of all entries in data 
#        frame = 100.      
#################################################################
NormLst <- function(DATALST) {
  # Check the incoming list of data
  if (length(DATALST) < 1) {
    stop("No matrices in list")
  }

  if (nrow(DATALST[[1]]) == 1 && ncol(DATALST[[1]]) == 1) {
    warning ("SINGLE ENTRY IN DATA FRAME. NOT NORMALIZING DATA")
    return (FALSE)
  }

  normlst <- list()
   for (i in 1:length(DATALST)) {
    inp<-DATALST[[i]] 

    #
    # Normalize the values for each benchmark relative
    # to the sum of all values in benchmark.  This means
    # that the sum of every element in matrix adds up to 
    # 100. 
    s <- sum(inp)
    if (s == 0) {
      s <- c(1)
    }
    # Normalize for entire matrix.
    # I lose the column labels when I divide (don't know why), so
    # I have to re-establish them. 
    colnames <- names(inp)
    inp <- 100*(inp/s)
    names(inp) <- colnames
    normlst[i] <- list(inp)
  }

  #
  # Reassign the names to each entry in list.  
  names(normlst) <- names(DATALST)
  
  return (normlst)
}

#################################################################
# Name: NormLstByCol
# Purpose: This R function takes a list of data frames, with each data 
#    frame  representing a benchmark, and normalizes the values in each 
#   data frame according to its column.  This means that the sum of
#   each column in every data frame is equal to 100.  This way, you
#   can compare one column versus another in each matrix, and each 
#   column is given equal weight.  This function does not do any cross 
#   matrix operation. The sum of all entries in each column = 100, 
#    and the sum of all entries in matrix equals number of columns * 100. 
# Inputs: 
#    DATALST: List of data frames which are to be normalized by column
# Defaults: NONE
# Output: A list of data frames, each the same size as the input,
#    but with the values in each column normalized. 
#################################################################
NormLstByCol <- function(DATALST) {
  # Check the incoming list of data
  if (length(DATALST) < 1) {
    stop("No matrices in list")
  }

  if (nrow(DATALST[[1]]) == 1) {
    warning ("SINGLE ELEMENT PER COLUMN. NOT NORMALIZING DATA")
    return (FALSE)
  }

  normlst <- list()
   for (i in 1:length(DATALST)) {
    inp<-DATALST[[i]] 
    # Normalize the values for each benchmark relative
    # to the sum of all values in each column. 
    for (j in 1:length(inp)) {
      s <- sum(inp[j])
      # Don't want to divide by 0. 
      if (s == 0) {
        s <- 1
      }
      # Normalize for entire matrix
      inp[j] <- 100*(inp[j]/s)
    }
    normlst[i] <- list(inp)
  }
  
  return (normlst)
}

#################################################################
# Name: NormLstByRow
# Purpose: This R function takes a list of data frames with each matrix 
#   representing a benchmark and normalizes the values in each 
#   data frame according to each row.  This means that the sum of
#   each row in every matrix is equal to 100.  This way, one can
#   compare one row versus another in each data frame. This function 
#   does not do any cross data frame operation. The sum of all entries 
#   in each row = 100, and the sum of all entries in data frame
#   equals number of rows * 100. 
# Inputs: 
#    DATALST: List of data frames which are to be normalized by column
# Defaults: NONE
# Output: A list with the same number of data frames as the input 
#     list.
#################################################################
NormLstByRow <- function(DATALST) {
  # Check the incoming list of data
  if (length(DATALST) < 1) {
    stop("No matrices in list")
  }

  if (ncow(DATALST[[1]]) == 1) {
    warning ("SINGLE ELEMENT PER ROW. NOT NORMALIZING DATA")
    return (FALSE)
  }

  normlst <- list()
   for (i in 1:length(DATALST)) {
    inp<-DATALST[[i]] 
    # Normalize the values for each benchmark relative
    # to the sum of all values in each column. 
    for (j in 1:nrow(inp)) {
      s <- sum(inp[j,])
      # Don't want to divide by 0. 
      if (s == 0) {
        s <- 1
      }
      # Normalize for entire matrix
      inp[j,] <- 100*(inp[j,]/s)
    }
    normlst[i] <- list(inp)
  }
  
  return (normlst)
}

#################################################################
# Name: GenLstMean
# Purpose: This R function takes a list of data frames (normalized 
#   or raw data) and generates the mean across all data frames.  
#   The resulting data frame is the same size as the original data 
#   frames in input list, but each entry of the resulting data 
#   frame is equivalent to the mean of the corresponding entry in 
#   all data frames in list.  If the data fed to this function is 
#   normalized data, then the resulting data frame will be the mean 
#    of normalized data and each  benchmark in the mean holds equal 
#   weight.  If the data fed to function is raw data, then the 
#    resulting data frame will be the mean of raw data which means 
#    that some benchmarks will have more impact than others.   
# Inputs: 
#    DATALST
# Defaults: NONE
# Outputs: 
#   A single data frame consisting of the mean of all data frames 
#   in input list. 
#################################################################
GenLstMean <- function(DATALST) {
  # Check the incoming list of data
  if (length(DATALST) < 1) {
    stop("No entries in list")
    }

  sumbench<-list()
   for (i in 1:length(DATALST)) {
    inp<-DATALST[[i]] 

    if (length(sumbench) > 0) {
      sumbench<-sumbench+inp
    }  
    else {
      sumbench<-inp
      # I'm assuming here that the column
      # names of the first entry in list
      # is the same as column names of all
      # entries. 
      colnames <- names(sumbench)
     }
   }

   #
  # Divide the sumbench by the number of benchmarks to get the
  # mean.  Variable mean is also a list. 
  #
  # Anytime we divide, we lose column names.  Therefore, 
  # save and restore column names. 
  mean<-sumbench/length(DATALST)
  names(mean) <- colnames
  return(mean)
}

######################################################
######################################################
############                           ###############
############  VISUALIZATION FUNCTIONS  ###############
############                           ###############
######################################################
######################################################

#################################################################
# Name: NewXPlot
# Purpose: Generates a new plotting screen.  
# Inputs: NONE
# Outputs: NONE
#################################################################
#
# Initialize the plotting type.
NewXPlot <- function () {
  # Start a new plot.    
  X11()
  plot.new()
}

#################################################################
# Name: NewPSPlot
# Purpose: Generates a new plot for printing.
# Inputs: Filename of postscript file.
# Outputs: NONE
#################################################################
#
# Initialize the plotting type.
NewPSPlot <- function (FILENAME) {
  # Start a new plot.    
  # If we don't have a null device, then turn off the other device
  if (dev.cur() != 1) {
    dev.off()
  }
  postscript(FILENAME, horizontal=FALSE, width=10.0, height=8.0)
  plot.new()
}

#####################################################################
# Name: PlotLines
# Purpose: Plot data for the requested matrix or vectory. In a matrix,
#   each column represents one line.  If requested, the plot function
#   will also plot the mean of all the columns.  
# Inputs: 
#  DATA: Input matrix containing data to plot
# STACKED: If true, data for each column is accumulated 
#	and plotted.  This means that the first column
#       is plotted as is, and the second line in the 
#       graph is the accumulation of the col 1 + col 2
#       and the nth line is the accumulation of n cols. 
# CUMULATIVE: If TRUE, then the data in each line is the accumulation
#	of all data preceding it.  The data accumulates as X increases.
# XMIN, YMIN, XMAX, YMAX: The dimensions of the plot.  
# TITLE, SUBTITLE, XTITLE, YTITLE: Title, subtitle, x and y axis 
#        labels for graph. 
# LEGEND: The values to use for the legend of the graph.  This is 
#      	only an option when plotting data.frames and not vectors. In 
#      	other words, this only makes sense when you're plotting more than 
#      	one line.  
# LEGENDLOC: A vector of two values specifying the X and Y location of 
#     legend.  The options for each value are: 1, 2, 3
#          For X location, 1 means left, 2 means center, 3 means right. 
#          For Y location, 1 means bottom, 2 means center, 3 means top.  
#          A vector of 1,2 means place legend at left side and center
#         it with respect to y axis. 
#  Defaults:  
#      XMIN=0
#      YMIN=0
#     XMAX = Number of cols in data.frame or the length of a vectory
#      YMAX = Maximum entry in data.frame or vector
#      STACKED = FALSE
#      TITLE = "title"
#      SUBTITLE = "subtitle"
#      XTITLE = "xtitle"
#      YTITLE = "ytitle"
#      LEGEND = column names if data.frame, or NULL if vectory
#      LEGENDLOC = c(2,2) (In center of graph)
#####################################################################
PlotLines <- function (DATA, STACKED = FALSE, 
      XMIN=0, YMIN=0, XMAX=c(0),
      YMAX=c(0), XLABEL="xtitle", YLABEL="ytitle", 
      TRANSPOSE=FALSE, TITLE="title", 
      CUMULATIVE = FALSE, 
      LEGEND=TRUE, LEGENDLOC=c(2,2)) { 


  # Copy over the original DATA
  plotdata <- as.matrix(DATA)

  #
  # The input variable LEGEND can either be a boolean value 
  # or a vector of strings.  If it is a boolean value and TRUE, 
  # or if it is defined, then we use it. If it is false, we 
  # don't print a legend. 
  if (is.logical(LEGEND) == FALSE) {
    # LEGEND is not a logical value.  Therefore, we have to
    # check its length. 
    if (TRANSPOSE == TRUE) {
       if (length(LEGEND) != nrow(DATA)) {
        stop("Number of labels do not match number of rows")
      }  
    }
    else {
      if (length(LEGEND) != ncol(DATA)) {
        stop("Number of labels do not match number of cols")
      }
    }
  }
  else {
    # LEGEND is a logical value.  If it is set to TRUE, 
    # then print values. 
    if (LEGEND == TRUE) {
      if (TRANSPOSE == TRUE) {
        LEGEND <- labels(DATA)[[1]]
      }
      else {
        LEGEND <- labels(DATA)[[2]]
      }
    }
  }

  if (is.data.frame(DATA) != TRUE && is.vector(DATA) != TRUE) {
    stop("Input must be of type data.frame or vector")
  }

  if (TRANSPOSE == TRUE) {
    plotdata <- t(plotdata)
  }

  if (STACKED == TRUE) {
    # If we use apply to cumsum across rows, 
    # it transposes the input matrix.  Hense, 
    # I end up transposing it back. 
    plotdata <- t(apply(plotdata, 1, cumsum));
  }

  #
  # Determine if we need to plot cumulative data.  If so, we 
  # add columns to each other.  Column 2 is the sum of Column
  # 2 + column 1, column 3 = column 1 + 2 + 3, and so on. 
  if (CUMULATIVE == TRUE) {
    if (nrow(plotdata) < 2) {
      stop("Cannot accumulate data when number of rows < 2")
    }
    for (i in 2:nrow(plotdata)) {
      plotdata[i,] <- plotdata[(i-1),] + plotdata[i,]
    }
  }

  # Determine the maximum Y value in plotting data
  # We need this to set the YMAX correctly.  
  maxy <- max(plotdata) * 1.1
  maxx <- nrow(plotdata)

  # If xmax and ymax are not defined, then replace them
  # with the correct values
  if (YMAX == 0) {
      YMAX <- maxy
  }
  if (XMAX == 0) {
      XMAX <- maxx
  }

  # Convert the XMIN, YMIN, XMAX, YMAX values to integers. 
  XMIN <- as.integer(XMIN)
  YMIN <- as.integer(YMIN)
  XMAX <- as.integer(XMAX)
  YMAX <-  as.integer(YMAX)

  # Check the number and size of elements for the X axis
  # If there are too many characters, or if any label is 
  # too long, then we plot them perpendicular to axis. 
  # Actually, if the labels get really long (like when the 
  # x-axis represents numbers), then we shouldn't try to 
  # to plot them vertically since it looks like garbage. 
  if (length(labels(plotdata)[[2]]) < 20) {
    if (sum(nchar(labels(plotdata)[[2]])) > 40 || 
        max(nchar(labels(plotdata)[[2]])) > 10) {
      par(las = 2)
    }
  }
    
  #
  # Plot a null graph for the moment, and then fill later
  plot(plotdata[,1], type="n", 
    ann=FALSE, 
    xlim=c(XMIN,XMAX),
    ylim=c(YMIN,YMAX))

  par(las = 0)

  usr <- par("usr")
  rect(usr[1], usr[3], usr[2], usr[4], border="black")
  
  linecol <- c()
  ptype <- c()
  if (ncol(plotdata) > 1) {
    plotcolors <- AsimColors(ncol(plotdata))
    colornum <- c(1)
    for(i in 1:ncol(plotdata)) {
      # We need to give the lines and points functions the 
      # X coordinates for what we are plotting.  We're
      # starting at XMIN and going to XMIN + the number of 
      # points in plot.
      lines (c(XMIN:(XMIN+length(plotdata[XMIN:XMAX,i]) - 1)),
            plotdata[XMIN:XMAX,i], col=plotcolors[i])
      points(c(XMIN:(XMIN+length(plotdata[XMIN:XMAX,i])-1)),
            plotdata[XMIN:XMAX,i], pch=i, col=plotcolors[i], 
            cex=1.25)

      # Save point types for use in legend. 
      ptype <- c(ptype, i)
    }
  }
  else {
    plotcolors <- AsimColors(1)
    lines(c(XMIN:(XMIN+length(plotdata[XMIN:XMAX,])-1)), 
      plotdata[XMIN:XMAX,], col=plotcolors)
    points(c(XMIN:(XMIN+length(plotdata[XMIN:XMAX,])-1)), 
      plotdata[XMIN:XMAX,], pch=18, col=plotcolors, cex=1.25)
    ptype <- 18
  }

  # If LEGEND is not a logical value, then it has 
  # either been specified by the user or its value 
  # was TRUE and we used the default values.  
  if (is.logical(LEGEND) == FALSE) {
    # Determine where to place the legend.  The function
    # TranslateLegendLoc returns a vector of 2 values, 
    # the first being the xaxis coordinate and the second
    # being the y-axis coordinate. 
    legendxy <- TranslateLegendLoc(LEGENDLOC, usr)

    # We want the legend to have both lines and characters representing the
    # different lines plotted.  pch controls the types of characters, 
    # and lty controls the lines.  We want the legend to be printed
    # in the same order as the lines.  Generally, this means reversing the
    # legend. 
    LEGEND <- rev(LEGEND)
    ptype <- rev(ptype)
    linecol <- rev(plotcolors)
    
    legend (legendxy[1], legendxy[2], legend=LEGEND, pch=ptype, 
      lty=rep(1,length(LEGEND)), col=linecol, 
      xjust=legendxy[3], yjust=legendxy[4])
  }

  # Add title and comments to chart
  title(main=TITLE,  xlab=XLABEL, ylab=YLABEL)
}

  
#####################################################################
# Name: PlotBarChart
# Purpose:  To plot a bar chart of given data.frame, matrix, 
#            or vector.  
#
#           If a vector is provided:
#            The function prints a simple bar chart of the 
#            entries in the vector. The labels assigned to 
#            each bar can be provided using the XLABELS argument.  
#        
#            If a matrix or data.frame is provided:
#            By default, the function will print a stacked
#           bar for each column of the matrix, resulting in a 
#           graph with #cols stacked bars.  Each entry in a 
#           stacked bar represents one row of the column of data.  
#           There are a number of ways to modify the default 
#           representation. 
#           1.) To print a chart where each row represents a
#               stacked bar and each column represents one entry
#               in the stack, set the flag TRANSPOSE=TRUE.
#           2.) To print a bar chart without stacking, set 
#                STACKED=FALSE.  
#           
# Inputs: 
#  DATA: Input matrix, data.frame or vector containing data to plot
# BARLABELS: A vector or names to use for the X axis.  If no labels
#          are provided, then the function uses the column labels 
#          associated with the matrix (if TRANSPOSE = FALSE) or
#          the row labels associated with matrix (if TRANSPOSE=TRUE).
#          If no labels are provided with the matrix or vector, 
#          then labels are assigned from 1 to the number of bars. 
# TRANSPOSE: A boolean flag which applies only to matrices or
#           data.frames.  If this flag is set, then the matrix is
#          transposed before the data is plotted.  Default is FALSE.
# STACKED: A boolean flag which again applies only to matrices or
#          data.frames.  If set, a stacked bar is plotted.  If not
#          set, then clusters of bar charts are plotted, with one 
#           cluster per column or row dependent on the TRANSPOSE 
#           flag.  The default is TRUE.   
# XLABEL, YLABEL, TITLE:  Labels for x, y axis and title. 
# LEGEND: Legend when input is a matrix or data.frame.  This is
#          not available for vector inputs.  This is a vector
#          containing the names to use for each contribution 
#          to a stacked bar chart (if STACKED = TRUE) or each
#          bar in the cluster of bar charts (if STACKED = FALSE).  
#          If not set, the legend defaults to the row labels 
#          (if TRANSPOSE = FALSE) or col labels 
#          (if TRANSPOSE = TRUE).        
# LEGENDLOC: Location where legend is to be placed.  Specify a 2 entry
#          vector where the values of the vector are 1, 2, or 3.  
#          For X coordinates, 1->3 mean left, center, right position
#          For Y coordinates, 1->3 mean bottom, center, top position
#          Default is center, center or c(2,2)    
# CUMULATIVE: If TRUE, it accumulates data as the graph moves across the 
#	   X axis.  This is opposite of STACKED, which sums up all
#          Y values for a particular value of X.  If CUMULATIVE is 
#          true, then the value at a location of X is the sum of the
#          values at that location plus all preceding locations. 
#####################################################################
PlotBarChart <- function (DATA, BARLABELS=c(), 
        TRANSPOSE=FALSE, STACKED=TRUE,
        XLABEL="xlabel", YLABEL="ylabel", 
        TITLE="title", LEGEND=TRUE,  LEGENDLOC=c(2,2), 
	CUMULATIVE = FALSE, 
        YMAX=c(0)) {

  plotdata <- DATA

  if (length(plotdata) == 0) {
    stop("matrix or vector cannot be size 0")
  }

  if (is.data.frame(plotdata)) {
    # We can't work with data.frames directly, so convert
    # this to matrix. 
    plotdata <- as.matrix(plotdata)
  }

  #
  # Check labels for bars. 
  #
  if (length(BARLABELS) == 0) {
    # No bar labels specified.  Therefore, use column or 
    # row labels, depending on whether TRANSPOSE is set or 
    # not. 
    if (TRANSPOSE == FALSE) {
      if (is.vector(DATA) == TRUE) {
        BARLABELS <- labels(DATA)
      }
      else {
        BARLABELS <- labels(DATA)[[2]]
      }
    }
    else {
      if (is.vector(DATA) == TRUE) {
        BARLABELS <- labels(DATA)
      }
      else {
        BARLABELS <- labels(DATA)[[1]]
      }
    }
  }
  else {
    if (TRANSPOSE == FALSE && (length(BARLABELS) != ncol(plotdata))) {
      stop("Num of bar labels don't match num of cols in matrix")
    }
    if (TRANSPOSE == TRUE && (length(BARLABELS) != nrow(plotdata))) {
      stop("Num of bar labels don't match num of rows in matrix")
    }
  }

  #
  # check legend    
  if (is.matrix(plotdata)) {
    # If LEGEND is specified as a boolean value, then use
    # the default specifications if LEGEND = TRUE
    if (is.logical(LEGEND) == TRUE) {
      if (LEGEND == TRUE)  {    
        # No legend specified but we want to print legend. 
        if (TRANSPOSE == FALSE) {
          LEGEND <- labels(DATA)[[1]]
        }
        else {
          LEGEND <- labels(DATA)[[2]]
        }
      }
    }
    else {
      # LEGEND is specified as a non-boolean value.  Therefore, 
      # check the names.  
      if (TRANSPOSE == FALSE && (length(LEGEND) != nrow(plotdata))) {
        stop("Num of legend labels don't match num of rows in matrix")
      }
      if (TRANSPOSE == FALSE && (length(LEGEND) != ncol(plotdata))) {
        stop("Num of legend labels don't match num of cols in matrix")
      }
    }
  }

  if (TRANSPOSE == TRUE) {
    plotdata <- t(plotdata)
  }

  #
  # Determine if we need to plot cumulative data.  If so, we 
  # add columns to each other.  Column 2 is the sum of Column
  # 2 + column 1, column 3 = column 1 + 2 + 3, and so on. 
  if (CUMULATIVE == TRUE) {
    for (i in 2:ncol(plotdata)) {
      plotdata[,i] <- plotdata[,(i-1)] + plotdata[,i]
    }
  }

  # Determine the maximum Y value for the bar charts.  If we   
  # have a stacked barchart, then the maximum is the sum of 
  # the columns of plotdata.  If we have a non-stacked bar chart,
  # then the maximum is max value in each row. 
  if (is.vector(plotdata) == TRUE || STACKED == FALSE) {
    Ymaxval <- max(plotdata)
  }
  else {
    # We have a stacked matrix, which means that we need to 
    # evaluate the max of every col to determine the maxvalue
    # to plot on bar chart. 
    Ymaxval <- c(0)
    for (i in 1:ncol(plotdata)) {
      m <- sum(plotdata[,i])
      if (Ymaxval < m) {
        Ymaxval <- m
      }
    }
  }

  # Set the min value to be a very small number less than 0.  
  # Otherwise, we don't see a 0 on graph. 
  Yminval <- c(-0.001)
  
  # The max Y value can be superceded by the user defined YMAX
  if (YMAX > 0) {
    Ymaxval <- YMAX
  }
  else {
    # Set the max value to be slightly greater than the max. 
    # Otherwise, the chart touches the top of the max bar
    Ymaxval <- Ymaxval * 1.10
  }

  # 
  # Generate a list of colors with possibly repeating 
  # colors which has the same number of entries as the 
  # of rows in plotdata. Colors might need to repeat 
  # because most machines have a fixed color set. 
  #
  col <- AsimColors(nrow(plotdata), PTYPE="BAR")

  # Check the number and size of elements for the X axis
  # If there are too many characters, or if any label is 
  # too long, then we plot them perpendicular to axis. 
  # Actually, if the labels get really long (like when the 
  # x-axis represents numbers), then we shouldn't try to 
  # to plot them vertically since it looks like garbage. 
  if (length(BARLABELS) < 20) {
    if (sum(nchar(BARLABELS)) > 40 || max(nchar(BARLABELS) > 10)) {
      par(las = 2)  
    }
  }
    
  #
  # Plot data
  # Note that I repeat colors because X complains if 
  # I use too many colors. 
  barplot(plotdata, names.arg = BARLABELS, 
      beside = !STACKED, 
      ylim = c(Yminval, Ymaxval), 
      col = col, 
      xlab = XLABEL, ylab = YLABEL, main = TITLE) 

  # Reset las to plot parallel to axis. 
  par(las = 0)

  # Draw a box around this 
  usr<-par("usr")
  rect(usr[1], usr[3], usr[2], usr[4])

  # We can't use the default legend creation capability in 
  # barplot since it puts the legend box in the corner and
  # covers up existing data.  Therefore, I create my own
  # legend box. 

  # We also have to reverse the way legend plots out 
  # the names and colors.  The stack bar and legend 
  # are in reverse order.  Therefore, to get them to 
  # match, the legend has to be reversed. 
  
  if (STACKED == TRUE) {
    legendcol <- rev(col)
    LEGEND <- rev(LEGEND)
  }
  else {
    legendcol <- col
  }

  if (is.logical(LEGEND) == FALSE) {
    legendxy <- TranslateLegendLoc(LEGENDLOC, usr)
    legend (legendxy[1], legendxy[2], legend = LEGEND,
      fill = legendcol, 
      xjust = legendxy[3], yjust = legendxy[4])
  }
}


#####################################################
#####################################################
############                          ###############
############  GENERIC MISS FUNCTIONS  ###############
############                          ###############
#####################################################
#####################################################

#####################################################################
# Name: SaveHardCopy
# Function: This saves a hard copy (in postscript format) of a 
#   plot.  The plot saved is either the on which is currently 
#   active (this is the default), or the one requested by the user. 
#   The user provides the names of the plot
# Inputs: 
#    1.) name of postscript file.
#    2.) which device to print (optional)
# Output: NONE
#####################################################################
SaveHardCopy <- function (name, PLOTDEV = dev.cur()) {
  current <- dev.cur()
  dev.set(PLOTDEV)
  dev.print(postscript, file=name, horizontal = FALSE, 
            width=10.0, height=8.0)
  if (dev.cur() != current) {
    dev.set(current)
  }
}

#####################################################################
# Name: TranslateLegendLoc
# Function: Translates a x, y location coordinate given as values 1,
#   2, or 3 into positions on graph.  It returns the X and Y locations
#   along with the X and Y justifications for the legend. If no location
#    is specified, then it asks the user to mark the location on the plot.
#
# Inputs: 
#    XYLOC: A vector of 2 values in the range of 1 to 3
#    PLOTDIM: Dimensions of plot. 
# Defaults: NONE
#####################################################################
TranslateLegendLoc <- function (XYLOC, PLOTDIM) {

  if (length(XYLOC) != 2) {
    # Ask the user where they want the legend. 
    dev <- dev.cur()
    cat ("\nSPECIFY LOCATION OF LEGEND ON DEVICE\n"); print (dev)
    points <- locator(1)

    # Justify the legend, based on where we're located.
    loc <- c(points[[1]], points[[2]], 0, 1)
    
    return (loc)
  }
  
  if (XYLOC[1] < 1 || XYLOC[1] > 3 || XYLOC[2] < 1 || XYLOC[2] > 3) {
    stop ("Illegal X Y Legend coordinates")
  }

  PLOTDIM <- abs(PLOTDIM)

  if (XYLOC[1] == 1 && XYLOC[2] == 1) {
    # Lower left side of plot
    loc <- c(PLOTDIM[1], PLOTDIM[3])
    loc <- c(loc, 0, 0)
  }
  if (XYLOC[1] == 1 && XYLOC[2] == 2) {
    # Left side, center of Y axis
    loc <- c(PLOTDIM[1], PLOTDIM[3] + ((PLOTDIM[4]-PLOTDIM[3])/2))
    loc <- c(loc, 0, .5)
  }
  if (XYLOC[1] == 1 && XYLOC[2] == 3) {
    # Left & top
    loc <- c(PLOTDIM[1], 0.95*PLOTDIM[4])
    loc <- c(loc, 0, 1)
  }
  if (XYLOC[1] == 2 && XYLOC[2] == 1) {
    # Center X, lower Y
    loc <- c(PLOTDIM[1] + ((PLOTDIM[2]-PLOTDIM[1])/2), PLOTDIM[3])
    loc <- c(loc, .5, 0)
  }
  if (XYLOC[1] == 2 && XYLOC[2] == 2) {
    # Center X, Center Y
    loc <- c((PLOTDIM[1] + (PLOTDIM[2]-PLOTDIM[1])/2), 
            (PLOTDIM[3] + (PLOTDIM[4]-PLOTDIM[3])/2))
    loc <- c(loc, .5, .5)
  }
  if (XYLOC[1] == 2 && XYLOC[2] == 3) {
    # Center X, top Y
    loc <- c(PLOTDIM[1] + ((PLOTDIM[2]-PLOTDIM[1])/2), 0.95*PLOTDIM[4])
    loc <- c(loc, .5, 1)
  }
  if (XYLOC[1] == 3 && XYLOC[2] == 1) {
    # Right X, Lower Y
    loc <- c(0.95*PLOTDIM[2], PLOTDIM[3])
    loc <- c(loc, 1, 0)
  }
  if (XYLOC[1] == 3 && XYLOC[2] == 2) {
    # Right X, Center Y
    loc <- c(0.95*PLOTDIM[2], PLOTDIM[3] + ((PLOTDIM[4]-PLOTDIM[3])/2))
    loc <- c(loc, 1, .5)
  }
  if (XYLOC[1] == 3 && XYLOC[2] == 3) {
    # Right X, top Y
    loc <- c(0.95*PLOTDIM[2], 0.95*PLOTDIM[4])
    loc <- c(loc, 1, 1)
  }

  # Adjust the locations a bit
  return (loc)
}
    
#################################################################
# Name: AddToMergeList
# Purpose: Adds new vectors onto a list of vectors. 
# Inputs: 
#    MERGELIST: Existing list of vectors or a boolean value of FALSE.
#               A boolean value means that there is no existing list
#               and the function should create one. 
#    COLS: New vector to add
# Defaults: 
#    MERGLIST = FALSE.  No existing list to add onto. 
# Outputs:
#    Modified MERGELIST with new vector added
#################################################################
#
# You can Add to the merge list multiple times. 
# cols contains the list of columns you want to merge.   
#
AddToMergeList <- function (COLS, MERGELIST=FALSE) {
  if (MERGELIST == FALSE) {
    # No list specified, so create a null list. 
    newlist <- list()
  }
  else {
    if (is.list(MERGELIST) == FALSE) {
      # We can only work on lists. 
      stop("We can only work on lists")
    }
    else {
      newlist <- MERGELIST
    }
  }

  if (length(COLS) < 1) {
    # No merge list specified
    return (newlist)
  }


  newlist[length(newlist) + 1] <- list(COLS)
  return (newlist)
}

#####################################################################
# Name: MassageLstData
# Function: Takes a list of data frames and massages the raw data
#    by taking subsets of the data and by summing the rows or cols, 
#    as requested. 
# Inputs: 
#    DATALST: List of input data frames
#    SUMROW: A boolean or vector or list of vectors defining how and
#        what rows to sum, if any. 
#    SUMCOL: A boolean or vector or list of vectors defining how and
#        what cols to sum, if any. 
#    ROWNAME: A vector of strings representing names of new rows 
#        created by the summing process. 
#    COLNAME: A vector of strings representing names of new cols
#        created by the summing process. 
#    SUBROW: A boolean or vector of min and max values of rows to 
#        grab from data frames. 
#    SUBCOL: A boolean or vector of min and max values of cols to 
#        grab from data frames. 
# Output: A list of data frames with the appropriate rows or columns
#       removed and the appropriate rows or columns summed up. 
#####################################################################
MassageLstData <- function (DATALST, SUMROW=FALSE, ROWNAME=c(), SUBROW=FALSE, 
                            SUMCOL=FALSE, COLNAME=c(), SUBCOL=FALSE) {

  if (is.list(DATALST) == FALSE) {
    stop("Input data must be a list")
  }
  
  mincol <- 1
  minrow <- 1
  maxcol <- ncol(DATALST[[1]])
  maxrow <- nrow(DATALST[[1]])
  # Did the user request subsets of rows? 
  if (is.logical(SUBROW) == FALSE) {
    if (length(SUBROW) != 2) {
      stop("Subset of rows for data frame not properly specified")
    }
    minrow <- SUBROW[1]
    maxrow <- SUBROW[2]
  }

  # How about columns? 
  if (is.logical(SUBCOL) == FALSE) {
    if (length(SUBCOL) != 2) {
      stop("Subset of cols for data frame not properly specified")
    }
    mincol <- SUBCOL[1]
    maxcol <- SUBCOL[2]
  }

  #
  # Now generate subsets
  if (is.logical(SUBROW) == FALSE || is.logical(SUBCOL) == FALSE)  {
    DATALST <- SubsetLstData(DATALST, MINCOL=mincol, MINROW=minrow, 
                            MAXCOL=maxcol, MAXROW=maxrow)
  }

  # 
  # Sum up columns, if requested. 
  if (is.logical(SUMCOL) == TRUE) {
    if (SUMCOL == TRUE) {
      # We want to sum up all columns in data frame. 
      mergelist <- list(c(1:ncol(DATALST[[1]])))
      DATALST <- SumColumns(DATALST, mergelist, COLNAME)
    }
  }
  else {
    #
    # WARNING: Have to check for list first and then vector
    # because a list shows up as both a vector and a list.    
    #
    if (is.list(SUMCOL) == TRUE) {
      DATALST <- SumColumns(DATALST, SUMCOL, COLNAME)
    }
    else { 
      if (is.vector(SUMCOL) == TRUE) {
        if (length(SUMCOL) == 1) {
          # We have a single column number, which means sum 
          # from this point to the end
          SUMCOL <- c(SUMCOL:ncol(DATALST[[1]]))
        }
        # We have a vector of columns to merge
        # SumColumns expects to see a list of vectors to 
        # merge, hence I'm "casting" SUMCOL to a list 
        # before passing it. 
        DATALST <- SumColumns(DATALST, list(SUMCOL), COLNAME)
      }
      else {
        stop("Can't handle the type of SUMCOL")
      }
    }
  }

  # 
  # Now do the same thing for rows
  if (is.logical(SUMROW) == TRUE) {
    if (SUMROW == TRUE) {
      # We want to sum up all columns in data frame. 
      mergelist <- list(c(1:nrow(DATALST[[1]])))
      DATALST <- SumRows(DATALST, mergelist, ROWNAME)
    }
  }
  else {
    if (is.list(SUMROW) == TRUE) {
      DATALST <- SumRows(DATALST, SUMROW, ROWNAME)
    }
    else {
      if (is.vector(SUMROW) == TRUE) {
        if (length(SUMROW) == 1) {
          # We have a single row number, which means sum 
          # from this point to the end
          SUMROW <- c(SUMROW:nrow(DATALST[[1]]))
        }
        # We have a vector of columns to merge
        # SumColumns expects to see a list of vectors to 
        # merge, hence I'm "casting" SUMROW to a list 
        # before passing it. 
        DATALST <- SumRows(DATALST, list(SUMROW), ROWNAME)
      }
      else {
        stop("Can't handle the type of SUMROW")
      }
    }
  }

  # return the final modified list of data frames
  return (DATALST)
  

}
                            


###########################################################
###########################################################
############                                ###############
############  PARAMETER PLOTTING FUNCTIONS  ###############
############                                ###############
###########################################################
###########################################################



#############################################################
# Name: GenericMungeData
# Purpose: Munges the data as requested for histograms with
#    one or more columns and >1 rows. 
# Inputs: 
#    PARAM: Name of parameter to munge
#    FPBENCHMARKS, INTBENCHMARKS: Vector of floating point benchmark 
#      names and INTEGER benchmark names to evaluate.  These are both
#      optional parameters.  The function uses these names to 
#     decide which benchmarks to consider.  It reads in all 
#      benchmarks in file <PARAM>.R and filters the benchmarks
#      based on FPBENCHMARKS and INTBENCHMARKS. If neither parameter 
#      is specified, then all benchmarks in file <PARAM>.R are used.
#      Also, if these flags are not specified, then it is impossible
#      for the program to decide how to categorize the benchmarks
#      according to integer and floating point behavior. 
#   INPATH: Path to location of files of type <Stats Name>.R  
#      Optional parameter.  If not specified, it assumes that files
#      are located in the local directory. 
#    OUTPATH: Path where output files (the munged data and 
#        postscript files) are to be dumped.  If not specified,
#        it assumes these files are to be dumped in the same 
#        directory as input files.  
#    SUMCOL, SUMROW: Can have a boolean value of TRUE or FALSE, or
#          can be a vector of cols (rows) to sum, or can be a 
#          list of vectors consisting of rows (cols) to sum.  
#          1.) If FALSE, then no summing of rows or columns is done.
#          2.) If TRUE, then all the rows or all the columns are summed
#              into one value.  
#          3.) If a vector is given, then the given rows or columns 
#              are summed and a new row or column is added to each 
#              data.frame which is the sum.  The summed rows and 
#              columns are then deleted.  Finally, the new column 
#              or row is given the name provided by SUMCOLNAME 
#              or SUMROWNAME, respectively.  
#
#              a.) If the vector contains one value, then all rows 
#                  (columns) from that point to the maximum row (column)
#                  are summed. 
#              b.) If the vector contains two values, then the first 
#                  is the minimum row (col) and the second is the maximum
#                  row (col) to sum.  
#          4.) If a list of vectors is given, then step (3) is 
#              repeated for every vector in the list of vectors. 
#              The number of names provided in SUMCOLNAME or 
#              SUMROWNAME must match  the number of vectors in the 
#              list of vectors.  Furthermore, if one of the vectors
#              in the list contains one row (col), then that row
#              (col) to the max row (col) are summed up as noted 
#              in 3a.  
#
#          If SUMCOL and SUMROW are both defined, then the summing 
#          of columns takes place first, and then the summing of rows.  
#          (Not that it makes much difference which order this happens)
#
#          WARNING, WARNING, WARNING:  Summing of columns and rows takes
#            place on raw data BEFORE the mean is generated.  
#
#    SUMCOLNAME, SUMROWNAME: Vector of strings representing the names 
#          to assign to the new columns or rows created by SUMCOL 
#          or SUMROW.  These parameters are ignored if SUMCOL and or
#          SUMROW is set to FALSE.  
#    SUBSETCOL, SUBSETROW: This parameter can either be a boolean 
#          value of FALSE, or a vector of min and max cols (rows) 
#          which you want to grab from the input data.  This allows 
#          the user to look at only a portion of the data.  
#          
#          WARNING, WARNING, WARNING: Subsetting of rows or columns 
#            happens BEFORE any summing of columns or rows. 
#
#    NORM: A boolean flag if set to TRUE will normalize the data in each 
#          benchmark.  Then the means calculated will give equal weight
#           to each benchmark.   
#
#    SAVEDATA: A boolean flag if set will save the values for 
#          every benchmark and for the mean of all benchmarks into a 
#          file. If both floating point and integer benchmarks are 
#          specified, then the program will generate three mean files: 
#          INTmean, FPmean, and ALLmean.  
#
#          The file is called <STATS NAME>.[raw|norm], depending on 
#          whether NORM=TRUE or not. 
#
# Defaults:
#    FPBENCHMARKS=c()
#    INTBENCHMARKS=c()
#    INPATH="./"
#    OUTPATH=INPATH
#    SUMCOL=FALSE
#    SUMROW=FALSE
#    SUBSETCOL=FALSE
#    SUBSETROW=FALSE
#    SUMCOLNAME=c()
#    SUMROWNAME=c()
#    NORM = TRUE
#    SAVEDATA = TRUE
# Outputs: 
#    A list of data frames, one for each benchmark in integer benchmarks, 
#    one for each benchmark in FP benchmarks, and mean data for FP, INT, 
#    and all benchmarks.  If either FP or INT are not defined, then their
#    corresponding MEAN benchmark will not exist along with the overal MEAN
#    results. 
#
#    Furthermore, if SAVEDATA flag is set, then each benchmark and the 
#    available means will be dumped out into files.  
#
###################################################################################
GenericMungeData <- function (PARAM, INTBENCHMARKS = c(), FPBENCHMARKS = c(), 
                    INPATH="./", OUTPATH=INPATH, 
                    SUMCOL=FALSE,                 # Sum a given vector of columns
                    SUMCOLNAME=c(),               # Names of summed columns
                    SUBSETCOL=FALSE,              # Truncate a given vector of cols
                    SUMROW=FALSE,                  # Sum the give vector of rows
                    SUMROWNAME=c(),               # Names of summed rows
                    SUBSETROW=FALSE,               # Get a subset of rows. 
                    NORM=TRUE,                     # Generate normalized data
                    SAVEDATA=TRUE                  # Save data for all benchmarks. 
                  ) {


  outlst <- list()
  meanlst <- list()

  # Step 1: Read the list of benchmarks in <PARAM>.R file. 
  inlst <- ReadParamData(PARAM, INPATH)

  if (length(INTBENCHMARKS) == 0 && length(FPBENCHMARKS) == 0) {
    # We don't have any benchmarks specified, so collect information 
    # on all parameters. 
    bmlst <- list(names(inlst));
    names(bmlst) <- c("ALL")
  }
  else {
    bmlst <- list(INTBENCHMARKS, FPBENCHMARKS)
    names(bmlst) <- c("INT", "FP")
  }

  for(i in 1:length(bmlst)) {
    bmnames <- bmlst[[i]]

    if (length(bmnames) != 0) {
      # Read input data
      # datalst contains a number of data frames representing
      # either the FP or INT benchmarks.  If neither were 
      # specified, then it contains the info for one benchmark
      # defined by the PARAM name. 
      datalst <- FilterParamData (inlst, bmnames)
  
      # Munge data
      # Note that subsets of data frames are generated
      # before the summing of columns or rows occurs. 
      datalst <- MassageLstData(datalst, SUMROW=SUMROW, 
                          ROWNAME=SUMROWNAME, SUBROW=SUBSETROW, 
                          SUMCOL=SUMCOL, COLNAME=SUMCOLNAME, 
                          SUBCOL=SUBSETCOL) 
  
      if (NORM == TRUE) {
        # Normalize values in each data array in list
        # If the normalization routine finds that 
        # no normalization is possible becuase there
        # is only one single element, it returns a 
        # value of FALSE. 
        templst <- NormLst(datalst)
        if (templst != FALSE) {
          datalst <- templst
        }
        else {
          NORM <- FALSE
        }
      }
      outlst <- c(outlst, datalst)

      # Now generate the mean across all benchmarks
      # Save the means in a separate list for now. 
      # Do this only if the number of entries in list
      # is greater than one. 
      if (length(datalst) > 1) {
        meanheader <- names(meanlst)
        meanlst <- c(meanlst, list(GenLstMean(datalst)))

        # Add new header name for mean of INT or FP benchmarks
        n <- paste(names(bmlst)[i], "mean", sep="")
        names(meanlst) <- c(meanheader, n)
      }
    }
  }

  # Calculate the mean across all benchmarks if both FP and INT
  # benchmarks were requested. 
  if (length(meanlst) == 2) {
    # We need to generate the mean over all benchmarks which 
    # are now located in outlst. 
    meanheader <- names(meanlst)
    meanlst <- c(meanlst, list(GenLstMean(outlst)))
      
    names(meanlst) <- c(meanheader, "ALLmean")
  }

  # Attach the mean values to the individual benchmarks
  outlst <- c(outlst, meanlst)
  
  # Write output data if requested. 
  # The data is written in ASCII format.  Although
  # this is not the most compact form, it seems the
  # safer thing to do. 
  if (SAVEDATA == TRUE) {
    # Data gets saved in file called
    # <OUTPATH>/<param>.<norm|raw>
    name <- paste(OUTPATH, PARAM, sep="/")
    if (NORM == TRUE) {
      name <- paste(name, "norm", sep=".")
    }
    else {
      name <- paste(name, "raw", sep=".")
    }
    save(outlst, file=name, ascii=TRUE)
  }
  return(outlst)
}

#############################################################
# Name: GenericPlotData
# Purpose: Plots data in either histogram or line form.   The
#    function can either take a list of data frames as input
#    data, or it can read the data from the INPATH directory. 
# Inputs: 
#    DATALST: This input can any one of the following values:
#      1.) One of two string either "norm" or "raw". 
#      2.) List of data frames containing information to 
#          be plotted.  Each data frame either represents one benchmark
#          or the means of different benchmarks.  The names of the
#          entries in the list are the names of the benchmarks or 
#          "INTmean", "FPmean", or "ALLmean".  
#
#      WARNING, WARNING, WARNING: The DATALST should be created
#        by GenericMungeData function.  If you don't want to 
#        regenerate this information, then then let the function find
#        the raw data from INPATH directory. 
#
#   PARAM: Name of parameter to plot
#   BENCHMARKS: Vector of benchmarks to evaluate.  If you used 
#      GenericMungeData function to generate the DATALST, then 
#      the benchmarks available to you are the integer and fp 
#      benchmarks specified to GenericMungeData, and the 
#      data for INTmean, FPmean, and ALLmean.  Note that ALLmean
#      will exist only if you specified both integer and fp 
#      benchmarks to GenericMungeData. If no benchmarks are specified, 
#      then all benchmarks in DATALST is plotted. 
#   INPATH: Path to location of files of type <STAT name>.[raw|norm]. 
#        If DATALST is defined, then INPATH is ignored.  If
#        DATALST is not defined, then the function searches 
#        the INPATH directory for the given file types. 
#   OUTPATH: Path where output files (postscript files)
#        are to be dumped.  
#   BYROW: Specifies whether the data should be looked at by
#        row or by column. If TRUE, the histograms plotted will 
#        have accumulation by each row.  In other words, each row 
#        will refer to one  bar in plot for a stacked histogram. 
#        If STACKED=FALSE, then each row will represent one cluster
#        of histograms. If BYROW=FALSE, then each col represents
#        one bar in plot.  In the case of line plots, if TRUE, each line
#        represents one row of information, and the number of lines
#       in plot equals the number of rows. If FALSE, then each column
#        equals one line and the number of lines equals the number 
#        of colums. 
#    PTYPE: If BAR, it plots a bar plot.  If LINE, it plots a line plot
#    STACKED: If TRUE, and PTYPE=BAR, then the output will be a 
#          stacked bar chart.  Else all entries of bar will be plotted 
#          besides each other.  If TRUE and PTYPE=LINE, then 
#          it will produce a line chart where the lines never cross each
#          other.  
#    CUMULATIVE: If TRUE, then the data is accumulates as X axis 
#	   increases. 
#    TITLES: A vector of 3 strings, where the first string is the Xaxis
#          label, second string is Y axis label, third string is graph title. 
#          If a graph title is omitted, then the <param name>:<benchmark name>
#          is used as the graph title. 
#    LEG: A boolean value or a vector of strings represeting the legend.  
#          If the value is FALSE, then no legend is printed. If value is           
#          a vector then the size of the vector must match the input data 
#          size in the following manner:
#            1.) If BYROW = TRUE and BARPLOT = TRUE: 
#                length(vector) must equal ncol(DATA)
#            2.) If BYROW = TRUE and BARPLOT = FALSE:
#                length(vector) must equal nrow(DATA)
#            3.) If BYROW = FALSE and BARPLOT = TRUE:
#                length(vector) must equal nrow(DATA)
#            4.) If BYROW = FALSE and BARPLOT = FALSE:
#                length(vector) must equal ncol(DATA)
#          
#    LEGLOC: Location of legend.  This is a vector of 2 values ranging
#       from 1 to 3.  The first number represents the left, center,
#       or right side of plot.  The second number represents the 
#       bottom, middle, or top of plot.  To make life easier, the
#       following coordinate macros will also work.  To specify
#       placing the legend in lower right, you can either use 
#       LEGLOC=c(3,1) or LEGLOC=SE.
#            1.) LEGLOC <- c(1,1): Place legend Left Lower of plot
#                Macro: SW
#            2.) LEGLOC <- c(1,2): Place legend Left Middle of plot
#                Macro: W
#           3.) LEGLOC <- c(1,3): Place legend Left Top of plot
#                Macro: NW
#           4.) LEGLOC <- c(2,1): Place legend Center Lower of plot
#                Macro: S
#           5.) LEGLOC <- c(2,2): Place legend Center Middle of plot
#                Macro: C
#           6.) LEGLOC <- c(2,3): Place legend Center Top of plot
#                Macro: N
#           7.) LEGLOC <- c(3,1): Place legend Right Lower of plot
#                Macro: SE
#           8.) LEGLOC <- c(3,2): Place legend Right Middle of plot
#                Macro: E
#           9.) LEGLOC <- c(3,3): Place legend Right Top of plot
#                Macro: NE
#
#        If no legend is specified and R is running in interactive
#        mode, then the user is asked to point to the location on 
#        graph where legend should be located.  A single left mouse
#        click will select the location. 
#    BARLABELS: A vector of labels associated with each bar.  Only 
#	 relavant for bar charts.  If the value is not defined, then
#        the names of the rows, columns, or benchmarks is used as 
#        the bar labels, depending on the type of plot being plotted. 
#    XMIN, XMAX, YMIN, YMAX: Limits on plot size.  XMIN, XMAX and YMIN 
#            has no meaning for bar charts, only for line plots.  
#    COMBINE: A boolean flag if true, combines all data requested onto 
#	     one plot. This flag is only valid when there is one column 
#            of data per benchmark. This is not valid for histograms 
#	     with multiple columns of data.  The input BENCHMARKS 
#	     determines which data will be plotted on the same graph.  
#	     If you include one of the mean data into the list of 
#	     benchmarks, then the data will be plotted along with the 
#            mean.  The order in which data appears is the order in which the
#            BENCHMARKS input is given. 
#    COMBINENAME: A string which is the name of the new combined plot.  This 
#            will be used in the title as in <GRAPH TITLE> : <COMBINENAME>
#            and will be the name of the postscript file generated for this 
#            plot <COMBINENAME>.<param>.ps.  
#    SAVEPLOT: A boolean value which is only valid when in interactive mode. 
#            If TRUE and in interactive mode, it will plot to screen
#            and save a PS file of the same plot.  The plots go in directory
#            OUTPATH.  By default, SAVEPLOT=TRUE
# Defaults:
#    PARAM: No Default
#    BENCHMARKS=All benchmarks in DATALST
#    DATALST="norm"
#    INPATH="./"
#    OUTPATH=INPATH
#    BYROW=TRUE if BARPLOT=TRUE, or BYROW = FALSE if BARPLOT=FALSE
#    PTYPE = BAR # Barplot by default
#    STACKED=TRUE if BAR plot, else FALSE. 
#    TITLES=c("XAXIS LABEL", "YAXIS LABEL", "GRAPH TITLE")
#    LEG: The defaults are based on other parameters. 
#            1.) If BYROW = TRUE and PTYPE = "BAR": legend = column names
#            2.) If BYROW = TRUE and PTYPE = "LINE": legend = row names
#            3.) If BYROW = FALSE and PTYPE = "BAR": legend = row names
#            4.) If BYROW = FALSE and PTYPE = "LINE": legend = column names
#    LEGLOC: c(), user has to pick location. 
#    COMBINE = FALSE (Plot each benchmark requested individually)
#    COMBINENAME = "PLOTALL"
#    SAVEPLOT = TRUE
#    CUMULATIVE = FALSE
#    BARLABELS = c()
# Outputs: 
#    PLOTS:
#        One plot for each benchmark in BENCHMARKS unless COMBINE=TRUE.
#        Then it prints one plot for all BENCHMARKS. 
# Returns:
#    A TRUE if the plot completed correctly, or a FALSE if it did not.     
#
#############################################################
GenericPlotData <- function (
                  PARAM,                          # Required inputs
                  BENCHMARKS=c(),               # By default, no benchmarks specified.
                  DATALST="norm",               # If a string of "norm" or 
                                                # "raw" is given, then find data in 
                                                # OUTPATH directory. 
                  INPATH="./", OUTPATH=INPATH,  # Location of input and output data
                  TITLES=c("XAXIS LABEL", "YAXIS LABEL", PARAM), 
                  BYROW=TRUE,                   # Plot data by row or column
                  PTYPE="BAR",                  # Barplot or line plot
                  STACKED=(if (PTYPE == "BAR") TRUE else FALSE),                  
                                                # Stack data if plot type is BAR. 
		  CUMULATIVE=FALSE,             # don't accumulate data. 
                  LEG=TRUE, LEGLOC=c(),         # Legend specific information
		  BARLABELS = c(), 		# Names for bar labels
                  XMIN=0, YMIN=0, 
                  XMAX=0, YMAX=0,               # Limits of plotted data
                  COMBINE=FALSE,                # Don't combine bm on one plot
                  COMBINENAME="PLOTALL",        # Name used for new plot
                  SAVEPLOT=TRUE
                ) {
  # Check titles
  if (length(TITLES) < 3) {
    stop("Titles are not provided correctly")
  }
  
  if (is.list(DATALST) == FALSE) {
    # Find data in file INPATH/<param>.norm
    filename <- paste(INPATH, PARAM, sep="/")
    filename <- paste(filename, DATALST, sep=".")
    load(filename)
    DATALST <- outlst
  }

  # Check to make sure we have some benchmarks to work with 
  if (length(BENCHMARKS) == 0) {
    # No benchmarks specified.  Therefore, we plot all benchmarks. 
    BENCHMARKS <- names(DATALST)
  }

  ####################
  # Combine benchmarks?
  ####################
  if (COMBINE == TRUE) {
    # Check to make sure that there is only one column in each benchmark. 
    if (ncol(DATALST[[1]]) > 1) {
      stop("Can't combine data because there are multiple columns for each benchmark")
    }
    else {
      # Create a new list with all the requested benchmarks. 
      # Take given list and create a new list which 
      # contains the data from the requested benchmarks.
      # I'm recasting this to a list so that I can use it
      # in the loop below (which expects a list)
      plotlst <- list(CatLstData(DATALST, 
	BENCHMARKS=BENCHMARKS))
      bm <- COMBINENAME
      names(plotlst) <- bm
    }
  }
  else {
    bm <- BENCHMARKS
    plotlst <- DATALST
  }
      
  res <- FALSE
  ####################
  # Plot benchmarks
  ####################
  datanames <- names(plotlst)
  for (i in bm) {
    # First check to see if we have data for this benchmark
    if ( length(grep(i, datanames)) == 0 ) {
      cat("\nBENCHMARK="); print(i)
      warning("DON'T HAVE DATA FOR BENCHMARK")
    }
    else {
      filename <- paste(i, PARAM, "ps", sep=".")
      filename <- paste(OUTPATH, filename, sep="/")
      if (interactive() == FALSE) {
        NewPSPlot(filename)
      }  
      else {
        NewXPlot()
      }
      title <- paste(TITLES[3], i, sep=" : ")
      if (PTYPE == "BAR") {
        PlotBarChart(plotlst[[i]], TRANSPOSE=BYROW, XLABEL=TITLES[1], 
          YLABEL = TITLES[2], TITLE=title, STACKED=STACKED,  
	  BARLABELS = BARLABELS, 
          LEGEND=LEG, LEGENDLOC=LEGLOC, YMAX=YMAX, CUMULATIVE=CUMULATIVE)
        if (SAVEPLOT == TRUE && interactive() == TRUE) {
          SaveHardCopy(filename)
        }
      }
      else if (PTYPE == "LINE") {
        PlotLines(plotlst[[i]], TRANSPOSE=BYROW, XLABEL=TITLES[1], 
          YLABEL=TITLES[2], TITLE=title, LEGEND=LEG, 
          LEGENDLOC=LEGLOC, STACKED=STACKED, XMAX=XMAX, YMAX=YMAX,
          XMIN=XMIN, YMIN=YMIN, CUMULATIVE=CUMULATIVE)
          res <- TRUE
        if (SAVEPLOT == TRUE && interactive() == TRUE) {
          SaveHardCopy(filename)
        }
      }
      else {
        stop("Unknown plot type.")
      }
    }
  }
  return (res)
}

