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
  auto const &nameTableSection = this->sectionTable[nameTableIndex];
  std::vector<char> localNameTable = nameTableSection->getSectionCode<char>();
  
   this->nameTable.reserve(this->sectionTable.size());
   for(auto &section : this->sectionTable){
     unsigned int nameIndex = section->nameIndex;
     section->nameIndex = this->nameTable.size();
     this->nameTable.push_back(std::string(&localNameTable[nameIndex]));
   }
}

ElfFile::ElfFile(const char* pathToElfFile)
{
  this->elfFile = fopen(pathToElfFile, "r+");

  if (this->elfFile == NULL) {
    printf("Failing to open %s\n exiting...\n", pathToElfFile);
    exit(-1);
  }

  char eident[16];
  std::fread(eident, sizeof(char), 16, elfFile);
  std::fseek(elfFile, 0, SEEK_SET); // set file pointer to zero
  
  const char ELF_MAGIC[] = {ELFMAG0, 'E', 'L', 'F'};
  if (!std::equal(std::begin(ELF_MAGIC), std::end(ELF_MAGIC), eident)){
      fprintf(stderr, "Error: Not a valid ELF file\n"); 
      exit(-1);
  }

  //TODO: Check for endianness: if eident[EI_DATA] == 1 -> little

  if (eident[EI_CLASS] == ELFCLASS32){
    this->readElfFile<Elf32_Ehdr, Elf32_Shdr, Elf32_Sym>(&this->fileHeader32);
  } else if (eident[EI_CLASS] == ELFCLASS64){
    this->readElfFile<Elf64_Ehdr, Elf64_Shdr, Elf64_Sym>(&this->fileHeader64);
  }else {
    fprintf(stderr, "Error while reading ELF file header, cannot handle this "
                    "type of ELF file...\n");
    exit(-1);
  }
}

ElfFile::~ElfFile()
{
  fclose(elfFile);
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

const std::string ElfSection::getName()
{
  return this->containingElfFile->nameTable.at(this->nameIndex);
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
