
# Instructions:
1. Read and confirm the declaration and license associated with these instructions.
2. Start by following the instructions to build and test the [Basic core logger page](https://github.com/KCLGeography/environmental-monitoring/tree/master/basic-logger).

### Adding the DO probe
3. Connect the wires required for the Atlas Scientific DO probe.
4. Download the [example code for the Atlas Scientific DO probe](https://www.atlas-scientific.com/_files/code/ino_files/Arduino_UNO_DO_sample_code.zip).
5. Define rx and tx to be 7 and 8 respectively.
6. Upload the file and open the Serial Monitor.
7. Follow the [instructions here](https://www.instructables.com/id/Atlas-Scientific-EZO-DO-Calibration-Procedure/) to calibrate the DO probe.

### Adding the remaining sensors
8. Connect the remaining sensors, including the 5V supply circuitry.
9. Upload the [WaterQualityLogger.ino sketch](WaterQualityLogger.ino).
10. Wait for the reading to occur (every minute) and check that readings are valid in the serial monitor. Results will be in CSV format of Year, month, day, hour, minute, Conductivity voltage, calibrated conductivity (ms/cm), pressure voltage, calibrated temperature (°C), Dissolved Oxygen reading (mg/L).

### Calibrating the EC probe
11. Download and install the [DFRobot_EC Arduino Library](https://github.com/DFRobot/DFRobot_EC/archive/master.zip) within your Arduino IDE.
12. Upload the [ECProbeCalibration.ino](ECProbeCalibration.ino) sketch and follow the calibration instructions [here](https://wiki.dfrobot.com/Gravity__Analog_Electrical_Conductivity_Sensor___Meter_V2__K%3D1__SKU_DFR0300#target_3) noting that you should be using the amended calibration sketch you have already installed, and changing the Serial baud rate to 9600.

### Preparing the final sketch
13. Back in the [WaterQualityLogger.ino sketch](WaterQualityLogger.ino), set the "sleep_period" to a desirable time and reupload the sketch (note, the maximum period without more significant amendment to the sketch is 59 minutes).
14. Test that the logger is working properly before deploying.
