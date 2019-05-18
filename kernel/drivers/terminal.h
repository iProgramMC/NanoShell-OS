#include "vgatext.h"

const char* hex_digits = "0123456789abcdef";

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
size_t strlen(const char*s);
 
void InitTerminal(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = 0x1f;//VgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = VgaEntry(' ', terminal_color);
		}
	}
}

void TerminalPutEntryAt(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = VgaEntry(c, color);
}
 
void ShiftUp()
{
	for(unsigned x = 0; x < VGA_WIDTH; x++)
	{
		for(unsigned y = 1; y < VGA_HEIGHT; y++)
		{
			terminal_buffer[(y - 1) * VGA_WIDTH + x] = terminal_buffer[y * VGA_WIDTH + x];
		}
	}
	for(unsigned x = 0; x < VGA_WIDTH; x++)
	{
		terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = VgaEntry(' ', terminal_color);
	}
}

void TerminalSetColor(uint8_t color) 
{
	terminal_color = color;
}

bool c_TerminalPutChar(char c) 
{
	bool m = false;
	switch(c)
	{
		case '\n':
			terminal_row++;
			if (++terminal_column == VGA_WIDTH) {
				terminal_column = 0;
				if (++terminal_row == VGA_HEIGHT)
				{
					ShiftUp();
					terminal_row--;
				}
			}
		case '\r':
			terminal_column = 0;
			break;
		case '\b':
			terminal_column--;
			if(terminal_column > VGA_WIDTH) 
			{
				terminal_column = VGA_WIDTH - 1; 
				terminal_row--;
			}
			if(terminal_row > VGA_HEIGHT) terminal_row = 0; // probably underflown
			TerminalPutEntryAt(' ', terminal_color, terminal_column, terminal_row);
			break;
		default:
			TerminalPutEntryAt(c, terminal_color, terminal_column, terminal_row);
			if (++terminal_column == VGA_WIDTH) {
				terminal_column = 0;
				if (++terminal_row == VGA_HEIGHT)
				{
					ShiftUp();
					m = true;
					terminal_row--;
				}
			}
	}
	return m;
}

void TerminalPutChar(char c) 
{
	//
	/* 
	  // We make use of some unconventional switch methods, as case pretty much acts like a goto.
	  // for example:
	  // Code A
	  switch(i)
	  {
		  case 'a': printf("Got a"); break;
		  case 'b': printf("Got b"); break;
		  default:  printf("Got %c", i); break;
	  }
	  
	  // Code B
	  // it's pretty much analogous to:
	  if(i == 'a') goto casea;
	  else if(i == 'b') goto caseb;
	  else goto default;
	  casea: printf("Got a"); goto break;
	  caseb: printf("Got b"); goto break;
	  default: printf("Got %c", i); goto break;
	  break;
	  
	  If we omit the break keyword, the switch control will just fall into the next case. This has since been removed in C#.
	  for example:
	  
	  // Code C
	  switch(a)
	  {
		  case 10:printf("got 10 ");
		  default:printf("%d", a);
	  }
	  
	  will result in the output showing:
	  // got 10 10 if a == 10 or
	  // *value of a* if a != 10
	  meaning that both the case 10 and default cases have been run. If we instead place the break keyword, we will get:
	  // got 10 if a == 10 or
	  // *value of a* if a != 10.
	  
	  In C#, code C will throw an error, but Code A and B will not, even though they are functionally similar, because in C#, Code A is more
	  analogous to this:
	  // Code D
	  if(i == 'a') { Console.Write("Got a");}
	  ... you get the idea.
	*/
	switch(c)
	{
		case '\n':
			terminal_row++;
			if (++terminal_column == VGA_WIDTH) {
				terminal_column = 0;
				if (++terminal_row == VGA_HEIGHT)
				{
					ShiftUp();
					terminal_row--;
				}
			}
		case '\r':
			terminal_column = 0;
			break;
		case '\b':
			terminal_column--;
			if(terminal_column > VGA_WIDTH) 
			{
				terminal_column = VGA_WIDTH - 1; 
				terminal_row--;
			}
			if(terminal_row > VGA_HEIGHT) terminal_row = 0; // probably underflown
			TerminalPutEntryAt(' ', terminal_color, terminal_column, terminal_row);
			break;
		default:
			TerminalPutEntryAt(c, terminal_color, terminal_column, terminal_row);
			if (++terminal_column == VGA_WIDTH) {
				terminal_column = 0;
				if (++terminal_row == VGA_HEIGHT)
				{
					ShiftUp();
					terminal_row--;
				}
			}
	}
}
 
 
bool c_TerminalWrite(const char* data, size_t size) 
{
	bool m = false;
	for (size_t i = 0; i < size; i++)
	{
		if(data[i] == '\n')
		{
			terminal_column = 0;
			terminal_row++;
		}
		else if(data[i] == '\xff'){}
		else if(data[i] == '\r')
		{
			terminal_column = 0;
		}
		else
		{
			if(terminal_row >= VGA_HEIGHT)
			{
				ShiftUp();
				m = true;
				terminal_row--;
			}
			m = c_TerminalPutChar(data[i]);
		}
	}
	return m;
}
void TerminalWrite(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
	{
		if(data[i] == '\n')
		{
			terminal_column = 0;
			terminal_row++;
		}
		else if(data[i] == '\xff'){}
		else if(data[i] == '\r')
		{
			terminal_column = 0;
		}
		else
		{
			if(terminal_row >= VGA_HEIGHT)
			{
				ShiftUp();
				terminal_row--;
			}
			TerminalPutChar(data[i]);
		}
	}
}

