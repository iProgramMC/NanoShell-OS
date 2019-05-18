#define NSFS_ONLY
#ifdef NSFS_ONLY
#define BootSector struct nsfs_boot_sector
BootSector
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

#define FatEntry struct nsfs_dir_entry
FatEntry
{
	char		FileName[8];
	char		Extension[3];
	#define ATTRIB_READONLY 	0x01
	#define ATTRIB_HIDDEN		0x02
	#define ATTRIB_SYSTEM		0x04
	#define ATTRIB_SUBDIR		0x10
	#define ATTRIB_ARCHIVE 		0x20
	#define ATTRIB_DEVICE		0x40
	uint8_t		Attributes;
    uint8_t 	Reserved[10];
    uint16_t 	ModifyTime;
    uint16_t 	ModifyDate;
    uint16_t 	StartingSector;
    uint32_t 	FileSize;
}__attribute__((packed));

BootSector bootSector;
FatEntry rootEntries[256];

#define File struct nsfs_file
File
{
	FatEntry* entry;
	uint8_t* fileCachePtr;
};


char* FileNameOpen = "        ";
char* ExtensionOpen = "   ";
#define DiskbufferSizeSectors 1024
#define DiskbufferSize 512 * DiskbufferSizeSectors // 512K of disk buffer should be enough to hold the first 512K of data area.
uint8_t DiskStack_Array[DiskbufferSize] = {0x0, };
uint8_t* DiskStack = DiskStack_Array;
#define FATBufSizeSectors 1024
#define FATBufSize 512 * DiskbufferSizeSectors // 512K of FAT buffer should be enough
uint16_t FATBuffer[FATBufSize] = {0x0, };

uint32_t RootDirLba = 0;
uint32_t DataAreaLba = 0;

bool FilesystemInitialized = false;

File fileCache[256];

void InitFilesystem_NSFS()
{
	AtaLbaReadS(0, &bootSector);
	if(memcmp(bootSector.FileSystemType, "NanoFS  ", 8) == 0 && memcmp(&bootSector, "NanoShellFS", 11) == 0)
	{
		for(int i = 1; i < 17; i++)
		{
			AtaLbaReadS(i, &rootEntries + (i - 1) * 512);
		}
		FilesystemInitialized = true;
	}
	else
	{
		puts("Could not find a valid NSFS partition in the computer. FAT is in development\nand is coming soon.\n\n");
	}
}
void InitFilesystem_FAT()
{
	AtaLbaReadS(0, &bootSector);
	RootDirLba = (	(bootSector.ReservedSectorCount - 1 + 
					 bootSector.FatSize16 * bootSector.FatCount) + 1);
	DataAreaLba = RootDirLba + (bootSector.FatEntryCount * 32) / 512;
	// Found the root directory, read everything in it
	for(int i = 0; i < bootSector.FatEntryCount; i++)
	{
		AtaLbaReadS(RootDirLba + i, &rootEntries + i * 512);
	}
	// We will also need to read the FAT.
	for(int i = 0; i < bootSector.FatSize16; i++)
	{
		AtaLbaReadS(1 + i, FATBuffer + i * 512);
	}
	HexDumpMemory(&rootEntries, 512);
	puts("\n\n");
	HexDumpMemory(&rootEntries, 512);
}

char* GetExtension(char* filename)
{
	for(int i = 0; i < 12; i++)
	{
		if(filename[i] == '.')
		{
			return filename + i;
			break;
		}
	}
	return filename;
}

int GetFSEntry(char* filename)
{
	if(FilesystemInitialized)
	{
		char* extension = GetExtension(filename);
		int flength = extension - filename;
		extension++;
		memset(FileNameOpen, 0x20, 8);
		memset(ExtensionOpen, 0x20, 3);
		memcpy(FileNameOpen, filename, flength);
		memcpy(ExtensionOpen, extension, 3);
		for(int i = 0; i < 256; i++)
		{
			if (memcmp(FileNameOpen, rootEntries[i].FileName, 8) == 0 &&
				memcmp(ExtensionOpen, rootEntries[i].Extension, 3) == 0)
			{
				// found the file
				return i;
				break;
			}
		}
		return -1;
	}
	else
	{
		puts("\nFile system features are not available right now. Sorry about that.");
	}
	return -1;
}

int notfceil(int t)
{
	int dp = t % 100;
	int f = t / 100;
	if(dp != 0)
	{
		return f + 1;
	}
	return f;
}

uint8_t* FileOpen(int entryIndex)
{
	if(FilesystemInitialized)
	{
		FatEntry* entry = &rootEntries[entryIndex];
		if(entryIndex == -1) return NULL;
		int sectorSize = notfceil(entry->FileSize * 100 / 512);
		if(fileCache[entryIndex].fileCachePtr != NULL)
		{
			return fileCache[entryIndex].fileCachePtr;
		}
		if(entry->FileSize != 0)
		{
			for(int i = 0; i < sectorSize; i++)
			{
				AtaLbaReadS(entry->StartingSector + i, DiskStack + i * 512);
			}
			fileCache[entryIndex].entry = entry;
			fileCache[entryIndex].fileCachePtr = DiskStack;
			DiskStack += entry->FileSize;
			return fileCache[entryIndex].fileCachePtr;
		}
		return NULL;
	}
	else
	{
		puts("\nFile system features are not available right now. Sorry about that.");
	}
	return NULL;
}

void PutDir()
{
	if(FilesystemInitialized)
	{
		printf("\n\nShowing root directory contents:\n");
		for(int i = 0; i < 256; i++)
		{
			if(memcmp(rootEntries[i].FileName, "\0\0\0\0\0\0\0\0", 8) == 0 &&
			memcmp(rootEntries[i].Extension, "\0\0\0", 3) == 0)
			{
				continue;
			}
			TerminalWrite(rootEntries[i].FileName, 8);
			TerminalWrite(rootEntries[i].Extension, 3);
			printf(", size %d\n", rootEntries[i].FileSize);
		}
	}
	else
	{
		puts("\nFile system features are not available right now. Sorry about that.");
	}
}
#endif