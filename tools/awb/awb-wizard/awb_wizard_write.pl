##################################################################################
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
##################################################################################

# Author: Joel Emer, Julio Gago
# Date: November 2002

#
#
#  Global fields:
#       awbfilename
#       hfilename
#       cppfilename
#
#       name - name of module
#       description = description of module
#       attributes - list of attributes
#       provides - asim-type provided by this module
#       requires - list of requires
#
#       ismodule  - class implements a normal module
#       isalgorithm  - class implements an algorithm module
#       ismessage  - class implements a message module
#
#       classname - classname implemented by module
#       author - module author
#       brief - (copy of descripton?)
#       isclocked - module is clocked or not
#
#       parameters - list of parameters
#
#       ports - list of ports
#
#
#  Requires fields
#       name
#       classname - classname of module (excluding final _CLASS)
#       type - type of module (module, algorithm, message)
#       ismodule - true/false
#       isalgorithm - true/false
#       ismessage - true/false
#       membername - variable name for class variable
#       modulename - textual name to use for module
#       isrequired - true/false
#       isclocked - true/false
#
#
#  Parameter fields:
#       name
#       description
#       mode (static/dynamic)
#       type (integer/string)
#       value
#
#
#  Port fields
#       name
#       readport - boolean
#       writeport - boolean
#       variable
#       class
#       latency
#       bandwidth
#
#
#  Statistics fields
#       name
#       scalar - boolean
#       histogram - boolean
#       size
#       variable
#       description
#

our $debug = 0;


sub write_module {
  my $info = shift;

  if ($info->{modulepath} !~ /src\/asim-/) {
    my $status = Qt::MessageBox::warning ( 
                this, 
                "awb-wizard save",
                "You seem to saving outside of the source (package) area. Continue anyway?",
                "&Yes",
                "&No",
                "Cancel",
                0,
                2);

    return if ($status == 1);
    return if ($status == 2);
  }

  foreach my $ext ("awb", "h", "cpp") {
    write_template($info, $ext);
  }

}

sub write_template {
  my $info = shift;
  my $ext = shift;
  my $type;

  my $filename = $info->{modulepath} . "/" .  $info->{"${ext}filename"};
  my @template;

  print "Writing to $filename\n";

  if ($info->{ismodule}) {
    $type = "module";
  } elsif ($info->{isalgorithm}) {
    $type = "algorithm";
  } elsif ($info->{ismessage}) {
    $type = "message";
  } else {
    die ("Unknown module type\n");
  }

    # Process templates
  @template = LoadTemplate($ext, $type);

  open(FILE, ">$filename")
    || die("Unable to open output file: $!\n");

  ProcessTemplate(\*FILE, $info, @template);

  close(FILE);
}


