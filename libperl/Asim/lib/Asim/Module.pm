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

package Asim::Module;
use warnings;
use strict;

use File::Basename;
use Getopt::Long;

use Asim::Base;
use Asim::Module::Attribute;
use Asim::Module::Param;
use Asim::Util;
use Asim::Module::SourceList;
use Asim::Module::Source;

our @ISA = qw(Asim::Base);

our %a =  ( name =>                 [ "name",
                                      "SCALAR" ], 
            description =>          [ "description",
                                      "SCALAR" ],
            attributes =>           [ "attributes",
                                      "ARRAY" ],
            provides =>             [ "provides",
                                      "SCALAR" ],
            template =>             [ "template",
                                      "SCALAR" ],
            parent =>               [ "parent",
                                      "SCALAR" ],
            requires =>             [ "requires",
                                      "ARRAY" ],
            notes =>                [ "notes",
                                      "ARRAY" ],
            public =>               [ "public",
                                      "ARRAY" ],
            private =>              [ "private",
                                      "ARRAY" ],
            sourcematrix =>         [ "sourcematrix",
                                      "ARRAY",
                                      "Asim::Module::SourceList" ],
            generatedmatrix =>      [ "generatedmatrix",
                                      "ARRAY",
                                      "Asim::Module::SourceList" ],
            makefile =>             [ "makefile",
                                      "ARRAY" ],
            scons =>                [ "scons",
                                      "ARRAY" ],
            parameters =>           [ "parameters",
                                      "ARRAY",
                                      "Asim::Module::Param" ],
            default_attributes =>   [ "default_attributes",
                                      "ARRAY" ],
            mandatory_attributes => [ "mandatory_attributes",
                                      "ARRAY" ],
            library =>              [ "library",
                                      "ARRAY" ],
            syslibrary =>           [ "syslibrary",
                                      "ARRAY" ],
            include =>              [ "include",
                                      "ARRAY" ]
          );

our $debug = 0;

=head1 NAME

Asim::Module - Library for manipulating ASIM modules

=head1 SYNOPSIS

use Asim::Module;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################

=item $module = Asim::Module-E<gt>new([$file])

Create a new module object, optionally reading awb configuration
file $file to populate the object.

=cut

################################################################

################################################################
# WARNING: 
#    The charactistics (especially file parsing) of this
#    object must stay in sync with the tcl file module.tcl.
#
################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = _initialize();

  bless       $self, $class;

  #
  # Parse file if given
  #
  if (@_) {
      $self->open($_[0]) || return undef;
  }

  return $self;
}

sub _initialize {

  my $self = {   accessors => \%a,
		 name => "" ,
		 description => "",
		 provides => "",
		 template => 0,
		 parent => undef,
		 requires => [],
		 submodules => [],
		 notes => [],
		 public => [],
		 private => [],
		 sourcematrix => [],
		 generatedmatrix => [],
		 makefile => [],
		 scons => {},
		 attributes => [],
		 default_attributes => [],
		 mandatory_attributes => [],
		 params => [],
                 isroot => 0,
                 owner => [],
		 library => [],
		 syslibrary => [],
		 include => {},
		 };

  return $self;
}

################################################################

=item $module-E<gt>open($file)

