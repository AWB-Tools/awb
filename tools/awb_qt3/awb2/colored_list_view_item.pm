use strict;

package ColoredListViewItem;
use Qt;
use Qt::isa qw(Qt::ListViewItem);
use Qt::attributes qw(backgroundColor);

sub NEW
{
  my $self = shift;
  my $parent = shift;
  my $label1 = shift;
  my $label2 = shift;
  $self->SUPER::NEW($parent, $label1, $label2);
  backgroundColor = Qt::Color(@_);
}

sub paintCell
{
  my $painter = shift;
  my $cg = shift;
  my $column = shift;
  my $width = shift;
  my $align = shift;

  my $grp = Qt::ColorGroup($cg);

  $painter->save();

  $grp->setColor(Qt::ColorGroup::Base(), backgroundColor);
  SUPER->paintCell($painter, $grp, $column, $width, $align);
  $painter->restore();
}
    
1;
