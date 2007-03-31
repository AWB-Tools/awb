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
## This is a "handler" for a SAX-based parser.
##
package Asim::Stats::ToText;
use XML::SAX;
use XML::SAX::ParserFactory;
use strict;
use warnings;


=head1 NAME

Asim::Stats::ToText - Class to convert Asim stats files to plain text format

=head1 SYNOPSIS

use Asim::Stats::ToText;

my $stat2txt = Asim::Stats::ToText::new( $in_filename );

$stat2txt->transcribe( [ $out_filename ] );

=head1 DESCRIPTION

This class is used to parse an Asim stats file (in XML format), and to print it
in plain text for easier viewing or grepping.

This class provides a simple output format, but by deriving a subclass from this
you can override the output methods to print it in a different format.

=cut


=head1 OBJECT DATA MEMBERS

Objects of this class have the following members:

  input_filename  => name of the XML stats input file
  output_filename => name of the text stats output file
  output_file     => file handle to the output file
  filter_proc     => filter subroutine

The following are parser state variables:

  contents        => current text found in between tags
  char_stack      => stack of current text values
  name_stack      => stack of enclosing compound names
  in_scalar       => are we currently parsing a scalar?
  in_vector       => are we currently parsing a vector?
  in_histogram    => 0:no, 1:in header, 2: in data
  header_printed  => have we printed the header info yet?
  vector          => vector data
  properties      => hash with header info (type, name, desc...)
  histogram       => hash with histogram info (name, rows, cols...)

=cut

sub init {
  my $self = shift;
  
  # configuration variables:
  $self->{input_filename} = shift;   # remember the input file name
  $self->{output_filename}= undef;   # the output file defaults to STDOUT
  $self->{output_file}    = *STDOUT; # the output file defaults to STDOUT
  $self->{filter_proc}    = sub {1;};# filter subroutine

  # parser state variables
  $self->{contents}       = '';      # current text found in between tag elements
  $self->{char_stack}     = [];      # stack of current text values
  $self->{name_stack}     = [];      # stack of enclosing compound names
  $self->{in_scalar}      = 0;       # are we currently parsing a scalar?
  $self->{in_vector}      = 0;       # are we currently parsing a vector?
  $self->{in_histogram}   = 0;       # histogram parsing state
  $self->{header_printed} = 0;       # have we printed the header info yet?
  $self->{vector}         = [];      # storage for vector data
  $self->{properties}     = {};      # hash containing compound header info (type, name)
  $self->{histogram}      = {};      # hash containing histogram info
}

=head1 PUBLIC API METHODS

The following public API methods are provided by this class:

=over 4

=item $stat2txt = Asim::Stats::ToText::new( $input_filename );

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


=item $this->transcribe( $outfile );

Transcribe the input file to an output file.
This basically does an XML-to-plain text conversion.

If $outfile is omitted, or given as 'stdout' or 'STDOUT',
then output is sent to the standard output.

=cut

sub transcribe {
  my $self = shift;
  
  # open the output file, if any
  $self->{output_filename} = shift;
  if ( $self->{output_filename} &&
       $self->{output_filename} !~ m/^stdout$/i ) {
    open( $self->{output_file}, '>', $self->{output_filename} ) || die $!;
  }

  # create a SAX parsrer, using this object as the handler
  # to receive the start_element(), end_element(), etc. callbacks.
  my $parser  = XML::SAX::ParserFactory->parser( Handler => $self );
  
  # invoke the SAX parse method to parse the file and transcribe to output
  $parser->parse_uri( $self->{input_filename} );
}



=back

=head1 SUBCLASS CUSTOMIZABLE METHODS

The following methods are also provided by this base class.
These are not normally called by the user, but are called internally by the parser.
Derived classes may override the following methods to customize the output format:

=over 4


=item $this->print_scalar()

Print scalar info.
The following information is available to this routine to print:

=over 4

=item $this->{properties}->{type}

The data type of the scalar, e.g. "uint"

=item $this->{properties}->{name}

The name of the scalar item

=item $this->{properties}->{desc}

A description of the item

=item $this->{contents}

The value of this scalar

=back

=cut

