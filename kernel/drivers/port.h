/*
   NanoShell OS
   
   Drivers/Port.h -> x86 built-in Port functions
   
   - No notes available
*/
extern char ReadPort(unsigned short port);
extern void WritePort(unsigned short port, unsigned char data);
extern short ReadPortW(unsigned short port);
extern void WritePortW(unsigned short port, unsigned short data);

void WritePortSB(size_t size, uint8_t* addr, uint16_t port)
{
	size_t m = 0;
	for(; m < size; m++)
	{
		WritePort(port, addr[m]);
	}
}
void ReadPortSB(size_t count, uint8_t* addr, uint16_t port)
{
	size_t m = 0;
	for(; m < count; m++)
	{
		addr[m] = ReadPort(port);
	}
}