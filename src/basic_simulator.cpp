#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "riscvISA.h"
#include "basic_simulator.h"
#include "core.h"
#include "elfFile.h"

BasicSimulator::BasicSimulator(const char* binaryFile, std::vector<std::string> args, const char* inFile,
                               const char* outFile, const char* tFile)
{

  char* coreAsChar = (char*)&core;
  memset(coreAsChar, 0, sizeof(Core));

  im = new HLS_UINT(32)[DRAM_SIZE >> 2];
  dm = new HLS_UINT(32)[DRAM_SIZE >> 2];

#ifdef USE_CACHE
  core.im = new CacheMemory<4, 16, 64>(new SimpleMemory<4>(im), false);
  core.dm = new CacheMemory<4, 16, 64>(new SimpleMemory<4>(dm), false);
#else
  core.im = new SimpleMemory<4>(im);
  core.dm = new SimpleMemory<4>(dm);
#endif

  heapAddress = 0;

  inputFile  = stdin;
  outputFile = stdout;
  traceFile  = stderr;

  if (inFile)
    inputFile = fopen(inFile, "rb");
  if (outFile)
    outputFile = fopen(outFile, "wb");
  if (tFile)
    traceFile = fopen(tFile, "wb");

  //****************************************************************************
  // Populate memory using ELF file
  ElfFile elfFile(binaryFile);

  for(auto const &section : *elfFile.sectionTable){
    if(section->address != 0 && section->getName() != ".text"){
      unsigned char* sectionContent = section->getSectionCode();
      for (unsigned byteNumber = 0; byteNumber < section->size; byteNumber++)
        this->stb(section->address + byteNumber, sectionContent[byteNumber]);

      // We update the size of the heap
      if (section->address + section->size > heapAddress)
        heapAddress = section->address + section->size;

      free(sectionContent);
    }
    if (section->getName() == ".text") {
      unsigned char* sectionContent = section->getSectionCode();
      for (unsigned int byteNumber = 0; byteNumber < section->size; byteNumber++) {
        // Write the instruction byte in Instruction Memory using Little Endian
        im[(section->address + byteNumber) / 4].SET_SLC(((section->address + byteNumber) % 4) * 8,
                                                           HLS_UINT(8)(sectionContent[byteNumber]));
      }
      free(sectionContent);
    }
  }

  //****************************************************************************
  // Looking for start symbol
  unsigned char* sectionContent = elfFile.sectionTable->at(elfFile.indexOfSymbolNameSection)->getSectionCode();
  for (auto const &symbol : *elfFile.symbols){
    const char* name = (const char*)&(sectionContent[symbol->name]);
    if (strcmp(name, "_start") == 0)
        core.pc = symbol->offset;
  }
  free(sectionContent);

  //****************************************************************************
  // Adding command line arguments on the stack
  unsigned int argc = args.size();

  // TODO: stw fills the cache, maybe here we should only fill the main memory. (Davide)
  //this->stw(STACK_INIT, argc);
  dm[STACK_INIT >> 2] = argc;

  HLS_INT(32) currentPlaceStrings = STACK_INIT + 4 + 4 * argc;
  for (unsigned oneArg = 0; oneArg < argc; oneArg++) {
    this->stw(STACK_INIT + 4 * oneArg + 4, currentPlaceStrings);

    int oneCharIndex = 0;
    char oneChar     = args[oneArg][oneCharIndex];
    while (oneChar != 0) {
      this->stb(currentPlaceStrings + oneCharIndex, oneChar);
      oneCharIndex++;
      oneChar = args[oneArg][oneCharIndex];
    }
    this->stb(currentPlaceStrings + oneCharIndex, oneChar);

    oneCharIndex++;
    currentPlaceStrings += oneCharIndex;
  }
  core.regFile[2] = STACK_INIT;
}

BasicSimulator::~BasicSimulator()
{
  if (inputFile)
    fclose(inputFile);
  if (outputFile)
    fclose(outputFile);
  if (traceFile)
    fclose(traceFile);
}

