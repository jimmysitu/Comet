XLEN?=32
CCRV=riscv$(XLEN)-unknown-elf-gcc
CCHOST=gcc
CFLAGS=-Wno-overflow
EXEC=dijkstra

DRIVERSDIR=../../drivers

all: $(EXEC).riscv$(XLEN) $(EXEC).riscv$(XLEN)_hw

$(EXEC).riscv$(XLEN): $(EXEC).c
	$(CCRV) $(EXEC).c $(CFLAGS) -o $(EXEC).riscv$(XLEN)
	$(CCHOST) $(EXEC).c -o $(EXEC)
	./$(EXEC) > expectedOutput
	rm -f $(EXEC)

$(EXEC).riscv$(XLEN)_hw: $(EXEC).c $(DRIVERSDIR)/hwSyscalls.c
	$(CCRV) $(EXEC).c $(DRIVERSDIR)/hwSyscalls.c $(CFLAGS) -o $(EXEC).riscv$(XLEN)_hw

clean:
	rm -f *.riscv* expectedOutput