Open and read the module specification (.awb) file $file, 
and fill in the attributes of the module object.
Returns module.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift
      || $self->ierror("No module file name specified\n") && return undef;

  print "Asim::Module - opening module - $file\n" if $debug;

  #
  # Cannonicalize file name and open
  #
  my $rfile = $Asim::default_workspace->resolve($file) 
    || return undef;

  CORE::open(M, "< $rfile") 
    || $self->ierror("Could not open module file ($file)\n") && return undef;

  $self->{filename} = $file;

  my $space = "[ \t]";
  my $spaces = "$space+";
  my $nospace = "[^ \t]";
  my $equal = "=";
  my $word = "$nospace+";
  my $words = "$nospace.+$nospace";

  while (<M>) {
    chop;

    # %name MODULENAME

    if (/^.*%name$spaces($words)/) {
      #
      # Record name of module
      #
      $self->{name} = $1;

      next;
    }

    # %desc DESCRIPTIONPHRASE

    if (/^.*%desc$spaces($words)/) {
      #
      # Record module description
      #
      $self->{description} = $1;
      next;
    }

    # %provides ASIMMODULETYPE

    if (/^.*%provides$spaces(.*$nospace)/) {
        my $template = 0;

        # parse switches
        local @ARGV = split($spaces, $1);
        GetOptions("template" => \$template);

        $self->{provides} = $ARGV[0];
	$self->{template} = $template;
        next;
    }

    # %requires ASIMMODULETYPE...

    if (/^.*%requires$spaces($words)/) {
      #
      # Add requires to list of requires
      #
      foreach my $r (split($spaces, $1)) {
        push(@{$self->{requires}}, $r);
        push(@{$self->{submodules}}, undef);
      }
      next;
    }

    # %attributes ATTRIBUTE...

    if (/^.*%attributes$spaces($words)/) {
      #
      # Add attributes to list of attributes
      #
      foreach my $a (split($spaces, $1)) {
	push(@{$self->{attributes}}, Asim::Module::Attribute->new($a));
      }
      next;
    }

    # %default_attributes ATTRIBUTE...

    if (/^.*%default_attributes$spaces($words)/) {
      #
      # Add default attributes to list of default attributes
      #
      push(@{$self->{default_attributes}}, split($spaces, $1));
      next;
    }

    # %mandatory_attributes ATTRIBUTE...

    if (/^.*%mandatory_attributes$spaces($words)/) {
      #
      # Add mandatory attributes to list of mandartory attributes
      #
      push(@{$self->{mandatory_attributes}}, split($spaces, $1));
      next;
    }

    # %notes FILENAME...

    if (/^.*%notes$spaces($words)/) {
      #
      # Add files to list of notes files
      #
      push(@{$self->{notes}}, split($spaces, $1));
      next;
    }

    # %scons [%CLASS] FILENAME...
    # scons is more specific than %private, and so should go first

    if (/^.*%scons\s*(%[^ \t]*)?\s+($word)/) {
      #
      # Add scons config file to specified category.
      #
      my $class = $1;
      my $fName = $2;

      if (! defined($class)) {
        $class = 'top';
      } 
      # this is getting ugly probably we want to refactor soon
      elsif ($class =~ /^%(iface|hw|sw|main|top|library|private)$/) {
        $class =~ s/^%//;
      }
      else {
        $self->ierror("Illegal category for %scons: $class\n");
        return undef;
      }

      push(@{$self->{scons}->{$class}}, split($spaces, $fName));
      next;
    }

    # %public FILENAME... backward compatibility

    if (/^.*%public$spaces($words)/) {
        #
        # Add files to list of public files
        #
        my @files = split($spaces, $1);
        $self->public(@files);
        next;
    }

    # %private FILENAME...

    if (/^.*%private$spaces($words)/) {
        #
        # Add files to list of private files
        #
        my @files = split($spaces, $1);
        $self->private(@files);
        next;
    }

    # %sources --type TYPE --visibility PRIVATE/PUBLIC --attribute foo=bar FILENAME...
    # %sources -t TYPE -v  PRIVATE|PUBLIC -a foo=bar FILENAME...

    if (/^.*%sources$spaces($words)/)
    {
        my $type = "";
        my $vis = "";
        my %attributes = ();

        # parse switches
        local @ARGV = split($spaces, $1);
        GetOptions("type=s" => \$type,
                   "visibility=s" => \$vis,
                   "attribute=s" => sub {  
                                            my @attributeStatement = split($equal, $_[1]);
                                            if (scalar (@attributeStatement) == 2) {
                                                $attributes{$attributeStatement[0]} = $attributeStatement[1]; 
                                            } else {
                                                die("Asim::Module::open: Attribute argument was not parsed.");
                                            }
                                        }, 
                   );

        $self->addsources($type, $vis, 1, \%attributes, @ARGV);
        next;
    }

    # The following permits users to list generated sources, which we can't 
    # easily determine (i.e. Bluespec BA files), which should be included 
    # in certain build steps.  This code is admittedly ugly, and it's no 
    # more that a momentary work around. 
    # %generated --type TYPE FILENAME...
    # %generated -t TYPE FILENAME...
    if (/^.*%generated$spaces($words)/)
    {
        my $type = "";
        my $vis = "";

        # parse switches
        local @ARGV = split($spaces, $1);
        GetOptions("type=s" => \$type);

        # insert into source matrix
        $self->addgenerated($type, @ARGV);
        next;
    }

    # %makefile FILENAME...

    if (/^.*%makefile$spaces($words)/) {
      #
      # Add files to list of makefiles
      #
      push(@{$self->{makefile}}, split($spaces, $1));
      next;
    }

    # %param [--dynamic] [--global] NAME DEFAULT "DESCRIPTION"
    # %export [--dynamic] [--global]  NAME DEFAULT "DESCRIPTION"
    # %const [--dynamic] [--global] NAME DEFAULT "DESCRIPTION"

    my $p_cmd      = '%param|%export|%const';
    my $p_switches = '(%dynamic\s+|--dynamic\s+)?(%global\s+|--global\s+)?';
    my $p_name     = '\w+';
    my $p_string   = '\"[^"]*\"';
    my $p_number   = '(-?\d+(\.\d+)?|0x[0-9A-Fa-f]+)[Uu]?[Ll]?[Ll]?';
    my $p_default  = '\'?\w+';
    my $p_desc     = '[^"]*';

    if (/^.*(${p_cmd})\s+${p_switches}\s*(${p_name})\s+(${p_string})\s+\"(${p_desc})\"/) {
      #
      # Add string parameter
      #
      my $param = Asim::Module::Param->new( type => $1,
                                            isString => 1,
                                            dynamic => ((defined $2) ? 1 : 0),
                                            global => ((defined $3) ? 1 : 0),
                                            name => $4,
                                            default => "$5",
                                            description => "$6");

      push(@{$self->{params}}, $param);
      next;
    } elsif (/^.*(${p_cmd})\s+${p_switches}\s*(${p_name})\s+(${p_number})\s+\"(${p_desc})\"/) {
      #
      # Add numeric parameter
      #
      my $param = Asim::Module::Param->new( type => $1,
                                            isInt => 1,
                                            dynamic => ((defined $2) ? 1 : 0),
                                            global => ((defined $3) ? 1 : 0),
                                            name => $4,
                                            default => "$5",
                                            description => "$8");

      push(@{$self->{params}}, $param);
      next;
    } elsif (/^.*(${p_cmd})\s+${p_switches}\s*(${p_name})\s+(${p_default})\s+\"(${p_desc})\"/) {
      #
      # Add unknown type parameter
      #
      my $param = Asim::Module::Param->new( type => $1,
                                            dynamic => ((defined $2) ? 1 : 0),
                                            global => ((defined $3) ? 1 : 0),
                                            name => $4,
                                            default => "$5",
                                            description => "$6");

      push(@{$self->{params}}, $param);
      next;
    }

    # %library FILENAME

    if (/^.*%library$spaces($word)/) {
      #
      # Add file to list of library files
      #
      push(@{$self->{library}}, $1);
      next;
    }

    # %syslibrary CPPARGS...

    if (/^.*%syslibrary$spaces($words)/) {
      #
      # Add compiler library args to the list of syslibs
      #
      push(@{$self->{syslibrary}}, split($spaces, $1));
      next;
    }

    # %include [--type=<type>] DIRNAME...
    # where, <type>:=sw|verilog

    if (/^.*%include$spaces($words)/)
    {
        my $type = "sw";

        # parse switches
        local @ARGV = split($spaces, $1);
        GetOptions("type=s" => \$type);

	push(@{$self->{include}->{$type}}, @ARGV);
        next;
    }
  }

  CORE::close(M);

  # At minimum we require a name and provides

  if ($self->{name} eq "" || $self->{provides} eq "") {
    return undef;
  }

  return $self;
}

################################################################

