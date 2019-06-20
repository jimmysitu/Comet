#ifndef __BASIC_SIMULATOR_H__
#define __BASIC_SIMULATOR_H__

#include <ca_int.h>
#include <map>
#include <vector>
#include "simulator.h"

class BasicSimulator : public Simulator 
{
    unsigned heapAddress;
    std::map<ca_uint<32>, ca_uint<8> > imemMap;
    std::map<ca_uint<32>, ca_uint<8> > dmemMap;
    
    ca_uint<32> *im, *dm;

    FILE* inputFile;
    FILE* outputFile;
    FILE* traceFile;
public:
    BasicSimulator(const char* binaryFile, std::vector<std::string>, const char* inFile, const char* outFile, const char *tFile ); 
    ~BasicSimulator();

protected:
    void fillMemory();

    void printCycle(); 
    void printStat(){};
    void extend(){};
    void solveSyscall();

    void insertInstructionMemoryMap(ca_uint<32> addr, ca_uint<8> value);
    void insertDataMemoryMap(ca_uint<32> addr, ca_uint<8> value);

    ca_int<32> doRead(ca_uint<32> file, ca_uint<32> bufferAddr, ca_uint<32> size);
    ca_int<32> doWrite(ca_uint<32> file, ca_uint<32> bufferAddr, ca_uint<32> size);
    ca_int<32> doOpen(ca_uint<32> name, ca_uint<32> flags, ca_uint<32> mode);
    ca_int<32> doOpenat(ca_uint<32> dir, ca_uint<32> name, ca_uint<32> flags, ca_uint<32> mode);
    ca_int<32> doLseek(ca_uint<32> file, ca_uint<32> ptr, ca_uint<32> dir);
    ca_int<32> doClose(ca_uint<32> file);
    ca_int<32> doStat(ca_uint<32> filename, ca_uint<32> ptr);
    ca_int<32> doSbrk(ca_uint<32> value);
    ca_int<32> doGettimeofday(ca_uint<32> timeValPtr);
    ca_int<32> doUnlink(ca_uint<32> path);
    ca_int<32> doFstat(ca_uint<32> file, ca_uint<32> stataddr);
    
    void stb(ca_uint<32> addr, ca_int<8> value);
    void sth(ca_uint<32> addr, ca_int<16> value);
    void stw(ca_uint<32> addr, ca_int<32> value);
    void std(ca_uint<32> addr, ca_int<64> value);

    ca_int<8> ldb(ca_uint<32> addr);
    ca_int<16> ldh(ca_uint<32> addr);
    ca_int<32> ldw(ca_uint<32> addr);
    ca_int<32> ldd(ca_uint<32> addr);
};

#endif // __BASIC_SIMULATOR_H__
