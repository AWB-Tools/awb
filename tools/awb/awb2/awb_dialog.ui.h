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


#
#   Creation of window pane contents
#

void awb_dialog::init()
{
    use File::Basename;
    use Asim;

    use awb_about;
    use awb_runlog;


    my $public_root;
    my $workspace;
    my $root;


#    our $stepno = 1;
    our @steps;

    print "awb_dialog::init - We are in init\n" if $::debug;

    $workspace = $Asim::default_workspace->rootdir();
    chdir $workspace;

    setCaption(basename($workspace) . " - awb");

    $steps[0] = "";
    $steps[1] = "Step 1: Select a model and click 'Configure'";
    $steps[2] = "Step 2: Select build options on click 'Build'";
    $steps[3] = "Step 3: Selcect a benchmark and click 'Setup'";
    $steps[4] = "Step 4: Select run options and click 'Run'";
    
#    Steps->setText($steps[0]);

    # Clear model info

    model_tree->clear();
    model_list->clear();

    # Build model tree

    $public_root = Qt::ListViewItem(model_tree, undef);
    $public_root->setExpandable(1); 
    $public_root->setText(0, trUtf8("Models"));

    $root = "config/pm";
    print "awb_dialog::init - Model root=$root\n" if $::debug;
    $public_root->setText(1, trUtf8($root));


    # Clear benchmark info

    benchmark_tree->clear();
    benchmark_list->clear();

    # Build benchmark tree

    $public_root = Qt::ListViewItem(benchmark_tree, undef);
    $public_root->setExpandable(1); 
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

}


#
# Exit program
#

void awb_dialog::Exit_activated()
{
    Qt::Application::exit();
}

#
# Expansion/collapse of model explorer tree
#
void awb_dialog::model_tree_expanded( QListViewItem * )
{
    my $item = shift;
    my $name = $item->text(1);
    
    print "awb_dialog::model_tree_expanded - Expand=$name\n" if $::debug;

    # Delete existing children if any

    my $myChild = $item->firstChild();
    while( defined($myChild) ) {
        my $nextChild = $myChild->nextSibling();
        $item->takeItem($myChild);
        $myChild = $nextChild;
    }

    # Reconstruct the list

    for my $i (awb_util::glob_dir($name)) {
        my $citem = Qt::ListViewItem($item, undef);
        $citem->setText(0, trUtf8(basename($i)));
        $citem->setText(1, trUtf8($i));

        my $n = awb_util::glob_dir($i);
        $citem->setExpandable($n > 0);
    }

}


void awb_dialog::model_tree_collapsed( QListViewItem * )
{
    print "Item collapsing\n" if $::debug;
}


#
# Selection of a model in model explorer tree
#


void awb_dialog::model_tree_selectionChanged( QListViewItem * )
{
    model_tree_clicked(shift);
}

void awb_dialog::model_tree_returnPressed( QListViewItem * )
{
    model_tree_clicked(shift);
}


void awb_dialog::model_tree_onItem( QListViewItem * )
{
    my $item = shift;

    # Force model tree expansion
    if (!$item->isOpen()) {
        model_tree_expanded($item);
    }
}

void awb_dialog::model_tree_clicked( QListViewItem * )
{
    my $item = shift || return;
    my $name = $item->text(1);

    print "awb_dialog::model_tree_clicked - Item = $name\n" if $::debug;

    # Force model tree expansion
    if (!$item->isOpen()) {
        model_tree_expanded($item);
    }

    # Add in the models in this directory in model pane

    model_list->clear();

    for my $i (awb_util::glob_apm($name)) {
        my $citem = Qt::ListViewItem(model_list, undef);
        $citem->setExpandable(1);

        # Column 0 = model name
        $citem->setText(0, trUtf8(basename($i,".apm")));

        # Column 1 = model file path
        $citem->setText(1, trUtf8($i));
    }

	    
}

#
# Selection of a model in model list
#
void awb_dialog::model_list_selectionChanged( QListViewItem * )
{
    model_list_clicked(shift);
}


