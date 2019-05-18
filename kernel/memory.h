#define BlockInfo struct blockinfo
#define BlockCount 4096

#define MemoryUsable (1024 * 1024 * 64) // 8 Megabytes of usable heap

void puts(const char*s);

//#define DEBUG

uint8_t memory[MemoryUsable];

/*
	There is no need for a USED tag.
	If the length == 0 and start == NULL, we can assume
	the pointer is freed already, so we don't need an
	extra "used" tag.
*/
BlockInfo
{
	void* start;
	size_t length;
};

BlockInfo blockInfo[BlockCount];

#ifdef MDEBUG
void* FindFreeMemory(size_t size, const char* why)
#else
void* FindFreeMemory(size_t size)
#endif
{
	// Go through each block and find a free one
	for(int i = 0; i < BlockCount; i++)
	{
		// We've found a free block.
		if(blockInfo[i].start == NULL && blockInfo[i].length == 0)
		{
			if(size > MemoryUsable)
			{
			#ifdef DEBUG
				puts("Memory to allocate too big (reason: ");
				puts(why);
				puts(") \n");
			#else
				puts("Memory to allocate too big\n");
			#endif
				return NULL;
			}
			else
			{
				if(i == 0)
				{
					void* j = memory;
					size_t len = 0;
					if(blockInfo[i + 1].start == NULL && blockInfo[i + 1].length == 0)
					{
						len = MemoryUsable;
					}
					else
					{
						len = blockInfo[i + 1].start - j;
					}
					if(len >= size)
					{
						// we found some free space!
						blockInfo[i].start = j;
						blockInfo[i].length = size;
						return j;
					}
					else
					{
						// continue to find free space
						continue;
					}
					//continue;
				}
				else
				{
					// Get pointer of this.
					void* j = blockInfo[i - 1].start + blockInfo[i - 1].length;
					size_t len = 0;
					if(blockInfo[i + 1].start == NULL && blockInfo[i + 1].length == 0)
					{
						len = MemoryUsable;
					}
					else
					{
						len = blockInfo[i + 1].start - j;
					}
					if(len >= size)
					{
						// we found some free space!
						blockInfo[i].start = j;
						blockInfo[i].length = size;
						return j;
					}
					else
					{
						// continue to find free space
						continue;
					}
				}
			}
		}
	}
#ifdef MDEBUG
	puts("Out of memory (reason: ");
	puts(why);
	puts(") \n");
#else
	puts("Out of memory\n");
#endif
	return NULL;
}

#ifdef MDEBUG
void* malloc(size_t size, const char* why)
{
	return FindFreeMemory(size, why);
}
#else
void* malloc(size_t size)
{
	return FindFreeMemory(size);
}
#endif

#ifdef MDEBUG
void free(void* ptr, const char* why)
#else
void free(void* ptr)
#endif
{
	if((uint8_t*)ptr < memory)
	{
#ifdef MDEBUG
		puts("Free failed, not part of malloc ecosystem (reason for free: ");
		puts(why);
		puts(") \n");
#else
		puts("Free failed, not part of malloc ecosystem (address smaller than memory)\n");
#endif
		return;
	}
	if(ptr == NULL)
	{
#ifdef MDEBUG
		puts("Free failed, got null pointer (reason for free: ");
		puts(why);
		puts(") \n");
#else
		puts("Free failed, got null pointer\n");
#endif
		return;
	}
	for(int i = 0; i < BlockCount; i++)
	{
		// Compare the two addresses, see if they are the same
		if(ptr == blockInfo[i].start)
		{
			// Zero-fill the pointer's address
			memset(blockInfo[i].start, 0, blockInfo[i].length);
			// and make it freed
			blockInfo[i].length = 0;
			blockInfo[i].start = 0;
			return;
		}
	}
#ifdef MDEBUG
	puts("Free failed, not part of malloc ecosystem (reason for free: ");
	puts(why);
	puts(") \n");
#else
	puts("Free failed, not part of malloc ecosystem (could not find associated block)\n");
#endif
}