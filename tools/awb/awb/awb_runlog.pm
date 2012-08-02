package awb_runlog;

use strict;
use warnings;
use QtCore4;
use QtGui4;
use QtCore4::isa qw( Qt::Dialog );

use QtCore4::slots
    Kill_clicked => [],
    Ok_clicked  => [],
    receivedLineStdout => [],  
    receivedLineStderr => [],
    receivedProcessExited => ['int', 'QProcess::ExitStatus'];

sub NEW 
{
    my ( $class, $parent ) = @_;
    $class->SUPER::NEW($parent);
    this->{ui} = Ui_Awb_runlog->setupUi(this);
}

sub init
{
    our $debug = 1;

    print "Init\n" if $debug;
}

sub run
{
    my $command = shift;
    my $callback_function = shift;
    my $callback_arg = shift;
    my $ui = this->{ui};

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

    $ui->log1()->append("$command\n");

    $ui->ok()->setEnabled(0);
    $ui->kill()->setEnabled(1);

    # Create a QProcess to run the command

    this->{logprocess} = Qt::Process(this);

    # Parse the command line for QProcess
    #   TBD: Need to parse lines with newline (\n) in them

    #my @args = ("/bin/sh", "-c", $command);
    my @args = ("-c", $command);

    #if ($debug) {
        print "----\n";
        print "Final Command = <$command>\n";
        print join("\n", @args);
        print "\n----\n\n";
    #}


    #this->{logprocess}->setArguments(\@args);

    #
    # Set working directory
    #
    #my $dir = Qt::Dir();

    #this->{logprocess}->setWorkingDirectory($dir);
    
    #
    # Connect up signals and slots
    #

    Qt::Object::connect(this->{logprocess}, SIGNAL 'readyReadStandardOutput()',
                        this,        SLOT   'receivedLineStdout()');

    Qt::Object::connect(this->{logprocess}, SIGNAL 'readyReadStandardError()',
                        this,        SLOT   'receivedLineStderr()');

    Qt::Object::connect(this->{logprocess}, SIGNAL 'finished(int, QProcess::ExitStatus)',
                        this,        SLOT   'receivedProcessExited(int, QProcess::ExitStatus)');


    #
    # Start up command

    this->{logprocess}->start('/bin/sh', \@args);

    my $status = this->{logprocess}->waitForStarted();

    if (! $status) {
        print STDERR "Process start failed!\n";
        #return 1;
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

    this->show();
    return 0;
}

sub wait
{
    our $debug;

    my $a = $main::app;

    print "Wait starting\n" if $debug;

    while (this->{logprocess}->state() == 2) {
        $a->processEvents();
        sleep 0.1;
    }

    print "Wait finished\n" if $debug;

    return this->{logprocess}->exitStatus();
}

sub receivedLineStdout
{
    our $debug;
    my $ui = this->{ui};

    my $line = this->{logprocess}->readAllStandardOutput()->constData();

    $ui->log1()->append("$line");
}

sub receivedLineStderr
{
    our $debug;
    my $ui = this->{ui};

    my $line = this->{logprocess}->readAllStandardError()->constData;

    $ui->log1()->append("$line");
}

sub receivedProcessExited
{
    our $debug;
    my $ui = this->{ui};

    this->{exitcode} = shift;
    this->{runstatus} = shift;

    if (! this->{killed}) {
        #
        # TBD: This should be handled with a callback or postprint argument
        #      not with a command specific sequence in the generic code
        #

        if (this->{command} =~ /dox/) {
            $ui->log1()->append("\n");
            $ui->log1()->append("Refresh the web browser to view the dox output\n");
            $ui->log1()->append("\n");
        }

        $ui->log1()->append("\n");
        $ui->log1()->append("*******DONE******\n");
        $ui->log1()->append("\n");
    }

    $ui->ok()->setEnabled(1);
    $ui->kill()->setEnabled(0);

}




sub Kill_clicked
{
    my $process = this->{logprocess};
    my $pid     = $process->pid();
    our $debug;
    my $ui = this->{ui};

    return if this->{killed};

    print "Kill!\n" if $debug;

    this->{killed} = 1;

#    $process->kill();

    system("kill $pid");

    $ui->ok()->setEnabled(1);
    $ui->kill()->setEnabled(0);

    $ui->log1()->append("\n");
    $ui->log1()->append("*******ABORTED******\n");
    $ui->log1()->append("\n");

}


sub Ok_clicked
{
    our %runlist;
    our $debug;
    my $ui = this->{ui};

    print "Ok!\n" if $debug;

    $ui->ok()->setEnabled(0);
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