sub print_scalar {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} "%-6s %-45s %20s",
          $self->{properties}->{type},
	  $self->{properties}->{name},
	  $self->{contents};
  if ( $self->{properties}->{desc} ) {
    print {$self->{output_file}} " # ", $self->{properties}->{desc};
  }
  print {$self->{output_file}} "\n";
}


=item $this->print_vector()

Print vector info.
The following information is available to this routine to print:

=over 4

=item $this->{properties}->{type}

the data type of the scalar, e.g. "uint"

=item $this->{properties}->{name}

the name of the scalar item

=item $this->{vector}

The vector data.
This is a reference to an array of values.

=back

=cut

sub print_vector {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} "%-6s %s :",
          $self->{properties}->{type},
	  $self->{properties}->{name};
  foreach my $value ( @{$self->{vector}} ) {
    print {$self->{output_file}} " ", $value;
  }
  print {$self->{output_file}} "\n";
}


=item $this->print_header()

Print the header for a compound object like a module or histogram.
This routine accesses the type, name, and description of the scalar
using the same fields as print_scalar(), i.e.
using $this->{properties}->{type or name or desc}.

=cut

sub print_header {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} "%-6s %-45s",
          $self->{properties}->{type},
	  $self->{properties}->{name};
  if ( $self->{properties}->{desc} ) {
    print  {$self->{output_file}}"                   # ",
          $self->{properties}->{desc};
  }
  print {$self->{output_file}} "\n";
}


=item $this->print_histogram_header()

Print the header for a histogram.
The following information is available to this routine to print:

=over 4

=item $this->{histogram}->{name}

The name of the histogram

=item $this->{histogram}->{desc}

A description of the enclosed data

=item $this->{histogram}->{rows}

The number of rows in the histogram data

=item $this->{histogram}->{cols}

The number of columns in the histogram data

=item $this->{histogram}->{entries}

The total number of data counts used the generate the data

=item $this->{histogram}->{column names}

A (reference to) an array of names, one for each column of the data.

=back

=cut

sub print_histogram_header {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} 
        "HISTOGRAM %-20s  rows: %-6d cols: %-3d count: %-10d # %s\n",
	$self->{histogram}->{name},
	$self->{histogram}->{rows},
	$self->{histogram}->{cols},
	$self->{histogram}->{entries},
	$self->{histogram}->{desc};
  $self->indentation();
  print {$self->{output_file}} "#            :";
  foreach my $cname ( @{$self->{histogram}->{column_names}} ) {
    printf {$self->{output_file}} " %20s", $cname;
  }
  print {$self->{output_file}} "\n";
}


=item $this->print_histogram_row()

Print a row of data in a histogram.
The following information is available to this routine to print:

=over 4

=item $this->{properties}->{name}

A label for this row of the data

=item $this->{vector}

A (ref to) an array of values, i.e. one row of data.

=back


=cut

sub print_histogram_row {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} "%-12s :", $self->{properties}->{name};
  foreach my $value ( @{$self->{vector}} ) {
    printf {$self->{output_file}} " %20s", $value;
  }
  print {$self->{output_file}} "\n";
}


=item $this->print_footer()

Print the footer for a compound object such as a module.
This routine accesses the name of the compound item
using $this->{properties}->{name}.

=cut

sub print_footer {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} "END    %s\n",
	  $self->{properties}->{name};
}


=item $this->print_histogram_footer()

Print the footer for a histogram.
This routine accesses the name (etc.) of the histogram
using $this->{histogram}->{name}.

=cut

sub print_histogram_footer {
  my $self = shift;
  $self->indentation();
  printf {$self->{output_file}} "END       %s\n",
	  $self->{histogram}->{name};
}


=item $this->indentation()

Print the current indentation level,
e.g. one space character for each nesting level.
This is an optional routine.  It is not called automatically by the parser,
but is called by the print_*() routines implemented in this package.

=cut

sub indentation {
  my $self = shift;
  for ( my $i=0;
        $i<$#{$self->{name_stack}};
	$i++
  ) {
    print {$self->{output_file}} " ";
  }
}



=back

=head1 FILTER ROUTINES

