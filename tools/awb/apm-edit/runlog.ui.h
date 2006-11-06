/*
 * Copyright (C) 2002-2006 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#
# TBD: 
#      Add a save option...
#

void RunLog::init()
{
    our $debug = 0;

    print "Init\n" if $debug;
}

void RunLog::run()
{
    our $debug = 0;
    my $command = shift;

    my $a = $main::app;

    our $running = 0;

    my $rin;
    my $win;
    my $ein;
    my $rout;
    my $timeout = 1;
    my $count = 0;

    my $nfound;
    my $timeleft;
    my $s; 
    my $x;

    print "Runit\n" if $debug;

    Log1->insert("$command\n");

    show();
    $a->processEvents();

    open(LOG, '-|', $command . " 2>&1") || die("Open failed");
    print "Open: OK\n" if $debug;

    $running = 1;
    Kill->setEnabled(1);

    $rin = $win = $ein = '';
    vec($rin,fileno(*LOG),1) = 1;
#   vec($win,fileno(STDOUT),1) = 1;
    $ein = $rin | $win;

    while ($running) {

        ($nfound,$timeleft) = select($rout=$rin, undef, undef, $timeout);
        if (! defined($nfound)) {
            print "Select failed\n" if $debug;
            $running = 0;
            last;
        }

        print $count++ . ": Nfound = $nfound, Timeleft = $timeleft\n" if $debug;
        if ($nfound) {
            my $first = 1;
            if  ($first || $s) {
                $s = sysread(*LOG, $x="", 1024);
                if (!defined($s) || ($first && ! $s)) {
                    print "End of file\n" if $debug;
                    $running = 0;
                    last;
                }
                $first = 0;
                print "$x\n" if $debug;
                Log1->insert($x);
                $a->processEvents();  
            }
        } else {
            print "Nothing to read\n" if $debug;
            $a->processEvents();  
        }
    }

    close(LOG);

    Log1->insert("\n");
    Log1->insert("*******DONE******\n");
    Log1->insert("\n");

    Ok->setEnabled(1);
    Kill->setEnabled(0);

    $a->exec();
}


void RunLog::Kill_clicked()
{
    our $debug;
    our $running;

    print "Kill!\n" if $debug;
    $running = 0;
}


