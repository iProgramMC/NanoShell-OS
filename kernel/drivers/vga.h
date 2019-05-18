/*
   NanoShell OS
   
   Drivers/VGA.h -> VGA functions
   
   - No notes available
*/
/* 
  VGA Library Lite
  https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
*/

#include "vgafont.h"

#define	inportb(P)			ReadPort(P)
#define	outportb(P,V)		WritePort(P, V)
#define	inportw(P)			ReadPortW(P)
#define	outportw(P,V)		WritePortW(P, V)

#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ		0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA

#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21
#define	VGA_NUM_REGS		(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
				VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
				
#define MK_FP(S, O) ((S << 4) + O)
#define FP_SEG(F) (F >> 16)
#define FP_OFF(F) (F & 0xffff)
			
#define	peekb(S,O)		*(unsigned char *)MK_FP(S, O)
#define	pokeb(S,O,V)		*(unsigned char *)MK_FP(S, O) = (V)
#define	pokew(S,O,V)		*(unsigned short *)MK_FP(S, O) = (V)
#define ppokeb(A,V)			*(unsigned char*)(A) = (V)
#define ppeekb(A)			*(unsigned char*)(A)
#define	_vmemwr(DS,DO,S,N)	movedata(FP_SEG(S), FP_OFF(S), DS, DO, N)
#define	vmemwr(a,b,c)	memcpy((void*)(a),b,c);

#define OptimizeVGA

static void (*GWritePixel)(unsigned x, unsigned y, unsigned c);
uint16_t GWidth, GHeight;
extern void WriteFont8px  (unsigned char* buf); // KERNEL.ASM!!
extern void WriteFont16px (unsigned char* buf); // KERNEL.ASM!!
			
unsigned char g_640x480x16[] =
{
/* MISC */
	0xE3,
/* SEQ */
	0x03, 0x01, 0x08, 0x00, 0x06,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x01, 0x00, 0x0F, 0x00, 0x00
};

unsigned char g_90x30_text[] =
{
/* MISC */
	0xE7,
/* SEQ */
	0x03, 0x01, 0x03, 0x00, 0x02,
/* CRTC */
	0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x2D, 0x10, 0xE8, 0x05, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};
unsigned char g_640x480x2[] =
{
/* MISC */
	0xE3,
/* SEQ */
	0x03, 0x01, 0x0F, 0x00, 0x06,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x01, 0x00, 0x0F, 0x00, 0x00
};
unsigned char g_320x200x256[] =
{
/* MISC */
	0x63,
/* SEQ */
	0x03, 0x01, 0x0F, 0x00, 0x0E,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9C, 0x0E, 0x8F, 0x28,	0x40, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, 0x00, 0x0F, 0x00,	0x00
};

unsigned char g_80x25_text[] =
{
/* MISC */
	0x67,
/* SEQ */
	0x03, 0x00, 0x03, 0x00, 0x02,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
	0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00
};

unsigned char g_80x50_text[] =
{
/* MISC */
	0x67,
/* SEQ */
	0x03, 0x00, 0x03, 0x00, 0x02,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
	0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x01, 0x40,
	0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF, 
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF, 
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};

unsigned char g_90x60_text[] =
{
/* MISC */
	0xE7,
/* SEQ */
	0x03, 0x01, 0x03, 0x00, 0x02,
/* CRTC */
	0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
	0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};


static unsigned GetFbSeg(void)
{
	unsigned seg;

	outportb(VGA_GC_INDEX, 6);
	seg = inportb(VGA_GC_DATA);
	seg >>= 2;
	seg &= 3;
	switch(seg)
	{
	case 0:
	case 1:
		seg = 0xA000;
		break;
	case 2:
		seg = 0xB000;
		break;
	case 3:
		seg = 0xB800;
		break;
	}
	return seg;
}

static void vpokeb(unsigned off, unsigned val)
{
	pokeb(GetFbSeg(), off, val);
}

static unsigned vpeekb(unsigned off)
{
	return peekb(GetFbSeg(), off);
}

