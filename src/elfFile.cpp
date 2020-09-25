#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>
#include <fstream>

#include "elfFile.h"


void checkElf(const std::vector<uint8_t> &content){
  if(!std::equal(std::begin(ELF_MAGIC), std::end(ELF_MAGIC), content.begin())) {
      fprintf(stderr, "Error: Not a valid ELF file\n"); 
      exit(-1);
  }
  if(content[EI_CLASS] != ELFCLASS32) {
    fprintf(stderr, "Error reading ELF file header: unkwnonw EI_CLASS\n");
    exit(-1);
  }
  if(content[EI_DATA] != 1){
    fprintf(stderr, "Error reading ELF file header: EI_DATA is not little-endian\n");
    exit(-1);
  }
}

ElfSection ElfFile::getSymbolNameSection(){
  return *std::find_if(sectionTable.begin(), sectionTable.end(), 
                       [](const ElfSection &s){return s.name == ".strtab";});
}

void ElfFile::fillNameTable(){ 
  const auto nameTableIndex = read_half(content, E_SHSTRNDX); 
  const auto nameTableOffset = sectionTable[nameTableIndex].offset;
  const char *names = reinterpret_cast<const char*>(&content[nameTableOffset]);
  for(auto &section : sectionTable)
    section.name = std::string(&names[section.nameIndex]);
}

ElfFile::ElfFile(const char* pathToElfFile)
{
  std::ifstream elfFile(pathToElfFile, std::ios::binary);
  if (!elfFile) {
    fprintf(stderr, "Error cannot open file %s\n", pathToElfFile);
    exit(-1);
  }

  content.assign(std::istreambuf_iterator<char>(elfFile), {});
  checkElf(content);
  fillSectionTable<Elf32_Shdr>();
  fillNameTable();
  readSymbolTable<Elf32_Sym>();
}


ElfSection::ElfSection(const Elf32_Shdr header)
{
  offset            = (header.sh_offset);
  size              = (header.sh_size);
  nameIndex         = (header.sh_name);
  address           = (header.sh_addr);
  type              = (header.sh_type);
  info              = (header.sh_info);
}

ElfSection::ElfSection(const Elf64_Shdr header)
{
  offset            = (header.sh_offset);
  size              = (header.sh_size);
  nameIndex         = (header.sh_name);
  address           = (header.sh_addr);
  type              = (header.sh_type);
  info              = (header.sh_info);
}


ElfSymbol::ElfSymbol(const Elf32_Sym sym)
{
  offset  = read_word((char*)&sym.st_value, 0);
  type    = (ELF32_ST_TYPE(sym.st_info));
  section = read_half((char*)&sym.st_shndx, 0);
  size    = read_half((char*)&sym.st_size, 0);
  name    = read_half((char*)&sym.st_name, 0);
}

ElfSymbol::ElfSymbol(const Elf64_Sym sym)
{
  offset  = (sym.st_value);
  type    = (ELF64_ST_TYPE(sym.st_info));
  section = (sym.st_shndx);
  size    = (sym.st_size);
  name    = (sym.st_name);
}