sub ProcessTemplate
{
  my $fd = shift;
  my $info = shift;
  my @file = @_;

  my $line;
  my $q;
  my $question;
  my $lc_question;
  my $replacement;

  my $loopstart;
  my $loopname;
  my $looppredicate;
  my @looplist;


  my $i = 0;

LINE:
  while ($i <= $#file) {
    print "Processing line $i of $#file\n" if ($debug);
    $line = $file[$i];

    #
    # Process loop start
    #
    if ($line =~ /<<<FOREACH ([^|]+)(\|(.+))?>>>/) {
      print "Processing - $line\n" if ($debug);

      #
      # Record information about loop
      #
      $loopstart = $i+1;
      $loopname = lc($1);
      if (defined($2)) {
        $looppredicate = lc($2);
        $looppredicate =~ s/\|//;
      } else {
        $looppredicate = undef;
      }

      #
      # Build list of elements for loop iterations
      #
      if (defined($info->{$loopname})) {
        @looplist = @{$info->{$loopname}};
      } else {
        @looplist = ();
      }

      #
      # Force execution of a <<<NEXT>>> to initiate loop
      #
      while (!($file[$i] =~ "<<<NEXT>>>")) {
        $i++;
      }
      next;
    }

    #
    # Process loop step
    #
    if ($line =~ /<<<NEXT>>>/) {
      print "Processing - <<<next>>>\n" if ($debug);

      my $loopinfo;

      #
      # Find the first element of @looplist that satisfies loop predicate
      #
      while ($loopinfo = pop(@looplist)) {
        if (!defined($looppredicate) || $loopinfo->{$looppredicate}) {
          last;
        }
      }

      #
      # Exit loop if @listlist is exhausted
      #
      if (!defined($loopinfo)) {
        $i++;
        next;
      }

      #
      # Assign values to loop variable
      #
      foreach my $k (keys %{$loopinfo}) {
        $info->{"$loopname.$k"} = $loopinfo->{$k};
#        print "Assigning $loopname.$k = " . $info->{"$loopname.$k"} . "\n";
      }

      #
      # Go back to start of loop
      #
      $i = $loopstart;
      next;
    }

    if ($line =~ /<<<IF (.*)>>>/) {
      print "Processing - $line\n" if ($debug);

      my $predicate = $info->{lc($1)};

      if ($predicate) {
        $i++;
      } else {
        while (! ($file[$i] =~ "<<<ENDIF>>>")) {
          $i++;
        }
      }
      next;
    }

    if ($line =~ /<<<ENDIF>>>/) {
      print "Processing - $line\n" if ($debug);
      $i++;
      next;
    }

    #
    # Do substitutions in line
    #
    while ($line =~ /<<<([^<>]*)>>>/) {
      $question = $1;
      $lc_question = lc($question);
      $replacement = $info->{$lc_question};
      if (!defined($replacement)) {
        # Skip lines with undefined substitions!
        $replacement = "***UNDEF***";
        $i++;
        next LINE;
      }

      $line =~ s/<<<${question}>>>/$replacement/;
    }

    #
    # Output processed line to file
    #
    print $fd "$line\n" || die("Write failed\n");
    $i++;
  }
}

#
# Load inline templates
#

sub LoadTemplate {
  my $ext = shift;
  my $type = shift;

  if ($type eq "module") {
    return LoadAwbTemplate() if ($ext eq "awb");
    return LoadHTemplate()   if ($ext eq "h");
    return LoadCppTemplate() if ($ext eq "cpp");
  }

  if ($type eq "algorithm") {
    return LoadAwbTemplate() if ($ext eq "awb");
    return LoadHTemplate()   if ($ext eq "h");
    return LoadCppTemplate() if ($ext eq "cpp");
  }

  if ($type eq "message") {
    return LoadAwbTemplate()         if ($ext eq "awb");
    return LoadHMessageTemplate()   if ($ext eq "h");
    return LoadCppMessageTemplate()  if ($ext eq "cpp");
  }

}


sub LoadAwbTemplate
{
  my $file1;

  $file1 = <<'__EOF__MARKER__';
/********************************************************************
 *
 * Awb module specification
 *
 *******************************************************************/

/*
 * %AWB_START
 *
 * %name <<<NAME>>>
 * %desc <<<DESCRIPTION>>>
 * %attributes <<<ATTRIBUTES>>>
 *
 * %provides <<<PROVIDES>>>
   <<<FOREACH REQUIRES|ISREQUIRED>>>
 * %requires <<<REQUIRES.NAME>>>
   <<<NEXT>>>
 *
 * %public <<<PUBLIC>>>
 * %private <<<PRIVATE>>>
 *
   <<<FOREACH PARAMETER>>>
 * <<<PARAMETER.EXPORT>>> <<<PARAMETER.DYNAMIC>>> <<<PARAMETER.NAME>>> <<<PARAMETER.VALUE>>> "<<<PARAMETER.DESCRIPTION>>>"
   <<<NEXT>>>
 *
 * %AWB_END
 */
__EOF__MARKER__

  return (split(/\n/, $file1));
}

