XLEN?=32
PATHTOCC=~/Thesis/toolchains/rv32im/bin
CC=$(PATHTOCC)/riscv$(XLEN)-unknown-elf-gcc
CCDUMP=riscv$(XLEN)-unknown-elf-objdump
CFLAGS=-Wno-overflow
BUILDDIR=build
RESDIR=res
TYPES = schar uchar short ushort int uint int64 uint64 float
size = 4 8 16 32

all: dupmatmul.riscv$(XLEN)

dupmatmul.riscv$(XLEN): dupmatmul.c matmul.h
	mkdir -p $(BUILDDIR) $(RESDIR)
	$(foreach size, $(size), $(foreach type, $(TYPES), $(CC) dupmatmul.c $(CFLAGS) -o $(BUILDDIR)/matmul_$(type)_$(size).riscv$(XLEN) -DSIZE=$(size) -DTYPE=$(type);))
	$(foreach size, $(size), $(foreach type, $(TYPES), gcc dupmatmul.c $(CFLAGS) -o $(BUILDDIR)/matmul_$(type)_$(size).x86 -DSIZE=$(size) -DTYPE=$(type);))
	$(CC) dupmatmul.c -o $(BUILDDIR)/matmul.riscv$(XLEN) -D__HLS__ -Wl,-Ttext,4
	$(CC) dupmatmul.c -S -o matmul.riscv$(XLEN).s -D__HLS__ -Wl,-Ttext,4
	$(foreach size, $(size), $(foreach type, $(TYPES), $(BUILDDIR)/matmul_$(type)_$(size).x86 > $(RESDIR)/matmul_$(type)_$(size);))
	$(foreach size, $(size), $(foreach type, $(TYPES), $(CCDUMP) -D $(BUILDDIR)/matmul_$(type)_$(size).riscv$(XLEN) > $(BUILDDIR)/matmul_$(type)_$(size).dump;))

clean:
	rm -rf $(BUILDDIR) $(RESDIR) *.riscv32 *.riscv64 *.x86

.PHONY: all clean