The data member $this->{filter_proc} can be set to refer to a subroutine
that is called just before each of the print_*() methods.  If the filter
routine returns 0, the item is not printed.

The filter routine is passed a reference to this parser object.
It is typically written something like the following:

    sub my_filter {
      my $this = shift;
      # code to return 1 if we should print this item...
    }

The following fields are typically used to determine what to print:

=over 4

=item $this->{properties}->{name}

The name of a scalar or non-histogram item.

=item $this->{histogram}->{name}

The name of a histogram item.

=item $this->{in_histogram}

If nonzero, the item to print is the header or a portion of the data of a histogram

=item $this->{name_stack}

A list of enclosing module names, starting with the outermost.

=back



=head1 PRIVATE PARSING METHODS

The following private methods are also implemented in this base class.
These are called internally by the XML::SAX parser,
and should not be overridden by base classes,
unless the XML stats format changes:

=over 4

=cut


=item $self->start_element( $element )

We get called here after seeing the start of an element.

=cut

sub start_element {
  my ($self, $el) = @_;
  
  # if we're parsing a new scalar, vector, or compound item,
  # maybe print a header for the enclosing compound.
  # Also, save the name of this enclosing item onto the stack:
  if ( $el->{LocalName} eq 'scalar'   ||
       $el->{LocalName} eq 'vector'   ||
       $el->{LocalName} eq 'compound' ) {
    $self->handle_header( );
    if ( $self->{properties}->{name} &&
         $self->{in_histogram} == 0  ) {
      push @{$self->{name_stack}}, $self->{properties}->{name};
      $self->{properties}->{name} = undef;
    }
  }

  # save the text for the current element, and start new text:
  push @{$self->{char_stack}}, $self->{contents};
  $self->{contents} = '';

  # set flag if we started parsing a scalar or a vector
  if      ( $el->{LocalName} eq 'scalar' ) {
    $self->{in_scalar} = 1;
  } elsif ( $el->{LocalName} eq 'vector' ) {
    $self->{in_vector} = 1;
    
  # if we started parsing a new compound, clear printed-header flag
  } elsif ( $el->{LocalName} eq 'compound' ) {
    $self->{header_printed} = 0;
  }
}


=item $self->end_element( $element )

We get called here after seeing the end of an element.

=cut

sub end_element {
  my ($self, $el) = @_;

  if      ( $el->{LocalName} eq 'scalar'   ) {
    $self->end_scalar();

  } elsif ( $el->{LocalName} eq 'vector'   ) {
    $self->end_vector();

  } elsif ( $el->{LocalName} eq 'compound' ) {
    $self->end_compound();

  # if we're done parsing a vector element, add it to vector data
  } elsif ( $el->{LocalName} eq 'value'    ) {
    push @{$self->{vector}}, $self->{contents};
  
  # if this is not a scalar, vector, compound, or vector element,
  # save this as header meta-data
  } else {
    $self->{properties}->{$el->{LocalName}} = $self->{contents};
  }

  # restore the name of the enclosing item from the stack:
  if ( $el->{LocalName} eq 'scalar'   ||
       $el->{LocalName} eq 'vector'   ||
       $el->{LocalName} eq 'compound' ) {
    if ( $self->{in_histogram} == 0  ) {
      $self->{properties}->{name} = pop @{$self->{name_stack}};
    }
  }

  # restore text for enclosing element
  $self->{contents} = pop @{$self->{char_stack}};
}



sub end_scalar() {
  my $self = shift;
  # if we're done parsing a scalar, clear flag and print info
  $self->handle_scalar();
  $self->{in_scalar}  = 0;
  $self->{properties} = {};
}

sub end_vector() {
  my $self = shift;
  # if we're done parsing a vector, clear flag,
  # print the vector, and reinitialize the data array
  $self->handle_vector();
  $self->{in_vector}  = 0;
  $self->{properties} = {};
  $self->{vector}     = [];
}

sub end_compound() {
  my $self = shift;
  # if we're done parsing a compound, then
  # if you never printed a header for it, print it now.
  # Also, print the footer, and clear the properties hash.
  $self->handle_header( );
  $self->handle_footer( );
  $self->{properties} = {};
}