void awb_dialog::model_list_clicked( QListViewItem * )
{
    my $item = shift;
    my $name;
    my $file;
    my $model = undef;
    my $model_enable;
    my $model_options;

    # Clear old model info

    model_name->setText("");
    model_description->clear();

    if (defined($item)) {
        # Get model name and file

        $name = $item->text(0);
        $file = $item->text(1);

        $model = awb_util::set_model($file);

        # If there is a model describe it

        if (defined($model)) {
            model_name->setText($name);

            # Create description

            model_description->insertItem("Name: " . $model->name());
            model_description->insertItem("Description: " . $model->description());
            model_description->insertItem("File: " . $model->filename());
            model_description->insertItem("Default build directory: " . $model->build_dir());
            my @dep = ($model->dependencies());
            model_description->insertItem("Dependencies: @dep");
            model_description->insertItem("Save Parameters: " . $model->saveparams());

            # Add model build options

            $model_options = $model->build_options();

            buildOptions->clear();
            for my $o (keys %{$model_options}) {
                Qt::ListBoxText(buildOptions, $o . " - " . $model_options->{$o});
            }
        }
    }

    # Enable/disable actions as appropriate

    nextStep_models->setEnabled(0);
    nextStep_buildopts->setEnabled(0);
    nextStep_benchmarks->setEnabled(0);
    nextStep_parameters->setEnabled(0);
    nextStep_runopts->setEnabled(0);
    nextStep_analysis->setEnabled(0);
    
    $model_enable = defined($model);

    EditModel->setEnabled($model_enable);
    CleanModel->setEnabled($model_enable);
    ConfigureModel->setEnabled($model_enable);
    BuildModel->setEnabled($model_enable);

    Button_nuke->setEnabled($model_enable);
    Button_clean->setEnabled($model_enable);
    Button_config->setEnabled($model_enable);
    Button_build->setEnabled($model_enable);


    # Clear benchmarks

    # TBD - unselect benchmark in pane...
    benchmark_list_clicked(undef);

    # Clear parameters
    paramList->clear();

}


void awb_dialog::model_list_contextMenuRequested( QListViewItem *, const QPoint &, int )
{
    my $item = shift;
    my $point = shift;

    modelMenu->popup($point);
    modelMenu->exec();
}

#
# Benchmark selection
#

void awb_dialog::benchmark_tree_expanded( QListViewItem * )
{
    my $item = shift;
    my $name = $item->text(1);
    
    print "awb_dialog::benchmark_tree_expanded - Expand=$name\n" if $::debug;

    # Delete existing children if any

    my $myChild = $item->firstChild();
    while( defined($myChild) ) {
        my $nextChild = $myChild->nextSibling();
        $item->takeItem($myChild);
        $myChild = $nextChild;
    }

    # Reconstruct the list

    for my $i (awb_util::glob_dir($name)) {
        my $citem = Qt::ListViewItem($item, undef);
        $citem->setText(0, trUtf8(basename($i)));
        $citem->setText(1, trUtf8($i));

        my $n = awb_util::glob_dir($i);
        $citem->setExpandable($n > 0);
    }

}


void awb_dialog::benchmark_tree_collapsed( QListViewItem * )
{
    print "Item collapsing\n" if $::debug;
}


void awb_dialog::benchmark_tree_selectionChanged( QListViewItem * )
{
    benchmark_tree_clicked(shift);
}

void awb_dialog::benchmark_tree_returnPressed( QListViewItem * )
{
    benchmark_tree_clicked(shift);
}


void awb_dialog::benchmark_tree_onItem( QListViewItem * )
{
    my $item = shift;

    # Force benchmark tree expansion
    if (!$item->isOpen()) {
        benchmark_tree_expanded($item);
    }
}


void awb_dialog::benchmark_tree_clicked( QListViewItem * )
{
    my $item = shift || return;
    my $name = $item->text(1);

    print "awb_dialog::benchmark_tree_clicked - Item = $name\n" if $::debug;

    # Force benchmark tree expansion
    if (!$item->isOpen()) {
        benchmark_tree_expanded($item);
    }

    # Add in the benchmarks in this directory

    benchmark_list->clear();

    for my $i (awb_util::glob_abm($name)) {
        my $citem = Qt::ListViewItem(benchmark_list, undef);
        $citem->setExpandable(1);
        $citem->setText(0, trUtf8(basename($i,".apm")));
        $citem->setText(1, trUtf8($i));
    }

}




void awb_dialog::benchmark_list_selectionChanged( QListViewItem * )
{
    benchmark_list_clicked(shift);
}


