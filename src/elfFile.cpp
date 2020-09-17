#include <cstdio>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <string>

#include "elfFile.h"

/*************************************************************************************************************
 ******************************************  Code for class ElfFile
 ******************************************
 *************************************************************************************************************/

void ElfFile::fillNameTable(unsigned long nameTableIndex){ 
  auto const &nameTableSection = this->sectionTable[nameTableIndex];
  std::vector<char> localNameTable = nameTableSection->getSectionCode<char>();
  
   this->nameTable.reserve(this->sectionTable.size());
   for(auto &section : this->sectionTable){
     unsigned int nameIndex = section->nameIndex;
     std::string name(&localNameTable[nameIndex]);
     section->nameIndex = this->nameTable.size();
     this->nameTable.push_back(name);
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

  fread(eident, sizeof(char), 16, elfFile);
  fseek(elfFile, 0, SEEK_SET); // set file pointer to zero

  if (eident[0] == 0x7f)
    needToFixEndianness = false;
  else
    needToFixEndianness = true;

  if (eident[EI_CLASS] == ELFCLASS32){
    this->is32Bits = 1;
    this->readElfFile<Elf32_Ehdr, Elf32_Shdr, Elf32_Sym>(&this->fileHeader32);
  } else if (eident[EI_CLASS] == ELFCLASS64){
    this->is32Bits = 0;
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

ElfSection::ElfSection(ElfFile* elfFile, int id, Elf32_Shdr header)
{
  this->containingElfFile = elfFile;
  this->id                = id;
  this->offset            = FIX_INT(header.sh_offset);
  this->size              = FIX_INT(header.sh_size);
  this->nameIndex         = FIX_INT(header.sh_name);
  this->address           = FIX_INT(header.sh_addr);
  this->type              = FIX_INT(header.sh_type);
  this->info              = FIX_INT(header.sh_info);
}

ElfSection::ElfSection(ElfFile* elfFile, int id, Elf64_Shdr header)
{
  this->containingElfFile = elfFile;
  this->id                = id;
  this->offset            = FIX_INT(header.sh_offset);
  this->size              = FIX_INT(header.sh_size);
  this->nameIndex         = FIX_INT(header.sh_name);
  this->address           = FIX_INT(header.sh_addr);
  this->type              = FIX_INT(header.sh_type);
  this->info              = FIX_INT(header.sh_info);
}

string ElfSection::getName()
{
  return this->containingElfFile->nameTable.at(this->nameIndex);
}

bool ElfSection::isRelSection()
{
  return type == SHT_REL;
}

bool ElfSection::isRelaSection()
{
  return type == SHT_RELA;
}

unsigned char* ElfSection::getSectionCode()
{
  unsigned char* sectionContent = (unsigned char*)malloc(this->size);
  fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
  fread(sectionContent, 1, this->size, this->containingElfFile->elfFile);
  return sectionContent;
}


/*************************************************************************************************************
 ****************************************  Code for class ElfSymbol
 **************************************
 *************************************************************************************************************/

ElfSymbol::ElfSymbol(Elf32_Sym sym)
{
  this->offset  = FIX_INT(sym.st_value);
  this->type    = (ELF32_ST_TYPE(sym.st_info));
  this->section = FIX_SHORT(sym.st_shndx);
  this->size    = FIX_INT(sym.st_size);
  this->name    = FIX_INT(sym.st_name);
}

ElfSymbol::ElfSymbol(Elf64_Sym sym)
{
  this->offset  = FIX_INT(sym.st_value);
  this->type    = (ELF64_ST_TYPE(sym.st_info));
  this->section = FIX_SHORT(sym.st_shndx);
  this->size    = FIX_INT(sym.st_size);
  this->name    = FIX_INT(sym.st_name);
}
