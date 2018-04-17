/*
  WISOL.cpp - Interface for Sigfox library.
  Created by Thomas Ho, Thinxtra Solutions Pty.
  July 17, 2017.
  Released into the public domain.
*/

#include "Arduino.h"
#include "WISOL.h"


// Assign IO for WISOL - Not needed since WISOL share TX, RX with Ardruino
void WISOL::configIO(pinIO pin){

}


/*
int init()
	- Output: return 0 if init succesfully. Return -1 otherwise.
	- Purpose: Init Serial and set UL Frequency to 920.8 MHz
*/
int WISOL::initSigfox(){
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
		case RCZ3:
		{
			Serial.println("RCZ3");
			break;
		}
		default:
		{
			Serial.println("No zone");
			clearBuffer();
			return -1;
		}
	}
	clearBuffer();
	return 0;
}

int WISOL::setPublicKey() {
	recvMsg *receivedMsg;
	int receivedResult;
	int ret = 0;

	receivedMsg = (recvMsg *)malloc(sizeof(recvMsg));
	receivedResult = sendMessage("ATS410=1", 8, receivedMsg);

	if (receivedResult == -1){
		Serial.println("Cannot set public key.");
		clearBuffer();
		ret = 1;
	} else {
		for (int i=0; i<receivedMsg->len; i++){
			Serial.print(receivedMsg->inData[i]);
		}
		Serial.println("");
	}

	free(receivedMsg);
	return ret;
}

int WISOL::setPrivateKey() {
	recvMsg *receivedMsg;
	int receivedResult;
	int ret = 0;

	receivedMsg = (recvMsg *)malloc(sizeof(recvMsg));
	receivedResult = sendMessage("ATS410=0", 8, receivedMsg);

	if (receivedResult == -1){
		Serial.println("Cannot set private key.");
		clearBuffer();
		ret = 1;
	} else {
		for (int i=0; i<receivedMsg->len; i++){
			Serial.print(receivedMsg->inData[i]);
		}
		Serial.println("");
	}

	free(receivedMsg);
	return ret;
}

int WISOL::resetMacroChannel() {
	recvMsg *receivedMsg;
	int receivedResult;
	int ret = 0;

	receivedMsg = (recvMsg *)malloc(sizeof(recvMsg));
	receivedResult = sendMessage("AT$RC", 5, receivedMsg);
	if (receivedResult == -1){
		Serial.println("Cannot reset Macro Channel.");
		clearBuffer();
		ret = 1;
	}

	free(receivedMsg);
	return ret;
}

int WISOL::getZone(){
	recvMsg *receivedMsg;
	int receivedResult;
	int ret = 0;
	
	receivedMsg = (recvMsg *)malloc(sizeof(recvMsg));
	
	receivedResult = sendMessage("AT$DR?", 6, receivedMsg);

	if (strCmp(receivedMsg->inData, "905200000", 9)) {
		ret = RCZ2;
	} else if (strCmp(receivedMsg->inData, "922300000", 9)) {
		ret = RCZ4;
	} else if (strCmp(receivedMsg->inData, "869525000", 9) || strCmp(receivedMsg->inData, "0869525000", 10)) {
		ret = RCZ1;
	} else if (strCmp(receivedMsg->inData, "922200000", 9)) {
		ret = RCZ3;
	} else {
		ret = 0;
	}

	free(receivedMsg);
	return ret;
}


int WISOL::setZone(){
	return -1;
}

void WISOL::printRecv(char *in, const int len) {
	for (int i=0; i<len; i++){
		Serial.print(in[i]);
	}
	Serial.println("");
	clearBuffer();
}


