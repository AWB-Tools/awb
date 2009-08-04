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
#   The contents of various columns (specified by number) are assumed in the code.
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

    Model->setSorting(-1,0);

    fileNew();
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

    Name->setText("New Asim Model");
    Description->setText("Add a model description here");
    Attributes->setText("");

    $model->modified(0);

    # Build root of model tree...

    Model->clear();

    my $root = Qt::ListViewItem(Model, undef);
    $root->setExpandable(1); 
    $root->setOpen(1); 
    $root->setText(0, trUtf8($model->provides()));
    $root->setPixmap(0, module_missing_pix);
    $root->setText(1, trUtf8(""));
    $root->setText(2, trUtf8(""));

    Model->setSelected($root, 1);
    Model->ensureItemVisible($root);

    setCaption("Untitled - apm-edit");
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

    Name->setText($model->name());
    Description->setText($model->description());
    Attributes->setText(join(" ", $model->default_attributes()));

    $model->modified(0);

    # Build root of model tree...

    Model->clear();

    my $root = Qt::ListViewItem(Model, undef);

    $root->setExpandable(1); 
    $root->setOpen(1); 
    $root->setText(0, trUtf8($model->provides()));

    my $module = $model->modelroot();

    if (! defined($module)) {

        # There is no root module

        $root->setPixmap(0, module_missing_pix);
        $root->setText(1, trUtf8(""));
        $root->setText(2, trUtf8(""));
    } else {

        # There IS a root module

        if ($model->is_default_module($module)) {
            $root->setPixmap(0, module_default_pix);
        } else {
            $root->setPixmap(0, module_pix);
        }

        $root->setText(1, trUtf8($module->name()));
        $root->setText(2, trUtf8($module->filename()));

        # Display rest of model...

        buildModel($root, $model->modelroot());
    }

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

    # Set focus to root of model

    Model->setSelected($root, 1);
    Model->ensureItemVisible($root);

    # Display captions and status

    setCaption(basename($model->filename()) . " - apm-edit");
    statusBar()->message("Model loaded...", 5000);
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
  
            $next_root->setText(0, trUtf8($m->provides()));

            if (! $m->isroot()) {
                # Normal internal node
                $next_root->setExpandable(1); 
                $next_root->setOpen(1); 

                if ($model->is_default_module($m)) {
                    $next_root->setPixmap(0, module_default_pix);
                } else {
                    $next_root->setPixmap(0, module_pix);
                }

                $next_root->setText(1, trUtf8($m->name()));
                $next_root->setText(2, trUtf8($m->filename()));
                buildModel($next_root, $m, $level+1);
            } else {
                # Submodel leaf node
                my $owner = $m->owner();

                $next_root->setExpandable(0); 
                $next_root->setOpen(0); 

                $next_root->setPixmap(0, submodel_pix);
                $next_root->setText(1, trUtf8($owner->name()));
                $next_root->setText(2, trUtf8($owner->filename()));
            }
        } else {
            statusBar()->message("No implementation specified for module of type $r...", 2000);

            my $empty_item = Qt::ListViewItem($root, undef);
            $empty_item->setExpandable(0); 
            $empty_item->setOpen(0); 
            $empty_item->setText(0, trUtf8($r));
            $empty_item->setPixmap(0, module_missing_pix);
            $empty_item->setText(1, trUtf8(""));
            $empty_item->setText(2, trUtf8(""));
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
        if ($current_module->text(1) eq "") {
            Model->setFocus();
            Model->setSelected($current_module, 1);
            Model->ensureItemVisible($current_module);
            return;
        }
    }

    #
    # Nothing found --- wrap search if not a root
    #
    #if ($start_module->text(0) ne "model") {
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
    my $cmd;

    $benchmark = $model->default_benchmark();
    if (! $benchmark) {
      Qt::MessageBox::information(
	  this, 
          "apm-edit run", 
          "Model has no default benchmark");

      return;
    }

    $cmd = $model->run($benchmark, "--getcommand" => 1);

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
    $previous_autoselect = Autoselect->state();
    Autoselect->setState(1);

    #
    # Walk tree autoselecting for empty modules
    #
    $current_module = Model->firstChild();

    while (defined($current_module)) {

        if ($current_module->text(1) eq "") {
            Model->setSelected($current_module,1);
        }

        # If still unspecified count it...

        if ($current_module->text(1) eq "") {
            $count++;
        }

        $current_module = Model_walk($current_module);
    }

    #
    # Restore autoselect state
    #
    Autoselect->setState($previous_autoselect);

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


# Module menu



