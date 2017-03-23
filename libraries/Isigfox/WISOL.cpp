/*
  WISOL.cpp - Interface for Sigfox library.
  Created by Thomas Ho, Thinxtra Solutions Pty.
  January 19, 2017.
  Released into the public domain.
*/

#include "Arduino.h"
#include "WISOL.h"


// Assign IO for WISOL - Not needed since WISOL share TX, RX with Ardruino
void WISOL::configIO(pinIO pin){
	
}


/*
int init()
	- Output: return 0 if init succesfully. Return 1 if otherwise.
	- Purpose: Init Serial and set UL Frequency to 920.8 MHz
*/
int WISOL::init(){
	recvMsg RecvMsg;
	
	// Init Serial
	Serial.begin(9600);
	currentZone = getZone();
	
	switch (currentZone){
		case RCZ1:
		{
			Serial.println("RCZ1");
			break;
		}
		case RCZ2:
		{
			Serial.println("RCZ2");
			break;
		}
		case RCZ4:
		{
			Serial.println("RCZ4");
			break;
		}
		default:
		{
			Serial.println("No zone");
			break;
		}
	}
	delay(50); while (Serial.available()){ Serial.read(); delay(10);}
	return 0;
}

int WISOL::getZone(){
	recvMsg RecvMsg;
	RecvMsg = sendMessage("AT$I=7", 6);
		
	if (strCmp(RecvMsg.inData, "FCC", 3)){
		RecvMsg = sendMessage("AT$DR?", 6);
		if (strCmp(RecvMsg.inData, "905200000", 9)){
			return RCZ2;
		} else if (strCmp(RecvMsg.inData, "922300000", 9)) {
			return RCZ4;
		} else {
			return 0;
		}
	} else if (strCmp(RecvMsg.inData, "ETSI", 4)){
		return RCZ1;
	} else {
		return 0;
	}
}


int WISOL::setZone(){
	return -1;
}

void WISOL::printRecv(char *in, int len){
	for (int i=0; i<len; i++){
		Serial.print(in[i]);
	}
	Serial.println("");
	delay(50); while (Serial.available()){ Serial.read(); delay(10);}
}


int WISOL::strCmp(char *in1, char *in2, int len){
	for (int i = 0; i<len; i++){
		if (in1[i]!=in2[i]){
			return 0;
		}
	}
	return 1;
}

/*
int testComms()
 - Send "AT" and wait for "OK" back
 - Return 0 if testing succesfully (receive "1234" back)
 - Return 1 otherwise
*/
int WISOL::testComms(){
	static char testchar[] = "AT";
	recvMsg RecvMsg;
	
	Buffer_Init(); // Prepare buffer
	RecvMsg = sendMessage(testchar, 2); // Send message
		
	// Read ACK
	if (RecvMsg.inData[0]=='O' && RecvMsg.inData[1]=='K'){
		return 0;
	} else {
		return 1;
	}
	
	delay(50); while (Serial.available()){ Serial.read(); delay(10);}
}

/*
recvMsg sendPayload(char *inData, int len){
	- Input: inData is a pointer to the sending payload.
	         len is the length (in bytes) of the sending payload.
	- Output: recvMsg is received message

Important note: inData is a string of hex (i.e., to send a byte of 0xAB, we need inData = {'A', 'B'})
*/
recvMsg WISOL::sendPayload(char *inData, int len){
	static char header[] = "AT$SF=";
	int headerLen = 6;
	int bytelen = len/2;
	recvMsg RecvMsg;
	
	if (bytelen<=12){
		delay(50); while (Serial.available()){ Serial.read(); delay(10);}
		prepareZone();
		Buffer_Init();
		for (int i=0; i<headerLen; i++){
			Serial.print(header[i]); // print header first
		}
		
		for (int i=0; i<len; i++){
			Serial.print(inData[i]); // print payload
		}
		Serial.println('\0'); // send end terminal
		
		RecvMsg = getRecvMsg(); // Read ACK
	} else if ( (len%2) == 1 ){
		Serial.println("Must send bytes.");
	} else {
		Serial.println("Payload length must not be longer than 12 bytes.");
	}
	return RecvMsg;
}

/*
recvMsg sendMessage(char *inData, int len){
	- Input: inData is a pointer to the sending message.
	         len is the length of the sending message.
	- Output: recvMsg is received message
*/
recvMsg WISOL::sendMessage(char *inData, int len){
	recvMsg RecvMsg;
	
	delay(50); while (Serial.available()){ Serial.read(); delay(10);}
	Buffer_Init(); // prepare buffer
	for (int i=0; i<len; i++){
		Serial.print(inData[i]); // send message
	}
	Serial.println('\0'); // send end terminal
	
	RecvMsg = getRecvMsg(); // read ack or return payload
	return RecvMsg;
}

/*  
recvMsg prepareZone()
	- Set the zone to ZC
*/
recvMsg WISOL::prepareZone(){
	switch (currentZone){
		case RCZ1:
		{
			char testchar[] = "AT302=15";
			return sendMessage(testchar, 8);
			break;
		}
		case RCZ2:
		{
			break;
		}
		case RCZ4:
		{
			char testchar[] = "AT$RC";
			return sendMessage(testchar, 5);
			break;
		}
		default:
		{
			break;
		}
	}
}

// prepare buffer
void WISOL::Buffer_Init()
{
  for (int i = 0; i < BUFFER_SIZE; i++) {
    master_send[i] = 0xFF;
    master_receive[i] = 0xFF;
  }
}


/*
recvMsg getRecvMsg()
	- Output: return receive message from WISOL
*/
recvMsg WISOL::getRecvMsg(){
	recvMsg RecvMsg;
	int count = 1;
	
	// Wait for the incomming message
	while (!Serial.available() && count <= 10) {count++; delay(100);}; delay (20); 
	
	// Prepare receive messge format
	RecvMsg.len = Serial.available();
	RecvMsg.inData = master_receive;
	if (RecvMsg.len){
		for (int i=0; i<RecvMsg.len; i++){
			RecvMsg.inData[i] = Serial.read(); // Read receive message
		}
	}
	
	while (Serial.available()){ Serial.read();} // Clear any remain characters
	
	return RecvMsg;	
}

/* Not used */
recvMsg WISOL::goDeepSleep(){
	Serial.println("AT$P=2");
	Serial.print('\0');
}


/* Not used */
void WISOL::wakeDeepSleep(){
	EIMSK &= 0xFD; // disable interrupt 1
	pinMode(3, OUTPUT);
	digitalWrite(3, LOW);
	delay(50);
	digitalWrite(3, HIGH);
	pinMode(3, INPUT_PULLUP);
	EIMSK |= 0x02; // enable interrupt 1
}