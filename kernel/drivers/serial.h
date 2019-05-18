#define COM1 0x3f8   /* COM1 */
#define COM2 0x2f8   /* COM2 */
#define COM3 0x3e8   /* COM3 */
#define COM4 0x2e8   /* COM4 */

uint16_t ports[4] = {
	COM1,
	COM2,
	COM3,
	COM4
};
uint8_t currentPort = 0;

#define PORT ports[currentPort]

#define SerialBaudRateDiv 1 /* 115200 baud */
 
void InitSerial(uint8_t port) 
{
	currentPort = port;
	
	WritePort(PORT + 1, 0x00);    				// Disable all interrupts
	WritePort(PORT + 3, 0x80);    				// Enable DLAB (set baud rate divisor)
	WritePort(PORT + 0, SerialBaudRateDiv);  	// Set divisor to 3 (lo byte) 38400 baud
	WritePort(PORT + 1, SerialBaudRateDiv >> 8);//                  (hi byte)
	WritePort(PORT + 3, 0x03);    				// 8 bits, no parity, one stop bit
	WritePort(PORT + 2, 0xC7);    				// Enable FIFO, clear them, with 14-byte threshold
	WritePort(PORT + 4, 0x0B);    				// IRQs enabled, RTS/DSR set
}

int SerialAvailable() 
{
	return ReadPort(PORT + 5) & 1;
}
 
char SerialRead() 
{
	while (SerialAvailable() == 0);
	
	return ReadPort(PORT);
}

int IsTransmitEmpty() 
{
	return ReadPort(PORT + 5) & 0x20;
}
 
void SerialWrite(char a) 
{
	while (IsTransmitEmpty() == 0);
	
	WritePort(PORT,a);
}