package awb_wizard;

use strict;
use warnings;

use QtCore4;
use QtGui4;
use QtCore4::isa qw(Qt::MainWindow);
use QtCore4::slots 
    init => [],
    NewModule => [],
    fileOpen => [],
    fileSave => [],
    fileSaveAs => [],
    fileBuild => [],
    fileExit => [],
    editUpdate => [],
    editCut => [],
    Name_textChanged => ['const QString&'],
    Autofill_clicked => [],
    Any_textChanged => ['const QString&'],
    contextMenuRequested => ['QTreeWidgetItem*', 'const QPoint&', 'int'],
    Requires_Update => ['QTreeWidgetItem*'],
    RequiresName_textChanged => ['const QString&'],
    Requires_Add_clicked => [],
    Parameters_Update => ['QTreeWidgetItem*'],
    Param_Add_clicked => [],
    Ports_Update => ['QTreeWidgetItem*'],
    Port_Add_clicked => [],
    Stats_Update => ['QTreeWidgetItem*'],
    Stat_Add_clicked => [],
    nextStep_1_clicked => [],
    nextStep_2_clicked => [],
    nextStep_3_clicked => [],
    nextStep_4_clicked => [],
    nextStep_5_clicked => [],
    nextStep_6_clicked => [],
    nextStep_8_clicked => [],
    browsePath_clicked => [],
    Done_clicked => [],
    PortName_textChanged => ['const QString&'],
    helpWhatsThisAction_activated => [],
    about_activated => [];


sub ui
{
    return this->{ui};
}

sub NEW
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Awb_wizard->setupUi(this);
    init();
}

sub init
{

    use File::Basename;
    use Asim::Inifile;

    print "awb_wizard::init - We are in init\n" if $::debug;

    NewModule();
    
    # set columns to resize based on the contents
    ui()->requires()->header()->setResizeMode(Qt::HeaderView::ResizeToContents());
    ui()->parameters()->header()->setResizeMode(Qt::HeaderView::ResizeToContents());
    ui()->ports()->header()->setResizeMode(Qt::HeaderView::ResizeToContents());
    ui()->stats()->header()->setResizeMode(Qt::HeaderView::ResizeToContents());

}

sub NewModule
{

    our $wizardinfo;

    while (defined($wizardinfo) && $wizardinfo->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "awb-wizard new", 
            "The current module has been modified. Do you want to save it?",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSave()                if ($status == 0);
        $wizardinfo->modified(0)  if ($status == 1);
        return                    if ($status == 2);
    }

    while (defined($wizardinfo) && $wizardinfo->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "apm-edit new", 
            "The current module has been modified. Do you want to save it?",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSaveActivated()       if ($status == 0);
        $wizardinfo->modified(0)  if ($status == 1);
        return                    if ($status == 2);
    }
    
    $wizardinfo = Asim::Inifile->new();


    ui()->name()->setText("");
    ui()->description()->setText("");
    ui()->attributes()->setText("");
    ui()->provides_2()->setText("");

    my $author = (getpwnam($ENV{USER}))[6] || $ENV{USER};
    ui()->author()->setText($author);

    ui()->classtypeModule()->setDown(1);
    ui()->classtypeAlgorithm()->setDown(0);
    ui()->classtypeMessage()->setDown(0);
    
    ui()->classname()->setText("");

    ui()->requires()->clear();
    ui()->parameters()->clear();
    ui()->ports()->clear();
    ui()->stats()->clear();

    my $dir = `pwd`;
    chomp($dir);
    ui()->modulePath()->setText($dir);

    $wizardinfo->modified(0);
    setWindowTitle("Untitled - awb-wizard");

}

