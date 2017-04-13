/*
  Isigfox.h - Interface for Sigfox library.
  Created by Thomas Ho, Thinxtra Solutions Pty.
  January 19, 2017.
  Released into the public domain.
*/

#ifndef Isigfox_h
#define Isigfox_h

/** Structure of the received message
	@param inData	a pointer to an array of received bytes
	@param len		the length of the array of received bytes
*/
typedef struct _recvMsg{
	int len;
	char* inData;
} recvMsg;

/** Structure of IO pins to be set for the communication with Sigfox module
	@param moduleType	the type of Sigfox module
	@param spiMISO		the IO pin of Arduino for MISO SPI
	@param spiMOSI		the IO pin of Arduino for MOSI SPI
	@param spiCLK		the IO pin of Arduino for CLK SPI
	@param spiWAKE		the IO pin of Arduino for WAKE SPI
	@param spiRST		the IO pin of Arduino for RST SPI
	@param spiCS		the IO pin of Arduino for CS SPI
	@param uartTX		the IO pin of Arduino for TX UART
	@param uartRX		the IO pin of Arduino for RX UART
*/
typedef struct {
	int moduleType	= 0;
	int spiMISO		= 0;
	int spiMOSI		= 0;
	int spiCLK		= 0;
	int spiWAKE		= 0;
	int spiRST		= 0;
	int spiCS		= 0;
	int uartTX		= 0;
	int uartRX		= 0;
} pinIO;

/** Abstract class for Sigfox*/
class Isigfox{
public:
    Isigfox(){}
    virtual ~Isigfox(){}

    /** Prepare Sigfox communication
	@return			0 if successful, -1 if failed
    */

    virtual int initSigfox()=0;


    /** Set IO pin for Sigfox Communication
	@param pin		a structure containing pin configuration
    */
    virtual void configIO(pinIO pin)=0;


    /** Test communication with the Sigfox module
    	@return			-1, not implemented
    */
    virtual int testComms()=0;



    /** Send a Sigfox frame
    	@param outData					a pointer to an array of bytes to send
    	@param len						[0...12], the length of the array of bytes to send
		@param donwlink					0, no downlink message required or 1, downlink message required. Other values are invalid.
		@param recvMsg *receivedMsg		pointer on recMsg structure. if NULL or not used, the function will not block until Sigfox module’s answer reception.
		@return							0 if succeed and -1, otherwise
    */
	virtual int sendPayload(uint8_t *outData, const uint8_t len, const int downlink, recvMsg *receivedMsg) = 0;
	virtual int sendPayload(uint8_t *outData, const uint8_t len, const int downlink) = 0;


    /** Send a command to the Sigfox module

    	@param outData					a pointer to an array of characters to send
    	@param len						the length of the array of characters to send
		@param recvMsg *receivedMsg		pointer on recMsg structure.
		@return							0 if succeed and -1, otherwise
    */
    virtual int sendMessage(char *outData, const uint8_t len, recvMsg *receivedMsg) = 0;



	/** Get Sigfox downlink message
		@param recvMsg *receivedMsg		pointer on recMsg structure.
		@return							0 if succeed and -1, otherwise
    */
	virtual int getdownlinkMsg(recvMsg *receivedMsg) = 0;

    /** Get the zone of the current Sigfox module
	@return			1 if RCZ1, 2 if RCZ2, 3 if RCZ3, or 4 if RCZ4
    */
    virtual int getZone() = 0;


    /** Set the zone of the current Sigfox module
	@return			-1, not implemented
    */
    virtual int setZone() = 0;

	/** Set the AES Key to public value in order to use SNEK dongle
	@return			0
	*/
	virtual int setPublicKey() = 0;

	/** Set the AES Key to default private value to use on sigfox network
    @return			0
    */
    virtual int setPrivateKey() = 0;

	/** Reset the FCC Macro Channel
	@return			0
    */
    virtual int resetMacroChannel() = 0;
};

#endif
