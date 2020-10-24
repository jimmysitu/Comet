#!/bin/bash

SUBFOLDERS=$(ls -d */)
SIMS=("../../build/bin/comet.sim" "../../build/bin/comet.sim.viva")
TIMEOUT="80s"

for COMETSIM in ${SIMS[@]}
do
    echo "Start to test $COMETSIM"
    for TEST in $SUBFOLDERS
    do
        echo "started test : " $TEST
        cd $TEST
        timeout $TIMEOUT $COMETSIM -f *.riscv* -o testOutput
        STATUS=$?
        if [ $STATUS -ne 0 ]
        then
            if [ $STATUS -eq 124 ]
            then
                echo "comet.sim timed out (possible infinite loop)"
            else
                echo "comet.sim crashed"
            fi
            rm testOutput
            exit 1
        fi
        DIFF=$(diff testOutput expectedOutput)
        if [ "$DIFF" != "" ]
        then
            #This is ugly
            diff testOutput expectedOutput
            rm testOutput
            exit 1
        fi
        rm testOutput
        cd ..
    done
done
