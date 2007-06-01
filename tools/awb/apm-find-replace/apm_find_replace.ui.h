/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


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
  findPushButton_clicked()
}


void apmFindReplace::helpAbout()
{
  my $message;

  $message  = "Apm-find-replace - bulk module replacement\n";
  $message .= "\n";
  $message .= "For help type: $0 --help\n";
  
  Qt::MessageBox::information(
            this, 
            "$0 help",
            $message);
}


#
#  Find Group Box
#

void apmFindReplace::newPushButton_clicked()
{
  #
  # Set up widgets in find groupbox
  #

  rootdirLineEdit->setText("config/pm");

  my $moduleDB = Asim::Module::DB->new();
  my @awbtypes = $moduleDB->module_types();

  findModuleTypeComboBox->clear();
  my  $awbtype = undef;
  searchModuleType = $awbtype;

  findModuleTypeComboBox->insertItem("<Select a module type to search for>");

  for my $i (sort(@awbtypes)) {
    findModuleTypeComboBox->insertItem($i);
  }

  findModuleComboBox->clear();
  modelsListBox->clear();

  #
  # Clear widgets in replace groupbox
  #

  replaceModuleComboBox->clear();
  my $module = undef;
  replacementModule = $module;

  moduleListBox->clear();

  #
  # Clear widgets in log groupbox
  #

  logListBox->clear();
}


void apmFindReplace::findModuleTypeComboBox_activated( const QString & )
{
  my $awbtype = shift;

  #
  # Clear out widgets
  #

  findModuleComboBox->clear();

  modelsListBox->clear();

  replaceModuleComboBox->clear();
  my $module = undef;
  replacementModule = $module;

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

  #
  # Fill module combobox with appropriate modules
  #

  searchModuleMap = {};

  findModuleComboBox->insertItem("<Optionally limit search to a specifc module>");

  #
  # Insert models
  #

  my $modelDB = Asim::Model::DB->new();
  my @models = $modelDB->find($awbtype);

  for my $i (sort @models) {
    searchModuleMap->{$i->name(). " (submodel)"} = $i;
    findModuleComboBox->insertItem($i->name() . " (submodel)");
  }

  #
  # Insert modules
  #  

  my $moduleDB = Asim::Module::DB->new();
  my @modules = $moduleDB->find($awbtype);

  for my $i (sort @modules) {
    searchModuleMap->{$i->name()} = $i;
    findModuleComboBox->insertItem($i->name());
  }



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

  searchModelMap = {};
  findModels($awbtype, rootdirLineEdit->text());

  my $hash1 = searchModelMap;

  for my $i (sort(keys(%{$hash1}))) {
    modelsListBox->insertItem($i);
  }

  #
  # Fill combobox with appropriate modules
  #

  replaceModuleComboBox->insertItem("<Pick a module to substitute>");

  my $hash2 = searchModuleMap;

  for my $i (sort(keys(%{$hash2}))) {
      replaceModuleComboBox->insertItem($i);
  }
}

#
# Copied from Asim::Model::DB....
#

void apmFindReplace::findModels()
{
  my $awbtype = shift;
  my $dir = shift || "config/pm";

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

      findModels($awbtype, $relname);

    } elsif ( $fname =~ /.*\.apm$/) {

      #
      # Add the model....
      #

      my $model = Asim::Model->new($absname);

      if (! defined($model)) {
        print("Could not open model file: $absname\n");
        next;
      }

      if ($model->modelroot()->provides() eq $awbtype) {
        # Skip models with desired module type at root of tree
        next;
      }

      my $module_of_type = $model->find_module_providing($awbtype);

      if (! defined($module_of_type)) {
        next;
      }

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


void apmFindReplace::invertPushButton_clicked()
{
  #
  # Invert selection on each item
  #

  my $max = modelsListBox->count();

  for (my $i=0; $i < $max; $i++) {
    my $item;
    
    $item = modelsListBox->item($i);
    next if (! defined($item));

    my $selected = modelsListBox->isSelected($i);

    modelsListBox->setSelected($i,! $selected);
  }
}


void apmFindReplace::purgePushButton_clicked()
{
  #
  # Invert selection on each item
  #

  my $max = 

  my $i = 0;

  while ($i < modelsListBox->count()) {
    my $item;
    
    $item = modelsListBox->item($i);
    next if (! defined($item));

    my $selected = modelsListBox->isSelected($i);

    if (! $selected) {
        modelsListBox->removeItem($i);
        $i = 0;
    } else {
        $i++;
    }
  }
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



void apmFindReplace::filterLineEdit_returnPressed()
{
  filterPushButton_clicked();
}

void apmFindReplace::filterPushButton_clicked()
{
  my $filter = filterLineEdit->text();

  #
  # Get action
  #
  my $select = selectRadioButton->isChecked();
  my $add    = addRadioButton->isChecked();
  my $remove = removeRadioButton->isChecked();

  #
  # Check (and select) model names that match regex
  #

  my $max = modelsListBox->count();

  for (my $i=0; $i < $max; $i++) {
    my $item;
    my $match;
    
    $item = modelsListBox->item($i);
    next if (! defined($item));

    $match = $item->text() =~ /$filter/;      

    if ($match) {
      if ($select || $add) {
        modelsListBox->setSelected($i,1);
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




void apmFindReplace::attrLineEdit_returnPressed()
{
  attrPushButton_clicked();
}


void apmFindReplace::attrPushButton_clicked()
{
  my $filter = attrLineEdit->text();

  #
  # Get action
  #
  my $select = selectRadioButton->isChecked();
  my $add    = addRadioButton->isChecked();
  my $remove = removeRadioButton->isChecked();

  #
  # Check (and select) model names that match regex
  #

  my $max = modelsListBox->count();

  for (my $i=0; $i < $max; $i++) {
    my $item;
    my $model;
    my $match;
    
    $item = modelsListBox->item($i);
    next if (! defined($item));

    $model = searchModelMap->{$item->text()};
    next if (! defined($model));

    my @attributes = $model->attributes();

    $match = grep(/$filter/, @attributes);

    # print "$filter - $match - " . join(",", @attributes);

    if ($match) {
      if ($select || $add) {
        modelsListBox->setSelected($i,1);
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
#  Replace Group Box
#

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

  my $module = searchModuleMap->{$modulename};

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

  if (! defined($module)) {
    logListBox->insertItem("Error: a module to substitute must be selected");
    return;
  }
  
  # TBD:Check that module is of right type....

  #
  # Do the replacement for each selected model
  #

  my $max = modelsListBox->count();
  my $selected = 0;

  for (my $i=0; $i < $max; $i++) {
    if (modelsListBox->isSelected($i)) {
      my $item = modelsListBox->item($i);
      my $name = $item->text();
  
      replaceModule($name, $module);
      $selected++;
    }
  }

  if ($selected == 0) {
    logListBox->insertItem("Error: it appears no models were selected");
    return;
  }


}



void apmFindReplace::replaceModule()
{
  my $name = shift;
  my $module = shift;

  my $model = searchModelMap->{$name};

  #
  # TBD: Create a new instance of the model
  #


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

  $model->smart_add_submodule($newmodule);

  #
  # Record that we did the replacement
  #

  my $log = "Processed: $name";

  logListBox->insertItem($log);
  searchModelMap->{$log} = $model;

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

    if ($name =~ /^Processed:/) {
      my $model = searchModelMap->{$name};

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
  } else {
    logListBox->insertItem("All models saved ($count)");
  }  
}

#
#  Log Group Box
#


void apmFindReplace::clearPushButton_clicked()
{
  #
  # Clear out log
  #

  logListBox->clear();
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



