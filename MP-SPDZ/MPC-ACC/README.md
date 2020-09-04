This directory contains the code used for the benchmarks. `*-mpcacc-party.cpp`
contains the high-level programs while the accumulator is implemented
in MPCAcc.h

#### Compilation

- Compile the binaries: `make -j8 mpcacc`

#### Running

The following binaries have been used for the paper:

| Protocol | Binary |
| --- | --- |
| MASCOT | `mascot-mpcacc-party.x` |
| Semi-honest OT | `semi-mpcacc-party.x` |
| Malicious Shamir | `mal-shamir-mpcacc-party.x` |
| Semi-Honest Shamir | `shamir-mpcacc-party.x` |

Additionally, there are binaries for replicated threshold sharing:

| Protocol | Binary |
| --- | --- |
| Malicious replicated | `mal-rep-mpcacc-party.x` |
| Semi-honest replicated | `rep-mpcacc-party.x` |

All binaries offer the same interface. With MASCOT for example, run
the following:
```
./mascot-mpcacc-party.x -p 0 [-N <number of parties>] [-h <host of party 0>]
./mascot-mpcacc-party.x -p 1 [-N <number of parties>] [-h <host of party 0<]
...
```

