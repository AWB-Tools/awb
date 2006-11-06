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

eval 'exec perl -w "$0" ${1+"$@"}'
       if 0;

#new
use File::Basename;
use Getopt::Std;
use Cwd 'realpath';

if ((defined $ARGV[0] && $ARGV[0] eq "-help" ) 
    ||(defined $ARGV[0] && $ARGV[0] eq "-h" )
    ||(defined $ARGV[0] && $ARGV[0] eq "-H" ))
  {
      print STDERR "This is a compress of the output for the gambit trace;\n";
      print STDERR "The options that are avaiable are: \n";
      print STDERR "-s=this defines how many instructions in the warmup time\n";
      print STDERR "-i= this is how many instruction bundles to put in each trace file\n";
      print STDERR "-p= this is where to store the files to\n";
      print STDERR "-b= this is the base name of the file that is added with file number\n";
      print STDERR "-f= file to read gambit trace from assume stdin otherwise\n";
      print STDERR "-g= gzip the files\n";
      print STDERR "-r= redirect output to stdout\n";
      exit 0;
    }

#global varibables
$start=1000000;
$interval=1000000;
$path=".";
$basename="gtrace";
$filename="stdin";
$gzip=0;
$count=0;
$fulloutname="";
$redirect =0;

$linenumber=0;
$tracestarted=0;

$stringval=0;
$debugarea=0;
#parse command file
getopts('s:i:p:b:f:g:r', \%opts);
#set the options
if ($opts{s})  { $start=$opts{s};}
if ($opts{i}) { $interval=$opts{i};}
if ($opts{p}) { $path=$opts{p};}
if ($opts{b}) { $basename=$opts{b};}
if ($opts{f}) { $filename=$opts{f};}
if ($opts{g}) { $gzip=$opts{g};}
if ($opts{r}) { $redirect=$opts{r};}

#print the usage
print STDERR "Using the following configuration\n";
print STDERR "start: $start\n";
print STDERR "interval: $interval\n";
print STDERR "path: $path\n";
print STDERR "basename: $basename\n";
print STDERR "filename: $filename\n";
$fulloutname=$path . "/". $basename."_".$count;
#open(OUT_FILE, ">STDOUT")

#print "??\n";
if($redirect eq 1){
print STDERR "output to STDOUT\n";
}
else
  {
    print STDERR "output to file\n";
    open(OUTFILE, ">$fulloutname")
      || die "Cannot open the trace file $fulloutname";
  }

#loop through the input
if( $filename eq "stdin" ){
  while(<>){
    $linenumber++;
    event_type();
  }
}
else {
  open(TRC_FILE, $filename)
    || die "Cannot open the trace file $filename";

  while(<TRC_FILE>){
    $linenumber++;
    event_type();
  }
}
exit;

#####################
#Subroutines
####################
sub unsupported{
  print STDERR "use of unsupported form\n";
  print STDERR "linenumber: $linenumber\nvalue: $_";
  exit;
}

sub nextline{
if( $filename eq "stdin" )
  {
    $_= <>;
  }
else
  {
    $_=<TRC_FILE>;
  }
  $linenumber++;
if($debugarea eq 1)
  {print STDERR $_;}
}
#######################
#bundle bits
######################
sub bb{
  #chop;
  nextline;
  #print STDERR "'$_'\n";
  if (/^\s+(data.FIXED.instruction.bundle_bytes)\s+(U128)\s+(0x)([.0-9a-fA-F]+)/)
    {
      printout("bb $4\n");
      $count++;
    }
  else
    {
      print STDERR "bundle parse error '$_'\n";
    }
    #if(($count%100000)==0)
#	{
#	print STDERR "processed $count instructions";
#	}
}

