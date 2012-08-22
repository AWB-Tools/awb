package apm_find_replace;

use strict;
use warnings;

use QtCore4;
use QtGui4;
use QtCore4::isa qw( Qt::MainWindow );
use QtCore4::slots
    fileNew => [],
    fileExit => [],
    editFind => [],
    moduleEdit_activated => [],
    helpAbout => [],
    newPushButton_clicked => [],
    findModuleTypeComboBox_activated => ['const QString&'],
    findPushButton_clicked => [],
    filterLineEdit_returnPressed => [],
    filterPushButton_clicked => [],
    attrLineEdit_returnPressed => [],
    attrPushButton_clicked => [],
    filterModuleTypeComboBox_activated => ['const QString&'],
    typePushButton_clicked => [],
    modulePushButton_clicked => [],
    brokenPushButton_clicked => [],
    replaceModuleTypeComboBox_activated => ['const QString&'],
    ModulesComboBox_activated => ['const QString&'],
    replacePushButton_clicked => [],
    savePushButton_clicked => [],
    renamePushButton_clicked => [],
    save2PushButton_clicked => [],
    replaceParamModuleTypeComboBox_activated => ['const QString&'],
    replaceParamModuleComboBox_activated => ['const QString&'],
    replaceParamComboBox_activated => ['const QString&'],
    replaceParamReplacePushButton_clicked => [],
    replaceParamSavePushButton_clicked => [],
    allPushButton_clicked => [],
    invertPushButton_clicked => [],
    purgePushButton_clicked => [],
    modelsListBox_doubleClicked => ['QListWidgetItem*'],
    modelEditAction_activated => [],
    clearPushButton_clicked => [],
    logListBox_doubleClicked => ['QListWidgetItem*'],
    movedPushButton_clicked => [],
    helpWhatsThisAction_activated => [];
    

sub ui
{
    return this->{ui};
}

sub NEW 
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_ApmFindReplace->setupUi(this);
}

sub init
{
    newPushButton_clicked();

    # create context menu
    ui()->modelsListBox->addAction(ui()->modelEditAction());
}

#
# Menus
#

sub fileNew
{
    newPushButton_clicked();
}


sub fileExit
{
    Qt::Application::exit();
}



sub editFind
{
    findPushButton_clicked();
}



sub moduleEdit_activated
{

}


sub helpAbout
{
    my $message;

    $message  = "Apm-find-replace - bulk module replacement\n";
    $message .= "\n";
    $message .= "For help type: apm-find-replace --help\n";
    
    Qt::MessageBox::information(
              this, 
              "$0 help",
              $message);
}

sub searchModelMap
{
    return this->{searchModelMap};
}

sub searchModuleType
{
    return this->{searchModuleType};
}

sub filterModuleMap
{
    return this->{filterModuleMap};
}

sub replacementModuleMap
{
    return this->{replacementModuleMap};
}

sub replaceParamModuleMap
{
    return this->{replaceParamModuleMap};
}

sub replacementModule
{
    return this->{replacementModule};
}

sub replaceParamMap
{
    return this->{replaceParamMap};
}

sub replaceParamModule
{
    return this->{replaceParamModule};
}

sub replaceParam
{
    return this->{replaceParam};
}

#
#  ui()->Find Tab
#

