#define NSFS_ONLY

uint8_t DriveNumber = 0; // Primary Master

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
	char  		BootSectionCode[448];
	uint16_t 	BootSignature;
}__attribute__((packed));

#define Partition struct nsfs_partition
Partition
{
	// This is the partition data
	uint8_t 	BootIndicator;
	uint8_t		StartCHS [3];
	uint8_t		PartTypeDesc;
	uint8_t		EndCHS [3];
	uint32_t	StartSector;
	uint32_t	PartSizeSectors;
}__attribute__((packed));

#define MasterRecord struct nsfs_master_record
MasterRecord
{
	// We mostly only need it to find the boot sector.
	uint8_t 	BootLoaderCode[446];
	Partition	PartitionTable[4];
	uint16_t	BootSignature;
}__attribute__((packed));


#define FatEntry struct nsfs_root_entry
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
#define FATBufSize 256 * DiskbufferSizeSectors // 512K of FAT buffer should be enough
uint16_t FATBuffer[FATBufSize] = {0x0, };

uint32_t RootDirLba = 0;
uint32_t DataAreaLba = 0;

bool FilesystemInitialized = false;

File fileCache[256];

void InitFilesystem_NSFS()
{
	AtaLbaReadS(0, &bootSector, DriveNumber);
	if(memcmp(bootSector.FileSystemType, "NanoFS  ", 8) == 0 && memcmp(&bootSector, "NanoShellFS", 11) == 0)
	{
		for(int i = 1; i < 17; i++)
		{
			AtaLbaReadS(i, &rootEntries + (i - 1) * 512, DriveNumber);
		}
		FilesystemInitialized = true;
	}
	else
	{
		puts("Could not find a valid NSFS partition in the computer. FAT is in development\nand is coming soon.\n\n");
	}
}
uint32_t PartitionSectorStart = 0;
MasterRecord masterBootRecord; // Store a copy of the MBR in memory

//uint32_t CHStoLBA(uint32_t c, uint32_t h, uint32_t s)
//{
//	A = (c * Nheads + h) * Nsectors + (s − 1);
//}

void InitFilesystem/*_FAT*/()
{
	// Get the partition start's LBA
	AtaLbaReadS(0, &masterBootRecord, DriveNumber);
	if(masterBootRecord.BootLoaderCode[54] == 'F' && masterBootRecord.BootLoaderCode[55] == 'A' && masterBootRecord.BootLoaderCode[56] == 'T')
	{
		PartitionSectorStart = 0; 
		// This is a drive with a MBR (unlike FDDs), which is supposed to have just one partition.
	}
	else
	{
		// This partition is backed by an MBR.
		PartitionSectorStart = masterBootRecord.PartitionTable[0].StartSector;
	}
	// Then read the file system metadata
	// 3852075008
	AtaLbaReadS(PartitionSectorStart, &bootSector, DriveNumber);
	RootDirLba = (	(bootSector.ReservedSectorCount - 1 + 
					 bootSector.FatSize16 * bootSector.FatCount) + 1);
	DataAreaLba = RootDirLba + (bootSector.FatEntryCount * 32) / 512;
	// Found the root directory, read everything in it
	for(int i = 0; i < bootSector.FatEntryCount; i++)
	{
		AtaLbaReadS(RootDirLba + i + PartitionSectorStart, &rootEntries + i * 512, DriveNumber);
	}
	// We will also need to read FAT #1.
	serprintf("BootSectorReservedSectorCount: %x\r\n", bootSector.ReservedSectorCount);
	serprintf("FAT Size:                      %x\r\n", bootSector.FatSize16);
	//AtaLbaRead(bootSector.ReservedSectorCount + PartitionSectorStart, FATBuffer, bootSector.FatSize16);
	for(int i = 0; i < bootSector.FatSize16; i++)
	{
		AtaLbaReadS(bootSector.ReservedSectorCount + PartitionSectorStart + i, FATBuffer + i * 512, DriveNumber);	
	}
	FilesystemInitialized = true;
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
		memtoupper(FileNameOpen, 8);
		memtoupper(ExtensionOpen, 3);
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
		//if(fileCache[entryIndex].fileCachePtr != NULL)
		//{
		//	return fileCache[entryIndex].fileCachePtr;
		//}
		serprintf("rdl, ddl: %x %x %x\r\n", RootDirLba, DataAreaLba, PartitionSectorStart);
		if(entry->FileSize != 0)
		{
			// ToDo
			int StartingCluster = entry->StartingSector * bootSector.SectorsPerCluster;
			AtaLbaReadS((StartingCluster) + PartitionSectorStart + DataAreaLba - 2, DiskStack, DriveNumber);
			serprintf("PSS: %x\r\n", PartitionSectorStart);
			serprintf("SCluster: %d\r\n", StartingCluster);
			
			if(entry->FileSize <= 512)
			{
				fileCache[entryIndex].entry = entry;
				fileCache[entryIndex].fileCachePtr = DiskStack;
				DiskStack += entry->FileSize;
				return fileCache[entryIndex].fileCachePtr;
			}
			else
			{
				int f = FATBuffer[entry->StartingSector];
				serprintf("FAT INDEX: %d\r\n", StartingCluster);
				if(f == 0xffff || f == 0xfff8 || f == 0xfff7)
				{
					fileCache[entryIndex].entry = entry;
					fileCache[entryIndex].fileCachePtr = DiskStack;
					DiskStack += entry->FileSize;
					return fileCache[entryIndex].fileCachePtr;
				}
				serprintf("FAT INDEX: %d\r\n", f);
				int i = 1;
				do
				{
					int next_s_f = f * bootSector.SectorsPerCluster;
					AtaLbaReadS((next_s_f) + PartitionSectorStart + DataAreaLba - 2, DiskStack + i * 512, DriveNumber);
					i++;
					serprintf("FAT INDEX: %d\r\n", next_s_f);
					f = FATBuffer[f];
				}
				while(f != 0xffff && f != 0xfff8 && f != 0xfff7);
				if(f == 0xfff7) return NULL; // File might be corrupted, just say it does not exist.
				fileCache[entryIndex].entry = entry;
				fileCache[entryIndex].fileCachePtr = DiskStack;
				DiskStack += entry->FileSize;
				return fileCache[entryIndex].fileCachePtr;
			}
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
			bool nulled = (memcmp(rootEntries[i].FileName, "\0\0\0\0\0\0\0\0", 8) == 0 && memcmp(rootEntries[i].Extension, "\0\0\0", 3) == 0);
			bool deleted = ((uint8_t)rootEntries[i].FileName[0] == 0xe5);
			if(nulled || deleted)
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