void awb_dialog::benchmark_list_clicked( QListViewItem * )
{
    my $item = shift;
    my $name;
    my $file;
    my $benchmark = undef;
    my $benchmark_enable;

    # Clear out old benchmark

    benchmark_name->setText("");
    benchmark_description->clear();

    awb_util::set_benchmark(undef);

    if (defined($item)) {
        $name = $item->text(0);
        $file = $item->text(1);

        $benchmark = awb_util::set_benchmark($file);

        if (defined($benchmark)) {
            benchmark_name->setText($name);
            benchmark_description->insertItem("Name: $name");
            # TBD - benchmark information
        }
    }


    $benchmark_enable = defined($benchmark)
                        && Button_build->isEnabled();

    EditBenchmark->setEnabled($benchmark_enable);
    SetupBenchmark->setEnabled($benchmark_enable);
    RunBenchmark->setEnabled($benchmark_enable);

    Button_setup->setEnabled($benchmark_enable);
    Button_run->setEnabled($benchmark_enable);

    # Refresh the parameters list
    #   Does not work before benchmark is set up

    # paramRefresh_clicked();    
}




void awb_dialog::benchmark_list_contextMenuRequested( QListViewItem *, const QPoint &, int )
{
    my $item = shift;
    my $point = shift;

    benchmarkMenu->popup($point);
    benchmarkMenu->exec();

}





void awb_dialog::model_list_doubleClicked( QListViewItem * )
{
    awb_util::edit_model(this);
}



#
# Wizard buttons
#

void awb_dialog::nextStep_models_clicked()
{
   Tabs_main->showPage(tab_2);
}


void awb_dialog::nextStep_buildopts_clicked()
{
   Tabs_main->showPage(tab_3);
}


void awb_dialog::nextStep_benchmarks_clicked()
{
   Tabs_main->showPage(tab_4);
}


void awb_dialog::nextStep_parameters_clicked()
{
   Tabs_main->showPage(tab_5);
}


void awb_dialog::nextStep_runopts_clicked()
{
   Tabs_main->showPage(tab_6);
}


void awb_dialog::nextStep_analysis_clicked()
{
    model_list_clicked(undef);
    benchmark_list_clicked(undef);

    nextStep_models->setEnabled(0);
    nextStep_buildopts->setEnabled(0);
    nextStep_benchmarks->setEnabled(0);
    nextStep_parameters->setEnabled(0);
    nextStep_runopts->setEnabled(0);
    nextStep_analysis->setEnabled(0);
    
    Tabs_main->showPage(tab);
}

#
# Button Actions
# 
void awb_dialog::Refresh_activated()
{
    my $item;

    if ($item = model_tree->selectedItem()) {
        model_tree_clicked($item);
    }

    if ($item = benchmark_tree->selectedItem()) {
        benchmark_tree_clicked($item);
    }
}


void awb_dialog::NewModel_activated()
{
    awb_util::new_model();
}


void awb_dialog::EditModel_activated()
{
    awb_util::edit_model();
}




void awb_dialog::Button_nuke_clicked()
{
    awb_util::nuke_model()
        ||  Qt::MessageBox::information(
            this, 
            "nuke_model",
            "Model nuke failed\n");
}

void awb_dialog::Button_config_clicked()
{
    my $status;

    # BUG - This does not work after subwindow creation!!!!
    nextStep_models->setEnabled(1);

    $status = awb_util::config_model();
    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "config_model",
            "Model configuration failed\n");
        return;
    }

}


void awb_dialog::Button_clean_clicked()
{
    awb_util::clean_model()
        ||  Qt::MessageBox::information(
            this, 
            "clean_model",
            "Model clean failed\n");
}


void awb_dialog::Button_build_clicked()
{
    my $status;

    # BUG - This does not work after subwindow creation!!!!
    nextStep_buildopts->setEnabled(1);

    # Force switch setup

    extraBuildSwitches_textChanged();

    $status = awb_util::build_model();
    if (! defined($status)) {
        Qt::MessageBox::information(
            this, 
            "build_model",
            "Model build failed\n");
        return;
    }

}





void awb_dialog::NewBenchmark_activated()
{
    awb_util::new_benchmark();
}


void awb_dialog::EditBenchmark_activated()
{
    awb_util::edit_benchmark();

}


void awb_dialog::Button_setup_clicked()
{
    my $status;

    # BUG - This does not work after subwindow creation!!!!
    nextStep_benchmarks->setEnabled(1);
    nextStep_parameters->setEnabled(1);

    $status = awb_util::setup_benchmark();
    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "setup_benchmark",
            "Benchmark setup failed\n");
        return;
    }

}

