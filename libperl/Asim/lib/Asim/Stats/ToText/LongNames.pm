#
# Copyright (C) 2003-2006 Intel Corporation
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

##
## Handler class for Asim stats-to-text converter.
## This version just prints scalars, with hierarchical names on each line
##
package       Asim::Stats::ToText::LongNames;
use           Asim::Stats::ToText;
our @ISA = qw(Asim::Stats::ToText);


=head1 NAME

Asim::Stats::ToText::LongNames

=head1 DESCRIPTION

Class to print Asim stats with long hierarchical names.
This is an example of how to derive a subclass of Asim::Stats::ToText
to print an alternate output format.

=head1 SYNOPSIS

use Asim::Stats::ToText::LongNames;

my $stat2txt = Asim::Stats::ToTextLongNames::new( $in_filename );

$stat2txt->transcribe( [ $out_filename ] );

=cut


=head1 PUBLIC API METHODS

The following public API methods are provided by this class.

=over 4

=item $obj = Asim::Stats::ToTextLongNames::new( $input_filename );

Create a new stats-to-text handler object, and specify the input file name

=cut

sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};
  bless	$self, $class;
  
  # initialize internal data structures
  $self->init( @_ );

  return $self;
}



=back

=head1 SUBCLASS CUSTOMIZED METHODS

The following print routines override the defaults
in Asim::Stats::ToText to provide an alternate output format:

=over 4


=item print_scalar( )

Print scalar info.
Prints the value followed by the full hierarchical name.

=cut

sub print_scalar {
  my $self = shift;
  printf {$self->{output_file}} "%20s   ", $self->{contents};
  foreach my $prefix ( @{$self->{name_stack}} ) {
    print {$self->{output_file}} $prefix, '.';
  }
  print {$self->{output_file}} $self->{properties}->{name}, "\n";
}


#
# Print a vector.
# Is this even really used?
# The only vectors we really care about are in histograms,
# and those are printed separately further below...
#
sub print_vector {
  my $self = shift;
  printf {$self->{output_file}} "%-20s   ", 'BEGIN_VECTOR';
  foreach my $prefix ( @{$self->{name_stack}} ) {
    print {$self->{output_file}} $prefix, '.';
  }
  print {$self->{output_file}} $self->{properties}->{name}, "\n";
  foreach my $value ( @{$self->{vector}} ) {
    printf {$self->{output_file}} "%20s\n", $value;
  }
  printf {$self->{output_file}} "END_VECTOR\n";
}


=item print_header( )

This routine prints nothing.

=cut

sub print_header {
  # do nothing
}


=item print_footer( )

This routine prints nothing.

=cut

sub print_footer {
  # do nothing
}


=item print_histogram_header( )

Print the header for a histogram.
This prints the keyword "HISTOGRAM" followed by the full hierarchical name.

=cut

sub print_histogram_header {
  my $self = shift;
  printf {$self->{output_file}} "%-20s   ", 'HISTOGRAM';
  foreach my $prefix ( @{$self->{name_stack}} ) {
    print {$self->{output_file}} $prefix, '.';
  }
  print {$self->{output_file}} $self->{histogram}->{name}, "\n";
  print {$self->{output_file}} "#            :";
  foreach my $cname ( @{$self->{histogram}->{column_names}} ) {
    printf {$self->{output_file}} " %20s", $cname;
  }
  print {$self->{output_file}} "\n";
}


=item print_histogram_footer( )

Print the footer for a histogram.
This prints the keyword "HISTOGRAM_END" followed by the full hierarchical name.

=cut

sub print_histogram_footer {
  my $self = shift;
  printf {$self->{output_file}} "%-20s   ", 'HISTOGRAM_END';
  foreach my $prefix ( @{$self->{name_stack}} ) {
    print {$self->{output_file}} $prefix, '.';
  }
  print {$self->{output_file}} $self->{histogram}->{name}, "\n";
}


=item indentation()

Print the current indentation level.
This style does not use level indentation,
so this routine does nothing.

=cut

sub indentation {
  # do nothing
}



=back

=head1 SEE ALSO

See also the documentation for the base class in Asim::Stats::ToText.

=head1 AUTHOR

Carl J. Beckmann

=head1 COPYRIGHT

Copyright 2007 Intel Corporation All Rights Reserved.

=cut
