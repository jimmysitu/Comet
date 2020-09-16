
// #ifdef __LINUX_API

#ifndef __ELFFILE
#define __ELFFILE

#include <memory>
#include <string>
#include <vector>

#include "elf.h"

class ElfSection;
class ElfSymbol;

class ElfFile {
public:
  Elf32_Ehdr fileHeader32;
  Elf64_Ehdr fileHeader64;

  int is32Bits;

  std::vector<std::unique_ptr<ElfSection>> sectionTable;
  std::vector<std::string> nameTable;
  std::vector<std::unique_ptr<ElfSymbol>> symbols;

  int indexOfSymbolNameSection;

  ElfFile(const char* pathToElfFile);
  ~ElfFile();

  FILE* elfFile;
};

class ElfSection {
public:
  ElfFile* containingElfFile;
  int id;

  unsigned int size;
  unsigned int offset;
  unsigned int nameIndex;
  unsigned int address;
  unsigned int type;
  unsigned int info;

  // General functions
  std::string getName();

  // Test for special section types
  bool isRelSection();
  bool isRelaSection();

  // Functions to access content
  unsigned char* getSectionCode();

  // Class constructor
  ElfSection(ElfFile* elfFile, int id, Elf32_Shdr header);
  ElfSection(ElfFile* elfFile, int id, Elf64_Shdr header);
};

class ElfSymbol {
public:
  unsigned int name;
  unsigned int type;
  unsigned int offset;
  unsigned int size;
  unsigned int section;
  unsigned int value;

  // Class constructors
  ElfSymbol(Elf32_Sym);
  ElfSymbol(Elf64_Sym);
};

#endif
