// Code developed by Environmental Research Group, Department of Geography, King's College London //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* LIBRARIES */
#include <SPI.h>                  //SPI Communication library
#include <avr/sleep.h>            //low power library
#include <avr/power.h>            //low power library
#include <SoftwareSerial.h>       //Software Serial Communication library
#include "ds3234.h"               //DS3234 RTC library. Available from https://github.com/rodan/ds3234
#include <SdFat.h>                //SD card library. Available from https://github.com/greiman/SdFat


//SERIAL COMMS HOUSEKEEPING (FOR DO PROBE)
#define rx 7    //set rx for SoftwareSerial
#define tx 8    //set tx for SoftwareSerial
SoftwareSerial myserial(rx, tx);
String inputstring = ""; 
String sensorstring = ""; 
boolean input_string_complete = false; 
boolean sensor_string_complete = false; 
float DO;


//SD CARD HOUSEKEEPING
const int CS = 10; 
SdFat sd; 
SdFile file;
char newfile[] = "Multiprobe_data.csv"; 


//RTC HOUSEKEEPING
int ss = 9; 
int AlarmPin = 2; 
unsigned char wakeup_min;
struct ts t; 
uint8_t sleep_period = 1; // the sleep interval in minutes between readings. Currently set to 10 minutes 


//OTHER SENSOR HOUSEKEEPING
int T_power=6;      //Thermistor power pin
int C_power=5;      //Central GND power pin (for EC and pressure sensor)

/* ALARM SUBROUTINE */
void set_alarm(void) {
  // calculate the minute when the next alarm will be triggered 
  wakeup_min = (t.min / sleep_period + 1) * sleep_period; 
  if (wakeup_min > 59) {
    wakeup_min -= 60;
    }

  // flags define what calendar component to be checked against the current time in order 
  // to trigger the alarm - see datasheet 
  // A1M1 (seconds) (0 to enable, 1 to disable) 
  // A1M2 (minutes) (0 to enable, 1 to disable) 
  // A1M3 Chour) (0 to enable, 1 to disable) 
  // A1M4 (day) (0 to enable, 1 to disable) 
  // DY/DT (dayofweek == 1/dayofmonth == 0)
  uint8_t flags [4] = { 0, 1, 1, 1};

  // set Alarm1 
  DS3234_set_a2(ss, wakeup_min, 0, 0, flags);

  // activate Alarm1 
  DS3234_set_creg(ss, DS3234_INTCN | DS3234_A2IE);
  
}



/* SETUP */
void setup() {
  Serial.begin(9600); 
  myserial.begin(9600);
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3)); 
  DS3234_init(ss, DS3234_INTCN); 
  delay(10); 
  DS3234_get(ss, &t); 
  delay(10); 
  set_alarm(); 
  DS3234_clear_a2f(ss); 
  delay(10);
  pinMode (AlarmPin, INPUT); 
  pinMode (T_power, OUTPUT); 
  pinMode (C_power, OUTPUT);
  attachInterrupt(0, alarm, FALLING); // setting the alarm interrupt
  while (!sd.begin(CS, SPI_HALF_SPEED)) {
    }

  //Serial.println("Card Initialised”); //would print to the Arduino screen if connected
  // open the file for write at end like the Native SD library 
  file.open(newfile, O_WRITE | O_CREAT | O_APPEND); 
  file.close();
  
  delay(1000);
  inputstring.reserve(10); 
  sensorstring.reserve(30);
}



/* LOOP */
void loop() {
  
  gotoSleep();                  //instigates sleep. Wakes up within subroutine
  delay(5);     
  
  digitalWrite(C_power, HIGH);  //turns on the pressure and EC sensor
  delay(500);                   //half second delay for results to stabilize
  int C = analogRead(A2);       //measure EC
  int P = analogRead(A1);       //measure pressure
  
  digitalWrite(C_power, LOW);   //turns off the pressure and EC sensor
  
  digitalWrite(T_power, HIGH);  //turns on the thermistor
  delay(100); 
  int T = analogRead(A0);       //measure temperature
  digitalWrite(T_power, LOW);   //turns off the thermistor
  
  //uncalibrated readings:
  float C_V = C*3.3/1024.0; 
  float T_V = T*3.3/1024.0; 
  float P_V = P*3.3/1024.0; 
  float T_R = 10.0/T_V*(3.3-T_V); 
  
  //calibrated readings
  float T_T = 1/(2.772539*pow(10,-3) + 2.50767*pow(10,-4)*log(T_R) + 3.37884*pow(10,-7)*pow(log(T_R),3))-273.15; //calibrated temperature reading
  float P_P = (P_V-1.58)*10000.0;   //calibrated pressure reading
  float C_C = 6182.0*C_V;           //calibrated conductivity reading
  
  myserial.print('r');              //send the string to the Atlas Scientific product
  myserial.print('\r');             //add a <CR> to the end of the string
  delay(1800);                      //first result flushed. Repeat reading
  myserial.print('r');              //send the string to the Atlas Scientific product
  myserial.print('\n');             //add a <CR> to the end of the string
  delay(1800);
  
  DO_reading();                     //Run Atlas Scientific DO reading subroutine
  delay(50);
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3)); //initiate SPI communication
  delay(20);
  DS3234_get(ss, &t); 
  set_alarm();                      //reset the alarm
  DS3234_clear_a2f(ss);
  
  String dataString = ""; 
  dataString += t.year; 
  dataString += ","; 
  dataString += t.mon;
  dataString += ","; 
  dataString += t.mday; 
  dataString += ","; 
  dataString += t.hour;
  dataString += ","; 
  dataString += t.min;
  dataString += ","; 
  dataString += C_V; 
  dataString += ","; 
  dataString += C_C; 
  dataString += ","; 
  dataString += P_V; 
  dataString += ","; 
  dataString += T_T; 
  dataString += ","; 
  dataString += DO;
  
  file.open(newfile, O_WRITE | O_APPEND); //Opens the file 
  delay(5); 
  file.println(dataString); 
  //prints data string to the file 
  delay(5); 
  file.close(); //closes the file 
  delay(5); 
  Serial.println(dataString); 
  delay(100);
  
  attachInterrupt(0, alarm, FALLING);
  
} //end loop


/* SLEEP SUBROUTINE */
void gotoSleep(void) { 
  byte adcsra = ADCSRA;   //save the ADC Control and Status Register 
  ADCSRA = 0;         //disable the ADC 
  sleep_enable(); 
  power_spi_disable(); 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  cli(); 
  sleep_bod_disable(); 
  sei(); 
  sleep_cpu(); 
  /* wake up here */ 
  sleep_disable(); 
  power_spi_enable(); 
  ADCSRA = adcsra;      //restore ADCSRA
}


/* ALARM SUBROUTINE */
void alarm() {
    detachInterrupt(0);
}


/* ATLAS SCIENTIFIC DO READING SUBROUTINE */
void DO_reading() {
  myserial.print('R');
  myserial.print('\r'); // delay(1800);
  
  while (myserial.available()) {
  char inchar = (char)myserial.read(); 
  //delay(10);
  sensorstring += inchar; 
  
  if (inchar == '\r') { 
    sensor_string_complete = true;
    //Serial.println(sensorstring); //if wanting to print the immediate DO reading
    }
  }

  if (sensor_string_complete== true) {
    if (isdigit(sensorstring[0])) DO = sensorstring.toFloat();
  
    sensorstring = "";
    sensor_string_complete = false;
    myserial.print("Sleep"); 
    myserial.print('\r'); 
    delay(1800);
    }
}
