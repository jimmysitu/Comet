#include "fault_inj_support.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>


//for the signal handlers to be able to output the core status and memory contents
Core* globalCore;   //yuk

/*
 * This is all terrible looking, the width of each field is hardcoded...
 * But eh, it works...
 */

int injectFault_FToDC(Core* core, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 1;

    //check in which field of the register to inject
    //and inject fault in the proper bit
    for(int i=0; i<bitPositions.size(); i++) {
        if(bitPositions[i] < 32) {      //pc
            switch(faultModel) {
            case BITFLIP:
                core->ftoDC.pc ^= (1 << bitPositions[i]);
                break;
            case STUCK_AT_ONE:
                core->ftoDC.pc |= (1 << bitPositions[i]);
                break;
            default :       //STUCK_AT_ZERO
                core->ftoDC.pc &= ~(1 << bitPositions[i]);
                break;
            }
        }
        else if(bitPositions[i] < 64) {   //instruction
            switch(faultModel) {
            case BITFLIP:
                core->ftoDC.instruction ^= (1 << (bitPositions[i]-32));
                break;
            case STUCK_AT_ONE:
                core->ftoDC.instruction |= (1 << (bitPositions[i]-32));
                break;
            default :       //STUCK_AT_ZERO
                core->ftoDC.instruction &= ~(1 << (bitPositions[i]-32));
                break;
            }
        }
        else if(bitPositions[i] < 65) {   //realInstruction
            switch(faultModel) {
            case BITFLIP:
                core->ftoDC.realInstruction = !core->ftoDC.realInstruction;
                break;
            case STUCK_AT_ONE:
                core->ftoDC.realInstruction = true;
                break;
            default :       //STUCK_AT_ZERO
                core->ftoDC.realInstruction = false;
                break;
            }
        }
        else if(bitPositions[i] < 97) {   //nextpc
            switch(faultModel) {
            case BITFLIP:
                core->ftoDC.nextpc ^= (1 << (bitPositions[i]-65));
                break;
            case STUCK_AT_ONE:
                core->ftoDC.nextpc |= (1 << (bitPositions[i]-65));
                break;
            default :       //STUCK_AT_ZERO
                core->ftoDC.nextpc &= ~(1 << (bitPositions[i]-65));
                break;
            }
        }
        else {  //wrong bit position, should never happen
            status = 0;
        }
    }
    return status;
}


