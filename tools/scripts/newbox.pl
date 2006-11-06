:
eval 'exec perl "$0" ${1+"$@"}'
       if 0;

#
# Copyright (c) 1999 Compaq Computer Corporation
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
 # Author:  Roger Espasa
 #

 $| = 1;

 $DOH = 1;
 $DOCPP = 1;
 while ( $ARGV[0] =~ /^-/ ) {
  if ( $ARGV[0] =~ /-noh/ )   { $DOH = 0; }
  elsif ( $ARGV[0] =~ /-nocpp/ ) { $DOCPP = 0; }
  else { die "usage: newbox.pl [-noh|-nocpp]"; }
  shift;
 }

 print <<"EofIntro";
+--------------------------------------------------------------------+
|                                                                    |
|                                                                    |
| Welcome to the automatic BOX creation script                       |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------+
EofIntro


 #######################################################################
 #######################################################################
 ##
 ##
 ## General Variables: You might need to change this when porting to
 ##         a different environment
 ##
 ##
 #######################################################################
 #######################################################################

 $HOSTNAME = "/usr/bin/hostname";
 $TAR = "tar";
 $GZIP = "gzip -9";
 $DATE = "/usr/bin/date";
 $RESOLVER = "awb-resolver";
 chomp ($msg = `$RESOLVER .`);
 if ($? != 0) {
   die "Can't find $RESOLVER - check your PATH environemt variable\n$msg\n";
 }

 #######################################################################
 #######################################################################
 ##
 ##
 ## Directory Checking... We need $ASIMDIR to find the box templates
 ##
 ##
 #######################################################################
 #######################################################################

 #
 # There are multiple places where you can find a pointer to the top
 # level asim directory:
 #  1- The most obvious is `pwd`, i.e., the directory where we are running
 #  2- Also, the user can have the ASIMDIR variable set in the environment
 #  3- Finally, the user can have a awb.config file located either in
 #      3a- his home directory under .awb/ , or
 #      3b- the directory pointed to by $ENV{'AWBLOCAL'}
 #
 # Here we collect all these different sources of information and try
 # to see if they are coherent or not. We keep an array of different 
 # directories and at the end, if we end up with more than one directory
 # we ask the user which one to use
 #
 @ASIMDIRS = ();
 @FROM = ();

 # (1) Current directory
 $dir = `pwd`;
 $dir =~ s/asim\/.*/asim/;
 chop $dir;
 push(@ASIMDIRS, $dir);		
 push(@FROM, "<current dir>");		

 # (2) Environment ASIMDIR
# r2r: this is not supported in AWB anywhere - I checked;
# commenting out
# $dir = $ENV{'ASIMDIR'};
# if ( -d $dir && !grep(/$dir/,@ASIMDIRS)) {
#  push (@ASIMDIRS, $dir);
#  push(@FROM,"\$ASIMDIR"); 
# }

 # (3) AWBCONFIG
 chomp ($dir = `$RESOLVER -config asimdir`);
 if ($? != 0) {
  die "ERROR:\n$dir\n";
 }
 if ( -d $dir && !grep(/$dir/,@ASIMDIRS)) {
  push(@ASIMDIRS, $dir);
  push(@FROM,"AWB config file"); 
 }

 # 
 # If we collected multipled dirs during steps (1), (2) and (3), warn the user and ask
 # him which one he wants to use. If we only have one option, just use $index to index
 # the ASIMDIRS array.
 #
 $index = 0;
 if ( $#ASIMDIRS > 0 ) {
  print "\n\n";
  print "Hey, you've got a nice little mess of directories here :-)\n";
  print "Which one of the following do you want to commit from: ?\n";
  for ($i = 0; $i <= $#ASIMDIRS; $i++ ) {
   $idx = $i+1;
   print "\t($idx) $ASIMDIRS[$i] (from $FROM[$i])\n";
  }

  #
  # Ask User
  #
  $index = getinput("Pick a number","1");
  $index--; 
  if ( $index < 0 || $index > $#ASIMDIRS ) {
   die "Wrong choice.\n";
  }
 }
 $ASIMDIR = $ASIMDIRS[$index];


 #######################################################################
 #######################################################################
 ##
 ##
 ## PHASE 0:  Get Box name. It is a two level name, consisting of 
 ##           <MAIN_BOX> followed by <SUB_MODULE> (for example, pbox_mc).
 ##
 ##
 #######################################################################
 #######################################################################

 print <<"EofPhase0";
