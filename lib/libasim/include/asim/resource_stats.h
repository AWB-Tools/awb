/*
 *Copyright (C) 2003-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
* *****************************************************************
* *                                                               *
* *    Copyright (c) Digital Equipment Corporation, 1998          *
* *                                                               *
* *   All Rights Reserved.  Unpublished rights  reserved  under   *
* *   the copyright laws of the United States.                    *
* *                                                               *
* *   The software contained on this media  is  proprietary  to   *
* *   and  embodies  the  confidential  technology  of  Digital   *
* *   Equipment Corporation.  Possession, use,  duplication  or   *
* *   dissemination of the software and media is authorized only  *
* *   pursuant to a valid written license from Digital Equipment  *
* *   Corporation.                                                *
* *                                                               *
* *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
* *   by the U.S. Government is subject to restrictions  as  set  *
* *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
* *   or  in  FAR 52.227-19, as applicable.                       *
* *                                                               *
* *****************************************************************
*/

/**
 * @file
 * @author Srilatha Manne
 * @brief
 */

#ifndef _HISTOGRAM_STATS_
#define _HISTOGRAM_STATS_

// generic
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

// ASIM core
#include "asim/ioformat.h"
#include "asim/stateout.h"

namespace iof = IoFormat;
using namespace iof;
using namespace std;

#define MAX_STRING_LENGTH 1024
#define SEPARATORS ","

//
// This file contains classes which allow the user to keep histogram
// stats for particular events or resource.  
//

// declaration of orphan routines
static UINT32 CountEntries(const char *s);

//
// This is the generic histogram stats class.  You can specify either binned
// or non-binned histograms.  A binned histogram will give you data for a 
// range of values rather than for a unique value. 
//
template<bool E = true>
class HISTOGRAM_TEMPLATE {
 private:
  INT32 rowSize;           // Size of each row for which we accumulate values. 
  INT32 colSize;           // Size of each col for which we accumulate values. 
  bool rowFlexcap;         // Flexible cap on max entries in row
  bool colFlexcap;         // Flexible cap on max entries in col
                           // pooled into maximum bin. 
  UINT32 maxRowsUsed;      // High water mark for rows actually holding data.
  UINT32 maxRowVal;        // This is the maximum row in histogram.  If
                           // we exceed this number, then pooled data must 
                           // be enabled or we trigger an assertion failure. 
  UINT32 maxColVal;        // This is the maximum col in histogram.  If
                           // we exceed this number, then pooled data must 
                           // be enabled or we trigger an assertion failure. 
  char *name;              // name of this histogram stat
  bool isSaveObj;          // indicates that this is a 'save'd object and
                           // its destructor needs to be careful not to
                           // free memory that it does not own (because
                           // the 'real' object owns and frees this memory)

 protected: 
  UINT64 **histData;       // histogram structure. 
  UINT64 *total;           // Total number of events in histogram
  bool enabled;            // Flag which notes whether this histogram
                           // stat should be collected or not. 
  bool tpuHist;            // Declares this to be a histogram with columns
                           // representing tpus. 
  UINT32 numCols;          // Number of columns in histogram array
  char **colNames;         // Names of columns
  char **rowNames;         // Names of rows
  UINT32 numRows;          // Max number of rows in histogram

  UINT64 *accumulated;     // Accumulated row values

 public:
  //
  // Constructors
  //
  HISTOGRAM_TEMPLATE () : enabled(E) {
      if (enabled == true) {
          Clear();
    /*
    cout << "NULL HISTOGRAM Constructor " << this << endl;
    cout << "HistData Addr: " << fmt_x(&histData) << endl;
    cout << "HistData Value: " << fmt_x(histData) << endl;
    cout << flush;
    */
      }
  }

  //
  // HISTOGRAM_TEMPLATE --
  //    row/col_size -- allow merging adjacent rows or columns into single
  //        buckets.
  //    row/col_flex_cap -- when true, cause row or column indices greater
  //        than the maximum to refer to the last entry.  That way the
  //        highest row or column accumulates for everything that doesn't
  //        fit in the histogram.
  //    skip_trailing_empty_rows -- when true the trailing rows in the
  //        histogram for which no data has been entered are skipped
  //        by Dump().
  //
  HISTOGRAM_TEMPLATE (UINT32 num_rows, UINT32 num_cols = 1, 
                      UINT32 row_size = 1, bool row_flex_cap = false,
                      UINT32 col_size = 1, bool col_flex_cap = false,
                      bool skip_trailing_empty_rows = false)
    : enabled(E) {
      if (enabled == true) {
          Clear();
      }
      Constructor(num_rows, num_cols, row_size, row_flex_cap, col_size, col_flex_cap, skip_trailing_empty_rows);
  }

