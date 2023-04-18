CFLAGS = -I . -I ./qemu-5.2.0/include/ -fPIC -Wall -Wno-tautological-compare

TARGET = libCxRAM-qemu-plugin.so
QEMUINSTALLDIR = /opt/H2/cxram-linux/libexec/qemu/

all: ${TARGET}

install: all
	mkdir -p ${QEMUINSTALLDIR}
	install ${TARGET} ${QEMUINSTALLDIR}

libCxRAM-qemu-plugin.so: libCxRAM-qemu-plugin.c cxram-vm.c 
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(CFLAGS)

demo:
	riscv32-unknown-linux-gcc -o CxRAM-SimpleAdd CxRAM-SimpleAdd.c -DH2_DEBUG
	qemu-riscv32 -plugin ./libCxRAM-qemu-plugin.so CxRAM-SimpleAdd 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16

getQemuSrc:
	wget https://download.qemu.org/qemu-5.2.0.tar.xz
	tar xf qemu-5.2.0.tar.xz

clean:
	rm -f test_vm test_vm.o
	rm -f CxRAM-SimpleAdd CxRAM-SimpleAdd.o
	rm -rf ${TARGET}