+--------------------------------------------------------------------+
| BOX NAME                                                           | 
|                                                                    |
|  Please enter the BOX name in two pieces <MAJOR> and <MINOR>. The  |
|  <major> corresponds to the major box where this new box will live;|
|  for example, 'pbox'. The <minor> name is the actual name of the   |
|  new box you are creating; for example 'mc' (map chooser).         |
|                                                                    |
|  Example: PBOX_MC : <MAJOR> = PBOX, <MINOR> = MC                   |
|           IBOX_CB : <MAJOR> = IBOX, <MINOR> = CB                   |
|                                                                    |
|  Note: the script will ignore Upper/Lower casing                   |
|                                                                    |
+--------------------------------------------------------------------+
EofPhase0

 $MAJOR = getinput("<MAJOR> ?");
 $MINOR = getinput("<MINOR> ?");
 

 print <<"EofPhase0b";
+--------------------------------------------------------------------+
| PROVIDES                                                           | 
|                                                                    |
|  Please enter the string describing what this box "%provides".     |
|                                                                    |
|  Note: the script will turn your text into Lower case              |
|                                                                    |
+--------------------------------------------------------------------+
EofPhase0b

 $PROVIDES = getinput("<%provides> ?");
 

 #######################################################################
 #######################################################################
 ##
 ##
 ## PHASE 1:  Get Buffer information. For each buffer we will ask for
 ##           the name, the type, whether it is IN/OUT (just for the
 ##           sake of adding a nice comment) and a description.
 ##
 #######################################################################
 #######################################################################

 print <<"EofPhase1";
+--------------------------------------------------------------------+
| BUFFERS                                                            | 
|                                                                    |
|  Please enter the Buffer information. For each buffer the script   |
|  will ask for:                                                     |
|                                                                    |
|  - <NAME> : do not include here either <MAJOR> or <MINOR>, because |
|             the script will do it for you. Just use a short name   |
|             or an acronym that you like                            |
|                                                                    |
|  - <IN|OUT> : whether the buffer is input or output                |
|                                                                    |
|  - <COMMENT> : brief description of what the buffer does           |
|                                                                    |
|  - <DATATYPE> : (if you know it) you can indicate the data type.   |
|             It can be either a basic data type (UINT32, BOOL)      |
|             or a more advanced one (CPU_INSTBUF, PBOX_PSTATBUF).   |
|                                                                    |
|  Note: the script will MAINTAIN your Upper/Lower casing            |
|                                                                    |
|  To end the list of buffers type '.' as the buffer name            |
+--------------------------------------------------------------------+
EofPhase1

 for ( $i = 0; 1 ; $i++ ) {
  print "Buffer:\n";
  $blist[$i]{'NAME'}     = getinput(" <NAME> ?");
  last if ( $blist[$i]{'NAME'} eq "." );
  $blist[$i]{'INOUT'}    = getinput(" <IN|OUT> ?","IN");
  $blist[$i]{'COMMENT'}  = getinput(" <COMMENT> ?");
  $blist[$i]{'DATATYPE'} = getinput(" <DATA TYPE> ?");
  print "\n";
 }
 $nbufs = $i;


 #######################################################################
 #######################################################################
 ##
 ##
 ## PHASE 2:  Get EVENT information. For each EVENT we will ask for
 ##           the name and a description.
 ##
 #######################################################################
 #######################################################################

 print <<"EofPhase2";
