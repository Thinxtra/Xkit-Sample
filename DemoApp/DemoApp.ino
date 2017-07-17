/*
  DemoApp.ino - Demo application for Xkit with Arduino board
  Version 1.0
  Created by Thomas Ho, Thinxtra Solution Pty.
  Febuary 14, 2017.

  Last update on July 17, 2017

  Using Adafruit_BMP280_Library and Adafruit_MMA8451_Library
  Copyright (c) 2012, Adafruit Industries
  Modified by Thomas Ho
  Using SimpleTimer library (Andrew Mascolo (HazardsMind))

  Released into the public domain.
*/


#include <WISOL.h>
#include <Tsensors.h>
#include <Wire.h>
#include <math.h>
#include <SimpleTimer.h>
#include <avr/wdt.h>

Isigfox *Isigfox = new WISOL();
Tsensors *tSensors = new Tsensors();
SimpleTimer timer;
int watchdogCounter;
uint8_t buttonCounter;
uint8_t PublicModeSF;
uint8_t stateLED;
uint8_t ledCounter;
const uint8_t buttonPin = A1;
const int redLED = 6;

typedef union{
    float number;
    uint8_t bytes[4];
} FLOATUNION_t;

typedef union{
    uint16_t number;
    uint8_t bytes[2];
} UINT16_t;

typedef union{
    int16_t number;
    uint8_t bytes[2];
} INT16_t;

void setup() {
  Wire.begin();
  Wire.setClock(100000);

  Serial.begin(9600);

  // Init watchdog timer
  watchdogSetup();
  watchdogCounter = 0;
  
  // WISOL test
  PublicModeSF = 0;
  Isigfox->initSigfox();
  Isigfox->testComms();
  GetDeviceID();
  //Isigfox->setPublicKey(); // set public key for usage with SNEK

  // Init sensors on Thinxtra Module
  tSensors->initSensors();
  tSensors->setReed(reedIR);
  buttonCounter = 0;
  tSensors->setButton(buttonIR);

  // Init LED
  stateLED = 0;
  ledCounter = 0;
//  pinMode(redLED, INPUT);

  // Init timer to send a Sigfox message every 10 minutes
  unsigned long sendInterval = 600000;
  timer.setInterval(sendInterval, timeIR);
}

void loop() {
  timer.run();
  wdt_reset();
  watchdogCounter = 0;
}

void Send_Sensors(){
  UINT16_t tempt, photo, pressure;
  INT16_t x_g, y_g, z_g;
  acceleration_xyz *xyz_g;
  FLOATUNION_t a_g;

  // Sending a float requires at least 4 bytes
  // In this demo, the measure values (temperature, pressure, sensor) are scaled to ranged from 0-65535.
  // Thus they can be stored in 2 bytes
  tempt.number = (uint16_t) (tSensors->getTemp() * 100);
  Serial.print("Temp: "); Serial.println((float)tempt.number/100);
  pressure.number =(uint16_t) (tSensors->getPressure()/3);
  Serial.print("Pressure: "); Serial.println((float)pressure.number*3);
  photo.number = (uint16_t) (tSensors->getPhoto() * 1000);
  Serial.print("Photo: "); Serial.println((float)photo.number/1000);

  xyz_g = (acceleration_xyz *)malloc(sizeof(acceleration_xyz));
  tSensors->getAccXYZ(xyz_g);
  x_g.number = (int16_t) (xyz_g->x_g * 250);
  y_g.number = (int16_t) (xyz_g->y_g * 250);
  z_g.number = (int16_t) (xyz_g->z_g * 250);
  Serial.print("Acc X: "); Serial.println((float)x_g.number/250);
  Serial.print("Acc Y: "); Serial.println((float)y_g.number/250);
  Serial.print("Acc Z: "); Serial.println((float)z_g.number/250);
  free(xyz_g);

  const uint8_t payloadSize = 12; //in bytes
//  byte* buf_str = (byte*) malloc (payloadSize);
  uint8_t buf_str[payloadSize];

  buf_str[0] = tempt.bytes[0];
  buf_str[1] = tempt.bytes[1];
  buf_str[2] = pressure.bytes[0];
  buf_str[3] = pressure.bytes[1];
  buf_str[4] = photo.bytes[0];
  buf_str[5] = photo.bytes[1];
  buf_str[6] = x_g.bytes[0];
  buf_str[7] = x_g.bytes[1];
  buf_str[8] = y_g.bytes[0];
  buf_str[9] = y_g.bytes[1];
  buf_str[10] = z_g.bytes[0];
  buf_str[11] = z_g.bytes[1];

  Send_Pload(buf_str, payloadSize);
//  free(buf_str);
}