=item $this->handle_header()

Once we have seen enough of a compound item, we get called here.
We might get called multiple times for each compound item.
Ordinarily we would print the header, but if we are parsing a
histogram, we might need to gather more header data.

=cut

# called from start_element AND end_compound
sub handle_header() {
  my $self = shift;
  # if we have gathered enough information (type, name, ...)
  # and we haven't printed a header yet, print it now
  if ( $self->{properties}->{type} &&
       $self->{properties}->{name} ) {
    if ( ! $self->{header_printed} ) {

      if      ( $self->{in_histogram} == 0
             && $self->{properties}->{type} eq 'histogram' ) {
        # if we just entered a histogram, flag it
	$self->{in_histogram} = 1;
	$self->{histogram}->{name} = $self->{properties}->{name};
	$self->{histogram}->{desc} = $self->{properties}->{desc};

      } elsif ( $self->{in_histogram} == 1
             && $self->{properties}->{type} eq 'info'
             && $self->{properties}->{name} eq 'data' ) {
        # if we just entered the histogram data section,
	# set flag and print histogram header
	$self->{in_histogram} = 2;
	if ( &{$self->{filter_proc}}( $self ) ) {
	  $self->print_histogram_header( );
	}

      } elsif ( $self->{in_histogram} == 0 ) {
        # but if we're not in a histogram, just print the header:
	if ( &{$self->{filter_proc}}( $self ) ) {
	  $self->print_header( );
	}
      }

      # set flag so we don't get called here again
      $self->{header_printed} = 1;

    }
  }
}

# called from end_compound
sub handle_footer() {
  my $self = shift;
  if ( $self->{in_histogram} ) {
    # if you're parsing a histogram, pop out of it
    $self->{in_histogram}--;
    if ( $self->{in_histogram} ) {
      if ( &{$self->{filter_proc}}( $self ) ) {
	$self->print_histogram_footer( );
      }
    }

  } else {
    # otherwise, print the footer
    if ( $self->{properties}->{name} ) {
      $self->print_footer( );
    }

  }
}

# called from end_vector
sub handle_vector() {
  my $self = shift;
  if ( $self->{in_histogram} ) {
    # if we're parsing a histogram...
    if      ( $self->{properties}->{type} eq 'info'         &&
              $self->{properties}->{name} eq 'column names' ) {
      # ...and this is the vector of column names,
      # add these column names to the "histogram" info
      $self->{histogram}->{column_names} = $self->{vector};

    } elsif ( $self->{properties}->{type} eq 'row'          ) {
      # ...and this is a data row vector, print it:
      if ( &{$self->{filter_proc}}( $self ) ) {
	$self->print_histogram_row();
      }
    }
  
  } else {
    # otherwise, just print the vector
    if ( &{$self->{filter_proc}}( $self ) ) {
      $self->print_vector( );
    }
  }
}

# called from end_scalar
sub handle_scalar() {
  my $self = shift;
  if ( $self->{in_histogram} ) {
    # if you're parsing a histogram,
    # add scalar values to histogram header info
    my $key   = $self->{properties}->{name};
    my $value = $self->{contents};
    $self->{histogram}->{$key} = $value;
  
  } else {
    # otherwise, just print the scalar
    if ( &{$self->{filter_proc}}( $self ) ) {
      $self->print_scalar( );
    }
  }
}


=item $self->characters( $text )

We get called here after seeing raw text in the file.
Add to the character buffer.

=cut

sub characters {
  my ($self, $chars) = @_;
  my $c = $chars->{Data};
  if ( $c !~ m/^\s*$/ ) {
    $self->{contents} .= $c;
  }
}

1;



=back

=head1 BUGS

There are no callbacks yet for the start and end
of the document.  These would be useful for printing
document headers and footers in the output.

Maybe I should provide a generic configure() method
that would be called like:

$this->configure( name1 => value1, name2 => value2, ... )

that you could use to configure input files, output files,
filter routines, and any future configuration variables
we might think of.

=head1 AUTHOR

Carl J. Beckmann

=head1 COPYRIGHT

Copyright 2007 Intel Corporation All Rights Reserved.

=cut
