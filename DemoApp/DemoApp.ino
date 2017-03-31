/*
  DemoApp.ino - Demo application for Xkit with Arduino board
  Version 1.0
  Created by Thomas Ho, Thinxtra Solution Pty.
  Febuary 14, 2017.
  
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

Isigfox *Isigfox = new WISOL();
Tsensors *tSensors = new Tsensors();
SimpleTimer timer;

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

  // WISOL test
  Isigfox->initSigfox();
  Isigfox->testComms();
  GetDeviceID();
 
  // Init sensors on Thinxtra Module
  tSensors->initSensors();
  tSensors->setReed(reedIR);
  tSensors->setButton(buttonIR); 

  // Init timer to send a SIgfox message every 10 minutes
  unsigned long sendInterval = 600000;
  timer.setInterval(sendInterval, timeIR);
}

void loop() {
  timer.run();
}

void Send_Sensors(){
  UINT16_t tempt, photo, pressure;
  INT16_t x_g, y_g, z_g;
  char sendMsg[20];
  int sendlength;
  char sendstr[2];
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
  
  int payloadSize = 12; //in byte
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
  timer.setTimeout(20, Send_Sensors); // send a Sigfox message after get ou IRS
}

void buttonIR(){
  Serial.println("Button");
  timer.setTimeout(20, Send_Sensors); // send a Sigfox message after get ou IRS
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


void Send_Pload(uint8_t *sendData, int len){
  // No downlink message require
  recvMsg *RecvMsg;
  
  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendPayload(sendData, len, 0, RecvMsg);
  for (int i=0; i<RecvMsg->len; i++){
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
  int headerLen = 6;
  recvMsg *RecvMsg;
  char msg[] = "AT$I=10";
  int msgLen = 7;
  
  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendMessage(msg, msgLen, RecvMsg);

  Serial.print("Device ID: ");
  for (int i=0; i<RecvMsg->len; i++){
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);
}
