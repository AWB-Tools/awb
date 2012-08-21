
package main;

use Getopt::Long;

use QtCore4;
use QtGui4;

use Asim;

use apm_find_replace;

#
# Parse and process command line switches
#
my $status;

my $help = 0;
my $list = undef;

$status = GetOptions( "list"           => \$list,
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

Asim::init();

my $a;
my $w;

$a = Qt::Application(\@ARGV);
$w = apm_find_replace();

$w->init();

$w->show();

$status = $a->exec();

if ($status) {
  exit $status;
}

if ($list) {
  my $modelhash = $w->models(1);

  foreach my $i (sort keys %{$modelhash}) {
    print $modelhash->{$i}->filename() . "\n";
  }
}


__END__

=head1 NAME

Apm-find-replace

=head1 SYNOPSIS

apm-find-replace [--help]

=head1 DESCRIPTION

Apm-find-replace is a graphical interface for doing bulk replacement 
of a module with another module (or submodel) in a set of models.

=head1 SWITCHES

The following command line switches are currently supported:

=over 4

=item --list

At exit, print the filenames for each of the selected models. This
allows this program to be used as a sort of model selection dialog.

=item --help

Get this help information.

=back

=head1 OPERATION

To use apm-find-replace one must first start a new search with the
"New" button. Then in the upper left text box one can select a
specific directory to be the origin of the model search (the given
directory and all its children will be searched recursively - abiding
by the AWB uniondir structure). Below that, one must select an asim
module type to be replaced. And then one clicks the "Find" button to
search for all models that use a module of the specificed type. The
names of those modles are listed in the next text box.

One must then select one or more of the models. The filter boxes can
be used to quickly select a set of models whose names match a given
Perl regular expression.

Then one selects a specific module to be substituted into each of the
selected models. The "Replace" button performs the replacement and the
"Save" button saves the model files. The original file is backed up
using the GNU emacs file versioning scheme, i.e., a version number of
the form .~<version>~.


=head1 BUGS

This tool relies on the pervasive assumption that a model can only
have a single module of a given asim module type.

=head1 AUTHOR

Joel Emer

=head1 COPYRIGHT

 ********************************************************
 *                                                      *
 *   Copyright (c) Intel Corporation, 2007              *
 *                                                      *
 *   All Rights Reserved.  Unpublished rights reserved  *
 *   under the copyright laws of the United States.     *
 *                                                      *
 ********************************************************

=cut

