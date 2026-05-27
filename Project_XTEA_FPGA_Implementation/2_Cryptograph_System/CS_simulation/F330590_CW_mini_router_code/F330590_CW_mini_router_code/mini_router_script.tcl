vlib work
vmap work work

vcom mini_router_vhdl.vhd
vcom mini_router_tb.vhd

vsim work.mini_router_tb

set StdArithNoWarnings 1
set NumericStdNoWarnings 1

add wave -label "clk"      /mini_router_tb/clk
add wave -label "reset"    /mini_router_tb/reset
add wave -label "req1"     /mini_router_tb/req1
add wave -label "data1"    /mini_router_tb/data1
add wave -label "req2"     /mini_router_tb/req2
add wave -label "data2"    /mini_router_tb/data2
add wave -label "grant1"   /mini_router_tb/grant1
add wave -label "grant2"   /mini_router_tb/grant2
add wave -label "valid"    /mini_router_tb/valid
add wave -label "data_out" /mini_router_tb/data_out

# Configure waveform display
configure wave -timelineunits ns

echo "----------- Simulation running ------------"

run -all

wave zoom full