###################################
#print to either a file or stdout
###################################
sub printout{
  my $STRINGVAR= lc($_[0]);
          if($debugarea eq 1){
        print STDERR  $STRINGVAR; }
 
  if($redirect eq 1)
    {
      print $STRINGVAR;
    }
  else
    {
      print OUTFILE $STRINGVAR;
    }
}
#####################################
#
####################################
sub printouth{
  my $STRINGVAR= lc($_[0]);
          if($debugarea eq 1){
        print STDERR  sprintf("%x", $STRINGVAR); }
 
  if($redirect eq 1)
    {
      printf sprintf("%x",$STRINGVAR);
    }
  else
    {
      printf OUTFILE sprintf("%x", $STRINGVAR);
    }
}

###################################
#line fetch
###################################
sub lf{
  unsupported;
  nextline;
  nextline;
}
###################################
#memory access
###################################
sub ma{
  unsupported;
  nextline;
  nextline;
  nextline;
  nextline;
  nextline;
  nextline;
}
########################################
#flush cache
#####################################
sub fc{
  unsupported;
    nextline;
}


######################################
#branch
#######################################
sub br{
  #unsupported;
  #print STDERR $_;
  if(/^form.FIXED.branch: event.FIXED.branch\.([a-zA-Z_]+)\.([a-zA-Z]+)\.([a-zA-Z]+)\n/)
    {
      #print STDERR "one";
      $br_taken=uc($1);
      if($2 eq "call"){ $br_type="CALL";}
      else { $br_type=uc($3)};
    }
  elsif(/^form.FIXED.branch: event.FIXED.branch\.([a-zA-Z_]+)\.([a-zA-Z]+)\n/)
    {
      #print STDERR "two";

      $br_taken=uc($1);
      $br_type=uc($2);
    }
  elsif (/^form.FIXED.branch: event.FIXED.branch\.none\n/)
    {
      #print STDERR "three";

      #print STDERR "other";
      $br_taken="NONE";
      $br_type="NONE";
    }
  else    {parse_error();}

  nextline;
  if(/^\s+data.ARCH.IL\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_IL=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.branch.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_id=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.branch.register\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_register=$1;
  }else  {parse_error(); }
      
  nextline;
  if(/^\s+data.FIXED.branch.hint\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_hint=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.branch.target\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_target=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.ARCH.register.CFM\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_cfm=$1;
  }else  {parse_error(); }

  #print out the gtrace line
  printout("br "); #CC code
  $stringval=$br_type; #type
  #print STDERR  $stringval;
  wordswitch();
  printout(" "); 
  $stringval=$br_taken; #taken or not take
  #print STDERR  $stringval;
  wordswitch();
  printout(" ".$br_IL); #the branch address
  printout(" ".$br_target); #the branch target
  printout(" ".$br_hint); #the branch hint
  printout(" ".$br_cfm); #the cfm
  printout(" ".$br_id); #the id of the instruction
  printout(" ".$br_register."\n"); #the register used in the branch

}
###################################
#branch hint
#####################################
sub br_hint{
#  unsupported;
  nextline;

  if(/^\s+data.ARCH.IL\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_IL=$1;
  }else  {parse_error(); }

  nextline;  
  if(/^\s+data.FIXED.branch_hint.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_id=$1;
  }else  {parse_error(); }
  
  nextline;
  if(/^\s+data.FIXED.branch_hint.hint\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_hint=$1;
  }else  {parse_error(); }
  
  nextline; 
  if(/^\s+data.FIXED.branch_hint.tag\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_tag=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.branch_hint.target\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $br_target=$1;
  }else  {parse_error(); }

  printout("bh");
  printout(" ".$br_IL);
  printout(" ".$br_target);
  printout(" ".$br_hint);
  printout(" ".$br_tag);
  printout(" ".$br_id."\n");

}
#####################################
#timed branch
##################################
sub br_timed{
  unsupported;
  nextline;
  nextline;
  nextline;
  nextline;
  nextline;
  nextline;
}
#####################################
#short branch
##################################
sub br_short{
  unsupported;
  nextline;
  nextline;
  nextline;
  nextline;
}
#####################################
#partial branch hint
##################################
sub br_partial{
  unsupported;
  nextline;
  nextline;
  nextline;
}
#####################################
#move to br
##################################
sub br_move_to_br{
  unsupported;
  nextline;
  nextline;
  nextline;
}
##################################
#change predicates 1
###################################
sub c1{
#  unsupported;
  nextline();
  if(/^\s+data.ARCH.IL\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_IL=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_id=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.register.1\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_reg=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.value.1\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_val=$1;
  }else  {parse_error(); }

  printout("c1");
  printout(" ".$pred_IL);
  printout(" ".$pred_reg);
  printout(" ".$pred_val);
  printout(" ".$pred_id."\n");
}
##################################
#change predicates 2
###################################
sub c2{
  nextline();
  if(/^\s+data.ARCH.IL\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_IL=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_id=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.register.1\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_reg1=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.value.1\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_val1=$1;
  }else  {parse_error(); }
  nextline;
  if(/^\s+data.FIXED.predicate.register.2\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_reg2=$1;
  }else  {parse_error(); }

  nextline;
  if(/^\s+data.FIXED.predicate.value.2\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_val2=$1;
  }else  {parse_error(); }

  printout("c2");
  printout(" ".$pred_IL);
  printout(" ".$pred_reg1);
  printout(" ".$pred_val1);
  printout(" ".$pred_reg2);
  printout(" ".$pred_val2);
  printout(" ".$pred_id."\n");

}
##################################
#set predicates
###################################
sub sp{
  #unsupported;
  #print STDERR $_;
  nextline;
  if(/^\s+data.ARCH.IL\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_IL=$1;
  }else  {parse_error(); }
  nextline;
  if(/^\s+data.FIXED.predicate.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_id=$1;
  }else  {parse_error(); }
  nextline;
  if(/^\s+data.FIXED.predicate.registers\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $pred_reg=$1;
  }else  {parse_error(); }
  #print STDERR $_;
  #nextline;
  #print STDERR $_;
  #unsupported;
  printout("sp");
  printout(" ".$pred_IL);
  printout(" ".$pred_reg);
  printout(" ".$pred_id."\n");
}