sub newPushButton_clicked
{
    #
    # Set up ui()->widgets in find tab
    #

    my $moduleDB = Asim::Module::DB->new();
    my @awbtypes = $moduleDB->module_types();

    my  $awbtype = undef;
    this->{searchModuleType} = $awbtype;

    this->{searchModelMap} = {};

    ui()->rootdirLineEdit->setText("config/pm");

    ui()->findModuleTypeComboBox->clear();
    ui()->findModuleTypeComboBox->addItem("<Select a module type to search for>");

    for my $i (sort(@awbtypes)) {
      ui()->findModuleTypeComboBox->addItem($i);
    }

    ui()->rootCheckBox->setChecked(0);
    ui()->invertCheckBox->setChecked(0);

    ui()->modelsListBox->clear();

    #
    # Clear ui()->widgets in filter tab
    #
    this->{filterModuleMap} = {};

    ui()->filterLineEdit->setText("");
    ui()->attrLineEdit->setText("");
    
    ui()->filterModuleTypeComboBox->clear();
    ui()->filterModuleTypeComboBox->addItem("<Select a module type to search for>");
    for my $i (sort(@awbtypes)) {
      ui()->filterModuleTypeComboBox->addItem($i);
    }

    ui()->filterModuleComboBox->clear();



    #
    # Clear ui()->widgets in replace tab
    #
    this->{replacementModuleMap} = {};


    ui()->selectRadioButton->setChecked(1);
    ui()->filterInvertCheckBox->setChecked(0);


    ui()->replaceModuleTypeComboBox->clear();
    ui()->replaceModuleTypeComboBox->addItem("<Select a module type to replace>");
    for my $i (sort(@awbtypes)) {
      ui()->replaceModuleTypeComboBox->addItem($i);
    }


    ui()->replaceModuleComboBox->clear();

    my $module = undef;
    this->{replacementModule} = $module;

    ui()->moduleListBox->clear();

    #
    # Clear ui()->widgets in rename tab
    #

    ui()->inPatternLineEdit->setText("");
    ui()->outPatternLineEdit->setText("");

    #
    # Clear ui()->widgets in parameter tab
    #
    this->{replaceParamModuleMap} = {};


    ui()->replaceParamModuleTypeComboBox->clear();
    ui()->replaceParamModuleTypeComboBox->addItem("<Select a module type>");
    for my $i (sort(@awbtypes)) {
      ui()->replaceParamModuleTypeComboBox->addItem($i);
    }


    ui()->replaceParamModuleComboBox->clear();
    ui()->replaceParamComboBox->clear();

    my $module2 = undef;
    this->{replaceParamModule} = $module2;

    my $param = undef;
    this->{replaceParam} = $param;

    ui()->replaceParamListBox->clear();

    #
    # Clear ui()->widgets in log groupbox
    #

    ui()->logListBox->clear();
    ui()->statusTextBox->setText("");
}


sub findModuleTypeComboBox_activated
{
    my $awbtype = shift;

    #
    # Clear out ui()->widgets
    #

    ui()->modelsListBox->clear();

    ui()->moduleListBox->clear();

    #
    # Just return if this is the null choice
    #

    if ($awbtype =~ /^<.*>$/) {
      $awbtype = undef;
      this->{searchModuleType} = $awbtype;
      return;
    }

    this->{searchModuleType} = $awbtype;



}

sub findPushButton_clicked
{
    my $awbtype = searchModuleType();

    #
    # Clear out ui()->widgets
    #

    ui()->modelsListBox->clear();

    ui()->replaceModuleComboBox->clear();
    my $module = undef;
    this->{replacementModule} = $module;

    ui()->moduleListBox->clear();
    ui()->logListBox->clear();
    ui()->statusTextBox->setText("");
      
    #
    # Return without an awbtype
    #

    if (! defined($awbtype)) {
      ui()->logListBox->addItem("Error: a module type must be selected");
      return;
    }

    #
    # Fill models selection box with appropriate models
    #

    my $rootonly = ui()->rootCheckBox->isChecked();
    my $inverttest = ui()->invertCheckBox->isChecked();

    this->{searchModelMap} = {};

    findModels($awbtype, ui()->rootdirLineEdit->text(), $rootonly, $inverttest);

    my $hash1 = searchModelMap();
    my $count = 0;

    for my $i (sort(keys(%{$hash1}))) {
      ui()->modelsListBox->addItem($i);
      $count++;
    }

    #
    # Report what happened
    #

    ui()->statusTextBox->setText("$count models found");

}

#
# Modified version of method from Asim::Model::DB....
#
# TBD: Convert to keyword-style arguments
#