int injectFault_DCToEX(Core* core, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 1;

    for(int i=0; i<bitPositions.size(); i++) {
        if(bitPositions[i] < 32) {      //pc
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.pc ^= (1 << bitPositions[i]);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.pc |= (1 << bitPositions[i]);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.pc &= ~(1 << bitPositions[i]);
                break;
            }
        }
        else if(bitPositions[i] < 39) { //opcode
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.opCode ^= (1 << (bitPositions[i])-32);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.opCode |= (1 << (bitPositions[i])-32);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.opCode &= ~(1 << (bitPositions[i])-32);
                break;
            }
        }
        else if(bitPositions[i] < 46) { //funct7
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.funct7 ^= (1 << (bitPositions[i])-39);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.funct7 |= (1 << (bitPositions[i])-39);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.funct7 &= ~(1 << (bitPositions[i])-39);
                break;
            }
        }
        else if(bitPositions[i] < 49) { //funct3
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.funct3 ^= (1 << (bitPositions[i])-46);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.funct3 |= (1 << (bitPositions[i])-46);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.funct3 &= ~(1 << (bitPositions[i])-46);
                break;
            }
        }
        else if(bitPositions[i] < 54) { //rd
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.rd ^= (1 << (bitPositions[i])-49);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.rd |= (1 << (bitPositions[i])-49);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.rd &= ~(1 << (bitPositions[i])-49);
                break;
            }
        }
        else if(bitPositions[i] < 55) { //realInstruction
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.realInstruction = !core->dctoEx.realInstruction;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.realInstruction = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.realInstruction = false;
                break;
            }
        }
        else if(bitPositions[i] < 87) { //lhs
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.lhs ^= (1 << (bitPositions[i])-55);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.lhs |= (1 << (bitPositions[i])-55);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.lhs &= ~(1 << (bitPositions[i])-55);
                break;
            }
        }
        else if(bitPositions[i] < 119) { //rhs
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.rhs ^= (1 << (bitPositions[i])-87);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.rhs |= (1 << (bitPositions[i])-87);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.rhs &= ~(1 << (bitPositions[i])-87);
                break;
            }
        }
        else if(bitPositions[i] < 151) { //datac
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.datac ^= (1 << (bitPositions[i])-119);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.datac |= (1 << (bitPositions[i])-119);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.datac &= ~(1 << (bitPositions[i])-119);
                break;
            }
        }
        else if(bitPositions[i] < 152) { //forward_lhs
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.forward_lhs = !core->dctoEx.forward_lhs;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.forward_lhs = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.forward_lhs = false;
                break;
            }
        }
        else if(bitPositions[i] < 153) { //forward_rhs
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.forward_rhs = !core->dctoEx.forward_rhs;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.forward_rhs = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.forward_rhs = false;
                break;
            }
        }
        else if(bitPositions[i] < 154) { //forward_datac
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.forward_datac = !core->dctoEx.forward_datac;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.forward_datac = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.forward_datac = false;
                break;
            }
        }
        else if(bitPositions[i] < 155) { //forward_mem_lhs
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.forward_mem_lhs = !core->dctoEx.forward_mem_lhs;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.forward_mem_lhs = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.forward_mem_lhs = false;
                break;
            }
        }
        else if(bitPositions[i] < 156) { //forward_mem_rhs
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.forward_mem_rhs = !core->dctoEx.forward_mem_rhs;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.forward_mem_rhs = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.forward_mem_rhs = false;
                break;
            }
        }
        else if(bitPositions[i] < 157) { //forward_mem_datac
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.forward_mem_datac = !core->dctoEx.forward_mem_datac;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.forward_mem_datac = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.forward_mem_datac = false;
                break;
            }
        }
        else if(bitPositions[i] < 158) { //csr
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.csr = !core->dctoEx.csr;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.csr = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.csr = false;
                break;
            }
        }
        else if(bitPositions[i] < 170) { //CSRid
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.CSRid ^= (1 << (bitPositions[i])-158);
                break;
            case STUCK_AT_ONE:
                core->dctoEx.CSRid |= (1 << (bitPositions[i])-158);
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.CSRid &= ~(1 << (bitPositions[i])-158);
                break;
            }
        }
        else if(bitPositions[i] < 171) { //external
            switch(faultModel) {
            case BITFLIP:
                core->dctoEx.external = !core->dctoEx.external;
                break;
            case STUCK_AT_ONE:
                core->dctoEx.external = true;
                break;
            default :       //STUCK_AT_ZERO
                core->dctoEx.external = false;
                break;
            }
        }
        else if(bitPositions[i] < 174) { //op
            int temp = (int)core->dctoEx.op;
            switch(faultModel) {
            case BITFLIP:
                temp ^= (1 << bitPositions[i]-171);
                break;
            case STUCK_AT_ONE:
                temp |= (1 << bitPositions[i]-171);
                break;
            default :       //STUCK_AT_ZERO
                temp &= ~(1 << bitPositions[i]-171);
                break;
            }
            core->dctoEx.op = (MultiCycleOperator::MultiCycleOperation)(temp);
        }
        else {  //wrong bit position, should never happen
            status = 0;
        }
    }
    return status;
}