##################################
#memory read
###################################
sub mem_read{
#  unsupported;
  $stringval="READ";

  nextline;
  if (/^\s+data.FIXED.memory_access.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $inst_id=$1;
  }
  else{
    parse_error();
  }
  nextline;
  if (/^\s+data.FIXED.memory_access.size\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $mem_size=$1;
  }
  else{
    parse_error();
  }

  nextline;
  if (/^\s+data.FIXED.memory_access.value\s+U128\s+0x([0-9a-fA-F]+)\n/){
    $mem_value=$1;
  }
  else{
    parse_error();
  }

  #output the line 
  printout("mv "); #output cc code
  wordswitch();; #output type
  printout(" ".$mem_size); #output size
  printout(" ".$mem_value);
  printout(" ".$inst_id."\n");

}
sub parse_error(){
  print STDERR "parse error at $linenumber\n";
  print STDERR "value $_";
}
##################################
#memory write
###################################
sub mem_write{
#  unsupported;
#  my $inst_id;
#  my $mem_type;
#  my $mem_size;
#  my $mem_value;
  $stringval="WRITE";

  nextline;
  if (/^\s+data.FIXED.memory_access.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $inst_id=$1;
  }
  else{
    parse_error();
  }
  nextline;
  if (/^\s+data.FIXED.memory_access.size\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $mem_size=$1;
  }
  else{
    parse_error();
  }

  nextline;
  if (/^\s+data.FIXED.memory_access.value\s+U128\s+0x([0-9a-fA-F]+)\n/){
    $mem_value=$1;
  }
  else{
    parse_error();
  }

  #output the line 
  printout("mv "); #output cc code
  wordswitch(); #output type
  printout(" ".$mem_size); #output size
  printout(" ".$mem_value);
  printout(" ".$inst_id."\n");
}
################################
#data
###############################
sub da{
#  unsupported;
#  print STDERR "TESTda";
#        form.FIXED.data_access: event.FIXED.memory_access.write
	$debugarea=0;
  if (/^form.FIXED.data_access: event.FIXED.memory_access\.(.+)\n/){
    $da_type=uc($1);
  }
  else{parse_error(); }
  nextline;

  if (/^\s+data.ARCH.IL\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $da_IL=$1;
  }
  else{parse_error(); }

  nextline;
  if (/^\s+data.FIXED.memory_access.inst_id\s+U16\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $da_id=$1;
  }
  else{parse_error(); }

  nextline;
  if (/^\s+data.FIXED.memory_access.hint\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $da_hint=$1;
  }
  else{parse_error(); }

  nextline;
  if (/^\s+data.FIXED.memory_access.size\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $da_size=$1;
  }
  else{parse_error();}


  nextline;
  if (/^\s+data.FIXED.memory_access.address\s+U64\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $da_address=$1;
  }
  else{parse_error(); }

  nextline;
  if (/^\s+data.FIXED.memory_access.non_access\s+U8\s+0x([0-9a-fA-F]+)\s+[0-9]+\n/){
    $da_non_access=$1;
  }
  else{parse_error();}

  printout("da ");
  $stringval=$da_type;
  wordswitch();
  printout(" ".$da_IL);
  printout(" ".$da_address);
  printout(" ".$da_hint);
  printout(" ".$da_size);
  printout(" ".$da_non_access);
  printout(" ".$da_id."\n");
  $debugarea=0;
  
}
################################
#line fetch IL
###############################
sub lf_IL{
  unsupported;
  nextline;
  nextline;
  nextline;
}
################################
#flush cache IL
###############################
sub fc_IL{
  unsupported;
  nextline;
  nextline;
}

