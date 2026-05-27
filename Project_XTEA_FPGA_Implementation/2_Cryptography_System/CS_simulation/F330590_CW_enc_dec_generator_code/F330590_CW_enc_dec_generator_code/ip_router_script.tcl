vlib work
vmap work work

vcom mini_router_vhdl.vhd
vlog enc_gen.sv dec_gen.sv


vlog ip_gen_tb.sv

vsim work.ip_router_tb

set StdArithNoWarnings 1
set NumericStdNoWarnings 1

add wave -divider "Clock and Reset"
add wave clk
add wave reset

add wave -divider "Link 1"
add wave req1
add wave data1
add wave grant1

add wave -divider "Link 2"
add wave req2
add wave data2
add wave grant2

add wave -divider "Mini Router Output"
add wave valid
add wave data_out

configure wave -timelineunits ns

run -all

wave zoom full