void BasicSimulator::printCycle()
{
  if (!core.stallSignals[0] && 0) {

    if (!core.stallSignals[0] && !core.stallIm && !core.stallDm) {
      printf("Debug trace : %x ", (unsigned int)core.ftoDC.pc);
      std::cout << printDecodedInstrRISCV(core.ftoDC.instruction);

      for (int oneReg = 0; oneReg < 32; oneReg++) {
        printf("%x  ", (unsigned int)core.regFile[oneReg]);
      }
      std::cout << std::endl;
    }
  }
}

// Function for handling memory accesses

void BasicSimulator::stb(HLS_UINT(32) addr, HLS_INT(8) value)
{
  HLS_UINT(32) wordRes = 0;
  bool stall                = true;
  while (stall)
    core.dm->process(addr, BYTE, STORE, value, wordRes, stall);
}

void BasicSimulator::sth(HLS_UINT(32) addr, HLS_INT(16) value)
{
  this->stb(addr + 1, value.SLC(8, 8));
  this->stb(addr + 0, value.SLC(8, 0));
}

void BasicSimulator::stw(HLS_UINT(32) addr, HLS_INT(32) value)
{
  this->stb(addr + 3, value.SLC(8, 24));
  this->stb(addr + 2, value.SLC(8, 16));
  this->stb(addr + 1, value.SLC(8, 8));
  this->stb(addr + 0, value.SLC(8, 0));
}

void BasicSimulator::std(HLS_UINT(32) addr, HLS_INT(64) value)
{
  this->stb(addr + 7, value.SLC(8, 56));
  this->stb(addr + 6, value.SLC(8, 48));
  this->stb(addr + 5, value.SLC(8, 40));
  this->stb(addr + 4, value.SLC(8, 32));
  this->stb(addr + 3, value.SLC(8, 24));
  this->stb(addr + 2, value.SLC(8, 16));
  this->stb(addr + 1, value.SLC(8, 8));
  this->stb(addr + 0, value.SLC(8, 0));
}

HLS_INT(8) BasicSimulator::ldb(HLS_UINT(32) addr)
{
  HLS_INT(8) result;
  result                    = dm[addr >> 2].SLC(8, ((int)addr.SLC(2, 0)) << 3);
  HLS_UINT(32) wordRes = 0;
  bool stall                = true;
  while (stall)
    core.dm->process(addr, BYTE_U, LOAD, 0, wordRes, stall);

  result = wordRes.SLC(8, 0);
  return result;
}

// Little endian version
HLS_INT(16) BasicSimulator::ldh(HLS_UINT(32) addr)
{
  HLS_INT(16) result = 0;
  result.SET_SLC(8, this->ldb(addr + 1));
  result.SET_SLC(0, this->ldb(addr));
  return result;
}

HLS_INT(32) BasicSimulator::ldw(HLS_UINT(32) addr)
{
  HLS_INT(32) result = 0;
  result.SET_SLC(24, this->ldb(addr + 3));
  result.SET_SLC(16, this->ldb(addr + 2));
  result.SET_SLC(8, this->ldb(addr + 1));
  result.SET_SLC(0, this->ldb(addr));
  return result;
}

HLS_INT(32) BasicSimulator::ldd(HLS_UINT(32) addr)
{
  HLS_INT(32) result = 0;
  result.SET_SLC(56, this->ldb(addr + 7));
  result.SET_SLC(48, this->ldb(addr + 6));
  result.SET_SLC(40, this->ldb(addr + 5));
  result.SET_SLC(32, this->ldb(addr + 4));
  result.SET_SLC(24, this->ldb(addr + 3));
  result.SET_SLC(16, this->ldb(addr + 2));
  result.SET_SLC(8, this->ldb(addr + 1));
  result.SET_SLC(0, this->ldb(addr));

  return result;
}

