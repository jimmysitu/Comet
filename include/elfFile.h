#ifndef __ELFFILE
#define __ELFFILE

#include <memory>
#include <string>
#include <vector>

#include "elf.h"

#define DEBUG 0
static bool needToFixEndianness;
static uint16_t SWAP_2(const uint16_t x){ return (((x&0xff) << 8) | (x >> 8)); }
static uint32_t SWAP_4(const uint32_t x){ return ((x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | (x >> 24)); }
static uint16_t FIX_SHORT(const uint16_t x) { return needToFixEndianness ? SWAP_2(x) : x; }
static uint32_t FIX_INT(const uint32_t x){ return needToFixEndianness ? SWAP_4(x) : x; }

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
  void fillSectionTable(const size_t start, const size_t tableSize, const size_t entrySize);

  void fillNameTable(const size_t nameTableIndex);

  template<typename FileHeaderT, typename ElfSecT, typename ElfSymT>
  void readElfFile(FileHeaderT *fileHeader);
};

class ElfSection {
public:
  ElfFile* containingElfFile;

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
  ElfSection(ElfFile* elfFile, const Elf32_Shdr header);
  ElfSection(ElfFile* elfFile, const Elf64_Shdr header);
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
  ElfSymbol(const Elf32_Sym);
  ElfSymbol(const Elf64_Sym);
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
void ElfFile::fillSectionTable(const size_t start, const size_t tableSize, const size_t entrySize){
  unsigned int res = fseek(elfFile, start, SEEK_SET);
  if (res != 0){
    printf("Error while moving to the beginning of section table\n");
    exit(-1);
  }

  std::vector<ElfSectHeader> localSectionTable(tableSize);// * entrySize);
  res = fread(&localSectionTable[0], sizeof(ElfSectHeader), tableSize, this->elfFile);
  if (res != tableSize){
    printf("Error while reading the section table ! (section size is %lu "
           "while we only read %u entries)\n",
           tableSize, res);
    exit(-1);
  }

  this->sectionTable.reserve(tableSize);
  for (const auto& sectionHeader : localSectionTable)
    this->sectionTable.push_back(std::unique_ptr<ElfSection>(new ElfSection(this, sectionHeader)));
}

template<typename FileHeaderT, typename ElfSecT, typename ElfSymT>
void ElfFile::readElfFile(FileHeaderT *fileHeader){
    fread(fileHeader, sizeof(FileHeaderT), 1, elfFile);
    size_t start          = FIX_INT(fileHeader->e_shoff);
    size_t tableSize      = FIX_SHORT(fileHeader->e_shnum);
    size_t entrySize      = FIX_SHORT(fileHeader->e_shentsize);
    size_t nameTableIndex = FIX_SHORT(fileHeader->e_shstrndx);

    if (DEBUG){
      printf("Program table is at %x and contains %u entries of %u bytes\n", FIX_INT(fileHeader->e_phoff),
             FIX_SHORT(fileHeader->e_phnum), FIX_SHORT(fileHeader->e_phentsize));
      printf("Section table is at %lu and contains %lu entries of %lu bytes\n", start, tableSize, entrySize);
    }

    this->fillSectionTable<ElfSecT>(start, tableSize, entrySize);
    this->fillNameTable(nameTableIndex);
    this->readSymbolTable<ElfSymT>();
}

#endif
