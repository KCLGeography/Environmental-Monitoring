// Code developed by Environmental Research Group, Department of Geography, King's College London //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* LIBRARIES */
#include <SPI.h>                  //SPI Communication library
#include <Wire.h>                  //I2C Communication library
#include <avr/sleep.h>            //low power library
#include <avr/power.h>            //low power library
#include <SoftwareSerial.h>       //Software Serial Communication library (for PM sensor)
#include "ds3234.h"               //DS3234 RTC library. Available from https://github.com/rodan/ds3234
#include <SdFat.h>                //SD card library. Available from https://github.com/greiman/SdFat
#include <Adafruit_Sensor.h>      //Adafruit Sensor library (prerequisite for BME280). Available from https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h>      //Adafruit BME280 library. Available from https://github.com/adafruit/Adafruit_BME280_Library

//SD CARD HOUSEKEEPING
const int CS = 10;            //Chip Select pin for SD card
SdFat sd;
SdFile file;
char newfile[] = "Plantower.csv";     //Name of file to be written to microSD

//RTC & ALARM HOUSEKEEPING
int AlarmPin = 2;
const int ss = 9;                          //Chip Select pin for RTC
struct ts t;
unsigned char wakeup_min;
uint8_t sleep_period = 1;                  // the sleep interval in minutes between readings. Currently set to 1 minutes.
volatile boolean extInterrupt1 = false;    //external interrupt flag1 

//PM SENSOR HOUSEKEEPING
int power=8;
SoftwareSerial PMSerial(7, 6); // RX, TX
int PM01Value; //PM1.0
int PM2_5Value; //PM2.5
int PM10Value; //PM10
#define LENG 31 //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
int i=0;

//BME280 SENSOR HOUSEKEEPING
Adafruit_BME280 bme;

/* ALARM SUBROUTINE */
void set_alarm(void)
{  
    // calculate the minute when the next alarm will be triggered
    wakeup_min = (t.min / sleep_period + 1) * sleep_period;
    if (wakeup_min > 59) {
        wakeup_min -= 60;
    }

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm - see datasheet
    // A1M1 (seconds) (0 to enable, 1 to disable)
    // A1M2 (minutes) (0 to enable, 1 to disable)
    // A1M3 (hour)    (0 to enable, 1 to disable) 
    // A1M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[4] = { 0, 1, 1, 1};

    // set Alarm1
    DS3234_set_a2(ss, wakeup_min, 0, 0, flags);

    // activate Alarm1
    DS3234_set_creg(ss, DS3234_INTCN | DS3234_A2IE);
}


/* SETUP */
void setup() {
  Serial.begin(9600);

  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  
  DS3234_init(ss, DS3234_INTCN);  
  delay(10);
  DS3234_get(ss, &t);
  delay(10);
  set_alarm();
  DS3234_clear_a2f(ss);
  delay(10);
  pinMode (AlarmPin, INPUT);
  
  if (!bme.begin()) {
    Serial.println("BME280 issue");
    while (5);
  }
  

  PMSerial.begin(9600);
  PMSerial.setTimeout(1500);
  pinMode(power,OUTPUT);
  
  attachInterrupt(0, alarm, FALLING); // setting the alarm interrupt 
  
  delay(10);

 while (!sd.begin(CS, SPI_HALF_SPEED)) {
   
  }
  
  // open the file for write at end like the Native SD library
  file.open(newfile, O_WRITE | O_CREAT | O_APPEND);
  file.close();

 delay(100); 
 }


/* LOOP */
void loop() {

  gotoSleep();
  detachInterrupt(0);
  delay(5);
  if (extInterrupt1) extInterrupt1 = false;

  //Start PM sensor 
  
  digitalWrite(power,HIGH);
  delay(10000);                           //wait 10 seconds whilst fan starts and results stabilize
  //Receive results from PM sensor:
  for (i=1;i<50;i++){
    delay(5);
    if(PMSerial.find(0x42)){              //start to read when detect 0x42 (dust sensor code start)
      PMSerial.readBytes(buf,LENG);
      if(buf[0] == 0x4d){
        if(checkValue(buf,LENG)){
          PM01Value=transmitPM01(buf);    //receive PM1.0 value
          PM2_5Value=transmitPM2_5(buf);  //receive PM2.5 value
          PM10Value=transmitPM10(buf);    //receive PM10 value
          break;
          }
        }
      }
    }
  digitalWrite(power,LOW); //turn off PM sensor
  

  //Read the time and set new alarm
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  delay(20);
  DS3234_get(ss, &t);
  set_alarm();
  DS3234_clear_a2f(ss);
  delay(10);

  //prepare data to be written to SD card
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
      dataString += bme.readTemperature();
      dataString += ",";
      dataString += bme.readHumidity();
      dataString += ",";
      dataString += PM10Value;
      dataString += ",";
      dataString += PM2_5Value;
      dataString += ",";
      dataString += PM01Value;


       while (!sd.begin(CS,SPI_HALF_SPEED)) {
        
        } // initialises SD card again - for when sd card is removed for data
       
       file.open(newfile, O_WRITE | O_APPEND); //Opens the file
       delay(5);
       file.println(dataString); //prints data string to the file
       delay(5);
       file.close(); //closes the file
       delay(5);

       Serial.println(dataString);
       delay(100);
    

  delay(20);

  attachInterrupt(0, alarm, FALLING);

} //end loop

/* SLEEP SUBROUTINE */
void gotoSleep(void)
{
   byte adcsra = ADCSRA;          //save the ADC Control and Status Register A
   ADCSRA = 0;  //disable the ADC
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
   ADCSRA = adcsra;               //restore ADCSRA
}

/* ALARM SUBROUTINE */
void alarm(){
  extInterrupt1 = true;
}

/* CHECK PMS-5003 STARTING VALUE SUBROUTINE */
char checkValue(unsigned char *thebuf, char leng){
  char receiveflag=0;
  int receiveSum=0;
  for(int i=0; i<(leng-2); i++){
    receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

/*TRANSMIT PM1 VALUE SUBROUTINE */
int transmitPM01(unsigned char *thebuf){
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

/*TRANSMIT PM2.5 VALUE SUBROUTINE */
int transmitPM2_5(unsigned char *thebuf){
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
}

/*TRANSMIT PM10 VALUE SUBROUTINE */
int transmitPM10(unsigned char *thebuf){
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}