=item $module-E<gt>save([$file])

Save the module specification into (.awb) the file $file, 
$file defaults to the original filename.

=cut

################################################################

sub save {
  my $self = shift;
  my $file = shift
    || $self->{filename} 
  || return 0;

  CORE::open(M, "> $file") || return 0;

  my $name = $self->{m};

  print M "\n";
  print M "/********************************************************************\n";
  print M " *\n";
  print M " * Awb module specification\n";
  print M " *\n";
  print M " *******************************************************************/\n";
  print M "\n";
  print M "/*\n";
  print M " * %AWB_START\n";
  print M " *\n";
  print M " * %name " . $self->name() . "\n";

  foreach my $i ($self->attributes()) {
    print M " * %attributes " . $i->fullname() . "\n";
  }

  print M " * %desc " . $self->description() . "\n";
  print M " * %provides " . $self->template()?"--template ":"" 
                          . $self->provides() . "\n";

  foreach my $i ($self->requires()) {
    print M " * %requires $i\n";
  }

  foreach my $i ($self->notes()) {
    print M " * %notes $i\n";
  }

  # foreach my $i ($self->public()) {
  #   print M " * %public $i\n";
  # }

  # foreach my $i ($self->private()) {
  #   print M " * %private $i\n";
  # }

  my @smat = @{$self->{sourcematrix}};
  foreach my $slist (@smat)
  {
      my $type  = $slist->type();
      my $vis   = $slist->visibility();
      my @fobjs = $slist->files();

      # Pull files out of fobjs. We should handle attributes syntax as
      # well.  We assume that all the files in the fobj have the same
      # attributes.
      for my $fobj (@fobjs) {

          my $attributes = "";
          my @flist = $fobj->files();
 
          while( my ($attributeKey, $attributeValue) = each %$fobj->getAllAttributes() ) {
              $attributes .= " --attribute $attributeKey=$attributeValue "
          }
                        
          print M " * %sources -t $type -v $vis $attributes @flist\n";
      }

  }

  foreach my $i ($self->makefile()) {
    print M " * %makefile $i\n";
  }

  foreach my $k (keys %{$self->{scons}}) {
    foreach my $i ($self->{scons}->{$k}) {
      print M " * %scons %$k $i\n";
    }
  }

  foreach my $i ($self->library()) {
    print M " * %library $i\n";
  }

  print M " * %syslibrary";
  foreach my $i ($self->syslibrary()) {
    print M " $i";
  }
  print M "\n";

  foreach my $k (keys %{$self->{include}}) {
    foreach my $i ($self->{include}->{$k}) {
      print M " * %include --type=%$k $i\n";
    }
  }

  # Save parameters too.

  print M " *\n";
  print M " * %AWB_END\n";
  print M " */\n";

  CORE::close(M);

  $self->{filename} = $file;

  return 1;
}

#
# Simple accessor functions
#


################################################################

=item $module-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}
################################################################

=item $module-E<gt>base_dir()

Directory containing module. 
Use this to prefix info from private() and public().

=cut

################################################################

sub base_dir {
  my $self = shift;

  return dirname($self->{filename});
}

################################################################

=item $module-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    my $self = shift;

    return qw(name
	    description
	    attributes
	    provides
            template
	    parent
	    requires
            notes
	    public
	    private
	    makefile
	    scons
	    parameters
	    default_attributes
	    mandatory_attributes
	    library
	    syslibrary
	    include
	    );
}


################################################################

=item $module-E<gt>name([$value])

Optionally update name of module to $value.
Return the current (updated) name of module.

=cut

################################################################


# Implemented in Asim::Base


################################################################

=item $module-E<gt>description([$value])

Optionally update name of module to $value.
Return the current (updated) description of module.

=cut

################################################################

# Implemented in Asim::Base

################################################################

=item $module-E<gt>provides([$value])

Optionally update ASIM module type this module provides to $value.
Return the current (updated) ASIM module type this module provides.


=cut

################################################################


sub provides {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"provides"} = $value;
  }

  return $self->{"provides"};
}

################################################################

=item $module-E<gt>template([$value])

Optionally update the flag indicating that this module is a template 
with $value.
Return the current (updated) ASIM module type this module provides.


=cut

################################################################


sub template {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"template"} = $value;
  }

  return $self->{"template"};
}

################################################################

=item $module-E<gt>parent([$value])

Optionally update ASIM module parent to $value. Return the parent.

=cut

################################################################


sub parent {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"parent"} = $value;
  }

  return $self->{"parent"};
}

################################################################

=item $module-E<gt>requires([$list])

Optionally update the list of ASIM module types required by this
module to $list. 
Return the current (updated) list of ASIM module types requried 
by this module.


=cut

################################################################


sub requires {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"requires"}} = (@value);
  }

  return @{$self->{"requires"}};
}

################################################################

=item $module-E<gt>submodules()

Return list of submodules of this module.

=cut

################################################################


sub submodules {
  my $self = shift;
  my @value = (@_);

  # We're assuming the modules are in the right order!

  if (@value) {
    @{$self->{"submodules"}} = (@value);
  }

  return @{$self->{"submodules"}};
}

################################################################

=item $module-E<gt>add_submodule($module)

Add submodule $module to list right spot in the list submodule
of this module.

=cut

################################################################


sub add_submodule {
  my $self = shift;
  my $mod = shift;
  my $provides = $mod->provides();

  my @requires = ($self->requires());

  foreach my $index (0 .. $#requires) {
    if ($requires[$index] eq $provides) {
      $self->{"submodules"}[$index] = $mod;
      last;
    }
  }

  $mod->parent($self);

  return @{$self->{"submodules"}};
}


################################################################

=item $module-E<gt>remove_submodule($module)

Remove submodule $module from the submodule list of this module

=cut

################################################################


sub remove_submodule {
  my $self = shift;
  my $mod = shift;
  my $provides = $mod->provides();

  my @requires = ($self->requires());

  foreach my $index (0 .. $#requires) {
    if ($requires[$index] eq $provides) {
      $self->{"submodules"}[$index] = undef;
      last;
    }
  }

  $mod->parent(undef);

  return @{$self->{"submodules"}};
}

