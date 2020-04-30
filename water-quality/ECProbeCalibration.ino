/* KCL Acquatic Multiprobe EC calibration sketch. 
 * file amended from DFRobot_EC.ino
 * @ https://github.com/DFRobot/DFRobot_EC
 *
 * This is the sample code for Gravity: Analog Electrical Conductivity Sensor / Meter Kit V2 (K=1.0), SKU: DFR0300.
 * In order to guarantee precision, a temperature sensor such as DS18B20 is needed, to execute automatic temperature compensation.
 * You can send commands in the serial monitor to execute the calibration.
 * Serial Commands:
 *   enter -> enter the calibration mode
 *   cal -> calibrate with the standard buffer solution, two buffer solutions(1413us/cm and 12.88ms/cm) will be automaticlly recognized
 *   exit -> save the calibrated parameters and exit from calibration mode
 *
 * Copyright   [DFRobot](https://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * version  V2.0
 * date  2019-03-11
 */

#include "DFRobot_EC.h"
#include <EEPROM.h>

#define EC_PIN A2
int T_power=6;      //Thermistor power pin
int C_power=5;      //Central GND power pin (for EC and pressure sensor)
float voltage,ecValue,temperature = 25;
DFRobot_EC ec;

void setup()
{ pinMode (T_power, OUTPUT); 
  pinMode (C_power, OUTPUT);
  digitalWrite(T_power, HIGH); 
  pinMode (C_power, HIGH);
  Serial.begin(9600);
  ec.begin();
}

void loop()
{   
    static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U)  //time interval: 1s
    {
      timepoint = millis();
      voltage = analogRead(EC_PIN)/1024.0*5000;  // read the voltage
      
      int T = analogRead(A0);       //measure temperature
      float T_V = T*3.3/1024.0; 
      float T_R = 10.0/T_V*(3.3-T_V); 
      temperature = 1/(2.772539*pow(10,-3) + 2.50767*pow(10,-4)*log(T_R) + 3.37884*pow(10,-7)*pow(log(T_R),3))-273.15; //calibrated temperature reading
      delay(5);
      
      ecValue =  ec.readEC(voltage,temperature);  // convert voltage to EC with temperature compensation
      Serial.print("temperature:");
      Serial.print(temperature);
      Serial.print("^C  EC:");
      Serial.print(ecValue,2);
      Serial.println("ms/cm");
    }
    ec.calibration(voltage,temperature);  // calibration process by Serail CMD
}

float readTemperature()
{
    
}