/********************************************************************************************************************
**  Software emulation of system calls.
**
** Currently all system calls are solved in the simulator. The function solveSyscall check the opcode in the
** extoMem pipeline registers and verifies whether it is a syscall or not. If it is, they solve the forwarding,
** and switch to the correct function according to reg[17].
*********************************************************************************************************************/
void BasicSimulator::solveSyscall()
{

  if ((core.extoMem.opCode == RISCV_SYSTEM) && core.extoMem.instruction.SLC(12, 20) == 0 && !core.stallSignals[2] &&
      !core.stallIm && !core.stallDm) {

    HLS_INT(32) syscallId = core.regFile[17];
    HLS_INT(32) arg1      = core.regFile[10];
    HLS_INT(32) arg2      = core.regFile[11];
    HLS_INT(32) arg3      = core.regFile[12];
    HLS_INT(32) arg4      = core.regFile[13];

    if (core.memtoWB.useRd && core.memtoWB.we && !core.stallSignals[3]) {
      if (core.memtoWB.rd == 10)
        arg1 = core.memtoWB.result;
      else if (core.memtoWB.rd == 11)
        arg2 = core.memtoWB.result;
      else if (core.memtoWB.rd == 12)
        arg3 = core.memtoWB.result;
      else if (core.memtoWB.rd == 13)
        arg4 = core.memtoWB.result;
      else if (core.memtoWB.rd == 17)
        syscallId = core.memtoWB.result;
    }

    HLS_INT(32) result = 0;

    switch (syscallId) {
      case SYS_exit:
        exitFlag = 1; // Currently we break on ECALL
        break;
      case SYS_read:
        result = this->doRead(arg1, arg2, arg3);
        break;
      case SYS_write:
        result = this->doWrite(arg1, arg2, arg3);
        break;
      case SYS_brk:
        result = this->doSbrk(arg1);
        break;
      case SYS_open:
        result = this->doOpen(arg1, arg2, arg3);
        break;
      case SYS_openat:
        result = this->doOpenat(arg1, arg2, arg3, arg4);
        break;
      case SYS_lseek:
        result = this->doLseek(arg1, arg2, arg3);
        break;
      case SYS_close:
        result = this->doClose(arg1);
        break;
      case SYS_fstat:
        result = this->doFstat(arg1, arg2);
        break;
      case SYS_stat:
        result = this->doStat(arg1, arg2);
        break;
      case SYS_gettimeofday:
        result = this->doGettimeofday(arg1);
        break;
      case SYS_unlink:
        result = this->doUnlink(arg1);
        break;
      case SYS_exit_group:
        fprintf(stderr, "Syscall : SYS_exit_group\n");
        exitFlag = 1;
        break;
      case SYS_getpid:
        fprintf(stderr, "Syscall : SYS_getpid\n");
        exitFlag = 1;
        break;
      case SYS_kill:
        fprintf(stderr, "Syscall : SYS_kill\n");
        exitFlag = 1;
        break;
      case SYS_link:
        fprintf(stderr, "Syscall : SYS_link\n");
        exitFlag = 1;
        break;
      case SYS_mkdir:
        fprintf(stderr, "Syscall : SYS_mkdir\n");
        exitFlag = 1;
        break;
      case SYS_chdir:
        fprintf(stderr, "Syscall : SYS_chdir\n");
        exitFlag = 1;
        break;
      case SYS_getcwd:
        fprintf(stderr, "Syscall : SYS_getcwd\n");
        exitFlag = 1;
        break;
      case SYS_lstat:
        fprintf(stderr, "Syscall : SYS_lstat\n");
        exitFlag = 1;
        break;
      case SYS_fstatat:
        fprintf(stderr, "Syscall : SYS_fstatat\n");
        exitFlag = 1;
        break;
      case SYS_access:
        fprintf(stderr, "Syscall : SYS_access\n");
        exitFlag = 1;
        break;
      case SYS_faccessat:
        fprintf(stderr, "Syscall : SYS_faccessat\n");
        exitFlag = 1;
        break;
      case SYS_pread:
        fprintf(stderr, "Syscall : SYS_pread\n");
        exitFlag = 1;
        break;
      case SYS_pwrite:
        fprintf(stderr, "Syscall : SYS_pwrite\n");
        exitFlag = 1;
        break;
      case SYS_uname:
        fprintf(stderr, "Syscall : SYS_uname\n");
        exitFlag = 1;
        break;
      case SYS_getuid:
        fprintf(stderr, "Syscall : SYS_getuid\n");
        exitFlag = 1;
        break;
      case SYS_geteuid:
        fprintf(stderr, "Syscall : SYS_geteuid\n");
        exitFlag = 1;
        break;
      case SYS_getgid:
        fprintf(stderr, "Syscall : SYS_getgid\n");
        exitFlag = 1;
        break;
      case SYS_getegid:
        fprintf(stderr, "Syscall : SYS_getegid\n");
        exitFlag = 1;
        break;
      case SYS_mmap:
        fprintf(stderr, "Syscall : SYS_mmap\n");
        exitFlag = 1;
        break;
      case SYS_munmap:
        fprintf(stderr, "Syscall : SYS_munmap\n");
        exitFlag = 1;
        break;
      case SYS_mremap:
        fprintf(stderr, "Syscall : SYS_mremap\n");
        exitFlag = 1;
        break;
      case SYS_time:
        fprintf(stderr, "Syscall : SYS_time\n");
        exitFlag = 1;
        break;
      case SYS_getmainvars:
        fprintf(stderr, "Syscall : SYS_getmainvars\n");
        exitFlag = 1;
        break;
      case SYS_rt_sigaction:
        fprintf(stderr, "Syscall : SYS_rt_sigaction\n");
        exitFlag = 1;
        break;
      case SYS_writev:
        fprintf(stderr, "Syscall : SYS_writev\n");
        exitFlag = 1;
        break;
      case SYS_times:
        fprintf(stderr, "Syscall : SYS_times\n");
        exitFlag = 1;
        break;
      case SYS_fcntl:
        fprintf(stderr, "Syscall : SYS_fcntl\n");
        exitFlag = 1;
        break;
      case SYS_getdents:
        fprintf(stderr, "Syscall : SYS_getdents\n");
        exitFlag = 1;
        break;
      case SYS_dup:
        fprintf(stderr, "Syscall : SYS_dup\n");
        exitFlag = 1;
        break;

        // Custom syscalls
      case SYS_threadstart:
        result = 0;
        break;
      case SYS_nbcore:
        result = 1;
        break;

      default:
        fprintf(stderr, "Syscall : Unknown system call, %d (%x) with arguments :\n", syscallId.to_int(),
                syscallId.to_int());
        fprintf(stderr, "%d (%x)\n%d (%x)\n%d (%x)\n%d (%x)\n", arg1.to_int(), arg1.to_int(), arg2.to_int(),
                arg2.to_int(), arg3.to_int(), arg3.to_int(), arg4.to_int(), arg4.to_int());
        exitFlag = 1;
        break;
    }

    // We write the result and forward
    core.memtoWB.result = result;
    core.memtoWB.rd     = 10;
    core.memtoWB.useRd  = 1;

    if (core.dctoEx.useRs1 && (core.dctoEx.rs1 == 10))
      core.dctoEx.lhs = result;
    if (core.dctoEx.useRs2 && (core.dctoEx.rs2 == 10))
      core.dctoEx.rhs = result;
    if (core.dctoEx.useRs3 && (core.dctoEx.rs3 == 10))
      core.dctoEx.datac = result;
  }
}

