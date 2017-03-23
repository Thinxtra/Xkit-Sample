/*
  Tsensors.cpp - Library for Xkit Sensors Interface.
  Created by Thomas Ho, Thinxtra Solution Pty.
  Febuary 14, 2017.
  
  Using Adafruit_BMP280_Library and Adafruit_MMA8451_Library
  Copyright (c) 2012, Adafruit Industries
  Modified by Thomas Ho
  
  Released into the public domain.
*/

#include "Tsensors.h"
#include "Wire.h"
#include "stdlib.h"

const int reedPin = 3;
const int buttonPin = A1;
const int photoPin = A0;

void (*ButtonService)(void);
void (*ReedService)(void);

// Assign IO for WISOL - Not needed since WISOL share TX, RX with Ardruino
void Tsensors::configIO(){
	int reedPin = 3;
	int buttonPin = A1;
	int photoPin = A0;
}


/*
int init()
	- Output: return 0 if init succesfully. Return 1 if otherwise.
	- Purpose: Init Serial and set UL Frequency to 920.8 MHz
*/
int Tsensors::init(){
	// Config IO
	configIO();

	// Init I2C
	Wire.begin();
	Wire.setClock(100000);
	
	// Init interrupt for REED
    pinMode(reedPin, INPUT_PULLUP);
	
	// Init Button
	pinMode(buttonPin, INPUT_PULLUP);
	
	// Init ADC for photo sensor
	pinMode(photoPin, INPUT);
	
	// Init Accelerometer
	write8(MMA8451_REG_CTRL_REG2, 0x40, accelerometerAddr); // reset
	delay(10);
	while (read8(MMA8451_REG_CTRL_REG2, accelerometerAddr) & 0x40);
	// enable 4G range
	write8(MMA8451_REG_XYZ_DATA_CFG, 0b01, accelerometerAddr);
	// High res
	write8(MMA8451_REG_CTRL_REG2, 0x02, accelerometerAddr);
	// DRDY on INT1
	write8(MMA8451_REG_CTRL_REG4, 0x01, accelerometerAddr);
	write8(MMA8451_REG_CTRL_REG5, 0x01, accelerometerAddr);
	// Turn on orientation config
	write8(MMA8451_REG_PL_CFG, 0x40, accelerometerAddr);
	// Activate at max rate, low noise mode
	write8(MMA8451_REG_CTRL_REG1, 0x01 | 0x04, accelerometerAddr);

}

int Tsensors::getACK(uint8_t addr){
  Wire.beginTransmission(addr);
  return Wire.endTransmission();
}

acceleration_xyz Tsensors::getAccXYZ(){
	int reading;
	acceleration_xyz _acceleration_xyz;
	uint8_t range;
	
	int ack = getACK(accelerometerAddr);
	if (ack==0){
		range = read8(MMA8451_REG_XYZ_DATA_CFG, accelerometerAddr) & 0x03;
				
		Wire.beginTransmission(accelerometerAddr);
		Wire.write(REG_OUT_X_MSB);
		Wire.endTransmission(false);
		
		Wire.requestFrom(accelerometerAddr, 6);
		int x = Wire.read(); x <<= 8; x |= Wire.read(); x >>= 2;
		int y = Wire.read(); y <<= 8; y |= Wire.read(); y >>= 2;
		int z = Wire.read(); z <<= 8; z |= Wire.read(); z >>= 2;
		
		uint16_t divider = 1;
		uint8_t  MMA8451_RANGE_8_G           = 0b10;   // +/- 8g
		uint8_t MMA8451_RANGE_4_G           = 0b01;   // +/- 4g
		uint8_t MMA8451_RANGE_2_G           = 0b00;    // +/- 2g (default value)

		if (range == MMA8451_RANGE_8_G) divider = 1024;
		if (range == MMA8451_RANGE_4_G) divider = 2048;
		if (range == MMA8451_RANGE_2_G) divider = 4096;
		
		_acceleration_xyz.x_g = (float)x / divider;
		_acceleration_xyz.y_g = (float)y / divider;
		_acceleration_xyz.z_g = (float)z / divider;
	}
	return _acceleration_xyz;
}

