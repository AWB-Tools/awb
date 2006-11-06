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

# Author: Joel Emer
# Date: November 2002

package main;

use strict;

use File::Basename;
use File::Spec;
use Getopt::Long;

use Qt;
use awb_wizard;

$::debug = $ENV{AWB_WIZARD_DEBUG} || 0;

my $workspace;
my $help;

my $status;

#
# Parse and process command line switches
#
$status = GetOptions( "help"           => \$help
                    );


if (!$status) {
  my $prog = basename($0);
  print STDERR "$prog: Illegal argument, try '$prog --help'\n";
  exit 0;
}

#
# Process --help
#
if ($help) {
  system "perldoc -t $0";
  exit 0;
}


#
# Create main window
#
our $app = Qt::Application(\@ARGV);
my $w = awb_wizard;

$app->setMainWidget($w);
$w->show();

if (defined $ARGV[0]) {
  $w->fileOpen($ARGV[0]);
}

exit $app->exec();


__END__

=head1 NAME

awb-wizard - Asim GUI interface to create a new module

=head1 SYNOPSIS

awb  [--help] [<amz file>]

=head1 DESCRIPTION

The "architects workbench wizard", awb-wizard, is a program that 
provides a GUI to create a new asim module.

=head1 SWITCHES

The following command line switches are currently supported:

=over 4

=item --help

Get this help information.

=item <amz file>

A file containing the wizard information filled in once before for
creating another copy of a module skeleton.

=back

=head1 OPERATION

Fill in all the fields and click build to create a skeleton of a new
asim module. The files created are a .awb file a .cpp file and a .h file
for the module.

Use the "what's this" help to find out more about the information
required in each field.

You can use the open/save/saveas operations to save and restore the
information you have filled in. Note: that this is not the module
files themselves, but just the information in the fields that you have
filled in. This program will not allow you to update an already
created .awb, .cpp or .h files.

=head1 BUGS

Add more documentation.

=head1 AUTHORS

Joel Emer, Julio Gago

=head1 COPYRIGHT

 ********************************************************
 *                                                      *
 *   Copyright (c) Intel Corporation, 2002              *
 *                                                      *
 *   All Rights Reserved.  Unpublished rights reserved  *
 *   under the copyright laws of the United States.     *
 *                                                      *
 ********************************************************

=cut

