### Disclaimer: 
The material in this repository is intended as documentation of the process by which the King's College London environmental monitoring team build our open-source loggers. Though we take care to ensure that the pages are accurate as of the date of publication, Arduino software, libraries, electronic components and interface devices are all subject to variation, change with time, and all introduce the potential for risk. The authors take no responsibility for the consequences of error or for any loss, damage or injury suffered by users or their property as a result of any of the information published on any of these pages, and such information does not form any basis of a contract with readers or users of it. The audience should verify any information provided and only proceed if they have an adequate understanding of electronics and electronics safety.

## Instructions
1. Source the material listed in the ![Bill-Of-Materials document]().
2. Connect the logger as shown in the breadboard connection diagram (basic-logger-breadboard.jpg file or shown below). Take care to ensure you connect the battery terminals correctly and make sure you do not leave loose wires (with the potential for short-circuiting) trailing.
3. Remove the battery pack from the circuit whenever plugging the Arduino into your computer (taking the necessary precautions)
4. Ensure the DS3234 clock has the correct battery in the correct way around.
5. Ensure the microSD card holder has a suitable microSD card in formatted to the correct file type (FAT
3. Connect the 3.3V Pro Mini to your computer using a suitable programmer.
4. Set the correct Arduino (3.3V pro mini) and COM port which the device is connected to.
#### Clock setup and testing
4. Download and install the ![DS3234 library](https://github.com/rodan/ds3234) to your Arduino IDE.
. flash the rtc_ds3234 example sketch to the Arduino.
5. Open the serial monitor and ensure the baud rate is set at '9600 baud' and carriage return/line ending is set to 'Both NL & CR'). If connected correctly, your clock should be showing an incrementally increasing time every 5 seconds; if it is not, disconnect the programmer from your computer, recheck the wiring and restart the process.
6. 
7. plug your Arduino back into your computer and reopen the Serial monitor (you may need to make sure the correct com port is set under Tools > Port). Your clock should be maintaining and Serial printing the time without the need for resetting.

#### SD card setup and testing
5. Download and install the ![SDFat library](https://github.com/greiman/SdFat) to your Arduino IDE.
. Flash the 
6. Open the serial monitor (ensuring baud rate is set at 9600 and NL/CR).
7. 


### Basic logger breadboard connection diagram (image made with Fritzing):

![Basic logger breadboard connection diagram](basic-logger-breadboard.jpg)  

Note that the wires are coloured with the following colouration:
Red - 3.3V/VCC
Black - GND
Blue - CS/SS
Green - MOSI
Yellow - MISO
Brown - CLK
Orange - SQW/INT

### Basic logger breadboard circuit schematics (image made with Fritzing):

![Basic ](basic-logger-schematic.jpg)

Note: the wires in the breadboard connection diagram are 
