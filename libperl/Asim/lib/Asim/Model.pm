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


package Asim::Model;
use warnings;
use strict;

use File::Basename;

use Asim::Base;
use Asim::Inifile;
use Asim::Packfile;
use Asim::Util;

our @ISA = qw(Asim::Base Asim::Inifile);

our %a =  ( version =>              [ "version",
                                      "SCALAR" ],
            name =>                 [ "name",
                                      "SCALAR" ],
            description =>          [ "description",
                                      "SCALAR" ],
            dependencies =>         [ "dependencies",
                                      "ARRAY" ],
            default_attributes =>   [ "default_attributes",
                                      "SCALAR" ],
            provides =>             [ "provides",
                                      "SCALAR" ],
            build_dir =>            [ "build_dir",
                                      "SCALAR" ],
            default_benchmark =>    [ "default_benchmark",
                                      "SCALAR" ],
            default_runopts =>      [ "default_runopts",
                                      "SCALAR" ],
            saveparams =>           [ "saveparams",
                                      "SCALAR" ],
            autoselect =>           [ "autoselect",
                                      "SCALAR" ],
            type =>                 [ "type",
                                      "SCALAR" ],
      );


our $debug = 0 || defined($ENV{ASIM_DEBUG}) || defined($ENV{ASIM_MODEL_DEBUG});


=head1 NAME

Asim::Model - Library for manipulating ASIM performance models

=head1 SYNOPSIS

use Asim::Model;

TBD

=head1 DESCRIPTION

This module provides an object to manipulate an ASIM model

=head1 METHODS

The following methods are supported:

=over 4

=cut

################################################################


=item $model = Asim::Model-E<gt>new([$file])

Create a new performance model, optionally reading configuration 
file $file to populate the object.

=cut

################################################################


sub new {
  my $this = shift;
  my $class = ref($this) || $this;
  my $self = {};

  bless	$self, $class;

  $self->_initialize();

  #
  # Parse file if given
  #
  if (@_) {
      $self->open($_[0]) || return undef;
  }

  return $self;
}

sub _initialize {
  my $self = shift;

  $self->{accessors} = \%a;
  $self->{filename}  = undef;
  $self->{inifile}   = {};
  $self->{system}    = undef;
  $self->{workbench} = undef;
  $self->{packfile}  = ();
  $self->{provides} = "model";
  $self->{build_dir} = undef;
  $self->{run_dir} = undef;

  $self->{params}    = [];

  $self->{missing} = 0;
  $self->{missing_packages} = {};

  $self->{inifile} = Asim::Inifile->new();

  $self->name("New Asim Model");
  $self->description("");
  $self->default_attributes("");

  $self->default_benchmark("");
  $self->default_runopts("");

  $self->saveparams(0);
  $self->autoselect(0);

  $self->modified(0);
  return $self;
}

################################################################

=item $model-E<gt>open($file)

Parse a performance model file $file and populate the attributes
of the model object with the information in the file.

=cut

################################################################


sub open {
  my $self = shift;
  my $file = shift;
  my $inifile;

  #
  # Cannonicalize the filename and parse the configuration file
  #
  $file = $Asim::default_workspace->resolve($file) || return undef;
  $inifile = Asim::Inifile->new($file) || return undef;

  $self->{filename} = $file;
  $self->{inifile} = $inifile;

  #
  # Get the well known root modules
  #
  my $version = $inifile->get("Global", "Version");

  if ($version == "1.0") {
    my $workbench = $inifile->get("Model","workbench");
    $self->{workbench} = $self->_get_module($workbench);

    my $system = $inifile->get("Model","system");
    $self->{system} = $self->_get_module($system);

  } elsif ($version >= 2.0 && $version < 3.0) {
    #
    # Althought we are actually version 2.1, we
    # should be able to read versions up to 3.0
    #
    my $model =  $inifile->get("Model","model");
    $self->modelroot($self->_get_module($model));

  } else {
    _process_error("Illegal model version - $version\n");
    return ();
  }

  #
  # Collect globally export parameters from modules
  # and add them as parameters of the model itself
  #
  my $modelroot = $self->modelroot();

  if (defined($modelroot)) {
    @{$self->{params}} = $modelroot->find_global_parameters();
  }

  #
  # Get module packfile
  #
  $self->{packfile} = ($self->_get_packfile());


  $self->modified(0);
  return 1;
}


