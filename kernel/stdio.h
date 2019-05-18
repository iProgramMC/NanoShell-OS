// Define a bunch of macros for the OUT and IN.
#define outb(data, port) WritePort(port, data)
#define inb(port) ReadPort(port)
#define outw(data, port) WritePortW(port, data)
#define inw(port) ReadPortW(port)
#define outl(data, port) WritePortL(port, data)
#define inl(port) ReadPortL(port)
#define outsb(size, addr, port) WritePortSB(size, addr, port)
#define insb(size, addr, port) ReadPortSB(size, addr, port)
#define outsw(size, addr, port) WritePortSW(size, addr, port)
#define insw(size, addr, port) ReadPortSW(size, addr, port)
#define outsl(size, addr, port) WritePortSL(size, addr, port)
//#define insl(size, addr, port) ReadPortSL(size, addr, port)

void puts(const char*s)
{
	TerminalWriteString(s);
}

void putch(char s)
{
	TerminalPutChar(s);
}

void serputs(const char*str)
{
	const char*s = str;
	while(*s != 0)
	{
		SerialWrite(*s++);
	}
}

void serputch(const char s)
{
	SerialWrite(s);
}

bool cputs(const char*s)
{
	return c_TerminalWrite(s, strlen(s));
}

bool getchflag = false;
int somevalue = 0;
void pause()
{
	getchflag = true;
	while(getchflag == true) { somevalue++; }
}

int pow(int base, int exp)
{
	int res = 1;
	for(int i = 0; i < exp; i++)
	{
		res *= base;
	}
	return res;
}

int abs(int i)
{
	return (i >= 0) ? i : i-i-i;
}

void vprintf(char* format, va_list list)
{
	bool escape = false;
	while(*format != 0)
	{
		// Now do the format parsing...
		char m = *format;
		format++;
		// if we've detected an escape sequence...
		if(m == '%')
		{
			// set escape flag to true and continue
			escape = true; 
			continue;
		}
		// else
		if(!escape)
		{
			if(m != '\n')
			{
				putch(m);
			}
			else
			{
				terminal_column = 0;
				terminal_row++;
				if(terminal_row >= VGA_HEIGHT)
				{
					ShiftUp();
					terminal_row--;
				}
			}
			continue;
		}
		switch(m)
		{
			case 's':;
				// Print string...
				char* stringToPrint = va_arg(list, char*);
				puts(stringToPrint);
				escape = false;
				continue;
			case 'd':
			case 'i':;
				// Print an integer
				int intToPrint = va_arg(list, int);
				// Allocate 11 digits for the integer printing, the max value is 2147483647 and the min value is -2147483648.
				// The string's length will be 12, including the null termination char.
				//
				// We can make use of the new-fangled malloc instruction.
				void* d = malloc(12);
				char* s = d;
				int8_t digits[11];
				if(intToPrint < 0)
				{
					s[0] = '-';
				}
				else
				{
					s[0] = '\xff';
				}
				int index = 9;
				intToPrint = abs(intToPrint);
				while(intToPrint > 0)
				{
					digits[index] = intToPrint % 10;
					intToPrint /= 10;
					index--;
				}
				int m = 0;
				for(int i = index + 1; i < 10; i++)
				{
					s[1 + m] = '0' + digits[i];
					m++;
				}
				puts(s);
				free(d);
				escape = false;
				continue;
			case 'u':;
				// Print an unsigned integer
				uint32_t intToPrint2 = va_arg(list, uint32_t);
				// Allocate 11 digits for the integer printing, the max value is 4294967295
				// The string's length will be 12, including the null termination char.
				//
				// We can make use of the new-fangled malloc instruction.
				void* d2 = malloc(12);
				char* s2 = d2;
				int8_t digits2[11];
				int index2 = 9;
				while(intToPrint2 > 0)
				{
					digits2[index2] = intToPrint2 % 10;
					intToPrint2 /= 10;
					index2--;
				}
				int m2 = 0;
				for(int i2 = index2 + 1; i2 < 10; i2++)
				{
					s2[m2] = '0' + digits2[i2];
					m2++;
				}
				puts(s2);
				free(d2);
				escape = false;
				continue;
			case 'x':
				puts("0x");
				puts(ToHex(va_arg(list, uint32_t)));
				escape = false;
				continue;
			case 'X':
				puts("0x");
				puts(ToHexUp(va_arg(list, uint32_t)));
				escape = false;
				continue;
			case '(':
				putch('%');
				escape = false;
				continue;
			case 'c':
				putch((char)va_arg(list, int));
				escape = false;
				continue;
		}
	}
}

