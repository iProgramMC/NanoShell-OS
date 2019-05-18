/* 
   NanoShell Interpreter v1.1. Applications written for NSIv1.1 WILL NOT work in NSIv1.0.
   Trying to run one would ideally throw a few unknown opcode errors. Some new opcodes are 
   0x5c (SYS), SMPH and SMPL, SMP, SPP and GCP.
   
   This WILL run NSIv1.0 applications though.
*/

// Constants
#define RAMSize 1048576
#define StackSize 1024

// Struct definition
#define NanoShellProgram struct nanoshell_program
#define NSProg NanoShellProgram
struct nanoshell_program 
{
	uint16_t 	PC;
	uint8_t 	Registers[16];
	uint8_t 	RAM[RAMSize];
	int 		RAMPage;
	int 		JumpPage;
	bool		Initialized; // Some features are not available when device isn't initialized
	uint8_t		Stack[StackSize];
	uint16_t	StackPtr;
	uint8_t*	ProgramData;
	int			ProgramSize;
};

// Helper functions
void PushToStack(NSProg* prog, uint8_t b)
{
	prog->Stack[prog->StackPtr++] = b;
}

uint8_t PopFromStack(NSProg* prog)
{
	uint8_t f = prog->Stack[prog->StackPtr];
	prog->Stack[prog->StackPtr--] = 0;
	return f;
}

bool Is4BitInstruction(uint8_t reg) { return (reg & 1) == 1; }

void Initialize(NSProg* prog)
{
	prog->Initialized = true;
}

