
// #ifdef __LINUX_API

#ifndef __ELFFILE
#define __ELFFILE

#include <memory>
#include <string>
#include <vector>

#include "elf.h"

#define DEBUG 0
#define SWAP_2(x) ((((x)&0xff) << 8) | ((unsigned short)(x) >> 8))
#define SWAP_4(x) ((x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | (x >> 24))
#define FIX_SHORT(x) (x) = needToFixEndianness ? SWAP_2(x) : x
#define FIX_INT(x) (x) = needToFixEndianness ? SWAP_4(x) : x

static bool needToFixEndianness;
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
  template<typename T>
  std::vector<T> getSectionCode(){
    std::vector<T> content(this->size / sizeof(T));
    fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
    std::fread(&content[0], sizeof(T), content.size(), this->containingElfFile->elfFile);
    return content;
  }

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
