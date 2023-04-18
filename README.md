# CSRAM qemu plugin

This is a qemu plugin which emulate C-SRAM (Computing SRAM) functionnal
behaviour and allow to evaluate full application performance.

It contains instructions count per categories (Memory access
instruction, scalar instructions, vector instructions) for RISCV
and RISCV with a CSRAM compagnon accelerator

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

Qemu can use a plugin with two flavors : 

* With a `-plugin ./libCxRAM-qemu-plugin.so` flag
* With the `QEMU_PLUGIN` shell environment variable containing the
  full file name of the plugin. The later is used in the HybroGen
  environment. 

## Get Report, Trace and Verbose 

### Report
Using the environment variable `QEMU_CXRAM_REPORT` will generate a performance report `perf_report.csv` with nb ops, latancy and energy used by the system.

Activate this functionality with `export QEMU_CXRAM_REPORT="yes"` and desactivate it with `unset QEMU_CXRAM_REPORT`

**/!\\ This model is completely artificial and does not reflect the performance of any C-SRAM circuit**

### Trace 
Using the environment variable `QEMU_CXRAM_TRACE` will show all executed C-SRAM instructions and the data in memory before and after the execution of instructions: 

```shell
in1:  1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1 
in2: 42  42  42  42  42  42  42  42  42  42  42  42  42  42  42  42 
res:  0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0 
Decode CxRAM operation _cm_add8_mmm
res: 43  43  43  43  43  43  43  43  43  43  43  43  43  43  43  43 
```

Activate this functionality with `export QEMU_CXRAM_TRACE="yes"` and desactivate it with `unset QEMU_CXRAM_TRACE`

### Verbose 
Using the environment variable `QEMU_CXRAM_VERBOSE` will show all plugin funcion call. 

Activate this functionality with `export QEMU_CXRAM_TRACE="yes"` and desactivate with `unset QEMU_CXRAM_TRACE`

## Authors

* Thaddée BRICOUT
* Kevin MAMBU
* Henri-Pierre CHARLES
