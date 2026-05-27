*** Application versions ***
STM32CubeIDE Version 1.19.0
Putty Version 0.83
X-Cube-AI Version 10.2.0

*** Optimisations ***
Optimisation level set to -O3

*** Known issues ***
- Issue caused by linker meaning the program does not build or run when initiated for the first time.

*** How to run ***
- Press build/run once, ignore the errors and press run again, the sine wave model should then run as normal.
- Note pressing build or run for the second time removes all errors and warnings.
- Code regeneration is not required as it will make unwanted changes to main.h.

*** How the code works ***
- Sine wave model should run inferences on 16 different input values.
- The output line will show numbers in the following order [model prediction, actual value, error between prediction and actual].
- Inference time is obtained using getCurrentMicros().
- Average inferences time is obtained.
- Accuracy metrics include mean absolute error, mean squared error and root mean squared error.

*** Expected putty window output ***
...
Test case 15: Input = 5.8643 rad (336.00 deg)
Output = [-0.382001  -0.406736  -0.024735] > Negative: Red LED ON

Inference time: 60 us

Test case 16: Input = 6.2832 rad (360.00 deg)
Output = [-0.054234  0.000000  +0.054235] > Intersecting Zero: Both LEDs OFF

Inference time: 60 us

  Average time   : 61 us
  MAE  (Mean Absolute Error): 0.028237
  MSE  (Mean Squared Error): 0.001215
  RMSE (Root Mean Squared Error): 0.034855

