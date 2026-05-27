*** Application versions ***
STM32CubeIDE Version 1.19.0
Putty Version 0.83

*** Optimisations ***
Optimisation level set to -O3.

*** Known issues ***
- None

*** How to run ***
- Press build once then run once and the model should run as normal.
- Code regeneration is not required as it will make unwanted changes to main.h.

*** How the code works ***
- The ANN instantiated and the test data is fed into the neural_net_run() function.
- max_run can be modified to increase the number of test runs.
- Expected output should match the results given in the CW_material.

*** Expected putty window output ***
...
0.082915 0.461439 0.514818 -> 2
0.079324 0.472678 0.535256 -> 2
0.069109 0.464019 0.568307 -> 2
0.069048 0.459567 0.570034 -> 2
0.077958 0.462513 0.531307 -> 2
0.082218 0.473305 0.510189 -> 2
0.083478 0.469800 0.522209 -> 2
0.075871 0.462768 0.558957 -> 2
0.087133 0.474291 0.523780 -> 2
Run 1: 10367 us

Run 2: 10363 us

Run 3: 10362 us

Run 4: 10366 us

Run 5: 10366 us

Run 6: 10365 us

Run 7: 10363 us

Run 8: 10362 us

Run 9: 10362 us

Run 10: 10362 us

Average inference time across 10 runs: 10363 us