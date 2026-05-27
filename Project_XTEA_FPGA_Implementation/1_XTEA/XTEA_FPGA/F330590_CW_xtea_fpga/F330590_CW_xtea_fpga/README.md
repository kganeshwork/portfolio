*** Hardware ***
XC7A35T-1CPG236C (Basys 3)

*** Application versions ***
Vivado 2019.1.1
Python 3.11

*** Optimisations ***
- Pipelined compute stage

*** Known issues ***
- None

*** How to run ***
- Change the constraint file and add the commands in the next section
- Generate bitstream and program your desired board
- Run the python script provided in F330590_CW_xtea_fpga\F330590_XTEA_python_script
- This script will need these modules serial, struct, time
- Once the script is run you should see all tests that were shown in the testbench alongside the inference time.


*** Additions to .xdc ***
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]
set_property CONFIG_MODE SPIx4 [current_design]
