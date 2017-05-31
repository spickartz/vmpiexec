# vmpiexec

A startup command for MPI programms on virtualized clusters.

vmpiexec works like other startup mechanisms of common MPI implementations with the execption that it embedds the processes within isolated domains (e.g., virtual machines).
Currently, it relies on the [migration-framework](https://github.com/rwth-os/migration-framework) for the management of domains and on [ParaStation~MPI](https://github.com/parastation)
as MPI distribution.


## Setup
To use vmpiexec you have to:

1. install the [migration-framework](https://github.com/rwth-os/migration-framework)
   on all nodes you plan to use  running parallel applications.
2. start the migration-framework and the used virtualization solution~(e.g., libvirt).
3. compile vmpiexec as every other CMake based project.
6. run the vmpiexec binary with --help and read the options on how to run your
   applications.

## Requirements
The source code in this repository is essential self-contained and all unusual
dependencies are automatically built when compiling vmpiexec. Nonetheless, you
require:

* A compiler supporting C++14.
* CMake (version >= 3.1)
