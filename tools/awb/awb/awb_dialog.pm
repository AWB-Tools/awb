package awb_dialog;

use strict;
use warnings;

use awb_runlog;

use QtCore4;
use QtGui4;
use QtCore4::isa qw( Qt::MainWindow);
use Qt3Support4;
use QtCore4::slots
    init => [],
    Exit_activated => [],
    model_tree_expanded => ['QTreeWidgetItem*'],
    model_tree_collapsed => ['QTreeWidgetItem*'],
    model_tree_selectionChanged => [],
    model_tree_returnPressed => ['QTreeWidgetItem*'],
    model_tree_onItem => ['QTreeWidgetItem*'],
    model_tree_clicked => ['QTreeWidgetItem*'],
    model_list_selectionChanged =>[],
    model_list_clicked => ['QTreeWidgetItem*'],
    benchmark_tree_expanded => ['QTreeWidgetItem*'],
    benchmark_tree_collapsed => ['QTreeWidgetItem*'],
    benchmark_tree_selectionChanged => [],
    benchmark_tree_returnPressed => ['QTreeWidgetItem*'],
    benchmark_tree_onItem => ['QTreeWidgetItem*'],
    benchmark_tree_clicked => ['QTreeWidgetItem*'],
    benchmark_list_selectionChanged => [],
    benchmark_list_clicked => ['QTreeWidgetItem*'],
    model_list_doubleClicked => ['QTreeWidgetItem*'],
    highlightProblemsCheckBox_stateChanged => ['int'],
    nextStep_models_clicked => [],
    nextStep_buildopts_clicked => [],
    nextStep_benchmarks_clicked => [],
    nextStep_parameters_clicked => [],
    nextStep_runopts_clicked => [],
    nextStep_analysis_clicked => [],
    Refresh_activated => [],
    Rehash_activated => [],
    NewModel_activated => [],
    EditModel_activated => [],
    OpenModel_Containing_Folder_activated => [],
    ShellModel_at_Containing_Folder_activated => [],
    Button_nuke_clicked => [],
    Button_config_clicked => [],
    Button_clean_clicked => [],
    Button_build_clicked => [],
    NewBenchmark_activated => [],
    EditBenchmark_activated => [],
    Button_setup_clicked => [],
    Button_run_clicked => [],
    FindReplace_activated => [],
    Manual_activated => [],
    About_activated => [],
    parallelMake_clicked => [],
    compilerType_clicked => [],
    documentation_clicked => [],
    buildType_clicked => [],
    extraBuildSwitches_textChanged => ['const QString&'],
    paramRefresh_clicked => [],
    paramList_itemClicked => ['QListWidgetItem*'],
    paramValue_returnPressed => [],
    paramUpdate_clicked => [],
    traceRun_clicked => [],
    extraRunSwitches_textChanged => ['const QString&'],
    viewStatistics_clicked => [],
    viewStripChart_clicked => [],
    viewCycleDisplay_clicked => [],
    browseAdfPath_clicked => [],
    workspaceDirLineEdit_textChanged => ['const QString&'],
    workspaceBrowsePushButton_clicked => [],
    workspaceNameLineEdit_returnPressed => [],
    workspaceCreatePushButton_clicked => [],
    workspaceComboBox_activated => ['const QString&'],
    workspaceSwitchPushButton_clicked => [],
    repositoriesTabWidget_currentChanged => ['int'],
    checkoutListWidget_itemClicked => ['QListWidgetItem*'],
    repositoryBrowsePushButton_clicked => [],
    checkoutListWidget_doubleClicked => ['QListWidgetItem*'],
    checkoutBundleListWidget_doubleClicked => ['QListWidgetItem*'],
    checkoutBundleListWidget_itemClicked => ['QListWidgetItem*'],
    refreshReposPushButton_clicked => [],
    checkoutPushButton_clicked => [],
    updatePackagesListWidget_itemClicked => ['QListWidgetItem*'],
    packagesBrowsePushButton_clicked => [],
    updatePackagesListWidget_doubleClicked => ['QListWidgetItem*'],
    updateAllCheckBox_toggled => ['bool'],
    refreshPushButton_clicked => [],
    updatePushButton_clicked => [],
    pushBusyCursor => [],
    helpWhatsThisAction_activated => [],
    popBusyCursor => [];

sub repoType {
    return this->{repoType};
}

sub ui() 
{
    return this->{ui};
}

sub NEW 
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Awb_dialog->setupUi(this);
}

#
#   Creation of window pane contents
#

