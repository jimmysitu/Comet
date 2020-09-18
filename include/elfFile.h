#ifndef __ELFFILE
#define __ELFFILE

#include <memory>
#include <string>
#include <vector>

#include "elf.h"

#define DEBUG 0

class ElfSection;
struct ElfSymbol;

class ElfFile {
public:
  Elf32_Ehdr fileHeader32;
  Elf64_Ehdr fileHeader64;

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

  const std::string getName();

  template<typename T>
  std::vector<T> getSectionCode(){
    std::vector<T> content(this->size / sizeof(T));
    std::fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
    std::fread(&content[0], sizeof(T), content.size(), this->containingElfFile->elfFile);
    return content;
  }

  ElfSection(ElfFile* elfFile, const Elf32_Shdr header);
  ElfSection(ElfFile* elfFile, const Elf64_Shdr header);
};

struct ElfSymbol {
  unsigned int name;
  unsigned int type;
  unsigned int offset;
  unsigned int size;
  unsigned int section;
  unsigned int value;

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
  unsigned int res = std::fseek(elfFile, start, SEEK_SET);
  if (res != 0){
    fprintf(stderr, "Error while moving to the beginning of section table\n");
    exit(-1);
  }

  std::vector<ElfSectHeader> localSectionTable(tableSize);
  res = std::fread(&localSectionTable[0], sizeof(ElfSectHeader), tableSize, this->elfFile);
  if (res != tableSize){
    fprintf(stderr, "Error while reading the section table ! (section size is %lu "
           "while we only read %u entries)\n", tableSize, res);
    exit(-1);
  }

  this->sectionTable.reserve(tableSize);
  for (const auto& sectionHeader : localSectionTable)
    this->sectionTable.push_back(std::unique_ptr<ElfSection>(new ElfSection(this, sectionHeader)));
}

template<typename FileHeaderT, typename ElfSecT, typename ElfSymT>
void ElfFile::readElfFile(FileHeaderT *fileHeader){
    fread(fileHeader, sizeof(FileHeaderT), 1, elfFile);
    const size_t start          = fileHeader->e_shoff;
    const size_t tableSize      = fileHeader->e_shnum;
    const size_t entrySize      = fileHeader->e_shentsize;
    const size_t nameTableIndex = fileHeader->e_shstrndx;

    if (DEBUG){
      printf("Program table is at %x and contains %u entries of %u bytes\n", 
              fileHeader->e_phoff, fileHeader->e_phnum, fileHeader->e_phentsize);
      printf("Section table is at %lu and contains %lu entries of %lu bytes\n", start, tableSize, entrySize);
    }

    this->fillSectionTable<ElfSecT>(start, tableSize, entrySize);
    this->fillNameTable(nameTableIndex);
    this->readSymbolTable<ElfSymT>();
}

#endif
