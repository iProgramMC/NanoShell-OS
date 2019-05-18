int memcmp(const void* ap, const void* bp, size_t size)
{
	const BYTE* a = (const BYTE*) ap;
	const BYTE* b = (const BYTE*) bp;
	for(size_t i = 0; i < size; i++)
	{
		if(a[i] < b[i])
			return -1;
		else if(b[i] < a[i])
			return 1;
	}
	return 0;
}

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size)
{
	BYTE* dst = (BYTE*) dstptr;
	const BYTE* src = (const BYTE*) srcptr;
	for(size_t i = 0; i < size; i++)
	{
		dst[i] = src[i];
	}
	return dstptr;
}

void* memmove(void* dstptr, const void* srcptr, size_t size)
{
	BYTE* dst = (BYTE*) dstptr;
	const BYTE* src = (const BYTE*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

void* memset(void* bufptr, BYTE val, size_t size)
{
	BYTE* buf = (BYTE*) bufptr;
	for(size_t i = 0; i < size; i++)
	{
		buf[i] = val;
	}
	return bufptr;
}