sub findModels
{
    my $awbtype = shift;
    my $dir = shift;
    my $rootonly = shift;
    my $inverttest = shift;

    #
    # Get the list of files in the current directory.
    #

    my @filenames = $Asim::default_workspace->listdir($dir);

    foreach my $fname (@filenames) {
      next if $fname eq '.';
      next if $fname eq '..';

      my $relname = "$dir/$fname";

      # remove ./ prefix...
      $relname =~ s/^\.\///;

      my $absname = $Asim::default_workspace->resolve($relname);

      if (!defined($absname)) {
        print("Filename unresolvable: $relname - possible dangling link\n");
        next;
      }

      if ( -d $absname ) {

        # Recursively scan for models

        findModels($awbtype, $relname, $rootonly, $inverttest);

      } elsif ( $fname =~ /.*\.apm$/) {

        #
        # Add the model....
        #

        my $model = Asim::Model->new($absname);

        if (! defined($model)) {
          print("Could not open model file: $absname\n");
          next;
        }

        #
        # Look if model needs a module of type $awbtype
        #
        my $provides;

        if ($rootonly) {
          $provides = $model->provides() eq $awbtype;
        } else {
          $provides = $model->contains_module_providing($awbtype);
        }

        if ($inverttest) {
          $provides = !$provides;
        }

        next if ! $provides;

        my $modelname = $model->name();
        my $count = 1;
        
        while (defined(searchModelMap()->{$modelname})) {
          ui()->logListBox->addItem("Error: Duplicate modelname - $modelname");
          $modelname =~ s/( - duplicate.*)*$/ - duplicate $count/;
          $count++;
        }

        searchModelMap()->{$modelname} = $model;
      }
    }
}


#
# Filter Tab
#

#
# Note: A number of these routines have massive replication of code
#       a common search function that takes a 'match' function as a
#       argument would be a much better approach
# 

sub filterLineEdit_returnPressed
{
    filterPushButton_clicked();
}

sub filterPushButton_clicked
{
    my $filter = ui()->filterLineEdit->text();

    #
    # Check (and select) model names that match regex
    #

    my $modelhash = models();
    my $count = 0;

    foreach my $i (reverse sort keys %{$modelhash}) {
      my $model = $modelhash->{$i};

      my $match;

      $match = $model->name() =~ /$filter/;      

      $count++ if $match;

      updateModelSelection($match, $i);
    }

    ui()->statusTextBox->setText("$count matches found");
}




sub attrLineEdit_returnPressed
{
    attrPushButton_clicked();
}


sub attrPushButton_clicked
{
    my $filter = ui()->attrLineEdit->text();

    #
    # Check (and select) model names with specified attribute
    #

    my $modelhash = models();
    my $count = 0;

    foreach my $i (reverse sort keys %{$modelhash}) {
      my $model = $modelhash->{$i};

      my @attributes = $model->attributes();
      my $match;
     
      $match = grep(/${filter}/, @attributes);
      $count++ if $match;

      updateModelSelection($match, $i);
    }

    ui()->statusTextBox->setText("$count matches found");
}



sub filterModuleTypeComboBox_activated
{
    my $awbtype = shift;

    ui()->filterModuleComboBox->clear();

    if (! defined($awbtype)) {
      return;
    }

    #
    # Fill module combobox with appropriate modules
    #

    this->{filterModuleMap} = {};
    
    #
    # Insert models
    #

    my $modelDB = Asim::Model::DB->new();
    my @models = $modelDB->find($awbtype);

    for my $i (sort @models) {
      filterModuleMap()->{$i->name() . " (submodel)"} = $i;
    }

    #
    # Insert modules
    #  

    my $moduleDB = Asim::Module::DB->new();
    my @modules = $moduleDB->find($awbtype);

    for my $i (sort @modules) {
      filterModuleMap()->{$i->name()} = $i;
    }


    #
    # Fill module filter combobox with appropriate modules
    #

    ui()->filterModuleComboBox->clear();
    ui()->filterModuleComboBox->addItem("<Filter models to a specific module>");

    my $hash1 = filterModuleMap();

    for my $i (sort(keys(%{$hash1}))) {
      ui()->filterModuleComboBox->addItem($i);
    }
}


