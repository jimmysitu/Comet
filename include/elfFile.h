
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
private:
  template<typename ElfSymType>
  void readSymbolTable();

  template<typename ElfSectHeader>
  void fillSectionTable(const unsigned long tableSize, const unsigned long entrySize);

  void fillNameTable(unsigned long nameTableIndex);

  template<typename FileHeaderT, typename ElfSecT, typename ElfSymT>
  void readElfFile(FileHeaderT *fileHeader);
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

template<typename ElfSymType>
void ElfFile::readSymbolTable(){
  for(auto &section : this->sectionTable){
    if (section->type == SHT_SYMTAB) {
      std::vector<ElfSymType> symbols = section->getSectionCode<ElfSymType>();
      for (const auto symbol : symbols)
        this->symbols.push_back(std::unique_ptr<ElfSymbol>(new ElfSymbol(symbol)));
    }
  }

  for (unsigned sectionNumber = 0; sectionNumber < this->sectionTable.size(); sectionNumber++) {
    const auto &section = this->sectionTable.at(sectionNumber);
    if (section->getName() == ".strtab") {
      this->indexOfSymbolNameSection = sectionNumber;
      break;
    }
  }
}


template<typename ElfSectHeader>
void ElfFile::fillSectionTable(const unsigned long tableSize, const unsigned long entrySize){
  this->sectionTable.reserve(tableSize);

  ElfSectHeader *localSectionTable = (ElfSectHeader*)malloc(tableSize * entrySize);

  unsigned res = fread(localSectionTable, entrySize, tableSize, this->elfFile);
  if (res != tableSize){
    printf("Error while reading the section table ! (section size is %lu "
           "while we only read %u entries)\n",
           tableSize, res);
    exit(-1);
  }

  for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++)
    this->sectionTable.push_back(std::unique_ptr<ElfSection>(new ElfSection(this, sectionNumber, localSectionTable[sectionNumber])));

  free(localSectionTable);
}

template<typename FileHeaderT, typename ElfSecT, typename ElfSymT>
void ElfFile::readElfFile(FileHeaderT *fileHeader){
    fread(fileHeader, sizeof(FileHeaderT), 1, elfFile);

    if (DEBUG && this->is32Bits)
      printf("Program table is at %x and contains %u entries of %u bytes\n", FIX_INT(this->fileHeader32.e_phoff),
             FIX_SHORT(this->fileHeader32.e_phnum), FIX_SHORT(this->fileHeader32.e_phentsize));

    unsigned long start          = FIX_INT(fileHeader->e_shoff);
    unsigned long tableSize      = FIX_SHORT(fileHeader->e_shnum);
    unsigned long entrySize      = FIX_SHORT(fileHeader->e_shentsize);
    unsigned long nameTableIndex = FIX_SHORT(fileHeader->e_shstrndx);

    if (DEBUG)
      printf("Section table is at %lu and contains %lu entries of %lu bytes\n", start, tableSize, entrySize);

    unsigned int res = fseek(elfFile, start, SEEK_SET);
    if (res != 0){
      printf("Error while moving to the beginning of section table\n");
      exit(-1);
    }

    this->fillSectionTable<ElfSecT>(tableSize, entrySize);
    this->fillNameTable(nameTableIndex);
    this->readSymbolTable<ElfSymT>();
}

#endif
