// Code developed by Environmental Research Group, Department of Geography, King's College London //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* LIBRARIES */
#include <SPI.h>                  //SPI Communication library
#include <avr/sleep.h>            //low power library
#include <avr/power.h>            //low power library
#include "ds3234.h"               //DS3234 RTC library. Available from https://github.com/rodan/ds3234
#include <SdFat.h>                //SD card library. Available from https://github.com/greiman/SdFat
#include <Stepper.h>              //Stepper motor library. Installable in Arduino IDE via Sketch>Include Library>Manage Libraries>Search for "Stepper". Version used: 1.1.3

//SD HOUSEKEEPING
  SdFat sd;
  SdFile file;
  char newfile[] = "LakeTemplogger.csv";      //name of file
  const int CS = 10;        //sd card chip select

//RTC & ALARM HOUSEKEEPING
  int ss = 9;               //clock chip select
  int AlarmPin = 2;         //pin which alarm communicates with
  unsigned int wADC;
  // time when to wake up
  //uint8_t wake_DAY = 1;
  uint8_t wake_HOUR = 12;
  uint8_t wake_MINUTE = 0;
  uint8_t wake_SECOND = 0;
  struct ts t;

//STEPPER MOTOR HOUSEKEEPING
  //---( Number of steps per revolution of INTERNAL motor in 4-step mode )---
  #define STEPS_PER_MOTOR_REVOLUTION 32   
  
  //---( Steps per OUTPUT SHAFT of gear reduction )---
  #define STEPS_PER_OUTPUT_REVOLUTION 32 * 64  //2048  
  
  //The pin connections need to be 4 pins connected
  // to Motor Driver In1, In2, In3, In4  and then the pins entered
  // here in the sequence 1-3-2-4 for proper sequencing
  Stepper small_stepper(STEPS_PER_MOTOR_REVOLUTION, 3, 5, 4, 6);// 3, 5, 4, 6
  
  /*-----( Declare Variables )-----*/
  int  Steps2Take;
  const int relay_on = 7;
  const int relay_off = 8;


/* ALARM SUBROUTINE. Note, this uncommented version is for 24 testing */
void set_alarm(void){
    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm - see datasheet
    // A1M1 (seconds) (0 to enable, 1 to disable)
    // A1M2 (minutes) (0 to enable, 1 to disable)
    // A1M3 (hour)    (0 to enable, 1 to disable) 
    // A1M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[5] = { 0, 0, 0, 1, 1 };

    // set Alarm1
    DS3234_set_a1(ss, wake_SECOND, 0, 0, 0, flags);

    // activate Alarm1
    DS3234_set_creg(ss, DS3234_INTCN | DS3234_A1IE);
}


/* Uncomment out this subroutine if wanting the sediment collector to expose a new trap every month 
void set_alarm(void){
    uint8_t wake_DAY = 1;
    uint8_t wake_HOUR = 12;
    uint8_t wake_MINUTE = 0;
    uint8_t wake_SECOND = 0;
    uint8_t flags[5] = { 0, 0, 0, 0, 0 };
    DS3234_set_a1(ss, wake_SECOND, wake_MINUTE, wake_HOUR, wake_DAY, flags);
    DS3234_set_creg(ss, DS3234_INTCN | DS3234_A1IE);
}
*/
 
void setup() {
Serial.begin(9600);
//  Serial.println("setupstart");
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  DS3234_init(ss, DS3234_INTCN);  
  delay(10);
  DS3234_get(ss, &t);
  delay(10);
  DS3234_clear_a1f(ss);
  delay(10);
  set_alarm();
  delay(5);
  pinMode (AlarmPin, INPUT);
  pinMode(relay_on, OUTPUT);
  pinMode(relay_off, OUTPUT);
  
  digitalWrite(relay_off, HIGH);
  delay(50);
  digitalWrite(relay_off, LOW);

  
  attachInterrupt(0, alarm, FALLING); // setting the alarm interrupt 
  delay(100);

 while (!sd.begin(CS, SPI_HALF_SPEED)) {}
  // open the file for write at end like the Native SD library
  file.open(newfile, O_WRITE | O_CREAT | O_APPEND);
  file.close();
Serial.println("setupend"); //would print to the Arduino screen if connected
 delay(100); 
 }


void loop() {
  gotoSleep();              //Arduino put to sleep
  
  Serial.println("loop"); //would print to the Arduino screen if connected
  delay(50);
  GetTemp();
  int InternalTemp = wADC;
//Arduino wakes up here

  digitalWrite(relay_on, HIGH);
  delay(50);
  digitalWrite(relay_on, LOW);
  delay(1000);
  Serial.println("ON");
  
  Steps2Take  =  STEPS_PER_OUTPUT_REVOLUTION / 12;  // Rotate CW 1/12 turn
  small_stepper.setSpeed(100);   
  small_stepper.step(Steps2Take);
  delay(10);
  
  digitalWrite(relay_off, HIGH);
  delay(50);
  digitalWrite(relay_off, LOW);
  Serial.println("OFF");
  delay(10);

  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  delay(20);
  DS3234_get(ss, &t);
  DS3234_clear_a1f(ss);
  float dstemp = (DS3234_get_treg(ss));

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
      dataString += dstemp;
      delay(50);


       while (!sd.begin(CS,SPI_HALF_SPEED)) {
        
        } // initialises SD card again - for when sd card is removed for data
       
       file.open(newfile, O_WRITE | O_APPEND); //Opens the file
       delay(5);
       file.println(dataString); //prints data string to the file
       delay(5);
       Serial.println(dataString);
       delay(100);
       file.close(); //closes the file
       delay(20);
      attachInterrupt(0, alarm, FALLING); //turns back on alarm
}


//SLEEP LOOP
void gotoSleep(void)
{
   byte adcsra = ADCSRA;          //save the ADC Control and Status Register A
   ADCSRA = 0;  //disable the ADC
   sleep_enable();
   power_spi_disable(); 
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   cli();
   //sleep_bod_disable();
   sei();
   sleep_cpu();
   /* wake up here */
   sleep_disable();
   power_spi_enable(); 
   ADCSRA = adcsra;               //restore ADCSRA
}


//ALARM LOOP
void alarm()
{
  detachInterrupt(0);
}
double GetTemp(void)
{
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  
  delay(20);          
  ADCSRA |= _BV(ADSC);  
  while (bit_is_set(ADCSRA,ADSC));
  wADC = ADCW;
}