void awb_dialog::Button_run_clicked()
{
    my $status;

    # BUG - This does not work after subwindow creation!!!!
    nextStep_runopts->setEnabled(1);
    nextStep_analysis->setEnabled(1);

    # Force run options to be set

    extraRunSwitches_textChanged();
    
    $status = awb_util::run_model();
    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "run_model",
            "Model run failed\n");
        return;
    }
}


void awb_dialog::FindReplace_activated()
{
    system("apm-find-replace &");
}


void awb_dialog::Manual_activated()
{
    my $w = awb_runlog(0,0,1);

    return $w->run("awb2 --help");
}


void awb_dialog::About_activated()
{
    my $a = awb_about();
    $a->exec();
}


#
# Build Options page
#     I do not like the fact that this module knows the actual make switches....
#



void awb_dialog::parallelMake_clicked( int )
{
    extraBuildSwitches_textChanged();
}


void awb_dialog::compilerType_clicked( int )
{
    extraBuildSwitches_textChanged();
}

void awb_dialog::documentation_clicked( int )
{
    extraBuildSwitches_textChanged();
}

void awb_dialog::buildType_clicked( int )
{
    extraBuildSwitches_textChanged();
}



void awb_dialog::extraBuildSwitches_textChanged( const QString & )
{
    my $options = "";

    if (documentationYes->isOn()) {
        $options .= "dox";
    }

    if (parallelNo->isOn()) {
        $options .= " PAR=0";
    }

    if (parallelYes->isOn()) {
        $options .= " PAR=1";
    }

    if (compilerGEM->isOn()) {
        $options .= " GNU=0";
    }

    if (compilerGNU->isOn()) {
        $options .= " GNU=1";
    }

    if (buildOpt->isOn()) {
        $options .= " DEBUG=0 OPT=1";
    }

    if (buildDebug->isOn()) {
        $options .= " DEBUG=1 OPT=0";
    }

    for my $i (1..buildOptions->count()) {
        if (buildOptions->isSelected($i-1)) {
            my $entry = buildOptions->text($i-1);
	    if ($entry =~ /<T>$/) {
		$entry =~ s/ .*//;
		$options .= " $entry";
	    } else {
		$entry =~ s/ .*//;
		$options .= " $entry=1";
	    }
        }
    }

    $options .= " " . extraBuildSwitches->text();

    awb_util::set_build_switches($options);
}

#
# Dynamic parameters page
#


void awb_dialog::paramRefresh_clicked()
{
    paramList->clear();

    for my $i (awb_util::get_params_model()) {
        paramList->insertItem("$i");
    }
}


void awb_dialog::paramList_selectionChanged( QListBoxItem * )
{
    my $item = shift;
    my $line = $item->text();

    if ($line =~ "^([^=]+) = ([^ ]+) ") {
        paramName->setText($1);
        paramValue->setEnabled(1);
        paramValue->setText($2);
        paramUpdate->setEnabled(1);
    } else {
        paramName->setText("");
        paramValue->setEnabled(0);
        paramValue->setText("");
        paramUpdate->setEnabled(0); 
   }

}

void awb_dialog::paramValue_returnPressed()
{
    paramUpdate_clicked();
}


void awb_dialog::paramUpdate_clicked()
{
    my $itemno = paramList->currentItem();
    my $line = paramList->text($itemno);

    #
    # Parse out information on parameter
    #    Note: implicit dependence on format of line
    #
    $line =~ "([^=]+) = ([^ ]+) [[](.+)[]]";

    my $name = $1;
    if (!defined($name)) {
        die("Illegally formatted parameter line");
    }

    my $default = $3;
    if (!defined($default)) {
        print("Illegally formatted parameter line");
        $default = $2;
    }       

    my $value = paramValue->text() || $3;
    
    #
    # Update information on parameter
    #    Note: implicit dependence on format of line
    #
    paramList->changeItem("$name = $value [$default]", $itemno);

}

#
# Run Options page
#     I do not like the fact that this module knows the actual run switches....
#


void awb_dialog::traceRun_clicked( int )
{
    extraRunSwitches_textChanged();
}


