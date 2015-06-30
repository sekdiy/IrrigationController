/**
 * irrigation controller
 *
 * configuration file
 *
 * @author sekdiy
 * @date 26.06.2015
 * @version 1
 */

 /**
  * global settings
  */
const float tickPeriod = 1.0f;          // tick period duration in seconds
const int baudRate = 9600;              // serial interface baud rate
const int tomatoValve = 9;
const int strawberryValve = 8;
const int reserved1Valve = 7;
const int reserved2Valve = 6;
const int shtData = 4;
const int shtClock = 5;

/**
 * schedule tick variables
 */
volatile float tickCount = 0;   // number of ticks since power up
volatile float tickHistory = 0; // number of ticks until last cycle

/**
 * irrigation objects
 */ 
IrrigationZone Tomatoes = IrrigationZone("Tomatoes", 20000.0f, 5, tomatoValve);              // tomatoes: max. 20l or 2min, valve on pin 9
IrrigationZone Strawberries = IrrigationZone("Strawberries", 20000.0f, 5, strawberryValve);  // strawberries: max. 20l or 2min, valve on pin 8
SensirionSHT Soil = SensirionSHT(shtData, shtClock);                                       // soil temperature and moisture sensor

/*
 * service routines and callbacks
 *
 * @version 4
 */
void timerISR() { tickCount +=tickPeriod; }         // update number of ticks since power up
void tomatoStart() { Tomatoes.start(); }            // tomatoes
void strawberryStart() { Strawberries.start(); }    // strawberries