void apm_edit::moduleNewAction_activated()
{
    our $model;

    #
    # Find a reasonable directory to start in
    #
    my $item=Model->selectedItem() || return;
    my $provides = $item->text(0)  || return;
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
#
void apm_edit::moduleEditAction_activated()
{
    my $current_item = Model->selectedItem();

    my $filename = $current_item->text(2);

    # Handle submodels...

    if ($filename =~ /\.apm$/) {
        $filename = $Asim::default_workspace->resolve($filename);
        system("apm-edit $filename &");
        return;
    }

    my $current_module = Model->selectedItem()        || return;

    my $module_filename = $current_module->text(2)    || return;
    my $module = Asim::Module->new($module_filename)  || return;
    my $dir = $module->base_dir();

    my @files = ();

    push(@files, Asim::resolve($module->filename()));

    foreach my $f ($module->inventory()) {
      my $resolved_f = Asim::resolve("$dir/$f");

      if (!defined($resolved_f)) {
	print "apm_edit: Skipping non-existent file $f\n";
	next;
      }

      push(@files, $resolved_f);
    }

    Asim::invoke_editor("--background", @files);

}

#
# Open the notes for this module
#

void apm_edit::moduleNotesAction_activated()
{
    my $current_module = Model->selectedItem()        || return;

    my $module_filename = $current_module->text(2)    || return;

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
    my $current_module = Model->selectedItem()        || return;

    my $module_filename = $current_module->text(2)    || return;
    my $module = Asim::Module->new($module_filename)  || return;
    my $filename = $Asim::default_workspace->resolve($module->filename());

    Asim::invoke_editor("--background", $filename);
}


#
# Edit the public/private and .awb files for the selected module
#
void apm_edit::moduleViewSourceAction_activated()
{
    my $current_module = Model->selectedItem()        || return;

    my $module_filename = $current_module->text(2)    || return;
    my $module = Asim::Module->new($module_filename)  || return;
    my $dir = $module->base_dir();

    my @files = ();

    push(@files, $Asim::default_workspace->resolve($module->filename()));

    foreach my $f ($module->public()) {
        push(@files, $Asim::default_workspace->resolve("$dir/$f"));
    }

    foreach my $f ($module->private()) {
        push(@files, $Asim::default_workspace->resolve("$dir/$f"));
    }

    Asim::invoke_editor("--background", @files);

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

    if ($submodel->provides() ne $item->text(0)) {
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

    $item->setPixmap(0, submodel_pix);
    $item->setText(0, trUtf8($submodel->provides()));
    $item->setText(1, trUtf8($submodel->name()));
    $item->setText(2, trUtf8($submodel->filename()));

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
    my $provides = $item->text(0);
    my $filename = $item->text(2);

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
    my $child  = $model->find_module_providing($item->text(0)) || return;
    my $parent = $model->find_module_requiring($item->text(0));

    $model->remove_submodule($parent, $child);

    #
    # Clear the implementation info from the item and remove all children
    #
    $item->setPixmap(0, module_missing_pix);
    $item->setText(1, trUtf8(""));
    $item->setText(2, trUtf8(""));

    my $child_item = $item->firstChild();
    while( defined($child_item) ) {
        my $next_child_item = $child_item->nextSibling();
        $item->takeItem($child_item);
        $child_item = $next_child_item;
    }

    return;
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
        if (lc($current_module->text(0)) =~ /$pattern/) {
            last;
        }

        if (lc($current_module->text(1)) =~ /$pattern/) {
            last;
        }

        $current_module = Model_walk($current_module);
    }


    #
    # Check if search failed
    #
    if (!defined($current_module)) {
        if ($start_module->text(0) ne Model->firstChild->text(0)) {
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
# Name, Description, Attributes text boxes
#

void apm_edit::Name_textChanged( const QString & )
{
    our $model;

    $model->name(shift);
}


void apm_edit::Description_textChanged( const QString & )
{
    our $model;

    $model->description(shift);
}


void apm_edit::Attributes_textChanged( const QString & )
{
    our $model;

    $model->default_attributes(shift);
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

    my $provides = $item->text(0);

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

        if ($module->isroot() && ($module != $model->modelroot())) {
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

    #
    # Display <<NEW>> module
    #
    #$next_root = Qt::ListViewItem(Alternatives, undef);

    #$next_root->setText(0, trUtf8("<<NEW>>"));
    #$next_root->setText(1, trUtf8(""));
    #$next_root->setPixmap(0, module_pix);
    

    #
    # Display all the other modules
    #
    foreach my $m (@models, @modules) {
        my $next_root = Qt::ListViewItem(Alternatives, undef);

        $next_root->setText(0, trUtf8($m->name()));
        $next_root->setText(1, trUtf8($m->filename()));

        if ($module_name eq $m->name()) {
            # 
            # This is module in current model
            #   Note: ">> ... <<" form is parsed elsewhere...
            #
            $next_root->setText(0, trUtf8(">> " . $m->name() . " <<"));
            Alternatives->setSelected($next_root,1);
            Alternatives->ensureItemVisible($next_root);
        }
    
        # TBD: Score for submodels

        my $is_model = $m->filename() =~ /.apm$/;
        my $score = $model->is_default_module($m);

        if ($is_model) {
            $next_root->setPixmap(0, submodel_pix);

        } else {
            if ($score) {
                $next_root->setPixmap(0, module_default_pix);
            } else {
                $next_root->setPixmap(0, module_pix);
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
        $module_default->setPixmap(0, module_current_pix);
    }

    #
    # Optionally autoselect a module...
    #

    if ( Autoselect->state() && !defined($module)) {
        statusBar()->message("Autoselecting....\n", 2000);
        if (defined($module_default)) {
            statusBar()->message("Autoselecting....selected a module for $provides\n", 2000);
            Alternatives_returnPressed($module_default);
        }
    }
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

    #
    # Open the selected module, 
    #    either from the current model, or the .awb file itself
    #
    #    Note: implicit dependence on single provides
    #
    my $name = $item->text(0);

    if ($name =~ ">> .+ <<") {
        # From current model

        my $module_item = Model->selectedItem();
        my $provides = $module_item->text(0);

        $module = $model->find_module_providing($provides);
        if ($module->isroot() && ($module != $model->modelroot())) {
            # This is a submodel...

            $module = $module->owner();
        }

        $delimiter = "=";
    } else {
        # From .apm/.awb file

        my $filename = $item->text(1);

        if ($filename =~ /.awb/) {
            $module = Asim::Module->new($filename);
        } elsif ($filename =~ /.apm/) {
            $module = Asim::Model->new($filename);
        }
        $delimiter = "~";
    }

    Info->clear();

    if (! defined($module)) {
        Info->insertItem("Module not found - refresh probably needed!!!");
        return;
    }

    #
    # Write out description of module
    #
    Info->insertItem("Name:        " . $module->name());
    Info->insertItem("Description: " . $module->description());
    Info->insertItem("Attributes:  " . join(" ", $module->attributes()));
    Info->insertItem("Provides:    " . $module->provides());
    Info->insertItem("Requires:    " . join(" ", ($module->requires())));
    Info->insertItem("Filename:    " . $module->filename());
    Info->insertItem("Notes:       " . join(" ", ($module->notes())));
    Info->insertItem("Makefiles:   " . join(" ", ($module->makefile())));
    Info->insertItem("Scons:       " . join(" ", ($module->scons("*"))));
    Info->insertItem("Public:      " . join(" ", ($module->public())));
    Info->insertItem("Private:     " . join(" ", ($module->private())));
    Info->insertItem("Parameters: ");

    #
    # Write out parameters
    #    Note: the format is parsed textually - be careful changing it.
    #
    foreach my $p ($module->parameters()) {
        Info->insertItem("   " . $p->description() . ($p->dynamic()?" (dynamic)":""));
        Info->insertItem("       " . $p->name() . $delimiter .
                     $p->value() . " [" . $p->default() . "]");
    }
    
    Info->setCurrentItem(0);
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
        if ($model_item->text(1) eq "") {
            last;
        }
        $model_item = Model_walk($model_item);
    }


    if (!defined($model_item)) {
        $model_item = Model->firstChild();
    }

    Model->setSelected($model_item, 1);
    Model->ensureItemVisible($model_item);
}

 
void apm_edit::Alternatives_returnPressed( QListViewItem * )
{
    our $model;

    my $alt_item = shift;

    my $name = $alt_item->text(0) || return;
    my $filename = $alt_item->text(1) || return;

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

}


void apm_edit::Alternatives_returnPressed_onModule( QListViewItem * )
{
    our $model;

    my $alt_item = shift;

    my $item = Model->selectedItem() || die("No item was selected in model");
    my $child_item;

    my $module_name = $alt_item->text(0);
    my $module_filename = $alt_item->text(1) || return;

    my $module = Asim::Module->new($module_filename) || return;

    #
    # TBD: Refactor this code with submodel insertion routines
    #

    # Add the new child module into the tree
    #

    $model->smart_add_submodule($module);
        
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

    if ($model->is_default_module($module)) {
        $item->setPixmap(0, module_default_pix);
    } else {
        $item->setPixmap(0, module_pix);
    }

    $item->setText(0, trUtf8($module->provides()));
    $item->setText(1, trUtf8($module->name()));
    $item->setText(2, trUtf8($module->filename()));

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

    my $model_name = $alt_item->text(0);
    my $model_filename = $alt_item->text(1) || return;

    my $submodel = Asim::Model->new($model_filename) || return;

    ###
    ### TBD: Do not duplicate code from moduleInsertSubmodelAction_activated
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

    $item->setPixmap(0, submodel_pix);
    $item->setText(0, trUtf8($submodel->provides()));
    $item->setText(1, trUtf8($submodel->name()));
    $item->setText(2, trUtf8($submodel->filename()));

    return;
}


#
# Module information box
#

void apm_edit::Info_selectionChanged( QListBoxItem * )
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

    my $itemno = Info->currentItem();
    my $line = Info->text($itemno);

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
    Info->changeItem("       $name=$value [$default]", $itemno);


    #
    # Update module itself
    #    Note: implicit dependence on single provides
    #
    my $module_item = Model->selectedItem();
    my $provides = $module_item->text(0);
    my $module = $model->find_module_providing($provides);

    #
    # Check if module is really the root of a submodel.
    # In that case, we want to change the submodels's 'global' parameter,
    # which is in the model that is the owner() of the module.
    #
    if ($module->isroot() && ($module != $model->modelroot())) {
      $module = $module->owner();
    }

    $module->setparameter($name, $value);
    $model->modified(1);
}





#
#
#

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