void awb_dialog::extraRunSwitches_textChanged( const QString & )
{
    my $options = "";
    my $data;

    if ($data = endCycle->text()) {
        $options .= " -c $data";
    }

    if ($data = endInstruction->text()) {
        $options .= " -i $data";
    }

    if (traceYes->isOn()) {
        $options .= " -t";
    }

    if ($data = startTrace->text()) {
        $options .= " -tsc $data";
    }

    if ($data = endTrace->text()) {
        $options .= " -tec $data";
    }

    if (eventlogYes->isOn()) {
        $options .= " -e";
    }

    if ($data = startEventlog->text()) {
        $options .= " -esc $data";
    }

    if ($data = endEventlog->text()) {
        $options .= " -eec $data";
    }

    $options .= " " . extraRunSwitches->text();

    #
    # Pick up parameter updates
    #
    for my $i (1..paramList->count()) {
        #
        # Parse out information on parameter
        #    Note: implicit dependence on format of line
        #
        my $line=paramList->text($i-1);

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
void awb_dialog::viewStatistics_clicked()
{
    awb_util::view_statistics();
}


void awb_dialog::viewStripChart_clicked()
{
    awb_util::view_stripchart();
}


void awb_dialog::viewCycleDisplay_clicked()
{
    my $adf = adfPath->text();

    awb_util::view_cycledisplay($adf);
}


void awb_dialog::browseAdfPath_clicked()
{
    my $cwd;

    chomp($cwd = `pwd`);
    my $s = shift ||
        Qt::FileDialog::getOpenFileName(
            $cwd,
            "ADF file (*.xml)",
            this,
            "open ADF file dialog",
            "Choose an ADF file" ) ||
        return;;

    adfPath->setText($s);
}


#
# Setup tab
#



#
# Workspace group
#

void awb_dialog::workspaceDirLineEdit_textChanged( const QString & )
{
    my $dir = shift;

    if ( -d $dir) {
        my $workspacename = basename($Asim::default_workspace->rootdir());

        workspaceComboBox->clear();

        for my $i (glob("$dir/*/awb.config")) {
            my $n = basename(dirname($i));

            workspaceComboBox->insertItem($n);
            if ($n eq $workspacename) {
                workspaceComboBox->setCurrentItem(workspaceComboBox->count()-1);
            }
        }
    }

}

void awb_dialog::workspaceBrowsePushButton_clicked()
{
    my $dir = Qt::FileDialog::getExistingDirectory(
                    workspaceDirLineEdit->text(),
                    this,
                    "get existing directory",
                    "Get directory to hold workspace");

    if ($dir) {
        workspaceDirLineEdit->setText($dir);
    }

    return;
}



void awb_dialog::workspaceNameLineEdit_returnPressed()
{
    workspaceCreatePushButton_clicked();
}

void awb_dialog::workspaceCreatePushButton_clicked()
{
    my $dir = workspaceDirLineEdit->text();
    if (! -d $dir) {
        Qt::MessageBox::information(
            this, 
            "awb", 
            "Root directory for workspace must exist\n");
        return;
    }

    my $workspacename = workspaceNameLineEdit->text();
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

    my $w = awb_runlog(0,0,1);

    #
    # Debugging hack...
    #
    if ($workspacename =~ /^% (.*)/) {
        $command = $1;
        print $w->run($command);
        workspaceNameLineEdit->setText("% ");
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

    workspaceNameLineEdit->clear();
    return;
}


void awb_dialog::workspaceComboBox_activated( const QString & )
{
#    This behavior seemed annoying
#
#    workspaceSwitchPushButton_clicked();
}

void awb_dialog::workspaceSwitchPushButton_clicked()
{

    my $dir = workspaceDirLineEdit->text();
    my $workspace = "$dir/" . workspaceComboBox->currentText();

    if (Asim::open($workspace)) {
        init();
    }
}



#
# Repository Group
#



void awb_dialog::checkoutListBox_highlighted( const QString & )
{
    my $repo = shift;

    repoVersionComboBox->clear();
    repoVersionComboBox->insertItem("<Optionally select alternate version>");

    for my $v (sort keys  %{this->{repos}->{$repo}}) {
        repoVersionComboBox->insertItem($v);
    }
}


void awb_dialog::checkoutPushButton_clicked()
{
    my $command = "asim-shell --batch -- checkout package";

    my $w = awb_runlog(0,0,1);

    #
    # Get name of package to check out
    #
    my $i = checkoutListBox->currentItem();
    my $item = checkoutListBox->item($i);
    return if (! defined($item));
    my $package = $item->text();

    my $version = repoVersionComboBox->currentText();

    if (! $version =~ /<.*>/) {
        $package .= "/$version";
    }

    $command .= " $package";

    #
    # Optionally add uesrname
    #
    my $username = usernameLineEdit->text();
    print "$username\n";
    if ($username ne "") {
        $command .= " --user=$username";
    }

    #
    # TBD: Do something with password
    #
    my $password = passwordLineEdit->text();


    #
    # Respect build checkbox
    #

    if (! checkoutBuildCheckBox->isChecked() ) {
        $command .= " --nobuild ";
    }

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
    # TBD: This is causing crashes...
    #
#    setupInit();

}


#
# Packages Group
#

void awb_dialog::updatePackagesListBox_selectionChanged()
{
    updateAllCheckBox->setChecked(0);
}

void awb_dialog::updatePackagesListBox_selected( QListBoxItem * )
{
    updateAllCheckBox->setChecked(0);
}


void awb_dialog::updateAllCheckBox_toggled( bool )
{
    my $checked = shift;

    if ($checked) {
        my $max = updatePackagesListBox->count();

        for (my $i=0; $i < $max; $i++) {
            updatePackagesListBox->setSelected($i,0);
        }
    }
    
    #
    # Restore checkbox state
    #
    updateAllCheckBox->setChecked($checked);

}

void awb_dialog::refreshPushButton_clicked()
{
    # Reopen the workspace and refresh the package list
    #    TBD: Maybe this should be a method in Asim:: & Asim::Workspace

    my $workspace = Asim::rootdir();
    Asim::open($workspace);

    packagesInit();
}

void awb_dialog::updatePushButton_clicked()
{
    my $command = "asim-shell --batch -- ";

    my $update = packageUpdateRadioButton->isChecked();
    my $commit = packageCommitRadioButton->isChecked();
    my $delete = packageDeleteRadioButton->isChecked();
    my $build  = packageBuildRadioButton->isChecked();
    my $clean  = packageCleanRadioButton->isChecked();

    if ($update) {
        $command .= "update package";
    }

    if ($commit) {
        $command .= "commit package";
    }

    if ($delete) {
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

    if ($build) {
        $command .= "build package";
    }

    if ($clean) {
        $command .= "clean package";
    }

    if (updateAllCheckBox->isChecked()) {
        if (! $update) {
           Qt::MessageBox::information(
             this, 
             "awb", 
            "Only update can be performed on all packages\n");
           return;
        }

        $command .= " all";
    } else {
        my $max = updatePackagesListBox->count();
        my $count = 0;

        for (my $i=0; $i < $max; $i++) {
            if (updatePackagesListBox->isSelected($i)) {
                my $item = updatePackagesListBox->item($i);
                my $name = $item->text();

                $command .= " $name";
            }
        }
    }

    if ($update && ! updateBuildCheckBox->isChecked()) {
        $command .= " --nobuild ";
    }

    my $w = awb_runlog(0,0,1);

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



void awb_dialog::setupInit()
{
    #
    # Set up the workspace group
    #
    my $workspacedir = Asim::rootdir();
    my $workspacebase = dirname($workspacedir);

    if (! -w $workspacebase) {
        $workspacebase = $ENV{HOME};
    }

    workspaceDirLineEdit->setText("");
    workspaceDirLineEdit->setText($workspacebase);

    reposInit();

    packagesInit();
}




void awb_dialog::reposInit()
{

    #
    # Set up repository list
    #
    my $repoDB = Asim::Repository::DB->new();
    my @repodirs = $repoDB->directory();

    checkoutListBox->clear();
    repoVersionComboBox->clear();

    this->{repsos} = {};

    #
    # Keep a hash of repos each of which is s hash of versions
    #
    foreach my $p (@repodirs) {

        if ($p =~ /^[^\/]*$/) {
            this->{repos}->{$p} = {};
            checkoutListBox->insertItem($p);
        }

        if ($p =~ /^(.*)\/(.*)/) {
            this->{repos}->{$1}->{$2} = 1;
        }
    }
}


void awb_dialog::packagesInit()
{
    #
    # Set up packages list
    #
    my $packageDB = Asim::Package::DB->new();
    my @packages = $packageDB->directory();

    updatePackagesListBox->clear();

    foreach my $p (@packages) {
        updatePackagesListBox->insertItem($p);
    }


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
#      Suport for multiple benchmarks, ala asim-run
#
#      Show benchmark description
#
#      Provide step-by-step instructions...
#      More intelligent menu/button enable/disable
#
#      Refine splitter sizes/layout
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


