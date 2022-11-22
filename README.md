# CROSS-mini

A mini proof-of-concept high-dimensional sparse tensor contraction accelerator in SystemC.

## Building

Just run `make`. Requires `g++` and the `systemc` library.

## Running

Either execute `make test` or manually invoke `./sim`.
If there are any messages logged during execution, they will appear in `report.log`.
Test benches should be written to read in test data from `test_inputs` and write test results into `test_outputs`.