int WISOL::strCmp(char *in1, char *in2, const int len){
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
 - Return -1 otherwise
*/
int WISOL::testComms(){
	const char testchar[] = "AT";
	recvMsg *receivedMsg;
	int receivedResult;
	int ret = 0;

	Buffer_Init(); // Prepare buffer
	receivedMsg = (recvMsg *)malloc(sizeof(recvMsg));
	receivedResult = sendMessage(testchar, 2, receivedMsg); // Send message

	// Read ACK
	if (receivedResult == -1) {
		ret = -1;
	} else {
		if (receivedMsg->inData[0] == 'O' && receivedMsg->inData[1] == 'K') {
			ret = 0;
		} else {
			ret = -1;
		}
	}
	
	free(receivedMsg);
	clearBuffer();
	return ret;
}

/*
recvMsg sendPayload(char *inData, int len){
	- Input: inData is a pointer to the sending payload.
	         len is the length (in bytes) of the sending payload.
	- Output: recvMsg is received message
Important note: inData is a string of hex (i.e., to send a byte of 0xAB, we need inData = {'A', 'B'})
*/
int WISOL::sendPayload(uint8_t *outData, const uint8_t len, const int downlink, recvMsg *receivedMsg){
	int receivedResult;

	if ((len > 12) || (len <= 0)){
		Serial.println("Payload length must be positive and not be longer than 12 bytes.");
		clearBuffer();
		return -1;
	}

	if (outData == NULL){
		Serial.println("outData is NULL.");
		clearBuffer();
		return -1;
	}

	if (!((downlink == 0) || (downlink == 1))){
		Serial.println("downlink must be 0 or 1.");
		clearBuffer();
		return -1;
	}

	if (receivedMsg == NULL){
		Serial.println("receivedMsg is NULL.");
		clearBuffer();
		return -1;
	}

	receivedResult = sendPayloadProcess(outData, len, downlink, receivedMsg);

	return receivedResult;
}


/*
recvMsg sendPayload(char *inData, int len){
	- Input: inData is a pointer to the sending payload.
	         len is the length (in bytes) of the sending payload.
	- Output: recvMsg is received message
Important note: inData is a string of hex (i.e., to send a byte of 0xAB, we need inData = {'A', 'B'})
*/
int WISOL::sendPayload(uint8_t *outData, const uint8_t len, int downlink){
	int receivedResult;

	if ((len > 12) || (len <= 0)){
		Serial.println("Payload length must be positive and not be longer than 12 bytes.");
		clearBuffer();
		return -1;
	}

	if (outData == NULL){
		Serial.println("outData is NULL.");
		clearBuffer();
		return -1;
	}

	if (!((downlink == 0) || (downlink == 1))){
		Serial.println("downlink must be 0 or 1.");
		clearBuffer();
		return -1;
	}

	receivedResult = sendPayloadProcess(outData, len, downlink, NULL);

	return receivedResult;
}


int WISOL::sendPayloadProcess(uint8_t *outData, const uint8_t len, const int downlink, recvMsg *receivedMsg){
	const char header[] = "AT$SF=";
	const uint8_t headerLen = (uint8_t) strlen(header);
	int actualLen;
	int sendLen;
	char* hex_str;
	int receivedResult;

	if ((outData[len] == '\0') || (outData[len] == '\n')){
		actualLen = len - 1;
	} else {
		actualLen = len;
	}

	hex_str = (char*) malloc (2 * actualLen);
	ASCII2Hex(outData, actualLen, hex_str);
	sendLen = 2*actualLen;

	clearBuffer();
	receivedResult = prepareZone();
	
	if (receivedResult == -1){
		Serial.println("Prepare zone failed");
		clearBuffer();
		return -1;
	}
	
	delay(20);
	Buffer_Init();
	for (int i=0; i<headerLen; i++){
		Serial.print(header[i]); // print header first
	}

	for (int i=0; i<sendLen; i++){
		Serial.print(hex_str[i]); // print payload
	}

	if (downlink == 1){
		Serial.print(",1");
	} else {

	}
	
	Serial.println('\0'); // send end terminal
	free(hex_str); // free hex_str from the memory

	if (receivedMsg != NULL){
		receivedResult = getRecvMsg(receivedMsg, downlink); // Read ACK
	} else {
		return 0; // No wait for ack
	}
	return receivedResult;
}



/*
	sendMessage
	- Input: outData is a pointer to the sending message.
	         len is the length of the sending message.
	- Output: recvMsg is received message
*/
int WISOL::sendMessage(char *outData, const uint8_t len, recvMsg *receivedMsg){
	int receivedResult;

	if (len <= 0){
		Serial.println("Payload length must be positive.");
		clearBuffer();
		return -1;
	}

	if (outData == NULL){
		Serial.println("outData is NULL.");
		clearBuffer();
		return -1;
	}

	if (receivedMsg == NULL){
		Serial.println("receivedMsg is NULL.");
		clearBuffer();
		return -1;
	}

	clearBuffer();
	Buffer_Init(); // prepare buffer
	for (int i=0; i<len; i++){
		Serial.print(outData[i]); // send message
	}
	Serial.println('\0'); // send end terminal

	receivedResult = getRecvMsg(receivedMsg, 0); // read ack or return payload

	return receivedResult;
}

/*
recvMsg prepareZone()
	- Set the zone to ZC
*/
int WISOL::prepareZone(){
	recvMsg *receivedMsg;
	int receivedResult;

	receivedMsg = (recvMsg *)malloc(sizeof(recvMsg));
	switch (currentZone){
		case RCZ1:
		{
			const char testchar[] = "ATS302=15";
			receivedResult = sendMessage(testchar, (int) strlen(testchar), receivedMsg);
			break;
		}
		case RCZ2:
		{
			checkChangeZone();
			break;
		}
		case RCZ4:
		{
			checkChangeZone();
			// const char testchar[] = "AT$RC";
			// receivedResult = sendMessage(testchar, (int) strlen(testchar), receivedMsg);
			break;
		}
		case RCZ3:
		{
			const char testchar[] = "ATS302=15";
			receivedResult = sendMessage(testchar, (int) strlen(testchar), receivedMsg);
			break;
		}	
		default:
		{
			receivedResult = -1;
			break;
		}
	}
	free(receivedMsg);
	return receivedResult;
}


// check the need of reset the macro channels
void WISOL::checkChangeZone() {
	recvMsg *RecvMsg;
	const char msg[] = "AT$GI?";
	const char testchar[] = "AT$RC";
	char X, Y;
	int receivedResult;
	
	RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
	receivedResult = sendMessage(msg, (int) strlen(msg), RecvMsg);
	
	X = RecvMsg->inData[0];
	Y = RecvMsg->inData[2];
	
	for (int i=0; i<RecvMsg->len; i++){
		Serial.print(RecvMsg->inData[i]);
	}
	Serial.println("");
	
	if ((X=='0') || (Y<'3')) {
		receivedResult = sendMessage(testchar, (int) strlen(testchar), RecvMsg);
	}

	free(RecvMsg);
	clearBuffer();
	return receivedResult;
}


// prepare buffer
void WISOL::Buffer_Init()
{
  for (int i = 0; i < BUFFER_SIZE; i++) {
    master_receive[i] = 0xFF;
  }
}


/*
recvMsg getRecvMsg()
	- Output: return receive message from WISOL
*/
int WISOL::getRecvMsg(recvMsg *receivedMsg, const int downlink){
	int count = 1;
	int countMax;
	int receivedResult;

	if (downlink == 1){
		countMax = 460; // wait 45s + 1 extra second
	} else {
		countMax = 100; // wait 10 second
	}

	// Wait for the incomming message
	while ((Serial.available() == 0) && (count <= countMax)) {
		count++;
		delay(100);
	}

	receivedResult = getdownlinkMsg(receivedMsg);

	return receivedResult;
}


int WISOL::getdownlinkMsg(recvMsg *receivedMsg){

	// Prepare receive message format
	receivedMsg->len = Serial.available();
	receivedMsg->inData = master_receive;
	for (int i=0; i<BUFFER_SIZE; i++) { // clear master receive buffer
		master_receive[i] = '\0';
	}
	if (receivedMsg->len){
		for (int i=0; (i < receivedMsg->len) && (i < BUFFER_SIZE); i++){   // FIXED FOR BUFFER OVERRUN!
			master_receive[i] = Serial.read(); // Read receive message
		}

		if (strCmp(receivedMsg->inData, "OK", 2)==1){
			return 0;
		} else {

		}

		if (strCmp(receivedMsg->inData, "Er", 2)){
			return -1;
		} else {
			return 0;
		}

	} else {
		return -1;
	}
	clearBuffer();
}


void WISOL::ASCII2Hex(uint8_t* input, int length, char* buf_str){
    for (int i = 0; i < length; i++)
    {
        buf_str += sprintf(buf_str, "%02X", input[i]);
    }
    sprintf(buf_str,"\n");
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

void WISOL::clearBuffer(){
	Serial.print("\0"); // Make sure there is no unfinished message.
	switch (currentZone){
		case RCZ1:
		{
			delay(1000);
			break;
		}
		case RCZ2:
		{
			delay(50);
			break;
		}
		case RCZ4:
		{
			delay(50);
			break;
		}
		default:
		{
			
		}
	}

	while (Serial.available()!=0){
		Serial.read();
		delay(10);
	}
}