void ReadRegs(unsigned char *regs)
{
	unsigned i;

/* read MISCELLANEOUS reg */
	*regs = inportb(VGA_MISC_READ);
	regs++;
/* read SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		*regs = inportb(VGA_SEQ_DATA);
		regs++;
	}
/* read CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outportb(VGA_CRTC_INDEX, i);
		*regs = inportb(VGA_CRTC_DATA);
		regs++;
	}
/* read GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outportb(VGA_GC_INDEX, i);
		*regs = inportb(VGA_GC_DATA);
		regs++;
	}
/* read ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inportb(VGA_INSTAT_READ);
		outportb(VGA_AC_INDEX, i);
		*regs = inportb(VGA_AC_READ);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inportb(VGA_INSTAT_READ);
	outportb(VGA_AC_INDEX, 0x20);
}

void WriteRegs(unsigned char *regs)
{
	unsigned i;

/* write MISCELLANEOUS reg */
	outportb(VGA_MISC_WRITE, *regs);
	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		outportb(VGA_SEQ_DATA, *regs);
		regs++;
	}
/* unlock CRTC registers */
	outportb(VGA_CRTC_INDEX, 0x03);
	outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) | 0x80);
	outportb(VGA_CRTC_INDEX, 0x11);
	outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outportb(VGA_CRTC_INDEX, i);
		outportb(VGA_CRTC_DATA, *regs);
		regs++;
	}
/* write GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outportb(VGA_GC_INDEX, i);
		outportb(VGA_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inportb(VGA_INSTAT_READ);
		outportb(VGA_AC_INDEX, i);
		outportb(VGA_AC_WRITE, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inportb(VGA_INSTAT_READ);
	outportb(VGA_AC_INDEX, 0x20);
}

static void SetPlane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
/* set read plane */
	outportb(VGA_GC_INDEX, 4);
	outportb(VGA_GC_DATA, p);
/* set write plane */
	outportb(VGA_SEQ_INDEX, 2);
	outportb(VGA_SEQ_DATA, pmask);
}

/*
static void WriteFont(unsigned char *buf, unsigned font_height)
{
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned i;

	outportb(VGA_SEQ_INDEX, 2);
	seq2 = inportb(VGA_SEQ_DATA);

	outportb(VGA_SEQ_INDEX, 4);
	seq4 = inportb(VGA_SEQ_DATA);
	outportb(VGA_SEQ_DATA, seq4 | 0x04);

	outportb(VGA_GC_INDEX, 4);
	gc4 = inportb(VGA_GC_DATA);

	outportb(VGA_GC_INDEX, 5);
	gc5 = inportb(VGA_GC_DATA);
	outportb(VGA_GC_DATA, gc5 & ~0x10);

	outportb(VGA_GC_INDEX, 6);
	gc6 = inportb(VGA_GC_DATA);
	outportb(VGA_GC_DATA, gc6 & ~0x02);
	SetPlane(2);
	for(i = 0; i < 256; i++)
	{
		vmemwr(0xa0000 + 16384u * 0 + i * 32, buf, font_height);
		buf += font_height;
	}
#if 0
	for(i = 0; i < 256; i++)
	{
		vmemwr(0xa0000 + 16384u * 1 + i * 32, buf, font_height);
		buf += font_height;
	}
#endif
	outportb(VGA_SEQ_INDEX, 2);
	outportb(VGA_SEQ_DATA, seq2);
	outportb(VGA_SEQ_INDEX, 4);
	outportb(VGA_SEQ_DATA, seq4);
	outportb(VGA_GC_INDEX, 4);
	outportb(VGA_GC_DATA, gc4);
	outportb(VGA_GC_INDEX, 5);
	outportb(VGA_GC_DATA, gc5);
	outportb(VGA_GC_INDEX, 6);
	outportb(VGA_GC_DATA, gc6);
}*/

#ifndef OptimizeVGA
static void WritePixel4P(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes, off, mask, p, pmask;

	wd_in_bytes = GWidth / 8;
	off = wd_in_bytes * y + x / 8;
	x = (x & 7) * 1;
	mask = 0x80 >> x;
	pmask = 1;
	for(p = 0; p < 4; p++)
	{
		SetPlane(p);
		if(pmask & c)
			vpokeb(off, vpeekb(off) | mask);
		else
			vpokeb(off, vpeekb(off) & ~mask);
		pmask <<= 1;
	}
}
#endif

static void WritePixel1P(unsigned x, unsigned y, unsigned c2)
{
	unsigned wd_in_bytes, off, mask;

	bool c = c2%2;
	
	wd_in_bytes = GWidth / 8;
	off = wd_in_bytes * y + x / 8;
	x = (x & 7) * 1;
	mask = 0x80 >> x;
	if(c)
		vpokeb(off, vpeekb(off) | mask);
	else
		vpokeb(off, vpeekb(off) & ~mask);
}

static void WritePixel8(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes;
	unsigned off;

	wd_in_bytes = GWidth;
	off = wd_in_bytes * y + x;
	vpokeb(off, c);
}