sub _get_module {
  my $self = shift;
  my $modname = shift;
  my $inifile = $self->{inifile};

  my $awbfile = $inifile->get($modname, "File");

  if (! defined($awbfile)) {
    _process_warning("Module ($modname) has no file in .apm file\n");

    $self->{missing}++;
    return undef;
  }

  if ($awbfile =~ /\.apm$/) {
    # It is a sub-model!!!
    return $self->_get_submodel($modname, $awbfile);
  }

  my $module = Asim::Module->new($awbfile);

  if (! defined($module)) {
    _process_warning("Model needed module with non-existant awb file ($awbfile)\n");
    $self->_check_module($modname, $awbfile);

    $self->{missing}++;
    return undef;
  }

  #
  # Override parameters
  #
  my @overrides = $inifile->get_itemlist("$modname/Params");
  foreach my $o (@overrides) {
      my $v = $inifile->get("$modname/Params", $o);
      $module->setparameter($o, $v) 
        || _process_warning("Attempt to set undefined parameter ($o)\n");
  }

  #
  # Recursively get the submodules
  #
  my @requires = $module->requires();
  my $submodname;
  my $submod;

  foreach my $r (@requires) {
      $submodname = $inifile->get("$modname/Requires", $r);

      if (! defined($submodname)) {
        _process_warning("Model missing \"$r\" in [$modname/Requires] in .apm file\n");
        $self->{missing}++;
        next;
      }

      $submod = $self->_get_module($submodname) || next;
      $module->add_submodule($submod);
  }

#  print ".";
  return $module;
}

sub _get_submodel {
  my $self = shift;
  my $modname = shift;
  my $apmfile = shift;

  my $inifile = $self->{inifile};

  my $submodel = Asim::Model->new($apmfile);

  if (!defined($submodel)) {
    _process_warning("Somethng wrong with submodel $apmfile\n");
    $self->{missing}++;
    return undef;
  }

  #
  # Override parameters (copy of code in _get_module)
  #
  my @overrides = $inifile->get_itemlist("$modname/Params");

  foreach my $o (@overrides) {
      my $v = $inifile->get("$modname/Params", $o);
      $submodel->setparameter($o, $v) 
        || _process_warning("Attempt to set undefined parameter ($o)\n");
  }


  #
  # Collect information about missing modules and
  # potential missing packages from submodel
  #
  $self->{missing} += $submodel->missing_module_count();

  foreach my $p ($submodel->missing_packages()) {
    $self->{missing_packages}->{$p} = 1;
  }

  #
  # A submodel is represented simply as a module in the tree, but
  # because it has the property isroot() and its owner() will not be
  # the top-level model, one can check if it is a submodel. The method
  # 'is_submodel' does this check...
  #

  return $submodel->modelroot();
}

sub _get_packfile {
  my $self = shift;

  print "Asim::Model - Looking for packfile\n" if $debug;

  #
  # Find the module with that provides the "packages"
  #
  my $system = $self->modelroot()                              || return ();
  my $packmodule = $system->find_module_providing("packages")  || return ();


  #
  # Open the module with the 
  my $awbfilename = $packmodule->filename()                    || return ();
  my $awbfile = Asim::Module->new($awbfilename)                || return ();


  #
  # Extract the packfile out of the module private files
  # (do not forget that privates are located relative to the awbfile itself)
  #
  my ($packfilename) = ($awbfile->private());
  $packfilename = dirname($awbfile->filename) . "/" . $packfilename;

  #
  # Open the packfile
  #
  my $packfile = Asim::Packfile->new($packfilename)            || return ();

  return $packfile;
}

#
# Output warning with info on package a missing module
# might have come from
#

sub _check_module {
  my $self = shift;
  my $modname = shift;
  my $awbfile = shift;

  my $inifile = $self->{inifile};
  my $package = $inifile->get($modname, "Packagehint");

  if ($package) {
    $self->{missing_packages}->{$package} = 1;
    _process_warning("Awbfile \"$awbfile\" was expected to be in package \"$package\"\n");
  }

  return;
}

################################################################

=item $model-E<gt>save($file)

Parse a performance model file $file and populate the attributes
of the model object with the information in the file.

=cut

################################################################


sub save {
  my $self = shift;
  my $file = shift || $self->filename() || return undef;
  my $root = $self->modelroot()         || return undef;

  print "Asim::Model - Saving $file\n" if ($debug);

  #
  # Create a new empty configuration files
  #    Note: we create the new one out of the old one,
  #          so that the format will be saved on a save
  #

  my $new_inifile = Asim::Inifile->new($self->filename()) || return undef;
  $new_inifile->flush();

  #
  # Save [Global] section
  #
  $new_inifile->put("Global", "Version", "2.1");
  $new_inifile->put("Global", "Class", "Asim::Model");
  #
  $new_inifile->put("Global", "Name", $self->name());
  $new_inifile->put("Global", "Description", $self->description());
  $new_inifile->put("Global", "Type", $self->type());
  $new_inifile->put("Global", "SaveParameters", $self->saveparams());
  $new_inifile->put("Global", "DefaultBenchmark", $self->default_benchmark());
  $new_inifile->put("Global", "DefaultRunOpts", $self->default_runopts());

  my $name = basename($file);
  $name =~ s/\.[^.]*$//;
  $new_inifile->put("Global", "File", $name);

  #
  # Save [Model] section
  #
  $new_inifile->put("Model", "DefaultAttributes", $self->default_attributes());
  $new_inifile->put("Model", "model", $root->name());

  #
  # put the module hierarchy
  #
  $self->_put_module($new_inifile, $root);

  $new_inifile->save($file) || return undef;

  #
  # Use new inifile as the inifile for the model
  # and remember the possibly new filename
  #
  $self->{inifile} = $new_inifile;

  if ($self->{filename} ne $file) {
      $self->{filename} = $file;
      $self->{build_dir} = undef;
      $self->{run_dir} = undef;
  }

  $self->modified(0);
  return 1;
}