sub LoadHTemplate {
  my $file2;

  $file2 = <<'__EOF__MARKER__';
/*****************************************************************************
 *
 * @brief Header file for <<<DESC>>>
 *
 * @author <<<AUTHOR>>>
 *
 * Copyright (c) 2001 Intel Corporation, all rights reserved.
 * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
 * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
 *
 *****************************************************************************/

#ifndef _<<<CLASSNAME>>>_
#define _<<<CLASSNAME>>>_

// ASIM core
#include "asim/syntax.h"
#include "asim/module.h"
#include "asim/port.h"

// ASIM messages

<<<FOREACH REQUIRES|ISMESSAGE>>>
#include "asim/provides/<<<REQUIRES.NAME>>>.h"

<<<NEXT>>>

// ASIM sub-modules
//    either use full include or just typedef

<<<FOREACH REQUIRES|ISMODULE>>>
//#include "asim/provides/<<<REQUIRES.NAME>>>.h"
typedef class <<<REQUIRES.CLASSNAME>>>_CLASS * <<<REQUIRES.CLASSNAME>>>;

<<<NEXT>>>

// ASIM sub-algorithms
//    either use full include or just typedef

<<<FOREACH REQUIRES|ISALGORITHM>>>
//#include "asim/provides/<<<REQUIRES.NAME>>>.h"
typedef class <<<REQUIRES.CLASSNAME>>>_CLASS * <<<REQUIRES.CLASSNAME>>>;

<<<NEXT>>>


/**
 * Class <<<CLASSNAME>>>_CLASS
 *
 * <<<DESC>>>
 *
 */

typedef class <<<CLASSNAME>>>_CLASS * <<<CLASSNAME>>>;

class <<<CLASSNAME>>>_CLASS : public ASIM_MODULE_CLASS
{
  private:

    // Submodules
    <<<FOREACH REQUIRES|ISMODULE>>>
    <<<REQUIRES.CLASSNAME>>>  <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Algorithms
    <<<FOREACH REQUIRES|ISALGORITHM>>>
    <<<REQUIRES.CLASSNAME>>>  <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Ports
    <<<FOREACH PORT|READPORT>>>
    ReadPort<<<<PORT.CLASS>>>> <<<PORT.VARIABLE>>>_RPort;
    <<<NEXT>>>

    <<<FOREACH PORT|WRITEPORT>>>
    WritePort<<<<PORT.CLASS>>>> <<<PORT.VARIABLE>>>_WPort;
    <<<NEXT>>>

    // Statistics

    <<<FOREACH STATISTIC|SCALAR>>>
    UINT64  <<<STATISTIC.VARIABLE>>>;
    <<<NEXT>>>

    <<<FOREACH STATISTIC|HISTOGRAM>>>
    UINT64  <<<STATISTIC.VARIABLE>>>[<<<STATISTIC.SIZE>>>];
    <<<NEXT>>>

    // Other <<<CLASSNAME>>> private variables


    // <<<CLASSNAME>>> private methods


  public:
    // <<<CLASSNAME>>> public variables


    // Constructor
    <<<CLASSNAME>>>_CLASS(ASIM_MODULE parent, const char * const name);

    // Destructor
    ~<<<CLASSNAME>>>_CLASS();

    // Required by ASIM
    bool InitModule();

<<<IF ISCLOCKED>>>
    // Do a cycle of work...
    void Clock (UINT64 cycle);
<<<ENDIF>>>

    // Additional <<<CLASSNAME>>> public methods


};

#endif /* _<<<CLASSNAME>>>_ */
__EOF__MARKER__

  return (split(/\n/, $file2));
}

