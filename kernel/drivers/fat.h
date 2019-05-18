#define FAT_BOOTLOAD_SIZE 512
uint8_t fat_bootloader[FAT_BOOTLOAD_SIZE] = {
	#include "fat_bootloader.h"
};

#define Fat32BootSector struct bootsector_fat32
Fat32BootSector
{
	uint8_t 	jmpInstruction;     
	uint16_t 	jmpAddress;         
	char 		OemName[8];         
	uint16_t 	BytesPerSector;     
	uint8_t 	SectorsPerCluster;  
	uint16_t 	ReservedSectorCount;
	uint8_t		FatCount;			
	uint16_t 	FatEntryCount;		
	uint16_t 	TotalSectors16;		
	uint8_t		Media;
	uint16_t 	FatSize16;
	uint16_t 	SectorsPerTrack;
	uint16_t 	HeadCount;
	uint32_t	HiddenSectors;
	uint32_t 	TotalSectors32;
	// At offset 36, the BPB/boot sector for FAT12 and FAT16 differs from the 
	// BPB/Boot Sector for FAT32. Thus, we needed separate structs.
	uint32_t	FatSize32;
	uint16_t	ExtFlags;
	uint16_t	FileSystemVersion;
	uint32_t	RootCluster;
	uint16_t	FileSystemInfo;
	uint16_t 	BkBootSector;
	uint8_t		Reserved[12];
	uint8_t		DriveNumber;
	uint8_t		Reserved1;
	uint8_t 	BootSig;
	uint32_t	VolumeID;
	char		VolumeLabel[11];
	char		FileSystemType[8];
	uint8_t		BootCode[448];
	uint16_t	BootSecSignature;
}__attribute__((packed));

#define Fat12BootSector struct bootsector_fat12or16
#define Fat16BootSector struct bootsector_fat12or16
Fat12BootSector
{
	uint8_t 	jmpInstruction;     
	uint16_t 	jmpAddress;         
	char 		OemName[8];         
	uint16_t 	BytesPerSector;     
	uint8_t 	SectorsPerCluster;  
	uint16_t 	ReservedSectorCount;
	uint8_t		FatCount;			
	uint16_t 	FatEntryCount;		
	uint16_t 	TotalSectors16;		
	uint8_t		Media;
	uint16_t 	FatSize16;
	uint16_t 	SectorsPerTrack;
	uint16_t 	HeadCount;
	uint32_t	HiddenSectors;
	uint32_t 	TotalSectors32;
	// At offset 36, the BPB/boot sector for FAT12 and FAT16 differs from the 
	// BPB/Boot Sector for FAT32. Thus, we needed separate structs.
	uint8_t		DriveNumber;			// Unused since BIOS interrupts are not available
	uint8_t 	Reserved1; 				// Used by Windows NT, ALWAYS set to 0
	uint8_t 	ExtBootSig;
	uint32_t	VolumeID;				// Volume Serial Number
	char 		VolumeLabel[11];		// Volume label
	char		FileSystemType[8];		// File system type string (unsure)
}__attribute__((packed));

#define Fat16Entry struct entry_fat
struct entry_fat{
    char Filename[8];
    char Extension[3];
    unsigned char Attributes;
    unsigned char Reserved[10];
    unsigned short ModifyTime;
    unsigned short ModifyDate;
    unsigned short StartingCluster;
    unsigned long FileSize;
} __attribute((packed));

Fat16Entry Fat16RootDirEntries[64];
Fat12BootSector FatBootSector;
uint32_t RootDirLba;

void InitFat()
{
	AtaLbaRead(0, &FatBootSector, 1); // Read 1 sector's worth of data.
	RootDirLba = (	(FatBootSector.ReservedSectorCount - 1 + 
					 FatBootSector.FatSize16 * FatBootSector.FatCount) + 1);
	
	terminal_color = 0x1f;
	putch('\n');
	int m = 512;
	void* d = malloc(m);
	char* p = (char*)d;
	terminal_color = 0x1f;
	printf("%x", RootDirLba);
	AtaLbaRead(RootDirLba, d, 1);
	terminal_color = 0x1f;
	for(int i = 0; i < m; i++)
	{
		if(p[i] == 0)
		{
			putch('.');
		}
		else
		{
			putch(p[i]);
		}
	}
	memcpy(&Fat16RootDirEntries, d, m);
}

void FormatFat()
{
	for(int i = 0; i < 32; i++)
	{
		printf("File ");
		TerminalWrite(Fat16RootDirEntries[i].Filename, 8);
		putch('.');
		TerminalWrite(Fat16RootDirEntries[i].Extension, 3);
		printf(", size %d\n");
	}
	terminal_color = 0x1f;
}