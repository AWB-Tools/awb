package apm_edit;

use strict;
use warnings;

use runlog;

use QtCore4;
use QtGui4;
use QtCore4::isa qw( Qt::MainWindow);
use Qt3Support4;
use QtCore4::slots
    initPixmaps => [],
    init => [],
    fileNew => [],
    fileOpen => [],
    buildModel => [],
    filePrint => [],
    fileSave => [],
    fileSaveAs => [],
    fileExit => [],
    editUndo => [],
    editRedo => [],
    editCut => [],
    editCopy => [],
    editPaste => [],
    editFind => [],
    editFindMissing => [],
    viewOpen_build_folderAction_activated => [],
    viewShell_at_build_folderAction_activated => [],
    viewOpen_run_folderAction_activated => [],
    viewShell_at_run_folderAction_activated => [],
    modelNuke => [],
    modelConfigure => [],
    modelCleanAction_activated => [],
    modelBuild => [],
    modelSetupAction_activated => [],
    modelRunAction_activated => [],
    modelAutoBuild_activated => [],
    modelRefresh_activated => [],
    modelProperties_clicked => [],
    modelPropertiesAction_activated => [],
    moduleNewAction_activated => [],
    moduleEditAction_activated => [],
    moduleOpening_containing_folderAction_activated => [],
    moduleShell_at_containing_folderAction_activated => [],
    moduleNotesAction_activated => [],
    moduleViewAwbAction_activated => [],
    moduleViewSourceAction_activated => [],
    moduleAddSubmodelAction_activated => [],
    moduleInsertSubmodelAction_activated => [],
    moduleReplaceAction_activated => [],
    moduleInsertModuleAction_activated => [],
    moduleInsertAsRootAction_activated => [],
    moduleDeleteAction_activated => [],
    moduleDelete => ['QTreeWidgetItem*'],
    alternativesInsertAction_activated => [],
    alternativesEditAction_activated => [],
    alternativesOpen_containing_folderAction_activated => [],
    alternativesShell_at_containing_folderAction_activated => [],
    helpIndex => [],
    helpContents => [],
    helpAbout => [],
    Search_textChanged => ['const QString&'],
    Search_returnPressed => [],
    Name_textChanged => ['const QString&'],
    Model_selectionChanged => ['QTreeWidgetItem*'],
    Model_doubleClicked => ['QTreeWidgetItem*'],
    Alternatives_selectionChanged => ['QTreeWidgetItem*'],
    Alternatives_doubleClicked => ['QTreeWidgetItem*'],
    Alternatives_returnPressed => ['QTreeWidgetItem*'],
    Alternatives_returnPressed_onModule => ['QTreeWIdgetItem*'],
    Alternatives_returnPressed_onSubmodel => ['QTreeWidgetItem*'],
    alternativesEdit_clicked => [],
    Files_doubleClicked => ['QListWidgetItem*'],
    Parameters_selectionChanged => [],
    ParamValue_returnPressed => [],
    ParamChange_clicked => [],
    Model_find => [],
    Model_walk => [],
    pushBusyCursor => [],
    popBusyCursor => [];
 
use constant {module_type_col => 0, module_flags_col => 1, module_implementation_col => 2, module_file_col => 3};

use constant {alt_implementation_col => 0, alt_file_col => 1, alt_sort_col => 2};

sub ui() 
{
    this->{ui};
}

sub module_missing_pix 
{
    return this->{module_missing_pix};
}

sub module_pix 
{
    return this->{module_pix}
    
}

sub module_default_pix 
{   
    return this->{module_default_pix};
}
 
sub module_current_pix 
{
    return this->{module_current_pix};
} 

sub submodel_pix
{
    return this->{submodel_pix};
}

sub NEW 
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Apm_edit->setupUi(this);
}

sub apm_edit_load_pixmap
{
    my $pix = Qt::Pixmap(shift);
    
    return $pix;
}

sub initPixmaps
{
    this->{module_missing_pix} = apm_edit_load_pixmap(":/images/module_missing.png");
    this->{module_pix} = apm_edit_load_pixmap(":/images/module.png");
    this->{module_default_pix} = apm_edit_load_pixmap(":/images/module_default.png");
    this->{module_current_pix} = apm_edit_load_pixmap(":/images/module_current.png");
    this->{submodel_pix} = apm_edit_load_pixmap(":/images/submodel.png");
}

sub init
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
    
    $model = Asim::Model->new();
    $model->name("Unamed project");
    $model->modified(0);

    # Create model tree

    modelShow();

    # create context menus
    
    createContextMenus();

    # Create tool bars

    createToolBars();

}

sub createToolBars 
{
    ui()->toolBar()->addAction(ui()->fileNewAction());
    ui()->toolBar()->addAction(ui()->fileOpenAction());
    ui()->toolBar()->addAction(ui()->fileSaveAction());
    ui()->toolBar()->addAction(ui()->filePrintAction());
    ui()->toolBar()->addWidget(ui()->searchLabel());
    ui()->toolBar()->addWidget(ui()->search());
    
}
sub createContextMenus
{
    # Alternatives
    ui()->alternatives()->addActions(ui()->alternativesMenu()->actions());

    # Model context menu
    ui()->model()->addActions(ui()->moduleMenu()->actions());
}

