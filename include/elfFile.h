#ifndef __ELFFILE
#define __ELFFILE

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "elf.h"

#define DEBUG 0

static const uint8_t ELF_MAGIC[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

class ElfSection;
struct ElfSymbol;
static const size_t E_SHOFF = 0x20; 
static const size_t E_SHENTSIZE = 0x2E; 
static const size_t E_SHNUM = 0x30; 
static const size_t E_SHSTRNDX = 0x32;

template<typename Data>
constexpr uint16_t big_endian2(const Data &x, const size_t loc){ return (x[loc + 1] | (x[loc + 0] << 8));}

template<typename Data>
constexpr uint16_t lit_endian2(const Data &x, const size_t loc){ return (x[loc + 0] | (x[loc + 1] << 8));}

template<typename Data>
constexpr uint32_t big_endian4(const Data &x, const size_t loc){ return (x[loc + 3] | (x[loc + 2] << 8) | (x[loc + 1] << 16) | (x[loc + 0] << 24));}

template<typename Data>
constexpr uint32_t lit_endian4(const Data &x, const size_t loc){ return (x[loc + 0] | (x[loc + 1] << 8) | (x[loc + 2] << 16) | (x[loc + 3] << 24));}

template<typename Data>
constexpr uint32_t read_word(const Data &x, const size_t loc, const bool is_little=true){ return is_little ? lit_endian4(x, loc) : big_endian4(x, loc); } 

template<typename Data>
constexpr uint16_t read_half(const Data &x, const size_t loc, const bool is_little=true){ return is_little ? lit_endian2(x, loc) : big_endian2(x, loc); } 


class ElfFile {
public:
  std::ifstream elfFile;
  Elf32_Ehdr fileHeader32;

  std::vector<ElfSection> sectionTable;
  std::vector<ElfSymbol> symbols;

  int indexOfSymbolNameSection;

  ElfFile(const char* pathToElfFile);
  ~ElfFile() = default;

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

  std::string name;

  std::string getName() const;

  template<typename T>
  std::vector<T> getSectionCode() const{
    std::vector<T> content(this->size / sizeof(T));
    containingElfFile->elfFile.seekg(this->offset);
    containingElfFile->elfFile.read(reinterpret_cast<char *>(&content[0]), this->size);
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
  for(const auto &section : sectionTable){
    if (section.type == SHT_SYMTAB) {
      for (const auto &symbol : section.getSectionCode<ElfSymType>())
        symbols.push_back(ElfSymbol(symbol));
    }
  }

  for (unsigned sectionNumber = 0; sectionNumber < this->sectionTable.size(); sectionNumber++) {
    const auto &section = sectionTable[sectionNumber];
    if (section.getName() == ".strtab") {
      this->indexOfSymbolNameSection = sectionNumber;
      break;
    }
  }
}

template<typename ElfSectHeader>
void ElfFile::fillSectionTable(const size_t start, const size_t tableSize, const size_t entrySize){
  elfFile.seekg(start);
  if (!elfFile){
    fprintf(stderr, "Error while moving to the beginning of section table\n");
    exit(-1);
  }

  std::vector<ElfSectHeader> localSectionTable(tableSize);
  elfFile.read(reinterpret_cast<char *>(&localSectionTable[0]), sizeof(ElfSectHeader)*tableSize);
  if (!elfFile){
    fprintf(stderr, "Error while reading the section table ! (section size is %lu "
           "while we only read %u entries)\n", tableSize, elfFile.gcount() / sizeof(ElfSectHeader));
    exit(-1);
  }

  sectionTable.reserve(tableSize);
  for (const auto& sectionHeader : localSectionTable)
    sectionTable.push_back(ElfSection(this, sectionHeader));
}

template<typename FileHeaderT, typename ElfSecT, typename ElfSymT>
void ElfFile::readElfFile(FileHeaderT *fileHeader){
    elfFile.seekg(0);
    elfFile.read(reinterpret_cast<char *>(fileHeader), sizeof(FileHeaderT));
    const size_t start          = fileHeader->e_shoff;
    const size_t tableSize      = fileHeader->e_shnum;
    const size_t entrySize      = fileHeader->e_shentsize;
    const size_t nameTableIndex = fileHeader->e_shstrndx;

    if (DEBUG){
      printf("Program table is at %x and contains %u entries of %u bytes\n", 
              fileHeader->e_phoff, fileHeader->e_phnum, fileHeader->e_phentsize);
      printf("Section table is at %lu and contains %lu entries of %lu bytes\n", start, tableSize, entrySize);
    }

    fillSectionTable<ElfSecT>(start, tableSize, entrySize);
    fillNameTable(nameTableIndex);
    readSymbolTable<ElfSymT>();
}

#endif
