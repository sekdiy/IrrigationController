/**
 * irrigation controller
 *
 * @author sekdiy
 * @date 25.06.2015
 * @version 4
 */

#include <Streaming.h>                  // http://arduiniana.org/libraries/streaming/
#include <LED.h>                        // http://playground.arduino.cc/Code/LED
#include <DS3232RTC.h>                  // http://github.com/JChristensen/DS3232RTC
#include <Wire.h>                       // http://arduino.cc/en/Reference/Wire
#include <TimeAlarms.h>                 // https://github.com/PaulStoffregen/TimeAlarms
#include <TimerOne.h>                   // https://github.com/PaulStoffregen/TimerOne 
#include <Time.h>                       // https://github.com/PaulStoffregen/Time 
#include <Sensirion.h>                  // http://playground.arduino.cc/Code/Sensirion
#include <Relay.h>                      // Relay library
#include <FlowMeter.h>                  // Flow Meter library
#include <IrrigationZone.h>             // Irrigation Zone library

/**
 * global settings
 */
const float tickPeriod = 1.0f;          // tick period duration in seconds
const int baudRate = 9600;              // serial interface baud rate
const int tomatoValve = 9;
const int strawberryValve = 8;
const int reserved1Valve = 7;
const int reserved2Valve = 6;
const int SensirionClock = 5;
const int SensirionData = 4;

/**
 * loop tick variables
 */
volatile unsigned long tickCount = 0;   // number of ticks since power up
volatile unsigned long tickHistory = 0; // number of ticks until last cycle

/**
 * irrigation zones
 */ 
IrrigationZone Tomatoes = IrrigationZone("Tomatoes", 20000.0f, 5, tomatoValve);              // tomatoes: max. 20l or 2min, valve on pin 9
IrrigationZone Strawberries = IrrigationZone("Strawberries", 20000.0f, 5, strawberryValve);  // strawberries: max. 20l or 2min, valve on pin 8
Sensirion Soil = Sensirion(SensirionData, SensirionClock);                                   // soil temperature and moisture sensor

/*
 * service routines and callbacks
 *
 * @version 3
 */
void timerISR() { tickCount +=tickPeriod; }    // update number of ticks since power up
void flowISR() { Meter.count(); }              // flow meter
void tomatoISR() { Tomatoes.start(); }         // tomatoes
void strawberryISR() { Strawberries.start(); } // strawberries

/*
 * setup
 *
 * @version 2
 */
void setup() {
  /* serial  */
  Serial.begin(baudRate);                    // initialize serial port to default baud rate

  /* flow meter interrupt */
  Meter.attach(flowISR);                     // attach interrupt service routine to flow meter

  /* real time clock */
  setSyncProvider(RTC.get);                  // synchronize to RTC, if available
  if (timeStatus() != timeSet) Serial << F("RTC Sync FAIL!") << endl;

  /* irrigtaion tasks */
  // TODO make *absolutely* sure the tasks never overlap (i.e. by using different, absolute start times and limited durations)
  Alarm.timerRepeat(10, tomatoISR);          // refill tomato barrell every 30 seconds, attach interrupt service routine
  Alarm.timerRepeat(30, strawberryISR);      // refill strawberry barrell every 30 seconds, attach interrupt service routine
//  Alarm.alarmRepeat(20, 57, 30, tomatoISR);          // refill tomato barrell every 30 seconds, attach interrupt service routine
//  Alarm.alarmRepeat(20, 58, 00, strawberryISR);      // refill strawberry barrell every 30 seconds, attach interrupt service routine

  /* timer one */
  Timer1.initialize((long) (tickPeriod * 1000000));    // initialize timer interrupt to tick seconds
  Timer1.attachInterrupt(timerISR);          // attach interrupt service routine timer ome
}

unsigned int soilRawData = 0;
unsigned int soilMode = TEMP;
float soilTemperature = -1000.0f; 
float soilHumidity = -1000.0f; 
float soilDewpoint = -1000.0f; 
boolean soilWaiting = false;

/*
 * loop
 *
 * @version 4
 * @date 25.06.2015
 */
void loop() {

  /* only process once per tick */
  if (tickHistory != tickCount) {

    Alarm.delay(0);                          // run the irrigation scheduler (odd way of saying: TimeAlarms::serviceAlarms())
    DEBUG_LED.toggle();                      // show a pulse, indicating that we might still be alive
  
    Meter.tick(tickPeriod);                  // update flow meter calculations
    Tomatoes.tick(tickPeriod);               // update tomato zone state
    Strawberries.tick(tickPeriod);           // update strawberry zone state

    if (!soilWaiting) {
      Soil.meas(soilMode, &soilRawData, NONBLOCK);
      soilWaiting = true;
    } 
    else if (Soil.measRdy()) {
      switch (soilMode) {
        case TEMP:
            soilTemperature = Soil.calcTemp(soilRawData);
            soilMode = HUMI;
            soilWaiting = false;
          break;
        case HUMI:
            soilHumidity = Soil.calcHumi(soilRawData, soilTemperature);
            soilDewpoint = Soil.calcDewpoint(soilHumidity, soilTemperature);
            soilMode = TEMP;
            soilWaiting = false;
          break;
      }
    }
    
    Serial << "[" << hour() << ":" << minute() << ":" << second() << "] RTC internal Temperature: " << RTC.temperature()/4.0f << "°C, soil temperature: " << soilTemperature << "°C, soil humidity: " << soilHumidity << "%, dew point: " << soilDewpoint << "°C." << endl;
    
    tickHistory = tickCount;                 // mark tick as processed
  }

}