sub typePushButton_clicked
{
    my $filter = ui()->filterModuleTypeComboBox->currentText();

    #
    # Check (and select) model names that match regex
    #

    my $modelhash = models();
    my $count = 0;

    foreach my $i (reverse sort keys %{$modelhash}) {
      my $model = $modelhash->{$i};

      my $match;
      
      #
      # Look for use of module
      #

      $match =  $model->contains_module_providing($filter);
      $count++ if $match;

      updateModelSelection($match, $i);
    }

    ui()->statusTextBox->setText("$count matches found");
}


sub modulePushButton_clicked
{
     my $modulename = ui()->filterModuleComboBox->currentText();
     if (! defined($modulename)) {
      print "No modulename\n";
      return;
    }

     my $module = filterModuleMap()->{$modulename};

    if (! defined($module)) {
        print "No such module\n";
      return;
    }

    my $filter1 = $module->provides();
    my $filter2 = $module->name();

    #
    # Check (and select) models containing specified module
    #

    my $modelhash = models();
    my $count = 0;

    foreach my $i (reverse sort keys %{$modelhash}) {
      my $model = $modelhash->{$i};

      my $match1;
      my $match2;

      $match1 = $model->find_module_providing($filter1);

      # Convert module to model if the module is the root of submodel

      if (defined($match1) && $match1->isroot() && $match1->owner() ne $model) {
        $match1 = $match1->owner();
      }
      
      if (defined($match1) && ($match1->name() eq $filter2)) {
        $match2 = 1;
      } else {
        $match2 = 0;
      }
      $count++ if $match2;
      
      updateModelSelection($match2, $i);
    }

    ui()->statusTextBox->setText("$count matches found");
}




sub brokenPushButton_clicked
{
    #
    # Check (and select) models that are broken
    #

    my $modelhash = models();
    my $count = 0;

    foreach my $i (reverse sort keys %{$modelhash}) {
      my $model = $modelhash->{$i};
      my $match;
      
      $match = $model->missing_module_count() > 0;
      $count++ if $match;

      updateModelSelection($match, $i);
    }

    ui()->statusTextBox->setText("$count matches found");
}


sub movedPushButton_clicked
{
    #
    # Check (and select) models that are broken
    #

    my $modelhash = models();
    my $count = 0;

    foreach my $i (reverse sort keys %{$modelhash}) {
      my $model = $modelhash->{$i};
      my $match;
      
      $match = $model->moved_module_count() > 0;
      $count++ if $match;

      updateModelSelection($match, $i);
    }

    ui()->statusTextBox->setText("$count matches found");

}


#
# Utility function to update the models selected
#

sub updateModelSelection
{
    my $match = shift;
    my @indices = @_;

    #
    # Get action
    #

    my $select = ui()->selectRadioButton->isChecked();
    my $add    = ui()->addRadioButton->isChecked();
    my $remove = ui()->removeRadioButton->isChecked();


    my $invert = ui()->filterInvertCheckBox->isChecked();

    #
    # Update selection for each item in the list
    #

    if ($invert) {
        $match = ! $match;
    }

    for my $i (@indices) {

      if ($match) {
        if ($select || $add) {
          ui()->modelsListBox->item($i)->setSelected(1);
          ui()->modelsListBox->setCurrentRow($i);
          ui()->modelsListBox->scrollToItem(ui()->modelsListBox->item($i));
        }

        if ($remove) {
          ui()->modelsListBox->item($i)->setSelected(0);
        }
      } else {
        if ($select) {
          ui()->modelsListBox->item($i)->setSelected(0);
        }
      }
    }
}