  // do the actual work of the constructor.
  // Separated out, so we can call this explicitly after insantiating an array,
  // since ISO C++ only allows default constructors for array elements.
  void Constructor   (UINT32 num_rows, UINT32 num_cols = 1, 
                      UINT32 row_size = 1, bool row_flex_cap = false,
                      UINT32 col_size = 1, bool col_flex_cap = false,
                      bool skip_trailing_empty_rows = false)
  {
      if (enabled == true) {
          rowFlexcap = row_flex_cap;
          colFlexcap = col_flex_cap;
          
          ASSERT (row_size > 0, Name());
          rowSize = row_size;
          
          ASSERT (col_size > 0, Name());
          colSize = col_size;
          
          ASSERT (num_rows > 0, Name());
          numRows = num_rows;
          maxRowVal = (num_rows * rowSize) - 1;
          
          ASSERT (num_cols > 0, Name());
          numCols = num_cols;
          maxColVal = (num_cols * colSize) - 1;
          
          if (! skip_trailing_empty_rows)
          {
              maxRowsUsed = numRows;
          }

          /*
            cout << "HISTOGRAM Constructor 1: " << this << endl;
            cout << "HistData Addr: " << fmt_x(&histData) << endl;
            cout << "HistData Value: " << fmt_x(histData) << endl;
            cout << flush;
          */
          InitializeHistogram();
          /*
            cout << "HIST: " << fmt_x(this)
            << ", HistData: " << fmt_x(this->histData)
            << ", HistData[0]: " << this->histData[0] << endl;
          */
      }
  }

  /// free what we have allocated
  virtual ~HISTOGRAM_TEMPLATE () {
      if (enabled) {
          // note: 'save'd object do not own the memory that their *name
          // pointers are referencing since the copy constructor has not
          // duplicated them - so we must not free them here!
          if ( ! isSaveObj) {
              // name
              if (name) {
                  free (name);
              }

              // row names
              if (rowNames) {
                  for (UINT32 i = 0; i < numRows; i++)
                  {
                      if (rowNames[i]) {
                          delete [] rowNames[i];
                      }
                  }
                  delete [] rowNames;
              }

              // column names
              if (colNames) {
                  for (UINT32 i = 0; i < numCols; i++)
                  {
                      if (colNames[i]) {
                          delete [] colNames[i];
                      }
                  }
                  delete [] colNames;
              }
          }
          
          // histData
          if (histData) {
              for (UINT32 i = 0; i < numRows; i++)
              {
                  if (histData[i]) {
                      delete [] histData[i];
                  }
              }
              delete [] histData;
          }

          // total
          if (total) {
              delete [] total;
          }
          
          if (accumulated) {
              delete [] accumulated;
          }
      }
  }
      
  //
  // Prepare this 'save' object to be deleted correctly;
  // (just remember that the distructor has to be careful)
  //
  void PrepareSaveDelete(void) {
      if (enabled) {
          isSaveObj = true;
      }
  }

  //
  // Set name of the histogram
  //
  void SetName(char *n) {
      name = n;
  }
    
  //
  // Get name of the histogram
  //
  char* Name(void) {
    return name ? name : (char*) "unknown histogram name";
  }

  //
  // Initialize row name pointer array.  Because of the way histograms
  // are saved by the stats package using the copy operator you need
  // to have the array of pointers to names initialized before the
  // first time the histogram is copied.  (The copy happens as a side
  // effect soon after the histogram is registered.)  Use this call
  // if you intend to fill in row names later with RowName().
  //
  void RowNamesInit(void) {
      ASSERT (rowNames == NULL, this->Name());

      rowNames = new char*[numRows];
      for (UINT32 i = 0; i < numRows; i++)
      {
          rowNames[i] = NULL;
      }
  }

  //
  // Declare names for the rows of the histogram.
  // Assumes that the last entry in names is "LAST".  If 
  // it sees this, then it knows that it has exceeded the 
  // boundary. 
  //
  void RowNames(const char * const * const row_names, bool checknames = true) 
  {
      if (enabled == true) {
          UINT32 i;
          
          ASSERT (rowSize == 1, this->Name());
          ASSERT (rowFlexcap == false, this->Name());
          if (rowNames == NULL)
          {
              RowNamesInit();
          }
          for (i = 0; i < numRows; i++)
          {
              ASSERT (!checknames || strcmp(row_names[i],"LAST") != 0, this->Name());
              rowNames[i] = new char[strlen(row_names[i]) + 1];
              strcpy (rowNames[i], row_names[i]);
          }
          maxRowsUsed = numRows;
          // We better see an entry called "LAST" or else we have 
          // more entries in our names list than rows in histogram. 
          ASSERT (!checknames || strcmp(row_names[i],"LAST") == 0, this->Name());
      }
  }
    
  //
  // A row name interface that allows specification of names one at a time...
  //
  void RowName(const char *row_name, UINT32 row)
  {
      if (enabled == true)
      {
          ASSERT(row < numRows, this->Name());
          if (rowNames == NULL)
          {
              RowNamesInit();
          }

          //
          // Overwriting old value?
          //
          if (rowNames[row] != NULL)
          {
              delete [] rowNames[row];
          }

          rowNames[row] = new char[strlen(row_name) + 1];
          strcpy(rowNames[row], row_name);

          if (row >= maxRowsUsed)
          {
              maxRowsUsed = row + 1;
          }
      }
  }

  //
  // Declare names for the cols of the histogram.  
  // If string is "LAST", then we're exceeding boundaries of
  // string array. 
  //
  void ColNames(const char **col_names) {

      if (enabled == true) {

          UINT32 i;
          
          ASSERT (colSize == 1, this->Name());
          ASSERT (colFlexcap == false, this->Name());
          colNames = new char*[numCols];
          for (i = 0; i < numCols; i++)
          {
              ASSERT (strcmp(col_names[i], "LAST") != 0, this->Name());
              colNames[i] = new char[64];
              strncpy (colNames[i], col_names[i], 63);
          }
          // We better see an entry called "LAST" or else we have 
          // more entries in our names list than rows in histogram. 
          ASSERT (strcmp(col_names[i], "LAST") == 0, this->Name());
      }
  }

