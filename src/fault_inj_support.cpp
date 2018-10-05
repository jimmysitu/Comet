#include "fault_inj_support.h"
#include <cstdio>


//for the signal handlers to be able to output the core status and memory contents
Core* globalCore;   //yuk

/*
 * This is all terrible looking, the width of each field is hardcoded...
 * But eh, it works...
 */

int injectFault_FToDC(Core* core, int bitPosition) {
    int status = 1;

    //check in which field of the register to inject
    //and inject fault in the proper bit
    if(bitPosition < 32) {      //pc
        core->ftoDC.pc ^= (1 << bitPosition);
    }
    else if(bitPosition < 64) {   //instruction
        core->ftoDC.instruction ^= (1 << (bitPosition-32));
    }
    else if(bitPosition < 65) {   //realInstruction
        core->ftoDC.realInstruction = !core->ftoDC.realInstruction;
    }
    else if(bitPosition < 97) {   //nextpc
        core->ftoDC.instruction ^= (1 << (bitPosition-65));
    }
    else {  //wrong bit position, should never happen
        status = 0;
    }
    return status;
}


int injectFault_DCToEX(Core* core, int bitPosition) {
    int status = 1;

    if(bitPosition < 32) {      //pc
        core->dctoEx.pc ^= (1 << bitPosition);
    }
    else if(bitPosition < 39) { //opcode
        core->dctoEx.opCode ^= (1 << (bitPosition)-32);
    }
    else if(bitPosition < 46) { //funct7
        core->dctoEx.funct7 ^= (1 << (bitPosition)-39);
    }
    else if(bitPosition < 49) { //funct3
        core->dctoEx.funct3 ^= (1 << (bitPosition)-46);
    }
    else if(bitPosition < 54) { //rd
        core->dctoEx.rd ^= (1 << (bitPosition)-49);
    }
    else if(bitPosition < 55) { //realInstruction
        core->dctoEx.realInstruction = !core->dctoEx.realInstruction;
    }
    else if(bitPosition < 87) { //lhs
        core->dctoEx.rd ^= (1 << (bitPosition)-55);
    }
    else if(bitPosition < 119) { //rhs
        core->dctoEx.rd ^= (1 << (bitPosition)-87);
    }
    else if(bitPosition < 151) { //datac
        core->dctoEx.rd ^= (1 << (bitPosition)-119);
    }
    else if(bitPosition < 152) { //forward_lhs
        core->dctoEx.forward_lhs = !core->dctoEx.forward_lhs;
    }
    else if(bitPosition < 153) { //forward_rhs
        core->dctoEx.forward_rhs = !core->dctoEx.forward_rhs;
    }
    else if(bitPosition < 154) { //forward_datac
        core->dctoEx.forward_datac = !core->dctoEx.forward_datac;
    }
    else if(bitPosition < 155) { //forward_mem_lhs
        core->dctoEx.forward_mem_lhs = !core->dctoEx.forward_mem_lhs;
    }
    else if(bitPosition < 156) { //forward_mem_rhs
        core->dctoEx.forward_mem_rhs = !core->dctoEx.forward_mem_rhs;
    }
    else if(bitPosition < 157) { //forward_mem_datac
        core->dctoEx.forward_mem_datac = !core->dctoEx.forward_mem_datac;
    }
    else if(bitPosition < 158) { //csr
        core->dctoEx.csr = !core->dctoEx.csr;
    }
    else if(bitPosition < 170) { //CSRid
        core->dctoEx.CSRid ^= (1 << (bitPosition)-158);
    }
    else if(bitPosition < 171) { //external
        core->dctoEx.external = !core->dctoEx.external;
    }
    else if(bitPosition < 174) { //op
        int temp = (int)core->dctoEx.op;
        temp ^= (1 << bitPosition-171);
        core->dctoEx.op = (MultiCycleOperator::MultiCycleOperation)(temp);
    }
    else {  //wrong bit position, should never happen
        status = 0;
    }
    return status;
}


int injectFault_EXToMEM(Core* core, int bitPosition) {
    int status = 1;

    if(bitPosition < 32) {      //pc
        core->extoMem.pc ^= (1 << bitPosition);
    }
    else if(bitPosition < 64) { //result
        core->extoMem.result ^= (1 << (bitPosition-32));
    }
    else if(bitPosition < 69) { //rd
        core->extoMem.rd ^= (1 << (bitPosition-64));
    }
    else if(bitPosition < 76) { //opCode
        core->extoMem.opCode ^= (1 << (bitPosition-69));
    }
    else if(bitPosition < 79) { //funct3
        core->extoMem.funct3 ^= (1 << (bitPosition-76));
    }
    else if(bitPosition < 80) { //realInstruction
        core->extoMem.realInstruction = !core->extoMem.realInstruction;
    }
    else if(bitPosition < 81) { //external
        core->extoMem.external = !core->extoMem.external;
    }
    else if(bitPosition < 82) { //csr
        core->extoMem.csr = !core->extoMem.csr;
    }
    else if(bitPosition < 94) { //CSRid
        core->extoMem.CSRid ^= (1 << (bitPosition-82));
    }
    else if(bitPosition < 126) { //datac
        core->extoMem.datac ^= (1 << (bitPosition-94));
    }
    else {  //wrong bit position, should never happen
        status = 0;
    }
    return status;
}