#define VGA_HIRES_MODE_WIDTH 640
#define VGA_HIRES_MODE_HEIGHT 480
#define VGA_HIRES_MODE_PIXELSPERBYTE 2

uint8_t HiresFrameBuffer[153600];
bool Optimize = false;

void ClearScreen()
{
	if(Optimize == false)
	{
		for(int y = 0; y < GHeight; y++)
			for(int x = 0; x < GWidth; x++)
				GWritePixel(x, y, 0);
	}
	else
	{
		memset(HiresFrameBuffer, 0x00, 153600);
	}
}

void FillScreen(uint8_t color)
{
	if(Optimize == false)
	{
		for(int y = 0; y < GHeight; y++)
			for(int x = 0; x < GWidth; x++)
				GWritePixel(x, y, color);
	}
	else
	{
		unsigned p, pmask;
		pmask = 1;
		for(p = 0; p < 4; p++)
		{
			if(pmask & color)
			{
				memset(HiresFrameBuffer, 0xff, 38400);
			}
			else
			{
				memset(HiresFrameBuffer, 0x00, 38400);
			}
			pmask <<= 1;
		}
	}
}


void PushFrame()
{
	void* video_buffer = (void*)0xa0000;
	for(int p = 0; p < 4; p++)
	{
		SetPlane(p);
		fast_memcpy(video_buffer, HiresFrameBuffer + p * 38400, 38400);
	}
}

void WritePixelHires(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes, off, mask, p, pmask;

	wd_in_bytes = GWidth / 8;
	off = wd_in_bytes * y + x / 8;
	x = (x & 7) * 1;
	mask = 0x80 >> x;
	pmask = 1;
	for(p = 0; p < 4; p++)
	{
		if(pmask & c)
			ppokeb(HiresFrameBuffer + p * 38400 + off, ppeekb(HiresFrameBuffer + p * 38400 + off) | mask);
		else
			ppokeb(HiresFrameBuffer + p * 38400 + off, ppeekb(HiresFrameBuffer + p * 38400 + off) & ~mask);
		pmask <<= 1;
	}
}


#define GMode640x480x16 0x12
#define GMode320x200x256 0x13
#define GMode640x480x2 0x14
#define TMode80x25 0xea
#define TMode80x50 0xc3
#define TMode90x30 0xfa
#define TMode90x60 0xce
bool GMode = false;
void SwitchMode(uint8_t mode)
{
	switch(mode)
	{
		case GMode640x480x16:
		#ifdef OptimizeVGA
			WriteRegs(g_640x480x16);
			GWidth = VGA_HIRES_MODE_WIDTH;
			GHeight = VGA_HIRES_MODE_HEIGHT;
			GWritePixel = WritePixelHires;
			GMode = true;
			Optimize = true;
		#else
			WriteRegs(g_640x480x16);
			GWidth = VGA_HIRES_MODE_WIDTH;
			GHeight = VGA_HIRES_MODE_HEIGHT;
			GWritePixel = WritePixel4P;
			GMode = true;
			Optimize = false;
		#endif
			break;
		case GMode640x480x2:
			WriteRegs(g_640x480x2);
			GWidth = 640;
			GHeight = 480;
			GWritePixel = WritePixel1P;
			GMode = true;
			Optimize = false;
			break;
		case GMode320x200x256:
			WriteRegs(g_320x200x256);
			GWidth = 320;
			GHeight = 240;
			GWritePixel = WritePixel8;
			GMode = true;
			Optimize = false;
			break;
		case TMode80x25:
			WriteRegs(g_80x25_text);
			VGA_WIDTH = 80;
			VGA_HEIGHT = 25;
			GMode = false;
			WriteFont16px(g_8x16_font);
			Optimize = false;
			break;
		case TMode80x50:
			WriteRegs(g_80x50_text);
			VGA_WIDTH = 80;
			VGA_HEIGHT = 50;
			GMode = false;
			WriteFont8px(g_8x8_font);
			Optimize = false;
			break;
		case TMode90x30:
			WriteRegs(g_90x30_text);
			VGA_WIDTH = 90;
			VGA_HEIGHT = 30;
			GMode = false;
			WriteFont16px(g_8x16_font);
			Optimize = false;
			break;
		case TMode90x60:
			WriteRegs(g_90x60_text);
			VGA_WIDTH = 90;
			VGA_HEIGHT = 60;
			GMode = false;
			WriteFont8px(g_8x8_font);
			Optimize = false;
			break;
	}
}