sub event_type{


  #branch
  if(/^form.FIXED.branch: event.FIXED.branch.([a-zA-Z_]+).([.a-zA-Z]+)\n/)
    {

      br();
    }
  #branch_hint
  elsif(/^form.FIXED.branch_hint:\s+(.+)\n/)
    {
      br_hint();
    }
  #timed branch
  #elsif(/^form.FIXED.timed_branch:\s+(.+)\n/)
  #  {
  #    br_timed();
  #  }
  #short br
  #elsif(/^form.FIXED.short_branch:\s+(.+)\n/)
  #  {
  #    br_short();
  #  }

  #partial_Branch_hint  
  #elsif(/^form.FIXED.partial_branch_hint:\s+(.+)\n/)
  # {
  #    br_partial();
  #  }
  #move_to_br
  #elsif(/^form.FIXED.move_to_br:\s+(.+)\n/)
  #  {
  #    br_move_to_br();
  #  }
  #change predicate 1
  elsif(/^form.FIXED.change_1_pred:\s+(.+)\n/)
    {
     c1();
    }
  #change predicate 2
  elsif(/^form.FIXED.change_2_pred:\s+(.+)\n/)
    {
     c2();
    }
  #set predicate 1
  elsif(/^form.FIXED.set_preds:\s+(.+)\n/)
    {
     sp();
    }
  #memory_access
  elsif(/^form.FIXED.memory_access:\s+(.+)\n/)
    {
      ma();
    }
  #line fetch
  #if (/^(form.FIXED.memory_access: event.FIXED.code_fetch)\n/)
  #  {
  #    lf();
  #  }

  #line fetch
  elsif(/^form.FIXED.line_fetch:\s+(.+)\n/)
    {
      lf();
    }
  #flush
  #elsif(/^form.FIXED.flush_cache:\s+(.+)\n/)
  #  {
  #    fc();
  #  }
  #data access
  elsif(/^form.FIXED.data_access:\s+(.+)\n/)
    {
      da();
    }
  elsif(/^form.FIXED.line_fetch_IL:\s+(.+)\n/)
    {
      lf_IL();
    }
  #elsif(/^form.FIXED.flush_cache_IL:\s+(.+)\n/)
  #  {
  #    fc_IL();
  #  }
  elsif(/^form.FIXED.mem_value: event.FIXED.memory_access.read\n/)
    {
      mem_read();
    }
  elsif(/^form.FIXED.mem_value: event.FIXED.memory_access.write\n/)
    {
      mem_write();
    }
  elsif (/^(form.FIXED.bundle_bytes: event.FIXED.instruction.execute)\n/)
    {
      bb();
      #print OUT_FILE "match made '$_'\n";
    }
  elsif (/^Trace version 3.2/)
    {
      $tracestarted=1;
    }
  elsif (/^\n/)
    {
      #white space ignore
    }
  else {
    if($tracestarted)
      {
        print STDERR "warning:could not parse line number $linenumber: '$_'\n";
      }
  }

}


