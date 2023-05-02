# CSRAM qemu plugin

This is a QEMU plugin which emulates the Computational SRAM (C-SRAM).

C-SRAM is a domain-specific near-memory computing architecture grouping a subset of energy-optimized operations.
This near-memory computing architecture allows an optimized coupling between SRAM and vector-ALU based on a custom ISA requiring a specific programming.
Thus, the C-SRAM could be used as a programmable vector processing unit driven by the host scalar processor and as a low-latency SRAM (e.g. TCM or scratchpad) the rest of the time.
This privileged mode of use drastically reduces energy consumption by minimizing the data movement (limited number of Load/Store) as well as the code size.


It emulates the functional
behavior of the C-SRAM and allows to evaluate full application performance.


It contains instructions count per categories (memory access
instructions, scalar instructions, vector instructions) for RISCV-based system
and a system with a C-SRAM and RISCV processor

## How to build and install

* `make getQemuSrc all`
* `make demo`
* `make install` (adapt the INSTALLDIR in the makefile)

The demo target should show the result for an addition of two vectors:
```shell
make demo
riscv32-unknown-linux-gcc -o CxRAM-SimpleAdd CxRAM-SimpleAdd.c
qemu-riscv32 -plugin ./libCxRAM-qemu-plugin.so CxRAM-SimpleAdd 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
   001  002  003  004  005  006  007  008  009  010  011  012  013  014  015  016
-  042  042  042  042  042  042  042  042  042  042  042  042  042  042  042  042
=  043  044  045  046  047  048  049  050  051  052  053  054  055  056  057  058
```

## How to use

QEMU can use a plugin with two flavors : 

* With a `-plugin ./libCxRAM-qemu-plugin.so` flag
* With the `QEMU_PLUGIN` shell environment variable containing the
  full file name of the plugin. The later is used in the HybroGen
  environment available here: https://github.com/CEA-LIST/HybroGen. 

## Get Report, Trace and Verbose 

### Report
Using the environment variable `QEMU_CXRAM_REPORT`, it will generate a performance report `perf_report.csv` with nb ops, latancy and energy used by the system.

Activate this functionality with `export QEMU_CXRAM_REPORT="yes"` and desactivate it with `unset QEMU_CXRAM_REPORT`

**/!\\ The performance and energy model available online is completely artificial and does not reflect the performance of any C-SRAM circuit**

### Trace 
Using the environment variable `QEMU_CXRAM_TRACE`, it will show all executed C-SRAM instructions and the data in memory before and after the execution of instructions: 

```shell
in1:  1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1 
in2: 42  42  42  42  42  42  42  42  42  42  42  42  42  42  42  42 
res:  0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0 
Decode CxRAM operation _cm_add8_mmm
res: 43  43  43  43  43  43  43  43  43  43  43  43  43  43  43  43 
```

Activate this functionality with `export QEMU_CXRAM_TRACE="yes"` and desactivate it with `unset QEMU_CXRAM_TRACE`

### Verbose 
Using the environment variable `QEMU_CXRAM_VERBOSE`, it will show all plugin function call. 

Activate this functionality with `export QEMU_CXRAM_TRACE="yes"` and desactivate with `unset QEMU_CXRAM_TRACE`

## Authors

* Thaddée BRICOUT
* Kevin MAMBU
* Henri-Pierre CHARLES