+--------------------------------------------------------------------+
| EVENTS                                                             | 
|                                                                    |
|  Please enter the Event information. For each event  the script    |
|  will ask for:                                                     |
|                                                                    |
|  - <NAME> : do not include here the word 'Event', because          |
|             the script will do it for you. Just use a short name   |
|             or an acronym that you like                            |
|                                                                    |
|  - <COMMENT> : brief description of what the event conveys         |
|                                                                    |
|  Note: the script will MAINTAIN Upper/Lower casing                 |
|                                                                    |
|  To end the list of events type '.' as the event name              |
+--------------------------------------------------------------------+
EofPhase2

 for ( $i = 0; 1 ; $i++ ) {
  print "Event:\n";
  $elist[$i]{'NAME'}     = getinput(" <NAME> ?");
  last if ( $elist[$i]{'NAME'} eq "." );
  $elist[$i]{'COMMENT'}  = getinput(" <COMMENT> ?");
  print "\n";
 }
 $nevents = $i;


 #######################################################################
 #######################################################################
 ##
 ##
 ## PHASE 3:  Get EXCEPTION information. We will present a list of exceptions
 ##           tto the user and let him pick the ones he wants
 ##
 #######################################################################
 #######################################################################

 @KNOWN_EXCEPTIONS = (
  "EXCEPT_ICMISS",
  "EXCEPT_INDEXMP",
  "EXCEPT_WAYMP",
  "EXCEPT_SLOT1SQSH",
  "EXCEPT_CBRANCHMP",
  "EXCEPT_JUMPMP",
  "EXCEPT_RETURNMP",
  "EXCEPT_STARTTHREAD",
  "EXCEPT_KILLTHREAD",
  "EXCEPT_TRAP",
 );

 print <<"EofPhase3";
+--------------------------------------------------------------------+
| EXCEPTIONS                                                         | 
|                                                                    |
|  Please enter which exceptions you want to register for. Here is a |
|  list of possible exceptions:                                      |
|                                                                    |
EofPhase3

 for ( $i = 0; $i <= $#KNOWN_EXCEPTIONS; $i++ ) {
  print "\t$i: $KNOWN_EXCEPTIONS[$i]\n";
 }

 print <<"EofPhase3b";
