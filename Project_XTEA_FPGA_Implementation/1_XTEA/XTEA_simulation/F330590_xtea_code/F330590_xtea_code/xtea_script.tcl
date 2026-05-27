vlib work
vmap work work

vcom xtea_top.vhd
vcom xtea_tb_simplex.vhd

vsim work.xtea_tb

set StdArithNoWarnings 1
set NumericStdNoWarnings 1

# Add testbench top-level signals
add wave -divider "Testbench I/O"
add wave *

# Configure waveform display
configure wave -timelineunits ns

echo "----------- Simulation running ------------"

run -all

wave zoom full