# Create a Vivado HLS project
set WORKING_DIR $::env(PWD)

open_project -reset Comet_zynq7
set_top doCore
add_files $WORKING_DIR/../src/core.cpp \
    -cflags "-I$WORKING_DIR/../include -D__VIVADO__ -D__HLS__ -DMEMORY_INTERFACE=IncompleteMemory -std=c++11 -Wall"

# Solution *************************
open_solution -reset "default"
set_part {xc7z020clg400-2}
create_clock -period 10 -name default
config_bind -effort high
config_schedule -effort high -verbose

# Directives *************************
set_directive_interface -mode ap_none "doCore" globalStall
set_directive_interface -mode ap_memory -latency 1 "doCore" imData
set_directive_interface -mode ap_memory -latency 1 "doCore" dmData

set_directive_resource -latency 1 "doCore" core.regFile
set_directive_dependence -variable core.regFile -direction WAR -type intra -dependent false "doCycle"
set_directive_dependence -variable core.dctoEx.lhs -direction WAR -type intra -dependent false "doCycle"
set_directive_dependence -variable core.dctoEx.rhs -direction WAR -type intra -dependent false "doCycle"
set_directive_dependence -variable core.dctoEx.datac -direction WAR -type intra -dependent false "doCycle"

set_directive_pipeline -II 1 -rewind "doCycle"

# Run C simulation
#csim_design
# Run Synthesis
csynth_design
# Run RTL verification
#cosim_design
# Create the IP package
export_design -format ip_catalog -rtl verilog \
    -library "hsl" \
    -vendor "jimmystone.cn" -version "0.1.0" \
    -flow syn

exit