int injectFault_EXToMEM(Core* core, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 1;

    for(int i=0; i<bitPositions.size(); i++) {
        if(bitPositions[i] < 32) { //result
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.result ^= (1 << (bitPositions[i]));
                break;
            case STUCK_AT_ONE:
                core->extoMem.result |= (1 << (bitPositions[i]));
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.result &= ~(1 << (bitPositions[i]));
                break;
            }
        }
        else if(bitPositions[i] < 37) { //rd
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.rd ^= (1 << (bitPositions[i]-32));
                break;
            case STUCK_AT_ONE:
                core->extoMem.rd |= (1 << (bitPositions[i]-32));
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.rd &= ~(1 << (bitPositions[i]-32));
                break;
            }
        }
        else if(bitPositions[i] < 44) { //opCode
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.opCode ^= (1 << (bitPositions[i]-37));
                break;
            case STUCK_AT_ONE:
                core->extoMem.opCode |= (1 << (bitPositions[i]-37));
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.opCode &= ~(1 << (bitPositions[i]-37));
                break;
            }
        }
        else if(bitPositions[i] < 47) { //funct3
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.funct3 ^= (1 << (bitPositions[i]-44));
                break;
            case STUCK_AT_ONE:
                core->extoMem.funct3 |= (1 << (bitPositions[i]-44));
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.funct3 &= ~(1 << (bitPositions[i]-44));
                break;
            }
        }
        else if(bitPositions[i] < 48) { //realInstruction
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.realInstruction = !core->extoMem.realInstruction;
                break;
            case STUCK_AT_ONE:
                core->extoMem.realInstruction = true;
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.realInstruction = false;
                break;
            }
        }
        else if(bitPositions[i] < 49) { //external
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.external = !core->extoMem.external;
                break;
            case STUCK_AT_ONE:
                core->extoMem.external = true;
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.external = false;
                break;
            }
        }
        else if(bitPositions[i] < 50) { //csr
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.csr = !core->extoMem.csr;
                break;
            case STUCK_AT_ONE:
                core->extoMem.csr = true;
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.csr = false;
                break;
            }
        }
        else if(bitPositions[i] < 62) { //CSRid
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.CSRid ^= (1 << (bitPositions[i]-50));
                break;
            case STUCK_AT_ONE:
                core->extoMem.CSRid |= (1 << (bitPositions[i]-50));
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.CSRid &= ~(1 << (bitPositions[i]-50));
                break;
            }
        }
        else if(bitPositions[i] < 94) { //datac
            switch(faultModel) {
            case BITFLIP:
                core->extoMem.datac ^= (1 << (bitPositions[i]-62));
                break;
            case STUCK_AT_ONE:
                core->extoMem.datac |= (1 << (bitPositions[i]-62));
                break;
            default :       //STUCK_AT_ZERO
                core->extoMem.datac &= ~(1 << (bitPositions[i]-62));
                break;
            }
        }
        else {  //wrong bit position, should never happen
            status = 0;
        }
    }
    return status;
}

int injectFault_MEMToWB(Core* core, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 1;

    for(int i=0; i<bitPositions.size(); i++) {
        if(bitPositions[i] < 32) { //result
            switch(faultModel) {
            case BITFLIP:
                core->memtoWB.result ^= (1 << (bitPositions[i]));
                break;
            case STUCK_AT_ONE:
                core->memtoWB.result |= (1 << (bitPositions[i]));
                break;
            default :       //STUCK_AT_ZERO
                core->memtoWB.result &= ~(1 << (bitPositions[i]));
                break;
            }
        }
        else if(bitPositions[i] < 37) { //rd
            switch(faultModel) {
            case BITFLIP:
                core->memtoWB.rd ^= (1 << (bitPositions[i]-32));
                break;
            case STUCK_AT_ONE:
                core->memtoWB.rd |= (1 << (bitPositions[i]-32));
                break;
            default :       //STUCK_AT_ZERO
                core->memtoWB.rd &= ~(1 << (bitPositions[i]-32));
                break;
            }
        }
        else if(bitPositions[i] < 38) { //realInstruction
            switch(faultModel) {
            case BITFLIP:
                core->memtoWB.realInstruction = !core->memtoWB.realInstruction;
                break;
            case STUCK_AT_ONE:
                core->memtoWB.realInstruction = true;
                break;
            default :       //STUCK_AT_ZERO
                core->memtoWB.realInstruction = false;
                break;
            }
        }
        else if(bitPositions[i] < 39) { //csr
            switch(faultModel) {
            case BITFLIP:
                core->memtoWB.csr = !core->memtoWB.csr;
                break;
            case STUCK_AT_ONE:
                core->memtoWB.csr = true;
                break;
            default :       //STUCK_AT_ZERO
                core->memtoWB.csr = false;
                break;
            }
        }
        else if(bitPositions[i] < 51) { //CSRid
            switch(faultModel) {
            case BITFLIP:
                core->memtoWB.CSRid ^= (1 << (bitPositions[i]-39));
                break;
            case STUCK_AT_ONE:
                core->memtoWB.CSRid |= (1 << (bitPositions[i]-39));
                break;
            default :       //STUCK_AT_ZERO
                core->memtoWB.CSRid &= ~(1 << (bitPositions[i]-39));
                break;
            }
        }
        else if(bitPositions[i] < 83) { //rescsr
            switch(faultModel) {
            case BITFLIP:
                core->memtoWB.rescsr ^= (1 << (bitPositions[i]-51));
                break;
            case STUCK_AT_ONE:
                core->memtoWB.rescsr |= (1 << (bitPositions[i]-51));
                break;
            default :       //STUCK_AT_ZERO
                core->memtoWB.rescsr &= ~(1 << (bitPositions[i]-51));
                break;
            }
        }
        else {  //wrong bit position, should never happen
            status = 0;
        }
    }
    return status;
}