sub init 
{
    use File::Basename;
    use Asim;

    my $public_root;
    my $public_root_item;
    my $workspace;
    my $root;
    my $root_item;
    my $ui = ui();

#    our $stepno = 1;
    our @steps;

    print "awb_dialog::init - We are in init\n" if $debug;

    $workspace = $Asim::default_workspace->rootdir();
    chdir $workspace;

    setWindowTitle(basename($workspace) . " - awb");

    $steps[0] = "";
    $steps[1] = "Step 1: Select a model and click 'Configure'";
    $steps[2] = "Step 2: Select build options on click 'Build'";
    $steps[3] = "Step 3: Selcect a benchmark and click 'Setup'";
    $steps[4] = "Step 4: Select run options and click 'Run'";
    
#    Steps->setText($steps[0]);

    # Clear model info

    $ui->model_tree()->clear();
    $ui->model_list()->clear();

    # Build model tree

    $public_root = Qt::TreeWidgetItem($ui->model_tree(), 0);
    $public_root->setChildIndicatorPolicy(0); 
    $public_root->setText(0, trUtf8("Models"));

    $root = "config/pm";
    print "awb_dialog::init - Model root=$root\n" if $debug;
    $public_root->setText(1, trUtf8($root));
    

    # Clear benchmark info

    $ui->benchmark_tree()->clear();
    $ui->benchmark_list()->clear();

    # Build benchmark tree
    
    $public_root = Qt::TreeWidgetItem($ui->benchmark_tree(), 0);
    $public_root->setChildIndicatorPolicy(0); 
    $public_root->setText(0, trUtf8("Benchmarks"));

    $root = "config/bm";
    $public_root->setText(1, trUtf8($root));


    # Pick up model from $AWB_MODEL (if defined)


    if (defined($ENV{AWB_MODEL})) {
      awb_util::set_model($ENV{AWB_MODEL});
      # TBD - expand tree out to this model
    } else {
        model_list_clicked(undef);
    }

    if (0 && defined($ENV{AWB_BENCHMARK})) {
      awb_util:set_benchmark($ENV{AWB_BENCHMARK});
      # TBD - expand tree out to this benchmark
    } else {
        benchmark_list_clicked(undef);
    }

    # Set initial options settings
    extraBuildSwitches_textChanged();
    extraRunSwitches_textChanged();


    # Set up the setup tab

    setupInit();

    # Create context menus
    createContextMenus();

}

sub createContextMenus 
{
    my $ui = ui();

    # model_list context menu
    $ui->model_list()->addActions($ui->modelMenu()->actions());

    # benchmark list context menu
    $ui->benchmark_list()->addActions($ui->benchmarkMenu()->actions());
    
}

#
# Exit program
#

sub Exit_activated()
{
    Qt::Application::exit();
}

#
# Expansion/collapse of model explorer tree
#
sub model_tree_expanded
{
    my $item = shift;
    my $name = $item->text(1);
    
    print "awb_dialog::model_tree_expanded - Expand=$name\n" if $debug;

    # Delete existing children if any

    $item->takeChildren();

    # Reconstruct the list

    for my $i (awb_util::glob_dir($name)) {
        my $citem = Qt::TreeWidgetItem($item, 0);
        $citem->setText(0, trUtf8(basename($i)));
        $citem->setText(1, trUtf8($i));

        my $n = awb_util::glob_dir($i);
        $citem->setChildIndicatorPolicy(($n > 0)? 0:1);
    }
}


sub model_tree_collapsed
{
    print "Item collapsing\n" if $debug;
}


#
# Selection of a model in model explorer tree
#


sub model_tree_selectionChanged
{
    my $ui = ui();
    model_tree_clicked($ui->model_tree()->selectedItems()->[0]);
}

sub model_tree_returnPressed
{
    model_tree_clicked(shift);
}


sub model_tree_onItem
{
    my $item = shift;
    my $ui = ui();

    # Force model tree expansion
    if (!$item->isExpanded()) {
        this->model_tree_expanded($item);
    }
}

sub model_tree_clicked
{

    my $ui = ui();

    my $item = shift || return;
    my $name = $item->text(1);
    
    this->pushBusyCursor();

    print "awb_dialog::model_tree_clicked - Item = $name\n" if $debug;

    # Force model tree expansion
    if (!$item->isExpanded()) {
        this->model_tree_expanded($item);
    }

    # Add in the models in this directory in model pane

    $ui->model_list->clear();

    for my $i (awb_util::glob_apm($name)) {
        print "awb_dialog::model_tree_clicked - Populating model list = $i\n" if $debug;

        my $color = Qt::transparent();
        if ($ui->highlightProblemsCheckBox->isChecked()) {
            my $mod = Asim::Model->new($i);
            $color = Qt::yellow() if ($mod->is_stale());
            $color = Qt::red() if ($mod->is_broken());
        }
        my $citem = Qt::TreeWidgetItem($ui->model_list(), 0);
        $citem->setBackground(0, Qt::Brush(($color)));
        $citem->setChildIndicatorPolicy(2); 
        #$citem->setExpandable(1);

         #Column 0 = model name
        $citem->setText(0, trUtf8(basename($i,".apm")));

        # Column 1 = model file path
        $citem->setText(1, trUtf8($i));
    }
    
    this->popBusyCursor();

}

#
# Selection of a model in model list
#
sub model_list_selectionChanged
{
    my $ui = ui();
    model_list_clicked($ui->model_list()->selectedItems()->[0]);
}