 private:
  //
  // Initialize members
  //
  void Clear() {
      if (enabled == true) {
          // 
          // Clear objects
          //
          numRows = 0;
          rowSize = 0;
          rowFlexcap = false;
          maxRowVal = 0;
          maxRowsUsed = 0;
          
          colSize = 0;
          colFlexcap = false;
          numCols = 0;
          maxColVal = 0;
          
          tpuHist = false;
          isSaveObj = false;
          /*    
                cout << "CLEAR: tpuHist: " << tpuHist << endl;
                cout << flush;
          */
          // 
          // Clear pointers.
          //
          colNames = NULL;
          rowNames = NULL;
          name = NULL;
          histData = NULL;
          total = NULL;
          accumulated = NULL;
      }
  }
  
  //
  // count string names
  //
  UINT32 CountStringNames(const char *n) {
      if (enabled == true) {
          char copy[MAX_STRING_LENGTH];
          
          ASSERT(strlen(n) <= MAX_STRING_LENGTH-1, this->Name());
          strcpy (copy, n);
          return (this->CountEntries(copy));
      }
  }

  // 
  // Parse names and store in array. 
  //
  UINT32 ParseString(const char *n, char **names) {
      if (enabled == true) {
          char copy[MAX_STRING_LENGTH];
          char *s;
          int count;
          
          strcpy (copy, n);
          s = strtok(copy, SEPARATORS);
          count = 0;
          while (s != NULL) {
              names[count] = new char[64];
              
              // Check for 63 instead of 64 string length because of \0. 
              ASSERT (strlen(s) <= 63, this->Name());
              // cout << "Name in Bar Chart: " << s << endl;
              strcpy (names[count], s);
              count++;
              s = strtok(NULL, SEPARATORS);
          }
          return count;
      }
  }
    