# File menu

sub fileNew
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

    ui()->model()->clear();

    # Collect initial properties for model

    modelPropertiesAction_activated();

    modelShow();
}


sub fileOpen
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

    my $filename = shift ||
        Qt::FileDialog::getOpenFileName(
            this,
            "Choose a model to edit",
            $cwd,
            "Asim Models (*.apm)") ||
        return;
    
    my $check_health = shift;
    # Can't use shift || 1 for GetOpt boolean
    $check_health = 1 if (!defined($check_health));

    this->pushBusyCursor();
    my $m = Asim::Model->new($filename);
    this->popBusyCursor();
    if (! defined $m) {
        Qt::MessageBox::information(
            this, 
            "apm-edit open", 
            "File open failed");
        return;
    }

    $model = $m;

    # Build root of model tree...

    modelShow();
    
    ui()->statusBar()->showMessage("Model loaded...", 5000);
    
    return if (!$check_health);

    #
    # Perform a comprehensive check of the health of the model
    # and its submodels, alerting the user to any problems.
    # This can be disabled with the --nocheck-health option.
    #

    #
    # Open erroneous submodels if needed
    # This returns models recursively in a "depth-first" manner,
    # which ensures that problems are reported at most once.
    #

    my @broken_submodels = $model->get_broken_submodels();    
    my $open_broken = 0;

    if ($#broken_submodels > -1) {
    
      my $status = Qt::MessageBox::warning ( 
          this, 
          "apm-edit open", 
          "The current model uses submodels that appear to be broken.\nDo you wish to open these submodels?",
          "&Yes",
          "&No",
          "",
          0,
          1);

       if ($status == 0) {

         this->pushBusyCursor();

         foreach my $s (@broken_submodels) {

            # invoke APM edit without checking model health.
            my $command = "apm-edit --nocheck-health " . $s->filename();
            if (system("$command &")) {
                Qt::MessageBox::information(
                    this, 
                    "apm-edit open", 
                    "File open failed");
                return;
            }
         }

         this->popBusyCursor();

       }

       # Re-open the current model in case anything has changed.

       $m = Asim::Model->new($filename);

       if (! defined $m) {
           Qt::MessageBox::information(
               this, 
               "apm-edit open", 
               "File open failed");
           return;
       }

       $model = $m;
    }

    my @stale_submodels = $model->get_stale_submodels();
    my $open_stale = 0;

    if ($#stale_submodels > -1) {
    
      my $status = Qt::MessageBox::warning ( 
          this, 
          "apm-edit open", 
          "The current model uses submodels that contain stale information.\nDo you wish to open these submodels?",
          "&Yes",
          "&No",
          "",
          0,
          1);

       if ($status == 0) {

         this->pushBusyCursor();

         foreach my $s (@stale_submodels) {

            # invoke APM edit without checking model health.
            my $command = "apm-edit --nocheck-health " . $s->filename();
            if (system("$command &")) {
                Qt::MessageBox::information(
                    this, 
                    "apm-edit open", 
                    "File open failed");
                return;
            }
         }

         this->popBusyCursor();

       }

       # Re-open the current model in case anything has changed.

       $m = Asim::Model->new($filename);

       if (! defined $m) {
           Qt::MessageBox::information(
               this, 
               "apm-edit open", 
               "File open failed");
           return;
       }

       $model = $m;
    }

      
    my @obsolete_submodels = $model->get_obsolete_submodels();
    my $open_obsolete = 0;

    if ($#obsolete_submodels > -1) {
    
      my $status = Qt::MessageBox::warning ( 
          this, 
          "apm-edit open", 
          "The current model uses obsolete submodels\nDo you wish to open these submodels?",
          "&Yes",
          "&No",
          "",
          0,
          1);

       if ($status == 0) {
       
         this->pushBusyCursor();
         
         foreach my $s (@obsolete_submodels) {
            # invoke APM edit - but unlike the above cases do check model health.
            my $command = "apm-edit --nocheck-health " . $s->filename();

            if (system("$command &")) {
                Qt::MessageBox::information(
                    this, 
                    "apm-edit open", 
                    "File open failed");
                return;
            }
         }
         
         this->popBusyCursor();

       }

       # Re-open the current model in case anything has changed.

       $m = Asim::Model->new($filename);

       if (! defined $m) {
           Qt::MessageBox::information(
               this, 
               "apm-edit open", 
               "File open failed");
           return;
       }

       $model = $m;
    }


    #
    # Finished with check of submodels, now
    # display error popup for THIS model if needed.
    #
    # Purposely ignore submodels as they are handled above
    #

    if ($model->is_broken(0)) {
        my $error;
        my @missing;

        my $count = $model->missing_module_count();

        $error  = "Model has $count unsatisfied requirements or missing modules\n";
        @missing = $model->missing_packages();

        if (@missing) {
            $error .= "Potentially missing packages: " . join(", ", @missing);
        }

        Qt::MessageBox::information(
            this, 
            "apm-edit open",
            $error);
    }
    elsif ($model->is_stale(0)) {

    # Display stale warning popup if needed and not broken
    # Purposely ignore submodels as they are handled above

        my $moved_count = $model->moved_module_count();    
        # Warn the user.
        my $popup;

        $popup = "Model contains stale information that seems to be out of date.\n";
        $popup .= "This includes $moved_count files that seem to have moved on disk.\n" if ($moved_count > 0);
        $popup .= "It is recommended that you save your model to update this information.";

        Qt::MessageBox::information(
            this, 
            "apm-edit open",
            $popup);
    }

    # Display error popup for this model if needed
    # Purposely ignore submodels as they are handled above



    my @obsolete = $model->get_obsolete_modules(1);

    if (@obsolete) {
      my $error;

      $error  = "Model contains obsolete modules!\n";
      $error .= "Obsolete modules:\n\n" . 
                 join("\n\t", map {$_->provides() . ": " . $_->name()} @obsolete );

      Qt::MessageBox::information(
          this, 
          "apm-edit open",
          $error);
    }

    ui()->statusBar()->showMessage("Model health check complete...", 5000);
    
}




