#
# Copyright (C) 2003-2006 Intel Corporation
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

package Asim::Model::Update;
use warnings;
use strict;


=head1 NAME

Asim::Model::Update - Library for updating .apm file to latest version.

=head1 SYNOPSIS

TBD

=head1 DESCRIPTION

This module provides an object to manipulate...

=cut

################################################################

=head1 METHODS

The following public methods are supported:

=over 4

=item $modelDB = Asim::Model::DB-E<gt>update($filename)

Check if the given .apm file is broken. If not, check if it
has any moved modules or is not the latest version. If so,
open it and save it.

=cut

################################################################

sub update {
  my $filename = shift;
  
  my $model = Asim::Model->new($filename) || return undef;
  
  my $is_broken = $model->is_broken();
  my $is_stale = $model->is_stale();
  my $is_old_version = $model->version() < Asim::Model::latest_version();
  
  my $modelname = $model->name();
  
  if ($is_broken) {
    
    print("Unable to update broken model: $filename\n");
    return 0;
    
  } elsif ($is_stale || $is_old_version) {
  
    print("Updating model $modelname.");

    if ($is_stale) {
      print(" (has stale information");
      
      if ($model->moved_module_count() > 0) {
	print(", including moved files");
      }
      
      print(")");

    }

    if ($is_old_version) {
      print(" (old file format)");
    }
    
    print("\n");
    
    # This is actually all we have to do to update it.
    return ($model->save());
  
  } else {
  
    print ("Model $modelname is up to date.\n");
    return 1;
  
  }

}


=back


=head1 AUTHORS

Michael Pellauer

=head1 COPYRIGHT

Copyright (c) Intel Computer Corporation, 2011

All Rights Reserved.  Unpublished rights reserved
under the copyright laws of the United States.

=cut


1;