sub _put_module {
  my $self = shift;
  my $inifile = shift;
  my $module = shift;
  my $modname = $module->name();
  my $filename;
  my $packagename;

  if ($module->isroot() && $self != $module->owner()) {
    #
    # This is a submodel - put reference to submodel
    # (but we call it module for operations below)
    #
    $module = $module->owner();
    $filename = $Asim::default_workspace->unresolve($module->filename());
  } else {
    $filename = $module->filename();
  }

  #
  # Save filename and packagehint
  #
  $inifile->put($modname, "File", $filename);

  $packagename = Asim::file2package($filename);
  $inifile->put($modname, "Packagehint", $packagename);

  #
  # Save overriden parameters
  #
  foreach my $p ($module->parameters()) {
    if ($self->saveparams() || ($p->value() ne $p->default())) {
      $inifile->put("$modname/Params", $p->name(), $p->value());
    }
  }

  #
  # Recursively put the submodules
  #    Note: submodels will have no submodules
  #
  foreach my $s  ($module->submodules()) {
    if (!defined($s)) {
      print "Missing submodule of $modname\n";
      next;
    }

    $inifile->put("$modname/Requires", $s->provides(), $s->name());
    $self->_put_module($inifile, $s);
  }

  return 1;
}

################################################################

=item $model-E<gt>backup($file)

Backup the file.

=cut

################################################################


sub backup {
  my $self = shift;
  my $file = shift || $self->filename() || return undef;

  return $self->{inifile}->backup($file);
}


################################################################

=item $model-E<gt>accessors()

Return a list of accessor functions for this object

=cut

################################################################

sub accessors {
    return qw(version name description dependencies saveparams default_benchmark default_runopts type);
}


################################################################

=item $model-E<gt>filename()

Filename of source of module information

=cut

################################################################

sub filename {
  my $self = shift;

  return $self->{filename};
}

################################################################

=item $model-E<gt>build_dir($dir)

Directory model is build in. Optionally set to $dir.

=cut

################################################################

sub build_dir {
  my $self = shift;
  my $dir = shift;

  # Assign new build_dir if appropriate

  if (defined($dir)) {
    $self->{build_dir} = $dir;
  }

  # Define default build_dir if needed

  if (!defined($self->{build_dir})) {
    my $configfile = $self->{filename};
    my ($base,$path,$type) = fileparse($configfile, '\.apm');
    $self->{build_dir} = $Asim::default_workspace->build_dir . "/$base/pm";
  }

  return $self->{build_dir};
}

################################################################

=item $model-E<gt>run_dir([$dir|$benchmarkfile])

Directory model with benchmark is run in. If an argument is
a directory, set run_dir to that directory. If argument is
a benchmark configuration file, determine default run_dir for
that benchmark configuration file.

=cut

################################################################

sub run_dir {
  my $self = shift;
  my $arg = shift;

  if (defined($arg)) {

    # If $arg is a directory use it as the run_dir

    if (-d $arg) {
      $self->{run_dir} = $arg;
    }

    # If $arg is a .cfg file, use it to calculate a run_dir

    if ($arg =~ /\.cfg$/) {
      # TBD: Should be relative to build_dir()!!!!

      my $configfile = $self->{filename};
      my ($pm_base,$pm_path,$pm_type) = fileparse($configfile, '\.apm');
      my ($bm_base,$bm_path,$bm_type) = fileparse($arg, '\.cfg');

      $self->{run_dir} = $Asim::default_workspace->build_dir() . "/$pm_base/bm/$bm_base";
    }
  }

  # If we have no run_dir calculate a default

  if (!defined($self->{run_dir})) {
    # TBD: Should be relative to build_dir()!!!!

    my $configfile = $self->{filename};
    my ($pm_base,$pm_path,$pm_type) = fileparse($configfile, '\.apm');
    my $bm_base = "benchmark";
    $self->{run_dir} = $Asim::default_workspace->build_dir() . "/$pm_base/bm/$bm_base";
  }

  return $self->{run_dir};
}


################################################################

=item $model-E<gt>version([$value])

Set model "version" to $value if supplied. 
Always return configuration file "version".

=cut

################################################################

sub version     { return $_[0]->_accessor("Global","Version",$_[1]) || "2.0"; }



################################################################

=item $model-E<gt>type([$value])

Set model "type" to $value if supplied. 
Always return model "type".

=cut

################################################################

sub type     { return $_[0]->_accessor("Global","Type",$_[1]) || "Asim"; }



################################################################

