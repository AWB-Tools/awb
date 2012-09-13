package awb_wizard_about;

use strict;
use warnings;

use QtCore4;
use QtGui4;
use QtCore4::isa qw(Qt::Dialog);

sub NEW 
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Awb_about->setupUi(this);
    return this();
}

