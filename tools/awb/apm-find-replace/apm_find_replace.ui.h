/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#
# Constructor
#

void apmFindReplace::init()
{
  newPushButton_clicked();
}

#
# Menus
#

void apmFindReplace::fileNew()
{
  newPushButton_clicked();
}


void apmFindReplace::fileExit()
{
  Qt::Application::exit();
}



void apmFindReplace::editFind()
{
  findPushButton_clicked();
}



void apmFindReplace::moduleEdit_activated()
{

}


void apmFindReplace::helpAbout()
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


#
#  Find Tab
#

void apmFindReplace::newPushButton_clicked()
{
  #
  # Set up widgets in find tab
  #

  my $moduleDB = Asim::Module::DB->new();
  my @awbtypes = $moduleDB->module_types();

  my  $awbtype = undef;
  searchModuleType = $awbtype;

  searchModelMap = {};

  rootdirLineEdit->setText("config/pm");

  findModuleTypeComboBox->clear();
  findModuleTypeComboBox->insertItem("<Select a module type to search for>");

  for my $i (sort(@awbtypes)) {
    findModuleTypeComboBox->insertItem($i);
  }

  rootCheckBox->setChecked(0);
  invertCheckBox->setChecked(0);

  modelsListBox->clear();

  #
  # Clear widgets in filter tab
  #
  filterModuleMap = {};

  filterLineEdit->setText("");
  attrLineEdit->setText("");
  
  filterModuleTypeComboBox->clear();
  filterModuleTypeComboBox->insertItem("<Select a module type to search for>");
  for my $i (sort(@awbtypes)) {
    filterModuleTypeComboBox->insertItem($i);
  }

  filterModuleComboBox->clear();



  #
  # Clear widgets in replace tab
  #
  replacementModuleMap = {};


  selectRadioButton->setChecked(1);
  filterInvertCheckBox->setChecked(0);


  replaceModuleTypeComboBox->clear();
  replaceModuleTypeComboBox->insertItem("<Select a module type to replace>");
  for my $i (sort(@awbtypes)) {
    replaceModuleTypeComboBox->insertItem($i);
  }


  replaceModuleComboBox->clear();

  my $module = undef;
  replacementModule = $module;

  moduleListBox->clear();

  #
  # Clear widgets in rename tab
  #

  inPatternLineEdit->setText("");
  outPatternLineEdit->setText("");

  #
  # Clear widgets in parameter tab
  #
  replaceParamModuleMap = {};


  replaceParamModuleTypeComboBox->clear();
  replaceParamModuleTypeComboBox->insertItem("<Select a module type>");
  for my $i (sort(@awbtypes)) {
    replaceParamModuleTypeComboBox->insertItem($i);
  }


  replaceParamModuleComboBox->clear();
  replaceParamComboBox->clear();

  my $module2 = undef;
  replaceParamModule = $module2;

  my $param = undef;
  replaceParam = $param;

  replaceParamListBox->clear();

  #
  # Clear widgets in log groupbox
  #

  logListBox->clear();
  statusTextBox->setText("");
}


void apmFindReplace::findModuleTypeComboBox_activated( const QString & )
{
  my $awbtype = shift;

  #
  # Clear out widgets
  #

  modelsListBox->clear();

  moduleListBox->clear();

  #
  # Just return if this is the null choice
  #

  if ($awbtype =~ /^<.*>$/) {
    $awbtype = undef;
    searchModuleType = $awbtype;
    return;
  }

  searchModuleType = $awbtype;



}


void apmFindReplace::findPushButton_clicked()
{
  my $awbtype = searchModuleType;

  #
  # Clear out widgets
  #

  modelsListBox->clear();

  replaceModuleComboBox->clear();
  my $module = undef;
  replacementModule = $module;

  moduleListBox->clear();
  logListBox->clear();
  statusTextBox->setText("");
    
  #
  # Return without an awbtype
  #

  if (! defined($awbtype)) {
    logListBox->insertItem("Error: a module type must be selected");
    return;
  }

  #
  # Fill models selection box with appropriate models
  #

  my $rootonly = rootCheckBox->isChecked();
  my $inverttest = invertCheckBox->isChecked();

  searchModelMap = {};

  findModels($awbtype, rootdirLineEdit->text(), $rootonly, $inverttest);

  my $hash1 = searchModelMap;
  my $count = 0;

  for my $i (sort(keys(%{$hash1}))) {
    modelsListBox->insertItem($i);
    $count++;
  }

  #
  # Report what happened
  #

  statusTextBox->setText("$count models found");

}

