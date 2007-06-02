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

void RunLog::init()
{
    our $debug = 0;

    print "Init\n" if $debug;
}

void RunLog::run()
{
    my $command = shift;
    my $callback_function = shift;
    my $callback_arg = shift;

    our $debug;

    chomp($command);
    this->{command} = $command;
    this->{callback_function} = $callback_function;
    this->{callback_arg} = $callback_arg;

    this->{logprocess} = undef;
    this->{runstatus} = 1;
    this->{killed} = 0;

    print "Runit <$command>\n" if $debug;

    if (! defined($command) || ! $command) {
        return 1;
    }

    Log1->append("$command\n");

    Ok->setEnabled(0);
    Kill->setEnabled(1);

    # Create a QProcess to run the command

    this->{logprocess} = Qt::Process(this);

    # Parse the command line for QProcess
    #   TBD: Need to parse lines with newline (\n) in them

    my @args = ("/bin/sh", "-c", $command);

    if ($debug) {
        print "----\n";
        print "Final Command = <$command>\n";
        print join("\n", @args);
        print "\n----\n\n";
    }


    this->{logprocess}->setArguments(\@args);

    #
    # Set working directory
    #
    my $dir = Qt::Dir();

    this->{logprocess}->setWorkingDirectory($dir);
    
    #
    # Connect up signals and slots
    #

    Qt::Object::connect(this->{logprocess}, SIGNAL 'readyReadStdout()',
                        this,        SLOT   'receivedLineStdout()');

    Qt::Object::connect(this->{logprocess}, SIGNAL 'readyReadStderr()',
                        this,        SLOT   'receivedLineStderr()');

    Qt::Object::connect(this->{logprocess}, SIGNAL 'processExited()',
                        this,        SLOT   'receivedProcessExited()');


    #
    # Start up command

    my $status = this->{logprocess}->start();

    if (! $status) {
        print STDERR "Process start failed!\n";
        return 1;
    }

    print "Process started\n" if $debug;


    # Force an extra reference count for our modal dialog (hack)

    our $runcount;
    our %runlist;

    if (! defined($runcount)) {
        $runcount = 0;
    }

    this->{runcount}= $runcount++;
    $runlist{this->{runcount}} = this;

    # Display the dialog and return

    show();
    return 0;
}

void awb_runlog::wait()
{
    our $debug;

    my $a = $main::app;

    print "Wait starting\n" if $debug;

    while (this->{logprocess}->isRunning()) {
        $a->processEvents();
        sleep 0.1;
    }

    print "Wait finished\n" if $debug;

    return this->{logprocess}->exitStatus();
}

void RunLog::receivedLineStdout()
{
    our $debug;

    while (this->{logprocess}->canReadLineStdout()) {
        my $line = this->{logprocess}->readLineStdout();

        Log1->append("$line\n");
    }
}

void RunLog::receivedLineStderr()
{
    our $debug;

    while (this->{logprocess}->canReadLineStderr()) {
        my $line = this->{logprocess}->readLineStderr();

        Log1->append("$line\n");
    }
}

void RunLog::receivedProcessExited()
{
    our $debug;

    this->{runstatus} = this->{logprocess}->exitStatus();

    if (! this->{killed}) {
        #
        # TBD: This should be handled with a callback or postprint argument
        #      not with a command specific sequence in the generic code
        #

        if (this->{command} =~ /dox/) {
            Log1->append("\n");
            Log1->append("Refresh the web browser to view the dox output\n");
            Log1->append("\n");
        }

        Log1->append("\n");
        Log1->append("*******DONE******\n");
        Log1->append("\n");
    }

    Ok->setEnabled(1);
    Kill->setEnabled(0);

}




void RunLog::Kill_clicked()
{
    my $process = this->{logprocess};
    my $pid     = $process->processIdentifier();
    our $debug;

    return if this->{killed};

    print "Kill!\n" if $debug;

    this->{killed} = 1;

#    $process->kill();

    system("kill $pid");

    Ok->setEnabled(1);
    Kill->setEnabled(0);

    Log1->append("\n");
    Log1->append("*******ABORTED******\n");
    Log1->append("\n");

}


void RunLog::Ok_clicked()
{
    our %runlist;
    our $debug;

    print "Ok!\n" if $debug;

    Ok->setEnabled(0);
    this->close();

    # Clear out our extra reference out (hack)

    $runlist{this->{runcount}} = undef;

}



#
# TBD:
#
#      Implemented a callback mechanism so that the invoker can
#      find out about the completion of the process instead of waiting...
#
#      Add a save option...
#
#      Make sure that kill is always killing the process
#           If not, use $process->processIdentifier with a raw kill -9
#


