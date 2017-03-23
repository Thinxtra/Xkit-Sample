/*
  Tsensors.h - Library for Xkit Sensors Interface.
  Created by Thomas Ho, Thinxtra Solution Pty.
  Febuary 14, 2017.
  
  Copyright (c) 2012, Adafruit Industries - Adafruit_MMA8451_Library
  Modified by Thomas Ho
  
  Released into the public domain.
*/

#ifndef Tsensors_h
#define Tsensors_h
#include "Arduino.h"

/** Structure of IO pins to be set for the communication with Sigfox module
	@param x_g	the type of Sigfox module
	@param y_g		the IO pin of Arduino for MISO SPI
	@param x_g		the IO pin of Arduino for MOSI SPI
*/
typedef struct {
	float x_g=0;	/*!< The acceleration on the X axis */
	float y_g=0;	/*!< The acceleration on the Y axis */
	float z_g=0;	/*!< The acceleration on the Z axis */
	float a_g=0;	/*!< The 3D magnitude of the acceleration */
} acceleration_xyz;
	
/** Class for Xkit sensors*/
class Tsensors{
public:
	Tsensors(){}
    ~Tsensors(){}
	
	/** Initialization process for sensors on Xkit
		@return								0 if successful, -1 if failed
	*/
    int init();
	
	/** Get the current ambient temperature
		@return								a float value of the current ambient temperature
	*/
	float getTemp();
	
	/** Get the current ambient pressure
		@return								a float value of the current ambient pressure
	*/
	float getPressure();
	
	/** Get the current ambient light
		@return								a float value of the current output voltage from the photovoltaic (PNJ4K01F)
	*/
	float getPhoto();
	
	/** Get the current acceleration of the X axis
		@return								a float value of the current acceleration of the X axis
	*/
	float getAccX();
	
	/** Get the current acceleration of the Y axis
		@return								a float value of the current acceleration of the Y axis
	*/
	float getAccY();
	
	/** Get the current acceleration of the Z axis
		@return								a float value of the current acceleration of the Z axis
	*/
	float getAccZ();
	
	/** Get the current acceleration on all axis
		@return								a structured value of the current acceleration on all axes
	*/
	acceleration_xyz getAccXYZ();
	
	/** Set a callback which is triggered on a button press
		@param *service(void)				a function pointer which points to the desired callback
	*/
	void setButton(void (*service(void)));
	
	/** Set a callback which is triggered when a magnet is close to the reed switch
		@param *InterruptService(void)		a function pointer which points to the desired callback
	*/
	void  setReed(void (*InterruptService(void)));
	
	/** Convert an array of bytes to an array of hex character
		@param *input						a pointer to the array of byte to convert
		@param length						length of the array of byte to convert
	*/
	char* Byte2Hex(unsigned char* input, int length);
	
private:		
	const int REG_OUT_X_MSB = 0x01;
	const int REG_OUT_Y_MSB = 0x03;
	const int REG_OUT_Z_MSB = 0x05;
	const int UINT14_MAX    = 16383;
	const int accelerometerAddr = 0x1C;
	const int BMP280Addr        = 0x76;
	
	
	void checkButton(void);
	void InitialiseInterrupt(void);
	
	
	void configIO();
	void readCoefficients();
	uint16_t read16(byte reg, uint8_t addr);
	uint16_t read16_LE(byte reg, uint8_t addr);
	int16_t readS16(byte reg, uint8_t addr);
	int16_t readS16_LE(byte reg, uint8_t addr);
	uint32_t read24(byte reg, uint8_t addr);
	void write8(byte reg, byte value, uint8_t addr);
	uint8_t read8(uint8_t reg, uint8_t addr); // MMA8451 + friends uses repeated start!!
	int Tsensors::getACK(uint8_t addr);
	
	enum
    {
      BMP280_REGISTER_DIG_T1              = 0x88,
      BMP280_REGISTER_DIG_T2              = 0x8A,
      BMP280_REGISTER_DIG_T3              = 0x8C,

      BMP280_REGISTER_DIG_P1              = 0x8E,
      BMP280_REGISTER_DIG_P2              = 0x90,
      BMP280_REGISTER_DIG_P3              = 0x92,
      BMP280_REGISTER_DIG_P4              = 0x94,
      BMP280_REGISTER_DIG_P5              = 0x96,
      BMP280_REGISTER_DIG_P6              = 0x98,
      BMP280_REGISTER_DIG_P7              = 0x9A,
      BMP280_REGISTER_DIG_P8              = 0x9C,
      BMP280_REGISTER_DIG_P9              = 0x9E,

      BMP280_REGISTER_CHIPID             = 0xD0,
      BMP280_REGISTER_VERSION            = 0xD1,
      BMP280_REGISTER_SOFTRESET          = 0xE0,

      BMP280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

      BMP280_REGISTER_CONTROL            = 0xF4,
      BMP280_REGISTER_CONFIG             = 0xF5,
      BMP280_REGISTER_PRESSUREDATA       = 0xF7,
      BMP280_REGISTER_TEMPDATA           = 0xFA,
    };
	
	enum{
		MMA8451_REG_OUT_X_MSB     = 0x01,
		MMA8451_REG_SYSMOD        = 0x0B,
		MMA8451_REG_WHOAMI        = 0x0D,
		MMA8451_REG_XYZ_DATA_CFG  = 0x0E,
		MMA8451_REG_PL_STATUS     = 0x10,
		MMA8451_REG_PL_CFG        = 0x11,
		MMA8451_REG_CTRL_REG1     = 0x2A,
		MMA8451_REG_CTRL_REG2     = 0x2B,
		MMA8451_REG_CTRL_REG4     = 0x2D,
		MMA8451_REG_CTRL_REG5     = 0x2E,
	}; // ada_fruit_lib
	
	typedef struct
	{
	  uint16_t dig_T1;
	  int16_t  dig_T2;
	  int16_t  dig_T3;

	  uint16_t dig_P1;
	  int16_t  dig_P2;
	  int16_t  dig_P3;
	  int16_t  dig_P4;
	  int16_t  dig_P5;
	  int16_t  dig_P6;
	  int16_t  dig_P7;
	  int16_t  dig_P8;
	  int16_t  dig_P9;

	  uint8_t  dig_H1;
	  int16_t  dig_H2;
	  uint8_t  dig_H3;
	  int16_t  dig_H4;
	  int16_t  dig_H5;
	  int8_t   dig_H6;
	  int32_t  t_fine;
	} bmp280_calib_data;
	bmp280_calib_data _bmp280_calib;
	
};



#endif