#
#  Replace Tab
#



sub replaceModuleTypeComboBox_activated
{
    my $awbtype = shift;

    #
    # Fill module combobox with appropriate modules
    #

    this->{replacementModuleMap} = {};

    #
    # Insert models
    #

    my $modelDB = Asim::Model::DB->new();
    my @models = $modelDB->find($awbtype);

    for my $i (sort @models) {
      replacementModuleMap()->{$i->name(). " (submodel)"} = $i;
    }

    #
    # Insert modules
    #  

    my $moduleDB = Asim::Module::DB->new();
    my @modules = $moduleDB->find($awbtype);

    for my $i (sort @modules) {
      replacementModuleMap()->{$i->name()} = $i;
    }

    #
    # Fill combobox with appropriate modules
    #

    ui()->replaceModuleComboBox->clear();
    ui()->replaceModuleComboBox->addItem("<Pick a module to substitute>");

    my $hash2 = replacementModuleMap();

    for my $i (sort(keys(%{$hash2}))) {
        ui()->replaceModuleComboBox->addItem($i);
    }

}


sub ModulesComboBox_activated
{
    my $modulename = shift;

    #
    # Clear out widgets
    #

    ui()->moduleListBox->clear();

    #
    # Check that something was selected
    #

    if ($modulename =~ /^<.*>$/) {
      $modulename = undef;
      this->{replacementModule} = $modulename;
      return;
    }


    #
    # Describe module
    #

    my $module = replacementModuleMap()->{$modulename};

    this->{replacementModule} = $module;


    ui()->moduleListBox->addItem("Name:        " . $module->name());
    ui()->moduleListBox->addItem("Description: " . $module->description());
    ui()->moduleListBox->addItem("Provides:    " . $module->provides());
    ui()->moduleListBox->addItem("Requires:    " . join(", ", $module->requires()));
 
}


sub replacePushButton_clicked
{
    my $module = replacementModule();

    #
    # Clear out the log
    #

    ui()->logListBox->clear();
    ui()->statusTextBox->setText("");

    if (! defined($module)) {
      ui()->logListBox->addItem("Error: a module to substitute must be selected");
      return;
    }
    
    # TBD:Check that module is of right type....

    #
    # Do the replacement for each selected model
    #

    my $max = ui()->modelsListBox->count();
    my $status;
    my $count = 0;
    my $error = 1;
    my $selected = 0;

    for (my $i=0; $i < $max; $i++) {
      if (ui()->modelsListBox->item($i)->isSelected()) {
        my $item = ui()->modelsListBox->item($i);
        my $name = $item->text();
    
        $status = replaceModule($name, $module);
        $count++ if $status;

        $error &= $status;

        $selected++;
      }
    }

    #
    # Report what happened
    #

    if ($selected == 0) {
      ui()->logListBox->addItem("Error: it appears no models were selected");
      return;
    }

    ui()->statusTextBox->setText("$count replacements made");

    if (!$error) {
      ui()->logListBox->addItem("Error: some replacmements failed");
    }
    
}



sub replaceModule
{
    my $name = shift;
    my $module = shift;

    #
    # TBD: Create a new instance of the model
    #

    my $model = searchModelMap()->{$name};

    #
    # Create new instance of the module
    #

    my $newmodule;

    if (ref($module) eq "Asim::Module") {
      $newmodule = Asim::Module->new($module->filename());
    } elsif (ref($module) eq "Asim::Model") {
      $newmodule = Asim::Model->new($module->filename());
    } else {
      die("Module of unknown type\n");
    }
     
    #
    # ui()->Replace the module
    #

    my $status = defined($model->smart_add_submodule($newmodule));

    #
    # Record that we did the replacement
    #

    my $log = ($status?"Processed: ":"ui()->Replacement failure: ") . $name;

    ui()->logListBox->addItem($log);
    searchModelMap()->{$log} = $model;

    return $status;
}


