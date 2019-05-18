
size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
	{
		len++;
	}
	return len;
}
void* strcpy(const char* ds, const char* ss)
{
	return memcpy((void*)ds, (void*)ss, strlen(ss));
}
void strtolower(char* as)
{
	while(*as != 0)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
	}
}
void strtoupper(char* as)
{
	while(*as != 0)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
	}
}
void memtolower(char* as, int w)
{
	int a = 0;
	while(a <= w)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
		a++;
	}
}
void memtoupper(char* as, int w)
{
	int a = 0;
	while(a <= w)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
		a++;
	}
}

int strcmp(const char* as, const char* bs)
{
	size_t al = strlen(as);
	size_t bl = strlen(bs);
	if(al < bl)
	{
		return -1;
	}
	else if(al > bl)
	{
		return 1;
	}
	else if(al == bl)
	{
		return memcmp((void*)as, (void*)bs, al);
	}
	return 0;
}

bool _____ContainsDelimiters(const char* str, const char* delimiters)
{
	for(uint32_t i = 0; i < strlen(delimiters); i++)
	{
		if(*str == delimiters[i]) return true;
	}
	if(*str == 0) return true;
	return false;
}
bool _____ContainsDelimiter(const char* str, const char delimiter)
{
	if(*str == delimiter) return true;
	if(*str == 0) return true;
	return false;
}

const char* strBeingScanned______ = NULL;
char* strtok_(const char* str, const char* delimiters)
{
	if(str != NULL)
	{	
		int l = 0;
		while(_____ContainsDelimiters(++str, delimiters))
		{
			l++;
		}
		// now we've found position
		char* o = (char*)malloc(l);
		memcpy(o, str, l);
		strBeingScanned______ = str + l + 1;
		return o;
	}
	else
	{
		const char* str = strBeingScanned______;
		int l = 0;
		while(_____ContainsDelimiters(++str, delimiters))
		{
			l++;
		}
		// now we've found position
		char* o = malloc(l);
		memcpy(o, str, l);
		strBeingScanned______ = str + l + 1;
		return o;
	}
	return NULL;
}
char* strtok(const char* str, char delimiter)
{
	if(str != NULL)
	{	
		int l = 0;
		while(_____ContainsDelimiter(++str, delimiter))
		{
			l++;
		}
		// now we've found position
		char* o = malloc(l);
		memcpy(o, str, l);
		strBeingScanned______ = str + l + 1;
		return o;
	}
	else
	{
		const char* str = strBeingScanned______;
		int l = 0;
		while(_____ContainsDelimiter(++str, delimiter))
		{
			l++;
		}
		// now we've found position
		char* o = malloc(l);
		memcpy(o, str, l);
		strBeingScanned______ = str + l + 1;
		return o;
	}
	return NULL;
}

int atoi(const char* s)
{
	int val = 0;
	while(*s != 0)
	{
		if(*s >= '0' && *s <= '9')
		{
			val += (*s - '0');
			val *= 10;
		}
		else
		{
			return 0;
		}
		s++;
	}
	return 0;
}