sub model_list_clicked
{
    my $item = shift;
    my $name;
    my $file;
    my $model = undef;
    my $model_enable;
    my $model_options;
    my $ui = ui();

    this->pushBusyCursor();

    # Clear old model info

    $ui->model_name->setText("");
    $ui->model_description->clear();

    if (defined($item)) {
        # Get model name and file

        $name = $item->text(0);
        $file = $item->text(1);

        $model = awb_util::set_model($file);

        # If there is a model describe it

        if (defined($model)) {
            $ui->model_name->setText($name);

            # Create description

            my $pkg = Asim::file2package($model->filename());
            if (defined($pkg)) {
                $ui->model_description->addItem("Package: " . $pkg);
            }
            $ui->model_description->addItem("Name: " . $model->name());
            $ui->model_description->addItem("Description: " . $model->description());
            $ui->model_description->addItem("File: " . $model->filename());
            $ui->model_description->addItem("Default build directory: " . $model->build_dir());
            my @dep = ($model->dependencies());
            $ui->model_description->addItem("Dependencies: @dep");
            $ui->model_description->addItem("Save Parameters: " . $model->saveparams());

            # Add model build options

            $model_options = $model->build_options();

            $ui->buildOptions->clear();
            for my $o (keys %{$model_options}) {
                Qt::ListWidgetItem(trUtf8($o . " - " . $model_options->{$o}), $ui->buildOptions());
            }
        }
    }

    # Enable/disable actions as appropriate

    $ui->nextStep_models->setEnabled(0);
    $ui->nextStep_buildopts->setEnabled(0);
    $ui->nextStep_benchmarks->setEnabled(0);
    $ui->nextStep_parameters->setEnabled(0);
    $ui->nextStep_runopts->setEnabled(0);
    $ui->nextStep_analysis->setEnabled(0);
    
    $model_enable = defined($model);

    $ui->editModel->setEnabled($model_enable);
    $ui->cleanModel->setEnabled($model_enable);
    $ui->configureModel->setEnabled($model_enable);
    $ui->buildModel->setEnabled($model_enable);

    $ui->button_nuke->setEnabled($model_enable);
    $ui->button_clean->setEnabled($model_enable);
    $ui->button_config->setEnabled($model_enable);
    $ui->button_build->setEnabled($model_enable);


    # Clear benchmarks

    # TBD - unselect benchmark in pane...
    benchmark_list_clicked(undef);

    # Clear parameters
    $ui->paramList->clear();

    this->popBusyCursor();

}

#
# Benchmark selection
#

sub benchmark_tree_expanded
{
    my $item = shift;
    my $name = $item->text(1);
    
    print "awb_dialog::benchmark_tree_expanded - Expand=$name\n" if $debug;

    # Delete existing children if any
    
    $item->takeChildren();

    # Reconstruct the list

    for my $i (awb_util::glob_dir($name)) {
        my $citem = Qt::TreeWidgetItem($item, 0);
        $citem->setText(0, trUtf8(basename($i)));
        $citem->setText(1, trUtf8($i));

        my $n = awb_util::glob_dir($i);
        $citem->setChildIndicatorPolicy($n>0? 0:1);
    }

}


sub benchmark_tree_collapsed
{
    print "Item collapsing\n" if $debug;
}


sub benchmark_tree_selectionChanged
{
    my $ui = ui();
    benchmark_tree_clicked($ui->benchmark_tree()->selectedItems()->[0]);
}

sub benchmark_tree_returnPressed
{
    benchmark_tree_clicked(shift);
}


sub benchmark_tree_onItem
{
    my $item = shift;

    # Force benchmark tree expansion
    if (!$item->isExpanded()) {
        benchmark_tree_expanded($item);
    }
}


sub benchmark_tree_clicked
{
    my $item = shift || return;
    my $name = $item->text(1);
    my $ui = ui();

    print "awb_dialog::benchmark_tree_clicked - Item = $name\n" if $debug;

    this->pushBusyCursor();

    # Force benchmark tree expansion
    if (!$item->isExpanded()) {
        benchmark_tree_expanded($item);
    }

    # Add in the benchmarks in this directory

    $ui->benchmark_list()->clear();

    for my $i (awb_util::glob_abm($name)) {
        my $citem = Qt::TreeWidgetItem($ui->benchmark_list(), 0);
        $citem->setText(0, trUtf8(basename($i,".apm")));
        $citem->setText(1, trUtf8($i));
    }

    this->popBusyCursor();

}


sub benchmark_list_selectionChanged
{
    my $ui = ui();
    benchmark_list_clicked($ui->benchmark_list()->selectedItems()->[0]);
}


sub benchmark_list_clicked
{
    my $item = shift;
    my $name;
    my $file;
    my $benchmark = undef;
    my $benchmark_enable;
    my $ui = ui();

    print "awb_dialog::benchmark_list_clicked \n" if $debug;
    
    # Clear out old benchmark

    this->pushBusyCursor();

    $ui->benchmark_name->setText("");
    $ui->benchmark_description->clear();

    awb_util::set_benchmark(undef);

    if (defined($item)) {
        $name = $item->text(0);
        $file = $item->text(1);

        $benchmark = awb_util::set_benchmark($file);

        if (defined($benchmark)) {
            $ui->benchmark_name->setText($name);
            $ui->benchmark_description->addItem("Name: $name");
            # TBD - benchmark information
        }
    }


    $benchmark_enable = defined($benchmark)
                        && $ui->button_build()->isEnabled();

    $ui->editBenchmark->setEnabled($benchmark_enable);
    $ui->setupBenchmark->setEnabled($benchmark_enable);
    $ui->runBenchmark->setEnabled($benchmark_enable);

    $ui->button_setup->setEnabled($benchmark_enable);
    $ui->button_run->setEnabled($benchmark_enable);

    # Refresh the parameters list
    #   Does not work before benchmark is set up

    # paramRefresh_clicked();    

    this->popBusyCursor();

}