void reedIR(){
  Serial.println("Reed");
  timer.setTimeout(50, Send_Sensors); // send a Sigfox message after get ou IRS
}

void buttonIR(){
  buttonCounter = buttonCounter + 1;
  if (buttonCounter==1) {
    timer.setTimeout(2000, checkDoubleClick); // check double click after 2s
  }
}

void checkDoubleClick() {
  if (buttonCounter==1) {
    Serial.println("Single Press");
    Send_Sensors();
  } else {
    Serial.print("Double Press - ");
    BlinkLED();
    pinMode(redLED, OUTPUT);
    if (PublicModeSF == 0) {
      Serial.println("Set public key");
      Isigfox->setPublicKey();
      PublicModeSF = 1;
  
    } else {
      Serial.println("Set private key");
      Isigfox->setPrivateKey();
      PublicModeSF = 0;
    }
  }
  buttonCounter = 0;
}


void BlinkLED() {
  ledCounter++;
  if (ledCounter<=6) {
    if (stateLED == 0){
      digitalWrite(redLED, HIGH);
      stateLED = 1;
      timer.setTimeout(200, BlinkLED);
    } else {
      digitalWrite(redLED, LOW);
      stateLED = 0;
      timer.setTimeout(200, BlinkLED);
    }
  } else {
    pinMode(redLED, INPUT);
    ledCounter = 0;
  }
  
  
}

void timeIR(){
  Serial.println("Time");
  Send_Sensors();
}

void getDLMsg(){
  recvMsg *RecvMsg;
  int result;

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  result = Isigfox->getdownlinkMsg(RecvMsg);
  for (int i=0; i<RecvMsg->len; i++){
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);
}


void Send_Pload(uint8_t *sendData, const uint8_t len){
  // No downlink message require
  recvMsg *RecvMsg;

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendPayload(sendData, len, 0, RecvMsg);
  for (int i = 0; i < RecvMsg->len; i++) {
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);


  // If want to get blocking downlink message, use the folling block instead
  /*
  recvMsg *RecvMsg;

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendPayload(sendData, len, 1, RecvMsg);
  for (int i=0; i<RecvMsg->len; i++){
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);
  */

  // If want to get non-blocking downlink message, use the folling block instead
  /*
  Isigfox->sendPayload(sendData, len, 1);
  timer.setTimeout(46000, getDLMsg);
  */
}


void GetDeviceID(){
  recvMsg *RecvMsg;
  const char msg[] = "AT$I=10";

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendMessage(msg, 7, RecvMsg);

  Serial.print("Device ID: ");
  for (int i=0; i<RecvMsg->len; i++){
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);
}


void watchdogSetup(void) { // Enable watchdog timer
  cli();  // disable all interrupts
  wdt_reset(); // reset the WDT timer
  /*
   WDTCSR configuration:
   WDIE = 1: Interrupt Enable
   WDE = 1 :Reset Enable
   WDP3 = 1 :For 8000ms Time-out
   WDP2 = 1 :For 8000ms Time-out
   WDP1 = 1 :For 8000ms Time-out
   WDP0 = 1 :For 8000ms Time-out
  */
  // Enter Watchdog Configuration mode:
  // IF | IE | P3 | CE | E | P2 | P1 | P0
  WDTCSR |= B00011000;
  WDTCSR = B01110001;
//  WDTCSR |= (1<<WDCE) | (1<<WDE);
//  // Set Watchdog settings:
//   WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0);
  sei();
}


void watchdog_disable() { // Disable watchdog timer
  cli();  // disable all interrupts
  WDTCSR |= B00011000;
  WDTCSR = B00110001;
  sei();
}


ISR(WDT_vect) // Watchdog timer interrupt.
{
// Include your code here - be careful not to use functions they may cause the interrupt to hang and
// prevent a reset.
  Serial.print("WD reset: ");
  Serial.println(watchdogCounter);
  watchdogCounter++;
  if (watchdogCounter == 20) { // reset CPU after about 180 s
      // Reset the CPU next time
      // Enable WD reset
      cli();  // disable all interrupts
      WDTCSR |= B00011000;
      WDTCSR = B01111001;
      sei();
      wdt_reset();
  } else if (watchdogCounter < 8) {
    wdt_reset();
  }
}
