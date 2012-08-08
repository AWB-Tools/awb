
package main;

use File::Basename;
use File::Spec;
use Getopt::Long;

use Asim;

use QtCore4;
use QtGui4;

use apm_edit_about;
use apm_edit;

my $debug = 0;

my $workspace;
my $help;
my $status;
my $check_health = 1;

#
# Parse and process command line switches
#
$status = GetOptions( "workspace=s"    => \$workspace,
                      "check-health!"  => \$check_health,
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
  system "perldoc $0";
  exit 0;
}

#
# Process --workspace
#
if ($workspace) {
  $ENV{AWBLOCAL} = File::Spec->rel2abs($workspace);
}

Asim::init()
  || die("FAILURE: Couldn't initialize workspace!\n");


#
# Create main window
#
our $app = Qt::Application(\@ARGV);
my $w = apm_edit();

$w->init();

$w->show();

if (defined $ARGV[0]) {

  $w->fileOpen($ARGV[0], $check_health);

}

exit $app->exec();


__END__

=head1 NAME

Apm-edit - GUI to edit asim model files

=head1 SYNOPSIS

awb  [--workspace <workspace>]
     [--help]

=head1 DESCRIPTION

This is a program that provides a GUI

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



=head1 BUGS

Unknown

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (C) 2002-2012 Intel Corporation

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