HLS_INT(32) BasicSimulator::doRead(HLS_UINT(32) file, HLS_UINT(32) bufferAddr, HLS_UINT(32) size)
{
  char* localBuffer = new char[size.to_int()];
  HLS_INT(32) result;

  if (file == 0 && inputFile)
    result = read(inputFile->_fileno, localBuffer, size);
  else
    result = read(file, localBuffer, size);

  for (int i(0); i < result; i++) {
    this->stb(bufferAddr + i, localBuffer[i]);
  }

  delete[] localBuffer;
  return result;
}

HLS_INT(32) BasicSimulator::doWrite(HLS_UINT(32) file, HLS_UINT(32) bufferAddr, HLS_UINT(32) size)
{
  char* localBuffer = new char[size.to_int()];

  for (int i = 0; i < size; i++)
    localBuffer[i] = this->ldb(bufferAddr + i);

  HLS_INT(32) result = 0;
  if (file == 1 && outputFile) // 3 is the first available file descriptor
  {
    fflush(stdout);
    result = write(outputFile->_fileno, localBuffer, size);
  } else {
    if (file == 1)
      fflush(stdout);
    else if (file == 2)
      fflush(stderr);
    result = write(file, localBuffer, size);
  }
  delete[] localBuffer;
  return result;
}