sub LoadCppTemplate {
  my $file3;

  $file3 = <<'__EOF__MARKER__';
/*****************************************************************************
 *
 * @brief <<<NAME>>> 
 *
 * @author <<<AUTHOR>>>
 *
 * Copyright (c) 2001 Intel Corporation, all rights reserved.
 * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
 * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
 *
 *****************************************************************************/

// Generic C++
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>

// ASIM core
#include "asim/syntax.h"

// ASIM public module
#include "asim/provides/<<<PROVIDES>>>.h"

// ASIM sub-modules

<<<FOREACH REQUIRES|ISMODULE>>>
#include "asim/provides/<<<REQUIRES.NAME>>>.h"
<<<NEXT>>>

// ASIM sub-algorithms
<<<FOREACH REQUIRES|ISALGORITHM>>>
#include "asim/provides/<<<REQUIRES.NAME>>>.h"
<<<NEXT>>>

/**
 * Instantiate module and submodules.
 */
<<<CLASSNAME>>>_CLASS::<<<CLASSNAME>>>_CLASS (
    ASIM_MODULE parent, ///< parent module
    const char * const name)   ///< name of this module
    : ASIM_MODULE_CLASS(parent, name)
{

      // New each submodule

<<<FOREACH REQUIRES|ISMODULE>>>
      <<<REQUIRES.MEMBERNAME>>> = new <<<REQUIRES.CLASSNAME>>>_CLASS(this, "<<<REQUIRES.MODULENAME>>>");
<<<NEXT>>>

      // New each algorithm

<<<FOREACH REQUIRES|ISALGORITHM>>>
      <<<REQUIRES.MEMBERNAME>>> = new <<<REQUIRES.CLASSNAME>>>_CLASS(this, "<<<REQUIRES.MODULENAME>>>");
<<<NEXT>>>

      // Initialize the read ports

      <<<FOREACH PORT|READPORT>>>
      <<<PORT.VARIABLE>>>_RPort.Init("<<<PORT.NAME>>>", this->GetUniqueId());
      <<<PORT.VARIABLE>>>_RPort.SetLatency(<<<PORT.LATENCY>>>);
      <<<PORT.VARIABLE>>>_RPort.SetBandwidth(<<<PORT.BANDWIDTH>>>);

      <<<NEXT>>>

      // Initialize the write ports

      <<<FOREACH PORT|WRITEPORT>>>
      <<<PORT.VARIABLE>>>_WPort.Init("<<<PORT.NAME>>>", this->GetUniqueId());
      <<<PORT.VARIABLE>>>_WPort.SetLatency(<<<PORT.LATENCY>>>);
      <<<PORT.VARIABLE>>>_WPort.SetBandwidth(<<<PORT.BANDWIDTH>>>);

      <<<NEXT>>>

      // Initialize the statistics

      <<<FOREACH STATISTIC|SCALAR>>>
      RegisterState(&<<<STATISTIC.VARIABLE>>>, "<<<STATISTIC.NAME>>>",
                    "<<<STATISTIC.DESCRIPTION>>>");
      <<<NEXT>>>

      <<<FOREACH STATISTIC|HISTOGRAM>>>
      RegisterState(<<<STATISTIC.VARIABLE>>>, <<<STATISTIC.SIZE>>>, "<<<STATISTIC.NAME>>>",
                    "<<<STATISTIC.DESCRIPTION>>>");
      <<<NEXT>>>

      // Initialize other <<<CLASSNAME>>> variables

}

/**
 * Destroy module and submodules
 */
<<<CLASSNAME>>>_CLASS::~<<<CLASSNAME>>>_CLASS ()
{
    // Delete submodules

    <<<FOREACH REQUIRES|ISMODULE>>>
    delete <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Delete sub-algorithms

    <<<FOREACH REQUIRES|ISALGORITHM>>>
    delete <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Delete other <<<CLASSNAME>>> variables

}

/**
 * Initialize module and submodules
 */
bool
<<<CLASSNAME>>>_CLASS::InitModule ()
{
    bool ok = true;

    TRACE(Trace_Debug, cout << "0: Init Module " << Name() << endl);

    // Submodule initilizations go here

    <<<FOREACH REQUIRES|ISMODULE>>>
    ok &= <<<REQUIRES.MEMBERNAME>>>->InitModule();
    <<<NEXT>>>

    // Algorithms are not inited....


    // Additional initialization code...

    if ( !ok ) 
    {
        cout << Name() << ": InitModule failed" << endl;
    }

    return ok;
}

<<<IF ISCLOCKED>>>
/**
 * Perform one cycle of work.
 */
void
<<<CLASSNAME>>>_CLASS::Clock (
    UINT64 cycle)        ///< current cycle
{
    // Read port input variables

    <<<FOREACH PORT|READPORT>>>
    <<<PORT.CLASS>>> <<<PORT.VARIABLE>>>_in;
    <<<NEXT>>>

    // Write port output variables

    <<<FOREACH PORT|WRITEPORT>>>
    <<<PORT.CLASS>>> <<<PORT.VARIABLE>>>_out;
    <<<NEXT>>>

    TRACE(Trace_Sys, cout << cycle << ": Clocking - " << Name() << endl);

    // Read the input ports

    <<<FOREACH PORT|READPORT>>>
//  while (<<<PORT.VARIABLE>>>_RPort.Read(<<<PORT.VARIABLE>>>_in, cycle))
//  {
//    ... do something with the data
//  }

    <<<NEXT>>>


    // Perform the module's activity


    // Write the output ports

    <<<FOREACH PORT|WRITEPORT>>>
//  <<<PORT.VARIABLE>>>_out = new <<<PORT.CLASS>>>_CLASS();
//  <<<PORT.VARIABLE>>>_WPort.Write(<<<PORT.VARIABLE>>>_out, cycle);

    <<<NEXT>>>

    // Update statistics

    <<<FOREACH STATISTIC|SCALAR>>>
    <<<STATISTIC.VARIABLE>>>++;
    <<<NEXT>>>

    // Clock submodules

   <<<FOREACH REQUIRES|ISCLOCKED>>>
   <<<REQUIRES.MEMBERNAME>>>->Clock(cycle);
   <<<NEXT>>>


}
<<<ENDIF>>>

__EOF__MARKER__

  return (split(/\n/, $file3));
}