################################################################

=item $module-E<gt>inventory([--editable-only => 0|1])

Return the list of all source files associated with the module

Notes: 
    1) All files are relative to the file including the .awb file
    2) The .awb file itself is not included.
    3) Files from %library and %include directives are not included

=cut

################################################################

sub inventory {
  my $self = shift;

  my @files = ();

  push(@files, $self->notes());
  push(@files, $self->sources("*", "*"));
  push(@files, $self->makefile());
  push(@files, $self->scons("*"));

  return (@files);
}

################################################################

=item $module-E<gt>notes([$list])

Optionally update the list of notes for this module to $list

Return the current (updated) list of notes files for this module

Note: All filenames are relative to the directory containing the awbfile...

=cut

################################################################

sub notes {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"notes"}} = (@value);
  }

  return @{$self->{"notes"}};
}


################################################################

=item $module-E<gt>public([$list])

Optionally update the list of public files that implement this 
module to $list.

Return the current (updated) list of public files that implement
this module.

Note: All public filenames are relative to the directory containing
the awbfile...

=cut

################################################################

sub public
{
    # capture params
    my $self = shift;
    my @infiles = @_;

    # do we have input files to update?
    if (@infiles)
    {
        foreach my $f (@infiles)
        {
            # figure out type of file
            my $type = decipher_type_from_extension($f);

            # update source matrix
            #
            # NOTE: slight backwards-incompatibility: the original
            # %public directive would *replace* the existing public
            # file list with the new list. We will instead *add*
            # this file to the list of existing files of this type.
            $self->addsources($type, "PUBLIC", 0, {}, ($f));
        }
    }

    # get (possibly updated) list of PUBLIC files of all types
    my @outfiles = $self->sources("*", "PUBLIC");
    return @outfiles;
}

################################################################

=item $module-E<gt>private([$list])

Optionally update the list of private files that implement this 
module to $list.

Return the current (updated) list of private files that implement
this module.

Note: All private filenames are relative to the directory containing
the awbfile...

=cut

################################################################

sub private
{
    # capture params
    my $self = shift;
    my @infiles = @_;

    # do we have input files to update?
    if (@infiles)
    {
        foreach my $f (@infiles)
        {
            # figure out type of file
            my $type = decipher_type_from_extension($f);

            # update source matrix
            #
            # NOTE: slight backwards-incompatibility: the original
            # %private directive would *replace* the existing private
            # file list with the new list. We will instead *add*
            # this file to the list of existing files of this type.
            $self->addsources($type, "PRIVATE", 0, {}, ($f));
        }
    }

    # get (possibly updated) list of PRIVATE files of all types
    my @outfiles = $self->sources("*", "PRIVATE");
    return @outfiles;
}

################################################################

=item $module-E<gt>generated($t, [$list])

Return the current list of generated files of specified type. A
wildcard "*" can be used for type parameters.

Note: All filenames are relative to the directory
containing the awbfile...

=cut

################################################################

sub generated
{
    # capture params
    my $self = shift;
    my $type = shift;

    return $self->getfilelist($type, "PUBLIC", $self->{generatedmatrix});
}

################################################################

=item $module-E<gt>generated($t, [$list])

Return the current list of generated file objects of specified type. A
wildcard "*" can be used for type parameters.

Note: All filenames are relative to the directory
containing the awbfile...

=cut

################################################################

sub generatedobjects
{
    # capture params
    my $self = shift;
    my $type = shift;

    return $self->getfileobjects($type, "PUBLIC", $self->{generatedmatrix});
}


################################################################

=item $module-E<gt>generatedtypes($t, [$list])

Return all categories of generated files available.

=cut

################################################################

sub generatedtypes
{
    # capture params
    my $self = shift;

    return $self->getfiletypes($self->{generatedmatrix});
}


################################################################

=item $module-E<gt>sourcetypes($t, [$list])

Return all categories of generated files available.

=cut

################################################################

sub sourcetypes
{
    # capture params
    my $self = shift;

    return $self->getfiletypes($self->{sourcematrix});
}


################################################################

=item $module-E<gt>sources($t, $v, [$list])

Return the current list of source files of specified
type and visibility. A wildcard "*" can be used for both type
and visibility parameters.

Note: All source filenames are relative to the directory
containing the awbfile...

=cut

################################################################

sub sources
{
    # capture params
    my $self = shift;
    my $type = shift;
    my $vis  = shift;

    return $self->getfilelist($type, $vis,$self->{sourcematrix});
}

################################################################

=item $module-E<gt>sourceobjectss($t, $v, [$list])

Return the current list of source objects of specified
type and visibility. A wildcard "*" can be used for both type
and visibility parameters.

Note: All source filenames are relative to the directory
containing the awbfile...

=cut

################################################################

sub sourceobjects
{
    # capture params
    my $self = shift;
    my $type = shift;
    my $vis  = shift;

    return $self->getfileobjects($type, $vis,$self->{sourcematrix});
}

################################################################

=item $module-E<gt>getfilelist($t, $v, $matrix, [$list])

Return the current list of source files of specified type and
visibility, from the specified matrix. A wildcard "*" can be used for
both type and visibility parameters.

=cut

################################################################
sub getfilelist
{
    # capture params
    my $self = shift;
    my $type = shift;
    my $vis  = shift;
    my $matrixReference = shift;

    # list of files I'll send out
    my @outfiles = ();

    my @fobjs =  $self->getfileobjects($type, $vis, $matrixReference);

    # Pull files out of fobjs. 
    for my $fobj (@fobjs) {
        my @files = $fobj->files();            
        push(@outfiles, $fobj->files());
    }


    # return
    return @outfiles;
}

################################################################

=item $module-E<gt>getfileobjects($t, $v, $matrix, [$list])

Return the current list of source files objects of specified type and
visibility, from the specified matrix. A wildcard "*" can be used for
both type and visibility parameters.

=cut

