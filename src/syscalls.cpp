/*
 * genericSimulator.cpp
 *
 *  Created on: 25 avr. 2017
 *      Author: simon
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <syscalls.h>

#include "portability.h"


void GenericSimulator::initialize(int argc, char** argv){


    profilingStarts[0] = 0;
    profilingStarts[1] = 0;
    profilingStarts[2] = 0;

    profilingDomains[0] = 0;
    profilingDomains[1] = 0;
    profilingDomains[2] = 0;
    cycle = 0;

    //We initialize registers
    for (int oneReg = 0; oneReg < 32; oneReg++)
        REG[oneReg] = 0;
    REG[2] = 0x70000000;

    /******************************************************
     * Argument passing:
     * In this part of the initialization code, we will copy argc and argv into the simulator stack memory.
     *
     ******************************************************/

    ac_int<32, true> currentPlaceStrings = REG[2] + 8 + 8*argc;

    this->std(REG[2], argc);
    for (int oneArg = 0; oneArg<argc; oneArg++){
        this->std(REG[2] + 8*oneArg + 8, currentPlaceStrings);


        int oneCharIndex = 0;
        char oneChar = argv[oneArg][oneCharIndex];
        while (oneChar != 0){
            this->stb(currentPlaceStrings + oneCharIndex, oneChar);
            oneCharIndex++;
            oneChar = argv[oneArg][oneCharIndex];
        }
        this->stb(currentPlaceStrings + oneCharIndex, oneChar);
        oneCharIndex++;
        currentPlaceStrings += oneCharIndex;

    }

}

void GenericSimulator::stb(ac_int<32, false> addr, ac_int<8, true> value){
    ac_int<32, false> mem = memory[addr];
    formatwrite(addr, 0, mem, value);
    this->memory[addr] = mem;
    fprintf(stderr, "Write @%06x   %02x\n", addr.to_int(), value.to_int());
}

void GenericSimulator::sth(ac_int<32, false> addr, ac_int<16, true> value){
    this->stb(addr+1, value.slc<8>(8));
    this->stb(addr+0, value.slc<8>(0));
}

void GenericSimulator::stw(ac_int<32, false> addr, ac_int<32, true> value){
    this->stb(addr+3, value.slc<8>(24));
    this->stb(addr+2, value.slc<8>(16));
    this->stb(addr+1, value.slc<8>(8));
    this->stb(addr+0, value.slc<8>(0));
}

void GenericSimulator::std(ac_int<32, false> addr, ac_int<32, true> value){
    this->stb(addr+7, value.slc<8>(56));
    this->stb(addr+6, value.slc<8>(48));
    this->stb(addr+5, value.slc<8>(40));
    this->stb(addr+4, value.slc<8>(32));
    this->stb(addr+3, value.slc<8>(24));
    this->stb(addr+2, value.slc<8>(16));
    this->stb(addr+1, value.slc<8>(8));
    this->stb(addr+0, value.slc<8>(0));
}


ac_int<8, true> GenericSimulator::ldb(ac_int<32, false> addr){

    /*
    ac_int<8, true> result = 0;
    if (this->memory.find(addr) != this->memory.end())
        result = this->memory[addr];
    else
        result= 0;*/
    ac_int<8, true> result;
    ac_int<32, false> read = memory[addr];
    formatread(addr, 0, 0, read);
    result = read;
    fprintf(stderr, "Read @%06x    %02x\n", addr.to_int(), result.to_int());
    return result;
}


//Little endian version
ac_int<16, true> GenericSimulator::ldh(ac_int<32, false> addr){

    ac_int<16, true> result = 0;
    result.set_slc(8, this->ldb(addr+1));
    result.set_slc(0, this->ldb(addr));
    return result;
}

ac_int<32, true> GenericSimulator::ldw(ac_int<32, false> addr){

    ac_int<32, true> result = 0;
    result.set_slc(24, this->ldb(addr+3));
    result.set_slc(16, this->ldb(addr+2));
    result.set_slc(8, this->ldb(addr+1));
    result.set_slc(0, this->ldb(addr));
    return result;
}

