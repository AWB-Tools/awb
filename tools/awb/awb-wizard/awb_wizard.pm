package awb_wizard;

use strict;
use warnings;

use QtCore4;
use QtGui4;
use QtCore4::isa qw(Qt::MainWindow);

sub ui()
{
    return this->{ui};
}

sub NEW
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Awb_wizard->setupUi(this);
}

