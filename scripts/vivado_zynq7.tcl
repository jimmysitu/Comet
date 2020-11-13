# Create a Vivado HLS project
set WORKING_DIR $::env(PWD)

open_project -reset Comet_zynq7
set_top doCore
add_files $WORKING_DIR/../src/core.cpp \
    -cflags "-I$WORKING_DIR/../include -D__VIVADO__ -D__HLS__ -DMEMORY_INTERFACE=IncompleteMemory -std=c++11 -Wall"

# Solution for SRAM interface *********************
open_solution -reset "default"
set_part {xc7z020clg400-2}
create_clock -period 5 -name default
config_bind -effort high
config_schedule -effort high -verbose

# Directives *************************
set_directive_interface -mode ap_none "doCore" globalStall
set_directive_interface -mode ap_memory -latency 1 "doCore" imData
set_directive_interface -mode ap_memory -latency 1 "doCore" dmData

# To achieve 1W2R register file, directive core.regFile to registers
set_directive_array_partition -dim 0 "doCore" core.regFile
set_directive_pipeline -II 1 "doCycle"

#FIXME: Try to pipeline manually, not work
set_directive_latency -min 5 "doCore"
set_directive_stream -depth 1 "fetch"   ftoDC
set_directive_stream -depth 1 "decode"  dctoEx
set_directive_stream -depth 1 "execute" extoMem
set_directive_stream -depth 1 "memory"  memtoWB
#set_directive_latency -min 1 "fetch"
#set_directive_latency -min 1 "decode"
#set_directive_latency -min 1 "execute"
#set_directive_latency -min 1 "memory"
#set_directive_latency -min 1 "writeback"


# Run C simulation
#csim_design
# Run Synthesis
csynth_design
# Run RTL verification
#cosim_design
# Create the IP package
export_design -format ip_catalog -rtl verilog \
    -library "hls" \
    -vendor "jimmystone.cn" -version "0.1.0" \

#    -flow impl

## Solution for AXI interface *********************
#open_solution -reset "axi_interface"
#set_part {xc7z020clg400-2}
#create_clock -period 10 -name default
#config_bind -effort high
#config_schedule -effort high -verbose
#
## Directives *************************
#set_directive_interface -mode ap_none "doCore" globalStall
#set_directive_interface -mode m_axi -latency 1 "doCore" imData
#set_directive_interface -mode m_axi -latency 1 "doCore" dmData
#
#
#set_directive_array_partition -dim 0 "doCore" core.regFile
#set_directive_pipeline -II 1 "doCycle"
#
## Run C simulation
##csim_design
## Run Synthesis
#csynth_design
## Run RTL verification
##cosim_design
## Create the IP package
#export_design -format ip_catalog -rtl verilog \
#    -library "hsl" \
#    -vendor "jimmystone.cn" -version "0.1.0" \
#
##    -flow syn

exit