  //
  // Initialization routine. 
  // 
  void InitializeHistogram() {
      if (enabled == true) {
          UINT32 i, j;
          
          // Allocate memory for histogram
          /*
            cout << "HistData Addr: " << fmt_x(&histData) << endl;
            cout << "HistData Value: " << fmt_x(histData) << endl;
            cout << flush;
          */
          ASSERT (histData == NULL, this->Name());
          ASSERT (total == NULL, this->Name());
          ASSERT (accumulated == NULL, this->Name());
          ASSERT (numRows != 0, this->Name());
          ASSERT (numCols != 0, this->Name());
          histData = new UINT64*[numRows];
          total = new UINT64[numCols];
          accumulated = new UINT64[numCols];
          
          for (i=0; i<numCols; i++) {
              total[i] = 0;
              accumulated[i] = 0;
          }
          
          //    cout << "INITIALIZING " << fmt_x(histData)
          //         << ": Rows: " << numRows
          //         << ", Cols: " << numCols << endl;
          
          //
          // Initialize data members. 
          // Note we always initialize to maxBins + 1 because
          // we have bins labeled "0" through "maxBins"
          //
          for (i = 0; i < numRows; i++) {
              histData[i] = new UINT64[numCols];
              // cout << "Row: " << i << ", Addr: " << fmt_x(histData[i]) << endl;
              // cout << flush;
              for (j = 0; j < numCols; j++) {
                  histData[i][j] = 0;
              }
          }
      }
  }
 public:
  //
  // Override = operator.  This method is used primarily to copy saved
  // data to collected data and vice-versa in the stats package.  When
  // STATS are off, a snapshot of the current histogram is saved, and
  // the histogram is allowed to collect stats. When STATS are turned
  // on again, the histogram is overwritten with the saved copy of the
  // histogram.  This way, the stats which were collected when STATS
  // was off don't show up in the results.  With the histogram class,
  // the stats which change are the histData structure, and the pooled
  // and total data information.  Everything else should remain the
  // same (Ex: number of rows, number of cols, bin size, col names,
  // row names, etc).
  //
  const HISTOGRAM_TEMPLATE &operator=(const HISTOGRAM_TEMPLATE &save) {

      if (enabled == true) {
          // 
          // They two classes must have the same
          // size histogram, or the new class has to have a NULL
          // histogram. 
          //
          UINT32 i, j;
          
          //
          // This case occurs the first time when we save a snapshot of
          // histogram to the "save" member in stats.h.  Since save was
          // initialized to be a NULL histogram, it will not have an array
          // allocated.
          
          //    HISTOGRAM_TEMPLATE<1> *t = (HISTOGRAM_TEMPLATE<1> *)(0x1402a74c0);
          //    cout << "QUEUEFULL: " << fmt_x(t)
          //         << ": Rows: " << t->numRows
          //         << ", Cols: " << t->numCols
          //         << ", binSize: " << t->binSize
          //         << ", maxBin: " << t->maxBin << endl;
          //    cout << flush;
          if (histData == NULL) {
              //
              // We're copying to a uninitialized instance of HISTOGRAM.
              // 
              histData = new UINT64*[save.numRows];
              ASSERT (total == NULL, this->Name());
              ASSERT (accumulated == NULL, this->Name());
              total = new UINT64[save.numCols];
              accumulated = new UINT64[save.numCols];
              
              for (i = 0; i < save.numRows; i++) {
                  histData[i] = new UINT64[save.numCols];
              }
              
              numRows = save.numRows;
              maxRowsUsed = save.maxRowsUsed;
              numCols = save.numCols;
              rowSize = save.rowSize;
              colSize = save.colSize;
              maxRowVal = save.maxRowVal;
              maxColVal = save.maxColVal;
              rowFlexcap = save.rowFlexcap;
              colFlexcap = save.colFlexcap;
              enabled = save.enabled;
              rowNames = save.rowNames;
              colNames = save.colNames;
              tpuHist = save.tpuHist;
              /*
                cout << "COPY: tpuHist: " << tpuHist << endl;
                cout << flush;
                cout << "New Data." << endl;
                cout << "SAVED: " << fmt_x(&save)
                << ": Rows: " << save.numRows
                << ", Cols: " << save.numCols
                << ", binSize: " << save.binSize
                << ", maxBin: " << save.maxBin << endl;
                cout << "new: " << fmt_x(this)
                << ": Rows: " << numRows
                << ", Cols: " << numCols
                << ", binSize: " << binSize
                << ", maxBin: " << maxBin << endl;
                cout << flush;
              */
          }
          else {
              /*
                cout << "Existing Data." << endl;
                cout << "SAVED: " << fmt_x(&save)
                << ": Rows: " << save.numRows
                << ", Cols: " << save.numCols
                << ", binSize: " << save.binSize
                << ", maxBin: " << save.maxBin << endl;
                cout << "new: " << fmt_x(this)
                << ": Rows: " << numRows
                << ", Cols: " << numCols
                << ", binSize: " << binSize
                << ", maxBin: " << maxBin << endl;
                cout << flush;
              */
              ASSERT (numRows == save.numRows, this->Name());
              ASSERT (numCols == save.numCols, this->Name());
              ASSERT (rowSize == save.rowSize, this->Name());
              ASSERT (colSize == save.colSize, this->Name());
              ASSERT (maxRowVal == save.maxRowVal, this->Name());
              ASSERT (maxColVal == save.maxColVal, this->Name());
              ASSERT (rowFlexcap == save.rowFlexcap, this->Name());
              ASSERT (colFlexcap == save.colFlexcap, this->Name());
              ASSERT (enabled == save.enabled, this->Name());
              ASSERT (rowNames == save.rowNames, this->Name());
              ASSERT (colNames == save.colNames, this->Name());
              ASSERT (tpuHist == save.tpuHist, this->Name());
          }
          //    cout << "QUEUEFULL: " << fmt_x(t)
          //         << ": Rows: " << t->numRows
          //         << ", Cols: " << t->numCols
          //         << ", binSize: " << t->binSize
          //         << ", maxBin: " << t->maxBin << endl;
          //    cout << flush;
          //
          // Copy data which changes while the stats are not being
          // collected.  
          //
          for (i = 0; i < numRows; i++) {
              for (j = 0; j < numCols; j++) {
                  histData[i][j] = save.histData[i][j];
              }
          }
          
          for (i = 0; i < numCols; i++) {
              total[i] = save.total[i];
              accumulated[i] = save.accumulated[i];
          }
          
          maxRowsUsed = save.maxRowsUsed;

          /*
            cout << "End of Copy function." << endl;
            cout << "SAVED: " << fmt_x(&save)
            << ": Rows: " << save.numRows
            << ", Cols: " << save.numCols
            << ", binSize: " << save.binSize
            << ", maxBin: " << save.maxBin << endl;
            cout << "new: " << fmt_x(this)
            << ": Rows: " << numRows
            << ", Cols: " << numCols
            << ", binSize: " << binSize
            << ", maxBin: " << maxBin << endl;
            cout << flush;
          */
      }
      return *this;
  }

  //
  // HISTOGRAM MODIFICATION METHODS
  //
  
  //
  // If you know that you have unit size bins, then you should use this
  // method because it's faster. 
  //
  void AddEvent(UINT32 row_val, UINT32 col_val = 0, UINT64 value = 1) {
      if (enabled == true) {
          // Profiling showed updating histogram entries to be slow.  Prefetch.
          __builtin_prefetch(&histData[row_val][col_val], 1, 1);

          ASSERT ((rowSize == 1) && (colSize == 1), this->Name());
          if (row_val >= numRows) {
              ASSERT (rowFlexcap == true, this->Name());
              row_val = numRows - 1; 
          }
          if (row_val >= maxRowsUsed)
          {
              maxRowsUsed = row_val + 1;
          }
          if (col_val >= numCols) {
              ASSERT (colFlexcap == true, this->Name());
              col_val = numCols - 1;
          }
          total[col_val] += value;
          accumulated[col_val] += row_val;
          histData[row_val][col_val] += value;
      }
  }
    