sub modelShow
{
    our $model;

    this->pushBusyCursor();

    # Set model name
    
    ui()->name->setText($model->name());

    # Fill in model tree

    ui()->model()->clear();

    my $root = Qt::TreeWidgetItem(ui()->model(), 0);
    $root->setChildIndicatorPolicy(2); 
    $root->setExpanded(1); 

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

    ui()->model()->setCurrentItem($root, 1);
    ui()->model()->scrollToItem($root);

    # Display captions and status

    my $filename = basename($model->filename() || "Unnamed");
    setWindowTitle("$filename - apm-edit");
    
    this->popBusyCursor();

}


sub buildModel
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

            my $next_root = Qt::TreeWidgetItem($root, 0);
  
            $next_root->setText(module_type_col, trUtf8($m->provides()));

            if (! $m->isroot()) {
                # Normal internal node
                $next_root->setChildIndicatorPolicy(2); 
                $next_root->setExpanded(1); 

		modulePaint($next_root, $m);

                buildModel($next_root, $m, $level+1);
            } else {
                # Submodel leaf node
                $next_root->setChildIndicatorPolicy(2); 
                $next_root->setExpanded(0); 

		modulePaint($next_root, $m->owner());
            }
        } else {
            ui()->statusBar()->showMessage("No implementation specified for module of type $r...", 2000);

            my $empty_item = Qt::TreeWidgetItem($root, 0);
            $empty_item->setChildIndicatorPolicy(2); 
            $empty_item->setExpanded(0); 

            $empty_item->setText(module_type_col, trUtf8($r));
	        modulePaint($empty_item);
        }
    }

    return 1;
}


sub filePrint
{
    our $model;

    print $model->name() . "\n";
    $model->dump();
}

sub fileSave
{
    our $model;

    fileSaveAs($model->filename());
}

sub fileSaveAs
{
    our $model;

    my $cwd;
    my $status;
    my $options;
    my $filter;


    my $s = shift;

    if (defined($model) && defined($model->filename())) {
        $cwd = dirname($model->filename());
    } else {
        chomp($cwd = `pwd`);
    }

    $options |= Qt::FileDialog::DontConfirmOverwrite();

    if (!defined($s)) {
        $s = Qt::FileDialog::getSaveFileName(
            this,
            "Choose a filename to save model",
            $cwd, 
            "Asim Models (*.apm)", 
            $filter,
            $options) ||
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

    setWindowTitle(basename($model->filename()) . " - apm-edit");
}

sub fileExit
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

sub editUndo
{
     Qt::MessageBox::information(
         this, 
         "apm-edit undo",
         "Unimplemented function - editUndo.\n");
}

sub editRedo
{

}

sub editCut
{

}

sub editCopy
{

}

sub editPaste
{

}

sub editFind
{
    ui()->search->setFocus(7);
}



sub editFindMissing
{
    # TBD: Does not work if no item selected...

    my $start_module = ui()->model()->currentItem() || return;
    my $current_module = $start_module;
    
    while ($current_module = Model_walk($current_module)) {
        if ($current_module->text(module_implementation_col) eq "") {
            ui()->model()->setFocus();
            ui()->model()->setCurrentItem($current_module, 1);
            ui()->model()->scrollToItem($current_module);
            return;
        }
    }

    #
    # Nothing found --- wrap search if not a root
    #
    #if ($start_module->text(module_type_col) ne "model") {
    #    editFindMissing(ui()->model()->firstChild());
    #    return;
    #}

    #ui()->statusBar()->message("All requires satisfied...", 2000);
}

# View menu


sub viewOpen_build_folderAction_activated
{
    our $model;

    Asim::open_at($model->build_dir());
}

sub viewShell_at_build_folderAction_activated
{
    our $model;

    Asim::shell_at($model->build_dir());
}


sub viewOpen_run_folderAction_activated
{
    our $model;

    Asim::open_at($model->run_dir());
}


sub viewShell_at_run_folderAction_activated
{
    our $model;

    Asim::shell_at($model->run_dir());
}

# Model menu


sub modelNuke
{
    our $model;

    my $cmd;

    $cmd = $model->nuke("--getcommand" => 1);

    modelOperation("clean", $cmd);
}


sub modelConfigure
{
    our $model;

    my $cmd;

    $cmd = $model->configure("--getcommand" => 1);

    modelOperation("configure", $cmd);
}


sub modelCleanAction_activated
{
    our $model;

    my $cmd;

    $cmd = $model->clean("--getcommand" => 1);

    modelOperation("clean", $cmd);
}

sub modelBuild
{
    our $model;

    my $cmd;

    $cmd = $model->build("--getcommand" => 1);

    modelOperation("build", $cmd);
}


sub modelSetupAction_activated
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


sub modelRunAction_activated
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



sub modelOperation
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

    $w = runlog();

    $w->run($cmd);
}



