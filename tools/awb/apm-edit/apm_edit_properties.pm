package apm_edit_properties;

use strict;
use warnings;
use QtCore4;
use QtGui4;
use QtCore4::isa qw( Qt::Dialog );

use QtCore4::slots
    init => [],
    propertiesOk_clicked => [],
    propertiesHelp_clicked => [];

use constant {type_asim => 0, type_hasim => 1, type_leap => 2};

sub ui() 
{
    this->{ui};
}

sub NEW
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Apm_edit_properties->setupUi(this);
}

sub getDefaultProvides
{
    my $type = shift;
    
    if ($type eq "Leap") {
        return "project";
    } else {
        return "model";
    }
}

sub init
{
    my $model = $apm_edit::model;
    
    if (! defined($model)) {
      return;
    }

    ui()->name()->setText($model->name());

    my $type = $model->type();

    if ($type eq "Asim") {
        ui()->typeComboBox->setCurrentIndex(type_asim);
    } elsif ($type eq "HAsim") {
        ui()->typeComboBox->setCurrentIndex(type_hasim);
    } elsif ($type eq "Leap") {
        ui()->typeComboBox->setCurrentIndex(type_leap);
    } else {
        ui()->typeComboBox->setCurrentIndex(type_leap);
    }
        
    ui()->description->setText($model->description());
    ui()->attributes->setText($model->attributes2string());

    ui()->autoselect->setChecked($model->autoselect());

    ui()->benchmark->setText($model->default_benchmark());
    ui()->runOpts->setText($model->default_runopts());
    ui()->rootProvides->setText($model->provides());
    this->{current_root_provides} = $model->provides();
}


sub propertiesOk_clicked
{
    my $model = $apm_edit::model;

    $model->name(ui()->name->text());
    $model->type(ui()->typeComboBox->currentText());

    $model->description(ui()->description->text());
    $model->set_attributes(ui()->attributes->text());

    $model->autoselect(ui()->autoselect->isChecked());

    $model->default_benchmark(ui()->benchmark->text());
    $model->default_runopts(ui()->runOpts->text());
    
    $model->provides(ui()->rootProvides->text());

    this->accept();
}

sub propertiesHelp_clicked
{
    Qt::WhatsThis::enterWhatsThisMode();
}

sub typeComboBox_activated
{
    my $newval = shift;
    my $old_default_provides = apm_edit_properties::getDefaultProvides(this->{current_root_provides});
  
    if (RootProvides->text() eq $old_default_provides) {
        my $new_default_provides = apm_edit_properties::getDefaultProvides($newval);
        RootProvides->setText($new_default_provides);
    }
    this->{current_root_provides} = $newval;
}