sub savePushButton_clicked
{

    #
    # Save each model in the log
    #

    my $max = ui()->logListBox->count();
    my $count = 0;

    for (my $i=0; $i < $max; $i++) {
      my $item;
      my $name;

      $item = ui()->logListBox->item($i);
      next if (! defined($item));

      $name = $item->text();
      next if (! defined($name));

      #
      # For properly formatted lines in log, save and backup the model
      #

      if ($name =~ /^Processed:/ || $name =~ /^Renamed:/ ) {
        my $model = searchModelMap()->{$name};
        next if (! defined($model));

        print "Saving: " . $model->name() . "\n";

        $model->backup();
        $model->save();

        $count++;
      }
    }

    #
    # Report what happened
    #

    if ($count == 0) {
      ui()->logListBox->addItem("No models saved");
      ui()->statusTextBox->setText("No models saved");
      return;
    }

    ui()->logListBox->addItem("All models saved ($count)");
    ui()->statusTextBox->setText("$count models saved");
}


#
# Rename Tab
#



sub renamePushButton_clicked
{
    my $inpattern  = ui()->inPatternLineEdit->text();
    my $outpattern = ui()->outPatternLineEdit->text();

    #
    # Clear out the log
    #

    ui()->logListBox->clear();
    ui()->statusTextBox->setText("");

    if ($outpattern eq "") {
      ui()->logListBox->addItem("Error: an output pattern must be specified");
      return;
    }
    
    #
    # Do the rename for each selected model
    #

    my $max = ui()->modelsListBox->count();
    my $status = 1;
    my $selected = 0;

    for (my $i=0; $i < $max; $i++) {
      if (ui()->modelsListBox->item($i)->isSelected()) {
        my $item = ui()->modelsListBox->item($i);
        my $name = $item->text();

        my $model = searchModelMap()->{$name};

        #
        # Rename the model
        #

        my $inname = $model->name();
        my $outname = $inname;

        $outname =~ s/$inpattern/$outpattern/;

        $model->name($outname);

        #
        # Record that we did the replacement
        #

        my $log = "Renamed: $outname <- $inname";

        ui()->logListBox->addItem($log);
        searchModelMap()->{$log} = $model;

        $selected++;
      }
    }

    if ($selected == 0) {
      ui()->logListBox->addItem("Error: it appears no models were selected");
      return;
    }

    ui()->statusTextBox->setText("$selected replacements made");


    if (!$status) {
      ui()->logListBox->addItem("Error: some replacmements failed");
    }
    

}


sub save2PushButton_clicked
{
      savePushButton_clicked();
}


#
# Parameter Tab
#

sub replaceParamModuleTypeComboBox_activated
{
    my $awbtype = shift;

    #
    # Fill module combobox with appropriate modules
    #

    this->{replaceParamModuleMap} = {};

    #
    # Insert models
    #

    my $modelDB = Asim::Model::DB->new();
    my @models = $modelDB->find($awbtype);

    for my $i (sort @models) {
      replaceParamModuleMap()->{$i->name() . " (submodel)"} = $i;
    }

    #
    # Insert modules
    #  

    my $moduleDB = Asim::Module::DB->new();
    my @modules = $moduleDB->find($awbtype);

    for my $i (sort @modules) {
      replaceParamModuleMap()->{$i->name()} = $i;
    }

    #
    # Fill combobox with appropriate modules
    #

    ui()->replaceParamModuleComboBox->clear();
    ui()->replaceParamModuleComboBox->addItem("<Pick a module to change parameters in>");

    my $hash2 = replaceParamModuleMap();

    for my $i (sort(keys(%{$hash2}))) {
        ui()->replaceParamModuleComboBox->addItem($i);
    }

    #
    # Clear the parameters combobox and listbox
    # 

    ui()->replaceParamComboBox->clear();
    ui()->replaceParamListBox->clear();

    #
    # Initialize patterns
    #

    ui()->replaceParamInPatternLineEdit->setText(".*");
    ui()->replaceParamOutPatternLineEdit->setText("");

}


