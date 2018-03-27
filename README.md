# Ink evaluation Apps

[Context behind the test application](https://docs.google.com/document/d/1X30cX489PXjNY8vYAaPlYhnaDSBf8fz7RggUYcwZHU8/edit?usp=sharing)

**alpaca_evaluation** is the file to be tested for evaluating the performance of InK vs Alpaca

**cm_acc** is the main testing application

**fake_acc** is providing 2 interrupt signals on the main board by raising two pins according to a some predefined values.

**python_scripts** holds the scripts used for evaluating the performance based on csv files extracted from a logic analyzer

**logic_data** holds the data extracted from a logic analyzer and the settings used on them

##Hardware:

ADXL345 is the accelerometer used connected with I2C

SPW2430 is the microphone used for sound sampling 
