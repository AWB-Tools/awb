/*
 * Copyright (C) 2002-2006 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

# TODO
#   Implement SaveParam control
#
#   Wrap missing module search to top of model...
#
#   Reasonable sorting of Alternatives
#
#   Check if module replacement is actually doing something
#
#   Progress bar on moduleDB generation.
#   Show modified state with * in caption
#
#   Uniform treatment of model root and rest of modules
#   Control layout of initial box sizes better
#
#
# UGLYNESS
#
#   In many places there are notes about implict dependence on single provides
#   this is a warning that this code will not work if multiple modules of the 
#   same asim-type exist in the model!!!
#
#   The format of the parameter lines in the Info pane are assumed.
#
#
# BUGS
#   Crash after deletion of module files a model being edited is using.
#
#   Search does not Wrap from last element of tree
#
#   Keyboard editting functions get overridden in edit boxes, e.g,
#      so ^D deletes a module when focus is elsewhere!
#

void apm_edit::initPixmaps()
{
    module_missing_pix = apm_edit_load_pixmap("module_missing.png");
    module_pix = apm_edit_load_pixmap("module.png");
    module_default_pix = apm_edit_load_pixmap("module_default.png");
    module_current_pix = apm_edit_load_pixmap("module_current.png");
    submodel_pix = apm_edit_load_pixmap("submodel.png");
}

void apm__edit::init()
{
    initPixmaps();
    use File::Basename;
    use Asim;

    use runlog;

    our $model = undef;
    our $moduleDB = undef;
    our $modelDB = undef;

    $moduleDB = Asim::Module::DB->new(".");
    $modelDB = Asim::Model::DB->new();

    #
    # Column setup
    #
    Model->setSorting(-1,0);

    module_type_col = 0;
    module_flags_col = 1;
    module_implementation_col = 2;
    module_file_col = 3;

    alt_implementation_col = 0;
    alt_file_col = 1;
    alt_sort_col = 2;

    $model = Asim::Model->new();
    $model->name("Unamed project");
    $model->modified(0);

    # Create model tree

    modelShow();
}


# File menu

void apm_edit::fileNew()
{
    our $model;

    while (defined($model) && $model->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit new", 
            "The current model has been modified. Do you want to save it?",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSave()           if ($status == 0);
        $model->modified(0)  if ($status == 1);
        return               if ($status == 2);
    }
    
    $model = Asim::Model->new();
    $model->name("Unamed project");
    $model->modified(0);

    # Build root of model tree...

    Model->clear();

    # Collect initial properties for model

    modelPropertiesAction_activated();

    # Create model tree

    modelShow();
}


void apm_edit::fileOpen()
{
    our $model;

    my $cwd;

    while (defined($model) && $model->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit open", 
            "The current model has been modified. Do you want to save it?",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSave()           if ($status == 0);
        $model->modified(0)  if ($status == 1);
        return               if ($status == 2);
    }

    if (defined($model) && defined($model->filename())) {
        $cwd = dirname($model->filename());
    } else {
        chomp($cwd = `pwd`);
    }

    my $s = shift ||
        Qt::FileDialog::getOpenFileName(
            $cwd,
            "Asim Models (*.apm)",
            this,
            "open asim model dialog",
            "Choose a model to edit" ) ||
        return;;

    my $m = Asim::Model->new($s);
    if (! defined $m) {
        Qt::MessageBox::information(
            this, 
            "apm-edit open", 
            "File open failed");
        return;
    }

    $model = $m;
    $model->modified(0);


    # Build root of model tree...

    modelShow();

    # Display error popup if needed

    my $count = $model->missing_module_count();

    if ($count > 0) {
        my $error;
        my @missing;

        $error  = "Model has $count unsatisfied requirements\n";
        $error .= "(possibly in a submodel)\n"; 
        @missing = $model->missing_packages();

        if (@missing) {
            $error .= "Potentially missing packages: " . join(", ", @missing);
        }

        Qt::MessageBox::information(
            this, 
            "apm-edit open",
            $error);
    }

    statusBar()->message("Model loaded...", 5000);
}




void apm_edit::modelShow()
{
    our $model;

    # Set model name
    
    Name->setText($model->name());

    # Fill in model tree

    Model->clear();

    my $root = Qt::ListViewItem(Model, undef);
    $root->setExpandable(1); 
    $root->setOpen(1); 

    $root->setText(module_type_col, trUtf8($model->provides()));

    my $module = $model->modelroot();

    if (! defined($module)) {

        # There is no root module

        modulePaint($root, undef);
    } else {

        # There IS a root module

        modulePaint($root, $module);

        # Display rest of model...

        buildModel($root, $model->modelroot());
    }

    # Set focus to root of model

    Model->setSelected($root, 1);
    Model->ensureItemVisible($root);

    # Display captions and status

    my $filename = basename($model->filename() || "Unnamed");
    setCaption("$filename - apm-edit");

}


void apm_edit::buildModel()
{
    our $model;

    my $root = shift;
    my $module = shift;
    my $level = shift || 1;
    
    my @requires = reverse($module->requires());
    my @submodules = reverse($module->submodules());

    foreach my $index (0 .. $#requires) {
        my $m = $submodules[$index];
        my $r = $requires[$index];

        if (defined($m)) {
            if ($r ne $m->provides()) {
                print ("Messed up submodules!!! $r and " . $m->provides() . "\n");
            }

            my $next_root = Qt::ListViewItem($root, undef);
  
            $next_root->setText(module_type_col, trUtf8($m->provides()));

            if (! $m->isroot()) {
                # Normal internal node
                $next_root->setExpandable(1); 
                $next_root->setOpen(1); 

		modulePaint($next_root, $m);

                buildModel($next_root, $m, $level+1);
            } else {
                # Submodel leaf node
                $next_root->setExpandable(0); 
                $next_root->setOpen(0); 

		modulePaint($next_root, $m->owner());
            }
        } else {
            statusBar()->message("No implementation specified for module of type $r...", 2000);

            my $empty_item = Qt::ListViewItem($root, undef);
            $empty_item->setExpandable(0); 
            $empty_item->setOpen(0); 

            $empty_item->setText(module_type_col, trUtf8($r));
	    modulePaint($empty_item);
        }
    }

    return 1;
}


void apm_edit::filePrint()
{
    our $model;

    print $model->name() . "\n";
    $model->dump();
}

void apm_edit::fileSave()
{
    our $model;

    fileSaveAs($model->filename());
}

void apm_edit::fileSaveAs()
{
    our $model;

    my $cwd;
    my $status;

    my $s = shift;

    if (defined($model) && defined($model->filename())) {
        $cwd = dirname($model->filename());
    } else {
        chomp($cwd = `pwd`);
    }


    if (!defined($s)) {
        $s = Qt::FileDialog::getSaveFileName(
            $cwd,
            "Asim Models (*.apm)",
            this,
            "save asim model dialog",
            "Choose a filename to save model" ) ||
            return;

        # Force extension to .apm

        if ($s !~ /\.apm$/) {
            $s .= ".apm";
        }

        if (-e $s) {
            my $status = Qt::MessageBox::warning ( 
                this, 
                "apm-edit save", 
                "The model file $s exists. Overwrite it?",
                "&Yes",
                "&No",
                "Cancel",
                0,
                2);

            return if ($status == 1);
            return if ($status == 2);
        }
    }


    $status = $model->save($s);

    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "apm-edit save",
            "File save failed");
        return;
    }

    setCaption(basename($model->filename()) . " - apm-edit");
}

void apm_edit::fileExit()
{
    our $model;

    while ($model->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit exit", 
            "The model has been modified. Do you want to save it first",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSave()           if ($status == 0);
        $model->modified(0)  if ($status == 1);
        return               if ($status == 2);
    }

   this->close();
}


# Edit menu

void apm_edit::editUndo()
{
     Qt::MessageBox::information(
         this, 
         "apm-edit undo",
         "Unimplemented function - editUndo.\n");
}

void apm_edit::editRedo()
{

}

void apm_edit::editCut()
{

}

void apm_edit::editCopy()
{

}

void apm_edit::editPaste()
{

}

void apm_edit::editFind()
{
    Search->setFocus();
}



void apm_edit::editFindMissing()
{
    # TBD: Does not work if no item selected...

    my $start_module = Model->selectedItem() || return;
    my $current_module = $start_module;
    
    while ($current_module = Model_walk($current_module)) {
        if ($current_module->text(module_implementation_col) eq "") {
            Model->setFocus();
            Model->setSelected($current_module, 1);
            Model->ensureItemVisible($current_module);
            return;
        }
    }

    #
    # Nothing found --- wrap search if not a root
    #
    #if ($start_module->text(module_type_col) ne "model") {
    #    editFindMissing(Model->firstChild());
    #    return;
    #}

    #statusBar()->message("All requires satisfied...", 2000);
}


# Model menu


void apm_edit::modelNuke()
{
    our $model;

    my $cmd;

    $cmd = $model->nuke("--getcommand" => 1);

    modelOperation("clean", $cmd);
}


void apm_edit::modelConfigure()
{
    our $model;

    my $cmd;

    $cmd = $model->configure("--getcommand" => 1);

    modelOperation("configure", $cmd);
}


void apm_edit::modelCleanAction_activated()
{
    our $model;

    my $cmd;

    $cmd = $model->clean("--getcommand" => 1);

    modelOperation("clean", $cmd);
}

void apm_edit::modelBuild()
{
    our $model;

    my $cmd;

    $cmd = $model->build("--getcommand" => 1);

    modelOperation("build", $cmd);
}


void apm_edit::modelSetupAction_activated()
{
    our $model;

    my $benchmark;
    my $cmd;

    $benchmark = $model->default_benchmark();
    if (! $benchmark) {
      Qt::MessageBox::information(
	  this, 
          "apm-edit run", 
          "Model has no default benchmark");

      return;
    }

    $cmd = $model->setup($benchmark, "--getcommand" => 1);

    modelOperation("clean", $cmd);
}


void apm_edit::modelRunAction_activated()
{
    our $model;

    my $benchmark;
    my $runopts;

    my $cmd;

    $benchmark = $model->default_benchmark();
    if (! $benchmark) {
      Qt::MessageBox::information(
	  this, 
          "apm-edit run", 
          "Model has no default benchmark");

      return;
    }

    $runopts = $model->default_runopts() || "";
    
    $cmd = $model->run($benchmark, "--runopt" => "$runopts", "--getcommand" => 1);

    modelOperation("run", $cmd);
}



void apm_edit::modelOperation()
{
    my $operation = shift;
    my $cmd = shift;

    our $model;

    my $filename = $model->filename() || return;
    my $w;

    while (defined($model) && $model->modified()) {
      my $status = Qt::MessageBox::warning ( 
	               this, 
                       "apm-edit $operation", 
                       "The current model has been modified.\n"
                       . "Do you want to save it first?",
                       "&Yes",
                       "&No",
                       "Cancel",
		       0,
		       2);

      fileSave()           if ($status == 0);
      last                 if ($status == 1);
      return               if ($status == 2);
    }

    $w = runlog(0, 0, 1);

    $w->run($cmd);
}



void apm_edit::modelAutoBuild_activated()
{
    our $model;

    my $current_module;
    my $previous_autoselect;
    my $count = 0;

    #
    # Save Autoselect old state - then turn it on
    #
    $previous_autoselect = $model->autoselect();
    $model->autoselect(1);

    #
    # Walk tree autoselecting for empty modules
    #
    $current_module = Model->firstChild();

    while (defined($current_module)) {

        if ($current_module->text(module_implementation_col) eq "") {
            Model->setSelected($current_module,1);
        }

        # If still unspecified count it...

        if ($current_module->text(module_implementation_col) eq "") {
            $count++;
        }

        $current_module = Model_walk($current_module);
    }

    #
    # Restore autoselect state
    #
    $model->autoselect($previous_autoselect);

    #
    # Display root of model tree
    #
    $current_module = Model->firstChild();
    Model->setSelected($current_module, 1);
    Model->ensureItemVisible($current_module);

    if ($count > 0) {
        Qt::MessageBox::information(
            this, 
            "apm-edit autobuild", 
            "Model still has $count unsatisfied requirements");
    }        

}



void apm_edit::modelRefresh_activated()
{
    our $moduleDB;
    our $modelDB;

    # Rehash the module DB

    statusBar()->message("Loading moduleDB...");
    print "Loading moduleDB...";
    STDOUT->flush();

    $moduleDB->rehash();

    statusBar()->message("Loading moduleDB...done", 5000);
    print "...done.\n";

    # Rehash the model DB

    statusBar()->message("Loading modelDB...");
    print "Loading modelDB...";
    STDOUT->flush();

    $modelDB->rehash();

    statusBar()->message("Loading modelDB...done", 5000);
    print "...done.\n";

    #
    # TBD - Redraw the model tree too...
    #

    #
    # Force refresh of alternatives pane
    #
    my $item=Model->selectedItem() || return;
    Model_selectionChanged($item)
}



void apm_edit::modelProperties_clicked()
{
    modelPropertiesAction_activated();
}

void apm_edit::modelPropertiesAction_activated()
{
    our $model;

    use apm_edit_properties;

    my $w = apm_edit_properties;

    $w->show();
    $w->exec();

    Name->setText($model->name());
}


# Module menu



void apm_edit::moduleNewAction_activated()
{
    our $model;

    #
    # Find a reasonable directory to start in
    #
    my $item=Model->selectedItem() || return;
    my $provides = $item->text(module_type_col)  || return;
    my $parent = $model->find_module_requiring($provides) || return;

    my $dirname = $parent->base_dir();
    $dirname = $Asim::default_workspace->resolve($dirname);

    #
    # Invoke .awb wizard
    #
    system("cd $dirname; awb-wizard &");

    #
    # Wait for user to resume
    #
    Qt::MessageBox::information(
        this, 
        "apm-edit", 
        "Reload moduleDB and resume editting after creating new module");

    #
    # Refresh module DB
    #
    modelRefresh_activated();
}

#
# Edit the selected module or submodel as appropriate
# TBD: Unify this action with the edit action for 'alternative modules'
#
void apm_edit::moduleEditAction_activated()
{
    my $current_item = Model->selectedItem()
                       || return;

    my $filename = $current_item->text(module_file_col)
                   || return;

    # Handle submodels...

    my $module;

    if ($filename =~ /\.apm$/) {
        $module = Asim::Model->new($filename)   || return;
    } else {
        $module = Asim::Module->new($filename)  || return;
    }

    moduleEditFiles($module);
}

#
# Open a file browser at the directory containing the module
#


void apm_edit::moduleOpening_containing_folderAction_activated()
{
    my $current_item = Model->selectedItem()
                       || return;

    my $filename = $current_item->text(module_file_col)
                   || return;


    moduleOpenContainer($filename);
}


#
# Open a shell at the directory containing the module
#


void apm_edit::moduleShell_at_containing_folderAction_activated()
{
    my $current_item = Model->selectedItem()
                       || return;

    my $filename = $current_item->text(module_file_col)
                   || return;


    moduleShellContainer($filename);
}

#
# Open the notes for this module
#

void apm_edit::moduleNotesAction_activated()
{
    my $current_module = Model->selectedItem()
                         || return;

    my $module_filename = $current_module->text(module_file_col)
                          || return;

    # Handle submodels...

    if ($module_filename =~ /\.apm$/) {
        Qt::MessageBox::information(
            this, 
            "apm-edit Notes", 
            "Apm files currently have no notes");
        return;
    }

    my $module = Asim::Module->new($module_filename)  || return;
    my $dir = $module->base_dir();

    my @files = ();

    foreach my $f ($module->notes()) {
      if (!($f =~ /\.txt$/)) {
        Qt::MessageBox::information(
	    this, 
            "apm-edit Notes", 
            "Non-text files are currently not supported - $f will not be displayed");
        next;
      }
        push(@files, $Asim::default_workspace->resolve("$dir/$f"));
    }

    if (! @files) {
      Qt::MessageBox::information(
	  this, 
          "apm-edit Notes", 
          "This module contains no notes.\n" .
          "Add a notes file and the line '%notes <name>.txt' to the .awb file.");

      return;
    }
      
    Asim::invoke_editor("--background", @files);
}

#
# Edit the .awb file for the selected module
#
void apm_edit::moduleViewAwbAction_activated()
{
    my $current_module = Model->selectedItem()
                         || return;

    my $module_filename = $current_module->text(module_file_col)
                          || return;
    my $module = Asim::Module->new($module_filename)
                 || return;

    my $filename = $Asim::default_workspace->resolve($module->filename());

    Asim::invoke_editor("--background", $filename);
}


#
# Edit the public/private and .awb files for the selected module
#
void apm_edit::moduleViewSourceAction_activated()
{
    moduleEditAction_activated();
}


#
# Selected and add a submodel at the selected node
#

void apm_edit::moduleAddSubmodelAction_activated()
{
    moduleInsertSubmodelAction_activated();
}

void apm_edit::moduleInsertSubmodelAction_activated()
{
    our $model;
    
    my $item = Model->selectedItem() || return;
    my $child_item;

    my $cwd;

    if (defined($model) && defined($model->filename())) {
        $cwd = dirname($model->filename());
    } else {
        chomp($cwd = `pwd`);
    }

    my $file = shift ||
        Qt::FileDialog::getOpenFileName(
            $cwd,
            "Asim Models (*.apm)",
            this,
            "open asim model dialog",
            "Choose a submodel" ) || 
        return;;

    my $submodel = Asim::Model->new($file);
    if (!defined($submodel)) {
        Qt::MessageBox::information(
            this, 
            "apm-edit open submodel", 
            "File open failed");
        return;
    }

    if ($submodel->provides() ne $item->text(module_type_col)) {
        Qt::MessageBox::information(
            this, 
            "apm-edit open submodel", 
            "Submodel of wrong type");
        return;
    }

    ###
    ### TBD: Do not duplicate code from Alternatives_returnPressed_onSubmodel
    ###

    #
    # Replace the module in the model
    #
    $model->smart_add_submodule($submodel);

    #
    # Remove children of module being replaced
    #

    $child_item = $item->firstChild();
    while( defined($child_item) ) {
            my $next_child_item = $child_item->nextSibling();
            $item->takeItem($child_item);
            $child_item = $next_child_item;
        }

    #
    # Add the submodel information in the tree
    #
    $item->setExpandable(0); 
    $item->setOpen(0); 

    $item->setText(module_type_col, trUtf8($submodel->provides()));
    modulePaint($item, $submodel);

    return;
}

#
# Replace the selected node with the module from the
# alternative modules pane
#


void apm_edit::moduleReplaceAction_activated()
{
    moduleInsertModuleAction_activated();
}


void apm_edit::moduleInsertModuleAction_activated()
{
    my $item = Alternatives->selectedItem() || return;

    Alternatives_returnPressed($item);
}



#
# Reinsert selected node as root of model
#
void apm_edit::moduleInsertAsRootAction_activated()
{
    our $model;

    my $item = Model->selectedItem() || return;
    my $provides = $item->text(module_type_col);
    my $filename = $item->text(module_file_col);

    if (! $filename =~ /\.awb$/) {
        Qt::MessageBox::information(
            this, 
            "apm-edit insert module as root", 
            "Only modules can be made the root");
        return;
    }

    my $newroot = $model->find_module_providing($provides);
    $model->modelroot($newroot);

    $item->parent()->takeItem($item);
    Model->clear();
    Model->insertItem($item);
}

#
# Delete the selected node
#
void apm_edit::moduleDeleteAction_activated()
{
    return if (! Model->hasFocus());

    #
    # Determine item to be deleted
    #
    my $item = Model->selectedItem() || return;

    moduleDelete($item);
    return;
}


void apm_edit::moduleDelete( QListViewItem * )
{
    our $model;

    my $item = shift;

    #
    # Find the parent and child modules and remove them
    #    Note: implicit dependence on single provides
    #
    my $child  = $model->find_module_providing($item->text(module_type_col)) || return;
    my $parent = $model->find_module_requiring($item->text(module_type_col));

    $model->remove_submodule($parent, $child);

    #
    # Clear the implementation info from the item and remove all children
    #
    modulePaint($item);

    my $child_item = $item->firstChild();
    while( defined($child_item) ) {
        my $next_child_item = $child_item->nextSibling();
        $item->takeItem($child_item);
        $child_item = $next_child_item;
    }

    return;
}

# Alternatives menu



void apm_edit::alternativesInsertAction_activated()
{
  my $item = Alternatives->selectedItem() || return;

  Alternatives_returnPressed($item);
}


void apm_edit::alternativesEditAction_activated()
{
    my $item   = Alternatives->selectedItem() || return;
    my $module = alternativesModule($item)    || return;

    # Edit all the files belonging to the module

    moduleEditFiles($module, "*");
}



void apm_edit::alternativesOpen_containing_folderAction_activated()
{
    my $item   = Alternatives->selectedItem() || return;
    my $module = alternativesModule($item)    || return;

    moduleOpenContainer($module->filename());
}


void apm_edit::alternativesShell_at_containing_folderAction_activated()
{
    my $item   = Alternatives->selectedItem() || return;
    my $module = alternativesModule($item)    || return;

    moduleShellContainer($module->filename());
}
	        

# Help menu

void apm_edit::helpIndex()
{

}

void apm_edit::helpContents()
{

}

void apm_edit::helpAbout()
{
    use apm_edit_about;

    my $w = apm_edit_about;

    $w->show();
    $w->exec();
}


#
# Search bar
#

void apm_edit::Search_textChanged( const QString & )
{
    my $pattern = lc(shift);
    my $start_module = shift || Model->selectedItem() || return;
    my $current_module = $start_module;

    #
    # Search model tree for pattern
    #
    while (defined($current_module)) {
        if (lc($current_module->text(module_type_col)) =~ /$pattern/) {
            last;
        }

        if (lc($current_module->text(module_implementation_col)) =~ /$pattern/) {
            last;
        }

        $current_module = Model_walk($current_module);
    }


    #
    # Check if search failed
    #
    if (!defined($current_module)) {
        if ($start_module->text(module_type_col) 
            ne Model->firstChild->text(module_type_col)) {
            #
            # Wrap search to root of model
            #
            statusBar()->message("Search wrapping...\n", 2000);
            Search_textChanged($pattern, Model->firstChild());
        } else {
            statusBar()->message("Search failed...\n", 2000);
        }

        return;
    }

    #
    # Search succeeded
    #
    Model->setSelected($current_module, 1);
    Model->ensureItemVisible($current_module);
}


void apm_edit::Search_returnPressed()
{
    my $start;

    $start = Model->selectedItem() || return;
    $start = Model_walk($start)    || return;

    Search_textChanged(Search->text(), $start);
    
}


#
# Name text box
#

void apm_edit::Name_textChanged( const QString & )
{
    our $model;

    $model->name(shift);
}


#
# Model view
#

void apm_edit::Model_selectionChanged( QListViewItem * )
{
    our $model;
    our $moduleDB;
    our $modelDB;

    my $item = shift;

    my $provides = $item->text(module_type_col);

    my $module = undef;
    my $module_name = "";
    my $module_filename = ""; 

    my $module_score = -1;
    my $module_default = undef;

    my $next_root;

    #
    # Get module from model
    #    Note: implicit dependence on single provides
    #
    $module = $model->find_module_providing($provides);
    
    if (defined($module)) {

        if ($model->is_submodel($module)) {
            # This is the root of a submodel

            my $submodel = $module->owner();

            $module_name = $submodel->name();
            $module_filename = $submodel->filename();
        } else {
            # This is an ordinary module

            $module_name = $module->name();
            $module_filename = $module->filename();
        }
    }

    #
    # Display module alternatives list
    #
    my @modules = $moduleDB->find($provides);
    my @models = $modelDB->find($provides);

    Alternatives->clear();

    Info->clear();
    Files->clear();
    Parameters->clear();
    Notes->clear();

    my $selected = undef;

    #
    # Display <<NEW>> module
    #
    #$next_root = Qt::ListViewItem(Alternatives, undef);

    #$next_root->setPixmap(alt_implementation_col, module_pix);
    #$next_root->setText(alt_implementation_col, trUtf8("<<NEW>>"));
    #$next_root->setText(alt_file_col, trUtf8(""));
    

    #
    # Display all the other modules
    #
    foreach my $m (@models, @modules) {
        my $next_root = Qt::ListViewItem(Alternatives, undef);

        $next_root->setText(alt_implementation_col, trUtf8($m->name()));
        $next_root->setText(alt_file_col, trUtf8($m->filename()));

	my $sortname = sprintf("%2d - %s", $m->template(), $m->name());
	$next_root->setText(alt_sort_col, $sortname);

        if ($module_name eq $m->name()) {
            # 
            # This is module in current model
            #   Note: ">> ... <<" form is parsed elsewhere...
            #
            $next_root->setText(alt_implementation_col, 
                                trUtf8(">> " . $m->name() . " <<"));

	    $selected = $next_root;
        }
    
        # TBD: Score for submodels

        my $is_model = $m->filename() =~ /.apm$/;
        my $score = $model->is_default_module($m);

        if ($is_model) {
            $next_root->setPixmap(alt_implementation_col, 
                                  submodel_pix);

        } else {
            if ($score) {
                $next_root->setPixmap(alt_implementation_col, 
                                      module_default_pix);
            } else {
                $next_root->setPixmap(alt_implementation_col,
                                      module_pix);
            }
        }
    
        # Select the default alternative...

        if ($score == $module_score) {
            # Tie - no default
            $module_default = undef;
        }

        if ($score > $module_score) {
            $module_score = $score;
            $module_default = $next_root;
        }
    }

    if (defined($module_default)) {
        $module_default->setPixmap(alt_implementation_col, module_current_pix);
    }

    #
    # Optionally autoselect a module...
    #

    if ( $model->autoselect() && !defined($module)) {
        statusBar()->message("Autoselecting....\n", 2000);
        if (defined($module_default)) {
            statusBar()->message("Autoselecting....selected a module for $provides\n", 2000);
            Alternatives_returnPressed($module_default);
        }
    }

    #
    # Hightlight the appropriate module
    #    NOTE: This is in an eval block since a perlqt bug
    #          sometimes causes a SEGV on these operations
    #

    if (defined($selected)) {
      eval { 
           Alternatives->setCurrentItem($selected);
           Alternatives->setSelected($selected,1);
           Alternatives->ensureItemVisible($selected);
      };

      print $@ if $@;
    }

#
#   Try to sort alphabetically, but with templates at bottom
#   TBD: Figure out why this doesn't work
# 
#    Alternatives->setSorting(2);
#    Alternatives->sort();
}




void apm_edit::Model_contextMenuRequested( QListViewItem *, const QPoint &, int )
{
    my $item = shift;
    my $point = shift;

    moduleMenu->popup($point);
    moduleMenu->exec();

}

void apm_edit::Model_doubleClicked( QListViewItem * )
{
    moduleEditAction_activated()
}



#
# Alternatives view
#

 
void apm_edit::Alternatives_selectionChanged( QListViewItem * )
{
    our $model;

    my $item = shift;

    my $module = undef;
    my $delimiter;   # Used to distinquish if part of real model

    $module = alternativesModule($item) || return;

    #
    # Determine if this item is part of real model
    #

    if ($item->text(alt_implementation_col) =~ ">> .+ <<") {
        $delimiter = "=";
    } else {
        $delimiter = "~";
    }

    #
    # Determine if we can insert this module
    #

    my $template = $module->template();

    alternativesInsertAction->setEnabled(!$template);
    moduleInsertModuleAction->setEnabled(!$template);

    #
    # Fill in information boxes
    #

    Info->clear();
    Files->clear();
    Parameters->clear();
    Notes->clear();

    if (! defined($module)) {
        Info->insertItem("Module not found - refresh probably needed!!!");
        return;
    }

    #
    # Write out description of module
    #
    Info->insertItem("Module:      " . $module->filename());
    Info->insertItem("Name:        " . $module->name());
    Info->insertItem("Description: " . $module->description());
    Info->insertItem("Attributes:  " . join(" ", $module->attributes()));
    Info->insertItem("Provides:    " . $module->provides());
    Info->insertItem("Requires:    " . join(" ", ($module->requires())));

    Files->insertItem("Module:      " . basename($module->filename()));
    Files->insertItem("Notes:       " . join(" ", ($module->notes())));
    Files->insertItem("Makefiles:   " . join(" ", ($module->makefile())));
    Files->insertItem("Scons:       " . join(" ", ($module->scons("*"))));
    Files->insertItem("Public:      " . join(" ", ($module->public())));
    Files->insertItem("Private:     " . join(" ", ($module->private())));

    Parameters->insertItem("Parameters: ");

    #
    # Write out parameters
    #    Note: the format is parsed textually - be careful changing it.
    #

    my $nondefault = 0;

    foreach my $p ($module->parameters()) {
        Parameters->insertItem("   " . $p->description() . ($p->dynamic()?" (dynamic)":""));


	my $name = $p->name();
	my $value = $p->value();
	my $default = $p->default();

        Parameters->insertItem("       $name$delimiter$value [$default]");

	$nondefault |= ($value ne $default);
    }

    if ($nondefault) {
        alternativesTabWidget->changeTab(ParametersPage, trUtf8("Parameters *"));
    } else {
        alternativesTabWidget->changeTab(ParametersPage, trUtf8("Parameters"));
    }
        
    #
    # Get notes file and fill into Notes pane
    #
    my $notes;
    my $dir;

    foreach my $n ($module->notes()) {
      if ($n =~ /\.txt$/) {
        $dir = $module->base_dir();
	$notes = Asim::resolve("$dir/$n");
      }
    }

    if (defined($notes) && -r $notes) {

        alternativesTabWidget->changeTab(NotesPage, trUtf8("Notes *"));

        open(NOTES, $notes);

        while (<NOTES>) {
            chomp;
            Notes->insertItem($_);
	}

        close(NOTES);
    } else {
        alternativesTabWidget->changeTab(NotesPage, trUtf8("Notes"));
    }

    Info->setCurrentItem(0);
    Files->setCurrentItem(0);
    Parameters->setCurrentItem(0);
    Notes->setCurrentItem(0);
}
 
 

void apm_edit::Alternatives_contextMenuRequested( QListViewItem *, const QPoint &, int )
{
    my $item = shift;
    my $point = shift;

    alternativesMenu->popup($point);
    alternativesMenu->exec();

}

#
# Replace selected module into parent module 
# that requires a module of this type.
#

void apm_edit::Alternatives_doubleClicked( QListViewItem * )
{
    my $item = shift;

    Alternatives_returnPressed($item);

    #
    # Move to next unsatisfied requirement
    # or root of model if nothing more to do
    #
    my $model_item = Model->selectedItem();


    while (defined($model_item)) {
        if ($model_item->text(module_implementation_col) eq "") {
            last;
        }
        $model_item = Model_walk($model_item);
    }


    if (!defined($model_item)) {
        return;
    }

    Model->setSelected($model_item, 1);
    Model->ensureItemVisible($model_item);
}

 
void apm_edit::Alternatives_returnPressed( QListViewItem * )
{
    our $model;

    my $alt_item = shift;

    my $name = $alt_item->text(alt_implementation_col) || return;
    my $filename = $alt_item->text(alt_file_col) || return;

    if ($name eq "<<NEW>>") {
        moduleNewAction_activated();
        return;
    }

    if ( $filename =~ /\.awb$/) {
        Alternatives_returnPressed_onModule($alt_item);
    }

    if ( $filename =~ /\.apm$/) {
        Alternatives_returnPressed_onSubmodel($alt_item);
    }

    Model_selectionChanged(Model->selectedItem());
}


void apm_edit::Alternatives_returnPressed_onModule( QListViewItem * )
{
    our $model;

    my $alt_item = shift;

    my $item = Model->selectedItem() || die("No item was selected in model");
    my $child_item;

    my $module_name = $alt_item->text(alt_implementation_col);
    my $module_filename = $alt_item->text(alt_file_col) || return;

    my $module = Asim::Module->new($module_filename) || return;

    #
    # TBD: Refactor this code with submodel insertion routines
    #

    if ($module->template()) {
        Qt::MessageBox::information(
	    this, 
            "apm-edit insert", 
            "Cannot insert a template module");
	return;
    }

    #
    # Add the new child module into the tree
    #

    $model->smart_add_submodule($module);
        
    #
    # Check for non-default parameters
    #
    moduleInsertParams($module);
 
    #
    # Remove children of module being replaced
    #

    $child_item = $item->firstChild();
    while( defined($child_item) ) {
            my $next_child_item = $child_item->nextSibling();
            $item->takeItem($child_item);
            $child_item = $next_child_item;
        }

    #
    # Display updated item
    #

    $item->setExpandable(1); 
    $item->setOpen(1); 

    $item->setText(module_type_col, trUtf8($module->provides()));
    modulePaint($item, $module);


    #
    # Display rest of model...
    #

    buildModel($item, $module);

    Model->setSelected($item, 0);
    Model->setSelected($item, 1);
}

void apm_edit::Alternatives_returnPressed_onSubmodel( QListViewItem * )
{
    our $model;

    my $alt_item = shift;

    my $item = Model->selectedItem() || die("No item was selected in model");
    my $child_item;

    my $model_name = $alt_item->text(alt_implementation_col);
    my $model_filename = $alt_item->text(alt_file_col) || return;

    my $submodel = Asim::Model->new($model_filename) || return;

    ###
    ### TBD: Do not duplicate code from moduleInsertSubmodelAction_activated
    ###

    #
    # Replace the module in the model
    #

    $model->smart_add_submodule($submodel);

    #
    # Check for non-default parameters
    #
    moduleInsertParams($submodel);
 
    #
    # Remove children of module being replaced
    #

    $child_item = $item->firstChild();
    while( defined($child_item) ) {
            my $next_child_item = $child_item->nextSibling();
            $item->takeItem($child_item);
            $child_item = $next_child_item;
        }

    #
    # Add the submodel information in the tree
    #
    $item->setExpandable(0); 
    $item->setOpen(0); 

    $item->setPixmap(module_type_col, submodel_pix);
    modulePaint($item, $submodel);

    return;
}

#
# Utility functions for model tree
#
void apm_edit::modulePaint()
{
    my $item = shift;
    my $module = shift;

    if (! defined($module)) {
        $item->setPixmap(module_type_col, module_missing_pix);
        $item->setText(module_flags_col, trUtf8("     "));
        $item->setText(module_implementation_col, trUtf8(""));
        $item->setText(module_file_col, trUtf8(""));
	return;
    }

    our $model;

    if (ref($module) eq "Asim::Model") {
        $item->setPixmap(module_type_col, submodel_pix);
    } else {
        if ($model->is_default_module($module)) {
            $item->setPixmap(module_type_col, module_default_pix);
        } else {
            $item->setPixmap(module_type_col, module_pix);
        }
    }

    $item->setText(module_implementation_col, trUtf8($module->name()));

    my $flags = moduleFlags($module);

    $item->setText(module_flags_col, trUtf8($flags));
    $item->setText(module_file_col, trUtf8($module->filename()));
   
}


void apm_edit::moduleFlags()
{
    my $module = shift;

    my $flags = "  "; 

    $flags .= moduleCheckParams($module, 0)?"p":" ";
    $flags .= ($module->notes())?"n":" ";

    $flags .= "  ";

    return $flags;
}



#
# Utility functions for checking and defaulting parameters
#

void apm_edit::moduleInsertParams()
{
   my $module = shift;

   if (moduleCheckParams($module, 1)) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit insert", 
            "The newly inserted module (or its children) have non-default parameter values\n"
          . "Do you want to change them ALL to the default values?",
            "&Yes",
            "&No",
            "",
            0,
            1);

        if ($status == 0) {
            moduleDefaultParams($module);
        }
    }
}

#
# Check a module tree for any module has non-default parameter values
# Note: only examines global parameters of submodels
#

void apm_edit::moduleCheckParams()
{
    my $module = shift || return;
    my $recurse = shift || 0;

    our $model;

    if ($model->is_submodel($module)) {
        $module = $module->owner();
    }

    foreach my $p ($module->parameters()) {
        my $v = $p->value();

        if (defined($v) && $p->value() ne $p->default()) {
           return 1;
        }
    }

    if (! $recurse) {
        return 0;
    }

    foreach my $s ($module->submodules()) {
        my $m = moduleCheckParams($s, 1);

        return 1 if $m;
    }

    return 0;
}

#
# Set all parameters in a module tree to their default values
# Note: only sets global parameters of submodels
#

void apm_edit::moduleDefaultParams()
{
    my $module = shift || return;

    our $model;

    if ($model->is_submodel($module)) {
        $module = $module->owner();
    }

    foreach my $p ($module->parameters()) {
        $p->value($p->default());
    }

    foreach my $s ($module->submodules()) {
        moduleDefaultParams($s);
    }
}

#
# Module information box
#

# Open button

void apm_edit::alternativesEdit_clicked()
{
    my $item   = Files->selectedItem() || return;

    Files_doubleClicked($item);
}


# Double click on line in Files box

void apm_edit::Files_doubleClicked( QListBoxItem * )
{
    my $fileitem = shift                      || return;
    my $fileline = $fileitem->text();

    # Get module from Alternatives box

    my $item   = Alternatives->selectedItem() || return;
    my $module = alternativesModule($item)    || return;

    $fileline =~ /^.*: *(.*)/;
    my $files = $1;
   
    moduleEditFiles($module, $files);
}




void apm_edit::Parameters_selectionChanged( QListBoxItem * )
{
    my $item = shift;
    my $line = $item->text();

    if ($line =~ "^ *([^=]+)=([^ ]+) ") {
        ParamName->setText($1);
        ParamValue->setEnabled(1);
        ParamValue->setText($2);
        ParamChange->setEnabled(1);
    } else {
        ParamName->setText("");
        ParamValue->setEnabled(0);
        ParamValue->setText("");
        ParamChange->setEnabled(0); 
   }
}



void apm_edit::ParamValue_returnPressed()
{
    ParamChange_clicked();
}

void apm_edit::ParamChange_clicked()
{
    our $model;

    my $itemno = Parameters->currentItem();
    my $line = Parameters->text($itemno);

    #
    # Parse out information on parameter
    #    Note: implicit dependence on format of line
    #
    $line =~ "^ *([^=]+)=([^ ]+) [[](.+)[]]";

    my $name = $1;
    if (!defined($name)) {
        die("Illegally formatted parameter line");
    }

    my $default = $3;
    if (!defined($default)) {
        print("Illegally formatted parameter line");
        $default = $2;
    }       

    my $value = ParamValue->text();
    if ($value eq "") {
        $value = $3;
    }
    
    #
    # Update information on parameter
    #    Note: implicit dependence on format of line
    #
    Parameters->changeItem("       $name=$value [$default]", $itemno);


    #
    # Update module itself
    #    Note: implicit dependence on single provides
    #
    my $module_item = Model->selectedItem();
    my $provides = $module_item->text(module_type_col);
    my $module = $model->find_module_providing($provides);

    #
    # Check if module is really the root of a submodel.
    # In that case, we want to change the submodels's 'global' parameter,
    # which is in the model that is the owner() of the module.
    #
    if ($model->is_submodel($module)) {
        $module = $module->owner();
    }

    $module->setparameter($name, $value);

    $model->modified(1);

    modulePaint($module_item, $module);
}





#
#
#

void apm_edit::moduleEditFiles()
{
    my $module = shift;
    my $filelist = shift || "*";

    # Handle submodels...

    my $filename = $module->filename();

    if ($filename =~ /\.apm$/) {
        $filename = $Asim::default_workspace->resolve($filename);
        system("apm-edit $filename &");
        return;
    }

    # It's really a module

    my $dir = $module->base_dir();

    my @files = ();
    my @module_files = ();

    if ($filelist eq "*") {
        push(@files, $Asim::default_workspace->resolve($module->filename()));
	push(@module_files, $module->inventory());
    } else {
	push(@module_files, split(' ', $filelist));
    }

    foreach my $f (@module_files) {
      my $resolved_f = Asim::resolve("$dir/$f");

      if (!defined($resolved_f)) {
        print "apm_edit: Skipping non-existent file $f\n";
        next;
      }

      push(@files, $resolved_f);
    }

    Asim::invoke_editor("--background", @files);
}

void apm_edit::moduleOpenContainer()
{
    my $filename = shift;

    use File::Basename;

    my $dirname = dirname(Asim::resolve($filename));

    system("xdg-open $dirname");
}

void apm_edit::moduleShellContainer()
{
    my $filename = shift;

    use File::Basename;

    my $dirname = dirname(Asim::resolve($filename));

    system("gnome-terminal --working-directory $dirname");
}



void apm_edit::alternativesModule()
{
    my $item = shift || return;

    our $model;

    #
    # Open the selected module, 
    #    either from the current model, or the .awb file itself
    #
    #    Note: implicit dependence on single provides
    #
    my $module;
    my $name = $item->text(alt_implementation_col);

    if ($name =~ ">> .+ <<") {
        # From current model

        my $module_item = Model->selectedItem() || return;
        my $provides = $module_item->text(module_type_col);

        $module = $model->find_module_providing($provides);
        if ($model->is_submodel($module)) {
            # This is a submodel...

            $module = $module->owner();
        }
    } else {
        # From .apm/.awb file

        my $filename = $item->text(alt_file_col);

        if ($filename =~ /.awb/) {
            $module = Asim::Module->new($filename);
        } elsif ($filename =~ /.apm/) {
            $module = Asim::Model->new($filename);
        }
    }

    return $module;
}


int apm_edit::Model_find()
{
    # Unused function

    return 0;
}




int * apm_edit::Model_walk()
{
    my $current = shift;
    my $next;

    $next = $current->firstChild();
    if (defined($next)) {
        return $next;
    }

    while (defined($current)) {
        $next = $current->nextSibling();
        if (defined($next)) {
            return $next;
        }
        $current = $current->parent();
    }

    return undef;
}