void printf(char* format, ...)
{
	va_list list;
	va_start(list, format);
	vprintf(format, list);
	va_end(list);
}

void vserprintf(char* format, va_list list)
{
	bool escape = false;
	while(*format != 0)
	{
		// Now do the format parsing...
		char m = *format;
		format++;
		// if we've detected an escape sequence...
		if(m == '%')
		{
			// set escape flag to true and continue
			escape = true; 
			continue;
		}
		// else
		if(!escape)
		{
			serputch(m);
			continue;
		}
		switch(m)
		{
			case 's':;
				// Print string...
				char* stringToPrint = va_arg(list, char*);
				serputs(stringToPrint);
				escape = false;
				continue;
			case 'd':
			case 'i':;
				// Print an integer
				int intToPrint = va_arg(list, int);
				// Allocate 11 digits for the integer printing, the max value is 2147483647 and the min value is -2147483648.
				// The string's length will be 12, including the null termination char.
				//
				// We can make use of the new-fangled malloc instruction.
				void* d = malloc(12);
				char* s = d;
				int8_t digits[11];
				if(intToPrint < 0)
				{
					s[0] = '-';
				}
				else
				{
					s[0] = '\xff';
				}
				int index = 9;
				intToPrint = abs(intToPrint);
				while(intToPrint > 0)
				{
					digits[index] = intToPrint % 10;
					intToPrint /= 10;
					index--;
				}
				int m = 0;
				for(int i = index + 1; i < 10; i++)
				{
					s[1 + m] = '0' + digits[i];
					m++;
				}
				serputs(s);
				free(d);
				escape = false;
				continue;
			case 'u':;
				// Print an unsigned integer
				uint32_t intToPrint2 = va_arg(list, uint32_t);
				// Allocate 11 digits for the integer printing, the max value is 4294967295
				// The string's length will be 12, including the null termination char.
				//
				// We can make use of the new-fangled malloc instruction.
				void* d2 = malloc(12);
				char* s2 = d2;
				int8_t digits2[11];
				int index2 = 9;
				while(intToPrint2 > 0)
				{
					digits2[index2] = intToPrint2 % 10;
					intToPrint2 /= 10;
					index2--;
				}
				int m2 = 0;
				for(int i2 = index2 + 1; i2 < 10; i2++)
				{
					s2[m2] = '0' + digits2[i2];
					m2++;
				}
				serputs(s2);
				free(d2);
				escape = false;
				continue;
			case 'x':
				serputs("0x");
				serputs(ToHex(va_arg(list, uint32_t)));
				escape = false;
				continue;
			case 'X':
				serputs("0x");
				serputs(ToHexUp(va_arg(list, uint32_t)));
				escape = false;
				continue;
			case '(':
				serputch('%');
				escape = false;
				continue;
			case 'c':
				serputch((char)va_arg(list, int));
				escape = false;
				continue;
		}
	}
}

void serprintf(char* format, ...)
{
	va_list list;
	va_start(list, format);
	vserprintf(format, list);
	va_end(list);
}

// Not standard to the GNU C Library, but whatever.
void HexDumpMemory(unsigned long f_, size_t size)
{
	uint8_t* f = (uint8_t*) f_;
	for(size_t i = 0; i < size; i++)
	{
		char f1, f2;
		uint8_t fs = *(f + i);
		f1 = HexNumbersUp[fs >> 4];
		f2 = HexNumbersUp[fs & 15];
		serputch(f1);
		serputch(f2);
		serputch(' ');
	}
}
void DumpMemory(unsigned long f_, size_t size)
{
	uint8_t* f = (uint8_t*) f_;
	for(size_t i = 0; i < size; i++)
	{
		serputch((char)(*(f + i)));
	}
}