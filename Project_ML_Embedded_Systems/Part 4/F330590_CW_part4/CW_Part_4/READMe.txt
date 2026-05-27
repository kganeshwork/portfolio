*** Application version ***
PyCharm IDE version 2025.2.4
Python 3.12

*** Known issues ***
- None

*** How to run ***
- Run the line provided provided below in the python terminal.
python infer.py --test_dir data/test

*** How the code works ***
- [Path] runs/best.pt holds the trained values from the most train_cnn.py run (100 epochs).
- Iterates through every file in the data/test directory.
- Outputs the confidence of each prediction and if the image class was correctly or incorrectly predicted
- Outputs the number of predictions made, correct and total.
- Outputs the models average accuracy for each class seperately and together.


*** Expected output ***
...
trianglepolis_194.png | Confidence = 1.000 | Correct
trianglepolis_195.png | Confidence = 1.000 | Correct
trianglepolis_196.png | Confidence = 1.000 | Correct
trianglepolis_197.png | Confidence = 1.000 | Correct
trianglepolis_198.png | Confidence = 1.000 | Correct
trianglepolis_199.png | Confidence = 1.000 | Correct

Inference results for the entire dataset:

Class                 Correct    Total   Accuracy
circlepolis                16       30      0.533
ovalopolis                 28       30      0.933
rectanglepolis             30       30      1.000
trianglepolis              30       30      1.000

Overall                   104      120      0.867