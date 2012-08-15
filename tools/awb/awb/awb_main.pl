package main;

use strict;

use File::Basename;
use File::Spec;
use Getopt::Long;

use awb_dialog;

use Asim;

$debug = $ENV{AWB2_DEBUG} || 0;

my $workspace;
my $help;

my $status;

#
# Parse and process command line switches
#
$status = GetOptions( "workspace=s"    => \$workspace,
                      "help"           => \$help
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
# Process --workspace
#
if ($workspace) {
  $ENV{AWBLOCAL} = File::Spec->rel2abs($workspace);
}

Asim::init()
  || die ("FAILURE: Workspace could not be initialized\n");



#
# Create main window
#
our $app = Qt::Application(\@ARGV);
my $w = awb_dialog();
$w->init();

$w->show();
exit $app->exec();


__END__

=head1 NAME

Awb - Asim GUI interface

=head1 SYNOPSIS

awb  [--workspace <workspace>]
     [--help]

=head1 DESCRIPTION

The "architects workbench", awb, is a program that provides a GUI to
access the models and benchmarks of an ASIM workspace.

=head1 SWITCHES

The following command line switches are currently supported:

=over 4

=item --workspace <workspace>

Set current workspace to be rooted at the directory <workspace>. Impliclty
this sets the environment variable $AWBLOCAL to be <workspace> and the
file $AWBLOCAL/awb.config must exist.

=item --help

Get this help information.

=back

=head1 OPERATION

On 

=head1 BUGS

Unknown

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (C) 2002-2006 Intel Corporation

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

=cut