bool PerformInstruction(NSProg* prog, uint8_t instr, uint8_t data, uint8_t reg)
{
	if(prog->StackPtr >= StackSize)
	{
		puts("! EMERGENCY !\nThe stack pointer has exceeded stack boundaries, exiting\n");
		return false;
	}
	
	if(Is4BitInstruction(reg))
	{
		if(instr == 0xff && data == 0xff && reg == 0xff)
		{
			Initialize(prog);
		}
		else
		{
			uint8_t a = instr >> 4;
			uint16_t newAddr = (instr << 12);
			newAddr >>= 4;
			newAddr += data;
			uint8_t rega, regb;
			switch (a)
			{
				case 0x02:
					rega = (reg >> 5);
					regb = ((reg >> 1) & 15);
					if(prog->Registers[rega] != prog->Registers[regb])
					{
						prog->PC = prog->JumpPage * 4096 + newAddr;
					}
					break;
				case 0x03:
					rega = (reg >> 5);
					regb = ((reg >> 1) & 15);
					if(prog->Registers[rega] == prog->Registers[regb])
					{
						prog->PC = prog->JumpPage * 4096 + newAddr;
					}
					break;
				case 0x06:
					regb = ((reg >> 1) & 15);
					if(prog->Registers[regb] != 0)
					{
						prog->PC = prog->JumpPage * 4096 + newAddr;
					}
					break;
				case 0x07:
					regb = ((reg >> 1) & 15);
					if(prog->Registers[regb] == 0)
					{
						prog->PC = prog->JumpPage * 4096 + newAddr;
					}
					break;
				case 0x08:
					prog->PC = prog->JumpPage * 4096 + newAddr;
					break;
				case 0x0b:
					prog->RAMPage = newAddr;
					break;
				case 0x0c:
					prog->JumpPage = newAddr;
					break;
				case 0x0f:;
					uint8_t pgm0, pgm1;
					pgm0 = prog->PC >> 8;
					pgm1 = prog->PC & 0xff;
					PushToStack(prog, pgm1);
					PushToStack(prog, pgm0);
					prog->PC = newAddr;
					break;
			}
		}
	}
	else
	{
		//printf("Executing instruction (%X).\n", (instr << 16) + (data << 8) + reg);
		uint8_t register_, ram_addr, pdata;
		uint16_t pgm_addr;
		switch(instr)
		{
			case 0x01:
				ram_addr = data;
				register_ = reg >> 1;
				prog->Registers[register_] = prog->RAM[ram_addr];
				break;
			case 0x29:
				pgm_addr = (prog->Registers[data >> 4] << 8) + prog->Registers[data & 15];
				pdata = prog->ProgramData[prog->JumpPage * 4096 + pgm_addr];
				prog->Registers[reg >> 1] = pdata;
				break;
			case 0x08:
				prog->Registers[reg >> 1] = data;
				break;
			case 0x3d:
				prog->Registers[reg >> 1] = prog->RAM[prog->RAMPage * 256 + prog->Registers[data]];
				break;
			case 0x11:
				prog->RAM[data] = prog->Registers[reg >> 1];
				break;
			case 0x39:
				prog->RAM[prog->Registers[data]] = prog->Registers[reg >> 1];
				break;
			case 0x75:
				if(data == 0x2b && reg == 0xa6)
				{
					for(int i = 0; i < 16; i++)
					{
						prog->Registers[i] = 0;
					}
				}
				else
				{
					puts("Warning! Counterfeit CLR instruction detected, not clearing the registers. This may cause problems.\n");
				}
				break;
			case 0x94:
				prog->Registers[data >> 4] = prog->Registers[data & 15];
				break;
			case 0x06:
				prog->Registers[reg >> 1] *= data;
				break;
			case 0x04:
				prog->Registers[reg >> 1] += data;
				putch(prog->Registers[reg >> 1]);
				break;
			case 0x86:
				prog->Registers[reg >> 1] *= prog->Registers[data];
				break;
			case 0x84:
				prog->Registers[reg >> 1] += prog->Registers[data];
				break;
			case 0x95:
				prog->Registers[reg >> 1] <<= 1;
				break;
			case 0x97:
				prog->Registers[reg >> 1] >>= 1;
				break;
			case 0xaa:
				putch((prog->Registers[reg >> 1] >= 32) ? prog->Registers[reg >> 1] : '.');
				break;
			case 0x0f:
				prog->Registers[reg >> 1] = GetKey();
				break;
			case 0x4a:
				prog->Registers[reg >> 1] = (prog->Registers[data >> 4] ^ prog->Registers[data & 15]);
				break;
			case 0x6a:
				prog->Registers[reg >> 1] = (prog->Registers[data >> 4] | prog->Registers[data & 15]);
				break;
			case 0x1a:
				prog->Registers[reg >> 1] = (prog->Registers[data >> 4] & prog->Registers[data & 15]);
				break;
			case 0x57:
                prog->Registers[reg >> 1] = (~prog->Registers[data & 15]);
				break;
			case 0xab:
				putch(data >= 32 ? data : '.');
				break;
			case 0xbf:
				if(data == 0x50 && reg == 0x32)
				{
					return false;
				}
				puts("Warning! Counterfeit EXIT instruction detected, not exiting. This may cause problems.\n");
				break;
			case 0xe7:
				// Write display pixel
				;int x, y, z;
				x = prog->Registers[data >> 4];
				y = prog->Registers[data & 15];
				z = prog->Registers[reg >> 1];
				GWritePixel(x, y, z);
				// TODO
				break;
			case 0xd0:
				if(Optimize)
				{
					PushFrame();
				}
				break;
			case 0xf5:
				if(data == 0x55)
				{
					uint8_t pgm0, pgm1;
					pgm1 = PopFromStack(prog);
					pgm0 = PopFromStack(prog);
					prog->PC = (pgm0 << 8) + pgm1;
				}
				else
				{
					puts("Warning! Counterfeit return instruction detected, not returning. This may cause problems.\n");
				}
			case 0x02:
				PushToStack(prog, prog->Registers[reg >> 1]);
				break;
			case 0x03:
				prog->Registers[reg >> 1] = PopFromStack(prog);
				break;
			case 0x0a:
				// ToDo: Delay millis
				break;
			case 0x0b:
				// ToDo: Delay seconds
				break;
			case 0x0c:
				// ToDo: Get program seconds
				break;
			case 0x0d:
				// ToDo: Get program millis
				break;
			case 0x61:;
				int f = prog->RAMPage;
				f &= 0xffffff00;
				f += prog->Registers[reg >> 1];
				break;
			case 0x63:;
				int f2 = prog->RAMPage;
				f2 &= 0xffff00ff;
				f2 += prog->Registers[reg >> 1] << 8;
				break;
			case 0xb5:
				// Set video mode
				switch(data)
				{
					case 0x00:
						// Character mode:
						SwitchMode(TMode80x50);
						TerminalClear();
						break;
					case 0x01:
						// Graphics mode 320x200x256
						SwitchMode(GMode320x200x256);
						FillScreen(0);
						break;
					case 0x02:
						serputs("Warning! This mode will be very slow!");
						SwitchMode(GMode640x480x16);
						FillScreen(0);
						PushFrame();
						break;
				}
				break;
			case 0x5c:
				// SysCall
				switch(data)
				{
					case 0x8e:;
					    // Print String Syscall (loads address from registers A and B
						int ssssss = ((int)prog->ProgramData + prog->JumpPage * 4096) + (prog->Registers[1] << 8) + (prog->Registers[2]);
						puts((const char*)(ssssss));
						break;
					case 0x00:
					    // Some test syscall
						printf("Hello, World!\n");
						break;
					case 0xaf:
					    // Some test syscall
						printf("Hello!\n");
						break;
				}
				break;
			case 0x00: break;
			default:
				printf("Unknown instruction detected (%X)\n", (instr << 16) + (data << 8) + reg);
				break;
		}
	}
	return true;
}

/* Single tasking based code. Avoid at all costs except for when task scheduling is implemented. */
void Execute(NSProg* prog)
{
	uint8_t* f = prog->ProgramData;
	uint8_t instr, data, reg;
	while (prog->ProgramSize / 3 >= prog->PC / 3)
	{
		instr = f[prog->PC + 0];
		data = f[prog->PC + 1];
		reg = f[prog->PC + 2];
		prog->PC += 3;
		if(!PerformInstruction(prog, instr, data, reg))
		{
			return;
		}
	}
	puts("\nProgram exited. Welcome back to the real world.");
}