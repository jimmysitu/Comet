#include <ac_int.h>
#include <core.h>

bool ReqAckUnit::process(struct DCtoEx dctoEx, ac_int<32, false>& result, bool& stall)
{
  ac_int<NB_CORES, false> localStatus = status;

  if (state == 0) {
    // IDLE
    if (dctoEx.opCode == 0x0b) {
      // We are handling an extra opcode
      if (dctoEx.funct3 == 0) {
        // Notify opcode
        notifyOut = dctoEx.lhs.slc<NB_CORES>(0) & (!statusIn);
        maskOut   = dctoEx.lhs.slc<NB_CORES>(0) & statusIn;

        if (maskOut != 0) {
          state = 1;
        }
      } else if (dctoEx.funct3 == 1) {
        // Check opCode
        if ((dctoEx.lhs.slc<NB_CORES>(0) & !status) == 0) {
          // Task is ready
          localStatus = localStatus ^ dctoEx.lhs.slc<NB_CORES>(0);
        } else {
          state = 2;
        }
      }
    }

  } else if (state == 1) {
    // Notifies
    notifyOut = maskOut & (!statusIn);
    maskOut   = maskOut & statusIn;

    if (maskOut == 0) {
      state = 0;
    }
  } else if (state == 2) {
    // Check
    if ((dctoEx.lhs.slc<NB_CORES>(0) & !status) == 0) {
      // Task is ready
      localStatus = localStatus ^ dctoEx.lhs.slc<NB_CORES>(0);
      state       = 0;
    }
  }

  status = localStatus | notifyIn;
  return false;
}
