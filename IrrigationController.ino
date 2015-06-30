/**
 * irrigation controller
 *
 * @author sekdiy
 * @date 01.07.2015
 * @version 6
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
#include <SensirionSHT.h>               // Sensirion SHT sensor library
#include "IrrigationController.h"       // project configuration...

/*
 * setup
 *
 * @version 2
 */
void setup() {
  /* serial  */
  Serial.begin(baudRate);                    // initialize serial port to default baud rate

  /* real time clock */
  setSyncProvider(RTC.get);                  // synchronize to RTC, if available

  /* timer one */
  Timer1.initialize((long) (tickPeriod * 1000000));    // initialize timer interrupt to tick seconds
  Timer1.attachInterrupt(timerISR);          // attach interrupt service routine timer one

  /* irrigtaion tasks */
  // TODO make *absolutely* sure the tasks never overlap (i.e. by using different, absolute start times and limited durations)
  Alarm.timerRepeat(10, tomatoStart);          // refill tomato barrell every 30 seconds, attach interrupt service routine
  Alarm.timerRepeat(30, strawberryStart);      // refill strawberry barrell every 30 seconds, attach interrupt service routine
//  Alarm.alarmRepeat(20, 57, 30, tomatoStart);          // refill tomato barrell every 30 seconds, attach interrupt service routine
//  Alarm.alarmRepeat(20, 58, 00, strawberryStart);      // refill strawberry barrell every 30 seconds, attach interrupt service routine
}

/*
 * loop
 *
 * @version 5
 * @date 26.06.2015
 */
void loop() {
  Alarm.delay(0);                          // run the irrigation scheduler (official way of saying: TimeAlarmsClass::serviceAlarms())
  
  /* only process once per tick */
  if (tickHistory != tickCount) {

    DEBUG_LED.toggle();                      // show a pulse, indicating that we might still be alive
  
    Meter.tick(tickPeriod);                  // update flow meter calculations
    Soil.tick(tickPeriod);                   // update soil sensor calculations

    Tomatoes.tick(tickPeriod);               // update tomato zone state
    Strawberries.tick(tickPeriod);           // update strawberry zone state

    Serial << "[" << hour() << ":" << minute() << ":" << second() << "] RTC internal Temperature: " << RTC.temperature()/4.0f << "°C";
    if (!Soil.isOutdated()) {
      if (Soil.hasTemperature()) {
        Serial << ", soil temperature: " << Soil.getTemperature() << "°C";
      }
      if (Soil.hasHumidity()) {
        Serial << ", soil humidity: " << Soil.getHumidity() << "%";
      }
      if (Soil.hasDewpoint()) {
        Serial << ", dew point: " << Soil.getDewpoint() << "°C";
      }
    }
    Serial << "." << endl;
    
    tickHistory = tickCount;                 // mark tick as processed
  }

}