sub model_list_doubleClicked
{
    awb_util::edit_model(this);
}



sub highlightProblemsCheckBox_stateChanged
{
    my $ui = ui();
    if (my $item = $ui->model_tree->selectedItems()->[0]) {
        model_tree_clicked($item);
    }
}



#
# Wizard buttons
#

sub nextStep_models_clicked
{
   my $ui = ui();
   $ui->tabsMain->setCurrentWidget($ui->tabBuildOpts());
}


sub nextStep_buildopts_clicked
{
   my $ui = ui();
   $ui->tabsMain->setCurrentWidget($ui->tabBenchmarks());
}


sub nextStep_benchmarks_clicked
{
   my $ui = ui();
   $ui->tabsMain->setCurrentWidget($ui->tabParameters());
}


sub nextStep_parameters_clicked
{
   my $ui = ui();
   $ui->tabsMain->setCurrentWidget($ui->tabRunOpts());
}


sub nextStep_runopts_clicked
{
   my $ui = ui();
   $ui->tabsMain->setCurrentWidget($ui->tabAnalysis());
}


sub nextStep_analysis_clicked
{
    my $ui = ui();
    model_list_clicked(undef);
    benchmark_list_clicked(undef);

    $ui->nextStep_models->setEnabled(0);
    $ui->nextStep_buildopts->setEnabled(0);
    $ui->nextStep_benchmarks->setEnabled(0);
    $ui->nextStep_parameters->setEnabled(0);
    $ui->nextStep_runopts->setEnabled(0);
    $ui->nextStep_analysis->setEnabled(0);
    
    $ui->tabsMain->setCurrentWidget($ui->tabModels());
}

#
# Button Actions
# 
sub Refresh_activated()
{
    my $item;
    my $ui = ui();

    this->pushBusyCursor();

    if ($item = $ui->model_tree->selectedItems()->[0]) {
        model_tree_clicked($item);
    }

    if ($item = $ui->benchmark_tree->selectedItems()->[0]) {
        benchmark_tree_clicked($item);
    }

    this->popBusyCursor();

}



sub Rehash_activated()
{

    my  $moduleDB = Asim::Module::DB->new(".");
    my $modelDB = Asim::Model::DB->new();
    my $ui = ui();

    this->pushBusyCursor();

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

    this->popBusyCursor();
 
    # Refresh the highlighting list, if appropriate
    if (my $item = $ui->model_tree->selectedItems()->[0]) {
        model_tree_clicked($item);
    }
}

sub NewModel_activated()
{
    awb_util::new_model();
}


sub EditModel_activated()
{
    awb_util::edit_model();
}


sub OpenModel_Containing_Folder_activated()
{
    awb_util::open_model_container();
}


sub ShellModel_at_Containing_Folder_activated()
{
    awb_util::shell_model_container();
}



sub Button_nuke_clicked()
{
    this->pushBusyCursor();

    awb_util::nuke_model()
        ||  Qt::MessageBox::information(
            this, 
            "nuke_model",
            "Model nuke failed\n");

    this->popBusyCursor();

}

sub Button_config_clicked()
{
    my $status;
    my $ui = ui();

    this->pushBusyCursor();

    # BUG - This does not work after subwindow creation!!!!
    $ui->nextStep_models->setEnabled(1);

    $status = awb_util::config_model();

    this->popBusyCursor();

    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "config_model",
            "Model configuration failed\n");
        return;
    }

}


sub Button_clean_clicked()
{

    this->pushBusyCursor();

    awb_util::clean_model()
        ||  Qt::MessageBox::information(
            this, 
            "clean_model",
            "Model clean failed\n");

    this->popBusyCursor();

}


sub Button_build_clicked()
{
    my $status;
    my $ui = ui();

    this->pushBusyCursor();

    # BUG - This does not work after subwindow creation!!!!
    $ui->nextStep_buildopts->setEnabled(1);

    # Force switch setup

    extraBuildSwitches_textChanged();

    $status = awb_util::build_model();

    this->popBusyCursor();

    if (! defined($status)) {
        Qt::MessageBox::information(
            this, 
            "build_model",
            "Model build failed\n");
        return;
    }

}





sub NewBenchmark_activated()
{
    awb_util::new_benchmark();
}


sub EditBenchmark_activated()
{
    awb_util::edit_benchmark();

}


sub Button_setup_clicked
{
    my $status;
    my $ui = ui();

    this->pushBusyCursor();

    # BUG - This does not work after subwindow creation!!!!
    $ui->nextStep_benchmarks->setEnabled(1);
    $ui->nextStep_parameters->setEnabled(1);

    $status = awb_util::setup_benchmark();
        
    this->popBusyCursor();

    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "setup_benchmark",
            "Benchmark setup failed\n");
        return;
    }

}