  //
  // DON'T USE THIS METHOD UNLESS YOU HAVE NON-UNIT SIZE ROWS AND
  // COLS.  IF YOU HAVE UNIT SIZE ROWS AND COLS, THEN YOU SHOULD USE
  // AddEventUnitBins.
  //
  void AddEventWideBins(UINT32 row_val, UINT32 col_val = 0, UINT64 value = 1) {
      if (enabled == true) {
          INT32 row_number = row_val;
          INT32 col_number = col_val;
          
          if (rowSize != 1) {
              row_number = (INT32)(floor((double)(row_number)/(double)(rowSize)));
          }
          
          if (colSize != 1) {
              col_number = (INT32)(floor((double)(col_number)/(double)(colSize)));
          }
          
          if (row_number >= INT32(numRows)) {
              if (rowFlexcap == true) {
                  row_number = numRows - 1;
              }
              else {
                  VERIFY(false, "Exceeding number of rows of histogram");
              }
          }
          if (row_val >= maxRowsUsed)
          {
              maxRowsUsed = row_val;
          }
          
          if (col_number >= INT32(numCols)) {
              if (colFlexcap == true) {
                  col_number = numCols - 1;
              }
              else {
                  VERIFY(false, "Exceeding number of cols of histogram");
              }
          }
          
          histData[row_number][col_number] += value;
          total[col_number] += value;
          accumulated[col_val] += row_val;
      }
  }

  // iterator for column names
  class columnRangeIterator {
    private:
      UINT32 current;
      UINT32 delta;
    public:
      /// constructor; defaults to single step, full range
      columnRangeIterator (UINT32 c = 0, UINT32 d = 1)
      {
          current = c;
          delta = d;
      }
      /// post-increment operator
      columnRangeIterator & operator++ (int)
      {
          current += delta;
          return *this;
      }
      /// check if two iterators are at different points
      bool operator!= (columnRangeIterator & c)
      {
          return (current != c.current);
      }
      /// dereference operator returns the range string
      string operator* (void)
      {
          ostringstream os;
          if (delta == 1) {
              os << current;
          } else {
	      os << current << "-" << current + delta - 1;
          }
          return os.str();
      }
  };

  //
  // Print information about the histogram. 
  //
  virtual void Dump(STATE_OUT stateOut) {
      if (enabled == true) {
          ostringstream os;
          
          UINT32 i, j;
          UINT64 count = 0;
          UINT32 nActualRows = numRows;
          UINT32 nRowsMax = maxRowVal;

          //
          // Simple histograms might drop rows at the end that hold no data.
          //
          if (rowSize == 1)
          {
              nActualRows = maxRowsUsed;
              if (maxRowsUsed == 0)
              {
                  nRowsMax = 0;
              }
              else
              {
                  nRowsMax = maxRowsUsed - 1;
              }
          }
          
          for (i=0; i<numCols; i++) {
              count += total[i];
          }
          if (tpuHist == true) {
              os << "TPU DATA" << endl;
          }
          stateOut->AddScalar("info", "rows", NULL, nActualRows);
          stateOut->AddScalar("info", "cols", NULL, numCols);
          stateOut->AddScalar("info", "max row", NULL, nRowsMax);
          stateOut->AddScalar("info", "max col", NULL, maxColVal);
          stateOut->AddScalar("info", "entries", NULL, count);
          
          if (count == 0) {
              // If there are no entries in this histogram, then
              // return.
              return;
          }
          
          //
          // Print accumulated stats if rows have no name
          //
          if(rowNames == NULL) {
              stateOut->AddVector("info", "total entries per column", NULL,
                                  & total[0], & total[numCols]);
              stateOut->AddVector("info", "accumulated values per column", NULL,
                                  & accumulated[0], & accumulated[numCols]);
          }
          
          // 
          // Print out columns labels. 
          //
          if (colNames != NULL) {
              stateOut->AddVector("info", "column names", NULL,
                                  & colNames[0], & colNames[numCols]);
          }
          else {
              //
              // Print the range of values for which we're collecting data.  
              //
              UINT32 colMax = colSize * numCols;
              columnRangeIterator first(0, colSize);
              columnRangeIterator last(colMax, colSize);
              
              stateOut->AddVector("info", "column names", NULL, first, last);
          }
          
          //
          // Print out histogram data. 
          //
          stateOut->AddCompound("info", "data");
          
          //
          // If we have non-unit size rows....
          if (rowSize > 1) {
              //
              // Print a range of values for which we're collecting data.  
              // We cannot have bin names if we're using bin size > 1. 
              //
              ASSERT (rowNames == NULL, this->Name());
              
              UINT32 min_size = 0;
              UINT32 max_size = rowSize - 1;
              for (i = 0; i < nActualRows; i++) {
                  os.str(""); // clear
                  os << min_size << "-" << max_size;
                  
                  stateOut->AddVector("row", os.str().c_str(), NULL,
                                      & histData[i][0], & histData[i][numCols]);
                  
                  min_size = max_size + 1;
                  max_size = min_size + rowSize - 1;
              }
          }
          else {
              //
              // Check for bin names.  If they exist, then print them out
              // instead of a bin number. 
              //
              if (rowNames != NULL) {
                  for (i = 0; i < nActualRows; i++) {
                      const char *name = rowNames[i];
                      if (name == NULL)
                      {
                          //
                          // No name for this row.  Use a number instead.
                          //
                          os.str("");
                          os << i;
                          name = os.str().c_str();
                      }

                      stateOut->AddVector("row", name, NULL,
                                          & histData[i][0], & histData[i][numCols]);
                  }
              }
              
              //
              // Print out information when we have bin numbers and not bin names. 
              //
              else {
                  for (i = 0; i < nActualRows; i++) {
                      os.str(""); // clear
                      os << i;
                      
                      stateOut->AddVector("row", os.str().c_str(), NULL,
                                          & histData[i][0], & histData[i][numCols]);
                  }
              }
          }
          stateOut->CloseCompound();
      }
  }
  
