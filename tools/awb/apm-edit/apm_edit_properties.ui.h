/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/



void apm_edit_properties::init()
{
    my $model = $apm_edit::model;
    
    if (! defined($model)) {
      return;
    }

    Name->setText($model->name());
    Description->setText($model->description());
    Attributes->setText($model->default_attributes());

    Autoselect->setState($model->autoselect());

    Benchmark->setText($model->default_benchmark());
    RunOpts->setText($model->default_runopts());
}


void apm_edit_properties::propertiesOk_clicked()
{
    my $model = $apm_edit::model;

    $model->name(Name->text());
    $model->description(Description->text());
    $model->default_attributes(Attributes->text());

    $model->autoselect(Autoselect->state());

    $model->default_benchmark(Benchmark->text());
    $model->default_runopts(RunOpts->text());

    this->accept();
}

void apm_edit_properties::propertiesHelp_clicked()
{
    print "TBD: Display a what's this pointer\n";
#    this->whatsThis();
}