sub Button_run_clicked
{
    my $status;
    my $ui = ui();

    this->pushBusyCursor();

    # BUG - This does not work after subwindow creation!!!!
    $ui->nextStep_runopts->setEnabled(1);
    $ui->nextStep_analysis->setEnabled(1);

    # Force run options to be set

    extraRunSwitches_textChanged();
    
    $status = awb_util::run_model();

    this->popBusyCursor();

    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "run_model",
            "Model run failed\n");
        return;
    }
}


sub FindReplace_activated()
{
    system("apm-find-replace &");
}


sub Manual_activated()
{
    my $w = awb_runlog();

    return $w->run("awb2 --help");
}


sub About_activated()
{
    my $a = awb_about->NEW();
    $a->exec();
}


#
# Build Options page
#     I do not like the fact that this module knows the actual make switches....
#



sub parallelMake_clicked
{
    extraBuildSwitches_textChanged();
}


sub compilerType_clicked
{
    extraBuildSwitches_textChanged();
}

sub documentation_clicked
{
    extraBuildSwitches_textChanged();
}

sub buildType_clicked
{
    extraBuildSwitches_textChanged();
}



sub extraBuildSwitches_textChanged
{
    my $options = "";
    my $ui = ui();

    if ($ui->documentationYes->isChecked()) {
        $options .= "dox";
    }

    if ($ui->parallelNo->isChecked()) {
        $options .= " PAR=0";
    }

    if ($ui->parallelYes->isChecked()) {
        $options .= " PAR=1";
    }

    if ($ui->compilerGEM->isChecked()) {
        $options .= " GNU=0";
    }

    if ($ui->compilerGNU->isChecked()) {
        $options .= " GNU=1";
    }

    if ($ui->buildOpt->isChecked()) {
        $options .= " DEBUG=0 OPT=1";
    }

    if ($ui->buildDebug->isChecked()) {
        $options .= " DEBUG=1 OPT=0";
    }

    for my $i (1..$ui->buildOptions->count()) {
        if ($ui->buildOptions->item($i-1)->isSelected()) {
            my $entry = $ui->buildOptions->item($i-1)->text();
	    if ($entry =~ /<T>$/) {
		$entry =~ s/ .*//;
		$options .= " $entry";
	    } else {
		$entry =~ s/ .*//;
		$options .= " $entry=1";
	    }
        }
    }

    $options .= " " . $ui->extraBuildSwitches->text();

    awb_util::set_build_switches($options);
}

#
# Dynamic parameters page
#


sub paramRefresh_clicked
{
    my $ui = ui();
    $ui->paramList->clear();

    for my $i (awb_util::get_params_model()) {
        print "****** $i\n";
        $ui->paramList->insertItem("$i");
    }
}


sub paramList_itemClicked
{
    my $item = shift;
    my $line = $item->text();
    my $ui = ui();

    if ($line =~ "^([^=]+) = ([^ ]+) ") {
        $ui->paramName->setText($1);
        $ui->paramValue->setEnabled(1);
        $ui->paramValue->setText($2);
        $ui->paramUpdate->setEnabled(1);
    } else {
        $ui->paramName->setText("");
        $ui->paramValue->setEnabled(0);
        $ui->paramValue->setText("");
        $ui->paramUpdate->setEnabled(0); 
   }

}

sub paramValue_returnPressed
{
    my $ui = ui();
    paramUpdate_clicked();
}


sub paramUpdate_clicked
{
    my $item; 
    my $line; 
    my $ui = ui();

    #
    # Get parameter line out of dialog box
    #
    $item = $ui->paramList->currentItem();
    if (! defined ($item)) {
      print "awb: No parameter selected\n";
      return;
    }

    $line = $item->text();

    #
    # Parse out information on parameter
    #    Note: implicit dependence on format of line
    #

    $line =~ "([^=]+) = ([^ ]+) [[](.+)[]]";

    my $name = $1;
    if (!defined($name)) {
        print "awb: Illegally formatted parameter line: $line\n";
	return;
    }

    my $default = $3;
    if (!defined($default)) {
        print "awb: Illegally formatted parameter line $line\n";
        return;
    }       

    my $value = $ui->paramValue->text() || $default;
    
    #
    # Update information on parameter
    #    Note: implicit dependence on format of line
    #
    $ui->paramList->editItem($item);
    $item->setText("$name = $value [$default]");

}

#
# Run Options page
#     I do not like the fact that this module knows the actual run switches....
#


sub traceRun_clicked
{
    my $ui = ui();
    extraRunSwitches_textChanged();
}


