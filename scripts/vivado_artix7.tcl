# Create a Vivado HLS project
set WORKING_DIR $::env(PWD)

open_project -reset Comet_artix7
set_top doCore
add_files $WORKING_DIR/../src/core.cpp \
    -cflags "-I$WORKING_DIR/../include -D__VIVADO__ -D__HLS__ -DMEMORY_INTERFACE=IncompleteMemory -std=c++11 -Wall"

# Solution *************************
open_solution -reset "default"
set_part {xc7a12tcsg325-2}
create_clock -period 10 -name default

# Directives *************************
set_directive_interface -mode ap_none "doCore" globalStall

set_directive_interface -mode bram "doCore" imData
set_directive_resource -core RAM_1P "doCore" imData

set_directive_interface -mode bram "doCore" dmData
set_directive_resource -core RAM_1P "doCore" dmData

set_directive_resource -core XPM_MEMORY -memory_style auto "doCore" core.regFile

set_directive_pipeline -II 1 "doCycle"
set_directive_latency -min 4 -max 6 "doCycle"

# Run C simulation
#csim_design
# Run Synthesis
csynth_design
# Run RTL verification
#cosim_design
# Create the IP package
export_design -flow syn -format ip_catalog -rtl verilog \
    -library "hsl" \
    -vendor "jimmystone.cn" -version "0.1.0"

exit



