# MPC-Accumulator

This repository contains the source code of the paper [1] by researchers of TU Graz and the Austrian Institute of Technology (AIT). A current version of the paper will also be added to this repository. We implemented our work in two MPC Frameworks, MP-SPDZ (https://github.com/data61/MP-SPDZ) and FRESCO (https://github.com/aicis/fresco).

## MP-SPDZ

### Source Code:
The source code of the MPC-Accumulator can be found in `MP-SPDZ/MPC-ACC`. We use the RELIC toolkit (https://github.com/relic-toolkit/relic) for elliptic curve implementations. RELIC is included as submodule in this project. 

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

## FRESCO

### Source Code:
The source code of the MPC-Accumulator can be found in `FRESCO/demos/MPC-ACC`, our ECC extension to SPDZ in `FRESCO/suite/spdz/src/main/java/dk/alexandra/fresco/suite/spdz/ECCExtension`. We use the IAIK ECCelerate library (https://jce.iaik.tugraz.at/sic/Products/Core_Crypto_Toolkits/ECCelerate) for elliptic curve implementations. ECCelerate is NOT included in this repository.

Since we needed to adapt the SPDZ protocol to be able to run elliptic curve operations, we made a lot of changes to the framework. Thus we provide an altered version of FRESCO ourselves, where we omitted parts we do not need and added our changed files. For the elliptic curve calculations, we used the IAIK ECCelerate library [4] and the Java Cryptography Extension (JCE), previously developed by our team at the TU Graz. It is, therefore, necessary to get a license of the ECCerlerate, to run our demonstration. For research purposes, there exists educational license (https://jce.iaik.tugraz.at/sic/Sales/Licences/Educational). There is also an ECCelerate add-on, this includes speedups for the ECC computations. The curves used by the add-on may be patented in certain countries. When it is ensured that these curves may be used, one can integrate the add-on as well.

### Compilation:
The project is developed using maven. When using maven as a build manager, one has to install the ECCelerate .jar file in the local maven repository using the following command:
```
mvn install:install−file −Dfile=<path−to−file> −DgroupId=<group−id> −DartifactId=<artifact−id> −Dversion=<version> −Dpackaging=<packaging>
``` 
* <path-to-file> has to specify the path to the .jar file (the java archive)
* <group-id> has to specify the group, in our case **iaik**.
* <artifact-id> the name under which the artifact will be installed: **iaik_eccelerate** for the ECC library.
* <version> This demonstrator was developed and tested under version **6.0**.
* <packaging> When following the instruction above one has to specify the packaging as **jar**.

Therefore, the following commands are necessary:
```
mvn install:install-file -Dfile=iaik_eccelerate-6.0.jar -DgroupId=iaik -DartifactId=iaik_eccelerate -Dversion=6.0 -Dpackaging=jar
mvn install:install-file -Dfile=iaik_jce_full_signed-5.52.jar -DgroupId=iaik -DartifactId=iaik_jce -Dversion=5.52 -Dpackaging=jar
```

Note that the demonstrator also depends on a JCE library , developed at IAIK. If one wants to
run the program without this dependency, remove it from the `suite/spdz/pom.xml` and make
sure to provide a different JCE framework. The ECCelerate add-on is an optional improvement
to the ECC library. This add-on can be installed as described above. After installing it to the
local maven repository, it has to be added as a dependency is the `suite/spdz/pom.xml` file.
Following the previous steps, one has installed the ECCelerate library in the local maven
repository. Now the build process can be started. Navigate to the `demos/MPC-ACC` directory
to find the `Makefile` for this demonstrator. There are several make targets. To make a full and
clean installation, execute the command `make build`. This command will install FRESCO, the
ECC library, and the demonstrator. Then the assembled jar file will be put in a specific folder
so that the `run` target will find it later.


## Citing our work:
We will add a BibTex entry once the paper is published.


[1] not yet public