sub extraRunSwitches_textChanged
{
    my $options = "";
    my $data;
    my $ui = ui();

    if ($data = $ui->endCycle->text()) {
        $options .= " -c $data";
    }

    if ($data = $ui->endInstruction->text()) {
        $options .= " -i $data";
    }

    if ($ui->traceYes->isChecked()) {
        $options .= " -t";
    }

    if ($data = $ui->startTrace->text()) {
        $options .= " -tsc $data";
    }

    if ($data = $ui->endTrace->text()) {
        $options .= " -tec $data";
    }

    if ($ui->eventlogYes->isChecked()) {
        $options .= " -e";
    }

    if ($data = $ui->startEventlog->text()) {
        $options .= " -esc $data";
    }

    if ($data = $ui->endEventlog->text()) {
        $options .= " -eec $data";
    }

    $options .= " " . $ui->extraRunSwitches->text();

    #
    # Pick up parameter updates
    #
    for my $i (1..$ui->paramList->count()) {
        #
        # Parse out information on parameter
        #    Note: implicit dependence on format of line
        #
        my $line=$ui->paramList->text($i-1);

        $line =~ "([^=]+) = ([^ ]+) [[](.+)[]]";

        if ($2 ne $3) {
            $options .= " -param $1=$2";
        }
    }

    awb_util::set_run_switches($options);
}


#
# Analysis Page
#
sub viewStatistics_clicked
{
    awb_util::view_statistics();
}


sub viewStripChart_clicked
{
    awb_util::view_stripchart();
}


sub viewCycleDisplay_clicked
{
    my $ui = ui();
    my $adf = $ui->adfPath->text();

    awb_util::view_cycledisplay($adf);
}


sub browseAdfPath_clicked
{
    my $cwd;
    my $ui = ui();

    chomp($cwd = `pwd`);
    my $s = shift ||
        Qt::FileDialog::getOpenFileName(
            this,
            "ADF file (*.xml)",
            $cwd,
            "open ADF file dialog",
            "Choose an ADF file" ) ||
        return;;

    $ui->adfPath->setText($s);
}


#
# Setup $ui->tab
#



#
# Workspace group
#

sub workspaceDirLineEdit_textChanged
{
    my $ui = ui();
    my $dir = shift;

    if ( -d $dir) {
        my $workspacename = basename($Asim::default_workspace->rootdir());

        $ui->workspaceComboBox->clear();

        for my $i (glob("$dir/*/awb.config")) {
            my $n = basename(dirname($i));

            $ui->workspaceComboBox->insertItem($ui->workspaceComboBox->count(), $n);
            if ($n eq $workspacename) {
                $ui->workspaceComboBox->setCurrentIndex($ui->workspaceComboBox->count()-1);
            }
        }
    }

}

sub workspaceBrowsePushButton_clicked
{
    my $ui = ui();
    my $dir = Qt::FileDialog::getExistingDirectory(
                    this,
                    "Get directory to hold workspace",
                    $ui->workspaceDirLineEdit->text());

    if ($dir) {
        $ui->workspaceDirLineEdit->setText($dir);
    }

    return;
}



sub workspaceNameLineEdit_returnPressed
{
    my $ui = ui();
    $ui->workspaceCreatePushButton_clicked();
}

sub workspaceCreatePushButton_clicked
{
    my $ui = ui();
    my $dir = $ui->workspaceDirLineEdit->text();
    if (! -d $dir) {
        Qt::MessageBox::information(
            this, 
            "awb", 
            "Root directory for workspace must exist\n");
        return;
    }

    my $workspacename = $ui->workspaceNameLineEdit->text();
    my $workspace = "$dir/$workspacename";

    # TBD: Check that we are not in an existing workspace

    if ( -e $workspace) {
        Qt::MessageBox::information(
            this, 
            "awb", 
            "Workspace directory must not already exist\n");
        return;
    }


    my $command = "asim-shell --batch -- new workspace $workspace";

    my $w = awb_runlog();

    #
    # Debugging hack...
    #
    if ($workspacename =~ /^% (.*)/) {
        $command = $1;
        print $w->run($command);
        $ui->workspaceNameLineEdit->setText("% ");
        return;
    }


    $w->run($command);

    #
    # TBD: This needs to wait for the command to complete
    #

    my $status = $w->wait();

    if ( ! $status) {
        if (Asim::open($workspace)) {
            init();
        }
    }

    $ui->workspaceNameLineEdit->clear();
    return;
}


sub workspaceComboBox_activated
{
#    This behavior seemed annoying
#
#    $ui->workspaceSwitchPushButton_clicked();
}

sub workspaceSwitchPushButton_clicked
{
    my $ui = ui();

    my $dir = $ui->workspaceDirLineEdit->text();
    my $workspace = "$dir/" . $ui->workspaceComboBox->currentText();

    if (Asim::open($workspace)) {
        init();
    }
}



#
# Repository Group
#

sub repositoriesTabWidget_currentChanged
{
    my $ui = ui();
    # Remember if we working on repositories or bundles
    
    this->{repoType} = shift;
}



sub checkoutListWidget_itemClicked
{
    my $ui = ui();
    my $item = shift;
    my $repo = $item->text();

    $ui->repoVersionComboBox->clear();
    $ui->repoVersionComboBox->addItem("<Optionally select alternate version>");

    for my $v (sort keys  %{this->{repos}->{$repo}}) {
        $ui->repoVersionComboBox->addItem($v);
    }
}

sub checkoutBundleListWidget_itemClicked
{
    my $ui = ui();
    my $item = shift;
    my $bundle = $item->text();

    $ui->bundleVersionComboBox->clear();
    $ui->bundleVersionComboBox->addItem("<Optionally select alternate version>");

    for my $v (sort keys  %{this->{bundles}->{$bundle}}) {
        $ui->bundleVersionComboBox->addItem($v);
    }
}