=item $model-E<gt>name([$value])

Set model "name" to $value if supplied. 
Always return model "name".

=cut

################################################################

sub name        { return $_[0]->_accessor("Global","Name",$_[1]) || ""; }

################################################################

=item $model-E<gt>description([$value])

Set model "description" to $value if supplied. 
Always return current model "description".

=cut

################################################################

sub description { return $_[0]->_accessor("Global","Description",$_[1]) || ""; }

################################################################

=item $model-E<gt>default_benchmark([$value])

Set model "default_benchmark" to $value if supplied. 
Always return "default_benchmark".

=cut

################################################################

sub default_benchmark  { return $_[0]->_accessor("Global","DefaultBenchmark",$_[1]) || ""; }


################################################################

=item $model-E<gt>default_runopts([$value])

Set model "default_runopts" to $value if supplied. 
Always return "default_runopts".

=cut

################################################################

sub default_runopts  { return $_[0]->_accessor("Global","DefaultRunOpts",$_[1]) || ""; }


################################################################

=item $model-E<gt>saveparams([$value])

Set model "saveparams" to $value if supplied. 
Always return "saveparams".

=cut

################################################################

sub saveparams  { return $_[0]->_accessor("Global","SaveParameters",$_[1]) || 0; }



################################################################

=item $model-E<gt>autoselect([$value])

Set model "autoselect" to $value if supplied. 
Always return "autoselect".

Note: this value is not saved in the .apm file
=cut

################################################################

sub autoselect {
    my $self = shift;
    my $value = shift;

    if (defined($value)) {
	$self->{autoselect} = $value;
    }

    return $self->{autoselect};
}


################################################################

=item $model-E<gt>default_attributes([$list])

Set model "default attributs" to list $list if supplied.
Always return current "default attributes".

=cut

################################################################

sub attributes {
  return $_[0]->default_attributes();
}

sub default_attributes { return $_[0]->_accessor("Model","DefaultAttributes",$_[1]) || ""; }


################################################################

=item $model-E<gt>dependencies()

Return list of packages on which this model is dependent

=cut

################################################################

sub dependencies {
  my $self = shift;
  my $packfile = $self->{packfile} || return ();

  return ($packfile->dependencies());
}

################################################################

=item $model-E<gt>modelroot([$moduletree])

Return tree of modules for this model. Optionally setting it first to
$moduletree.

=cut

################################################################

sub modelroot  {
  my $self = shift;
  my $value = shift;

  if (defined($value)) {
    #
    # Remove isroot tag from old root
    #
    if (defined($self->{system})) {
      $self->{system}->isroot(0);
      # TBD: Probably should undefine owner() as well
    }

    # Set module as root of model and owned by this model

    $value->isroot(1);
    $value->owner($self);

    $self->{system} = $value;
    $self->{provides} = $value->provides();
    $self->modified(1);
  }

  return ($self->{system});
}

################################################################

=item $module-E<gt>is_submodel($module)

Return TRUE if $module is a submodel inside $self. If given a model
as $module we return FALSE, but otherwise we check if the module
is the root a submodel different from $self.

=cut

################################################################

sub is_submodel {
  my $self = shift;
  my $module = shift;

  if (ref($module) eq "Asim::Model") {
      return 0;
  }

  return ($module->isroot() && ($self->modelroot() != $module));
  
}

################################################################

=item $model-E<gt>submodules()

Return submodules of this model, which is the null set since 
this is a model.

Only here for compatibility with Asim::Module::submodules

=cut

################################################################

sub submodules {
  return ();
}


################################################################

=item $model-E<gt>smart_add_submodule($parent_module $child_module)

Add the $child_module as a submodule of the parent module.
If parent is undefined we make child the modelroot.

This function will work if the $child_module is a submodel.

IF the $child_module is replacing an existing mooule then an 
attempt will be made to add the old grand-children into the
new child.

=cut

################################################################