#
# Modified version of method from Asim::Model::DB....
#
# TBD: Convert to keyword-style arguments
#

void apmFindReplace::findModels()
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
      
      while (defined(searchModelMap->{$modelname})) {
        logListBox->insertItem("Error: Duplicate modelname - $modelname");
        $modelname =~ s/( - duplicate.*)*$/ - duplicate $count/;
        $count++;
      }

      searchModelMap->{$modelname} = $model;
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

void apmFindReplace::filterLineEdit_returnPressed()
{
  filterPushButton_clicked();
}

void apmFindReplace::filterPushButton_clicked()
{
  my $filter = filterLineEdit->text();

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

  statusTextBox->setText("$count matches found");
}




void apmFindReplace::attrLineEdit_returnPressed()
{
  attrPushButton_clicked();
}


void apmFindReplace::attrPushButton_clicked()
{
  my $filter = attrLineEdit->text();

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

  statusTextBox->setText("$count matches found");
}



void apmFindReplace::filterModuleTypeComboBox_activated( const QString & )
{
  my $awbtype = shift;

  filterModuleComboBox->clear();

  if (! defined($awbtype)) {
    return;
  }

  #
  # Fill module combobox with appropriate modules
  #

  filterModuleMap = {};
  
  #
  # Insert models
  #

  my $modelDB = Asim::Model::DB->new();
  my @models = $modelDB->find($awbtype);

  for my $i (sort @models) {
    filterModuleMap->{$i->name() . " (submodel)"} = $i;
  }

  #
  # Insert modules
  #  

  my $moduleDB = Asim::Module::DB->new();
  my @modules = $moduleDB->find($awbtype);

  for my $i (sort @modules) {
    filterModuleMap->{$i->name()} = $i;
  }


  #
  # Fill module filter combobox with appropriate modules
  #

  filterModuleComboBox->clear();
  filterModuleComboBox->insertItem("<Filter models to a specific module>");

  my $hash1 = filterModuleMap;

  for my $i (sort(keys(%{$hash1}))) {
    filterModuleComboBox->insertItem($i);
  }
}


void apmFindReplace::typePushButton_clicked()
{
  my $filter = filterModuleTypeComboBox->currentText();

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

  statusTextBox->setText("$count matches found");
}


void apmFindReplace::modulePushButton_clicked()
{
   my $modulename = filterModuleComboBox->currentText();
   if (! defined($modulename)) {
    print "No modulename\n";
    return;
  }

   my $module = filterModuleMap->{$modulename};

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

  statusTextBox->setText("$count matches found");
}




void apmFindReplace::brokenPushButton_clicked()
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

  statusTextBox->setText("$count matches found");
}


#
# Utility function to update the models selected
#

void apmFindReplace::updateModelSelection()
{
  my $match = shift;
  my @indices = @_;

  #
  # Get action
  #

  my $select = selectRadioButton->isChecked();
  my $add    = addRadioButton->isChecked();
  my $remove = removeRadioButton->isChecked();


  my $invert = filterInvertCheckBox->isChecked();

  #
  # Update selection for each item in the list
  #

  if ($invert) {
      $match = ! $match;
  }

  for my $i (@indices) {

    if ($match) {
      if ($select || $add) {
        modelsListBox->setSelected($i,1);
        modelsListBox->setCurrentItem($i);
	modelsListBox->ensureCurrentVisible();
      }

      if ($remove) {
        modelsListBox->setSelected($i,0);
      }
    } else {
      if ($select) {
        modelsListBox->setSelected($i,0);
      }
    }
  }
}

#
#  Replace Tab
#