  //
  // Clear information about the histogram. 
  //
  virtual void ClearValues() {
      if (enabled == true) {          
          UINT32 i, j;
          UINT64 count = 0;
          UINT32 nActualRows = numRows;

          //
          // Simple histograms might drop rows at the end that hold no data.
          //
          if (rowSize == 1)
          {
              nActualRows = maxRowsUsed;
          }          
          
          for (i=0; i<numCols; i++) {
              count += total[i];
          }
          
          if (count == 0) {
              // If there are no entries in this histogram, then
              // return.
              return;
          }
          
          for (i = 0; i < nActualRows; i++) {
              for(j = 0; j < numCols; ++j)
                 histData[i][j] = 0;
          }
      }
  }

 //
 // HISTOGRAM ACCESS METHODS
 //
 protected:
  UINT32 MaxRowVal() const { return maxRowVal; }
} ;

//
// This is a class which allows the user to keep stats
// about the occupancy of a particular resource. A instance of this 
// class should be added to every resource for which you want to 
// occupancy stats.  For example, use this class if you want to 
// know how often the queue has X number of entries in it.  
template<bool E = true>
class RESOURCE_TEMPLATE : public HISTOGRAM_TEMPLATE<E> {
 private:

  // Nacking information
  UINT64 numNacks;         // Number of times resource nacked request because it was full
  UINT64 numRequestsNacked;// Number of requests which were nacked.  If there is only 
                           // one request per cycle, then numNacks = numRequestsNacked.
  UINT64 lastCycleNacked;  // The cycle at which the resource last nacked request. 

  // High Water Mark informatoin
  UINT64 hwmEnableTime;    // Time at which high water mark signal was enabled. 
  UINT64 hwmEnabledCycles; // Number of cycles for which high water mark was enabled. 

  // Information on occupancy of resource.  
  INT32 numEntries;        // Number of current entries in resource
  UINT64 lastModifiedCycle;// Last time number of entries was modified

 public:
  RESOURCE_TEMPLATE () : HISTOGRAM_TEMPLATE<E>() {
      if (this->enabled == true) {
          /*
            cout << "NULL RESOURCE CONSTRUCTOR." << endl;
            cout << flush;
          */
          numNacks = 0;
          hwmEnableTime = 0;
          hwmEnabledCycles = 0;
          numEntries = 0;
          lastModifiedCycle = 0;
      }
  }

  RESOURCE_TEMPLATE (UINT32 num_rows) : 
    HISTOGRAM_TEMPLATE<E>(num_rows, 1, 1, false, 1, false) {
      if (this->enabled == true) {
          // Initialize data members. 
          // cout << "RESOURCE_TEMPLATE: HIST: " << fmt_x(this)
          //      << ", HistData: " << fmt_x(this->histData)
          //      << ", HistData[0]: " << this->histData[0] << endl;
          numNacks = 0;
          hwmEnableTime = 0;
          hwmEnabledCycles = 0;
          numEntries = 0;
          lastModifiedCycle = 0;
      }
  }

  //
  // COPY constRUCTOR: This is needed when stats are suspended and
  // restored.  We need to retain the state information for the
  // resource when we restore, but we overwrite the stats with the
  // saved stats.  For example, the numEntries SHOULD NOT be restored
  // since this is state information for the resource, and not
  // statistics information.
  //
  const RESOURCE_TEMPLATE &operator=(const RESOURCE_TEMPLATE &save) {
      if (this->enabled == true) {
          numNacks = save.numNacks;
          numRequestsNacked = save.numRequestsNacked;
          hwmEnabledCycles = save.hwmEnabledCycles;
          HISTOGRAM_TEMPLATE<E>::operator=(save);
      }
      return *this;
  }

  //
  // NACK UPDATE METHODS
  //

  // 
  // These methods must be called whenever a request is nacked by the
  // resource.  Some resources use a high water mark, which implies
  // that numNacks will be zero for this resource.  Similarly, if we
  // nack requests, then the resource doesn't use hwm and therefore
  // the hwmEnabledCycles must be 0.  There are two ways to specify nacks.
  // You can specify them for each resource, or you can specify one
  // nack per cycle for a number of resources.

  //
  // This method specifies a nack for each resource. It can be called a 
  // number of times per cycle. 
  void RequestNacked(UINT64 cycle) {
      if (this->enabled == true) {
          ASSERT (hwmEnabledCycles == 0, this->Name());
          
          if (lastCycleNacked != cycle) {
              // We've not nacked anything this cycle. 
              lastCycleNacked = cycle;
              numNacks++;
          }
          numRequestsNacked++;
      }
  }

  //
  // Here we nack all requests at one time.  Therefore, it is assumed that
  // this method is only called once per cycle. 
  void RequestNacked(UINT64 cycle, UINT32 num_requests) {
      if (this->enabled == true) {
          ASSERT (hwmEnabledCycles == 0, this->Name());
          ASSERT (lastCycleNacked != cycle, this->Name());
          
          lastCycleNacked = cycle;
          numNacks++;
          numRequestsNacked += num_requests;
      }
  }

  // 
  // HIGH WATER MARK UPDATE METHODS
  //

