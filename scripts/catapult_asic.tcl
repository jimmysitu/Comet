set WORKING_DIR $::env(PWD)

solution new -state initial
solution options defaults


solution options set /ComponentLibs/TechLibSearchPath /opt/DesignKit/cmos28fdsoi_29/C28SOI_SC_12_CORE_LL/5.1-05/libs

solution options set ComponentLibs/SearchPath /opt/DesignKit/catapult_lib -append
solution options set ComponentLibs/SearchPath /opt/DesignKit/catapult_lib/memory -append

solution options set /Input/CompilerFlags {-D __CATAPULT__ -D __HLS__ -D MEMORY_INTERFACE=IncompleteMemory}
solution options set /Input/SearchPath $WORKING_DIR/../include
solution options set /Output/GenerateCycleNetlist false
solution file add $WORKING_DIR/../src/core.cpp -type C++
solution file add $WORKING_DIR/../src/multiplicationUnit.cpp -type C++

directive set -DESIGN_GOAL area

go new
directive set -DESIGN_HIERARCHY doCore
go compile
solution library add ccs_sample_mem -- -rtlsyntool DesignCompiler -vendor STMicroelectronics -technology {28nm FDSOI}
solution library add C28SOI_SC_12_CORE_LL_ccs -file /opt/DesignKit/catapult_lib/C28SOI_SC_12_CORE_LL_ccs.lib
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 2 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND sync -RESET_SYNC_ACTIVE high -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
go assembly
directive set /doCore/globalStall:rsc -MAP_TO_MODULE {[DirectInput]}
directive set /doCore/core/core.regFile:rsc -MAP_TO_MODULE {[Register]}
directive set /doCore/core/while -PIPELINE_INIT_INTERVAL 1
directive set /doCore/core/core.multiplicationUnit.process:if:else:for -UNROLL yes
directive set /doCore/imData:rsc -MAP_TO_MODULE ccs_sample_mem.ccs_ram_sync_singleport
directive set /doCore/dmData:rsc -MAP_TO_MODULE ccs_sample_mem.ccs_ram_sync_singleport
go architect
go extract