|                                                                    |
|                                                                    |
|  Type in a comma-separated list of the exceptions you want to      |
|  register for.                                                     |
|                                                                    |
|  Example: <EXCEPTIONS>: 0,3,5,6                                    |
|                                                                    |
+--------------------------------------------------------------------+
EofPhase3b

 $exlist = getinput("<EXCEPTIONS> ?");
 @exarray = split(/,/,$exlist);

 #######################################################################
 #######################################################################
 ##
 ##
 ## GENERATE STRINGS
 ##
 ##
 #######################################################################
 #######################################################################

 $BOX  = $MINOR ? lc( $MAJOR . "_" . $MINOR ) : lc($MAJOR);
 $UBOX = $MINOR ? uc( $MAJOR . "_" . $MINOR ) : uc($MAJOR);
 $TRACE = "Trace_" . ucfirst(lc($MAJOR));

 #
 # Create an enumeration type with all buffers
 #
 $ENUM  = "enum $UBOX" . "_BUFFER_TYPE {\n";
 for ( $i = 0; $i < $nbufs ; $i++ ) {
  $comma = $i == ($nbufs - 1) ? "" : ",";
  $blist[$i]{'LONGNAME'} = "$UBOX" . "_" . uc($blist[$i]{'NAME'});
  $ENUM .= "\t$blist[$i]{'LONGNAME'}" . $comma . "\t\t// (" . lc($blist[$i]{'INOUT'}) . ")";
  $ENUM .= " $blist[$i]{'COMMENT'}\n";
 }
 $ENUM .= "     };\n";

 #
 # Create the buffer declarations for each buffer
 #
 @KNOWN_TYPES = ( "BOOL", "UINT32", "INT32", "UINT64", "INT64" );
 for ( $i = 0; $i < $nbufs ; $i++ ) {

  #
  # Construct the data type. We have to look for basic data types first; if we got one
  # of those, we create the explicit "ASIMBUFTEMPLATE" style of declaration. If it's not
  # a simple ddata type, then simply print it as is
  #
  $type = $blist[$i]{'DATATYPE'};
  if ( grep($type eq $_,@KNOWN_TYPES) ) { $type = "ASIM_BUFTEMPLATE<$type>\t*"; }
  elsif ( $type )                       { $type = "${type}BUF\t\t ";                  }
  else                                  { $type = "XXXXXXBUF\t\t ";             }

  $blist[$i]{'VARNAME'} = $blist[$i]{'NAME'} . "Buf";
  $DECL .= "\t" if ( $i > 0 );
  $DECL .= $type . $blist[$i]{'VARNAME'} . ";\n";
 }

 #
 # Do a 'NEW' for each buffer on the list
 #
 for ( $i = 0; $i < $nbufs ; $i++ ) {
  $type = $blist[$i]{'DATATYPE'};
  if ( grep($type eq $_,@KNOWN_TYPES) ) { $type = "ASIM_BUFTEMPLATE<$type>"; }
  elsif ( $type )                       { $type = "${type}BUF_CLASS";       }
  else                                  { $type = "XXXXXXBUF_CLASS";        }

  $CREATE .= " " . $blist[$i]{'VARNAME'} . "\t= NEW " . $type . "((ASIM_MODULE)this, \"";
  $CREATE .= $blist[$i]{'LONGNAME'} . "\");\n";
 }

 #
 # Do a 'delete' for each buffer on the list
 #
 for ( $i = 0; $i < $nbufs ; $i++ ) {
  $DELETE .= " delete " . $blist[$i]{'VARNAME'} . ";\n";
 }

 #
 # Print information for each buffer
 #
 for ( $i = 0; $i < $nbufs ; $i++ ) {
  $PRINTINFO .= " " . $blist[$i]{'VARNAME'} . "->PrintInfo();\n";
 }

 #
 # Generate the different "case" statements for the GetBufferPtr function
 #
 for ( $i = 0; $i < $nbufs ; $i++ ) {
  $GETBPTR .= "  case " . $blist[$i]{'LONGNAME'} . ":\t\treturn((ASIM_BUFFER *)&";
  $GETBPTR .= $blist[$i]{'VARNAME'} . ");\n";
 }

 #
 # Check input buffers at the end of the clock routine
 #
 for ( $i = 0; $i < $nbufs ; $i++ ) {
  $CHECKBUF .= " " . $blist[$i]{'VARNAME'} . "->CheckBuf(cycle);\n" if ( $blist[$i]{'INOUT'} =~ /in/i );
 }

 #
 # Generate the event declarations
 #
 for ( $i = 0; $i < $nevents ; $i++ ) {
  $DECLEVENT .= "\tASIM_EVENT\t" . $elist[$i]{'NAME'} . "Event;\n";
 }

 #
 # Generate the event registration
 #
 for ( $i = 0; $i < $nevents ; $i++ ) {
  $REGEVENT .= " " . $elist[$i]{'NAME'} . "Event\t= RegisterEvent(\"";
  $REGEVENT .= $elist[$i]{'NAME'} . "\", \"$elist[$i]{'COMMENT'}\");\n";
 }

 #
 # Register for all exceptions required
 #
 for ( $i = 0; $i <= $#exarray; $i++ ) {
  $ename = $KNOWN_EXCEPTIONS[$exarray[$i]];
  $REGEXCEPTION .= " Except()->Register(this, " . $ename . ");\n";
 }

 #
 # Create exception cases
 #
 for ( $i = 0; $i <= $#exarray; $i++ ) {
  $ename = $KNOWN_EXCEPTIONS[$exarray[$i]];
  $EXCEPTION .= "  case $ename:\n";
  $EXCEPTION .= "              break;\n\n";
 }

 #######################################################################
 #######################################################################
 ##
 ##
 ## OPEN TEMPLATE FILES  AND OUTPUT FILES
 ##
 ##
 #######################################################################
 #######################################################################

 chomp ($tmpl_h = `$RESOLVER tools/newbox_template.h`);
 if ($? != 0) {
  die "ERROR:\n$tmpl_h\n";
 }
 die $tmpl_h  if ($? != 0);
 open(TH,"$tmpl_h") || die "can not open $tmpl_h\n";
 chomp ($tmpl_cpp = `$RESOLVER tools/newbox_template.cpp`);
 if ($? != 0) {
  die "ERROR:\n$tmpl_cpp\n";
 }
 die $tmpl_cpp  if ($? != 0);
 open(TCPP,"$tmpl_cpp") || die "can not open $tmpl_cpp\n";

 if ( $DOH ) {
  open(NH,">${BOX}.h") || die "can not open ${BOX}.h\n";
 }
 if ( $DOCPP ) {
  open(NCPP,">${BOX}.cpp") || die "can not open ${BOX}.cpp\n";
 }
 open(NDEF,">${BOX}.def") || die "can not open ${BOX}.def\n";

 #######################################################################
 #######################################################################
 ##
 ##
 ## PROCESS .h FILE
 ##
 ##
 #######################################################################
 #######################################################################

 while ( <TH> ) {

  s/XX_PROVIDES_XX/$PROVIDES/g;
  s/XX_BOX_XX/$BOX/g;
  s/XX_UBOX_XX/$UBOX/g;
  s/XX_BUFFER_ENUM_XX/$ENUM/g;
  s/XX_BUFFER_DECLARATIONS_XX/$DECL/g;
  s/^.*XX_EVENT_DECLARATIONS_XX.*\n/$DECLEVENT/g;

  print NH $_ if ( $DOH );
 }

 while ( <TCPP> ) {

  s/XX_PROVIDES_XX/$PROVIDES/g;
  s/XX_BOX_XX/$BOX/g;
  s/XX_UBOX_XX/$UBOX/g;
  s/XX_TRACE_XX/$TRACE/g;

  s/XX_CREATE_BUFFERS_XX\n/$CREATE/g;
  s/XX_REGISTER_EVENTS_XX\n/$REGEVENT/g;
  s/XX_REGISTER_EXCEPTIONS_XX\n/$REGEXCEPTION/g;
  s/XX_DELETE_BUFFERS_XX\n/$DELETE/g;
  s/XX_PRINT_BUF_INFO_XX\n/$PRINTINFO/g;
  s/XX_GET_BUFFER_PTR_XX\n/$GETBPTR/g;
  s/XX_CHECK_BUF_XX\n/$CHECKBUF/g;
  s/XX_EXCEPTION_XX\n/$EXCEPTION/g;

  print NCPP $_ if ( $DOCPP );
 }

 #######################################################################
 #######################################################################
 ##
 ##
 ## END OF PROGRAM
 ##
 ##
 #######################################################################
 #######################################################################

 print NDEF $ALLINPUT;

 print "Done\n";
 exit 0;

