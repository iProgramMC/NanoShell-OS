
#define CMDSIZE 128
#define USER "admin"
#define DOMAIN "nanoshell"
char CmdBuffer [CMDSIZE];

uint16_t term_x = 0;
uint16_t term_y = 0;

void PrintUserPrompt()
{
	printf("\n\n%s@%s>", USER, DOMAIN);
	term_x = terminal_column;
	term_y = terminal_row;
	putch('_');
}

uint8_t GetNumber(char m)
{
	char s = m;
	if(s >= 'a')
	{
		s -= 0x27;
	}
	if(s >= 'A')
	{
		s -= 0x07;
	}
	return s - '0'; 
}

void ProcessCommand()
{
	char cmd[128];
	int aslkfj = 0;
	char* cmdbuf = CmdBuffer;
	while(*cmdbuf != ' ' && *cmdbuf != 0)
	{
		cmd[aslkfj] = *cmdbuf;
		cmdbuf++;
		aslkfj++;
	}
	if(strcmp(CmdBuffer, "test") == 0)
	{
		//uint8_t* applicationMemory = (uint8_t*) 0x00800000; // This is where applications are expected to be loaded
		int entry = GetFSEntry("test.bin");
		if(entry != -1)
		{
			uint8_t* f = FileOpen(entry);
			NSProg prog;
			prog.PC = 0;
			prog.ProgramData = f;
			prog.ProgramSize = rootEntries[entry].FileSize;
			Execute(&prog);
		}
		//AppRun(applicationMemory);
	}
	else if(strcmp(CmdBuffer, "ver") == 0)
	{
		printf("\n\n%s version %s", OS_NAME, VERSION_ID);
	}
	else if(strcmp(CmdBuffer, "cls") == 0)
	{
		TerminalClear();
		terminal_column = 0;
		terminal_row = 0;
	}
	else if(strcmp(CmdBuffer, "help") == 0)
	{
		printf("\n\nAvailable commands are: \nhelp   - Shows this table\ncls    - Clears the screen\nver    - Shows the version ID\ntest   - Shows the test string\ncolor  - Changes the screen colors and clears the screen.\ncolor2 - Changes the screen colors but does not clear the screen\necho   - Echoes all parameters you give\nmode   - Sets the video mode (0 - 80x50, 1 - 80x25, 2 - 90x30, 3 - 90x60)\n");
	}
	else if(strcmp(CmdBuffer, "dir") == 0)
	{
		PutDir();
	}
	else if(strcmp(CmdBuffer, "fsinit") == 0)
	{
		InitFilesystem();
		puts("File system initialized. Now you can use the 'dir' command to browse the root.");
	}
	else if(strcmp(CmdBuffer, "vga") == 0)
	{
		SwitchMode(GMode320x200x256);
		for(int i = 0; i < 256; i++)
		{
			FillScreen(i);
			// wait for key press
			char key = 0;
			while (key == 0)
			{
				for(unsigned char i = 0; i < 128; i++)
				{
					if(keyboardState[i] == KEY_PRESSED)
					{
						if(IsPrintable(i))
						{
							key = KeyboardMap[i + (ShiftPressed() ? 0x80 : 0x00)];
							keyboardState[i] = KEY_HELD;
							break;
						}
					}
				}
			}
			
			continue;
		}
	}
	else if(strcmp(CmdBuffer, "vga2") == 0)
	{
		SwitchMode(GMode320x200x256);
		FillScreen(0);
		for(int x = 0; x < 16; x++)
		{
			for(int y = 0; y < 16; y++)
			{
				GWritePixel(x * 2, y * 2, y * 16 + x);
				GWritePixel(x * 2 + 1, y * 2, y * 16 + x);
				GWritePixel(x * 2, y * 2 + 1, y * 16 + x);
				GWritePixel(x * 2 + 1, y * 2 + 1, y * 16 + x);
			}
		}
		WaitForKeyPress();
	}
	else if(strcmp(cmd, "format") == 0)
	{
		char* fn = CmdBuffer + 7;
		int f = GetFSEntry(fn);
		FatEntry* e = &rootEntries[f];
		printf("\nStarting cluster: %x\nFile size:%x\n\n", e->StartingSector, e->FileSize);
		uint8_t* p = FileOpen(f);
		if(p != NULL)
		{
			for(uint32_t i = 0; i < rootEntries[f].FileSize; i++)
			{
				if(p[i] != 0)
				{
					TerminalPutChar(p[i]);
				}
				else
				{
					TerminalPutChar('.');
				}
			}
		}
		//puts((char*)p);
		//FormatFat();
	}
	else if(strcmp(cmd, "interpret") == 0)
	{
		char* fn = CmdBuffer + 10;
		int entry = GetFSEntry(fn);
		if(entry != -1)
		{
			uint8_t* f = FileOpen(entry);
			puts("\nFile found, running...\nProgram output:\n\n");
			NSProg prog;
			prog.PC = 0;
			prog.ProgramData = f;
			prog.ProgramSize = rootEntries[entry].FileSize;
			Execute(&prog);
		}
	}
	else if(strcmp(cmd, "interpret_lite") == 0)
	{
		char* fn = CmdBuffer + 15;
		int f = GetFSEntry(fn);
		FatEntry* e = &rootEntries[f];
		if(f != -1)
		{
			uint8_t* p = FileOpen(f);
			puts("\nFile found, running...\nProgram output:\n\n");
			LiteInt_SimpleInt(p, e->FileSize);
		}
	}
	else if(strcmp(CmdBuffer, "vga") == 0)
	{
		SwitchMode(GMode640x480x16);
		for(int i = 0; i < 16; i++)
		{
			FillScreen(i);
			PushFrame();
		}
		SwitchMode(TMode80x50);
		TerminalClear();
		terminal_column = 0;
		terminal_row = 0;
		puts("\nThe test was successful");
	}
	else if(strcmp(cmd, "echo") == 0)
	{
		printf("\n%s", CmdBuffer + 5);
	}
	else if(strcmp(cmd, "color") == 0)
	{
		char m1, m2;
		m1 = CmdBuffer[6];
		m2 = CmdBuffer[7];
		if(m1 != m2)
		{
			terminal_color = (GetNumber(m1) << 4) + GetNumber(m2);
			TerminalClear();
			terminal_column = 0;
			terminal_row = 0;
		}
	}
	else if(strcmp(cmd, "mode") == 0)
	{
		char m1;
		m1 = CmdBuffer[5];
		TerminalClear();
		terminal_column = 0;
		terminal_row = 0;
		switch(m1)
		{
			case '0':
				SwitchMode(TMode80x50);
				break;
			case '1':
				SwitchMode(TMode80x25);
				break;
			case '2':
				SwitchMode(TMode90x30);
				break;
			case '3':
				SwitchMode(TMode90x60);
				break;
			default:
				printf("\nUnknown mode");
				break;
		}
	}
	else if(strcmp(cmd, "color2") == 0)
	{
		char m1, m2;
		m1 = CmdBuffer[7];
		m2 = CmdBuffer[8];
		if(m1 != m2)
		{
			terminal_color = (GetNumber(m1) << 4) + GetNumber(m2);
			//TerminalClear();
			//terminal_column = 0;
			//terminal_row = 0;
		}
	}
	else
	{
		printf("\nUnknown command: '%s'", CmdBuffer);
	}
	PrintUserPrompt();
	
	memset(CmdBuffer, 0, CMDSIZE);
	memset(cmd, 0, 128);
}

