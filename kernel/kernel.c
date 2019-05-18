/*
   NanoShell OS
   
   Kernel.c -> Main kernel functions
   
   - Also serves to unite all of the other code (spread around in builtin/ and drivers/.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define max(a, b) a > b ? a : b
#define min(a, b) a < b ? a : b

#define VERSION_ID "0.0.1 beta"
#define BUILD_ID "200"
#define OS_NAME "NanoShell OS"
#define COPYRIGHT_MSG "(c) 2019 iProgramInCpp"

#define BYTE unsigned char
#include "archdep.h"
#include "drivers/terminal.h"
#include "data.h"
#include "memory.h"
#include "drivers/port.h"
#include "drivers/serial.h"
#include "stdio.h"
#include "string.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "drivers/hdrive.h"
#include "drivers/filesystem.h"
#include "nsil.h"
#include "nstinyil.h"
#include "multiboot.h"


// Include the applications
#include "../AllApps.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This project will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This project needs to be compiled with a ix86-elf compiler"
#endif
 
#include "builtin/tests/malloc.h"
#include "builtin/tests/keybtest.h"

static bool VerboseBoot = false;
void verbose_puts(const char*s) { if(VerboseBoot) { puts(s); } }
void KernelMain(unsigned long magic, unsigned long addr) 
{
	multiboot_info_t* mbi;
	/* Initialize terminal interface */
	InitTerminal();
	verbose_puts("Please wait...\n");
	terminal_color = 0x1f;
	verbose_puts("Switching to 80x50 text mode...\n");
	SwitchMode(TMode80x50);
	//terminal_row = 0;
	//terminal_column = 0;
	verbose_puts("Please wait...\n");
	TerminalClear();
	
	printf("%s version %s [build %s]\n%s\n\n", OS_NAME, VERSION_ID, BUILD_ID, COPYRIGHT_MSG);
	serprintf("%s version %s [build %s]\r\n%s\r\n\r\n", OS_NAME, VERSION_ID, BUILD_ID, COPYRIGHT_MSG);
	
    /* Am I booted by a Multiboot-compliant boot loader? */
	verbose_puts("Checking multiboot status...");
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf ("Invalid magic number: 0x%x\n", (unsigned) magic);
        return;
    }
	
	// Check MBI
	mbi = (multiboot_info_t*) addr;
	
	#ifdef MULTIBOOT_DEBUG
	printf("flags = %x\n", (unsigned)mbi->flags);
	#endif
	
	verbose_puts("Checking memory...\n");
	// Are mem_* valid?
	if(MbCheckFlag(mbi->flags, 0))
		printf("[%d Kb Main Memory]\n",
			(unsigned)mbi->mem_upper);
	if(mbi->mem_upper < 120000)
	{
		printf("\n\nNanoShell has not found enough extended memory. 128 MB of extended\nmemory is required\
to run NanoShell. You may need to upgrade your\ncomputer or run a configuration program provided by the\
 manufacturer.\n\nHere is the memory map provided by GRUB.\n");
		/* Are mmap_* valid? */
		//if (MbCheckFlag (mbi->flags, 6)) {
		//	multiboot_memory_map_t *mmap;
	    //
		//	printf ("mmap_addr = %x, mmap_length = %x\n",
		//			(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		//	for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
		//			(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
		//			mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
		//				+ mmap->size + sizeof (mmap->size))) {
		//		printf (" size = %x, base_addr = %x %x,"
		//				" length = %x %x, type = %x\n",
		//				(unsigned) mmap->size,
		//				mmap->addr >> 32,
		//				mmap->addr & 0xffffffff,
		//				mmap->len >> 32,
		//				mmap->len & 0xffffffff,
		//				(unsigned) mmap->type);
		//	}
		//}
		return;
	}
	
	// Is the boot_device valid?
	verbose_puts("Checking the boot device...\n");
	if(MbCheckFlag(mbi->flags, 1))
	#ifdef MULTIBOOT_DEBUG
		printf("boot_device = %x\n", (unsigned) mbi->boot_device);
	#endif
	
	// Is the command line passed?
	verbose_puts("Checking GRUB command line...\n");
	if(MbCheckFlag(mbi->flags, 2))
	#ifdef MULTIBOOT_DEBUG
		printf("cmdline = %s\n", (char*)mbi->cmdline);
	#endif
	
	// Are the mods_* valid?
	verbose_puts("Checking mods...\n");
	if(MbCheckFlag(mbi->flags, 3))
	{
		#ifdef MULTIBOOT_DEBUG
		multiboot_module_t* mod;
		unsigned int i;
		printf ("mods_count = %d, mods_addr = 0x%x\n",
                (int) mbi->mods_count, (int) mbi->mods_addr);
        for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
                i < mbi->mods_count;
                i++, mod++) {
            printf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
                    (unsigned) mod->mod_start,
                    (unsigned) mod->mod_end,
                    (char *) mod->cmdline);
        }
		#endif
	}
	
    /* Bits 4 and 5 are mutually exclusive! */
	verbose_puts("Checking bits 4 and 5...\n");
    if (MbCheckFlag (mbi->flags, 4) && MbCheckFlag (mbi->flags, 5)) {
	#ifdef MULTIBOOT_DEBUG
        printf ("Both bits 4 and 5 are set.\n");
        return;
	#endif
    }
    /* Is the symbol table of a.out valid? */
	verbose_puts("Checking a.out symbol table validity...\n");
    if (MbCheckFlag (mbi->flags, 4)) {
	#ifdef MULTIBOOT_DEBUG
        multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);

        printf ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
                "strsize = 0x%x, addr = 0x%x\n",
                (unsigned) multiboot_aout_sym->tabsize,
                (unsigned) multiboot_aout_sym->strsize,
                (unsigned) multiboot_aout_sym->addr);
	#endif
    }

    /* Is the section header table of ELF valid? */
	verbose_puts("Checking ELF section header table...\n");
    if (MbCheckFlag (mbi->flags, 5)) {
	#ifdef MULTIBOOT_DEBUG
        multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

        printf ("multiboot_elf_sec: num = %u, size = 0x%x,"
                " addr = 0x%x, shndx = 0x%x\n",
                (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
                (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
	#endif
    }

    /* Are mmap_* valid? */
	verbose_puts("Checking mmap.\n");
    if (MbCheckFlag (mbi->flags, 6)) {
	#ifdef MULTIBOOT_DEBUG
        multiboot_memory_map_t *mmap;

        printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
                (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
        for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
                (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                    + mmap->size + sizeof (mmap->size))) {
            printf (" size = 0x%x, base_addr = 0x%x%x,"
                    " length = 0x%x%x, type = 0x%x\n",
                    (unsigned) mmap->size,
                    mmap->addr >> 32,
                    mmap->addr & 0xffffffff,
                    mmap->len >> 32,
                    mmap->len & 0xffffffff,
                    (unsigned) mmap->type);
        }
	#endif
    }
	
	verbose_puts("Initializing the IDT...\n");
	InitIDT();
	verbose_puts("Initializing keyboard...\n");
	InitKeyboard();
	puts("To initialize the file system, type 'fsinit'.\n");
	//InitFilesystem();
	verbose_puts("Initializing serial port COM1...\n");
	InitSerial(0);
	//printf("x: %d, y: %d, m: %x\r\n", 10, 20, 30);
	
	terminal_color = 0x1f;
	verbose_puts("Done!\n");
	
	CommandPrompt_Main();
}