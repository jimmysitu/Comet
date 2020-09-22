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
  
  nameTable.reserve(sectionTable.size());
  for(auto &section : sectionTable){
    unsigned int nameIndex = section.nameIndex;
    section.nameIndex = nameTable.size();
    nameTable.push_back(std::string(&localNameTable[nameIndex]));
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

  if (eident[EI_CLASS] == ELFCLASS32){
    this->readElfFile<Elf32_Ehdr, Elf32_Shdr, Elf32_Sym>(&this->fileHeader32);
  } else if (eident[EI_CLASS] == ELFCLASS64){
    fprintf(stderr, "Error reading ELF file header: unsupported EI_CLASS 64 bit\n");
    exit(-1);
  } else {
    fprintf(stderr, "Error reading ELF file header: unknown EI_CLASS\n");
    exit(-1);
  }
}


/*************************************************************************************************************
 *****************************************  Code for class ElfSection
 ****************************************
 *************************************************************************************************************/

ElfSection::ElfSection(ElfFile* elfFile, const Elf32_Shdr header)
{
  this->containingElfFile = elfFile;
  this->offset            = (header.sh_offset);
  this->size              = (header.sh_size);
  this->nameIndex         = (header.sh_name);
  this->address           = (header.sh_addr);
  this->type              = (header.sh_type);
  this->info              = (header.sh_info);
}

ElfSection::ElfSection(ElfFile* elfFile, const Elf64_Shdr header)
{
  this->containingElfFile = elfFile;
  this->offset            = (header.sh_offset);
  this->size              = (header.sh_size);
  this->nameIndex         = (header.sh_name);
  this->address           = (header.sh_addr);
  this->type              = (header.sh_type);
  this->info              = (header.sh_info);
}

std::string ElfSection::getName() const
{
  return containingElfFile->nameTable.at(this->nameIndex);
}


/*************************************************************************************************************
 ****************************************  Code for class ElfSymbol
 **************************************
 *************************************************************************************************************/

ElfSymbol::ElfSymbol(const Elf32_Sym sym)
{
  this->offset  = (sym.st_value);
  this->type    = (ELF32_ST_TYPE(sym.st_info));
  this->section = (sym.st_shndx);
  this->size    = (sym.st_size);
  this->name    = (sym.st_name);
}

ElfSymbol::ElfSymbol(const Elf64_Sym sym)
{
  this->offset  = (sym.st_value);
  this->type    = (ELF64_ST_TYPE(sym.st_info));
  this->section = (sym.st_shndx);
  this->size    = (sym.st_size);
  this->name    = (sym.st_name);
}