float Tsensors::getAccX(){
	acceleration_xyz _acceleration_xyz = getAccXYZ();
	return _acceleration_xyz.x_g;
}

float Tsensors::getAccY(){
	acceleration_xyz _acceleration_xyz = getAccXYZ();
	return _acceleration_xyz.y_g;
}

float Tsensors::getAccZ(){
	acceleration_xyz _acceleration_xyz = getAccXYZ();
	return _acceleration_xyz.z_g;
}

float Tsensors::getTemp(){
	int32_t var1, var2;
	float T;

	if ((int)getACK(BMP280Addr)==0){
		readCoefficients();
		write8(BMP280_REGISTER_CONTROL, 0x3F, BMP280Addr);
		  
		int32_t adc_T = read24(BMP280_REGISTER_TEMPDATA, BMP280Addr);
		adc_T >>= 4;

		var1  = ((((adc_T>>3) - ((int32_t)_bmp280_calib.dig_T1 <<1))) *
			((int32_t)_bmp280_calib.dig_T2)) >> 11;

		var2  = (((((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1)) *
			((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1))) >> 12) *
			((int32_t)_bmp280_calib.dig_T3)) >> 14;

		_bmp280_calib.t_fine = var1 + var2;

		T  = (_bmp280_calib.t_fine * 5 + 128) >> 8;
		  
		return T/100;
	} else {
		return 0;
	}
}

float Tsensors::getPressure(){
	int64_t var1, var2, p;

	if ((int)getACK(BMP280Addr)==0){
		// Must be done first to get the t_fine variable set up
		getTemp();

		int32_t adc_P = read24(BMP280_REGISTER_PRESSUREDATA, BMP280Addr);
		adc_P >>= 4;

		var1 = ((int64_t)_bmp280_calib.t_fine) - 128000;
		var2 = var1 * var1 * (int64_t)_bmp280_calib.dig_P6;
		var2 = var2 + ((var1*(int64_t)_bmp280_calib.dig_P5)<<17);
		var2 = var2 + (((int64_t)_bmp280_calib.dig_P4)<<35);
		var1 = ((var1 * var1 * (int64_t)_bmp280_calib.dig_P3)>>8) +
		((var1 * (int64_t)_bmp280_calib.dig_P2)<<12);
		var1 = (((((int64_t)1)<<47)+var1))*((int64_t)_bmp280_calib.dig_P1)>>33;

		if (var1 == 0) {
			return 0;  // avoid exception caused by division by zero
		}
		p = 1048576 - adc_P;
		p = (((p<<31) - var2)*3125) / var1;
		var1 = (((int64_t)_bmp280_calib.dig_P9) * (p>>13) * (p>>13)) >> 25;
		var2 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;

		p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib.dig_P7)<<4);
		
		return (float)p/256;
	} else {
		return 0;
	}
}


float Tsensors::getPhoto(){
  int quantize;
  float val, delta;
  
  quantize = analogRead(photoPin);
  delta = 5.0/1024;
  val = delta*quantize;

  return val;
}


void Tsensors::setButton(void (*service(void))){	
	ButtonService = service;
	InitialiseInterrupt();
}


void Tsensors::InitialiseInterrupt(){
  cli();				// switch interrupts off while messing with their settings  
  PCICR =0x02;          // Enable PCINT1 interrupt
  PCMSK1 = 0b00000111;
  sei();				// turn interrupts back on
}


unsigned long lasttimeButton = 0;
ISR(PCINT1_vect) {    // Interrupt service routine. Every single PCINT8..14 (=ADC0..5) change
            // will generate an interrupt: but this will always be the same interrupt routine
	unsigned long currenttime = millis();
	unsigned long interval = (unsigned long)(currenttime - lasttimeButton);
	if ( (digitalRead(buttonPin)==0) & (interval > 200) ) {
		ButtonService();
	}
	lasttimeButton = currenttime;
}


unsigned long lasttimeReed = 0;