HLS_INT(32) BasicSimulator::doFstat(HLS_UINT(32) file, HLS_UINT(32) stataddr)
{
  HLS_INT(32) result = 0;
  struct stat filestat    = {0};

  if (file != 1)
    result = fstat(file, &filestat);

  std(stataddr, filestat.st_dev);               // unsigned long long
  std(stataddr + 8, filestat.st_ino);           // unsigned long long
  stw(stataddr + 16, filestat.st_mode);         // unsigned int
  stw(stataddr + 20, filestat.st_nlink);        // unsigned int
  stw(stataddr + 24, filestat.st_uid);          // unsigned int
  stw(stataddr + 28, filestat.st_gid);          // unsigned int
  std(stataddr + 32, filestat.st_rdev);         // unsigned long long
  std(stataddr + 40, filestat.__pad0);          // unsigned long long
  std(stataddr + 48, filestat.st_size);         // long long
  stw(stataddr + 56, filestat.st_blksize);      // int
  stw(stataddr + 60, filestat.__pad0);          // int
  std(stataddr + 64, filestat.st_blocks);       // long long
  stw(stataddr + 72, filestat.st_atim.tv_sec);  // long
  stw(stataddr + 76, filestat.st_atim.tv_nsec); // long
  stw(stataddr + 80, filestat.st_mtim.tv_sec);  // long
  stw(stataddr + 84, filestat.st_mtim.tv_nsec); // long
  stw(stataddr + 88, filestat.st_ctim.tv_sec);  // long
  stw(stataddr + 92, filestat.st_ctim.tv_nsec); // long
  stw(stataddr + 96, filestat.__pad0);          // long
  stw(stataddr + 100, filestat.__pad0);         // long

  return result;
}

HLS_INT(32) BasicSimulator::doOpen(HLS_UINT(32) path, HLS_UINT(32) flags, HLS_UINT(32) mode)
{
  int oneStringElement = this->ldb(path);
  int index            = 0;
  while (oneStringElement != 0) {
    index++;
    oneStringElement = this->ldb(path + index);
  }

  int pathSize = index + 1;

  char* localPath = new char[pathSize + 1];
  for (int i = 0; i < pathSize; i++)
    localPath[i] = this->ldb(path + i);
  localPath[pathSize] = '\0';

  // convert riscv flags to unix flags
  int riscvflags = flags;
  std::string str;
  if (riscvflags & SYS_O_WRONLY)
    str += "WRONLY, ";
  else if (riscvflags & SYS_O_RDWR)
    str += "RDWR, ";
  else
    str += "RDONLY, ";
  int unixflags = riscvflags & 3; // O_RDONLY, O_WRITE, O_RDWR are the same
  riscvflags ^= unixflags;
  if (riscvflags & SYS_O_APPEND) {
    unixflags |= O_APPEND;
    riscvflags ^= SYS_O_APPEND;
    str += "APPEND, ";
  }
  if (riscvflags & SYS_O_CREAT) {
    unixflags |= O_CREAT;
    riscvflags ^= SYS_O_CREAT;
    str += "CREAT, ";
  }
  if (riscvflags & SYS_O_TRUNC) {
    unixflags |= O_TRUNC;
    riscvflags ^= SYS_O_TRUNC;
    str += "TRUNC, ";
  }
  if (riscvflags & SYS_O_EXCL) {
    unixflags |= O_EXCL;
    riscvflags ^= SYS_O_EXCL;
    str += "EXCL, ";
  }
  if (riscvflags & SYS_O_SYNC) {
    unixflags |= O_SYNC;
    riscvflags ^= SYS_O_SYNC;
    str += "SYNC, ";
  }
  if (riscvflags & SYS_O_NONBLOCK) {
    unixflags |= O_NONBLOCK;
    riscvflags ^= SYS_O_NONBLOCK;
    str += "NONBLOCK, ";
  }
  if (riscvflags & SYS_O_NOCTTY) {
    unixflags |= O_NOCTTY;
    riscvflags ^= SYS_O_NOCTTY;
    str += "NOCTTY";
  }
  int result = open(localPath, unixflags, mode.to_int());

  delete[] localPath;
  return result;
}