char* HexNumbers   = "0123456789abcdef";
char* HexNumbersUp = "0123456789ABCDEF";
char* ToHex(uint32_t m)
{
	char* n = "........";
	n[7] = HexNumbers[(m >> 0x00) & 15];
	n[6] = HexNumbers[(m >> 0x04) & 15];
	n[5] = HexNumbers[(m >> 0x08) & 15];
	n[4] = HexNumbers[(m >> 0x0c) & 15];
	n[3] = HexNumbers[(m >> 0x10) & 15];
	n[2] = HexNumbers[(m >> 0x14) & 15];
	n[1] = HexNumbers[(m >> 0x18) & 15];
	n[0] = HexNumbers[(m >> 0x1c) & 15];
	return n;
}
char* ToHexUp(uint32_t m)
{
	char* n = "........";
	n[7] = HexNumbersUp[(m >> 0x00) & 15];
	n[6] = HexNumbersUp[(m >> 0x04) & 15];
	n[5] = HexNumbersUp[(m >> 0x08) & 15];
	n[4] = HexNumbersUp[(m >> 0x0c) & 15];
	n[3] = HexNumbersUp[(m >> 0x10) & 15];
	n[2] = HexNumbersUp[(m >> 0x14) & 15];
	n[1] = HexNumbersUp[(m >> 0x18) & 15];
	n[0] = HexNumbersUp[(m >> 0x1c) & 15];
	return n;
}
 
char* ToHexShort(uint16_t m)
{
	char* n = "....";
	n[3] = HexNumbers[(m >> 0x10) & 15];
	n[2] = HexNumbers[(m >> 0x14) & 15];
	n[1] = HexNumbers[(m >> 0x18) & 15];
	n[0] = HexNumbers[(m >> 0x1c) & 15];
	return n;
}
char* ToHexUpShort(uint16_t m)
{
	char* n = "....";
	n[3] = HexNumbersUp[(m >> 0x10) & 15];
	n[2] = HexNumbersUp[(m >> 0x14) & 15];
	n[1] = HexNumbersUp[(m >> 0x18) & 15];
	n[0] = HexNumbersUp[(m >> 0x1c) & 15];
	return n;
}
char* ToHexByte(uint8_t m)
{
	char* n = "..";
	n[1] = HexNumbers[(m >> 0x18) & 15];
	n[0] = HexNumbers[(m >> 0x1c) & 15];
	return n;
}
char* ToHexUpByte(uint8_t m)
{
	char* n = "..";
	n[1] = HexNumbersUp[(m >> 0x18) & 15];
	n[0] = HexNumbersUp[(m >> 0x1c) & 15];
	return n;
}
 
void TerminalClear(void)
{
	uint16_t i = 0;
	while (i < VGA_WIDTH * VGA_HEIGHT) {
		terminal_buffer[i++] = VgaEntry(' ', terminal_color);
	}
}

void TerminalWriteString(const char* data) 
{
	TerminalWrite(data, strlen(data));
}

void TerminalWriteInt(uint16_t in)
{
	TerminalPutChar('0');
	TerminalPutChar('x');
	TerminalPutChar(hex_digits[((in >> 12) & 15)]);
	TerminalPutChar(hex_digits[((in >> 8) & 15)]);
	TerminalPutChar(hex_digits[((in >> 4) & 15)]);
	TerminalPutChar(hex_digits[((in) & 15)]);
}