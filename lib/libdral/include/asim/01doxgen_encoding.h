//
// @ORIGINAL_AUTHOR: Federico Ardanaz
//
// This file does not contain any code
// it just contains additional information for
// inclusion with doxygen


// ========================================================================================  
/*! 

\page Encoding
 
Written by ... 

\section INTRO Introduction

DRAL version major 2 has 20 commands. This chapter explains the
encoding of each command.

\section IDCMD Identifying the commands

Dral commands are identified by their name, but they also can be identified
according to their command code used in the binary format.

The following table shows the relation between the command names and their
command codes:

<p><DIV ALIGN=center>
<table border>
<TR> <TD align=center>
Command code </TD> <TD align=center> Command name </TD></TR>
<TR> <TD align=center>
000000 </TD> <TD align=center> cycle </TD></TR>
<TR> <TD align=center>
000001 </TD> <TD align=center> newItem </TD></TR>
<TR> <TD align=center>
000010 </TD> <TD align=center> moveItems </TD></TR>
<TR> <TD align=center>
000011 </TD> <TD align=center> deleteItem </TD></TR>
<TR> <TD align=center>
000100 </TD> <TD align=center> setItemTagSingleValue </TD></TR>
<TR> <TD align=center>
000101 </TD> <TD align=center> setItemTagString </TD></TR>
<TR> <TD align=center>
000110 </TD> <TD align=center> setItemTagSet </TD></TR>
<TR> <TD align=center>
000111 </TD> <TD align=center> enterNode </TD></TR>
<TR> <TD align=center>
001000 </TD> <TD align=center> exitNode </TD></TR>
<TR> <TD align=center>
001001 </TD> <TD align=center> newNode </TD></TR>
<TR> <TD align=center>
001010 </TD> <TD align=center> newEdge </TD></TR>
<TR> <TD align=center>
001011 </TD> <TD align=center> setLayout </TD></TR>
<TR> <TD align=center>
001100 </TD> <TD align=center> comment </TD></TR>
<TR> <TD align=center>
001101 </TD> <TD align=center> version </TD></TR>
<TR> <TD align=center>
001110 </TD> <TD align=center> setNodeTagSingleValue </TD></TR>
<TR> <TD align=center>
001111 </TD> <TD align=center> setNodeTagString </TD></TR>
<TR> <TD align=center>
010000 </TD> <TD align=center> setNodeTagSet </TD></TR>
<TR> <TD align=center>
010001 </TD> <TD align=center> setCycleTagSingleValue </TD></TR>
<TR> <TD align=center>
010010 </TD> <TD align=center> setCycleTagString </TD></TR>
<TR> <TD align=center>
010011 </TD> <TD align=center> setCycleTagSet </TD></TR>
<TR> <TD align=center>
</table>
</DIV>

\section BINENCNOTES Binary encoding notes

 The base of the binary encoding is what we call a packet (a 64-bit word).
 All the commands will consist of one or more of these packets.
 In many commands one will find that there is no enough data to fill those
 64-bit packets. If that happens in the header packets, those bits are 
 called RESERVED bits, and must be set to 0.
 There are also commands with some data with a variable length. That length
 may not fit the packet size too, so, we will use the
 ROUNDUP(real\_data\_length\_in\_packets) function to know how many packets that
 data will occupy:

<p><table>
<TR> <TD align=right>    ROUNDUP(X)</TD> <TD align=left>X (if X is an integer) </TD></TR>
<TR> <TD align=right> &nbsp; </TD> <TD align=left>the upper nearest integer (if X is not an integer)
</table>
   
In the second case, padding bytes set to zero will be added until
the last packet is filled up.

The leftmost bit of the packets represented in this chapter is the bit
number 63. The rightmost one is the number 0.
All the packets must be stored in little endian order.


\section COMENC Command encoding

Alphabetically ordered according to the command name

*/