sub fileOpen
{

    our $wizardinfo;

    my $cwd;

    while (defined($wizardinfo) && $wizardinfo->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "awb-wizard open",
            "The current module specification has been modified. Do you want to save it?",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSave()                  if ($status == 0);
        $wizardinfo->modified(0)    if ($status == 1);
        return                      if ($status == 2);
    }

    if (defined($wizardinfo) && defined($wizardinfo->filename())) {
        $cwd = dirname($wizardinfo->filename());
    } else {
        chomp($cwd = `pwd`);
    }

    my $s = shift ||
        Qt::FileDialog::getOpenFileName(
            this,
            "Choose a module wizard file to edit",
            $cwd,
            "Asim Module Wizard File (*.amz)") ||
        return;;


    # Clear out old information

    NewModule();

    # Create inifile...

    my $m = Asim::Inifile->new($s);
    if (! defined $m) {
        Qt::MessageBox::information(
            this, 
            "awb-wizard open", 
            "File open failed");
        return;
    }

    $wizardinfo = $m;

    my @list;
    my $item;
    my $value;

    # General information

    $value = $wizardinfo->get("Global", "name") || "";
    ui()->name()->setText($value);

    $value = $wizardinfo->get("Global", "description") || "";
    ui()->description()->setText($value);

    $value = $wizardinfo->get("Global", "attributes") || "";
    ui()->attributes()->setText($value);

    $value = $wizardinfo->get("Global", "provides") || "";
    ui()->provides_2()->setText($value);


    # Class information
    $value = $wizardinfo->get("Class", "classtype") || "module";
    ui()->classtypeModule()->setChecked($value eq "module");
    ui()->classtypeAlgorithm()->setChecked($value eq "algorithm");
    ui()->classtypeMessage()->setChecked($value eq "message");

    $value = $wizardinfo->get("Class", "classname") || "";
    ui()->classname()->setText($value);

    $value = $wizardinfo->get("Class", "briefdescription") || "";
    ui()->briefDescription()->setText($value);

    $value = $wizardinfo->get("Class", "author") || "";
    ui()->author()->setText($value);


    # ui()->requires()

    @list = split(" ", $wizardinfo->get("ui()->requires()","requires"));

    foreach my $name (@list) {
        $item = Qt::TreeWidgetItem(ui()->requires(), undef);

        $value = $wizardinfo->get("Require.$name", "name") || "";
        $item->setText(0, $value);

        $value = $wizardinfo->get("Require.$name", "membername") || "";

        $value = $wizardinfo->get("Require.$name", "type") || "module";
        $item->setText(1, $value);

        $value = $wizardinfo->get("Require.$name", "classname") || "";
        $item->setText(2, $value);

        $value = $wizardinfo->get("Require.$name", "modulename") || "";
        # Not explictly setable

        $value = $wizardinfo->get("Require.$name", "isrequired") || "0";
        $item->setText(3, $value);

        $value = $wizardinfo->get("Require.$name", "isclocked") || "0";
        $item->setText(4, $value);
    }

    # parameters

    @list = split(" ", $wizardinfo->get("Parameters","parameters"));

    foreach my $name (@list) {
        $item = Qt::TreeWidgetItem(ui()->parameters(), undef);

        $value = $wizardinfo->get("Parameter.$name", "name") || "";
        $item->setText(0, $value);

        $value = $wizardinfo->get("Parameter.$name", "value") || "";
        $item->setText(1, $value);

        $value = $wizardinfo->get("Parameter.$name", "dynamic") || "0";
        $item->setText(2, $value);

        $value = $wizardinfo->get("Parameter.$name", "export") || "0";
        $item->setText(3, $value);

        $value = $wizardinfo->get("Parameter.$name", "description") || "";
        $item->setText(4, $value);
    }

    # Clocking

    $value = $wizardinfo->get("Clock", "notclocked") || "0";
    ui()->clockedNot()->setChecked($value);

    $value = $wizardinfo->get("Clock", "clockedbyparent") || "0";
    ui()->clockedByParent()->setChecked($value);

    # Ports

    @list = split(" ", $wizardinfo->get("Ports","ports"));

    foreach my $name (@list) {
        $item = Qt::TreeWidgetItem(ui()->ports(), undef);

        $value = $wizardinfo->get("Port.$name", "name") || "";
        $item->setText(0, $value);

        $value = $wizardinfo->get("Port.$name", "type") || "";
        $item->setText(1, $value);

        $value = $wizardinfo->get("Port.$name", "variable") || "";
        $item->setText(2, $value);

        $value = $wizardinfo->get("Port.$name", "class") || "";
        $item->setText(3, $value);

        $value = $wizardinfo->get("Port.$name", "latency") || "";
        $item->setText(4, $value);

        $value = $wizardinfo->get("Port.$name", "bandwidth") || "";
        $item->setText(5, $value);
    }

    # stats

    @list = split(" ", $wizardinfo->get("Stats","stats"));

    foreach my $name (@list) {
        $item = Qt::TreeWidgetItem(ui()->stats(), undef);

        $value = $wizardinfo->get("Stat.$name", "name") || "";
        $item->setText(0, $value);

        $value = $wizardinfo->get("Stat.$name", "variable") || "";
        $item->setText(1, $value);

        $value = $wizardinfo->get("Stat.$name", "size") || "";
        $item->setText(2, $value);

        $value = $wizardinfo->get("Stat.$name", "description") || "";
        $item->setText(3, $value);
    }


    # File related fields

    $value = $wizardinfo->get("Files", "public") || "";
    ui()->hfileName->setText($value);

    $value = $wizardinfo->get("Files", "private") || "";
    ui()->cppFilename()->setText($value);


    $value = $wizardinfo->get("Files", "modulepath") || "";
    $value = `awb-resolver --quiet $value` || $value;
    chomp($value);
    ui()->modulePath()->setText($value);

    $value = $wizardinfo->get("Files", "awbfilename") || "";
    ui()->awbFilename()->setText($value);

    $value = $wizardinfo->get("Files", "hfilename") || "";
    ui()->hfileName->setText($value);

    $value = $wizardinfo->get("Files", "cppfilename") || "";
    ui()->cppFilename()->setText($value);


    $wizardinfo->modified(0);
    setCaption(basename($wizardinfo->filename()) . " - awb-wizard");
    return;

}

sub fileSave
{

    our $wizardinfo;

    fileSaveAs($wizardinfo->filename());

}

sub fileSaveAs
{

    our $wizardinfo;

    my $cwd;
    my $status;

    my $s = shift;

    if (defined($wizardinfo) && defined($wizardinfo->filename())) {
        $cwd = dirname($wizardinfo->filename());
    } else {
        chomp($cwd = `pwd`);
    }


    if (!defined($s)) {
        $s = Qt::FileDialog::getSaveFileName(
            this,
            "Choose a filename to save module wizard file",
            $cwd,
            "Asim Module Wizard File (*.amz)",
            ) ||
            return;

        # Force extension to .amz

        if ($s !~ /\.amz$/) {
            $s .= ".amz";
        }

        if (-e $s) {
            my $status = Qt::MessageBox::warning ( 
                this, 
                "apm-edit save", 
                "The file $s exists. Overwrite it?",
                "&Yes",
                "&No",
                "Cancel",
                0,
                2);

            return if ($status == 1);
            return if ($status == 2);
        }
    }


    my $item;
    my @list;

    # General information

    $wizardinfo->put("Global", "name", ui()->name()->text());
    $wizardinfo->put("Global", "description", ui()->description()->text());
    $wizardinfo->put("Global", "attributes", ui()->attributes()->text());
    $wizardinfo->put("Global", "provides", ui()->provides_2()->text());

    # Class information

    $wizardinfo->put("Class", "classtype", "module")    if (ui()->classtypeModule()->isChecked());
    $wizardinfo->put("Class", "classtype", "algorithm") if (ui()->classtypeAlgorithm()->isChecked());
    $wizardinfo->put("Class", "classtype", "message")   if (ui()->classtypeMessage()->isChecked());

    $wizardinfo->put("Class", "classname", ui()->classname()->text());
    $wizardinfo->put("Class", "briefdescription", ui()->briefDescription()->text());
    $wizardinfo->put("Class", "author", ui()->author()->text());

    # Requires()

    @list = ();

    $item = ui()->requires()->topLevelItem(0);
    while (defined($item)) {
        my $name = $item->text(0);
        push(@list, $name);

        $wizardinfo->put("Require.$name", "name", $name);
        $wizardinfo->put("Require.$name", "membername", $name);
        $wizardinfo->put("Require.$name", "type", $item->text(1));
        $wizardinfo->put("Require.$name", "classname", $item->text(2));
        $wizardinfo->put("Require.$name", "modulename", $item->text(2));
        $wizardinfo->put("Require.$name", "isrequired", $item->text(3) || "0");
        $wizardinfo->put("Require.$name", "isclocked", $item->text(4) || "0");

        $item=$item->nextSibling();
    }

    $wizardinfo->put("Requires","requires", join(" ", @list));

    # Parameters()

    @list = ();

    $item = ui()->parameters()->topLevelItem(0);
    while (defined($item)) {
        my $name = $item->text(0);
        push(@list, $name);

        $wizardinfo->put("Parameter.$name", "name", $item->text(0));
        $wizardinfo->put("Parameter.$name", "value", $item->text(1));
        $wizardinfo->put("Parameter.$name", "dynamic", $item->text(2) || "0");
        $wizardinfo->put("Parameter.$name", "export", $item->text(3) || "0");
        $wizardinfo->put("Parameter.$name", "description", $item->text(4));

        $item=$item->nextSibling();
    }

    $wizardinfo->put("Parameters","parameters", join(" ", @list));

    # Clocking

    $wizardinfo->put("Clock", "notclocked", ui()->clockedNot()->isChecked() || "0");
    $wizardinfo->put("Clock", "clockedbyparent", ui()->clockedByParent()->isChecked() || "0");

    # Ports

    @list = ();

    $item = ui()->ports()->topLevelItem(0);
    while (defined($item)) {
        my $name = $item->text(0);
        push(@list, $name);

        $wizardinfo->put("Port.$name", "name", $item->text(0));
        $wizardinfo->put("Port.$name", "type", $item->text(1));
        $wizardinfo->put("Port.$name", "variable", $item->text(2));
        $wizardinfo->put("Port.$name", "class",  $item->text(3));
        $wizardinfo->put("Port.$name", "latency",  $item->text(4));
        $wizardinfo->put("Port.$name", "bandwidth", $item->text(5));

        $item=$item->nextSibling();
    }

    $wizardinfo->put("Ports","ports", join(" ", @list));

    # Stats

    @list = ();

    $item = ui()->stats()->topLevelItem(0);
    while (defined($item)) {
        my $name = $item->text(0);
        push(@list, $name);

        $wizardinfo->put("Stat.$name", "name", $item->text(0));
        $wizardinfo->put("Stat.$name", "variable", $item->text(1));
        $wizardinfo->put("Stat.$name", "size",  $item->text(2));
        $wizardinfo->put("Stat.$name", "description", $item->text(3));

        $item=$item->nextSibling();
    }

    $wizardinfo->put("Stats","stats", join(" ", @list));


    # Files

    $wizardinfo->put("Files", "public",  ui()->hfileName->text());
    $wizardinfo->put("Files", "private",  ui()->cppFilename()->text());

    my $value = ui()->modulePath()->text();
    $value = `awb-resolver --quiet --suffix $value` || $value;
    chomp($value);
    $wizardinfo->put("Files", "modulepath", $value);

    $wizardinfo->put("Files", "awbfilename", ui()->awbFilename()->text());
    $wizardinfo->put("Files", "hfilename", ui()->hfileName->text());
    $wizardinfo->put("Files", "cppfilename", ui()->cppFilename()->text());

    #
    # Save the file
    #

    $status = $wizardinfo->save($s);

    if (!defined($status)) {
        Qt::MessageBox::information(
            this, 
            "awb-wizard save",
            "File save failed");
        return;
    }


    setCaption(basename($wizardinfo->filename()) . " - awb-wizard");

}

sub fileBuild
{

    my $info = {};
    my $item;

    # General information

    $info->{name} = ui()->name()->text();
    $info->{description} = ui()->description()->text();
    $info->{attributes} = ui()->attributes()->text();
    $info->{provides} = ui()->provides_2()->text();

    # Class information

    $info->{ismodule} = ui()->classtypeModule()->isChecked();
    $info->{isalgorithm} = ui()->classtypeAlgorithm()->isChecked();
    $info->{ismessage} = ui()->classtypeMessage()->isChecked();

    $info->{classname} = ui()->classname()->text();
    $info->{briefdescription} = ui()->briefDescription()->text();
    $info->{author} = ui()->author()->text();

    # Requires

    $info->{requires} = ();
    
    $item = ui()->requires()->topLevelItem(0);
    my $i = 1;
    while (defined($item)) {
        my $p = {};
        $p->{name} = $item->text(0);
        $p->{membername} = $p->{name};
        $p->{type} = $item->text(1);
        $p->{ismodule} = $p->{type} eq "module";
        $p->{isalgorithm} = $p->{type} eq "algorithm";
        $p->{ismessage} = $p->{type} eq "message";
        $p->{classname} = $item->text(2);
        $p->{modulename} = $p->{classname};
        $p->{isrequired} = $item->text(3);
        $p->{isclocked} = $item->text(4);
        push(@{$info->{requires}}, $p);
        $item=ui()->requires()->topLevelItem($i++);
    }

    # Parameters

    $info->{parameter} = ();
    
    $item = ui()->parameters()->topLevelItem(0);
    $i = 1;
    while (defined($item)) {
        my $p = {};
        $p->{name} = $item->text(0);
        $p->{value} = $item->text(1);
        $p->{dynamic} = $item->text(2)?"%dynamic":"";;
        $p->{export} = $item->text(3)?"%export":"%param";;
        $p->{description} = $item->text(4);
        push(@{$info->{parameter}}, $p);
        $item=ui()->requires()->topLevelItem($i++);
    }

    # Clocking

    $info->{isclocked} = ! ui()->clockedNot()->isChecked();

    # Ports

    $info->{port} = ();
    
    $item = ui()->ports()->topLevelItem(0);
    $i = 1;
    while (defined($item)) {
        my $p = {};
        $p->{name} = $item->text(0);
        $p->{readport} = ($item->text(1) eq "Read");
        $p->{writeport} = ($item->text(1) eq "Write");
        $p->{variable} = $item->text(2);
        $p->{class} =  $item->text(3);
        $p->{latency} =  ($item->text(4) ne "")?($item->text(4)):undef;
        $p->{bandwidth} = ($item->text(5) ne "")?($item->text(5)):undef;
        push(@{$info->{port}}, $p);
        $item=ui()->requires()->topLevelItem($i++);
    }

    # Stats

    $info->{statistic} = ();
    
    $item = ui()->stats()->topLevelItem(0);
    $i = 1;
    while (defined($item)) {
        my $p = {};
        $p->{name} = $item->text(0);
        $p->{variable} = $item->text(1);
        $p->{size} =  $item->text(2);
        $p->{description} = $item->text(3);
        $p->{scalar} = ($p->{size} eq "1");
        $p->{histogram} =  ($p->{size} ne "1");
        push(@{$info->{statistic}}, $p);
        $item=ui()->requires()->topLevelItem($i++);
    }


    $info->{public} =  ui()->hfileName->text();
    $info->{private} =  ui()->cppFilename()->text();

    $info->{modulepath} = ui()->modulePath()->text();
    $info->{awbfilename} = ui()->awbFilename()->text();
    $info->{hfilename} = ui()->hfileName->text();
    $info->{cppfilename} = ui()->cppFilename()->text();

    # Error checks...

    if ( !ui()->awbFilename()->text() ||
         !ui()->hfileName->text()   ||
         !ui()->cppFilename()->text()) {
        Qt::MessageBox::information(
            this, 
            "apm-wizard build", 
            "Filenames for the .awb, .h, and .cpp files must be specified");

        return;
    }

    my $path = $info->{modulepath};
    if (! -d $path) {
        Qt::MessageBox::information(
            this, 
            "apm-wizard build", 
            "Directory $path does not exist");

        return;
    }

    write_module($info);

}

sub fileExit
{

    our $wizardinfo;

    while ($wizardinfo->modified()) {
        my $status = Qt::MessageBox::warning ( 
            this, 
            "awb-wizard exit", 
            "The file has been modified. Do you want to save it first",
            "&Yes",
            "&No",
            "Cancel",
            0,
            2);

        fileSave()                if ($status == 0);
        $wizardinfo->modified(0)  if ($status == 1);
        return                    if ($status == 2);
    }

   this->close();


}

sub editUpdate
{

    my $item;

    if (ui()->parameters()->hasFocus()) {

        $item = ui()->parameters()->currentItem();
        Parameters_Update($item);

    } elsif (ui()->ports()->hasFocus()) {

        $item = ui()->ports()->currentItem();
        Ports_Update($item);

    } elsif (ui()->stats()->hasFocus()) {

        $item = ui()->stats()->currentItem();
        Stats_Update($item);

    } elsif (ui()->requires()->hasFocus()) {

        $item = ui()->requires()->currentItem();
        Requires_Update($item);
    }


}

sub editCut
{

    my $listbox = undef;

    if (ui()->parameters()->hasFocus()) {
        $listbox = ui()->parameters();
    } elsif (ui()->ports()->hasFocus()) {
        $listbox = ui()->ports();
    } elsif (ui()->stats()->hasFocus()) {
        $listbox = ui()->stats();
    } elsif (ui()->requires()->hasFocus()) {
        $listbox = ui()->requires();
    }

    if (defined($listbox)) {
        my $item = $listbox->currentItem() || return;
        $listbox->takeItem($item);
    }

}

sub Name_textChanged
{

    my $name = shift;
    $name =~ s/ /_/g;
    $name = lc($name);

    Any_textChanged("");

}

sub Autofill_clicked
{

    my $name = ui()->name()->text();
    my $name_clean = lc($name);
    $name_clean =~ s/ /_/g;

    ui()->awbFilename()->setText("$name_clean.awb");
    ui()->hfileName->setText("$name_clean.h");
    ui()->cppFilename()->setText("$name_clean.cpp");

    my $provides = ui()->provides_2()->text();
    my $classname = uc($provides);

    ui()->classname()->setText($classname);
    ui()->briefDescription()->setText("Implementation of module '$name' of asim module type '$provides'");

}

sub Any_textChanged
{

    our $wizardinfo;

    my $text = shift;

    $wizardinfo->modified(1);

}

sub contextMenuRequested
{

    my $item = shift || return;
    my $point = shift || return;

    editMenu->popup($point);
    editMenu->exec();

}

sub Requires_Update
{

    my $item = shift || return;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $type = $item->text(1);
    my $classname = $item->text(2);
    my $isrequired = $item->text(3);
    my $isclocked = $item->text(4);

    # Fill in update fields

    ui()->requiresName()->setText($name);
    ui()->requiresType()->setCurrentText($type);
    ui()->requiresClassname()->setText($classname);
    ui()->isRequired()->setChecked($isrequired);
    ui()->isClocked()->setChecked($isclocked);

    # Remove item from listbox

    ui()->requires()->takeItem($item);

}

sub RequiresName_textChanged
{

    my $name = shift;
    $name =~ s/ /_/g;
    $name = uc($name);

    ui()->requiresClassname()->setText($name);

    Any_textChanged("");

}

sub Requires_Add_clicked
{

    # Get values out of update fields

    my $name = ui()->requiresName()->text();
    my $type = ui()->requiresType()->currentText();
    my $classname = ui()->requiresClassname()->text();
    my $isrequired = ui()->isRequired()->isChecked() || "0";
    my $isclocked = ui()->isClocked()->isChecked() || "0";

    # Clear update fields

    ui()->requiresName()->setText("");
    ui()->requiresType()->setCurrentIndex(0);
    ui()->requiresClassname()->setText("");
    ui()->isRequired()->setChecked(1);
    ui()->isClocked()->setChecked(1);

    # Create item in tree widget

    my $item = Qt::TreeWidgetItem(ui()->requires(), 0);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($type));
    $item->setText(2, trUtf8($classname));
    $item->setText(3, trUtf8($isrequired));
    $item->setText(4, trUtf8($isclocked));

    #
    # Mark modified and
    # send focus back to ui()->name() to begin another entry
    #
    Any_textChanged("");
    ui()->requiresName()->setFocus();

}

sub Parameters_Update
{

    my $item = shift;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $value = $item->text(1);
    my $export = $item->text(2);
    my $dynamic = $item->text(3);
    my $description = $item->text(4);

    # Fill in update fields

    ui()->paramName()->setText($name);
    ui()->paramValue()->setText($value);
    ui()->paramDescription()->setText($description);
    ui()->paramExport()->setChecked($export);
    ui()->paramDynamic()->setChecked($dynamic);

    # Remove item from listbox

    ui()->parameters()->takeItem($item);

}

sub Param_Add_clicked
{


    # Get values out of update fields

    my $name = ui()->paramName()->text();
    my $value = ui()->paramValue()->text();
    my $description = ui()->paramDescription()->text();
    my $export = ui()->paramExport()->isChecked() || "0";
    my $dynamic = ui()->paramDynamic()->isChecked() || "0";

    # Clear update fields

    ui()->paramName()->setText("");
    ui()->paramValue()->setText("");
    ui()->paramDescription()->setText("");
    ui()->paramExport()->setChecked(0);
    ui()->paramDynamic()->setChecked(0);

    # Create item in tree widget

    my $item = Qt::TreeWidgetItem(ui()->parameters(), 0);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($value));
    $item->setText(2, trUtf8($dynamic));
    $item->setText(3, trUtf8($export));
    $item->setText(4, trUtf8($description));

    #
    # Mark modified and
    # send focus back to ui()->name() to begin another entry
    #
    Any_textChanged("");
    ui()->paramName()->setFocus();


}

sub Ports_Update
{

    my $item = shift;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $type = $item->text(1);
    my $variable = $item->text(2);
    my $class = $item->text(3);
    my $latency = $item->text(4);
    my $bandwidth = $item->text(5);

    # Fill in update fields

    ui()->portName()->setText($name);
    ui()->portType()->setCurrentText($type);
    ui()->portVariable()->setText($variable);
    ui()->portClass()->setText($class);
    ui()->portLatency()->setText($latency);
    ui()->portBandwidth()->setText($bandwidth);

    # Remove item from listbox

    ui()->ports()->takeItem($item);

}

sub Port_Add_clicked
{


    # Get values out of update fields

    my $name = ui()->portName()->text();
    my $type = ui()->portType()->currentText();
    my $variable = ui()->portVariable()->text();
    my $class = ui()->portClass()->text();
    my $latency = ui()->portLatency()->text();
    my $bandwidth = ui()->portBandwidth()->text();

    # Clear update fields

    ui()->portName()->setText("");
    ui()->portType()->setCurrentIndex(0);
    ui()->portVariable()->setText("");
    ui()->portClass()->setText("");
    ui()->portLatency()->setText("");
    ui()->portBandwidth()->setText("");

    # Create item in tree widget

    my $item = Qt::TreeWidgetItem(ui()->ports(), 0);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($type));
    $item->setText(2, trUtf8($variable));
    $item->setText(3, trUtf8($class));
    $item->setText(4, trUtf8($latency));
    $item->setText(5, trUtf8($bandwidth));


    #
    # Mark modified and
    # send focus back to ui()->name() to begin another entry
    #
    Any_textChanged("");
    ui()->portName()->setFocus();


}

sub Stats_Update
{

    my $item = shift;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $var = $item->text(1);
    my $size = $item->text(2);
    my $description = $item->text(3);

    # Fill in update fields

    ui()->statName()->setText($name);
    ui()->statVar()->setText($var);
    ui()->statSize()->setText($size);
    ui()->statDescription->setText($description);

    # Remove item from listbox

    ui()->stats()->takeItem($item);

}

sub Stat_Add_clicked
{


    # Get values out of update fields

    my $name = ui()->statName()->text();
    my $var = ui()->statVar()->text();
    my $size = ui()->statSize()->text();
    my $description = ui()->statDescription->text();

    # Clear update fields

    ui()->statName()->setText("");
    ui()->statVar()->setText("");
    ui()->statSize()->setText("1");
    ui()->statDescription->setText("");

    # Create item in listbox

    my $item = Qt::TreeWidgetItem(ui()->stats, 0);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($var));
    $item->setText(2, trUtf8($size));
    $item->setText(3, trUtf8($description));

   
    #
    # Mark modified and
    # send focus back to ui()->name() to begin another entry
    #
    Any_textChanged("");
    ui()->statName()->setFocus();

}

sub nextStep_1_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_2());
    ui()->classname()->setFocus();

}

sub nextStep_2_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_3());
    ui()->requiresName()->setFocus();

}

sub nextStep_3_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_4());
    ui()->paramName()->setFocus();

}

sub nextStep_4_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_5());
    ui()->clockedByParent()->setFocus();

}

sub nextStep_5_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_6());
    ui()->portName()->setFocus();

}

sub nextStep_6_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_7());
    ui()->statName()->setFocus();

}

sub nextStep_8_clicked
{

    ui()->tabs_main()->setCurrentWidget(ui()->tab_8());
    ui()->modulePath()->setFocus();

}

sub browsePath_clicked
{

    
    my $fd;

    $fd = Qt::FileDialog( this, "file dialog", 1 );
    $fd->setFileMode(Qt::FileDialog::Directory());
    $fd->setOptions(Qt::FileDialog::ShowDirsOnly());
    $fd->setDirectory(ui()->modulePath()->text());
    $fd->show();

    if ( $fd->exec()) {
        ui()->modulePath()->setText($fd->selectedFiles->[0]);

    }

}

sub Done_clicked
{

    fileBuild();

}

sub PortName_textChanged
{

    my $name = shift;
    $name =~ s/ /_/g;
    $name = lc($name);

    ui()->portVariable()->setText($name);

    Any_textChanged("");

}

sub helpWhatsThisAction_activated 
{
    Qt::WhatsThis::enterWhatsThisMode();
}

sub about_activated
{
    my $a = awb_wizard_about->NEW();
    $a->exec();

}