int injectFault_RF(Core* core, int registerNumber, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 1;

    for(int i=0; i<bitPositions.size(); i++) {
        if((bitPositions[i] >= 0) && (bitPositions[i] < 32) && (registerNumber > 0) && (registerNumber < 32)) {
            switch(faultModel) {
            case BITFLIP:
                core->REG[registerNumber] ^= (1 << bitPositions[i]);
                break;
            case STUCK_AT_ONE:
                core->REG[registerNumber] |= (1 << bitPositions[i]);
                break;
            default :       //STUCK_AT_ZERO
                core->REG[registerNumber] &= ~(1 << bitPositions[i]);
                break;
            }
        }
        else {  //wrong register or wrong bit position, should never happen
            status = 0;
        }
    }
    return status;
}

int injectFault_PC(Core* core, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 0;
    for(int i=0; i<bitPositions.size(); i++) {
        if((bitPositions[i] >= 0) && (bitPositions[i] < 32)) {
            switch(faultModel) {
            case BITFLIP:
                core->pc ^= (1 << bitPositions[i]);
                break;
            case STUCK_AT_ONE:
                core->pc |= (1 << bitPositions[i]);
                break;
            default :       //STUCK_AT_ZERO
                core->pc &= ~(1 << bitPositions[i]);
                break;
            }
            status = 1;
        }
    }
    return status;
}