sub replaceParamModuleComboBox_activated
{
    my $modulename = shift;

    #
    # Clear out ui()->widgets
    #

    ui()->replaceParamComboBox->clear();

    #
    # Check that something was selected
    #

    if ($modulename =~ /^<.*>$/) {
      $modulename = undef;
      this->{replaceParamModule} = $modulename;
      return;
    }

    #
    # List parameters
    #

    my $module = replaceParamModuleMap()->{$modulename};

    this->{replaceParamModule} = $module;

    #
    # Generate list of parameters
    #

    my @parameters = $module->parameters();

    this->{replaceParamMap} = {};

    for my $i (sort @parameters) {
      replaceParamMap()->{$i->name()} = $i;
    }

    #
    # Fill combobox with appropriate parameters
    #

    ui()->replaceParamComboBox->clear();
    ui()->replaceParamComboBox->addItem("<Pick a parameter to update>");

    my $hash2 = replaceParamMap();

    for my $i (sort(keys(%{$hash2}))) {
        ui()->replaceParamComboBox->addItem($i);
    }

    #
    # Clear out description box
    #

    ui()->replaceParamListBox->clear();

}


sub replaceParamComboBox_activated
{
    my $paramname = shift;

    #
    # Clear out ui()->widgets
    #

    ui()->replaceParamListBox->clear();

    #
    # Check that something was selected
    #

    if ($paramname =~ /^<.*>$/) {
      $paramname = undef;
      this->{replaceParam} = $paramname;
      return;
    }

    #
    # Desscribe parameter
    #

    my $param = replaceParamMap()->{$paramname};

    this->{replaceParam} = $param;

    ui()->replaceParamListBox->addItem("Name:        " . $param->name());
    ui()->replaceParamListBox->addItem("Description: " . $param->description());

}


sub replaceParamReplacePushButton_clicked
{
    my $module = replaceParamModule();
    my $param  = replaceParam();

    my $inpattern  = ui()->replaceParamInPatternLineEdit->text();
    my $outpattern = ui()->replaceParamOutPatternLineEdit->text();

    #
    # Clear out the log
    #

    ui()->logListBox->clear();
    ui()->statusTextBox->setText("");

    if (! defined($module)) {
      ui()->logListBox->addItem("Error: a module must be selected");
      return;
    }
    
    if (! defined($param)) {
      ui()->logListBox->addItem("Error: a parameter to update must be selected");
      return;
    }
    
    #
    # Do the replacement for each selected model
    #

    my $max = ui()->modelsListBox->count();
    my $status;
    my $count = 0;
    my $error = 1;
    my $selected = 0;

    for (my $i=0; $i < $max; $i++) {
      if (ui()->modelsListBox->item($i)->isSelected()) {
        my $item = ui()->modelsListBox->item($i);
        my $name = $item->text();
    
        $status = replaceParameter($name, $module, $param, $inpattern, $outpattern);
        $count++ if $status;

        $error &= $status;

        $selected++;
      }
    }

    #
    # Report what happened
    #

    if ($selected == 0) {
      ui()->logListBox->addItem("Error: it appears no models were selected");
      return;
    }

    ui()->statusTextBox->setText("$count replacements made");

    if (!$error) {
      ui()->logListBox->addItem("Error: some replacmements failed");
    }

}


sub replaceParamSavePushButton_clicked
{
    savePushButton_clicked();
}