sub smart_add_submodule {
  my $self = shift;
  my $new_child_module = shift;

  my $parent_module;
  my $old_child_module;

  my $provides = $new_child_module->provides();

  print "Smart adding " . $new_child_module->name() . "\n" if ($debug);

  if ($provides eq $self->provides()) {
    #
    # This is the root --- handle specially
    #
    if (ref($new_child_module) eq "Asim::Model") {
      _process_warning("Illegal attempt to insert submodel as root of model/n");
      return undef;
    }

    $parent_module = undef;
    $old_child_module = $self->modelroot();

  } else {
    #
    # This is an internal node...
    #

    #
    # Find the parent module by looking for the module that
    # requires a module of the chosen type.... 
    #
    #    Note: implicit dependence on single provides
    #
    $parent_module = $self->find_module_requiring($provides);
    if (! defined($parent_module)) {
      # Module type not used in this model
      return undef;
    }

    $old_child_module = $parent_module->find_module_providing($provides);
  }


  # Preserve values of identially named global parameters 
  # from previous module in new module

  if (defined($old_child_module)) {
    my @parameters = ();

    if ($old_child_module->isroot() && $self != $old_child_module->owner()) {
      @parameters = $old_child_module->owner()->parameters();
    } else {
      @parameters = $old_child_module->parameters();
    }

    foreach my $p (@parameters) {
      #
      # Try to set value to previous value
      #    - setparameter() will ignore parameter names it doesn't know
      #    - similarly setting a parameter to its default value is okay
      #
      $new_child_module->setparameter($p->name(), $p->value());
    }
  }

  #
  # Check if we're adding a submodel
  #
  if (ref($new_child_module) eq "Asim::Model") {

    print "Adding a new submodel\n" if ($debug);

    #
    # A submodel is represented simply as a module in the tree, but
    # because it has the property isroot() and its owner() will not be
    # the top-level model, one check check if it is a submodel
    #
    return $self->add_submodule($parent_module, $new_child_module->modelroot());
  }


  #
  # Preserve any sub-submodules that were of the same type 
  # as in the submodule being replaced, as long as we are 
  # not the root of a submodel.
  #
  if (defined($old_child_module)
      && !($old_child_module->isroot() && defined($parent_module))) {
    foreach my $s ($old_child_module->submodules()) {
      if (defined($s)) {
        $self->add_submodule($new_child_module, $s);
      }
    }
  }

  #
  # Add the new child module into the tree
  #
  return $self->add_submodule($parent_module, $new_child_module);
}

################################################################

=item $model-E<gt>add_submodule($parent_module $child_module)

Add the $child_module as a submodule of the parent module.
If parent is undefined we make child the modelroot.

(We actually do not check that the parent is part of the model!)

=cut

################################################################

sub add_submodule {
  my $self = shift;
  my $parent = shift;
  my $child = shift;

  if (! defined($parent)) {
    $self->modelroot($child);
    $self->modified(1);
    return 1;
  }

  # Note: this isn't recursive since $parent is a Asim::Module

  $parent->add_submodule($child);
  $self->modified(1);

  return 1;
}
################################################################

=item $model-E<gt>provides()

Return the asim-type of the model. Actually this is just the asim-type
of the root of the model tree as assigned when the model root is
created.

=cut

################################################################

sub provides {
  my $self = shift;

  return $self->{provides};

  return 1;
}

################################################################

=item $model-E<gt>template()

Return TRUE if $self is a template, but we return 0 since this is a MODEL

Only here for compatibility with Asim::Module::template()

=cut

################################################################

sub template {
  return 0;
}

################################################################

=item $model-E<gt>requires()

Return the asim-type of the submoodules requires. Just here for
compabitility with a modules, and so always returns an empty list.

=cut

################################################################

sub requires {
  my $self = shift;

  return ();
}


################################################################

=item $model-E<gt>notes()

Return the notes files. Just here for compabitility 
with a module, and so always returns an empty list.

=cut

################################################################

sub notes {
  my $self = shift;

  return ();
}

################################################################

=item $model-E<gt>public()

Return the public files. Just here for compabitility with a module,
and so always returns an empty list.

=cut

################################################################

sub public {
  my $self = shift;

  return ();
}


################################################################

=item $model-E<gt>private()

Return the private files. Just here for compabitility with a module,
and so always returns an empty list.

=cut

################################################################

sub private {
  my $self = shift;

  return ();
}


################################################################

=item $model-E<gt>makefile()

Return the Makefiles. Just here for compabitility with a module,
and so always returns an empty list.

=cut

################################################################

sub makefile {
  my $self = shift;

  return ();
}

################################################################

=item $model-E<gt>scons()

Return the scons files. Just here for compabitility with a module,
and so always returns an empty list.

=cut

################################################################

sub scons {
  my $self = shift;

  return ();
}


################################################################

=item $model-E<gt>parameters()

Return the parameters. Which are all the global parameters
of the submodel.

Exact copy of code from Asim::Module::paramters

=cut

################################################################

sub parameters {
  my $self = shift;
  my @value = (@_);

  if (@value) {
    @{$self->{params}} = (@value);
  }

  return @{$self->{params}};
}


################################################################

=item $model-E<gt>setparameter($name, $value)

Set parameter of this (sub)model with name $name to $value.
Returns undef if there is no such parameter.

Exact copy of code from Asim::Module::setparameter

WARNING: This changes the value of the paramters in the submodel
         so don't ever save the submodel...

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

=item $model-E<gt>getparameter($name)

Get parameter of this module with name $name.
Returns undef if there is no such parameter.

Exact copy of code from Asim::Module::setparameter

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

=item $model-E<gt>remove_submodule($parent_module $child_module)

Remove the $child_module as a submodule of the parent module.
If parent is undefined remove the modelroot.

(We actually do not check that the parent is part of the model!)

=cut

################################################################

sub remove_submodule {
  my $self = shift;
  my $parent = shift;
  my $child = shift;

  if (! defined($parent)) {
    $self->{system} = undef;
    $self->modified(1);
    return 1;
  }

  $parent->remove_submodule($child);
  $self->modified(1);

  return 1;
}