int injectFault_CoreCtrl(Core* core, std::vector<int> bitPositions, FaultModel faultModel) {
    int status = 1;
    std::cout << "This is the size : " << bitPositions.size() << std::endl;
    printf("injecting in CoreCtrl using FM : %d, bits [ ", faultModel);
    for(int i=0; i<bitPositions.size()-1; i++) {
        printf("%d, ", bitPositions[i]);
    }
    printf("%d]\n", bitPositions[bitPositions.size()-1]);

    for(int i=0; i<bitPositions.size(); i++) {
        if(bitPositions[i] < 15) {  //prev_rds
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.prev_rds[(int)(bitPositions[i]/5)] ^= (1 << (bitPositions[i]%5));
                break;
            case STUCK_AT_ONE:
                core->ctrl.prev_rds[(int)(bitPositions[i]/5)] |= (1 << (bitPositions[i]%5));
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.prev_rds[(int)(bitPositions[i]/5)] &= ~(1 << (bitPositions[i]%5));
                break;
            }
        }
        else if(bitPositions[i] < 36) {  //prev_opCode
            switch(faultModel) {
            case BITFLIP:
                printf("field %d, bit %d, before : %d\n", (int)((bitPositions[i]-15)/7), bitPositions[i], core->ctrl.prev_opCode[(int)((bitPositions[i]-15)/7)]);
                core->ctrl.prev_opCode[(int)((bitPositions[i]-15)/7)] ^= (1 << ((bitPositions[i]-15)%7));
                printf("after : %d\n", core->ctrl.prev_opCode[(int)((bitPositions[i]-15)/7)]);
                break;
            case STUCK_AT_ONE:
                core->ctrl.prev_opCode[(int)((bitPositions[i]-15)/7)] |= (1 << ((bitPositions[i]-15)%7));
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.prev_opCode[(int)((bitPositions[i]-15)/7)] &= ~(1 << ((bitPositions[i]-15)%7));
                break;
            }
        }
        else if(bitPositions[i] < 38) {  //lock
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.lock ^= (1 << (bitPositions[i]-36));
                break;
            case STUCK_AT_ONE:
                core->ctrl.lock |= (1 << (bitPositions[i]-36));
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.lock &= ~(1 << (bitPositions[i]-36));
                break;
            }
        }
        else if(bitPositions[i] < 39) {  //freeze_fetch
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.freeze_fetch = !core->ctrl.freeze_fetch;
                break;
            case STUCK_AT_ONE:
                core->ctrl.freeze_fetch = true;
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.freeze_fetch = false;
                break;
            }
        }
        else if(bitPositions[i] < 40) {  //cachelock
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.cachelock = !core->ctrl.cachelock;
                break;
            case STUCK_AT_ONE:
                core->ctrl.cachelock = true;
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.cachelock = false;
                break;
            }
        }
        else if(bitPositions[i] < 41) {  //init
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.init = !core->ctrl.init;
                break;
            case STUCK_AT_ONE:
                core->ctrl.init = true;
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.init = false;
                break;
            }
        }
        else if(bitPositions[i] < 42) {  //sleep
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.sleep = !core->ctrl.sleep;
                break;
            case STUCK_AT_ONE:
                core->ctrl.sleep = true;
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.sleep = false;
                break;
            }
        }
        else if(bitPositions[i] < 138) {  //prev_res
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.prev_res[(bitPositions[i]-42)/32] ^= (1 << ((bitPositions[i]-42)%32));
                break;
            case STUCK_AT_ONE:
                core->ctrl.prev_res[(bitPositions[i]-42)/32] |= (1 << ((bitPositions[i]-42)%32));
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.prev_res[(bitPositions[i]-42)/32] &= ~(1 << ((bitPositions[i]-42)%32));
                break;
            }
        }
        else if(bitPositions[i] < 141) {  //branch
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.branch[(bitPositions[i]-138)] = !core->ctrl.branch[(bitPositions[i]-138)];
                break;
            case STUCK_AT_ONE:
                core->ctrl.branch[(bitPositions[i]-138)] = true;
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.branch[(bitPositions[i]-138)] = false;
                break;
            }
        }
        else if(bitPositions[i] < 205) { //jump_pc
            switch(faultModel) {
            case BITFLIP:
                core->ctrl.prev_res[(bitPositions[i]-141)/32] ^= (1 << ((bitPositions[i]-141)%32));
                break;
            case STUCK_AT_ONE:
                core->ctrl.prev_res[(bitPositions[i]-141)/32] |= (1 << ((bitPositions[i]-141)%32));
                break;
            default :       //STUCK_AT_ZERO
                core->ctrl.prev_res[(bitPositions[i]-141)/32] &= ~(1 << ((bitPositions[i]-141)%32));
                break;
            }
        }
        else {  //wrong bit position, should never happen
            status = 0;
        }
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
    std::cout << "EndType : Crash" << std::endl;
    exit(0);
}

//no error checking...
void saveSystemSnapshot(char* filename, unsigned int *memory, unsigned long length, Core* core) {
    std::ofstream memDumpFile;
    int regDumpArray[TOTAL_REG_WIDTH] = {0};
    memDumpFile.open(filename);
    //write the memory
    memDumpFile.write(reinterpret_cast<const char *>(memory), length*sizeof(memory[0]));
    //then write the contents of the registers
    core->ftoDC.dumpContents(regDumpArray);
    memDumpFile.write(reinterpret_cast<const char *>(regDumpArray), FTODC_WIDTH*sizeof(regDumpArray[0]));
    core->dctoEx.dumpContents(regDumpArray);
    memDumpFile.write(reinterpret_cast<const char *>(regDumpArray), DCTOEX_WIDTH*sizeof(regDumpArray[0]));
    core->extoMem.dumpContents(regDumpArray);
    memDumpFile.write(reinterpret_cast<const char *>(regDumpArray), EXTOMEM_WIDTH*sizeof(regDumpArray[0]));
    core->memtoWB.dumpContents(regDumpArray);
    memDumpFile.write(reinterpret_cast<const char *>(regDumpArray), MEMTOWB_WIDTH*sizeof(regDumpArray[0]));
    core->ctrl.dumpContents(regDumpArray);
    memDumpFile.write(reinterpret_cast<const char *>(regDumpArray), CORECTRL_WIDTH*sizeof(regDumpArray[0]));
    core->dumpContents(regDumpArray);
    memDumpFile.write(reinterpret_cast<const char *>(regDumpArray), COREREG_WIDTH*sizeof(regDumpArray[0]));
    //close the fd
    memDumpFile.close();
}