HLS_INT(32) BasicSimulator::doOpenat(HLS_UINT(32) dir, HLS_UINT(32) path, HLS_UINT(32) flags,
                                          HLS_UINT(32) mode)
{
  fprintf(stderr, "Syscall : SYS_openat not implemented yet...\n");
  exit(-1);
}

HLS_INT(32) BasicSimulator::doClose(HLS_UINT(32) file)
{
  if (file > 2) // don't close simulator's stdin, stdout & stderr
  {
    return close(file);
  }

  return 0;
}

HLS_INT(32) BasicSimulator::doLseek(HLS_UINT(32) file, HLS_UINT(32) ptr, HLS_UINT(32) dir)
{
  int result = lseek(file, ptr, dir);
  return result;
}

HLS_INT(32) BasicSimulator::doStat(HLS_UINT(32) filename, HLS_UINT(32) stataddr)
{
  int oneStringElement = this->ldb(filename);
  int index            = 0;
  while (oneStringElement != 0) {
    index++;
    oneStringElement = this->ldb(filename + index);
  }

  int pathSize = index + 1;

  char* localPath = new char[pathSize + 1];
  for (int i = 0; i < pathSize; i++)
    localPath[i] = this->ldb(filename + i);
  localPath[pathSize] = '\0';

  struct stat filestat;
  int result = stat(localPath, &filestat);

  std(stataddr, filestat.st_dev);               // unsigned long long
  std(stataddr + 8, filestat.st_ino);           // unsigned long long
  stw(stataddr + 16, filestat.st_mode);         // unsigned int
  stw(stataddr + 20, filestat.st_nlink);        // unsigned int
  stw(stataddr + 24, filestat.st_uid);          // unsigned int
  stw(stataddr + 28, filestat.st_gid);          // unsigned int
  std(stataddr + 32, filestat.st_rdev);         // unsigned long long
  std(stataddr + 40, filestat.__pad0);          // unsigned long long
  std(stataddr + 48, filestat.st_size);         // long long
  stw(stataddr + 56, filestat.st_blksize);      // int
  stw(stataddr + 60, filestat.__pad0);          // int
  std(stataddr + 64, filestat.st_blocks);       // long long
  stw(stataddr + 72, filestat.st_atim.tv_sec);  // long
  stw(stataddr + 76, filestat.st_atim.tv_nsec); // long
  stw(stataddr + 80, filestat.st_mtim.tv_sec);  // long
  stw(stataddr + 84, filestat.st_mtim.tv_nsec); // long
  stw(stataddr + 88, filestat.st_ctim.tv_sec);  // long
  stw(stataddr + 92, filestat.st_ctim.tv_nsec); // long
  stw(stataddr + 96, filestat.__pad0);          // long
  stw(stataddr + 100, filestat.__pad0);         // long

  delete[] localPath;
  return result;
}

HLS_INT(32) BasicSimulator::doSbrk(HLS_UINT(32) value)
{
  HLS_INT(32) result;
  if (value == 0) {
    result = heapAddress;
  } else {
    heapAddress = value;
    result      = value;
  }

  return result;
}

HLS_INT(32) BasicSimulator::doGettimeofday(HLS_UINT(32) timeValPtr)
{
  struct timeval oneTimeVal;
  int result = gettimeofday(&oneTimeVal, NULL);

  this->stw(timeValPtr, oneTimeVal.tv_sec);
  this->stw(timeValPtr + 4, oneTimeVal.tv_usec);

  return result;
}

HLS_INT(32) BasicSimulator::doUnlink(HLS_UINT(32) path)
{
  int oneStringElement = this->ldb(path);
  int index            = 0;
  while (oneStringElement != '\0') {
    index++;
    oneStringElement = this->ldb(path + index);
  }

  int pathSize = index + 1;

  char* localPath = new char[pathSize + 1];
  for (int i = 0; i < pathSize; i++)
    localPath[i] = this->ldb(path + i);
  localPath[pathSize] = '\0';

  int result = unlink(localPath);

  delete[] localPath;
  return result;
}