################################################################

=item $model-E<gt>contains_module_providing($provides)

Recursively search the module list looking a stop in the
model that needs a module of type $provides

Note: this method does not recurse into submodels

=cut

################################################################

sub contains_module_providing {
  my $self = shift;
  my $provides = shift;

  # TBD: This is a pretty naive implementation...

  return defined($self->find_module_providing($provides)) ||
         defined($self->find_module_requiring($provides));
}

################################################################

=item $model-E<gt>find_module_providing($provides)

Recursively search the module list looking for the first module
that provides $provides

Maybe this should return the list of modules that provide $provides.

Note: this method does not recurse into submodels

=cut

################################################################

sub find_module_providing {
  my $self = shift;
  my $provides = shift;
  my $root = $self->modelroot() || return undef;

  return $root->find_module_providing($provides);
}

################################################################

=item $model-E<gt>find_module_requiring($requires)

Recursively search the module list looking for the first module
that requires $requires.

Maybe this should return the list of modules that requires $requires.

Note: this method does not recurse into submodels

=cut

################################################################

sub find_module_requiring {
  my $self = shift;
  my $requires = shift;
  my $root = $self->modelroot() || return undef;

  return $root->find_module_requiring($requires);
}


################################################################

=item $model-E<gt>is_default_module($module)

Check if module satisfies the default attributes of the model
Return value is a score of how well the module satisifies the
default attribute contraints - 0 signifies it does not match
any attribute.

The current scoring algorithm scores first by number of matching
attributes and then by the order of the first matching attribute
(left or first is highest).

=cut

################################################################

sub is_default_module {
  my $self = shift;
  my $module = shift;

  my $match_count = 0;
  my $attr_number = 0;
  my $first_attr_number = 0;
  my $score;

  my @model_attr = split(' ', $self->default_attributes());
  my @module_attr = $module->attributes();

  foreach my $a (@model_attr) {
    $attr_number++;

    foreach my $aa (@module_attr) {
      if ($a eq $aa) {
        $first_attr_number = $attr_number if ($first_attr_number == 0);
        $match_count++;
      }
    }
  }

  $score = $match_count*1000+$first_attr_number;
  print "Module = " . $module->name() . " SCORE = $score\n" if ($debug);

  return $score;
}

################################################################

=item $model-E<gt>missing_module_count()

Returns the number of modules missing from the model.

Note: Only valid right after model is read in.

=cut

################################################################

sub missing_module_count {
  my $self = shift;

  return $self->{missing};
}

################################################################

=item $model-E<gt>embed_submodels()

Turns any embedded submodels into direct components of the model

=cut

################################################################

sub embed_submodels {
  my $self = shift;
  my $root = $self->modelroot();

  if (! defined($root)) {
    return undef;
  }

  return $root->embed_submodels();
}

################################################################

=item $model-E<gt>missing_packages()

Returns a list of potentially missing packages. These were collected while the model
was being read, and includes the 'Packagehint's for any missing modules.

=cut

################################################################

sub missing_packages {
  my $self = shift;
  my @missing = keys %{$self->{missing_packages}};

  return (@missing)
}

################################################################

=item $model-E<gt>modified([$value])

Return a boolean indicating whether the object has been modified,
optionally updating the modified flag with value $value.

=cut

################################################################

# Implemented in Asim::Base

################################################################

=item $model-E<gt>edit()

Edit the model

=cut

################################################################

sub edit {
  my $self = shift;

  if (defined($ENV{DISPLAY})) {
    system("apm-edit " . $self->filename());
  } else {
    $self->Asim::Base::edit();

    $self->{workbench}->edit_hierarchy();
    $self->{system}->edit_hierarchy();
  }
}

################################################################

=item $model-E<gt>clean([--builddir => <builddir>]
			[--getcommand => {0,1}])

Clean the build area for the specified model.

=cut

################################################################

sub clean {
  my $self = shift;
  my %args = @_;
  my $builddir = $args{"--builddir"} || $self->build_dir();
  $self->build_dir($builddir);

  my $status;

  print "Asim::Model - clean model\n" if $debug;

  if (! -e "$builddir/Makefile") {
      return _process_warning("No Makefile for clean\n",%args);
  }

  # determine correct command (Asim v. Leap v. Hasim)
  my $cmd = "cd $builddir; make clean";
  if ($self->type() eq "Leap") {
      $cmd = "cd $builddir; make clean";
  }
  if ($self->type() eq "HAsim") {
      $cmd = "cd $builddir; make clean";
  }

  return _process_command($cmd,%args);
}

################################################################

=item $model-E<gt>nuke([--builddir => <builddir>]
		       [--getcommand => {0,1}])

Nukes (completely cleans) the build area for the specified model.

=cut

################################################################