void apmFindReplace::replaceModuleTypeComboBox_activated( const QString & )
{
  my $awbtype = shift;

  #
  # Fill module combobox with appropriate modules
  #

  replacementModuleMap = {};

  #
  # Insert models
  #

  my $modelDB = Asim::Model::DB->new();
  my @models = $modelDB->find($awbtype);

  for my $i (sort @models) {
    replacementModuleMap->{$i->name(). " (submodel)"} = $i;
  }

  #
  # Insert modules
  #  

  my $moduleDB = Asim::Module::DB->new();
  my @modules = $moduleDB->find($awbtype);

  for my $i (sort @modules) {
    replacementModuleMap->{$i->name()} = $i;
  }

  #
  # Fill combobox with appropriate modules
  #

  replaceModuleComboBox->clear();
  replaceModuleComboBox->insertItem("<Pick a module to substitute>");

  my $hash2 = replacementModuleMap;

  for my $i (sort(keys(%{$hash2}))) {
      replaceModuleComboBox->insertItem($i);
  }

}


void apmFindReplace::ModulesComboBox_activated( const QString & )
{
  my $modulename = shift;

  #
  # Clear out widgets
  #

  moduleListBox->clear();

  #
  # Check that something was selected
  #

  if ($modulename =~ /^<.*>$/) {
    $modulename = undef;
    replacementModule = $modulename;
    return;
  }


  #
  # Describe module
  #

  my $module = replacementModuleMap->{$modulename};

  replacementModule = $module;


  moduleListBox->insertItem("Name:        " . $module->name());
  moduleListBox->insertItem("Description: " . $module->description());
  moduleListBox->insertItem("Provides:    " . $module->provides());
  moduleListBox->insertItem("Requires:    " . join(", ", $module->requires()));
 
}