ac_int<32, true> GenericSimulator::ldd(ac_int<32, false> addr){

    ac_int<32, true> result = 0;
    result.set_slc(56, this->ldb(addr+7));
    result.set_slc(48, this->ldb(addr+6));
    result.set_slc(40, this->ldb(addr+5));
    result.set_slc(32, this->ldb(addr+4));
    result.set_slc(24, this->ldb(addr+3));
    result.set_slc(16, this->ldb(addr+2));
    result.set_slc(8, this->ldb(addr+1));
    result.set_slc(0, this->ldb(addr));

    return result;
}





ac_int<32, true> GenericSimulator::solveSyscall(ac_int<32, true> syscallId, ac_int<32, true> arg1, ac_int<32, true> arg2, ac_int<32, true> arg3, ac_int<32, true> arg4, ac_int<2, false> &sys_status){
    ac_int<32, true> result = 0;
    switch (syscallId)
    {
    case SYS_exit:
        stop = 1;
        sys_status = 1; //Currently we break on ECALL
        fprintf(stderr, "Syscall : SYS_exit\n");
        break;
    case SYS_read:
        fprintf(stderr, "Syscall : SYS_read\n");
        result = this->doRead(arg1, arg2, arg3);
        break;
    case SYS_write:
        fprintf(stderr, "Syscall : SYS_write\n");
        result = this->doWrite(arg1, arg2, arg3);
        break;
    case SYS_brk:
        fprintf(stderr, "Syscall : SYS_brk\n");
        result = this->doSbrk(arg1);
        break;
    case SYS_open:
        fprintf(stderr, "Syscall : SYS_open\n");
        result = this->doOpen(arg1, arg2, arg3);
        break;
    case SYS_openat:
        fprintf(stderr, "Syscall : SYS_openat\n");
        result = this->doOpenat(arg1, arg2, arg3, arg4);
        break;
    case SYS_lseek:
        fprintf(stderr, "Syscall : SYS_lseek\n");
        result = this->doLseek(arg1, arg2, arg3);
        break;
    case SYS_close:
        fprintf(stderr, "Syscall : SYS_close\n");
        result = this->doClose(arg1);
        break;
    case SYS_fstat:
        fprintf(stderr, "Syscall : SYS_fstat\n");
        result = 0;
        break;
    case SYS_stat:
        fprintf(stderr, "Syscall : SYS_stat\n");
        result = this->doStat(arg1, arg2);
        break;
    case SYS_gettimeofday:
        fprintf(stderr, "Syscall : SYS_gettimeofday\n");
        result = this->doGettimeofday(arg1);
        break;
    case SYS_unlink:
        fprintf(stderr, "Syscall : SYS_unlink\n");
        result = this->doUnlink(arg1);
        break;
    case SYS_exit_group:
        fprintf(stderr, "Syscall : SYS_exit_group\n");
        sys_status = 2;
        break;
    case SYS_getpid:
        fprintf(stderr, "Syscall : SYS_getpid\n");
        sys_status = 2;
        break;
    case SYS_kill:
        fprintf(stderr, "Syscall : SYS_kill\n");
        sys_status = 2;
        break;
    case SYS_link:
        fprintf(stderr, "Syscall : SYS_link\n");
        sys_status = 2;
        break;
    case SYS_mkdir:
        fprintf(stderr, "Syscall : SYS_mkdir\n");
        sys_status = 2;
        break;
    case SYS_chdir:
        fprintf(stderr, "Syscall : SYS_chdir\n");
        sys_status = 2;
        break;
    case SYS_getcwd:
        fprintf(stderr, "Syscall : SYS_getcwd\n");
        sys_status = 2;
        break;
    case SYS_lstat:
        fprintf(stderr, "Syscall : SYS_lstat\n");
        sys_status = 2;
        break;
    case SYS_fstatat:
        fprintf(stderr, "Syscall : SYS_fstatat\n");
        sys_status = 2;
        break;
    case SYS_access:
        fprintf(stderr, "Syscall : SYS_access\n");
        sys_status = 2;
        break;
    case SYS_faccessat:
        fprintf(stderr, "Syscall : SYS_faccessat\n");
        sys_status = 2;
        break;
    case SYS_pread:
        fprintf(stderr, "Syscall : SYS_pread\n");
        sys_status = 2;
        break;
    case SYS_pwrite:
        fprintf(stderr, "Syscall : SYS_pwrite\n");
        sys_status = 2;
        break;
    case SYS_uname:
        fprintf(stderr, "Syscall : SYS_uname\n");
        sys_status = 2;
        break;
    case SYS_getuid:
        fprintf(stderr, "Syscall : SYS_getuid\n");
        sys_status = 2;
        break;
    case SYS_geteuid:
        fprintf(stderr, "Syscall : SYS_geteuid\n");
        sys_status = 2;
        break;
    case SYS_getgid:
        fprintf(stderr, "Syscall : SYS_getgid\n");
        sys_status = 2;
        break;
    case SYS_getegid:
        fprintf(stderr, "Syscall : SYS_getegid\n");
        sys_status = 2;
        break;
    case SYS_mmap:
        fprintf(stderr, "Syscall : SYS_mmap\n");
        sys_status = 2;
        break;
    case SYS_munmap:
        fprintf(stderr, "Syscall : SYS_munmap\n");
        sys_status = 2;
        break;
    case SYS_mremap:
        fprintf(stderr, "Syscall : SYS_mremap\n");
        sys_status = 2;
        break;
    case SYS_time:
        fprintf(stderr, "Syscall : SYS_time\n");
        sys_status = 2;
        break;
    case SYS_getmainvars:
        fprintf(stderr, "Syscall : SYS_getmainvars\n");
        sys_status = 2;
        break;
    case SYS_rt_sigaction:
        fprintf(stderr, "Syscall : SYS_rt_sigaction\n");
        sys_status = 2;
        break;
    case SYS_writev:
        fprintf(stderr, "Syscall : SYS_writev\n");
        sys_status = 2;
        break;
    case SYS_times:
        fprintf(stderr, "Syscall : SYS_times\n");
        sys_status = 2;
        break;
    case SYS_fcntl:
        fprintf(stderr, "Syscall : SYS_fcntl\n");
        sys_status = 2;
        break;
    case SYS_getdents:
        fprintf(stderr, "Syscall : SYS_getdents\n");
        sys_status = 2;
        break;
    case SYS_dup:
        fprintf(stderr, "Syscall : SYS_dup\n");
        sys_status = 2;
        break;
    default:
        fprintf(stderr, "Syscall : Unknown system call, %x\n", syscallId.to_int());
        sys_status = 2;
        break;
    }

    return result;
}