sub repositoryBrowsePushButton_clicked
{
    my $ui = ui();
    if (repoType() == 0) {
        # Prepare to browse a repository

        my $item = $ui->checkoutListWidget->currentItem();
        return if (! defined($item));

        checkoutListWidget_doubleClicked($item);
    } else {
        # Prepare to browse a bundle

        my $item = $ui->checkoutBundleListWidget->currentItem();
        return if (! defined($item));

        checkoutBundleListWidget_doubleClicked($item);
    }
      
}

sub checkoutListWidget_doubleClicked
{
    my $item = shift;
    my $reponame = $item->text();

    my $repoDB = Asim::Repository::DB->new();
    my $repo = $repoDB->get_repository($reponame) || return undef;

    $repo->browse();
}




sub checkoutBundleListWidget_doubleClicked
{
    my $item = shift;
    my $bundlename = $item->text();
    my $ui = ui();

    my $version = $ui->bundleVersionComboBox->currentText();
    if ($version =~ /<.*>/) {
        $version = "default";
    }

    my $repoDB = Asim::Repository::DB->new();
    my $bundle = $repoDB->get_bundle("$bundlename/$version") || return undef;

    Qt::MessageBox::information(
	this, 
	"$bundlename/$version - Bundle Information",
	  "Type:    " . $bundle->type() . "\n"
	. "Status:   " . $bundle->status() . "\n"
	. "Packages: " . join(" ", $bundle->packages()) . "\n");

}



sub refreshReposPushButton_clicked
{
    reposInit();
}


sub checkoutPushButton_clicked
{
    my $command = "asim-shell --batch -- checkout ";
    my $ui = ui();

    if (repoType() == 0) {
        $command .= "package";
    } else {
        $command .= "bundle";
    }

    #
    # Get name of package to check out
    #
    
    my $package;
    my $version;

    if (repoType() == 0) {
        # Get repository name and $ui->version

        my $item = $ui->checkoutListWidget->currentItem();
    	return if (! defined($item));

	    $package = $item->text();
	    $version = $ui->repoVersionComboBox->currentText();
    } else {
        # Get bundle name and $ui->version

	    my $item = $ui->checkoutBundleListWidget->currentItem();
	    return if (! defined($item));

	    $package = $item->text();
	    $version = $ui->bundleVersionComboBox->currentText();
    }

    if (! ($version =~ /<.*>/)) {
        $package .= "/$version";
    }

    $command .= " $package";

    #
    # Optionally add uesrname
    #
    my $username = $ui->usernameLineEdit->text();

    if ($username ne "") {
        $command .= " --user=$username";
    }

    #
    # TBD: Do something with password
    #
    my $password = $ui->passwordLineEdit->text();


    #
    # Respect build checkbox
    #

    if (! $ui->checkoutBuildCheckBox->isChecked() ) {
        $command .= " --nobuild ";
    }

    my $w = awb_runlog();

    $w->run($command);

    #
    # Update package list after checkout
    #   TBD: This should wait for the command to finish
    #   TBD: Maybe we should just refresh the entire box
    #

    my $status = $w->wait();

    if ( $status) {
        Qt::MessageBox::information(
             this, 
             "awb", 
            "Checkout failed!\n");
    }

    #
    # TBD: setupInit was is causing crashes...
    #      so we just use the refresh action
    #
#    setupInit();

    refreshPushButton_clicked();
}


#
# Packages Group
#

sub updatePackagesListWidget_itemClicked
{
    my $ui = ui();
    $ui->updateAllCheckBox->setChecked(0);
}



sub packagesBrowsePushButton_clicked
{
    my $ui = ui();
    my $max = scalar @{$ui->updatePackagesListWidget()->selectedItems()};

    for (my $i=0; $i < $max; $i++) {
        my $item = $ui->updatePackagesListWidget->selectedItems->[$i];

        updatePackagesListWidget_doubleClicked($item);
    }

}


sub updatePackagesListWidget_doubleClicked
{
    my $item   = shift;
    my $packagename = $item->text();
    my $packageDB = Asim::Package::DB->new();
    my $package = $packageDB->get_package($packagename) || return undef;

    $package->browse();
}


sub updateAllCheckBox_toggled
{
    my $checked = shift;
    my $ui = ui();

    if ($checked) {
        my $max = $ui->updatePackagesListWidget->count();

        for (my $i=0; $i < $max; $i++) {
            $ui->updatePackagesListWidget->item($i)->setSelected(0);
        }
    }
    
    #
    # Restore checkbox state
    #
    $ui->updateAllCheckBox->setChecked($checked);

}

sub refreshPushButton_clicked
{
    # Reopen the workspace and refresh the package list
    #    TBD: Maybe this should be a method in Asim:: & Asim::Workspace
    
    my $workspace = Asim::rootdir();
    Asim::open($workspace);

    packagesInit();
}

