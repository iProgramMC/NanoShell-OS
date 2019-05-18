
void MallocTest()
{
	//char* m[513];
	//char te[10];
	//te[0] = 't'; te[1] = 'e'; te[2] = 's'; te[3] = 't'; te[4] = '0'; te[5] = '0'; te[9] = 0;
	//puts("Test starting...\n");
	//for(int i = 0; i < 513; i++)
	//{
	//	te[4] = i / 256;
	//	te[5] = i % 256;
	//	char* m = (char*) malloc(100, te);
	//	memcpy(m, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis mollis magna sem, in mollis cras amet", 100);
	//	TerminalWriteString(m);
	//}
	//puts("MAlloc test successful\n\n");
	//getch();
	//for(int i = 0; i < 512; i++)
	//{
	//	te[4] = i / 256;
	//	te[5] = i % 256;
	//	free(m, te);
	//}
	//puts("MALLOC TEST v1.0\n");
	//char* m = malloc(100);//, "alloc m");
	//if(m != NULL)
	//{
	//	memcpy(m, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis mollis magna sem, in mollis cras amet", 100);
	//	puts(m);
	//	char* c = malloc(50);//, "alloc c");
	//	memcpy(c, m, 50);
	//	puts(c);
	//	free(m);//, "free m");
	//	char* s = malloc(120);//, "alloc s");
	//	memcpy(s, c, 50);
	//	memcpy(s + 50, c + 50, 50);
	//	memcpy(s + 100, c + 100, 20);
	//	puts(s);
	//	free(s);//, "free s");
	//	free(c);//, "free c");
	//	puts("Test successful\n");
	//}
	//else
	//{
	//	puts("There is something wrong with malloc\n");
	//	char* s = "m -> 0x;;;;;;;;, r -> 0x;;;;;;;;";
	//	char* f = ToHex((uint32_t)m);
	//	memcpy(s + 7, f, 8);
	//	f = ToHex((uint32_t)memory);
	//	memcpy(s + 24, f, 8);
	//	puts(s);
	//}
	
	puts("Bug testing facility. Version 1.0\nIf the part of the bottom string is found at the start of the top string, that\nmeans the bug still exists.\n");
	char* s = malloc(25);
	char* m = malloc(50);
	memcpy(m, "Lorem ipsum dolor sit amet, consectetur volutpat.", 50);
	free(s);
	char* p = malloc(30);
	s = malloc(50);
	memcpy(s, "Lorem ipsum dolor sit amet, consectetur volutpat.", 50);
	puts("m: ");
	puts(m);
	puts("\ns: ");
	puts(s);
	puts(   "\n---                        |------------------------|");
	free(p);
	free(s);
	free(m);
	puts("\n\nTest results:\nBug existing: ");
	p = "                         ";
	memcpy(p, s + 25, 25);
	if(strcmp(p, s + 25) == 0)
	{
		puts("No");
	}
	else
	{
		puts("Yes");
	}
}