################################################################
sub getfileobjects
{
    # capture params
    my $self = shift;
    my $type = shift;
    my $vis  = shift;
    my $matrixReference = shift;

    # list of files I'll send out
    my @outobjs = ();

    # scan through source matrix
    my @smat = @{$matrixReference};
    foreach my $slist (@smat)
    {
        # check for type and visibility match
        if ((($type eq "*") || ($slist->type() eq $type)) &&
            (($vis  eq "*") || ($slist->visibility() eq $vis)))
        {           
            push(@outobjs, $slist->files());
        }
    }

    # return
    return @outobjs;
}

################################################################

=item $module-E<gt>getfiletypes($t, $v, $matrix, [$list])

Return the types of the files in the filelist

=cut

################################################################
sub getfiletypes
{
    # capture params
    my $self = shift;
    my $matrixReference = shift;

    my %ftypes = ();

    # scan through source matrix
    my @smat = @{$matrixReference};
    foreach my $slist (@smat)
    {
        # Use hash to remove duplicates
        $ftypes{$slist->type()}=0; 
    }

    # return
    return keys %ftypes;
}

################################################################

=item $module-E<gt>source_types()

Return a list of all source types defined for the module.

=cut

################################################################

sub source_types
{
    # capture params
    my $self = shift;

    my %types;

    # scan through source matrix
    my @smat = @{$self->{sourcematrix}};
    foreach my $slist (@smat)
    {
        $types{$slist->type()} = 1;
    }

    # Construct an array of unique types
    my @outtypes = ();
    foreach my $type (keys %types) {
        push(@outtypes, $type);
    }

    return @outtypes;
}


################################################################

=item $module-E<gt>addgenerated($t, $requireType, [$list])

Update the list of generated files of specified type with $list.  If
the type combination is not found in the matrix of generated files,
then create a new row for this combination. Wildcards are not allowed;
type and visibility have to be explicit.

Note: Generated files cannot have relative/absolute paths.  

=cut

################################################################
sub addgenerated
{
    # capture params
    my $self = shift;
    my $specType = shift;
    my @infiles = (@_);

    $self->addfilematrix($specType, 
                         "PUBLIC",      #has no meaning here, 
                                        #but allows source share 
                         0,
                         \{}, 
                         $self->{generatedmatrix},
                         @infiles);
}

################################################################

=item $module-E<gt>addsources($t, $v, $requireType, [$list])

Update the list of source files of specified type and visibility
with $list. If the type/visibility combination is not found in
the source matrix, then create a new row for this combination.
Wildcards are not allowed; type and visibility have to be
explicit.

Note: All source filenames are relative to the directory
containing the awbfile...

=cut

################################################################
sub addsources
{
    # capture params
    my $self = shift;
    my $specType = shift;
    my $vis  = shift;
    my $requireType = shift;
    my $attributesRef = shift;
    my @infiles = (@_);
 
    $self->addfilematrix($specType, 
                         $vis, 
                         $requireType, 
                         $attributesRef,
                         $self->{sourcematrix},
                         @infiles);
}


################################################################

=item $module-E<gt>addfilematrix($t, $v, $requireType, $matrixReference, [$list])

Update the list of specified files of specified type and visibility
with $list. If the type/visibility combination is not found in
the matrix, then create a new row for this combination.
Wildcards are not allowed; type and visibility have to be
explicit.  This is intended to be generic, allowing the module to maintain 
multiple source lists.

Note: All source filenames are relative to the directory
containing the awbfile...

=cut

################################################################

sub addfilematrix
{
    # capture params
    my $self = shift;
    my $specType = shift;
    my $vis  = shift;
    my $requireType = shift; 
    my $attributesRef = shift;
    my $matrixReference = shift; 
    my @infiles = (@_);
    my $lastSlist = undef;
    #my @smat = $matrixReference->();

    # Validate type claims against file suffixes
    foreach my $f (@infiles)
    {

        # insert into source matrix           
        my $fileObject = Asim::Module::Source->new(filelist => [$f],
                                                   attributes => $attributesRef);

        my $type = decipher_type_from_extension($f);
#        $type = $specType if ($specType ne '');
        if (is_known_type($specType)) { # if the specType is one of the type that can be decipher, they better agree with each other
            if ($specType ne $type) {
                # Type mismatch
                if ((($specType eq 'BDPI_C') && ($type eq 'CPP')) ||
                    (($specType eq 'BDPI_H') && ($type eq 'H'))) {
                    # Accept BDPI override for C sources
                    $type = $specType;
                }
                else {
                    $self->ierror("For file $f specified type ($specType) does not match implicit type ($type)\n");
                }
            }
        } 
        elsif ($specType ne '') {
            $type = $specType;
        } 

#         if ($specType ne '')
#         {
#             # A type was specified by the user
#             $type = $specType if ($type eq '');
#             if (($type ne '') && ($specType ne $type)) {
#                 # Type mismatch
#                 if ((($specType eq 'BDPI_C') && ($type eq 'CPP')) ||
#                     (($specType eq 'BDPI_H') && ($type eq 'H'))) {
#                     # Accept BDPI override for C sources
#                     $type = $specType;
#                 }
#                 else {
#                     $self->ierror("For file $f specified type ($specType) does not match implicit type ($type)\n");
#                 }
#             }
#         }

        if ($requireType && ($type eq '')) {
            $self->ierror("Type of file $f not specified\n");
        }
        if ($vis eq '') {
            $self->ierror("Visibility of file $f not specified\n");
        }
        elsif (($vis ne 'PUBLIC') && ($vis ne 'PRIVATE')) {
            $self->ierror("Unrecognized visibility of file $f ($vis)\n");
        }

        if (defined($lastSlist) &&
            ($lastSlist->type() eq $type) &&
            ($lastSlist->visibility() eq $vis))
        {
            # Last file was the same type/visibility.  Use the same source list.

            # insert into source matrix           
            $lastSlist->addfiles(($fileObject)); # array?
        }
        else {
            # scan through source matrix
            my @smat = @{$matrixReference};#@{$self->{sourcematrix}};
            my $found = 0;
            foreach my $slist (@smat)
            {
                # check for type and visibility match
                if (($slist->type() eq $type) &&
                    ($slist->visibility() eq $vis))
                {       
                    $slist->addfiles(($fileObject));
                    $lastSlist = $slist;
                    $found = 1;
                    last;
                }
            }

            # if we we haven't found a type/visibility match
            # in our matrix, then create a new matrix entry for
            # this file list
            if ($found == 0)
            {
                my $slist = Asim::Module::SourceList->new(type => "$type",
                                                          visibility => "$vis",
                                                          files => []);
                $slist->addfiles(($fileObject));
                push(@{$matrixReference}, $slist);
                $lastSlist = $slist;
            }
        }
    }
}


