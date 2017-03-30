/*
  WISOL.h - Interface for Sigfox library.
  Created by Thomas Ho, Thinxtra Solutions Pty.
  January 19, 2017.
  Released into the public domain.
*/

#ifndef WISOL_h
#define WISOL_h

#include "Arduino.h"
#include "Isigfox.h"

#define RCZ1 1
#define RCZ2 2
#define RCZ4 4

class WISOL : public Isigfox
{
public:
	WISOL(){}
    ~WISOL(){}
    int init();
    void configIO(pinIO pin);
	int testComms();
	recvMsg sendPayload(char *outData, int len);
	recvMsg sendMessage(char *outData, int len);
	int getZone();
	int setZone();
	int setPublicKey();
	int setPrivateKey();
	int resetMacroChannel();
private:
	void Buffer_Init();
	recvMsg getRecvMsg();
	recvMsg prepareZone();
	recvMsg goDeepSleep();
	void wakeDeepSleep();
	int strCmp(char *in1, char *in2, int len);
	void printRecv(char* in, int len);
	void clearBuffer();

	static const int BUFFER_SIZE = 0x18;

	char master_send[BUFFER_SIZE]    = {0};
	char master_receive[BUFFER_SIZE] = {0};

	int currentZone = 0;
	typedef enum{
	   Exit=0,
	   Send_Wakeup,             /*!< Wakes up OL2361 */
	}e_SPI_command_code;
};

#endif
