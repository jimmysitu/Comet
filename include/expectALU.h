
#ifndef INCLUDE_EXPECT_ALU_H_
#define INCLUDE_EXPECT_ALU_H_

#define CATAPULT_BACKEND
#include "hint.hpp"
#include "dot_product.hpp"
#include <exact_prod_to_custom_flt.hpp>

#include <riscvISA.h>
#include <pipelineRegisters.h>
#include <ac_int.h>

class ExpectALU{
public:
    ExpectALU(){};


    kulischCustomFloat_prod<3, 4, hint::CatapultWrapper> kulishRegFile[8];



	bool process(struct DCtoEx dctoEx, ac_int<32, false> &result, bool &stall){
        //no need to fill in the output register fields, the first ALU has that taken care of
        bool valRet = false;
        stall = false;

    	if (dctoEx.opCode == 0x0b) {
            //We are handling an extra opcode
            if (dctoEx.funct3 == 0){
                //Performing the dot product
                customFloat<3, 4, hint::CatapultWrapper> valuesA[4];

                valuesA[0] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.lhs.slc<8>(24)));
                valuesA[1] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.lhs.slc<8>(16)));
                valuesA[2] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.lhs.slc<8>(8)));
                valuesA[3] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.lhs.slc<8>(0)));

                customFloat<3, 4, hint::CatapultWrapper> valuesB[4];
                valuesB[0] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.rhs.slc<8>(24)));
                valuesB[1] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.rhs.slc<8>(16)));
                valuesB[2] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.rhs.slc<8>(8)));
                valuesB[3] = customFloat<3, 4, hint::CatapultWrapper>(hint::CatapultWrapper<8, false>(dctoEx.rhs.slc<8>(0)));

                exactDotProduct<4, 3, 4, hint::CatapultWrapper>(valuesA, valuesB, &(kulishRegFile[dctoEx.funct7 & 0x7]));
                valRet = true;
            }
            else if (dctoEx.funct3 == 1){
                // //dumping an exact register value into a 32 bits float value
                customFloat<3, 4, hint::CatapultWrapper> floatVal = {{0}};

                exactProdToCustomFloat(kulishRegFile[dctoEx.funct7 & 0x7], &floatVal);


                result = floatVal.unravel();
                valRet = true;
            }
            else if (dctoEx.funct3 == 2){
                //Setting to zero
                kulishRegFile[dctoEx.funct7 & 0x7] = kulischCustomFloat_prod<3, 4, hint::CatapultWrapper>{{0}};
            }

        }

    }
};

#endif
