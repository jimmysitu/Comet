#include <ac_int.h>
#include <elfFile.h>
#include <memoryInterface.h>

char binaryFile[] = "/home/srokicki/matmul.riscv32_hw";

void doCore(bool globalStall, ac_int<1, false>* crashFlag, ac_int<32, false> imData[1 << 24],
            ac_int<32, false> dmData[1 << 24]);

void stb(MemoryInterface<4>& interface, ac_int<32, false> addr, ac_int<8, true> value)
{
  ac_int<32, false> wordRes = 0;
  bool stall                = true;
  while (stall)
    interface.process(addr, BYTE, STORE, value, wordRes, stall);
}

int main(int argc, char** argv)
{

  ac_int<32, false> instr[65536];
  ac_int<32, false> data[65536];

  unsigned int maxAddrInstr = 0;
  unsigned int maxAddrData  = 0;

  SimpleMemory<4> dm = SimpleMemory<4>(data);
  SimpleMemory<4> im = SimpleMemory<4>(instr);

  //****************************************************************************
  // Populate memory using ELF file
  ElfFile elfFile(binaryFile);
  int counter = 0;
  for (unsigned int sectionCounter = 0; sectionCounter < elfFile.sectionTable->size(); sectionCounter++) {
    ElfSection* oneSection = elfFile.sectionTable->at(sectionCounter);
    if (oneSection->address >= 0x10000 && oneSection->address < 0x20000) {
      // If the address is not null we place its content into memory
      unsigned char* sectionContent = oneSection->getSectionCode();
      for (unsigned int byteNumber = 0; byteNumber < oneSection->size; byteNumber++) {
        counter++;
        stb(im, (oneSection->address + byteNumber) & 0xffff, sectionContent[byteNumber]);
      }

      if ((oneSection->address - 0x10000 + oneSection->size) / 4 > maxAddrInstr)
        maxAddrInstr = (oneSection->address - 0x10000 + oneSection->size) / 4;

      free(sectionContent);
    } else if (oneSection->address >= 0x20000 && oneSection->address < 0x30000) {
      // If the address is not null we place its content into memory
      unsigned char* sectionContent = oneSection->getSectionCode();
      for (unsigned int byteNumber = 0; byteNumber < oneSection->size; byteNumber++) {
        counter++;
        stb(dm, (oneSection->address + byteNumber) & 0xffff, sectionContent[byteNumber]);
      }

      if ((oneSection->address - 0x20000 + oneSection->size) / 4 > maxAddrData)
        maxAddrData = (oneSection->address - 0x20000 + oneSection->size) / 4;

      free(sectionContent);
    }
  }

  bool stall                 = false;
  ac_int<1, false> crashFlag = 0;

  // if (1) {
  //   for (int oneWord = 0; oneWord < maxAddrData; oneWord++) {
  //     printf("%08x %08x\n", data[oneWord], oneWord * 4 + 0x44020000);
  //   }
  //   return 0;
  // }

  doCore(stall, &crashFlag, instr, data);
}
