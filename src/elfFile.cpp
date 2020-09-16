#include <cstdio>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <memory>

#include "elfFile.h"

#define DEBUG 0

#define SWAP_2(x) ((((x)&0xff) << 8) | ((unsigned short)(x) >> 8))
#define SWAP_4(x) ((x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | (x >> 24))
#define FIX_SHORT(x) (x) = needToFixEndianness ? SWAP_2(x) : x
#define FIX_INT(x) (x) = needToFixEndianness ? SWAP_4(x) : x

using namespace std;

bool needToFixEndianness = 0;

/*************************************************************************************************************
 ******************************************  Code for class ElfFile
 ******************************************
 *************************************************************************************************************/

ElfFile::ElfFile(const char* pathToElfFile)
{

  this->elfFile       = fopen(pathToElfFile, "r+");
  if (this->elfFile == NULL) {
    printf("Failing to open %s\n exiting...\n", pathToElfFile);
    exit(-1);
  }

  //*************************************************************************************
  // First step is to read the 16 first bytes to determine the type of the elf
  // file to read.
  char eident[16];

  fread(eident, sizeof(char), 16, elfFile);
  fseek(elfFile, 0, SEEK_SET); // advance file pointer

  if (eident[EI_CLASS] == ELFCLASS32)
    this->is32Bits = 1;
  else if (eident[EI_CLASS] == ELFCLASS64)
    this->is32Bits = 0;
  else {
    fprintf(stderr, "Error while reading ELF file header, cannot handle this "
                    "type of ELF file...\n");
    exit(-1);
  }

  //*************************************************************************************
  // Reading the header of the elf file
  // With different code if it is 32 or 64 bits

  if (this->is32Bits) {
    fread(&this->fileHeader32, sizeof(this->fileHeader32), 1, elfFile);

    if (this->fileHeader32.e_ident[0] == 0x7f)
      needToFixEndianness = false;
    else
      needToFixEndianness = true;
  } else {
    fread(&this->fileHeader64, sizeof(this->fileHeader64), 1, elfFile);

    if (this->fileHeader64.e_ident[0] == 0x7f)
      needToFixEndianness = false;
    else
      needToFixEndianness = true;
  }

  if (DEBUG && this->is32Bits)
    printf("Program table is at %x and contains %u entries of %u bytes\n", FIX_INT(this->fileHeader32.e_phoff),
           FIX_SHORT(this->fileHeader32.e_phnum), FIX_SHORT(this->fileHeader32.e_phentsize));

  //*************************************************************************************
  // Parsing section table

  unsigned long start;
  unsigned long tableSize;
  unsigned long entrySize;

  if (this->is32Bits) {
    start     = FIX_INT(fileHeader32.e_shoff);
    tableSize = FIX_SHORT(fileHeader32.e_shnum);
    entrySize = FIX_SHORT(fileHeader32.e_shentsize);
  } else {
    start     = FIX_INT(fileHeader64.e_shoff);
    tableSize = FIX_SHORT(fileHeader64.e_shnum);
    entrySize = FIX_SHORT(fileHeader64.e_shentsize);
  }

  if (DEBUG)
    printf("Section table is at %lu and contains %lu entries of %lu bytes\n", start, tableSize, entrySize);

  unsigned int res = fseek(elfFile, start, SEEK_SET);
  if (res != 0)
    printf("Error while moving to the beginning of section table\n");

  //*************************************************************************************
  // We create a simple array and read the section table

  this->sectionTable.reserve(tableSize);

  if (this->is32Bits) {
    // TODO: find a cleaner way to read the headers
    Elf32_Shdr* localSectionTable = (Elf32_Shdr*)malloc(tableSize * entrySize);

    res = fread(localSectionTable, entrySize, tableSize, elfFile);
    if (res != tableSize)
      printf("Error while reading the section table ! (section size is %lu "
             "while we only read %u entries)\n",
             tableSize, res);

    // We then copy the section table into it
    for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++)
      this->sectionTable.push_back(std::unique_ptr<ElfSection>(new ElfSection(this, sectionNumber, localSectionTable[sectionNumber])));

    free(localSectionTable);
  } else {
    Elf64_Shdr* localSectionTable = (Elf64_Shdr*)malloc(tableSize * entrySize);

    res = fread(localSectionTable, entrySize, tableSize, elfFile);
    if (res != tableSize)
      printf("Error while reading the section table ! (section size is %lu "
             "while we only read %u entries)\n",
             tableSize, res);

    // We then copy the section table into it
    for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++)
      this->sectionTable.push_back(std::unique_ptr<ElfSection>(new ElfSection(this, sectionNumber, localSectionTable[sectionNumber])));

    free(localSectionTable);
  }

  if (DEBUG)
    for(auto const &sect : this->sectionTable)
      printf("Section is at %x, of size %x\n", sect->offset, sect->size);
    

  //*************************************************************************************
  // Location of the String table containing every name

  unsigned long nameTableIndex;
  if (this->is32Bits)
    nameTableIndex = FIX_SHORT(fileHeader32.e_shstrndx);
  else
    nameTableIndex = FIX_SHORT(fileHeader64.e_shstrndx);

  auto const &nameTableSection = this->sectionTable.at(nameTableIndex);
  unsigned char* localNameTable = nameTableSection->getSectionCode();

  this->nameTable.reserve(this->sectionTable.size());
  for(auto &section : this->sectionTable){
    unsigned int nameIndex = section->nameIndex;
    std::string name(reinterpret_cast<const char*>(&localNameTable[nameIndex]));
    section->nameIndex = this->nameTable.size();
    this->nameTable.push_back(name);
  }
  free(localNameTable);

  //*************************************************************************************
  // Reading the symbol table

  for(auto &section : this->sectionTable){
    if (section->type == SHT_SYMTAB) {
      if (this->is32Bits) {
        Elf32_Sym* symbols = (Elf32_Sym*)section->getSectionCode();
        for (unsigned int oneSymbolIndex = 0; oneSymbolIndex < section->size / sizeof(Elf32_Sym); oneSymbolIndex++)
          this->symbols.push_back(std::unique_ptr<ElfSymbol>(new ElfSymbol(symbols[oneSymbolIndex])));
        free(symbols);
      } else {
        Elf64_Sym* symbols = (Elf64_Sym*)section->getSectionCode();
        for (unsigned int oneSymbolIndex = 0; oneSymbolIndex < section->size / sizeof(Elf64_Sym); oneSymbolIndex++)
          this->symbols.push_back(std::unique_ptr<ElfSymbol>(new ElfSymbol(symbols[oneSymbolIndex])));
        free(symbols);
      }
    }
  }

  for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++) {
    const auto &section = this->sectionTable.at(sectionNumber);
    if (section->getName() == ".strtab") {
      this->indexOfSymbolNameSection = sectionNumber;
      break;
    }
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
