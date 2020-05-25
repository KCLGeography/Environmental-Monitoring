// Code developed by Environmental Research Group, Department of Geography, King's College London //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* LIBRARIES */
#include <SPI.h>                  //SPI Communication library
#include <avr/sleep.h>            //low power library
#include <avr/power.h>            //low power library
#include "ds3234.h"               //DS3234 RTC library. Available from https://github.com/rodan/ds3234
#include <SdFat.h>                //SD card library. Available from https://github.com/greiman/SdFat

//SD HOUSEKEEPING
  SdFat sd;
  SdFile file;
  char newfile[] = "BasicTemperatureLogger.csv";      //name of file
  const int CS = 10;        //sd card chip select


//RTC & ALARM HOUSEKEEPING
  int ss = 9;               //clock chip select
  int AlarmPin = 2;         //pin which alarm communicates with
  unsigned char wakeup_min;
  struct ts t;
  uint8_t sleep_period = 1;                 //sleep period. Currently set to every minute.
  volatile boolean extInterrupt1 = false;

unsigned int wADC;

/* ALARM SUBROUTINE */
void set_alarm(void){  
    wakeup_min = (t.min / sleep_period + 1) * sleep_period;
    if (wakeup_min > 59) {
        wakeup_min -= 60;
    }
    uint8_t flags[4] = { 0, 1, 1, 1};   
    DS3234_set_a2(ss, wakeup_min, 0, 0, flags);       //set Alarm1
    DS3234_set_creg(ss, DS3234_INTCN | DS3234_A2IE);  //activate Alarm1
}

/* SETUP */
void setup() {
  Serial.begin(9600);
  Serial.println("setupstart");
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  DS3234_init(ss, DS3234_INTCN);  
  delay(10);
  DS3234_get(ss, &t);
  delay(10);
  set_alarm();
  DS3234_clear_a2f(ss);
  delay(10);
  pinMode (AlarmPin, INPUT);
  attachInterrupt(0, alarm, FALLING); // setting the alarm interrupt 
  delay(10);

  while (!sd.begin(CS, SPI_HALF_SPEED)) {}
  // open the file for write at end like the Native SD library
  file.open(newfile, O_WRITE | O_CREAT | O_APPEND);
  file.close();
  Serial.println("setupend"); //would print to the Arduino screen if connected
  delay(100); 
 } //END SETUP

/* LOOP */
void loop() {
Serial.println("Loop begin -> Going to sleep."); //would print to the Arduino screen if connected
  delay(50);
  gotoSleep();              //Arduino put to sleep
  delay(50);
  detachInterrupt(0);       //turns off alarm
  if (extInterrupt1) extInterrupt1 = false;

  //Measure internal ATmega328p temperature
  GetTemp();
  int InternalTemp = wADC;
  
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  delay(20);
  DS3234_get(ss, &t);
  set_alarm();
  DS3234_clear_a2f(ss);
  float dstemp = (DS3234_get_treg(ss)); //get DS3234 temp

//data written to SD:
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
      dataString += InternalTemp; //ATmega328p uncalibrated temp
      dataString += ",";
      dataString += dstemp;       //DS3234 temp (°C)
      Serial.println(dataString);
      delay(50);


       while (!sd.begin(CS,SPI_HALF_SPEED)) {
        
        } // initialises SD card again - for when sd card is removed for data
       
       file.open(newfile, O_WRITE | O_APPEND); //Opens the file
       delay(5);
       file.println(dataString); //prints data string to the file
       delay(5);
       file.close(); //closes the file
       delay(20);
  attachInterrupt(0, alarm, FALLING); //turns back on alarm
} //End loop


/* SLEEP SUBROUTINE */
void gotoSleep(void){
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

/* ATMEGA328P INTERNAL TEMPERATURE SUBROUTINE */
double GetTemp(void){
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  
  delay(20);          
  ADCSRA |= _BV(ADSC);  
  while (bit_is_set(ADCSRA,ADSC));
  wADC = ADCW;
}
