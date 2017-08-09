/*
  WISOL.h - Interface for Sigfox library.
  Created by Thomas Ho, Thinxtra Solutions Pty.
  January 19, 2017.
  Updated August 9, 2017.
  Released into the public domain.
*/

#ifndef WISOL_h
#define WISOL_h

#include "Arduino.h"
#include "Isigfox.h"

#define RCZ1 1
#define RCZ2 2
#define RCZ3 3
#define RCZ4 4


class WISOL : public Isigfox
{
public:
	WISOL(){}
    ~WISOL(){}
    int initSigfox();
    void configIO(pinIO pin);
	int testComms();
	int sendPayload(uint8_t *outData, const uint8_t len, const int downlink, recvMsg *receivedMsg);
	int sendPayload(uint8_t *outData, const uint8_t len, const int downlink);
	int sendMessage(char *outData, const uint8_t len, recvMsg *receivedMsg);
	int getdownlinkMsg(recvMsg *receivedMsg);
	int getZone();
	int setZone();
	int setPublicKey();
	int setPrivateKey();
	int resetMacroChannel();
private:
	void Buffer_Init();
	int getRecvMsg(recvMsg *receivedMsg, const int downlink);
	int prepareZone();
	recvMsg goDeepSleep();
	void wakeDeepSleep();
	int strCmp(char *in1, char *in2, const int len);
	void printRecv(char* in, const int len);
	void clearBuffer();
	void ASCII2Hex(uint8_t* input, int length, char* buf_str);
	int sendPayloadProcess(uint8_t *outData, const uint8_t len, int downlink, recvMsg *receivedMsg);
	int getdownlinkMsg(int downlink, recvMsg *receivedMsg);
	void checkChangeZone();

	static const int BUFFER_SIZE = 40;
	char master_receive[BUFFER_SIZE] = {0};

	// char* master_receive;

	int currentZone;

	typedef enum{
	   Exit=0,
	   Send_Wakeup,             /*!< Wakes up OL2361 */
	}e_SPI_command_code;
};

#endif
