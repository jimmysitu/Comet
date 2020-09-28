#ifndef __ELFFILE
#define __ELFFILE

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include "elf.h"

static const uint8_t ELF_MAGIC[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

static const size_t E_SHOFF     = 0x20;
static const size_t E_SHENTSIZE = 0x2E;
static const size_t E_SHNUM     = 0x30;
static const size_t E_SHSTRNDX  = 0x32;

template <typename Data> constexpr uint16_t big_endian2(const Data& x, const size_t loc)
{
  return (x[loc + 1] | (x[loc + 0] << 8));
}

template <typename Data> constexpr uint16_t lit_endian2(const Data& x, const size_t loc)
{
  return (x[loc + 0] | (x[loc + 1] << 8));
}

template <typename Data> constexpr uint32_t big_endian4(const Data& x, const size_t loc)
{
  return (x[loc + 3] | (x[loc + 2] << 8) | (x[loc + 1] << 16) | (x[loc + 0] << 24));
}

template <typename Data> constexpr uint32_t lit_endian4(const Data& x, const size_t loc)
{
  return (x[loc + 0] | (x[loc + 1] << 8) | (x[loc + 2] << 16) | (x[loc + 3] << 24));
}

template <typename Data> constexpr uint32_t read_word(const Data& x, const size_t loc, const bool is_little = true)
{
  return is_little ? lit_endian4(x, loc) : big_endian4(x, loc);
}

template <typename Data> constexpr uint16_t read_half(const Data& x, const size_t loc, const bool is_little = true)
{
  return is_little ? lit_endian2(x, loc) : big_endian2(x, loc);
}

// Function used to lookup Sections or Symbols by name
template <typename T> T find_by_name(const std::vector<T> v, const std::string name)
{
  const auto it = std::find_if(v.begin(), v.end(), [&](const T& s) { return s.name == name; });
  if (it == v.end()) {
    fprintf(stderr, "Error: \"%s\" name not found\n", name.c_str());
    exit(-1);
  }
  return *it;
}

struct ElfSection {
  unsigned int size;
  unsigned int offset;
  unsigned int nameIndex;
  unsigned int address;
  unsigned int type;
  unsigned int info;

  std::string name;

  template <typename ElfShdr> ElfSection(const ElfShdr);
};

struct ElfSymbol {
  unsigned int nameIndex;
  unsigned int type;
  unsigned int offset;
  unsigned int size;
  unsigned int section;
  unsigned int value;

  std::string name;

  template <typename ElfSymT> ElfSymbol(const ElfSymT);
};

class ElfFile {
public:
  std::vector<ElfSection> sectionTable;
  std::vector<ElfSymbol> symbols;
  std::vector<uint8_t> content;

  ElfFile(const char* pathToElfFile);
  ~ElfFile() = default;

private:
  template <typename ElfSymT> void readSymbolTable();
  template <typename ElfShdrT> void fillSectionTable();

  void fillNameTable();
  void fillSymbolsName();
};

template <typename ElfSymT> void ElfFile::readSymbolTable()
{
  for (const auto& section : sectionTable) {
    if (section.type == SHT_SYMTAB) {
      const auto* rawSymbols = reinterpret_cast<ElfSymT*>(&content[section.offset]);
      const auto N           = section.size / sizeof(ElfSymT);
      for (int i = 0; i < N; i++)
        symbols.push_back(ElfSymbol(rawSymbols[i]));
    }
  }
}

template <typename ElfShdrT> void ElfFile::fillSectionTable()
{
  const auto tableOffset  = read_word(content, E_SHOFF);
  const auto tableSize    = read_half(content, E_SHNUM);
  const auto* rawSections = reinterpret_cast<ElfShdrT*>(&content[tableOffset]);

  sectionTable.reserve(tableSize);
  for (int i = 0; i < tableSize; i++)
    sectionTable.push_back(ElfSection(rawSections[i]));
}

template <typename ElfShdrT> ElfSection::ElfSection(const ElfShdrT header)
{
  offset    = (header.sh_offset);
  size      = (header.sh_size);
  nameIndex = (header.sh_name);
  address   = (header.sh_addr);
  type      = (header.sh_type);
  info      = (header.sh_info);
}

template <typename ElfSymT> ElfSymbol::ElfSymbol(const ElfSymT sym)
{
  offset    = sym.st_value;
  type      = ELF32_ST_TYPE(sym.st_info); // TODO: make this generic
  section   = sym.st_shndx;
  size      = sym.st_size;
  nameIndex = sym.st_name;
}

#endif