void apmFindReplace::replacePushButton_clicked()
{
  my $module = replacementModule;

  #
  # Clear out the log
  #

  logListBox->clear();
  statusTextBox->setText("");

  if (! defined($module)) {
    logListBox->insertItem("Error: a module to substitute must be selected");
    return;
  }
  
  # TBD:Check that module is of right type....

  #
  # Do the replacement for each selected model
  #

  my $max = modelsListBox->count();
  my $status;
  my $count = 0;
  my $error = 1;
  my $selected = 0;

  for (my $i=0; $i < $max; $i++) {
    if (modelsListBox->isSelected($i)) {
      my $item = modelsListBox->item($i);
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
    logListBox->insertItem("Error: it appears no models were selected");
    return;
  }

  statusTextBox->setText("$count replacements made");

  if (!$error) {
    logListBox->insertItem("Error: some replacmements failed");
  }
  
}



void apmFindReplace::replaceModule()
{
  my $name = shift;
  my $module = shift;

  #
  # TBD: Create a new instance of the model
  #

  my $model = searchModelMap->{$name};

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
  # Replace the module
  #

  my $status = defined($model->smart_add_submodule($newmodule));

  #
  # Record that we did the replacement
  #

  my $log = ($status?"Processed: ":"Replacement failure: ") . $name;

  logListBox->insertItem($log);
  searchModelMap->{$log} = $model;

  return $status;
}


void apmFindReplace::savePushButton_clicked()
{

  #
  # Save each model in the log
  #

  my $max = logListBox->count();
  my $count = 0;

  for (my $i=0; $i < $max; $i++) {
    my $item;
    my $name;

    $item = logListBox->item($i);
    next if (! defined($item));

    $name = $item->text();
    next if (! defined($name));

    #
    # For properly formatted lines in log, save and backup the model
    #

    if ($name =~ /^Processed:/ || $name =~ /^Renamed:/ ) {
      my $model = searchModelMap->{$name};
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
    logListBox->insertItem("No models saved");
    statusTextBox->setText("No models saved");
    return;
  }

  logListBox->insertItem("All models saved ($count)");
  statusTextBox->setText("$count models saved");
}


#
# Rename Tab
#



void apmFindReplace::renamePushButton_clicked()
{
  my $inpattern  = inPatternLineEdit->text();
  my $outpattern = outPatternLineEdit->text();

  #
  # Clear out the log
  #

  logListBox->clear();
  statusTextBox->setText("");

  if ($outpattern eq "") {
    logListBox->insertItem("Error: an output pattern must be specified");
    return;
  }
  
  #
  # Do the rename for each selected model
  #

  my $max = modelsListBox->count();
  my $status = 1;
  my $selected = 0;

  for (my $i=0; $i < $max; $i++) {
    if (modelsListBox->isSelected($i)) {
      my $item = modelsListBox->item($i);
      my $name = $item->text();

      my $model = searchModelMap->{$name};

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

      logListBox->insertItem($log);
      searchModelMap->{$log} = $model;

      $selected++;
    }
  }

  if ($selected == 0) {
    logListBox->insertItem("Error: it appears no models were selected");
    return;
  }

  statusTextBox->setText("$selected replacements made");


  if (!$status) {
    logListBox->insertItem("Error: some replacmements failed");
  }
  

}


void apmFindReplace::save2PushButton_clicked()
{
    savePushButton_clicked();
}


#
# Parameter Tab
#

void apmFindReplace::replaceParamModuleTypeComboBox_activated( const QString & )
{
  my $awbtype = shift;

  #
  # Fill module combobox with appropriate modules
  #

  replaceParamModuleMap = {};

  #
  # Insert models
  #

  my $modelDB = Asim::Model::DB->new();
  my @models = $modelDB->find($awbtype);

  for my $i (sort @models) {
    replaceParamModuleMap->{$i->name() . " (submodel)"} = $i;
  }

  #
  # Insert modules
  #  

  my $moduleDB = Asim::Module::DB->new();
  my @modules = $moduleDB->find($awbtype);

  for my $i (sort @modules) {
    replaceParamModuleMap->{$i->name()} = $i;
  }

  #
  # Fill combobox with appropriate modules
  #

  replaceParamModuleComboBox->clear();
  replaceParamModuleComboBox->insertItem("<Pick a module to change parameters in>");

  my $hash2 = replaceParamModuleMap;

  for my $i (sort(keys(%{$hash2}))) {
      replaceParamModuleComboBox->insertItem($i);
  }

  #
  # Clear the parameters combobox and listbox
  # 

  replaceParamComboBox->clear();
  replaceParamListBox->clear();

  #
  # Initialize patterns
  #

  replaceParamInPatternLineEdit->setText(".*");
  replaceParamOutPatternLineEdit->setText("");

}


void apmFindReplace::replaceParamModuleComboBox_activated( const QString & )
{
  my $modulename = shift;

  #
  # Clear out widgets
  #

  replaceParamComboBox->clear();

  #
  # Check that something was selected
  #

  if ($modulename =~ /^<.*>$/) {
    $modulename = undef;
    replaceParamModule = $modulename;
    return;
  }

  #
  # List parameters
  #

  my $module = replaceParamModuleMap->{$modulename};

  replaceParamModule = $module;

  #
  # Generate list of parameters
  #

  my @parameters = $module->parameters();

  replaceParamMap = {};

  for my $i (sort @parameters) {
    replaceParamMap->{$i->name()} = $i;
  }

  #
  # Fill combobox with appropriate parameters
  #

  replaceParamComboBox->clear();
  replaceParamComboBox->insertItem("<Pick a parameter to update>");

  my $hash2 = replaceParamMap;

  for my $i (sort(keys(%{$hash2}))) {
      replaceParamComboBox->insertItem($i);
  }

  #
  # Clear out description box
  #

  replaceParamListBox->clear();

}


void apmFindReplace::replaceParamComboBox_activated( const QString & )
{
  my $paramname = shift;

  #
  # Clear out widgets
  #

  replaceParamListBox->clear();

  #
  # Check that something was selected
  #

  if ($paramname =~ /^<.*>$/) {
    $paramname = undef;
    replaceParam = $paramname;
    return;
  }

  #
  # Desscribe parameter
  #

  my $param = replaceParamMap->{$paramname};

  replaceParam = $param;

  replaceParamListBox->insertItem("Name:        " . $param->name());
  replaceParamListBox->insertItem("Description: " . $param->description());

}


void apmFindReplace::replaceParamReplacePushButton_clicked()
{
  my $module = replaceParamModule;
  my $param  = replaceParam;

  my $inpattern  = replaceParamInPatternLineEdit->text();
  my $outpattern = replaceParamOutPatternLineEdit->text();

  #
  # Clear out the log
  #

  logListBox->clear();
  statusTextBox->setText("");

  if (! defined($module)) {
    logListBox->insertItem("Error: a module must be selected");
    return;
  }
  
  if (! defined($param)) {
    logListBox->insertItem("Error: a parameter to update must be selected");
    return;
  }
  
  #
  # Do the replacement for each selected model
  #

  my $max = modelsListBox->count();
  my $status;
  my $count = 0;
  my $error = 1;
  my $selected = 0;

  for (my $i=0; $i < $max; $i++) {
    if (modelsListBox->isSelected($i)) {
      my $item = modelsListBox->item($i);
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
    logListBox->insertItem("Error: it appears no models were selected");
    return;
  }

  statusTextBox->setText("$count replacements made");

  if (!$error) {
    logListBox->insertItem("Error: some replacmements failed");
  }

}


void apmFindReplace::replaceParamSavePushButton_clicked()
{
  savePushButton_clicked();
}


void apmFindReplace::replaceParameter()
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

  my $model = searchModelMap->{$name};

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

  logListBox->insertItem($log);
  searchModelMap->{$log} = $model;

  return $status;
}

#
# Model List Box
#


void apmFindReplace::allPushButton_clicked()
{
  #
  # Select all items
  #

  my $max = modelsListBox->count();
  my $count = 0;

  for (my $i=0; $i < $max; $i++) {
    modelsListBox->setSelected($i,1);
    $count++;
  }

  statusTextBox->setText("$count models selected");
}

void apmFindReplace::invertPushButton_clicked()
{
  #
  # Invert selection on each item
  #

  my $max = modelsListBox->count();
  my $count = 0;

  for (my $i=0; $i < $max; $i++) {
    my $item;
    
    $item = modelsListBox->item($i);
    next if (! defined($item));

    my $selected = modelsListBox->isSelected($i);
    $count++ if ! $selected;

    modelsListBox->setSelected($i,! $selected);
  }

  statusTextBox->setText("$count models selected");
}


void apmFindReplace::purgePushButton_clicked()
{
  #
  # Purge selected items from list
  #

  my $i = 0;
  my $count = 0;

  while ($i < modelsListBox->count()) {
    my $item;

    my $selected = modelsListBox->isSelected($i);

    if (! $selected) {
      modelsListBox->removeItem($i);
    } else {
      $i++;
      $count++;
    }
  }

  statusTextBox->setText("$count models selected");
}

void apmFindReplace::modelsListBox_doubleClicked( QListBoxItem * )
{
    my $item = shift;
    my $model;

    #
    # Open apm-edit on selected model
    #

    $model = searchModelMap->{$item->text()};
    if (! defined($model)) {
        return;
    }

    system("apm-edit " . $model->filename() . " &");
}


void apmFindReplace::modelsListBox_contextMenuRequested( QListBoxItem *, const QPoint & )
{
    my $item = shift;
    my $point = shift;

    modelMenu->popup($point);
    modelMenu->exec();
}



void apmFindReplace::modelEditAction_activated()
{
    my $model;

    #
    # Open apm-edit on selected model
    #
    $model = searchModelMap->{modelsListBox->currentText()};
    return if (! defined($model));

    system("apm-edit " . $model->filename() . " &");
}


#
# Return a hash of models keyed on the ModelListBox index
#    optionally only selected models will be returned
#

void apmFindReplace::models()
{
  my $justselected = shift;

  my %modelhash;

  my $max = modelsListBox->count();

  for (my $i=0; $i < $max; $i++) {
    my $item;
    my $model;
    my $match;
    
    $item = modelsListBox->item($i);
    next if (! defined($item));

    my $selected = modelsListBox->isSelected($i);

    $model = searchModelMap->{$item->text()};
    next if (! defined($model));

    if (! $justselected || $selected) {
      $modelhash{$i} = $model;
    }
  }

  return \%modelhash;
}

#
#  Log List Box
#


void apmFindReplace::clearPushButton_clicked()
{
  #
  # Clear out log
  #

  logListBox->clear();
  statusTextBox->setText("");
}


void apmFindReplace::logListBox_doubleClicked( QListBoxItem * )
{
    my $item = shift;

    #
    # Open apm-edit on model
    #

    my $model;

    $model = searchModelMap->{$item->text()};
    if (! defined($model)) {
        return;
    }

    system("apm-edit " . $model->filename() . " &");
}