sub replaceParameter
{
    my $name = shift;
    my $module = shift;
    my $parameter = shift;

    my $inpattern = shift;
    my $outpattern = shift;

    my $status;

    #
    # TBD: Create a new instance of the model
    #

    my $model = searchModelMap()->{$name};

    #
    # Find the module and check that it is right instance
    #

    my $update_module = $model->find_module_providing($module->provides());

    if ($update_module->isroot() && $model != $update_module->owner()) {
      #
      # This is a submodel - put reference to submodel
      # 
      $update_module = $update_module->owner();
    }
     
    #
    # Get the parameter to update...
    #

    my $update_parameter = $update_module->getparameter($parameter->name());

    #
    # Update the parameter to update...
    #

    my $update_value = $update_parameter->value();

    $status= ($update_value =~ s/$inpattern/$outpattern/);

    if ($status) {
      $update_parameter->value($update_value);
    }

    #
    # Record that we did the replacement
    #

    my $log = ($status?"Processed: ":"Replacement failure: ") . $name;

    ui()->logListBox->addItem($log);
    searchModelMap()->{$log} = $model;

    return $status;
}

#
# Model List Box
#


sub allPushButton_clicked
{
    #
    # Select all items
    #

    my $max = ui()->modelsListBox->count();
    my $count = 0;

    for (my $i=0; $i < $max; $i++) {
      ui()->modelsListBox->setCurrentRow($i,Qt::ItemSelectionModel::Select());
      $count++;
    }

    ui()->statusTextBox->setText("$count models selected");
}

sub invertPushButton_clicked
{
    #
    # Invert selection on each item
    #

    my $max = ui()->modelsListBox->count();

    for (my $i=0; $i < $max; $i++) {
      ui()->modelsListBox->setCurrentRow($i,Qt::ItemSelectionModel::Toggle());
    }

    my $count = scalar @{ui()->modelsListBox->selectedItems()};

    ui()->statusTextBox->setText("$count models selected");
}


sub purgePushButton_clicked
{
    #
    # Purge selected items from list
    #

    my $i = 0;
    my $count = 0;

    while ($i < ui()->modelsListBox->count()) {
      my $item;

      my $selected = ui()->modelsListBox->item($i)->isSelected();

      if (! $selected) {
        ui()->modelsListBox->takeItem($i);
      } else {
        $i++;
        $count++;
      }
    }
    ui()->statusTextBox->setText("$count models selected");
}

sub modelsListBox_doubleClicked
{
      my $item = shift;
      my $model;

      #
      # Open apm-edit on selected model
      #

      $model = searchModelMap()->{$item->text()};
      if (! defined($model)) {
          return;
      }

      system("apm-edit " . $model->filename() . " &");
}


sub modelsListBox_contextMenuRequested
{
      my $item = shift;
      my $point = shift;

      ui()->modelMenu->popup($point);
      ui()->modelMenu->exec();
}



sub modelEditAction_activated
{
      my $model;

      #
      # Open apm-edit on selected model
      #
      $model = searchModelMap()->{ui()->modelsListBox->currentItem()->text()};
      return if (! defined($model));

      system("apm-edit " . $model->filename() . " &");
}


#
# Return a hash of models keyed on the ModelListBox index
#    optionally only selected models will be returned
#

sub models
{
    my $justselected = shift;

    my %modelhash;

    my $max = ui()->modelsListBox->count();

    for (my $i=0; $i < $max; $i++) {
      my $item;
      my $model;
      my $match;
      
      $item = ui()->modelsListBox->item($i);
      next if (! defined($item));

      my $selected = ui()->modelsListBox->item($i)->isSelected();

      $model = searchModelMap()->{$item->text()};
      next if (! defined($model));

      if (! $justselected || $selected) {
        $modelhash{$i} = $model;
      }
    }

    return \%modelhash;
}

#
#  ui()->Log List Box
#


sub clearPushButton_clicked
{
    #
    # Clear out log
    #

    ui()->logListBox->clear();
    ui()->statusTextBox->setText("");
}


sub logListBox_doubleClicked
{
      my $item = shift;

      #
      # Open apm-edit on model
      #

      my $model;

      $model = searchModelMap()->{$item->text()};
      if (! defined($model)) {
          return;
      }

      system("apm-edit " . $model->filename() . " &");
}

sub helpWhatsThisAction_activated
{
    Qt::WhatsThis::enterWhatsThisMode();
}