sub nuke {
  my $self = shift;
  my %args = @_;
  my $builddir = $args{"--builddir"} || $self->build_dir();

  $self->build_dir($builddir);

  my $filename = $self->filename();
  my $status;

  print "Asim::Model - nuke model\n" if $debug;
  
  if (! $self->isconfigured()) {
      return _process_warning("Cannot nuke since model does not appear to be configured\n", %args);
  }

  # determine correct command (Asim v. Leap v. Hasim)
  my $cmd = "amc --model=$filename --builddir=\"$builddir\" nuke";
  if ($self->type() eq "Leap") {
      $cmd = "leap-configure --model=$filename --builddir=\"$builddir\" -nuke";
  }
  if ($self->type() eq "HAsim") {
      $cmd = "hasim-configure --model=$filename --builddir=\"$builddir\" -nuke";
  }

  return _process_command($cmd,%args);
}

################################################################

=item $model-E<gt>configure([--builddir => <builddir>]
			    [--getcommand => {0,1}])

Configure the build area for the specified model.

=cut

################################################################

sub configure {
  my $self = shift;
  my %args = @_;
  my $builddir = $args{"--builddir"} || $self->build_dir();
  my $persist = $args{"--persist"} || undef;

  $self->build_dir($builddir);

  my $filename = $self->filename();
  my $status;

  print "Asim::Model - configure model\n" if $debug;

  # determine correct command (Asim v. Leap v. Hasim)
  my $cmd;
  if (defined $persist && $persist) {
      $cmd = "amc --persist --model=$filename --builddir=\"$builddir\" configure";
  }
  else {
      $cmd = "amc --model=$filename --builddir=\"$builddir\" configure";
  }
  if ($self->type() eq "Leap") {
      $cmd = "leap-configure --model=$filename --builddir=\"$builddir\" -configure";
  }
  if ($self->type() eq "HAsim") {
      $cmd = "hasim-configure --model=$filename --builddir=\"$builddir\" -configure";
  }

  return _process_command($cmd,%args);
}


################################################################

=item $model-E<gt>build([--builddir => <builddir>]
                        [--buildopt => <options>]
			[--getcommand => {0,1}])

Build the specified model with make options $options.

=cut

################################################################

sub build {
  my $self = shift;
  my %args = @_;
  my $buildopt = $args{"--buildopt"} || "";
  my $builddir = $args{"--builddir"} || $self->build_dir();

  $self->build_dir($builddir);

  my $filename = $self->filename();
  my $status;

  print "Asim::Model - build model ($buildopt)\n" if $debug;

  if (! $self->isconfigured()) {
      return _process_error("Model does not appear to be configured\n", %args);
  }

  # determine correct command (Asim v. Leap  v. Hasim)
  my $cmd = "amc --model=$filename --builddir=\"$builddir\" --buildopt=\"$buildopt\" build";
  if ($self->type() eq "Leap") {
      $cmd = "leap-configure --model=$filename --builddir=\"$builddir\" --buildopt=\"$buildopt\" -build";
  }
  if ($self->type() eq "HAsim") {
      $cmd = "hasim-configure --model=$filename --builddir=\"$builddir\" --buildopt=\"$buildopt\" -build";
  }

  return _process_command($cmd,%args);
}

################################################################

=item $model-E<gt>build_options()

Return a hash of the build options for this model

=cut

################################################################

sub build_options {
  my $self = shift;

  my $model_type = $self->type();
  my %options;

  # TBD: These options should be generated dynamically

  if ($model_type eq "Asim") {
    $options{"TRACE"} = "Debugging Trace Code";
    $options{"EVENTS"} = "Event Log Code";
    $options{"PROFILE"} = "Instruction Profiling Code";
    $options{"DISTCC"} = "Use distcc with gcc";
    $options{"CCACHE"} = "Use ccache with gcc";
    $options{"GPROF"} = "add -pg option for gprof";
  }


  if ($model_type eq "HAsim" || $model_type eq "Leap") {
    $options{"vexe"} = "Verilog simulation executable <T>";
    $options{"exe"} = "C simulation executable <T>";
    $options{"bit"} = "FPGA bitfile <T>";
  }

  return \%options;

}

################################################################

=item $model-E<gt>setup($benchmark
                        [--builddir   => <builddir>]
                        [--rundir     => <rundir>]
			[--getcommand => {0,1}])

Setup the specified benchmark for model.

=cut

################################################################

sub setup {
  my $self = shift;
  my $benchmark = shift;
  my %args = @_;
  my $builddir = $args{"--builddir"} || $self->build_dir();
  my $logfile = $args{"--logfile"} || undef;
  $self->build_dir($builddir);

  my $rundir = $args{"--rundir"} || $self->run_dir($benchmark);

  my $filename = $self->filename();
  my $status;

  if (! $self->isconfigured()) {
      return _process_error("Model does not appear to be configured\n", %args);
  }

  # determine correct command (Asim v. Leap v. Hasim)
  my $cmd = "amc --model=$filename --builddir=\"$builddir\" " .
      "--benchmark=$benchmark --rundir=\"$rundir\" setup";
  if ($self->type() eq "Leap") {
    $cmd = "leap-configure  --model=$filename --builddir=\"$builddir\" " .
           "--benchmark=$benchmark --rundir=\"$rundir\" -setup";
  }
  if ($self->type() eq "HAsim") {
    $cmd = "hasim-configure  --model=$filename --builddir=\"$builddir\" " .
           "--benchmark=$benchmark --rundir=\"$rundir\" -setup";
  }

  return _process_command($cmd,%args);
}