int injectFault_MEMToWB(Core* core, int bitPosition) {
    int status = 1;

    if(bitPosition < 32) {      //pc
        core->memtoWB.pc ^= (1 << bitPosition);
    }
    else if(bitPosition < 64) { //result
        core->memtoWB.result ^= (1 << (bitPosition-32));
    }
    else if(bitPosition < 69) { //rd
        core->memtoWB.rd ^= (1 << (bitPosition-64));
    }
    else if(bitPosition < 70) { //realInstruction
        core->memtoWB.realInstruction = !core->memtoWB.realInstruction;
    }
    else if(bitPosition < 71) { //csr
        core->memtoWB.csr = !core->memtoWB.csr;
    }
    else if(bitPosition < 83) { //CSRid
        core->memtoWB.CSRid ^= (1 << (bitPosition-71));
    }
    else if(bitPosition < 115) { //CSRid
        core->memtoWB.rescsr ^= (1 << (bitPosition-83));
    }
    else {  //wrong bit position, should never happen
        status = 0;
    }
    return status;
}

int injectFault_RF(Core* core, int registerNumber, int bitPosition) {
    int status = 1;

    if((bitPosition > 0) && (bitPosition < 32) && (registerNumber > 0) && (registerNumber < 32)) {
        core->REG[registerNumber] ^= (1 << bitPosition);
    }
    else {  //wrong register or wrong bit position, should never happen
        status = 0;
    }
    return status;
}

int injectFault_PC(Core* core, int bitPosition) {
    int status = 0;

    if((bitPosition > 0) && (bitPosition < 32)) {
        core->pc ^= (1 << bitPosition);
        status = 1;
    }
    return status;
}

int injectFault_CoreCtrl(Core* core, int bitPosition) {
    int status = 1;

    if(bitPosition < 15) {  //prev_rds
        core->ctrl.prev_rds[(int)(bitPosition/5)] ^= (1 << (bitPosition%5));
    }
    if(bitPosition < 36) {  //prev_opCode
        core->ctrl.prev_opCode[(int)((bitPosition-15)/7)] ^= (1 << ((bitPosition-15)%7));
    }
    if(bitPosition < 38) {  //lock
        core->ctrl.lock ^= (1 << (bitPosition-36));
    }
    if(bitPosition < 39) {  //freeze_fetch
        core->ctrl.freeze_fetch = !core->ctrl.freeze_fetch;
    }
    if(bitPosition < 40) {  //cachelock
        core->ctrl.cachelock = !core->ctrl.cachelock;
    }
    if(bitPosition < 41) {  //init
        core->ctrl.init = !core->ctrl.init;
    }
    if(bitPosition < 42) {  //sleep
        core->ctrl.sleep = !core->ctrl.sleep;
    }
    if(bitPosition < 138) {  //prev_res
        core->ctrl.prev_res[(bitPosition-42)/32] ^= (1 << ((bitPosition-42)%32));
    }
    if(bitPosition < 141) {  //branch
        core->ctrl.branch[(bitPosition-138)] = !core->ctrl.branch[(bitPosition-138)];
    }
    if(bitPosition < 205) { //jump_pc
        core->ctrl.prev_res[(bitPosition-141)/32] ^= (1 << ((bitPosition-141)%32));
    }
    else {  //wrong bit position, should never happen
        status = 0;
    }
    return status;
}


void setGlobalCore(Core* _core) {
    globalCore = _core;
}

void faulInjection_setup_signals(void) {
    //prepare the segfault signal catcher
    struct sigaction actSegFault, actAssert;

    actSegFault.sa_handler = sigHandler_segfault;
    sigemptyset(&actSegFault.sa_mask);
    actSegFault.sa_flags = 0;
    sigaction(SIGSEGV, &actSegFault, 0);    //catch out of memory errors

    actAssert.sa_handler = sigHandler_assertFail;
    sigemptyset(&actAssert.sa_mask);
    actAssert.sa_flags = 0;
    sigaction(SIGABRT, &actAssert, 0);    //catch core asserts
}

void sigHandler_assertFail(int sig) {
    std::cout << "EndType : Crash" << std::endl;
}

void sigHandler_segfault(int sig) {
    std::cout << "EndType : Crash\n" << std::endl;
    exit(0);
}
