# MIPS Processor

This code is the starter code for Assignment 2 of the 2023 spring offering of the Computer Architecture course (2202-COL216).

## Files
- `MIPS_Processor.hpp` contains the implementation of the instruction parser and a non-pipelined implementation of the MIPS processor.
- `sample.cpp` contains a sample usage of the `MIPS_Architecture` struct.
- `sample.asm` contains a sample assembly program that can be run on the processor.

## Usage
After cloning the repository, run the following commands to compile and run the sample program:
```bash
make sure the file you want to test on is in input.asm
$ make compile
$ make run_5stage
$ make run_5stage_bypass
$ make run_79stage
$ make run_79stage_bypass
$ make branch
for testing branch, the input has to be in branchtrace.txt
to test branch trace accuracies, un comment lines in code if you want to see "taken/ not taken" for each branch trace
input file has to be in input.asm
```