################################################################

=item $model-E<gt>run([--builddir => <builddir>]
                      [--rundir => <rundir>]
                      [--runopt => <options>]
		      [--getcommand => {0,1}])
)

Run the specified model.

=cut

################################################################

sub run {
  my $self = shift;
  my $benchmark = shift;
  my %args = @_;
  my $runopt = $args{"--runopt"} || "";
  my $builddir = $args{"--builddir"} || $self->build_dir();
  $self->build_dir($builddir);

  my $rundir = $args{"--rundir"} || $self->run_dir($benchmark);

  my $filename = $self->filename();
  my $status;

  if (! $self->issetup()) {
      return _process_error("Model does not appear to be configured\n", %args);
  }

  # determine correct command (Asim v. Leap  v. Hasim)
  my $cmd = "amc --model=$filename --builddir=\"$builddir\" " . 
      "--benchmark=$benchmark --rundir=\"$rundir\" --runopt=\"$runopt\" run";
  if ($self->type() eq "Leap") {
    $cmd = "leap-configure --model=$filename --builddir=\"$builddir\" " . 
      "--benchmark=$benchmark --rundir=\"$rundir\" --runopt=\"$runopt\" -run";
  }
  if ($self->type() eq "HAsim") {
    $cmd = "hasim-configure --model=$filename --builddir=\"$builddir\" " . 
      "--benchmark=$benchmark --rundir=\"$rundir\" --runopt=\"$runopt\" -run";
  }

  return _process_command($cmd,%args);
}


################################################################

=item $model-E<gt>isconfigured()

Check if the model is configured.

=cut

################################################################

sub isconfigured {
  my $self = shift;
  my $build_dir = $self->build_dir();

  return (-e "$build_dir/Makefile");
}


################################################################

=item $model-E<gt>issetup()

Check if the benchmark is set up.

=cut

################################################################

sub issetup {
  my $self = shift;
  my $benchmark = shift;

  if (! $self->isconfigured()) {
    # Model isn't even set up
    return 0;
  }

  # TBD: really check something
  return 1;
}


################################################################

=item $object-E<gt>file_modified()

Return a boolean indicating whether the file has been modified.

Note: this method is only meaningful if used under some file lock.

=cut

################################################################


sub file_modified {
  my $self = shift;

  return $self->{inifile}->file_modified();
}



################################################################

=item $model-E<gt>dump()

Dump the model in ASCII to STDOUT

=cut

################################################################


sub dump {
  my $self = shift;

  $self->Asim::Base::dump();

  #$self->{workbench}->dump();
  $self->{system}->dump();

}


################################################################
#
# Internal error utility function
#
################################################################

#
# Each of these functions checks the arguments to determine
# what to do with the given command or message: 
#     1. run/print it immediately 
#           or 
#     2. return a command to do so?
#
sub _process_command {
    my $cmd = shift;
    my %args = @_;

    my $logfile = $args{"--logfile"} || undef;
    my $getcommand = $args{"--getcommand"} || undef;

    if (defined($logfile)) {
	$cmd .= " > $logfile 2>&1";
    }
    
    if (defined($getcommand)) {
	return $cmd;
    }

    my $status = system($cmd);
    return ! $status;
}

sub _process_warning {
  my $message = shift;
  my %args = @_;
  my $getcommand = $args{"--getcommand"} || undef;

  my $warning = "Asim::Model:: Warning: - $message";
  if (defined($getcommand)) {
      chomp $warning;
      return "echo \"$warning\"";
  }

  print "$warning";
  return 1;
}

sub _process_error {
  my $message = shift;
  my %args = @_;
  my $getcommand = $args{"--getcommand"} || undef;

  my $error = "Asim::Model:: Error: - $message";
  if (defined($getcommand)) {
    chomp $error;
    return "echo \"$error\"";
  }

  print "$error";
  return 1;
}

###########################################################################

=back

=head1 BUGS

TODO - Change name references from "system" to "model".....

"DefaultAttibutes" are treated as a string not a list.
More error checking needed.

Changes to the module hierarchy are not automatically detected as
modifications of the model!

It would be nice if model were derived from module so the root of the
module tree could be treated like any other node of the tree.

Save crashes if there if the modules tree is empty.

issetup() does not really check anything.

=head1 AUTHORS

Joel Emer

=head1 COPYRIGHT

Copyright (c) Compaq Computer Corporation, 2001

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut

###########################################################################

1;