sub LoadHMessageTemplate {
  my $file2;

  $file2 = <<'__EOF__MARKER__';
/*****************************************************************************
 *
 * @brief Header file for <<<DESC>>>
 *
 * @author <<<AUTHOR>>>
 *
 * Copyright (c) 2001 Intel Corporation, all rights reserved.
 * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
 * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
 *
 *****************************************************************************/

#ifndef _<<<CLASSNAME>>>_
#define _<<<CLASSNAME>>>_

#include <string.h>

// ASIM core
#include "asim/syntax.h"
#include "asim/mm.h"
#include "asim/item.h"
#include "asim/trace.h"


// ASIM public modules (include sub-modules)
//    either use full include or just typedef

<<<FOREACH REQUIRES|ISMODULE>>>
//#include "asim/provides/<<<REQUIRES.NAME>>>.h"
typedef class <<<REQUIRES.CLASSNAME>>>_CLASS * <<<REQUIRES.CLASSNAME>>>;

<<<NEXT>>>

<<<FOREACH REQUIRES|ISALGORITHM>>>
//#include "asim/provides/<<<REQUIRES.NAME>>>.h"
typedef class <<<REQUIRES.CLASSNAME>>>_CLASS * <<<REQUIRES.CLASSNAME>>>;

<<<NEXT>>>

/**
 * Class <<<CLASSNAME>>>_CLASS
 *
 * <<<DESC>>>
 *
 */

typedef class mmptr<class <<<CLASSNAME>>>_CLASS> <<<CLASSNAME>>>;

class <<<CLASSNAME>>>_CLASS
   : public ASIM_MM_CLASS<<<<CLASSNAME>>>_CLASS>,
     public ASIM_ITEM_CLASS
{
  private:
    static UINT64 theUidCtr;              ///< Static id to assign unique Ids
    const UINT64 myUid;                   ///< unique id for message
    const UINT64 myDisplayId;             ///< display id for the cycle display

    static bool theInitialized;            ///< Have the static members of this class been initialized?

    // Sub-modules
    <<<FOREACH REQUIRES|ISMODULE>>>
    <<<REQUIRES.CLASSNAME>>>  <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Sub-algorithms
    <<<FOREACH REQUIRES|ISALGORITHM>>>
    <<<REQUIRES.CLASSNAME>>>  <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

  public:

    // Constructor
    <<<CLASSNAME>>>_CLASS( const UINT64 display_id=0);

    // Destructor
    ~<<<CLASSNAME>>>_CLASS();

    // Initialize method
    // This method is called from outside the class to initialize static members
    static void Initialize();

    // Message dump method
    void DumpTrace(const char* const prefix);

    // Accessors
    UINT64 GetUid() const;
    UINT64 GetDisplayId() const;
    // ...more message-type specific accessors...

    // Modifiers
    // ...more message-type specific modifiers...

};

inline UINT64
<<<CLASSNAME>>>_CLASS::GetUid() const
{
    return myUid;
}

inline UINT64
<<<CLASSNAME>>>_CLASS::GetDisplayId() const
{
    return myDisplayId;
}

#endif /* _<<<CLASSNAME>>>_ */
__EOF__MARKER__

  return (split(/\n/, $file2));
}

