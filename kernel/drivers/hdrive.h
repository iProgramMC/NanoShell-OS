#define C_READ_FUNC

#ifdef C_READ_FUNC

#define CMD_READ 0x20
#define CMD_WRITE 0x30

#define BSY_FLAG 0x80

void AtaLbaRead(uint32_t lba, void* dest_, uint8_t drive_num, uint8_t secCount)
{
	uint16_t max, base, *buffer = (uint16_t*)dest_;
	uint8_t drive = 0x40;
	
	switch(drive_num)
	{
		case 0: case 1: base = 0x01f0; break;
		case 2: case 3: base = 0x0170; break;
		case 4: case 5: base = 0x01e8; break;
		case 6: case 7: base = 0x0168; break;
		default: return;
	}
	if(drive_num % 2) drive |= 0x10;
	WritePort(base + 2, secCount);
	WritePort(base + 3, (uint8_t)((lba & 0x000000ff)));
	WritePort(base + 4, (uint8_t)((lba & 0x0000ff00) >> 8));
	WritePort(base + 5, (uint8_t)((lba & 0x00ff0000) >> 16));
	WritePort(base + 6, (uint8_t)((lba & 0x0f000000) >> 24) | drive);
	WritePort(base + 7, CMD_READ);
	
	while(ReadPort(base + 7) & BSY_FLAG);
	
	max = secCount * 256;
	for(int as = 0; as < max; as++) *buffer++ = ReadPortW(base);
}

void AtaLbaReadS(uint32_t lba, void* dest_, uint8_t drive_num)
{
	AtaLbaRead(lba, dest_, drive_num, 1);
}

#else
extern void AtaLbaRead(uint32_t lba, void* dest, uint8_t sectorsToRead, uint8_t drive_num);
extern void AtaLbaWrite(uint32_t lba, void* dest, uint8_t sectorsToRead, uint8_t drive_num);

extern void AtaLbaReadS(uint32_t lba, void* dest, uint8_t drive_num);
extern void AtaLbaWriteS(uint32_t lba, void* dest, uint8_t drive_num);
#endif