  //
  // These methods must be called whenever the resource sends a high water 
  // mark signal indicating that other resources should wait on sending
  // requests, or sends a disable signal indicating that it is OK to send
  // requests.  The stat that is accumulated is how many cycles a resource
  // enables the high water mark signal.  Also note that if a resource
  // uses a hwm signal, it is assumed that it DOES NOT also nack requests.  
  //
  void EnableHighWaterMark(UINT64 cycle) {
      if (this->enabled == true) {
          ASSERT(hwmEnableTime == 0, this->Name());
          ASSERT(numNacks == 0, this->Name());
          hwmEnableTime = cycle;
      }
  }

  void DisableHighWaterMark(UINT64 cycle) {
      if (this->enabled == true) {
          ASSERT(hwmEnableTime != 0, this->Name());
          ASSERT(hwmEnableTime < cycle, this->Name());
          hwmEnabledCycles += (hwmEnableTime - cycle);
          hwmEnableTime = 0;
      }
  }

  //
  // OCCUPANCY MODIFICATION METHODS
  //
  
  // 
  // These methods register the entry and eviction of requests from the 
  // resource.  The stats are accumulated in a histogram which notes the
  // number of cycles a particular number of requests were in the resource.

 private:

  //
  // This method is called once for every request which is added or subracted
  // from the resource.  It may be called multiple times per cycle. 
  // 
  void ModifyResource(UINT64 cycle, bool add_req) {
    if (this->enabled == true) {
      ASSERT (lastModifiedCycle <= cycle, this->Name());
      if (lastModifiedCycle < cycle) {
	//
	// We're changing the status of the resource.  Therefore, update
	// the occupancy histogram with existing information.
	//
	this->AddEvent(numEntries, 0, (cycle - lastModifiedCycle));
	lastModifiedCycle = cycle;
      }
      // Update the number of entries, depending on whether we're adding or
      // deleting requests 
      if (add_req == true) {
	numEntries++;
	ASSERT (numEntries <= INT32(this->MaxRowVal()) && numEntries >= 0, this->Name());
      }
      else {
	numEntries--;
	ASSERT (numEntries >= 0 && numEntries <= INT32(this->MaxRowVal()), this->Name());
      }
    }
  }

  //    
  // This method is called once for cycle, and can add or subtract many requests 
  // per cycle. 
  //
  void ModifyResource(UINT64 cycle, UINT32 num_requests, bool add_req) {
    if (this->enabled == true) {
      ASSERT (lastModifiedCycle < cycle, this->Name());
      this->AddEvent(numEntries, 0, (cycle - lastModifiedCycle));
      if (add_req == true) {
	numEntries += num_requests;
	ASSERT (numEntries <= INT32(this->MaxRowVal()), this->Name());
      }
      else {
	numEntries -= num_requests;
	ASSERT (numEntries >= 0, this->Name());
      }
      lastModifiedCycle = cycle;
    }
  }
 public:
  //
  // These are the methods the user calls. 
  //
  void AddRequest(UINT64 cycle) {
    if (this->enabled == true) {
      ModifyResource(cycle, true);
    }
  }

  void AddRequest(UINT64 cycle, UINT32 num_req) {
    if (this->enabled == true) {
      ModifyResource(cycle, num_req, true);
    }
  }

  void DeleteRequest(UINT64 cycle) {
    if (this->enabled == true) {
      ModifyResource(cycle, false);
    }
  }

  void DeleteRequest(UINT64 cycle, UINT32 num_req) {
    if (this->enabled == true) {
      ModifyResource(cycle, num_req, false);
    }
  }

  void CurrentEntries(UINT64 cycle, UINT32 cur_entries) {
    if (this->enabled == true) {
      ASSERT (lastModifiedCycle <= cycle, this->Name());
      this->AddEvent(numEntries, 0, (cycle - lastModifiedCycle));
      numEntries = cur_entries;
      ASSERT (numEntries <= INT32(this->MaxRowVal()), this->Name());
      lastModifiedCycle = cycle;
    }
  }

  // Print information about this resource. 
  //
  void Dump(STATE_OUT stateOut) {

    if (this->enabled == true) {
      ostringstream os;

      if (numNacks != 0) {
	// The protocol for this structure is to nack when full. 
        stateOut->AddScalar("info", "nacks", NULL, numNacks);
        stateOut->AddScalar("info", "requests nacked", NULL, numRequestsNacked);
      }
      else if (hwmEnabledCycles != 0) {
        stateOut->AddScalar("info", "high water mark enabled cycles", NULL,
          hwmEnabledCycles);
      }
      
      // print out histogram of resource occupancy. 
      HISTOGRAM_TEMPLATE<E>::Dump(stateOut);
    }
  }
};

//
// This is a wrapper class around TPU_HISTOGRAM which basically 
// sets the tpuHist flag in histogram to be true.  THis flag will
// then print out information stating that each column in histogram 
// represents a thread.  
//
template<bool E = true>
class TPU_HISTOGRAM_TEMPLATE : public HISTOGRAM_TEMPLATE<E> {
 public: 

  //
  // Constructors. 
  //
  TPU_HISTOGRAM_TEMPLATE() : HISTOGRAM_TEMPLATE<E>() { this->tpuHist = true; }

  TPU_HISTOGRAM_TEMPLATE(UINT32 num_rows, UINT32 num_cols = 1, 
			 UINT32 row_size = 1, bool row_flex_cap = false,
			 UINT32 col_size = 1, bool col_flex_cap = false) :
    HISTOGRAM_TEMPLATE<E> (num_rows, num_cols, row_size, 
			   row_flex_cap, col_size, col_flex_cap) 
    { if (this->enabled == true) {this->tpuHist = true;} }
};

