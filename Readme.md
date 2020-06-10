# MPC-Accumulator

This repository contains the source code of the paper [1] by researchers of TU Graz and the Austrian Institute of Technology (AIT). A current version of the paper will also be added to this repository. We implemented our work in two MPC Frameworks, MP-SPDZ (https://github.com/data61/MP-SPDZ) and FRESCO (https://github.com/aicis/fresco).

## MP-SPDZ

### Source Code:
The source code of the MPC-Accumulator can be found in `MP-SPDZ/MPC-ACC`

### Compilation:
Execute the following commands to compile the source code:
```
git submodule update --init
cd MP-SPDZ

cd relic
mkdir relic-target
cd relic-target
cmake -DFP_PRIME=381 -DFP_WIDTH=6 ..
make
make install DESTDIR=../install
cd ../..

make tldr
make mpcacc
make bench_plain
```

### Execution
Compilation will add the following executables in the `MP-SPDZ` folder:
`mascot-mpcacc-party.x`
`semi-mpcacc-party.x`
`benchmarkmascot-mpcacc-party.x`
`benchmarksemi-mpcacc-party.x`
`bench_plain`

The *-mpcacc-party.x executables offer the same interface:
`LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./relic/install/usr/local/lib/ ./-mpcacc-party.x -p <party-id 0 to N - 1> [-N <number of parties>] [-h <host of party 0>]`

Note: Follow instructions in the benchmarks source code for benchmarks of the different phases of the MPC protocol.

## Citing our work:
We will add a BibTex entry once the paper is published.


[1] not yet public