sub cleanup_and_exit
{
 my($msg) = $_[0];


 unlink $tmpfile 	if ( $tmpfile );
 unlink $mylock 	if ( $mylock );
 unlink $tarfile 	if ( $tarfile );
 rmdir  $asimlock 	if ( $asimlock );

 if ( $modchanges ) {
  print "Reverting 'changes' file to previous state...\n";
  unlink "$ASIMDIR/changes";
  chdir $ASIMDIR;
  system "cvs update changes"; 
 }

 if ( $new_goldstats ) {
  print "Reverting the gold stats directory ($GOLDSTATS) to previous state...\n";
  chdir $GOLDSTATS;
  system "rm -f *.stats";
  system "cvs update .";
 }

 if ( $modipcfile ) {
  print "Reverting the ${oipcfile} to previous state...\n";
  system "rm -f ${oipcfile}";
  system "cvs update ${oipcfile}";
 }

 print "$msg";
 exit -1;
}

sub getinput
{
 my($msg) = $_[0];
 my($val) = $_[1];

 if ( $val ) {
  print "$msg [$val] : ";
 }
 else {
  print "$msg : ";
 }

 $in = <STDIN>;

 $in =~ s/^\s+//;	## eliminate white spaces before the input
 $in =~ s/\s+$//;	## eliminate white spaces after the input

 if ( $val ) {
  if ( $in ) {
   $ALLINPUT .= "$in\n";
   return $in;
  }
  $ALLINPUT .= "$val\n";
  return $val;
 }
 else {
  $ALLINPUT .= "$in\n";
  return $in;
 }
}

sub conditional_exit
{
 my($yorno) = $_[0];

 if ( $yorno ne "y" && $yorno ne "yes" ) {
  cleanup_and_exit("asimcommit [Aborting]...\n");
 }
}

sub user_abort
{
 my $signame = shift;

 cleanup_and_exit("asimcommit [Aborting due to ^C]...\n");
}

sub no_ctrl_c
{
 my $signame = shift;

 print STDERR "Sorry ^C is not acceptable at this point...\n";
}

sub partial_match
{
 my $dir = $_[0];
 my $aref =  $_[1];

 foreach $dangerdir ( @$aref ) {
  if ( $dir =~ $dangerdir ) {
   return 1;
  }
 }
 return 0;
}

sub read_path
{
 my $p;

 $p = <STDIN>;
 chop $p;
 $p =~ s/^\s+//;	## eliminate white spaces before the path
 $p =~ s/\s+$//;	## eliminate white spaces after the path
 if ( $p ne "" && ! -d $p ) { cleanup_and_exit("Sorry, '$p' is not a directory: $!\n"); }

 return $p;
}