sub modelAutoBuild_activated
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
    $current_module = ui()->model()->child(0);

    while (defined($current_module)) {

        if ($current_module->text(module_implementation_col) eq "") {
            ui()->model()->setSelected($current_module,1);
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
    $current_module = ui()->model()->child(0);
    ui()->model()->setCurrentItem($current_module, 1);
    ui()->model()->scrollToItem($current_module);

    if ($count > 0) {
        Qt::MessageBox::information(
            this, 
            "apm-edit autobuild", 
            "Model still has $count unsatisfied requirements");
    }        

}



sub modelRefresh_activated
{
    our $moduleDB;
    our $modelDB;

    # Rehash the module DB

    this->pushBusyCursor();

    ui()->statusBar()->showMessage("Loading moduleDB...");
    print "Loading moduleDB...";
    STDOUT->flush();

    $moduleDB->rehash();

    ui()->statusBar()->showMessage("Loading moduleDB...done", 5000);
    print "...done.\n";

    # Rehash the model DB

    ui()->statusBar()->showMessage("Loading modelDB...");
    print "Loading modelDB...";
    STDOUT->flush();

    $modelDB->rehash();

    ui()->statusBar()->showMessage("Loading modelDB...done", 5000);
    print "...done.\n";

    #
    # TBD - Redraw the model tree too...
    #

    #
    # Force refresh of alternatives pane
    #
    my $item=ui()->model()->currentItem() || return;
    Model_selectionChanged($item);

    this->popBusyCursor();

}



sub modelProperties_clicked
{
    modelPropertiesAction_activated();
}

sub modelPropertiesAction_activated
{
    our $model;

    use apm_edit_properties;

    my $w = apm_edit_properties;

    $w->init();
    $w->show();
    $w->exec();

    ui()->name->setText($model->name());
}


# Module menu



sub moduleNewAction_activated
{
    our $model;

    #
    # Find a reasonable directory to start in
    #
    my $item=ui()->model()->currentItem() || return;
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
sub moduleEditAction_activated
{
    my $current_item = ui()->model()->currentItem()
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


sub moduleOpening_containing_folderAction_activated
{
    my $current_item = ui()->model()->currentItem()
                       || return;

    my $filename = $current_item->text(module_file_col)
                   || return;


    moduleOpenContainer($filename);
}


#
# Open a shell at the directory containing the module
#


sub moduleShell_at_containing_folderAction_activated
{
    my $current_item = ui()->model()->currentItem()
                       || return;

    my $filename = $current_item->text(module_file_col)
                   || return;


    moduleShellContainer($filename);
}

#
# Open the notes for this module
#

sub moduleNotesAction_activated
{
    my $current_module = ui()->model()->currentItem()
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
      if (!($f =~ /\.txt$/) && ($f ne "README")) {
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
sub moduleViewAwbAction_activated
{
    my $current_module = ui()->model()->currentItem()
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
sub moduleViewSourceAction_activated
{
    moduleEditAction_activated();
}


#
# Selected and add a submodel at the selected node
#

sub moduleAddSubmodelAction_activated
{
    moduleInsertSubmodelAction_activated();
}

sub moduleInsertSubmodelAction_activated
{
    our $model;
    
    my $item = ui()->model()->currentItem() || return;
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

    if ($submodel->is_obsolete()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit new", 
            "The submodel about to be inserted is obsolete. Are you sure you want to insert it?",
            "&Yes",
            "&No",
            "",
            1,
            2);

        return  if ($status == 1);
        return  if ($status == 2);
    }

    #
    # Replace the module in the model
    #
    $model->smart_add_submodule($submodel);

    #
    # Remove children of module being replaced
    #

    $item->takeChildren();

    #
    # Add the submodel information in the tree
    #
    $item->setChildIndicatorPolicy(2); 
    $item->setExpanded(0); 

    $item->setText(module_type_col, trUtf8($submodel->provides()));
    modulePaint($item, $submodel);

    return;
}

#
# Replace the selected node with the module from the
# alternative modules pane
#


sub moduleReplaceAction_activated
{
    moduleInsertModuleAction_activated();
}


sub moduleInsertModuleAction_activated
{
    my $item = ui()->alternatives->currentItem() || return;

    Alternatives_returnPressed($item);
}



#
# Reinsert selected node as root of model
#
sub moduleInsertAsRootAction_activated
{
    our $model;

    my $item = ui()->model()->currentItem() || return;
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
    ui()->model()->clear();
    ui()->model()->insertItem($item);
}

#
# Delete the selected node
#
sub moduleDeleteAction_activated
{
    return if (! ui()->model()->hasFocus());

    #
    # Determine item to be deleted
    #
    my $item = ui()->model()->currentItem() || return;

    moduleDelete($item);
    return;
}


sub moduleDelete
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

    $item->takeChildren();

    return;
}

# Alternatives menu



sub alternativesInsertAction_activated
{
  my $item = ui()->alternatives->currentItem() || return;

  Alternatives_returnPressed($item);
}


sub alternativesEditAction_activated
{
    my $item   = ui()->alternatives->currentItem() || return;
    my $module = alternativesModule($item)    || return;

    # Edit all the files belonging to the module

    moduleEditFiles($module, "*");
}



sub alternativesOpen_containing_folderAction_activated
{
    my $item   = ui()->alternatives->currentItem() || return;
    my $module = alternativesModule($item)    || return;

    moduleOpenContainer($module->filename());
}


sub alternativesShell_at_containing_folderAction_activated
{
    my $item   = ui()->alternatives->currentItem() || return;
    my $module = alternativesModule($item)    || return;

    moduleShellContainer($module->filename());
}
	        

# Help menu

sub helpIndex
{

}

sub helpContents
{

}

sub helpAbout
{
    use apm_edit_about;

    my $w = apm_edit_about;

    $w->show();
    $w->exec();
}


#
# Search bar
#

sub Search_textChanged
{
    my $pattern = lc(shift);
    my $start_module = shift || ui()->model()->currentItem() || return;
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
            ne ui()->model()->child(0)->text(module_type_col)) {
            #
            # Wrap search to root of model
            #
            ui()->statusBar()->showMessage("Search wrapping...\n", 2000);
            Search_textChanged($pattern, ui()->model()->child(0));
        } else {
            ui()->statusBar()->showMessage("Search failed...\n", 2000);
        }

        return;
    }

    #
    # Search succeeded
    #
    ui()->model()->setCurrentItem($current_module, 1);
    ui()->model()->scrollToItem($current_module);
}


sub Search_returnPressed
{
    my $start;

    $start = ui()->model()->currentItem() || return;
    $start = Model_walk($start)    || return;

    Search_textChanged(ui()->search->text(), $start);
    
}


#
# Name text box
#

sub Name_textChanged
{
    our $model;

    $model->name(shift);
}


#
# Model view
#

sub Model_selectionChanged
{
    our $model;
    our $moduleDB;
    our $modelDB;

    my $item = shift || return;

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

    ui()->alternatives->clear();

    ui()->info->clear();
    ui()->files->clear();
    ui()->parameters->clear();
    ui()->notes->clear();

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
        #FIXME
        #use ColoredListViewItem;

        my @color = (255,255,255);

        if ($m->is_obsolete()) {
            # Make obsolete modules yellow
            @color = (255, 255, 128);
        }

        if ($module_name eq $m->name()) {
            # Make current module orange
            @color = (230, 165, 40);
        }

        #my $next_root = ColoredListViewItem(Alternatives, undef, undef, @color);
        my $next_root = Qt::TreeWidgetItem(ui()->alternatives, 0);
    
        $next_root->setText(alt_implementation_col, trUtf8($m->name()));
        $next_root->setText(alt_file_col, trUtf8($m->filename()));

	my $sortname = sprintf("%2d - %s", $m->template(), $m->name());
	$next_root->setText(alt_sort_col, $sortname);

        if ($module_name eq $m->name()) {
            # 
            # This is module in current model
            #   Note: ">> ... <<" form is parsed elsewhere...
            #
            #$next_root->setText(alt_implementation_col, 
            #                    trUtf8(">> " . $m->name() . " <<"));

    	    $selected = $next_root;
        }
    
        # TBD: Score for submodels

        my $is_model = $m->filename() =~ /.apm$/;
        my $score = $model->is_default_module($m);

        if ($is_model) {
            $next_root->setIcon(alt_implementation_col, Qt::Icon(submodel_pix()));

        } else {
            if ($score) {
                $next_root->setIcon(alt_implementation_col, Qt::Icon(module_default_pix()));
            } else {
                $next_root->setIcon(alt_implementation_col, Qt::Icon(module_pix()));
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
        $module_default->setIcon(alt_implementation_col, Qt::Icon(module_current_pix()));
    }

    #
    # Optionally autoselect a module...
    #

    if ( $model->autoselect() && !defined($module)) {
        ui()->statusBar()->showMessage("Autoselecting....\n", 2000);
        if (defined($module_default)) {
            ui()->statusBar()->showMessage("Autoselecting....selected a module for $provides\n", 2000);
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
           ui()->alternatives->setCurrentItem($selected);
           ui()->alternatives->scrollToItem($selected);
      };

      print $@ if $@;
    }

#
#   Try to sort alphabetically, but with templates at bottom
#   TBD: Figure out why this doesn't work
# 
#    ui()->alternatives->setSorting(2);
#    ui()->alternatives->sort();
}


sub Model_doubleClicked
{
    moduleEditAction_activated()
}



#
# Alternatives view
#

 
sub Alternatives_selectionChanged
{
    our $model;

    my $item = shift || return;

    my $module = undef;
    my $delimiter;   # Used to distinquish if part of real model

    $module = alternativesModule($item);

    if (! defined($module)) {
	print "Internal error: failed to open file for selected alternative module\n";
        return;
    }
 
    #
    # Determine if this item is part of real model
    #    Note: alternativesModule just did this!!!
    #

    my $model_item = ui()->model()->currentItem();
    my $model_item_file = defined($model_item)?$model_item->text(module_file_col):"";
    my $alt_item_file =  $item->text(alt_file_col);

    if ($model_item_file eq $alt_item_file) {
        $delimiter = "=";
    } else {
        $delimiter = "~";
    }

    #
    # Determine if we can insert this module
    #

    my $template = $module->template();

    ui()->alternativesInsertAction->setEnabled(!$template);
    ui()->moduleInsertModuleAction->setEnabled(!$template);

    #
    # Fill in information boxes
    #

    ui()->info->clear();
    ui()->files->clear();
    ui()->parameters->clear();
    ui()->notes->clear();

    if (! defined($module)) {
        ui()->info->addItem("Module not found - refresh probably needed!!!");
        return;
    }

    #
    # Write out description of module
    #
    my @label = ();
    my @info  = ();

    my $filename = $module->filename;

    #
    # Populate the Info tab
    #

    
    if ($module->is_obsolete()) {
      ui()->Info->addItem(" **** WARNING OBSOLETE MODULE **** ");
      ui()->Info->addItem("");
    }

    ui()->info->addItem("Package:     " . Asim::file2package($filename));
    ui()->info->addItem("Module:      " . basename($filename));
    ui()->info->addItem("Name:        " . $module->name());
    ui()->info->addItem("Description: " . $module->description());
    ui()->info->addItem("Attributes:  " . $module->attributes2string());
    ui()->info->addItem("Provides:    " . $module->provides());
    ui()->info->addItem("Requires:    " . join(" ", ($module->requires())));



    #
    # Populate the Files tab
    #

    push(@label, "Package:     ");
    push(@info,  Asim::file2package($filename));

    push(@label, "Path:        ");
    push(@info,  dirname($filename));

    push(@label, "Module:      ");
    push(@info,  basename($module->filename()));

    push(@label, "Notes:       ");
    push(@info,  join(" ", ($module->notes())));

    push(@label, "Makefiles:   ");
    push(@info,  join(" ", ($module->makefile())));

    push(@label, "Scons:       ");
    push(@info,  join(" ", ($module->scons("*"))));

    push(@label, "Public:      ");
    push(@info,  join(" ", ($module->public())));

    push(@label, "Private:     ");
    push(@info,  join(" ", ($module->private())));

    foreach my $l (@label) {
      my $i = shift(@info);
      next if ($i eq "");
      ui()->files->addItem($l . $i);
    }

    #
    # Populate the Parameters tab
    #

    ui()->parameters->addItem("Parameters: ");

    #
    # Write out parameters
    #    Note: the format is parsed textually - be careful changing it.
    #

    my $nondefault = 0;

    foreach my $p ($module->parameters()) {
        ui()->parameters->addItem("   " . $p->description() . ($p->dynamic()?" (dynamic)":""));


	    my $name = $p->name();
    	my $value = $p->value();
    	my $default = $p->default();

        ui()->parameters->addItem("       $name$delimiter$value [$default]");

    	$nondefault |= ($value ne $default);
    }

    if ($nondefault) {
        ui()->alternativesTabWidget->setTabText(ui()->alternativesTabWidget()->indexOf(ui()->parametersPage()), trUtf8("Parameters *"));
    } else {
        ui()->alternativesTabWidget->setTabText(ui()->alternativesTabWidget()->indexOf(ui()->parametersPage()), trUtf8("Parameters"));
    }
        
    #
    # Populate the Notes tab
    #

    #
    # Get notes file and fill into Notes pane
    #
    my $notes;
    my $dir;

    foreach my $n ($module->notes()) {
      if (($n =~ /\.txt$/) || ($n =~ /README/)) {
        $dir = $module->base_dir();
	$notes = Asim::resolve("$dir/$n");
      }
    }

    if (defined($notes) && -r $notes) {

        ui()->alternativesTabWidget->setTabText(ui()->alternativesTabWidget()->indexOf(ui()->notesPage), trUtf8("Notes *"));

        open(NOTES, $notes);

        while (<NOTES>) {
            chomp;
            ui()->notes->addItem($_);
	}

        close(NOTES);
    } else {
        ui()->alternativesTabWidget->setTabText(ui()->alternativesTabWidget()->indexOf(ui()->notesPage), trUtf8("Notes"));
    }


    ui()->info->setCurrentRow(0);
    ui()->files->setCurrentRow(0);
    ui()->parameters->setCurrentRow(0);
    ui()->notes->setCurrentRow(0);
}
 
 

#
# Replace selected module into parent module 
# that requires a module of this type.
#

sub Alternatives_doubleClicked
{
    my $item = shift;

    Alternatives_returnPressed($item);

    #
    # Move to next unsatisfied requirement
    # or root of model if nothing more to do
    #
    my $model_item = ui()->model()->currentItem();

    while (defined($model_item)) {
        if ($model_item->text(module_implementation_col) eq "") {
            last;
        }
        $model_item = Model_walk($model_item);
    }


    if (!defined($model_item)) {
        return;
    }

    ui()->model()->setCurrentItem($model_item, 1);
    ui()->model()->scrollToItem($model_item);
}

 
sub Alternatives_returnPressed
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

    Model_selectionChanged(ui()->model()->currentItem());
}


sub Alternatives_returnPressed_onModule
{
    our $model;

    my $alt_item = shift;

    my $item = ui()->model()->currentItem() || die("No item was selected in model");
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

    if ($module->is_obsolete()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit new", 
            "The module about to be inserted is obsolete. Are you sure you want to insert it?",
            "&Yes",
            "&No",
            "",
            1,
            2);

        return  if ($status == 1);
        return  if ($status == 2);
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

    $item->takeChildren();

    #
    # Display updated item
    #

    $item->setChildIndicatorPolicy(0); 
    $item->setExpanded(1); 

    $item->setText(module_type_col, trUtf8($module->provides()));
    modulePaint($item, $module);


    #
    # Display rest of model...
    #

    buildModel($item, $module);

    $item->setSelected(0);
    $item->setSelected(1);
}

sub Alternatives_returnPressed_onSubmodel
{
    our $model;

    my $alt_item = shift;

    my $item = ui()->model()->currentItem() || die("No item was selected in model");
    my $child_item;

    my $model_name = $alt_item->text(alt_implementation_col);
    my $model_filename = $alt_item->text(alt_file_col) || return;

    my $submodel = Asim::Model->new($model_filename) || return;

    ###
    ### TBD: Do not duplicate code from moduleInsertSubmodelAction_activated
    ###

    if ($submodel->is_obsolete()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit new", 
            "The submodel about to be inserted is obsolete. Are you sure you want to insert it?",
            "&Yes",
            "&No",
            "",
            1,
            2);

        return  if ($status == 1);
        return  if ($status == 2);
    }

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

    $item->takeChildren();

    #
    # Add the submodel information in the tree
    #
    $item->setChildIndicatorPolicy(2); 
    $item->setExpanded(0); 

    #FIXME: MG
    #$item->setPixmap(module_type_col, submodel_pix);
    modulePaint($item, $submodel);

    return;
}

#
# Utility functions for model tree
#
sub modulePaint
{
    my $item = shift;
    my $module = shift;

    if (! defined($module)) {
        $item->setIcon(module_type_col, Qt::Icon(module_missing_pix()));
        $item->setText(module_flags_col, trUtf8("     "));
        $item->setText(module_implementation_col, trUtf8(""));
        $item->setText(module_file_col, trUtf8(""));
	    return;
    }

    our $model;

    if (ref($module) eq "Asim::Model") {
        $item->setIcon(module_type_col, Qt::Icon(submodel_pix()));
    } else {
        if ($model->is_default_module($module)) {
            $item->setIcon(module_type_col, Qt::Icon(module_default_pix()));
        } else {
            $item->setIcon(module_type_col, Qt::Icon(module_pix()));
        }
    }

    $item->setText(module_implementation_col, trUtf8($module->name()));

    my $flags = moduleFlags($module);

    $item->setText(module_flags_col, trUtf8($flags));
    $item->setText(module_file_col, trUtf8($module->filename()));
   
}


sub moduleFlags
{
    my $module = shift;

    my $flags = "  "; 

    $flags .= moduleCheckParams($module, 0)?"p":" ";
    $flags .= ($module->notes())?"n":" ";
    $flags .= ($module->is_obsolete())?"o":" ";

    $flags .= "  ";

    return $flags;
}



#
# Utility functions for checking and defaulting parameters
#

sub moduleInsertParams
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

sub moduleCheckParams
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

sub moduleDefaultParams
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

sub alternativesEdit_clicked
{
    my $item   = Files->currentItem() || return;

    Files_doubleClicked($item);
}


# Double click on line in Files box

sub Files_doubleClicked
{
    my $fileitem = shift                      || return;
    my $fileline = $fileitem->text();

    # Get module from Alternatives box

    my $item   = ui()->alternatives->currentItem() || return;
    my $module = alternativesModule($item)    || return;

    $fileline =~ /^.*: *(.*)/;
    my $files = $1;
   
    moduleEditFiles($module, $files);
}




sub Parameters_selectionChanged
{
    my $item = ui()->parameters()->currentItem() || return;
    my $line = $item->text();

    my $p_string   = '\"[^"]*\"';
    my $p_number   = '\d+';

    if ($line =~ "^ *([^=]+)=($p_string) ") {
        ui()->paramName->setText($1);
        ui()->paramValue->setEnabled(1);
        ui()->paramValue->setText($2);
        ui()->paramChange->setEnabled(1);
    } elsif($line =~ "^ *([^=]+)=($p_number) ") {
        ui()->paramName->setText($1);
        ui()->paramValue->setEnabled(1);
        ui()->paramValue->setText($2);
        ui()->paramChange->setEnabled(1);
    } else {
        ui()->paramName->setText("");
        ui()->paramValue->setEnabled(0);
        ui()->paramValue->setText("");
        ui()->paramChange->setEnabled(0); 
   }
}



sub ParamValue_returnPressed
{
    ParamChange_clicked();
}

sub ParamChange_clicked
{
    our $model;

    my $item = ui()->parameters->currentItem();
    my $line = $item->text();

    #
    # Parse out information on parameter
    #    Note: implicit dependence on format of line
    # 
    #

    # are we dealing with a string?
    my $p_string   = '\"[^"]*\"';
    my $p_number   = '\d+';
    my $name;
    my $value;
    my $default;    

    if($line =~ "^ *([^=]+)=($p_string) [[]($p_string)[]]") {
        $name = $1;
        if (!defined($name)) {
            die("Illegally formatted parameter line");
        }

        $default = $3;
        if (!defined($default)) {
            print("Illegally formatted parameter line");
            $default = $2;
        }       

        $value = ui()->paramValue->text();
        if ($value eq "") {
            $value = $3;
        }
    } else {  # catch all
        $line =~ "^ *([^=]+)=($p_number) [[]($p_number)[]]";

        $name = $1;
        if (!defined($name)) {
            die("Illegally formatted parameter line");
        }

        $default = $3;
        if (!defined($default)) {
            print("Illegally formatted parameter line");
            $default = $2;
        }       

        $value = ui()->paramValue->text();
        if ($value eq "") {
            $value = $3;
        }
    } 
    
    #
    # Update information on parameter
    #    Note: implicit dependence on format of line
    #
    $item->setText("       $name=$value [$default]");


    #
    # Update module itself
    #    Note: implicit dependence on single provides
    #
    my $module_item = ui()->model()->currentItem();
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

sub moduleEditFiles
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

sub moduleOpenContainer
{
    my $filename = shift;

    use File::Basename;

    my $dirname = dirname(Asim::resolve($filename));

    Asim::open_at($dirname);
}

sub moduleShellContainer
{
    my $filename = shift;

    use File::Basename;

    my $dirname = dirname(Asim::resolve($filename));

    Asim::shell_at($dirname);
}



sub alternativesModule
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

    #
    # Determine if module is in current model
    #
    my $model_item = ui()->model()->currentItem();
    my $model_item_file = defined($model_item)?$model_item->text(module_file_col):"";
    my $alt_item_file = $item->text(alt_file_col);

    if ($model_item_file eq $alt_item_file) {
        # From current model

        my $provides = $model_item->text(module_type_col);
        
        $module = $model->find_module_providing($provides);
        
        if ($model->is_submodel($module)) {
            # This is a submodel...

            $module = $module->owner();
        }
    } else {
        # From .apm/.awb file

        my $filename = $alt_item_file;

        if ($filename =~ /.awb$/) {
            $module = Asim::Module->new($filename);
        } elsif ($filename =~ /.apm$/) {
            $module = Asim::Model->new($filename);
        }
    }

    return $module;
}


sub apm_edit::Model_find
{
    # Unused function

    return 0;
}




sub apm_edit::Model_walk
{
    
    my $current = shift;
    my $parent;
    my $next;

    $next = $current->child(0);
    if (defined($next)) {
        return $next;
    }

    while (defined($current)) {
        $parent = $current->parent();
        if (defined $parent) {
            $next = $parent->child($parent->indexOfChild($current)+1);
            if (defined($next)) {
                return $next;
            }
        }
        $current = $parent;
    }

    return undef;
    
}



sub pushBusyCursor
{
  Qt::Application::setOverrideCursor(Qt::Cursor(Qt::WaitCursor()));

}

sub popBusyCursor
{
  Qt::Application::restoreOverrideCursor();
}

