file ./amc
set args --benchmark config/bm/Traces/GTrace/Test/test-gtrace.cfg --model config/pm/ipf/tang_inorder/tang_inorder.apm --builddir foo --buildopt "OPT=0 DEBUG=1 PAR=1 -j2 CC=gcc3 CXX=g++3" --rundir bar --runopt "-c 100" nuke configure