void CommandPrompt_Main()
{
	uint8_t ptr = 0xff;
	
	printf("Initialization done.");
	PrintUserPrompt();
	terminal_column = term_x;
	terminal_row = term_y;
	puts(CmdBuffer);
	bool isRunning = true;
	do
	{
		char key = 0;
		for(unsigned char i = 0; i < 128; i++)
		{
			if(keyboardState[i] == KEY_PRESSED)
			{
				if(IsPrintable(i))
				{
					key = KeyboardMap[i + (ShiftPressed() ? 0x80 : 0x00)];
					keyboardState[i] = KEY_HELD;
					break;
				}
			}
		}
		if(key != 0)
		{
			if(key == '\b')
			{
				if(ptr < 0xff)
				{
					terminal_column = term_x;
					terminal_row = term_y;
					for(uint8_t i = 0; i < strlen(CmdBuffer) + 1; i++)
					{
						putch(' ');
					}
					CmdBuffer[ptr] = 0;
					ptr--;
					terminal_column = term_x;
					terminal_row = term_y;
					puts(CmdBuffer);
					putch('_');
				}
			}
			else if(key == '\n')
			{
				terminal_column = term_x;
				terminal_row = term_y;
				puts(CmdBuffer);
				putch(' ');
				ProcessCommand();
				ptr = 0xff;
			}
			else
			{
				ptr++;
				CmdBuffer[ptr] = key;
				terminal_column = term_x;
				terminal_row = term_y;
				bool m = cputs(CmdBuffer); // checks if screen scrolled
				putch('_');
				if(m)
				{
					term_y--;
				}
				terminal_column = term_x;
				terminal_row = term_y;
			}
			key = 0;
		}
		hlt;
	}
	while(isRunning);
}