ac_int<32, false> GenericSimulator::doRead(ac_int<32, false> file, ac_int<32, false> bufferAddr, ac_int<32, false> size){
    //printf("Doign read on file %x\n", file);

    int localSize = size.slc<32>(0);
    char* localBuffer = (char*) malloc(localSize*sizeof(char));
    ac_int<32, false> result;

    if (file == 0){
        if (nbInStreams == 1)
            result = fread(localBuffer, 1, size, inStreams[0]);
        else
            result = fread(localBuffer, 1, size, stdin);
    }
    else{
        FILE* localFile = this->fileMap[file.slc<16>(0)];
        result = fread(localBuffer, 1, size, localFile);
        if (localFile == 0)
            return -1;
    }

    for (int i=0; i<result; i++){
        this->stb(bufferAddr + i, localBuffer[i]);
    }

    free(localBuffer);
    return result;
}


ac_int<32, false> GenericSimulator::doWrite(ac_int<32, false> file, ac_int<32, false> bufferAddr, ac_int<32, false> size){
    int localSize = size.slc<32>(0);
    char* localBuffer = (char*) malloc(localSize*sizeof(char));
    for (int i=0; i<size; i++)
        localBuffer[i] = this->ldb(bufferAddr + i);

    ac_int<32, false> result = 0;
    if (file < 5){

        int streamNB = (int) file-nbInStreams;
        if (nbOutStreams + nbInStreams > file)
            result = fwrite(localBuffer, 1, size, outStreams[streamNB]);
        else
        {
            result = fwrite(localBuffer, 1, size, stdout);
            fprintf(stderr, "Write %d bytes : %s\nResult : %d\n", size.to_int(), localBuffer, result.to_int());
        }

    }
    else{

        FILE* localFile = this->fileMap[file.slc<16>(0)];
        if (localFile == 0)
            result = -1;
        else
            result = fwrite(localBuffer, 1, size, localFile);
    }

    free(localBuffer);
    return result;
}