//
// This template represents 3-D information, where 2-Dimensional charts
// are printed out while varying the third dimension.  
//

template <bool E = true> 
class THREE_DIM_HISTOGRAM_TEMPLATE {
 private:
  HISTOGRAM_TEMPLATE<E> *histArray;
  UINT32 numHist;
  bool enabled;

 public:
  // Constructors / Destructors
  THREE_DIM_HISTOGRAM_TEMPLATE() : 
    histArray(NULL), numHist(0), enabled(E) {}

  THREE_DIM_HISTOGRAM_TEMPLATE(UINT32 num_charts, UINT32 num_rows,
			       UINT32 num_cols = 2, UINT32 row_size = 1,
			       bool row_flex_cap = false, 
			       UINT32 col_size = 1, 
			       bool col_flex_cap = false) : enabled (E) {
      if (this->enabled == true) {                               
          UINT32 i;
          ASSERT (num_charts > 1, this->Name());
          numHist = num_charts;
          
          //
          // If you only had one row or one columns, then you could probably
          // get away with a 2-dim histogram. 
          //
          ASSERT (num_rows > 1, this->Name());
          ASSERT (num_cols > 1, this->Name());
          ASSERT (row_size >= 1, this->Name());
          ASSERT (col_size >= 1, this->Name());
          //
          // Allocate multiple histograms to create THREE_DIM array. 
          // Since c++ is a CRAPPY language and cannot declare an
          // array of objects with a non-null constructor, I first
          // declare a histogram which is non-null.  I then allocate
          // the space for my array of histograms.  I then copy the
          // non-null histogram into each entry of the array of 
          // histograms.
          //
          HISTOGRAM_TEMPLATE<E> dummy(num_rows, num_cols, row_size, 
                                      row_flex_cap, col_size, col_flex_cap);
          histArray = new HISTOGRAM_TEMPLATE<E> [numHist];
          // 
          // Use the overloaded "=" operator to create the correct
          // sized histograms.  
          for (i = 0; i < numHist; i++) {
              histArray[i] = dummy;
          }
      }
  }

  ~THREE_DIM_HISTOGRAM_TEMPLATE() {
      if (this->enabled) {
          if (histArray) {
              delete [] histArray;
          }
      }
  }


  //
  // Set name of the histogram
  //
  void SetName(char *n) {
    histArray[0].SetName(n);
  }
    
  //
  // Get name of the histogram
  //
  char* Name(void) {
    return histArray[0].Name();
  }
    
  //
  // Prepare this 'save' object to be deleted correctly;
  // (just remember that the distructor has to be careful)
  //
  void PrepareSaveDelete(void) {
      if (this->enabled) {
          for (UINT32 i = 0; i < numHist; i++) {
              histArray[i].PrepareSaveDelete();
          }
      }
  }

  //
  // Overload = operator so that we can save and restore stats values
  // when we sample code. 
  //
  const THREE_DIM_HISTOGRAM_TEMPLATE &operator=(const THREE_DIM_HISTOGRAM_TEMPLATE &save) {
    UINT32 i;
    if (this->enabled == true) {

        if (numHist == 0) {
            // 
            // We have to allocate space for histograms. 
            //
            ASSERT (histArray == NULL, this->Name());
            histArray = new HISTOGRAM_TEMPLATE<E>[numHist];
        }
        else {
            ASSERT (histArray != NULL, this->Name());
        }
        
        //
        // We now copy over the data for each histogram. 
        //
        for (i = 0; i < numHist; i++) {
            histArray[i] = save.histArray[i];
        }
    }
    return *this;
  }

  void AddEventWideBins(UINT32 array_loc, UINT32 row_number,
		UINT32 col = 0, UINT64 value = 1) {
    if (this->enabled == true) {
      ASSERT (array_loc < numHist, this->Name());
      histArray[array_loc].AddEventWideBins(row_number, col, value);
    }
  }

  void AddEvent(UINT32 array_loc, UINT32 row_number,
		UINT32 col = 0, UINT64 value = 1) {
    if (this->enabled == true) {
      ASSERT (array_loc < numHist, this->Name());
      histArray[array_loc].AddEvent(row_number, col, value);
    }
  }

  void Dump(STATE_OUT stateOut) {
    if (this->enabled == true) {
      UINT32 i;

      for (i = 0; i < numHist; i++) {
        stateOut->AddCompound("info", "array");
        histArray[i].Dump(stateOut);
        stateOut->CloseCompound();
      }
    }
  }
  
  void ClearValues() {
    if (this->enabled == true) {
      UINT32 i;

      for (i = 0; i < numHist; i++) {
        histArray[i].ClearValues();
      }
    }
  }
  
};

//
// Orphan function, not method.  Does not belong to any class. 
//
static UINT32
CountEntries(const char *s) {
  char copy[MAX_STRING_LENGTH];
  char *str;
  UINT32 count = 0;

  strcpy (copy, s);
  str = strtok(copy, SEPARATORS);
  while (str != NULL) {
    count++;
    str = strtok(NULL, SEPARATORS);
  }
  ASSERTX (count > 0);
  return count;
}
#endif // _HISTOGRAM_STATS_