sub LoadCppMessageTemplate {
  my $file3;

  $file3 = <<'__EOF__MARKER__';
/*****************************************************************************
 *
 * @brief <<<NAME>>>
 *
 * @author <<<AUTHOR>>>
 *
 * Copyright (c) 2001 Intel Corporation, all rights reserved.
 * THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
 * IS CONSIDERED A TRADE SECRET BELONGING TO THE INTEL CORPORATION.
 *
 *****************************************************************************/

// Generic C++
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>

// ASIM core
#include "asim/syntax.h"

// ASIM public module
#include "asim/provides/<<<PROVIDES>>>.h"

// ASIM sub-modules

<<<FOREACH REQUIRES|ISMODULE>>>
#include "asim/provides/<<<REQUIRES.NAME>>>.h"
<<<NEXT>>>

// ASIM sub-algorithms

<<<FOREACH REQUIRES|ISALGORITHM>>>
#include "asim/provides/<<<REQUIRES.NAME>>>.h"
<<<NEXT>>>

const UINT32 MAX_<<<CLASSNAME>>>S = 1024;
ASIM_MM_DEFINE(<<<CLASSNAME>>>_CLASS, MAX_<<<CLASSNAME>>>S);
UINT64 <<<CLASSNAME>>>_CLASS::theUidCtr = 0;

bool <<<CLASSNAME>>>_CLASS::theInitialized = false;

/**
 * Instantiate module and submodules.
 */
<<<CLASSNAME>>>_CLASS::<<<CLASSNAME>>>_CLASS (
   // Parameters
   const UINT64 display_id                              ///< module's display id
   )
   : ASIM_MM_CLASS<<<<CLASSNAME>>>_CLASS>(theUidCtr,0),
     myUid(theUidCtr++),
     myDisplayId(display_id!=0?display_id:GetUniqueDisplayId())
{

    // set tags for the cycle display
    EVENT(
        if (myDisplayId) { SetItemTag("DID", myDisplayId); }
        SetItemTag("UID", myUid);
        );

    // Other variable initializations

}

/**
 * Destroy module and submodules
 */
<<<CLASSNAME>>>_CLASS::~<<<CLASSNAME>>>_CLASS ()
{
    // Delete submodules

    <<<FOREACH REQUIRES|ISMODULE>>>
    delete <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Delete sub-algorithms

    <<<FOREACH REQUIRES|ISALGORITHM>>>
    delete <<<REQUIRES.MEMBERNAME>>>;
    <<<NEXT>>>

    // Other variable deletions

}

/**
 * This method needs to be called from outside the <<<CLASSNAME>>> class at model initialization time.
 */
void
<<<CLASSNAME>>>_CLASS::Initialize()
{
    if (theInitialized == true)
    {
        return;
    }
    else
    {
        theInitialized = true;
    }



}


void
<<<CLASSNAME>>>_CLASS::DumpTrace(const char* const prefix)
{
    const char* p = strrchr(prefix, '\n');
    p = p ? p : prefix;
    UINT32 p_len = strlen(p);
    char* str = new char[p_len+1];
    for (UINT32 i=0; i < p_len; i++)
    {
        str[i] = (p[i] == '\t') ? '\t' : ' ';
    }
    str[p_len] = '\0';

    cout << prefix
         << " <<<CLASSNAME>>> Msg:"
         << " uid=" << myUid
         << endl;

    cout << str
         << "         : origMCG=";
    cout << endl;

    delete str;
}


__EOF__MARKER__

  return (split(/\n/, $file3));
}