ac_int<32, false> GenericSimulator::doOpen(ac_int<32, false> path, ac_int<32, false> flags, ac_int<32, false> mode){
    int oneStringElement = this->ldb(path);
    int index = 0;
    while (oneStringElement != 0){
        index++;
        oneStringElement = this->ldb(path+index);
    }

    int pathSize = index+1;

    char* localPath = (char*) malloc(pathSize*sizeof(char));
    for (int i=0; i<pathSize; i++)
        localPath[i] = this->ldb(path + i);

    const char* localMode;
    if (flags==0)
        localMode = "r";
    else if (flags == 577)
        localMode = "w";
    else if (flags == 1089)
        localMode = "a";
    else if (flags == O_WRONLY|O_CREAT|O_EXCL)
        localMode = "wx";
    else{
        fprintf(stderr, "Trying to open files with unknown flags... %d\n", flags);
        exit(-1);
    }

    FILE* test = fopen(localPath, localMode);
    unsigned long long result = (unsigned long long) test;
    ac_int<32, true> result_ac = result;

    //For some reasons, newlib only store last 16 bits of this pointer, we will then compute a hash and return that.
    //The real pointer is stored here in a hashmap

    ac_int<32, true> returnedResult = 0;
    returnedResult.set_slc(0, result_ac.slc<15>(0) ^ result_ac.slc<15>(16));
    returnedResult[15] = 0;

    this->fileMap[returnedResult.slc<16>(0)] = test;


    free(localPath);
    return returnedResult;

}

ac_int<32, false> GenericSimulator::doOpenat(ac_int<32, false> dir, ac_int<32, false> path, ac_int<32, false> flags, ac_int<32, false> mode){
    fprintf(stderr, "Syscall openat not implemented yet...\n");
    exit(-1);
}

ac_int<32, false> GenericSimulator::doClose(ac_int<32, false> file){
    if (file > 2 ){
        FILE* localFile = this->fileMap[file.slc<16>(0)];
        int result = fclose(localFile);
        return result;
    }
    else
        return 0;
}

ac_int<32, true> GenericSimulator::doLseek(ac_int<32, false> file, ac_int<32, false> ptr, ac_int<32, false> dir){
    if (file>2){
        FILE* localFile = this->fileMap[file.slc<16>(0)];
        if (localFile == 0)
            return -1;
        int result = fseek(localFile, ptr, dir);
        return result;
    }
    else
        return 0;
}

ac_int<32, false> GenericSimulator::doStat(ac_int<32, false> filename, ac_int<32, false> ptr){

    int oneStringElement = this->ldb(filename);
    int index = 0;
    while (oneStringElement != 0){
        index++;
        oneStringElement = this->ldb(filename+index);
    }

    int pathSize = index+1;

    char* localPath = (char*) malloc(pathSize*sizeof(char));
    for (int i=0; i<pathSize; i++)
        localPath[i] = this->ldb(filename + i);

    struct stat fileStat;
    int result = stat(localPath, &fileStat);

    //We copy the result in simulator memory
    for (int oneChar = 0; oneChar<sizeof(struct stat); oneChar++)
        this->stb(ptr+oneChar, ((char*)(&stat))[oneChar]);

    free(localPath);
    return result;
}

ac_int<32, false> GenericSimulator::doSbrk(ac_int<32, false> value){
    fprintf(stderr, "sbrk : %d\n", value.to_int());
    if (value == 0){
        return this->heapAddress;
    }
    else {
        this->heapAddress = value;
        return value;
    }
}

ac_int<32, false> GenericSimulator::doGettimeofday(ac_int<32, false> timeValPtr){
    timeval* oneTimeVal;
    struct timezone* oneTimeZone;
    int result = gettimeofday(oneTimeVal, oneTimeZone);

//    this->std(timeValPtr, oneTimeVal->tv_sec);
//    this->std(timeValPtr+8, oneTimeVal->tv_usec);

    return result;


}

ac_int<32, false> GenericSimulator::doUnlink(ac_int<32, false> path){
    int oneStringElement = this->ldb(path);
    int index = 0;
    while (oneStringElement != 0){
        index++;
        oneStringElement = this->ldb(path+index);
    }

    int pathSize = index+1;

    char* localPath = (char*) malloc(pathSize*sizeof(char));
    for (int i=0; i<pathSize; i++)
        localPath[i] = this->ldb(path + i);


    int result = unlink(localPath);

    free(localPath);

    return result;

}
