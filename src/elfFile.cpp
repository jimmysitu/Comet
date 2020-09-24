#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include "elfFile.h"


void checkElf(const std::vector<uint8_t> &content){
  if (!std::equal(std::begin(ELF_MAGIC), std::end(ELF_MAGIC), content.begin())) {
      fprintf(stderr, "Error: Not a valid ELF file\n"); 
      exit(-1);
  }
  if (content[EI_CLASS] != ELFCLASS32) {
    fprintf(stderr, "Error reading ELF file header: unkwnonw EI_CLASS\n");
    exit(-1);
  }
}

ElfSection ElfFile::getSymbolNameSection(){
  auto it = std::find_if(sectionTable.begin(), sectionTable.end(), 
                         [](const ElfSection &s){return s.name == ".strtab";});
  return *it; 
}

void ElfFile::fillNameTable(){ 
  const auto nameTableIndex = read_half(content, E_SHSTRNDX); 
  const auto nameTableOffset = sectionTable[nameTableIndex].offset;
  
  for(auto &section : sectionTable){
    const char *nameStr = (const char*)&content[nameTableOffset + section.nameIndex];
    section.name = std::string(nameStr);
  }
}

ElfFile::ElfFile(const char* pathToElfFile)
{
  elfFile.open(pathToElfFile, std::ios::binary);

  if (!elfFile) {
    fprintf(stderr, "Error cannot open file %s\n", pathToElfFile);
    exit(-1);
  }

  elfFile.seekg(0, elfFile.end);
  const auto fileSize = elfFile.tellg();
  content.reserve(fileSize);
  elfFile.seekg(0, elfFile.beg);
  elfFile.read((char*)content.data(), fileSize);

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
