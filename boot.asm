bits 32
section .text
        ;multiboot v1 spec
        align 4
        dd 0x1BADB002              ;magic
        dd 0x00                    ;flags
        dd - (0x1BADB002 + 0x00)   ;checksum. m+f+c should be zero

global _start
extern KernelMain

GDTStart:
GDTNull:
	dd 0x0
	dd 0x0
GDTCode:
	dw 0xffff
	dw 0x00
	db 0x00
	db 0x9a
	db 0xcf
	db 0x00
GDTData:
	dw 0xffff
	dw 0x00
	db 0x00
	db 0x92
	db 0xcf
	db 0x00
GDTEnd:
GDTDescription:
	dw GDTEnd - GDTStart - 1
	dd GDTStart
CODE_SEG equ GDTCode - GDTStart
DATA_SEG equ GDTData - GDTStart
InitGDT:
	lgdt [GDTDescription]
	jmp CODE_SEG:.set_cs
.set_cs:
	mov eax, DATA_SEG
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	ret
_start:
	cli 				; block interrupts
	mov esp, stack_space
	mov edx, eax 		; Make a backup of EAX, that InitGDT bastard uses and destroys it...
	call InitGDT
	push ebx			; Push the pointer to the Multiboot information struct
	push edx			; Push the magic number
	call KernelMain
mht:hlt 				; halt the CPU
	jmp mht

section .bss
resb 8192; 8KB for stack
stack_space: