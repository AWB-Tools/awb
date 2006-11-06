/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

##################################################################################
#
# Copyright (C) 2002-2006 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#
##################################################################################

# Author: Joel Emer
# Date: November 2002


#
#   Creation of window pane contents
#

void awb_wizard::init()
{
    use File::Basename;
    use Asim::Inifile;
    use awb_about;

    print "awb_wizard::init - We are in init\n" if $::debug;

    NewModule();
}

#
# File menu actions
#

void awb_wizard::NewModule()
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


    Name->setText("");
    Description->setText("");
    Attributes->setText("");
    Provides_2->setText("");

    my $author = (getpwnam($ENV{USER}))[6] || $ENV{USER};
    Author->setText($author);

    ClasstypeModule->isChecked(1);
    ClasstypeAlgorithm->isChecked(0);
    ClasstypeMessage->isChecked(0);
    
    Classname->setText("");

    Requires->clear();
    Parameters->clear();
    Ports->clear();
    Stats->clear();

    my $dir = `pwd`;
    chomp($dir);
    ModulePath->setText($dir);

    $wizardinfo->modified(0);
    setCaption("Untitled - awb-wizard");
}



void awb_wizard::fileOpen()
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
            $cwd,
            "Asim Module Wizard File (*.amz)",
            this,
            "open asim module wizard file dialog",
            "Choose a module wizard file to edit" ) ||
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
    Name->setText($value);

    $value = $wizardinfo->get("Global", "description") || "";
    Description->setText($value);

    $value = $wizardinfo->get("Global", "attributes") || "";
    Attributes->setText($value);

    $value = $wizardinfo->get("Global", "provides") || "";
    Provides_2->setText($value);


    # Class information
    $value = $wizardinfo->get("Class", "classtype") || "module";
    ClasstypeModule->setChecked($value eq "module");
    ClasstypeAlgorithm->setChecked($value eq "algorithm");
    ClasstypeMessage->setChecked($value eq "message");

    $value = $wizardinfo->get("Class", "classname") || "";
    Classname->setText($value);

    $value = $wizardinfo->get("Class", "briefdescription") || "";
    BriefDescription->setText($value);

    $value = $wizardinfo->get("Class", "author") || "";
    Author->setText($value);


    # Requires

    @list = split(" ", $wizardinfo->get("Requires","requires"));

    foreach my $name (@list) {
        $item = Qt::ListViewItem(Requires, undef);

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

    # Parameters

    @list = split(" ", $wizardinfo->get("Parameters","parameters"));

    foreach my $name (@list) {
        $item = Qt::ListViewItem(Parameters, undef);

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
    ClockedNot->setChecked($value);

    $value = $wizardinfo->get("Clock", "clockedbyparent") || "0";
    ClockedByParent->setChecked($value);

    # Ports

    @list = split(" ", $wizardinfo->get("Ports","ports"));

    foreach my $name (@list) {
        $item = Qt::ListViewItem(Ports, undef);

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

    # Stats

    @list = split(" ", $wizardinfo->get("Stats","stats"));

    foreach my $name (@list) {
        $item = Qt::ListViewItem(Stats, undef);

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
    HFilename->setText($value);

    $value = $wizardinfo->get("Files", "private") || "";
    CppFilename->setText($value);


    $value = $wizardinfo->get("Files", "modulepath") || "";
    $value = `awb-resolver --quiet $value` || $value;
    chomp($value);
    ModulePath->setText($value);

    $value = $wizardinfo->get("Files", "awbfilename") || "";
    AwbFilename->setText($value);

    $value = $wizardinfo->get("Files", "hfilename") || "";
    HFilename->setText($value);

    $value = $wizardinfo->get("Files", "cppfilename") || "";
    CppFilename->setText($value);


    $wizardinfo->modified(0);
    setCaption(basename($wizardinfo->filename()) . " - awb-wizard");
    return;
}


void awb_wizard::fileSave()
{
    our $wizardinfo;

    fileSaveAs($wizardinfo->filename());
}


void awb_wizard::fileSaveAs()
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
            $cwd,
            "Asim Module Wizard File (*.amz)",
            this,
            "save asim module wizard file dialog",
            "Choose a filename to save module wizard file" ) ||
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

    $wizardinfo->put("Global", "name", Name->text());
    $wizardinfo->put("Global", "description", Description->text());
    $wizardinfo->put("Global", "attributes", Attributes->text());
    $wizardinfo->put("Global", "provides", Provides_2->text());

    # Class information

    $wizardinfo->put("Class", "classtype", "module")    if (ClasstypeModule->isChecked());
    $wizardinfo->put("Class", "classtype", "algorithm") if (ClasstypeAlgorithm->isChecked());
    $wizardinfo->put("Class", "classtype", "message")   if (ClasstypeMessage->isChecked());

    $wizardinfo->put("Class", "classname", Classname->text());
    $wizardinfo->put("Class", "briefdescription", BriefDescription->text());
    $wizardinfo->put("Class", "author", Author->text());

    # Requires

    @list = ();

    $item = Requires->firstChild();
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

    # Parameters

    @list = ();

    $item = Parameters->firstChild();
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

    $wizardinfo->put("Clock", "notclocked", ClockedNot->isChecked() || "0");
    $wizardinfo->put("Clock", "clockedbyparent", ClockedByParent->isChecked() || "0");

    # Ports

    @list = ();

    $item = Ports->firstChild();
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

    $item = Stats->firstChild();
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

    $wizardinfo->put("Files", "public",  HFilename->text());
    $wizardinfo->put("Files", "private",  CppFilename->text());

    my $value = ModulePath->text();
    $value = `awb-resolver --quiet --suffix $value` || $value;
    chomp($value);
    $wizardinfo->put("Files", "modulepath", $value);

    $wizardinfo->put("Files", "awbfilename", AwbFilename->text());
    $wizardinfo->put("Files", "hfilename", HFilename->text());
    $wizardinfo->put("Files", "cppfilename", CppFilename->text());

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


void awb_wizard::fileBuild()
{
    my $info = {};
    my $item;

    # General information

    $info->{name} = Name->text();
    $info->{description} = Description->text();
    $info->{attributes} = Attributes->text();
    $info->{provides} = Provides_2->text();

    # Class information

    $info->{ismodule} = ClasstypeModule->isChecked();
    $info->{isalgorithm} = ClasstypeAlgorithm->isChecked();
    $info->{ismessage} = ClasstypeMessage->isChecked();

    $info->{classname} = Classname->text();
    $info->{briefdescription} = BriefDescription->text();
    $info->{author} = Author->text();

    # Requires

    $info->{requires} = ();
    
    $item = Requires->firstChild();
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
        $item=$item->nextSibling();
    }

    # Parameters

    $info->{parameter} = ();
    
    $item = Parameters->firstChild();
    while (defined($item)) {
        my $p = {};
        $p->{name} = $item->text(0);
        $p->{value} = $item->text(1);
        $p->{dynamic} = $item->text(2)?"%dynamic":"";;
        $p->{export} = $item->text(3)?"%export":"%param";;
        $p->{description} = $item->text(4);
        push(@{$info->{parameter}}, $p);
        $item=$item->nextSibling();
    }

    # Clocking

    $info->{isclocked} = ! ClockedNot->isChecked();

    # Ports

    $info->{port} = ();
    
    $item = Ports->firstChild();
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
        $item=$item->nextSibling();
    }

    # Stats

    $info->{statistic} = ();
    
    $item = Stats->firstChild();
    while (defined($item)) {
        my $p = {};
        $p->{name} = $item->text(0);
        $p->{variable} = $item->text(1);
        $p->{size} =  $item->text(2);
        $p->{description} = $item->text(3);
        $p->{scalar} = ($p->{size} eq "1");
        $p->{histogram} =  ($p->{size} ne "1");
        push(@{$info->{statistic}}, $p);
        $item=$item->nextSibling();
    }


    $info->{public} =  HFilename->text();
    $info->{private} =  CppFilename->text();

    $info->{modulepath} = ModulePath->text();
    $info->{awbfilename} = AwbFilename->text();
    $info->{hfilename} = HFilename->text();
    $info->{cppfilename} = CppFilename->text();

    # Error checks...

    if ( !AwbFilename->text() ||
         !HFilename->text()   ||
         !CppFilename->text()) {
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




void awb_wizard::fileExit()
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

#
# Edit menu actions
#

void awb_wizard::editUpdate()
{
    my $item;

    if (Parameters->hasFocus()) {

        $item = Parameters->currentItem();
        Parameters_Update($item);

    } elsif (Ports->hasFocus()) {

        $item = Ports->currentItem();
        Ports_Update($item);

    } elsif (Stats->hasFocus()) {

        $item = Stats->currentItem();
        Stats_Update($item);

    } elsif (Requires->hasFocus()) {

        $item = Requires->currentItem();
        Requires_Update($item);
    }

}

void awb_wizard::editCut()
{
    my $listbox = undef;

    if (Parameters->hasFocus()) {
        $listbox = Parameters;
    } elsif (Ports->hasFocus()) {
        $listbox = Ports;
    } elsif (Stats->hasFocus()) {
        $listbox = Stats;
    } elsif (Requires->hasFocus()) {
        $listbox = Requires;
    }

    if (defined($listbox)) {
        my $item = $listbox->currentItem() || return;
        $listbox->takeItem($item);
    }
}


#
# Windows actions
#


void awb_wizard::Name_textChanged( const QString & )
{
    my $name = shift;
    $name =~ s/ /_/g;
    $name = lc($name);

    Any_textChanged("");
}




void awb_wizard::Autofill_clicked()
{
    my $name = Name->text();
    my $name_clean = lc($name);
    $name_clean =~ s/ /_/g;

    AwbFilename->setText("$name_clean.awb");
    HFilename->setText("$name_clean.h");
    CppFilename->setText("$name_clean.cpp");

    my $provides = Provides_2->text();
    my $classname = uc($provides);

    Classname->setText($classname);
    BriefDescription->setText("Implementation of module '$name' of asim module type '$provides'");
}



void awb_wizard::Any_textChanged( const QString & )
{
    our $wizardinfo;

    my $text = shift;

    $wizardinfo->modified(1);
}


#
# General purpose context menu for list boxes
#
void awb_wizard::contextMenuRequested( QListViewItem *, const QPoint &, int )
{
    my $item = shift || return;
    my $point = shift || return;

    editMenu->popup($point);
    editMenu->exec();
}





void awb_wizard::Requires_Update( QListViewItem * )
{
    my $item = shift || return;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $type = $item->text(1);
    my $classname = $item->text(2);
    my $isrequired = $item->text(3);
    my $isclocked = $item->text(4);

    # Fill in update fields

    RequiresName->setText($name);
    RequiresType->setCurrentText($type);
    RequiresClassname->setText($classname);
    IsRequired->setChecked($isrequired);
    IsClocked->setChecked($isclocked);

    # Remove item from listbox

    Requires->takeItem($item);
} 



void awb_wizard::RequiresName_textChanged( const QString & )
{
    my $name = shift;
    $name =~ s/ /_/g;
    $name = uc($name);

    RequiresClassname->setText($name);

    Any_textChanged("");
}

void awb_wizard::Requires_Add_clicked()
{
    # Get values out of update fields

    my $name = RequiresName->text();
    my $type = RequiresType->currentText();
    my $classname = RequiresClassname->text();
    my $isrequired = IsRequired->isChecked() || "0";
    my $isclocked = IsClocked->isChecked() || "0";

    # Clear update fields

    RequiresName->setText("");
    RequiresType->setCurrentItem(0);
    RequiresClassname->setText("");
    IsRequired->setChecked(1);
    IsClocked->setChecked(1);

    # Create item in listbox

    my $item = Qt::ListViewItem(Requires, undef);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($type));
    $item->setText(2, trUtf8($classname));
    $item->setText(3, trUtf8($isrequired));
    $item->setText(4, trUtf8($isclocked));

    #
    # Mark modified and
    # send focus back to Name to begin another entry
    #
    Any_textChanged("");
    RequiresName->setFocus();
}



void awb_wizard::Parameters_Update( QListViewItem * )
{
    my $item = shift;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $value = $item->text(1);
    my $export = $item->text(2);
    my $dynamic = $item->text(3);
    my $description = $item->text(4);

    # Fill in update fields

    ParamName->setText($name);
    ParamValue->setText($value);
    ParamDescription->setText($description);
    ParamExport->setChecked($export);
    ParamDynamic->setChecked($dynamic);

    # Remove item from listbox

    Parameters->takeItem($item);
}


void awb_wizard::Param_Add_clicked()
{

    # Get values out of update fields

    my $name = ParamName->text();
    my $value = ParamValue->text();
    my $description = ParamDescription->text();
    my $export = ParamExport->isChecked() || "0";
    my $dynamic = ParamDynamic->isChecked() || "0";

    # Clear update fields

    ParamName->setText("");
    ParamValue->setText("");
    ParamDescription->setText("");
    ParamExport->setChecked(0);
    ParamDynamic->setChecked(0);

    # Create item in listbox

    my $item = Qt::ListViewItem(Parameters, undef);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($value));
    $item->setText(2, trUtf8($dynamic));
    $item->setText(3, trUtf8($export));
    $item->setText(4, trUtf8($description));

    #
    # Mark modified and
    # send focus back to Name to begin another entry
    #
    Any_textChanged("");
    ParamName->setFocus();

}




void awb_wizard::Ports_Update( QListViewItem * )
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

    PortName->setText($name);
    PortType->setCurrentText($type);
    PortVariable->setText($variable);
    PortClass->setText($class);
    PortLatency->setText($latency);
    PortBandwidth->setText($bandwidth);

    # Remove item from listbox

    Ports->takeItem($item);
}



void awb_wizard::PortName_textChanged( const QString & )
{
    my $name = shift;
    $name =~ s/ /_/g;
    $name = lc($name);

    PortVariable->setText($name);

    Any_textChanged("");
}

void awb_wizard::Port_Add_clicked()
{

    # Get values out of update fields

    my $name = PortName->text();
    my $type = PortType->currentText();
    my $variable = PortVariable->text();
    my $class = PortClass->text();
    my $latency = PortLatency->text();
    my $bandwidth = PortBandwidth->text();

    # Clear update fields

    PortName->setText("");
    PortType->setCurrentItem(0);
    PortVariable->setText("");
    PortClass->setText("");
    PortLatency->setText("");
    PortBandwidth->setText("");

    # Create item in listbox

    my $item = Qt::ListViewItem(Ports, undef);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($type));
    $item->setText(2, trUtf8($variable));
    $item->setText(3, trUtf8($class));
    $item->setText(4, trUtf8($latency));
    $item->setText(5, trUtf8($bandwidth));

    #
    # Mark modified and
    # send focus back to Name to begin another entry
    #
    Any_textChanged("");
    PortName->setFocus();

}



void awb_wizard::Stats_Update( QListViewItem * )
{
    my $item = shift;

    # Get values out of listbox for update fields

    my $name = $item->text(0);
    my $var = $item->text(1);
    my $size = $item->text(2);
    my $description = $item->text(3);

    # Fill in update fields

    StatName->setText($name);
    StatVar->setText($var);
    StatSize->setText($size);
    StatDescription->setText($description);

    # Remove item from listbox

    Stats->takeItem($item);
}

void awb_wizard::Stat_Add_clicked()
{

    # Get values out of update fields

    my $name = StatName->text();
    my $var = StatVar->text();
    my $size = StatSize->text();
    my $description = StatDescription->text();

    # Clear update fields

    StatName->setText("");
    StatVar->setText("");
    StatSize->setText("1");
    StatDescription->setText("");

    # Create item in listbox

    my $item = Qt::ListViewItem(Stats, undef);

    $item->setText(0, trUtf8($name));
    $item->setText(1, trUtf8($var));
    $item->setText(2, trUtf8($size));
    $item->setText(3, trUtf8($description));

    #
    # Mark modified and
    # send focus back to Name to begin another entry
    #
    Any_textChanged("");
    StatName->setFocus();
}


# Next and Done buttons

void awb_wizard::nextStep_1_clicked()
{
    Tabs_main->showPage(tab_2);
    Classname->setFocus();
}


void awb_wizard::nextStep_2_clicked()
{
    Tabs_main->showPage(tab_3);
    RequiresName->setFocus();
}


void awb_wizard::nextStep_3_clicked()
{
    Tabs_main->showPage(tab_4);
    ParamName->setFocus();
}


void awb_wizard::nextStep_4_clicked()
{
    Tabs_main->showPage(tab_5);
    ClockedByParent->setFocus();
}


void awb_wizard::nextStep_5_clicked()
{
    Tabs_main->showPage(tab_6);
    PortName->setFocus();
}


void awb_wizard::nextStep_6_clicked()
{
    Tabs_main->showPage(tab_7);
    StatName->setFocus();
}


void awb_wizard::nextStep_8_clicked()
{
    Tabs_main->showPage(tab_8);
    ModulePath->setFocus();
}


void awb_wizard::browsePath_clicked()
{
    
    my $fd;

    $fd = Qt::FileDialog( this, "file dialog", 1 );
    $fd->setMode(&Qt::FileDialog::DirectoryOnly);
    $fd->setDir(ModulePath->text());
    $fd->show();

    if ( $fd->exec()) {
        ModulePath->setText($fd->selectedFile());
    }
}


void awb_wizard::Done_clicked()
{
    fileBuild();
}


#
# Todo:
#
#     Command line switches:
#          --name=<name>
#          --provides=<provides>
#
#     Warnings on overwrite of output files
#
#     On "next" button force focus on next textbox
#
#
# Bugs:
#     Change Provides_2 back to Provides...
#     Change Nextstep_2a back to Nextstep2...
#     Poor resizing on various group boxes
#



