#ifndef __ELFFILE
#define __ELFFILE

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "elf.h"

#define DEBUG 0

static const uint8_t ELF_MAGIC[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

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

struct ElfSection;
struct ElfSymbol;

class ElfFile {
public:
  Elf32_Ehdr fileHeader32;

  std::vector<ElfSection> sectionTable;
  std::vector<ElfSymbol> symbols;
  std::vector<uint8_t> content;

  ElfSection getSymbolNameSection();
  ElfFile(const char* pathToElfFile);
  ~ElfFile() = default;

private:
  template<typename ElfSymType>
  void readSymbolTable();

  template<typename ElfSectHeader>
  void fillSectionTable();

  void fillNameTable();
};

struct ElfSection {
  unsigned int size;
  unsigned int offset;
  unsigned int nameIndex;
  unsigned int address;
  unsigned int type;
  unsigned int info;

  std::string name;

  ElfSection(const Elf32_Shdr header);
  ElfSection(const Elf64_Shdr header);
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
      const auto *rawSymbols = reinterpret_cast<ElfSymType*>(&content[section.offset]);
      const auto N = section.size / sizeof(ElfSymType);
      for (int i=0; i < N; i++)
        symbols.push_back(ElfSymbol(rawSymbols[i]));
    }
  }
}

template<typename ElfSectHeader>
void ElfFile::fillSectionTable(){
  const bool endianness = content[EI_DATA] == 1;
  const auto tableOffset = read_word(content, E_SHOFF, endianness); 
  const auto tableSize = read_half(content, E_SHNUM, endianness); 
  
  const auto *rawSections = reinterpret_cast<ElfSectHeader*>(&content[tableOffset]);

  sectionTable.reserve(tableSize);
  for(int i=0; i<tableSize; i++)
    sectionTable.push_back(ElfSection(rawSections[i]));
}

#endif
