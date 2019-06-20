#ifndef PIPELINE_REGISTERS_H_
#define PIPELINE_REGISTERS_H_

/******************************************************************************************
 * Definition of all pipeline registers
 *
 * ****************************************************************************************
 */

struct ForwardReg {
	bool forwardWBtoVal1;
	bool forwardWBtoVal2;
	bool forwardWBtoVal3;

	bool forwardMemtoVal1;
	bool forwardMemtoVal2;
	bool forwardMemtoVal3;

	bool forwardExtoVal1;
	bool forwardExtoVal2;
	bool forwardExtoVal3;
};

struct FtoDC
{
    FtoDC() : pc(0), instruction(0x13), we(1)
    {}
    ca_uint<32> pc;           	// PC where to fetch
    ca_uint<32> instruction;  	// Instruction to execute
    ca_uint<32> nextPCFetch;      // Next pc according to fetch

    //Register for all stages
    bool we;
};

struct DCtoEx
{
    ca_uint<32> pc;       // used for branch
    ca_uint<32> instruction;

    ca_uint<7> opCode;    // opCode = instruction[6:0]
    ca_uint<7> funct7;    // funct7 = instruction[31:25]
    ca_uint<3> funct3;    // funct3 = instruction[14:12]

    ca_int<32> lhs;   //  left hand side : operand 1
    ca_int<32> rhs;   // right hand side : operand 2
    ca_int<32> datac; // ST, BR, JAL/R,

    //For branch unit
    ca_uint<32> nextPCDC;
    bool isBranch;

    //Information for forward/stall unit
    bool useRs1;
    bool useRs2;
    bool useRs3;
    bool useRd;
    ca_uint<5> rs1;       // rs1    = instruction[19:15]
    ca_uint<5> rs2;       // rs2    = instruction[24:20]
    ca_uint<5> rs3;
    ca_uint<5> rd;        // rd     = instruction[11:7]

    //Register for all stages
    bool we;
};

struct ExtoMem
{
    ca_uint<32> pc;
    ca_uint<32> instruction;

    ca_int<32> result;    // result of the EX stage
    ca_uint<5> rd;        // destination register
    bool useRd;
    bool isLongInstruction;
    ca_uint<7> opCode;    // LD or ST (can be reduced to 2 bits)
    ca_uint<3> funct3;    // datasize and sign extension bit

    ca_int<32> datac;     // data to be stored in memory or csr result

    //For branch unit
    ca_uint<32> nextPC;
    bool isBranch;

    //Register for all stages
    bool we;
};

struct MemtoWB
{
    ca_uint<32> result;    // Result to be written back
    ca_uint<5> rd;        // destination register
    bool useRd;

    ca_int<32> address;
    ca_uint<32> valueToWrite;
    ca_uint<4> byteEnable;
    bool isStore;
    bool isLoad;

    //Register for all stages
    bool we;
};

struct WBOut
{
	ca_uint<32> value;
	ca_uint<5> rd;
	bool useRd;
    bool we;
};

#endif /* PIPELINE_REGISTERS_H_ */