################################################################
#
# If the $specType is known type, should agree with the decipher type
# 
#       
#
################################################################

sub is_known_type
{
    # capture params
    my $specType = shift;

    if ($specType eq "BDPI_C") {
        return 1;  
    }        # Bluespec imported C  
    elsif ($specType eq "BDPI_H") {
        return 1;  
    }        # Bluespec imported C
    elsif ($specType eq "BSV") {
        return 1;  
    }           # Bluespec
    elsif ($specType eq "BSH") {
        return 1;  
    }           # Bluespec include
    elsif ($specType eq "CPP") {
        return 1;  
    }           # C++
    elsif ($specType eq "DICT") {
        return 1;  
    }          # HAsim dictionary
    elsif ($specType eq "H") {
        return 1;  
    }             # C++
    elsif ($specType eq "NGC") {
        return 1;  
    }           # Xilinx netlist
    elsif ($specType eq "PACK") {
        return 1;  
    }          # Asim package file
    elsif ($specType eq "PRJ") {
        return 1;  
    }           # Xilinx project
    elsif ($specType eq "RRR") {
        return 1;  
    }           # HAsim request-response
    elsif ($specType eq "UCF") {
        return 1;  
    }           # Xilinx constraint file
    elsif ($specType eq "UT") {
        return 1;  
    }            # Xilinx script
    elsif ($specType eq "VHD") {
        return 1;  
    }           # VHDL
    elsif ($specType eq "XST") {
        return 1;  
    }           # Xilinx script
    
    return undef;
}

################################################################
#
# Decipher the source "type" of a file from its extension
# e.g., "foo.cpp" => CPP
#       "bar.v"   => VERILOG
#
################################################################

sub decipher_type_from_extension
{
    # capture params
    my $filename = shift;

    if ($filename =~ /.bsv$/)
    {
        return "BSV";           # Bluespec
    }
    elsif ($filename =~ /.bsh$/)
    {
        return "BSH";           # Bluespec include
    }
    elsif ($filename =~ /.\.c$/)
    {
        return "CPP";           # C++
    }
    elsif ($filename =~ /.\.cpp$/)
    {
        return "CPP";           # C++
    }
    elsif ($filename =~ /.\.dic$/)
    {
        return "DICT";          # HAsim dictionary
    }
    elsif ($filename =~ /.\.[hH]$/)
    {
        return "H";             # C++
    }
    elsif ($filename =~ /.\.ngc$/)
    {
        return "NGC";           # Xilinx netlist
    }
    elsif ($filename =~ /.\.pack$/)
    {
        return "PACK";          # Asim package file
    }
    elsif ($filename =~ /.\.prj$/)
    {
        return "PRJ";           # Xilinx project
    }
    elsif ($filename =~ /.\.rrr$/)
    {
        return "RRR";           # HAsim request-response
    }
    elsif ($filename =~ /.\.ucf$/)
    {
        return "UCF";           # Xilinx constraint file
    }
    elsif ($filename =~ /.\.ut$/)
    {
        return "UT";            # Xilinx script
    }
    elsif ($filename =~ /.\.v$/)
    {
        return "VERILOG";       # Verilog
    }
    elsif ($filename =~ /.\.vhd$/)
    {
        return "VHD";           # VHDL
    }
    elsif ($filename =~ /.\.xst$/)
    {
        return "XST";           # Xilinx script
    }
    else
    {
        return "";
    }
}


################################################################

=item $module-E<gt>makefile([$list])

Optionally update the list of makefiles for this module to $list

Return the current (updated) list of makefiles for this module

Note: All makefilenames are relative to the directory containing
the awbfile...

=cut

################################################################

sub makefile {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"makefile"}} = (@value);
  }

  return @{$self->{"makefile"}};
}

################################################################

=item $module-E<gt>scons($category, $update, ...)

Optionally update the scons file for specified category to $update

Return the current (updated) scons files for this module

Note: All scons names are relative to the directory containing
the awbfile...

=cut

################################################################

sub scons {
  my $self = shift;
  my $category = shift;
  my @value = (@_);

  $category = 'top' if (! defined($category));
  if (@value) {
    die("Asim::Module::scons: Cannot assign with a category of *") if ($category eq "*");

    @{$self->{"scons"}->{$category}} = (@value);
  }

  if ($category eq "*") {
    my @files = ();

    foreach my $c (keys %{$self->{"scons"}}) {
      push(@files, @{$self->{"scons"}->{$c}});
    }
    return (@files);
  }

  if (!defined($self->{"scons"}->{$category})) {
    return ();
  }

  return @{$self->{"scons"}->{$category}};
}

################################################################

=item $module-E<gt>library([$list])

Optionally update the list of library files used by this module to $list.

Return the current (updated) list of libraries used by this module.

Note: All library filenames are relative to the top of a package,
and are resolved using the package search path.

=cut

################################################################


sub library {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"library"}} = (@value);
  }

  return @{$self->{"library"}};
}

################################################################

=item $module-E<gt>syslibrary([$list])

Optionally update the list of library compiler args used to
build this module, to $list.

Return the current (updated) list of library compiler args used
to build this module.

=cut

################################################################


sub syslibrary {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"syslibrary"}} = (@value);
  }

  return @{$self->{"syslibrary"}};
}

################################################################

=item $module-E<gt>include([$type, [$update, ...]])

Optionally update the include files for specified type $update...

Return the current (updated) include files for this module

