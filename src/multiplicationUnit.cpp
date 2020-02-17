#include <ac_int.h>
#include <core.h>

bool MultiplicationUnit::process(struct DCtoEx dctoEx, ac_int<32, false>& result, bool& stall)
{
  // no need to fill in the output register fields, the first ALU has that taken care of
  bool valRet = false;

  if (dctoEx.opCode == RISCV_OP && dctoEx.funct7 == RISCV_OP_M) {

    if (state == 0) {
      dataAUnsigned.set_slc(0, dctoEx.lhs);
      dataBUnsigned.set_slc(0, dctoEx.rhs);
      // mult results
      ac_int<32, false> resultU  = dataAUnsigned * dataBUnsigned;
      ac_int<32, false> resultS  = dctoEx.lhs * dctoEx.rhs;
      ac_int<32, false> resultSU = dctoEx.lhs * dataBUnsigned;
      resIsNeg                   = dctoEx.lhs[31] ^ dctoEx.rhs[31];

      switch (dctoEx.funct3) {
        case RISCV_OP_M_MUL:
          result = resultS.slc<32>(0);
          valRet = true;
          break;
        case RISCV_OP_M_MULH:
          result = resultS.slc<32>(32);
          valRet = true;
          break;
        case RISCV_OP_M_MULHSU:
          result = resultSU.slc<32>(32);
          valRet = true;
          break;
        case RISCV_OP_M_MULHU:
          result = resultU.slc<32>(32);
          valRet = true;
          break;
        case RISCV_OP_M_DIV:
          if (dctoEx.lhs[31]) {
            dataAUnsigned = -dctoEx.lhs;
          }
          if (dctoEx.rhs[31]) {
            dataBUnsigned = -dctoEx.rhs;
          }
        case RISCV_OP_M_DIVU:
          if (dataBUnsigned == 0) {
            result = -1;
            valRet = true;
          } else {
            state     = 32;
            quotient  = 0;
            remainder = 0;
          }
          break;
        case RISCV_OP_M_REM:
          if (dctoEx.lhs[31]) {
            dataAUnsigned = -dctoEx.lhs;
          }
          if (dctoEx.rhs[31]) {
            dataBUnsigned = -dctoEx.rhs;
          }
        case RISCV_OP_M_REMU:
          if (dataBUnsigned == 0) {
            result = dataAUnsigned;
          } else {
            state     = 32;
            quotient  = 0;
            remainder = 0;
          }
          break;
      }
    } else {
      // Loop for the division
      for (i = 0; i < 4; i++) {
        state--;
        remainder    = remainder << 1;
        remainder[0] = dataAUnsigned[state];
        if (remainder >= dataBUnsigned) {
          remainder       = remainder - dataBUnsigned;
          quotient[state] = 1;
        }
      }
      if (state == 0) {
        switch (dctoEx.funct3) {
          case RISCV_OP_M_DIV:
            if (resIsNeg)
              result = -quotient;
            else
              result = quotient;
            valRet = true;
            break;
          case RISCV_OP_M_DIVU:
            result = quotient;
            valRet = true;
            break;
          case RISCV_OP_M_REM:
            if (dataAUnsigned[31])
              result = -remainder;
            else
              result = remainder;
            valRet = true;
            break;
          case RISCV_OP_M_REMU:
            result = remainder;
            valRet = true;
            break;
        }
      }
    }
    stall |= (state != 0);
  }
  return valRet;
}
