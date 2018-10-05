#ifndef FAULT_INJ_SUPPORT_H
#define FAULT_INJ_SUPPORT_H

#include "core.h"
#include <signal.h>

// The different places faults can be injected (all pipeline registers, the register file, accumulator
// and the core controler
enum InjRegisterLocation {
    FToDC_loc = 0,
    DCToEX_loc,
    EXToMEM_loc,
    MEMToWB_loc,
    PC_loc,
    RF0_loc,
    RF1_loc,
    RF2_loc,
    RF3_loc,
    RF4_loc,
    RF5_loc,
    RF6_loc,
    RF7_loc,
    RF8_loc,
    RF9_loc,
    RF10_loc,
    RF11_loc,
    RF12_loc,
    RF13_loc,
    RF14_loc,
    RF15_loc,
    RF16_loc,
    RF17_loc,
    RF18_loc,
    RF19_loc,
    RF20_loc,
    RF21_loc,
    RF22_loc,
    RF23_loc,
    RF24_loc,
    RF25_loc,
    RF26_loc,
    RF27_loc,
    RF28_loc,
    RF29_loc,
    RF30_loc,
    RF31_loc,
    CoreCtrl_loc,
    NUM_REG_LOCATIONS
};

const int injRegisterWidth[NUM_REG_LOCATIONS] = {
    97,     //FtoDc
    174,    //DctoEx, 303 if simulator
    126,    //ExtoMem, 158 if simulator
    115,    //MemtoWb, 147 if simulator
    32,     //PC
    32,     //RF0
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,     //RF31
    205     //CoreCtrl
};

int injectFault_FToDC(Core* core, int bitPosition);
int injectFault_DCToEX(Core* core, int bitPosition);
int injectFault_EXToMEM(Core* core, int bitPosition);
int injectFault_MEMToWB(Core* core, int bitPosition);
int injectFault_RF(Core* core, int registerNumber, int bitPosition);
int injectFault_PC(Core* core, int bitPosition);
int injectFault_CoreCtrl(Core* core, int bitPosition);

//to catch segfaults cased by the fault injection
void setGlobalCore(Core* core);
void faulInjection_setup_signals(void);
void sigHandler_segfault(int sig);
void sigHandler_assertFail(int sig);

#endif // FAULT_INJ_SUPPORT_H