Note: All include directories are relative to the top of a package,
and are resolved using the package search path.

=cut

################################################################

sub include {
  my $self = shift;
  my $type = shift;
  my @value = (@_);

  $type = 'sw' if (! defined($type));
  if (@value) {
    die("Asim::Module::include: Cannot assign with a type of *") if ($type eq "*");

    @{$self->{"include"}->{$type}} = (@value);
  }

  if ($type eq "*") {
    my @files = ();

    foreach my $t (keys %{$self->{"include"}}) {
      push(@files, @{$self->{"include"}->{$t}});
    }
    return (@files);
  }

  if (!defined($self->{"include"}->{$type})) {
    return ();
  }

  return @{$self->{"include"}->{$type}};
}

################################################################

=item $module-E<gt>parameters()

Return list containing the parameters of a module

=cut

################################################################


sub parameters {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"params"}} = (@value);
  }

  return @{$self->{"params"}};
}


################################################################

=item $module-E<gt>setparameter($name, $value)

Set parameter of this module with name $name to $value.
Returns undef if there is no such parameter.

=cut

################################################################


sub setparameter {
  my $self = shift;
  my $pname = shift;
  my $pvalue = shift;

  my @params = $self->parameters();

  foreach my $p (@params) {
      if ($p->name() eq $pname) {
	  $p->value($pvalue);
	  return 1;
      }
  }

  return undef;
}


################################################################

=item $module-E<gt>getparameter($name)

Get parameter of this module with name $name.
Returns undef if there is no such parameter.

=cut

################################################################


sub getparameter {
  my $self = shift;
  my $pname = shift;

  my @params = $self->parameters();
  foreach my $p (@params) {
      if ($p->name() eq $pname) {
	  return $p;
      }
  }

  return undef;
}



################################################################

=item $module-E<gt>find_global_parameters();

Recursively search the module list looking for all the
global parameters.

Note: This method picks up the 'global' parameters from a
      submodel but does not recurse into them.

=cut

################################################################

sub find_global_parameters {
  my $self = shift;
  my $stop_at_submodel = shift || 0;

  my @result = ();


  if ($stop_at_submodel && $self->isroot()) {

    # Handle global parameters of a submodel

    my $submodel = $self->owner();

    foreach my $p ($submodel->parameters()) {
      if ($p->global()) {
        push(@result, $p);
      }
    }

    return (@result);
  }

  # Collect parameters of this module

  foreach my $p ($self->parameters()) {
    if ($p->global()) {
      push(@result, $p);
    }
  }

  # Collect parameters from each submodule

  foreach my $submodule ($self->submodules()) {
    next unless defined($submodule);

    push(@result, $submodule->find_global_parameters(1));
  }

  return (@result);
}


################################################################

=item $module-E<gt>attributes([$list])

Optionally update the list of attributes exhibited by this
module to $list.
Return the current (updated) list of attributes exhibited by
this module.

=cut

################################################################


sub attributes {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"attributes"}} = map Asim::Module::Attribute->new, @value;
  }

  return @{$self->{"attributes"}};
}

sub attributes2string {
  my $self = shift;
  my @attributes = ();

  foreach my $a ($self->attributes()) {
    push(@attributes, $a->fullname());
  }

  return join(" ", @attributes);
}


################################################################

=item $module-E<gt>find_modules_with_attribute($attribute [$exclude_submodels])

Recursively search the module list looking modules that have 
attribute $attribute. Return the list of such modules.

=cut

################################################################

sub find_modules_with_attribute {
  my $self = shift;
  my $attribute = shift;
  my $exclude_submodels = shift || 0;

  my @found = ();

  if ($self->has_attribute($attribute)) {
    push(@found, $self);
  }

  # Scan through submodules

  foreach my $submodule ($self->submodules()) {
    next unless defined($submodule);

    if ($submodule->isroot()) {
      my $submodel = $submodule->owner();
      if (!$exclude_submodels && $submodel->has_attribute($attribute)) {
	push(@found, $submodule);
      }
      next;
    }

    push(@found, $submodule->find_modules_with_attribute($attribute));

  }

  return (@found);
}


################################################################

=item $module-E<gt>is_obsolete()

Return 1 if module is obsolete

=cut

################################################################

sub is_obsolete {
  my $self = shift;

  return $self->has_attribute("obsolete");
}


################################################################

=item $module-E<gt>has_attribute($attribute)

Return 1 if module has attribute $attribute

=cut

################################################################

sub has_attribute {
  my $self = shift;
  my $attribute = shift;

  foreach my $i ($self->attributes()) {
    if ($i->name() eq $attribute) {
      return 1;
    }
  }

  return undef;
}

################################################################

=item $module-E<gt>default_attributes()

Optionally update the list of default attributes needed by this
module to $list.
Return the current (updated) list of attributes needed by
this module.

Return a array containing default attributes for a module.

=cut

################################################################


sub default_attributes {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"deafult_attributes"}} = (@value);
  }

  return @{$self->{"default_attributes"}};
}


################################################################

=item $module-E<gt>mandatory_attributes()

Optionally update the list of mandatory attributes needed by this
module to $list.
Return the current (updated) list of mandatory attributes needed by
this module.

=cut

################################################################


sub mandatory_attributes {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{"mandatory_attributes"}} = (@value);
  }

  return @{$self->{"mandatory_attributes"}};
}


################################################################

=item $module-E<gt>isroot([$value])

Return TRUE if $self is the root of a module tree. If boolean $value
is provided designate the module as the root of the module tree as
specified.

=cut

################################################################

sub isroot {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"isroot"} = $value;
      print $self->name() . ": Isroot = $value\n" if ($debug);
  }

  return $self->{"isroot"};
}


################################################################

=item $module-E<gt>owner([$value])

Return the owning model of this module. If $value
is provided designate it as the owner of this module.
specified.

Note: By convention only the root module of a model or
submodel will have an owner defined.

=cut

################################################################

sub owner {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
      $self->{"owner"} = $value;
  }

  return $self->{"owner"};
}


################################################################