sub wordswitch{

#if ($stringval eq "ACCESSES"){ printouth((3136)); } 

#if ($stringval eq "ADDRESS"){ printouth((3080)); } 
#elsif ($stringval eq "BRANCH"){ printouth((3071)); } 
if ($stringval eq "BRANCH_HINT"){ printouth((3072)); } 
elsif ($stringval eq "BRANCH_TAKEN"){ printouth((3249)); } 
#elsif ($stringval eq "BUNDLE_BYTES"){ printouth((4106)); } 
elsif ($stringval eq "CALL"){ printouth((3261)); } 
#elsif ($stringval eq "CFM"){ printouth((3034)); } 
elsif ($stringval eq "COND"){ printouth((3260)); } 
elsif ($stringval eq "CLOOP"){ printout(hex(3265)); }
elsif ($stringval eq "CTOP"){ printout(hex(3266)); }
elsif ($stringval eq "DIRECT"){ printouth((3269)); } 
#elsif ($stringval eq "EXECUTE"){printouth((4109));}
#elsif ($stringval eq "HINT"){printouth((3077)); } 
#elsif ($stringval eq "IMPLICIT_MEMORY_ACCESS"){printouth((4151)); } 
elsif ($stringval eq "INDIRECT"){printouth((3270)); } 
#elsif ($stringval eq "LOAD"){printouth((3092)); } 
#elsif ($stringval eq "MEMORY"){printouth((3125)); } 
#elsif ($stringval eq "MEMORY_ACCESS"){printouth((3073)); } 
#elsif ($stringval eq "MOVE_TO_BR"){printouth((3075)); } 
elsif ($stringval eq "NONE"){printouth((3257)); } 
#elsif ($stringval eq "NON_ACCESS"){printouth((3081)); } 
elsif ($stringval eq "NOT_TAKEN"){printouth((3259)); } 
#elsif ($stringval eq "PREDICATE"){printouth((3074)); } 
elsif ($stringval eq "READ"){printouth((3238)); } 
#elsif ($stringval eq "READS"){printouth((3138)); } 
#elsif ($stringval eq "REGISTER"){printouth((3017)); } 
#elsif ($stringval eq "REGISTERS"){printouth((3083)); } 
elsif ($stringval eq "RET"){printouth((3268)); } 
elsif ($stringval eq "RFI"){printouth((3246)); } 
#elsif ($stringval eq "RSE_READ"){printouth((4152)); } 
#elsif ($stringval eq "RSE_WRITE"){printouth((4153)); } 
#elsif ($stringval eq "TAG"){printouth((3079)); } 
elsif ($stringval eq "TAKEN"){printouth((3258)); } 
#elsif ($stringval eq "THREAD"){printouth((3090)); } 
#elsif ($stringval eq "THREADID"){printouth((3105)); } 
elsif ($stringval eq "UPDATE"){printouth((3271)); }  
#elsif ($stringval eq "VALUE"){printouth((3082)); } 
elsif ($stringval eq "WRITE"){printouth((3237)); } 
#elsif ($stringval eq "WRITES"){printouth((3139)); } 
else
  {
    print STDERR "unknown word!!! $stringval\n"; exit;}

}
