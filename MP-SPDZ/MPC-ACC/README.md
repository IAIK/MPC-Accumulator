This directory contains the code used for the benchmarks by [Kales
et al.](paper coming soon) `*-mpcacc-party.cpp`
contains the high-level programs while the two phases are implemented
in MPCAcc.h

#### Compilation

- Compile the binaries: `make -j8 mpcacc`

#### Running

The following binaries have been used for the paper:

| Protocol | Binary |
| --- | --- |
| MASCOT | `mascot-mpcacc-party.x` |
| Semi-honest OT | `semi-mpcacc-party.x` |
| Benchmark | `benchmark-mpcacc-party.x` |

All binaries offer the same interface. With MASCOT for example, run
the following:
```
./mascot-mpcacc-party.x -p 0 [-N <number of parties>] [-h <host of party 0>] [<number of prep tuples>]
./mascot-mpcacc-party.x -p 1 [-N <number of parties>] [-h <host of party 0<][<number of prep tuples>]
...
```

The number of parties defaults to 2 for OT-based protocols