sub updatePushButton_clicked
{
    my $command = "asim-shell --batch -- ";
    my $ui = ui();

    my $update = $ui->packageUpdateRadioButton->isChecked();
    my $commit = $ui->packageCommitRadioButton->isChecked();
    my $delete = $ui->packageDeleteRadioButton->isChecked();
    my $status = $ui->packageStatusRadioButton->isChecked();
    my $build  = $ui->packageBuildRadioButton->isChecked();
    my $clean  = $ui->packageCleanRadioButton->isChecked();

    if ($update) {
        $command .= "update package";
    }

    if ($commit) {
        $command .= "commit package";
    }

    if ($delete) {
        if ($ui->updateAllCheckBox->isChecked()) {
           Qt::MessageBox::information(
             this, 
             "awb", 
            "You cannot delete all packages\n");
           return;
        }

        my $status = Qt::MessageBox::warning ( 
            this, 
            "awb", 
            "Are you sure you want to delete the selected packages?",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        return  if ($status == 1);
        return  if ($status == 2);

        $command .= "delete package";
    }

    if ($status) {
        $command .= "status package";
    }

    if ($build) {
        $command .= "build package";
    }

    if ($clean) {
        $command .= "clean package";
    }

    if ($ui->updateAllCheckBox->isChecked()) {
        $command .= " all";
    } else {
        my $max = scalar @{$ui->updatePackagesListWidget->selectedItems()};
        my $count = 0;

        for (my $i=0; $i < $max; $i++) {
            my $item = $ui->updatePackagesListWidget->selectedItems->[$i];
            my $name = $item->text();
 
           $command .= " $name";
        }
    }

    if ($update && ! $ui->updateBuildCheckBox->isChecked()) {
        $command .= " --nobuild ";
    }

    my $w = awb_runlog();

    $w->run($command);

    #
    # Package list needs to be updated on delete
    #

    if ($delete) {
        my $status = $w->wait();

        if ( $status) {
            Qt::MessageBox::information(
                            this, 
                            "awb", 
                            "Delete failed!\n");
        }


        # Clean up package list - this does more than that, but computes are chaap...

       packagesInit();
    }
}



sub setupInit
{
    #
    # Set up the workspace group
    #
    my $workspacedir = Asim::rootdir();
    my $workspacebase = dirname($workspacedir);
    my $ui = ui();

    if (! -w $workspacebase) {
        $workspacebase = $ENV{HOME};
    }

    $ui->workspaceDirLineEdit->setText("");
    $ui->workspaceDirLineEdit->setText($workspacebase);

    reposInit();

    packagesInit();
}




sub reposInit
{

    my $repoDB = Asim::Repository::DB->new();
    my $ui = ui();

    #
    # Set up repository list
    #
    $ui->checkoutListWidget->clear();
    $ui->repoVersionComboBox->clear();

    #
    # Keep a hash of repos each of which is a hash of $ui->versions
    #
    this->{repos} = {};

    my @repodirs = $repoDB->directory();

    foreach my $p (@repodirs) {

        if ($p =~ /^[^\/]*$/) {
            this->{repos}->{$p} = {};
            $ui->checkoutListWidget->addItem($p);
        }

        if ($p =~ /^(.*)\/(.*)/) {
            this->{repos}->{$1}->{$2} = 1;
        }
    }


    #
    # Set up bundles list
    #

    $ui->checkoutBundleListWidget->clear();
    $ui->bundleVersionComboBox->clear();

    #
    # Keep a hash of bundles each of which is a hash of $ui->versions
    #

    this->{bundles} = {};

    my @bundledirs = $repoDB->bundle_directory();
    my @versions;

    foreach my $p (@bundledirs) {
	    $ui->checkoutBundleListWidget->addItem($p);

        this->{bundles}->{$p} = {};

	    @versions = $repoDB->bundle_ids($p);

    	foreach my $v (@versions) {
                this->{bundles}->{$p}->{$v} = 1;
	    }
    }
}


sub packagesInit
{
    #
    # Set up packages list
    #
    my $packageDB = Asim::Package::DB->new();
    my @packages = $packageDB->directory();
    my $ui = ui();

    $ui->updatePackagesListWidget->clear();

    foreach my $p (@packages) {
        $ui->updatePackagesListWidget->addItem($p);
    }


}

sub helpWhatsThisAction_activated 
{
    Qt::WhatsThis::enterWhatsThisMode();
}

sub pushBusyCursor
{
  Qt::Application::setOverrideCursor(Qt::Cursor(Qt::WaitCursor()));
}

sub popBusyCursor
{
  Qt::Application::restoreOverrideCursor();
}

#
# Todo:
#
#      Refresh on regain focus
#
#      Force unselect on *_list_clicked(undef).
#
#      Allow copy of model/benchmark filename to paste buffer
#      Persistent options (Qsettings)
#           Model
#           Benchmark
#           ADF file
#
#      Suport for multiple benchmarks, ala awb-run
#
#      Show benchmark description
#
#      Provide step-by-step instructions...
#      More intelligent menu/button enable/disable
#
#      Refine $ui->splitter sizes/layout
#      Leave manual page at top of window rather than bottom
#
#      Return correct status on runs in awb_runlog...
#
#      Support for viewing run outputs:
#           - stats (Explorer)
#           - strip charts (StripChartViewer)
#           - events (2Dreams)
# BUGS:
#      Sometimes {model,bencmark}_tree_expanded not called!
#
#
