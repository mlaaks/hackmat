# hackmat - HackRF support for Matlab, via libhackrf.
A rather quickly patched mex driver and a system object to interface with it.
Does not currently support transmit, but will when there comes a time I need it.

Compile the mex with: mex hackmat.c -L. -lhackrf -lpthread -R2018a
place the binary and HackRF.m in matlab path.