=item $module-E<gt>find_module_providing($provides)

Recursively search the module list looking for the first module
that provides $provides

Maybe this should return the list of modules that provide $provides.

Note: this method does not recurse into submodels, and
      $stop_at_submodel is not a public argument!

=cut

################################################################

sub find_module_providing {
  my $self = shift;
  my $modtype = shift;
  my $stop_at_submodel = shift || undef;

  my $found;

  return $self if ($self->provides() eq $modtype);

  # Do not look at child of a submodule root;
  return undef if  $stop_at_submodel && $self->isroot();

  foreach my $submodule ($self->submodules()) {
    next unless defined($submodule);

    $found = $submodule->find_module_providing($modtype, 1);
    return $found if ($found);
  }

  return undef;
}

################################################################

=item $module-E<gt>embed_submodel($provides)

Turns any embedded submodels into direct components of the
module tree.

=cut

################################################################

sub embed_submodels {
  my $self = shift;

  foreach my $submodule ($self->submodules()) {
    next unless defined($submodule);

    if ($submodule->isroot()) {
      $submodule->isroot(0);
    }

    $submodule->embed_submodels();
  }

  return undef;
}

################################################################

=item $module-E<gt>find_module_requiring($requires)

Recursively search the module list looking for the first module
that requires $requires

Note: this method does not recurse into submodels, and
      $stop_at_submodel is not a public argument!


=cut

################################################################

sub find_module_requiring {
  my $self = shift;
  my $modtype = shift;
  my $found;

  foreach my $requires ($self->requires()) {
    return $self if ($requires eq $modtype);
  }

  foreach my $submodule ($self->submodules()) {
    next unless defined($submodule);
    next if $submodule->isroot();

    $found = $submodule->find_module_requiring($modtype);
    return $found if ($found);
  }

  return undef;
}


################################################################
# 
# Search the module database looking for a module
# named $modname of type $modtype.
# 
################################################################

sub find_new_module_location {
  my $self = shift;
  my $modname = shift;
  my $modtype = shift;

  my $moduleDB = Asim::Module::DB->new(".");

  my @candidates = $moduleDB->find($modtype);
  
  foreach my $candidate (@candidates) {
    if ($candidate->{name} eq $modname) {
        return $candidate;
    }
  }
  
  return undef;
}


################################################################

=item $module-E<gt>issame($module2)

Return TRUE if $module2 is the same as the base module. This is not
the LISP eq operation, since comparison is done just on object name.

=cut

################################################################

# Implemented in Asim::Base.

################################################################

=item $module-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base.

################################################################


=item $module-E<gt>edit()

Textually edit a module.

=cut

################################################################

# Implemented in Asim::Base.

################################################################

=item $module-E<gt>edit_hierarchy()

Textually edit the module hierarchy rooted at $module.

=cut

################################################################

sub edit_hierarchy {
  my $self = shift;
  my @submodules = $self->submodules();
  my @newmodules = ();

  #
  # In parallel, scan through the types of modules required
  # and the actual modules currently selected.
  #
  # Should be a menu selection...
  #
  foreach my $r ($self->requires) {
    my $m = shift @submodules;

    if (!defined($m)) {
      print "Sorry - need to add a module here...but its not implemented yet\n";
      next;
    }

    my $c = $m->name();

    if (Asim::choose_yes_or_no("Replace:")) {
      $m = $m->_replace_module($r);
    }

    if (Asim::choose_yes_or_no("Edit module:")) {
      # Really should just edit parameters
      $m->edit(qw(parameters));
    }

    if ($m->requires()
	&& Asim::choose_yes_or_no("Edit submodules:")) {
      $m->edit_hierarchy();
    }
    
    push(@newmodules, $m);
  }
  $self->submodules(@newmodules);
}

################################################################
#
# Replace module with another of same type
#
# This should be generified into a list selection method...
#
################################################################

sub _replace_module {
  my $self = shift;
  my $moduletype = shift;


  my $prompt = "Replace with module";

  #
  # How do I get the moduleDB for doing lists
  #          ...peeking at AsimShell:: is awful
  #
  my $moduleDB = AsimShell::get_moduleDB();
  my @choices = moduleDB->find($moduletype);

  return Asim::choose_from_list($prompt, @choices);
}

################################################################

=item $module-E<gt>dump()

Textually dump module to STDOUT

=cut

################################################################

sub dump {
  my $self = shift;

  my $level = 0;
  if (defined($_[0]) && $_[0] =~ /^\d+$/) {
    $level = shift;
  }

  $self->Asim::Base::dump($level);

  my @list;

  @list = $self->submodules();
  foreach my $i (@list) {
    if (! defined($i)) {
      print "\n";
      print "No module here\n";
    } else {
      print "\n";
      $i->dump($level+1);
    }
  }

  return;
}


################################################################

=item @outlist = Asim::Module::module_grep(&match, $module [,@inlist])

The non-method call will search through all the modules starting
at $module and select the modules that satify the match criteria
of the expression &match.

Diabled due to Perl problem - 'called too early to check prototype'

=cut

################################################################


#sub module_grep (&$) {
#  my $match = shift;
#  my $module = shift;
#  my @result = ();
#
#  if (&$match($module)) {
#    push(@result, $module);
#  }
#
#  foreach my $submodule ($module->submodules()) {
#    next unless defined($submodule);
#    next if $submodule->isroot();
#
#    push(@result, module_grep($match, $submodule));
#  }
#
#  return @result;
#}


################################################################
#
# Internal error utility function
#
################################################################

sub ierror {
    my $self = shift;
    my $message = shift;

    if (exists($self->{filename})) {
        print "Asim::Module ($self->{filename}) Error - $message";
    }
    else {
        print "Asim::Module Error - $message";
    }

    return 1;
}

=back

=head1 BUGS

  Modules need to be enhanced to include information about its
  interface, including at least the input and output ports.

  A number of accessor functions cannot be set to be empty lists.
  I need to rethink the representations...

  Cannot yet write out module configuration files.

  More error checking needed, e.g., 
      malformed lines on .awb files should be reported.


=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;

