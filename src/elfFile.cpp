#include <cstdio>
#include <stdlib.h>
#include <string>
#include <vector>
#include <memory>

#include "elfFile.h"

/*************************************************************************************************************
 ******************************************  Code for class ElfFile
 ******************************************
 *************************************************************************************************************/

void ElfFile::fillNameTable(const unsigned long nameTableIndex){ 
  auto const &nameTableSection = sectionTable[nameTableIndex];
  std::vector<char> localNameTable = nameTableSection.getSectionCode<char>();
  
  for(auto &section : sectionTable){
    unsigned int nameIndex = section.nameIndex;
    section.name = std::string(&localNameTable[nameIndex]);
  }
}

ElfFile::ElfFile(const char* pathToElfFile)
{
  elfFile.open(pathToElfFile, std::ios::in | std::ios::binary);

  if (!elfFile) {
    fprintf(stderr, "Error cannot open file %s\n", pathToElfFile);
    exit(-1);
  }

  char eident[16];
  elfFile.read(eident, 16);
  
  const char ELF_MAGIC[] = {ELFMAG0, 'E', 'L', 'F'};
  if (!std::equal(std::begin(ELF_MAGIC), std::end(ELF_MAGIC), eident)){
      fprintf(stderr, "Error: Not a valid ELF file\n"); 
      exit(-1);
  }

  //TODO: Check for endianness: if eident[EI_DATA] == 1 -> little

  if (eident[EI_CLASS] != ELFCLASS32){
    fprintf(stderr, "Error reading ELF file header: unkwnonw EI_CLASS\n");
    exit(-1);
  }

  readElfFile<Elf32_Ehdr, Elf32_Shdr, Elf32_Sym>(&fileHeader32);
}


/*************************************************************************************************************
 *****************************************  Code for class ElfSection
 ****************************************
 *************************************************************************************************************/

ElfSection::ElfSection(ElfFile* elfFile, const Elf32_Shdr header)
{
  containingElfFile = elfFile;
  offset            = (header.sh_offset);
  size              = (header.sh_size);
  nameIndex         = (header.sh_name);
  address           = (header.sh_addr);
  type              = (header.sh_type);
  info              = (header.sh_info);
}

ElfSection::ElfSection(ElfFile* elfFile, const Elf64_Shdr header)
{
  containingElfFile = elfFile;
  offset            = (header.sh_offset);
  size              = (header.sh_size);
  nameIndex         = (header.sh_name);
  address           = (header.sh_addr);
  type              = (header.sh_type);
  info              = (header.sh_info);
}

std::string ElfSection::getName() const
{
  return name;
}


/*************************************************************************************************************
 ****************************************  Code for class ElfSymbol
 **************************************
 *************************************************************************************************************/

ElfSymbol::ElfSymbol(const Elf32_Sym sym)
{
  offset  = (sym.st_value);
  type    = (ELF32_ST_TYPE(sym.st_info));
  section = (sym.st_shndx);
  size    = (sym.st_size);
  name    = (sym.st_name);
}

ElfSymbol::ElfSymbol(const Elf64_Sym sym)
{
  offset  = (sym.st_value);
  type    = (ELF64_ST_TYPE(sym.st_info));
  section = (sym.st_shndx);
  size    = (sym.st_size);
  name    = (sym.st_name);
}