void stableReed(void){
	EIMSK &= 0xFD; // disable interrupt 1
	
	unsigned long currenttime = millis();
	unsigned long interval = (unsigned long)(currenttime - lasttimeReed);
	
	uint8_t reg = (EICRA & 0x0C) >> 2;
	if (reg==0){
		if ( (interval > 1000) ){
			EICRA &= 0xF0; // clear ext interrupt setting
			EICRA |= 0x0C; 
			lasttimeReed = currenttime;
		}
	} else if (reg==3){
		EICRA &= 0xF0; // clear ext interrupt setting
		EICRA |= 0x00;
		ReedService();
	} else {
		Serial.println("Unrecognized interrupt setting."); // unhandled event
	}
	
	EIMSK |= 0x02; // enable interrupt 1
}


void Tsensors::setReed(void (*InterruptService(void))){	
	ReedService = InterruptService;
	attachInterrupt(digitalPinToInterrupt(reedPin), stableReed, LOW);
}


void Tsensors::readCoefficients(void)
{
    _bmp280_calib.dig_T1 = read16_LE(BMP280_REGISTER_DIG_T1, BMP280Addr);
    _bmp280_calib.dig_T2 = readS16_LE(BMP280_REGISTER_DIG_T2, BMP280Addr);
    _bmp280_calib.dig_T3 = readS16_LE(BMP280_REGISTER_DIG_T3, BMP280Addr);

    _bmp280_calib.dig_P1 = read16_LE(BMP280_REGISTER_DIG_P1, BMP280Addr);
    _bmp280_calib.dig_P2 = readS16_LE(BMP280_REGISTER_DIG_P2, BMP280Addr);
    _bmp280_calib.dig_P3 = readS16_LE(BMP280_REGISTER_DIG_P3, BMP280Addr);
    _bmp280_calib.dig_P4 = readS16_LE(BMP280_REGISTER_DIG_P4, BMP280Addr);
    _bmp280_calib.dig_P5 = readS16_LE(BMP280_REGISTER_DIG_P5, BMP280Addr);
    _bmp280_calib.dig_P6 = readS16_LE(BMP280_REGISTER_DIG_P6, BMP280Addr);
    _bmp280_calib.dig_P7 = readS16_LE(BMP280_REGISTER_DIG_P7, BMP280Addr);
    _bmp280_calib.dig_P8 = readS16_LE(BMP280_REGISTER_DIG_P8, BMP280Addr);
    _bmp280_calib.dig_P9 = readS16_LE(BMP280_REGISTER_DIG_P9, BMP280Addr);
}


uint16_t Tsensors::read16(byte reg, uint8_t addr)
{
  uint16_t value;
  uint8_t _i2caddr = addr;
  
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)_i2caddr, (byte)2);
  value = (Wire.read() << 8) | Wire.read();

  return value;
}


uint16_t Tsensors::read16_LE(byte reg, uint8_t addr) {
  uint16_t temp = read16(reg, addr);
  return (temp >> 8) | (temp << 8);
}

int16_t Tsensors::readS16(byte reg, uint8_t addr)
{
  return (int16_t)read16(reg, addr);
}

int16_t Tsensors::readS16_LE(byte reg, uint8_t addr)
{
  return (int16_t)read16_LE(reg, addr);
}


uint32_t Tsensors::read24(byte reg, uint8_t addr)
{
  uint32_t value;
  uint8_t _i2caddr = addr;
  
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)_i2caddr, (byte)3);
  
  value = Wire.read();
  value <<= 8;
  value |= Wire.read();
  value <<= 8;
  value |= Wire.read();

  return value;
}


void Tsensors::write8(byte reg, byte value, uint8_t addr)
{
  uint8_t _i2caddr = addr;
  
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

uint8_t Tsensors::read8(uint8_t reg, uint8_t addr) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false); // MMA8451 + friends uses repeated start!!

    Wire.requestFrom(addr, 1);
    if (! Wire.available()) return -1;
    return (Wire.read());
}


char* Tsensors::Byte2Hex(unsigned char* input, int length){	
	unsigned char *buf = input;
    int i;
    int size = length;
    char* buf_str = (char*) malloc (2*size + 1);
    char* buf_ptr = buf_str;
    for (i = 0; i < size; i++)
    {
        buf_ptr += sprintf(buf_ptr, "%02X", buf[i]);
    }
    sprintf(buf_ptr,"\n");
    *(buf_ptr + 1) = '\0';
	
	return buf_str;
}